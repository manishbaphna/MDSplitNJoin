#pragma once

#include <string>
#include <sstream>
#include <map>
#include <thread>
#include <mutex>
#include <memory>

#include <condition_variable>

#include <vector>
#include <queue>
#include "utility.h"
#include "Serializable.h"

namespace QB {
	using namespace serialization;
	
	class BinaryReader{
		typedef std::map<std::string, std::ifstream *> str2ifs;
		typedef str2ifs::iterator mapIter;
		
		// To be used with Priority Queue
		struct MsgTimeCompare{
			bool operator()(  serialization::Msg * left,  serialization::Msg * right)			
			{
				return left->getSequenceNo() > right->getSequenceNo();
			}
		};
		
	public:
		BinaryReader(int argc, char *argv[])
		: stopped(false), ofs("output.csv")
		{
			for (int i=1; i < argc; i++)
			{ 
				std::ifstream * ifs = new std::ifstream(argv[i]);  // Create input file handlers
				if (!ifs->is_open())
				{
					PRINT("Can't open " << argv[i]);
				}
				else
				{
					ifsMap[std::string(argv[i], strchr(argv[i], FILENAMEDELIM) - argv[i])]	= ifs;
				}
			}			
			
		}
		
		~BinaryReader()
		{
			for (mapIter iter = ifsMap.begin(); iter != ifsMap.end(); ++iter)
			{ 
				iter->second->close();        // Free up resources
				delete iter->second;
			}		
		}
		void insertIntoMinHeap(const std::string& symbol, const std::string& line)
		{
			Msg * tPkt;
			switch (line[0])
			{
				case 'S' :
					tPkt = new SignalMsg(symbol, line); break;
				case 'Q' :
					tPkt = new QuoteMsg(symbol, line); break;
				case 'T' :
					tPkt = new TradeMsg(symbol, line); break;
				default:
					PRINT("Invalid Pkt Type " << line[0]); return;
			}
			pq.push(tPkt);		
		}
		
		void execute()
		{
			
			auto start = std::chrono::system_clock::now();
			
			for (mapIter iter = ifsMap.begin(); iter != ifsMap.end(); ++iter)
			{ 
				std::string line;
				if ( !iter->second->eof())
				{
					std::getline(*(iter->second), line, ENDDELIM);
					if (!line.empty())
						insertIntoMinHeap(iter->first, line);
				}
			}		
			
			while (!pq.empty())
			{
				Msg * tempPkt  = pq.top();
				const std::string symbol = tempPkt->getSymbol();
				{
					pkt2process = tempPkt;
					deserilzeAndWrite();  // Remove this in MT world
				}
				pq.pop();
				delete tempPkt;
				
				if ( !ifsMap[symbol]->eof())
				{
					std::string line;
					std::getline(*(ifsMap[symbol]), line, ENDDELIM);
					if (!line.empty())
						insertIntoMinHeap(symbol, line);
					PRINT(line);
				}
			}
			
			auto end = std::chrono::system_clock::now();
			std::cout << "BinaryReader time taken (microseconds) : " 
					  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() <<std::endl;		
		}
		
		void deserilzeAndWrite()
		{
			
			if ( !stopped)
			{
				//PRINT(pkt2process->getSymbol());
				dynamic_cast<Serializable *>(pkt2process)->deserialize(ofs);
			}	
		}
		
		void stop()
		{
			stopped = true;
		}
	
	private:
		std::ofstream ofs ; /// CSV file to be created
		str2ifs ifsMap;
		bool stopped;
		
		std::priority_queue<serialization::Msg *, std::vector<serialization::Msg *> , MsgTimeCompare> pq;
		Msg * pkt2process;
	};
	
	
} // end of namespace QB