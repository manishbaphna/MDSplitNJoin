	
#ifndef _SERIALIZATION_H_
#define _SERIALIZATION_H_

#include <vector>
#include <string>
#include <cstring>

/*
The file contains quotes, trades and signal records defined as below
-  quote record - time, symbol, bid, ask, bsize, asize   (datetime, string, double, double, int, int)
-  trade record -  time, symbol, price, condition            (datetime, string, double, char) - condition could be empty. 
-  signal record - time, symbol, value, code                  (datetime, string, double, int)
*/

using namespace QB::utils;

namespace QB {
	
	namespace serialization {
		
	typedef std::vector<std::string> VecStr;

		struct Serializable 
		{
			virtual void serialize(std::ofstream& os) = 0 ;	
			virtual void deserialize(std::ofstream& os) = 0 ;	
			
			virtual ~Serializable(){}
		};

		
		class Msg {
		public:
			Msg(char msgType, unsigned long long int seqNo=0, const std::string& tSymbol = "")
			: msgtype(msgType), symbol(tSymbol), time(0), sequenceNo(seqNo){}
			
		virtual ~Msg(){} 
			
		unsigned long long int getSequenceNo() const { return sequenceNo ;}	
		const std::string getSymbol() const { return symbol;}
		
		void serialize(std::ofstream& os)
		{
			os << msgtype << DELIM << sequenceNo << DELIM << time << DELIM;
		}
		 
		void deserialize(std::ofstream& os)
		{
			os << epochms2str(time) << DELIM << symbol << DELIM;
		}
		
		protected:
			std::string symbol;
			char msgtype;
			//size_t totalSize;
			unsigned long long int time;
			unsigned long long int sequenceNo;   // To ensure order of pkts (and also help debug pkt loss)
		};

		//-  signal record - time, symbol, value, code                  (datetime, string, double, int)
		class SignalMsg : public Serializable, public Msg
		{
		public:
		
		    // This would be used while deserializing
			SignalMsg(const std::string& symbol , const std::string& x)
			: Msg('S', 0, symbol), validcode(false)
			{
				vecStr v;
				splitString(x, DELIM, v);
				char * pEnd;
				int index = 0;
				sequenceNo = strtoull(v[++index].c_str(), &pEnd, 10);
				time = strtoull(v[++index].c_str(), &pEnd, 10);
				value = v[++index];
				code = atoi(v[++index].c_str()); 
			}
			
			// This would be used while serializing
			SignalMsg(VecStr& v, unsigned long long int seqNo)
			: Msg('S', seqNo), validcode(false)
			{
				time = str2epochms(v[0]);
				value = v[2];
				code = (atoi)(v[3].c_str());
				sequenceNo = seqNo;
			}
			
			void serialize(std::ofstream& os)
			{
				Msg::serialize(os);
				os << value << DELIM << code << ENDDELIM;
			}
			
			void deserialize(std::ofstream& os)
			{
				Msg::deserialize(os);
				os << value << DELIM << code << std::endl;		
			}
		private:
			size_t valueLen;
			std::string value;
			int code;
			bool validcode;
		};

		// Trade record time, symbol, price, condition : (datetime, string, double, char) - condition could be empty. 
		class TradeMsg : public Serializable, public Msg
		{
		public:
			// This would be ysed while deserializing
			TradeMsg(const std::string& symbol , const std::string& x)
			: Msg('T', 0, symbol), condition(' ')
			{
				vecStr v;
				splitString(x, DELIM, v);
				char * pEnd;
				int index = 0 ;
				sequenceNo = strtoull(v[++index].c_str(), &pEnd, 10);
				time = strtoull(v[++index].c_str(), &pEnd, 10);
				price= v[++index];
				if (v.size() == 5)
				{
					validCondition = true;
					condition = v[++index][0];				
				}
			}
		
			// This would be used while serializing
			TradeMsg(VecStr& v,unsigned long long int seqNo)
			: Msg('T', seqNo), validCondition(false)
			{
				time = str2epochms(v[0]);
				price = v[2];
				if (v.size() == 4 && v[3] != "\r")
				{
					validCondition = true;
					condition = v[3][0];
				}
				sequenceNo = seqNo;
				//totalSize = sizeof(msgtype)+sizeof(time)+sizeof(DELIM)+price.size()+sizeof(DELIM)+sizeof(condition);
			}
			
			void deserialize(std::ofstream& os)
			{
				Msg::deserialize(os);
				os << price << DELIM;
				if (validCondition)
					os << condition;
				os << std::endl;
			}
			
			void serialize(std::ofstream& os)
			{
				Msg::serialize(os);
				os << price ;
				if (validCondition)
					os << DELIM << condition;
				os << ENDDELIM;
			}
		private:
			size_t priceLen;
			std::string price;
			char condition;
			bool validCondition;
		};


		class QuoteMsg : public Serializable, public Msg
		{
		public:
		    // This would be used while deserializing
			QuoteMsg(const std::string& symbol , const std::string& x) 
			: Msg('Q', 0, symbol) 
			{
				vecStr v;
				splitString(x, DELIM, v);
				char * pEnd;
				int index =0 ;
				sequenceNo = strtoull(v[++index].c_str(), &pEnd, 10);
				time = strtoull(v[++index].c_str(), &pEnd, 10);
				bid = v[++index];
				ask = v[++index];
				bsize= atoi(v[++index].c_str());
				asize= atoi(v[++index].c_str());
			}
			
			QuoteMsg(VecStr& v, unsigned long long int seqNo)
			: Msg('Q', seqNo)
			{
				time = str2epochms(v[0]);
				bid = v[2];
				ask = v[3];
				bsize = (atoi)(v[4].c_str());
				asize = (atoi)(v[5].c_str());
				//totalSize = sizeof(msgtype)+sizeof(time)+sizeof(DELIM)+bid.size()+sizeof(DELIM)
				//				+ ask.size() + sizeof(DELIM) + sizeof(int)+sizeof(DELIM)+sizeof(int)+sizeof(DELIM);
				sequenceNo = seqNo;

			}
						
			void deserialize(std::ofstream& os)
			{
				Msg::deserialize(os);
				os << bid << DELIM << ask << DELIM << bsize << DELIM << asize << std::endl;
			}
			
			void serialize(std::ofstream& os)
			{
				Msg::serialize(os);
				os << bid << DELIM << ask << DELIM << bsize << DELIM << asize << ENDDELIM ;
			}
		private:
			std::string bid;
			int bsize;
			std::string ask;
			int asize;
		};

					   
		Serializable * getSerializableObject (std::string line, unsigned long long int seqNo)
		{
			VecStr token;
			splitString(line, ',', token);
			
			if( token.size() == 6 )
				return new QuoteMsg(token, seqNo);
			else if( token.size() == 3 )
				return new TradeMsg(token, seqNo);
			else if( token.size() == 4 )
			{
				//if ( token[3].size() == 1 && std::toupper(token[3][0]) >= 'A') 
				if (token[3].empty() || token[3] == "\r"  || (std::toupper(token[3][0]) >= 'A' && std::toupper(token[3][0]) <= 'Z')) 
					return new TradeMsg(token, seqNo);
				else 
					return new SignalMsg(token, seqNo);
			}
			else
			{
				PRINT("Unsupported file structure, please check CSV input " << line << ":" << token.size());
				return 0x0;
			}
		}

	}
}
#endif
