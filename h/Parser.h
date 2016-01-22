#ifndef _PARSER_H_
#define _PARSER_H_

#include "Config.h"
#include "BinaryWriter.h"

namespace QB {
	template <typename RoutingStrategy>
	class BinaryWriter;

	/* 
	 * @brief Parse input CSV file and send each line to BinaryWriter for processing
	 * This is single threaded module 
	 */

	class Parser {
	public:
		Parser(TimeKeeper *tKeeper, const std::string& tFileName)
		: keeper(tKeeper)
		, filename(tFileName)
		{}

		template <typename RoutingStrategy>
		int parserMain(BinaryWriter<RoutingStrategy> & writer)
		{
			std::clock_t startTime = clock();
			std::ifstream ifs(filename, std::ifstream::binary);
			std::string line, symbol;
			
			if (!ifs.is_open())
			{
				throw QBException("Error : can't open input file");
			}

			while (!ifs.eof())
			{
				std::getline(ifs, line);
				writer.producer(line);
				PRINT("In Parser " << line) ;
			}
			ifs.close();
			
			writer.stop();
			std::clock_t endTime = clock();
			double timetaken = (endTime - startTime) / (double)CLOCKS_PER_SEC ;

			keeper->push("Parser", timetaken);
		}
		
	private:
		TimeKeeper *keeper;
		std::string filename;
	};
	
}	
#endif