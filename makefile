all:
	g++ server.cpp -Iinclude -o build/server -lmingw32 -lws2_32
	g++ client.cpp -Iinclude -o build/client -lmingw32 -lws2_32