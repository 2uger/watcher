run:
	g++ -c ipc.cpp -o ipc.o
	g++ transmitter.cpp ipc.o -o transmitter
	g++ receiver.cpp ipc.o -o receiver
	g++ watcher.cpp -o watcher && ./watcher
