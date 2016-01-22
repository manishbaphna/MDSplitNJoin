#include <iostream>

#include "Config.h"
#include "BinaryReader.h"

using namespace QB;

int mymain(int argc, char *argv[])
{
	// Start all threads in Binwary Writer
	BinaryReader reader(argc, argv);
	reader.execute();
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "pgm <BinaryFiles ... >" ;
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