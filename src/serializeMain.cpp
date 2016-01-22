#include "Config.h"
#include "BinaryWriter.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <cstring>

using namespace QB;

int mymain(int argc, char *argv[])
{

	utils::TimeKeeper keeper;
	
	// Start all threads in Binwary Writer
	BinaryWriter<SimpleHash> writer(&keeper, argv[1]);
	writer.execute();
	writer.join();
	std::cout << "Time taken by BinaryWriter threads (microseconds) " << std::endl;
	keeper.print();
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "pgm CSVFile" ;
		exit(1);
	}
	try {
		mymain(argc, argv);
	}catch(utils::QBException e)
	{
		std::cout << e.what() << std::endl;
		exit(1);
	}
}