#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <atomic>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

#ifdef DEBUG
#define PRINT(msg) std::cout << msg << std::endl
#else
#define PRINT(msg) 
#endif

// Below section could be made configurable via config/XML if needed
#define DELIM ','
#define  ENDDELIM '+'
#define FILENAMEDELIM '.'

namespace QB{
	
	namespace utils {
		typedef std::vector<std::string> vecStr;
	
	/* 
	 * @brief utility to convert epoc time to-fro from string ( support millsecods )
	 */
		unsigned long long int str2epochms(const std::string& xstr){
			std::tm t = {};
			std::string str = xstr.substr(0, xstr.find('.'));	
			std::string ms = xstr.substr(xstr.find('.')+1); 

			int y,d,m,hr,min, sec;
			sscanf(str.c_str(),"%d-%d-%d %d:%d:%d", &y,&m,&d,&hr,&min,&sec);
			t.tm_sec = sec;
			t.tm_min = min;
			t.tm_hour= hr;
			t.tm_mon = m -1;
			t.tm_mday = d;
			t.tm_year = y - 1900;

			t.tm_isdst = -1;
			time_t tx  = mktime(&t);

			unsigned long long int x = (tx*1000ull) + ((std::atoi)(ms.c_str()) << (3- ms.size())) ;
			return x;
		}

		std::string epochms2str(unsigned long long int x)
		{
			time_t t = x / 1000;  // convert into seconds first
			struct tm * tm = localtime(&t);
			
			char * ptr = new char[19];
			strftime(ptr, 23 , "%Y-%m-%d %H:%M:%S", tm) ;

			std::stringstream ss;
			ss << std::string(ptr) << '.' << std::setfill('0') << std::setw(3) << x%1000;
			
			return ss.str();
		}	

		//! split string into tokens
		void splitString(const std::string &s, char delim, std::vector<std::string> &elems) {
			std::stringstream ss(s);
			std::string item;
			while (std::getline(ss, item, delim)) {
				elems.push_back(item);
			}
		}


		class TimeKeeper
		{
		public:	
			void push(const std::string& str, unsigned long long int& t)
			{
				timedata.insert(std::make_pair(str, t));
			}
			
			void print()
			{
				std::map<std::string, unsigned long long int>::const_iterator iter = timedata.begin();
				
				for(; iter != timedata.end(); iter++)
				{
					std::cout << iter->first << DELIM << iter->second << std::endl;
				}
			}

		private:
			std::map<std::string, unsigned long long int> timedata;		
		};
		 
		class SpinLock
		{
		public:
			void lock()
			{
				while(lck.test_and_set(std::memory_order_acquire))
				{}
			}
		 
			void unlock()
			{
				lck.clear(std::memory_order_release);
			}
		 
		private:
			std::atomic_flag lck = ATOMIC_FLAG_INIT;
		};

		class QBException: public std::exception
		{
		public:
			QBException(const char* message): exceptionMsg(message) {}
			  
			QBException(const std::string& message): exceptionMsg(message){}

			virtual ~QBException() throw (){}

			virtual const char* what() const throw (){
			   return exceptionMsg.c_str();
			}

		protected:
			std::string exceptionMsg;
		};

	}
}

#endif