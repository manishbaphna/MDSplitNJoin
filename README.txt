Design : 
-------

Whole project consists of two components. 
BinaryWriter ( which reads CSV and dumps BINs per symbol )
BinaryReader ( Takes all BINs as input and created original CSV)
I have removed Parser to simplify design and added sequence number to preserve ordering
i.e. messages having same timestamp would be in exact same order as of original CSV.
As of now Number of threads are configured to be 4. They can be changed compile time
via Config.h or gcc parameter (DMAX_THREADS)

Features Supported:
1. Parsing of CSVs into symbol specific binary files + Serialization
2. Order is persistes and preserved ( via Sequence no )
3. Multithreads component to handle processing and disk writing
4. Serialization : 
	4.1 Time is converted into epoc ( supported milisecond ) 
	4.2 Symbols are omitted
	4.3 Double prices : Passed as string to preserve precision
	4.4 Optional field 'condition' for trade is taken care 
5. Deserialization
6. Time Logging


Important Components/files:
[ SERIALIZER ]
1. utility.h : utilities concerning epoc/string manipulation, spinLock etc
2. RoutingStrategy.h : BinaryWrite is multithreaded components where set of instruments are processed in a thread.
   RoutingStrategy helps determine which instrument should go to which thread. A Sample implementation of it SimpleHash
   is used. There could be many more implmenetation like weight based or dynamic load storage etc.
3. Serializable.h : Defined serializable object interface.    
5. BinaryWriter.h : Converts text into binaries. It's got N consumer threads who will get lines to process from 
Producer function. Producer function simply reads file and routes them to appropriate thread for processing.
Each thread would process the data, serialize it and write into BIN file.

[ DESERIALIZER ]
6. BinaryReader.h : Converts N BIN files into "output.csv" . Algorithm it works is based on MinHeap ( PriorityQueue).
We start by reading all inputs files ( 1 line each ), store all lines into MinHeap based on Sequence Number. 
Then, take out one with the minimum sequence number, process the pkt and deserialize it, finally writing it into CSV file.
Now, take next line from the file, whose pkt was found in last step and redo this. This way we keep iterating till all
files are finished. 

Issues :
I found one nasty race condition issue with BinaryWriter but some how it seems more like windows issue.
Unfortunately I don't have linux neither any debugging tool to do advanced debug.
[ I even tried wait_for ( instead of wait ) but it was still stuck there, which it shouldn't ] 
It worked pretty 


Build :
>>make all OR make all-dbg

RUN :
>> ./serpgm <CSVInput>  
>> ./despgm <Name of BINs, each as cmd line (sorr!) >

Example :
$ ./serpgm.exe input.csv
$ ./despgm.exe UBM3.bin ZBM3.bin

Sample TestCase:
----------------
$ cd test
$ source test.sh
g++ -o serpgm src/serializeMain.cpp -std=c++11 -Ih/
g++ -o despgm src/deserializeMain.cpp -std=c++11 -Ih/
Time taken by BinaryWriter threads
Thread0,14875000
Thread1,14625000
Thread2,14734375
Thread3,14703125
BinaryReader time taken (microseconds) : 62500


ENV:
Developed on cygwin: Using C++11
gcc version 4.9.2 (GCC)

