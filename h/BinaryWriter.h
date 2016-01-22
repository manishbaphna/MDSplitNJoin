#ifndef _BINARY_WRITER_H_
#define _BINARY_WRITER_H_

#include "Config.h"
#include "RoutingStrategy.h"


#include <string>
#include <sstream>
#include <map>
#include <thread>
#include <mutex>
#include <memory>

#include <condition_variable>

#include "utility.h"
#include "Serializable.h"

namespace QB{

	/* 
	 * @brief Multithreaded module to write incoming data from CSV into 
	 * symbol specific biinary files ( like IBM.bin). All incoming data
	 * are dividede over existing threads using RoutingStrategy.
	 */
	 
	template <typename RoutingStrategy = SimpleHash>
	class BinaryWriter {
		typedef std::map<std::string, std::ofstream* > str2ofs;
		
	public:
		BinaryWriter(TimeKeeper *tKeeper, const std::string& tFileName)
		: stopped(false)
		, keeper(tKeeper)
		, filename(tFileName)
		, sequenceNo(0)
		{
			data.resize(MAX_THREADS);
			currSymbol.resize(MAX_THREADS);
			ready2proc.resize(MAX_THREADS, false);
			consumerDone.resize(MAX_THREADS, true);
			seqNo.resize(MAX_THREADS, 0);
		}
		
		~BinaryWriter()
		{
			str2ofs::iterator iter = ofsMap.begin();
			for (;iter != ofsMap.end(); iter++)
			{
				iter->second->close();
				delete iter->second;
			}
		}
		
		
		void producer()
		{
			std::ifstream ifs(filename, std::ifstream::binary);
			std::string line, symbol;
			
			if (!ifs.is_open())
			{
				throw QBException("Error : can't open input file");
			}

			while (true)
			{
				std::getline(ifs, line);
				
				if (ifs.eof()) break;
				PRINT(line);
				
				// Find the symbol for Routing
				size_t pos = line.find(',');
				std::string symbol = line.substr(pos+1, line.find(',', pos+1) - pos - 1);
			
				int k =  RoutingStrategy::getId(symbol);
				{
					std::unique_lock<std::mutex> ulr(mlockr);
					while (consumerDone[k] == false)
						condr.wait(ulr);
					consumerDone[k] = false;
					ulr.unlock();
					
					std::lock_guard<std::mutex> l(mlock[k]);
					data[k] = line;
					currSymbol[k] = symbol;
					ready2proc[k] = true;
					//consumerDone[k]=false;
					seqNo[k] = sequenceNo++;
				}
				cond[k].notify_one();
				line = "";
			}
			
			ifs.close();
			stop();
		}
		
		void stop()
		{
			stopped = true;
			for (int k=0; k<MAX_THREADS; k++)
			{
				{
					std::lock_guard<std::mutex> l(mlock[k]);
					ready2proc[k] = true;
				}
				cond[k].notify_all();	
			}
		}
		
		void binaryWriteToFile(const std::string& sym, const std::string& line, unsigned long long int seqNo)
		{
			//PRINT("BinaryWriter : " << line);
			str2ofs::iterator fs = ofsMap.find(sym);
			if (fs != ofsMap.end())
			{
				serialization::Serializable * tPtr = serialization::getSerializableObject(line, seqNo);
				if (tPtr){
					tPtr->serialize(*(fs->second));
					delete tPtr;
				}
			}
			else
			{
				std::string filename = sym + ".bin";
				std::ofstream * fh = new std::ofstream(filename, std::ofstream::binary);
				//ofsMap.insert(std::make_pair<std::string, std::ofstream &>(sym, fh));
				if ( !fh || !*fh)
				{
					PRINT(" Error opening file " << filename);
					throw QBException("Error opening file");
				}
				ofsMap.insert(std::make_pair(sym, fh));
				serialization::Serializable * tPtr = serialization::getSerializableObject(line, seqNo);
				if (tPtr){
					tPtr->serialize(*fh);
					delete tPtr;
				}
//				PRINT("<---BW ** " << *fh << DELIM << line);		
			}
			
		}
		
		void consumer(int i)
		{
			auto startTime = std::chrono::system_clock::now();
			auto endTime = std::chrono::system_clock::now();
			
//			PRINT("Starting thread #" << i);
			while(!stopped)
			{
				std::unique_lock<std::mutex> ul(mlock[i]);
				while( !ready2proc[i] )
					cond[i].wait(ul); 
				
				if ( data[i].empty()) continue;
				
				binaryWriteToFile(currSymbol[i], data[i], seqNo[i]);
					
				ready2proc[i] = false;
				{
					std::lock_guard<std::mutex> lgr(mlockr);
					consumerDone[i]=true;
					condr.notify_one();
				}
				
				data[i] = "";
				ul.unlock();	
				
				endTime = std::chrono::system_clock::now();
			}
			
			std::stringstream text; 
			text << "Thread" << i;
			unsigned long long int timetaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
			keeper->push(text.str(), timetaken);
		}
		

		void execute(){
			for (int count = 0 ; count < MAX_THREADS ; count++){
				threads.push_back(std::thread(&BinaryWriter::consumer, this, count));
			}
			producer();
		}
		
		void join(){
			for (int count = 0 ; count < MAX_THREADS ; count++)
			{
				threads[count].join();
			}		
		}
		

	private:
		std::mutex mlock[MAX_THREADS];
		std::condition_variable cond[MAX_THREADS];
		std::vector<std::string> data;
		std::vector<std::string> currSymbol;
		std::vector<bool>  ready2proc;
		std::vector<bool> consumerDone;
		std::vector<std::thread> threads;
		std::vector<unsigned long long int> seqNo;
		
		// Reverse synchronization from Consumer to Producer
		std::mutex mlockr;
		std::condition_variable condr;
		
		
		str2ofs ofsMap;
		bool stopped;
		TimeKeeper * keeper;
		std::string filename;
		unsigned long long int sequenceNo;
			
	};
}
#endif
