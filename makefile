all:
	g++ server.cpp -Iinclude -o build/server -lmingw32 -lws2_32 -static-libgcc -static-libstdc++ -lwinpthread -static
	g++ client.cpp -Iinclude -o build/client -lmingw32 -lws2_32 -static-libgcc -static-libstdc++ -lwinpthread -static