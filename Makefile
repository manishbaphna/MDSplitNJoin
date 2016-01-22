all: 
	g++ -o serpgm src/serializeMain.cpp -std=c++11 -Ih/ 
	g++ -o despgm src/deserializeMain.cpp -std=c++11 -Ih/ 

all-dbg: 
	g++ -o serpgm src/serializeMain.cpp -std=c++11 -Ih/ -DDEBUG -g3 -O0
	g++ -o despgm src/deserializeMain.cpp -std=c++11 -Ih/ -DDEBUG -g3 -O0

ser:
	g++ -o serpgm src/serializeMain.cpp -std=c++11 -Ih/ -DDEBUG -g3 -O0
	
des:
	g++ -o despgm src/deserializeMain.cpp -std=c++11 -Ih/ -DDEBUG -g3 -O0

clean:
	\rm *.exe *.o *.bin
