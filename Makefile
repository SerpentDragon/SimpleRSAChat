first:
	g++ -pthread client.cpp -o client -lgmp
	g++ -pthread server.cpp -o server
