#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <vector>

using namespace std;

struct Client
{
	int id;
	int soket;
	thread transfer_thread;
};

vector<Client> vec;

int SearchClient(int name)
{
	for (int i = 0; i < vec.size(); i++)
		if (vec[i].id == name) return i;
	return -1;
}

int FindClient(int name)
{
	int i = 0;
	for (i = 0; i < vec.size(); i++)
		if (vec[i].id == name) break;
	return i;
}

string ClientList()
{
	string list;
	for (int i = 0; i < vec.size(); i++)
		list += "\nКлиент №" + to_string(i+1) + ": " + to_string(vec[i].id);
	return list;
}

string ClientsNames(int id)
{
	if (!vec.size() || vec.size() == 1) return "-1";

	string list;
	for (int i = 0; i < vec.size(); i++)
		if (vec[i].id != id) list += to_string(vec[i].id) + " ";

	return list;
}

void Transfer(int id)
{
	char IN[100000], OUT[100000];
	int id1 = SearchClient(id), id2;
	bool stop = true;

	while(stop)
	{
		memset(IN, 0, sizeof(IN)); memset(OUT, 0, sizeof(OUT));
		
		if (read(vec[id1].soket, IN, sizeof(IN)) < 0) cout << "Не удалось прочитать данные!\n";
		else id1 = SearchClient(id);
		
		string str(IN);
		
		if (str == "all")
		{
			strcpy(OUT, ClientList().c_str());
			write(vec[id1].soket, OUT, strlen(OUT));
		}
		else if (str == "#")
		{
			stop = false;
			int id = vec[id1].id;

			cout<<"Отсоединение от "<<vec[id1].id<<endl;

			vec[id1].transfer_thread.detach();
			vec.erase(vec.begin() + id1);

			for(int i = 0; i < vec.size(); i++)
			{
				string warn = "#" + to_string(id);
				strcpy(OUT, warn.c_str());
				write(vec[i].soket, OUT, sizeof(OUT));
			}
		}
		else if (str[0])
		{
			id2 = stoi(str.substr(0, str.find(' ')));
			str = str.substr(str.find(' ')+1);

			id2 = SearchClient(id2);

			if (id2 == -1) 
			{
				strcpy(OUT, "Такого клиента нет!");
				write(vec[id1].soket, OUT, strlen(OUT));
				continue;
			}
			else strcpy(OUT, str.c_str());
			write(vec[id2].soket, OUT, strlen(OUT));
		}
		str = "";	
	}
}

int main(int argc, char *argv[])
{
	int soket, newsoket, port = atoi(argv[1]);
	struct sockaddr_in server, client;
	if (argc < 2)
	{
		cout << "Неверное число аргументов!\n";
		exit(1);
	}
		
	soket = socket(AF_INET, SOCK_STREAM, 0);
	if (soket < 1) cout<<"Ошибка открытия сокета!\n";
	
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(soket, (struct sockaddr*) &server, sizeof(server)) < 0) cout << "Ошибка привязки имени к сокету!\n";
	else cout << "Сокет сервера создан и связан с именем " << port << "\n";
	listen(soket, 5);
	
	socklen_t clientsize = sizeof(client);
	
	while(1) // Add new client
	{
		char name[255];
		memset(name, 0, 255);
		newsoket = accept(soket, (struct sockaddr*) &client, &clientsize);
		
		if (newsoket < 0) cout<<"Не удалось создать новое соединение!\n";
		if (read(newsoket, name, 255) < 0) cout<<"Не удалось прочитать данные!\n";
		int id = atoi(name);
		
		int check = SearchClient(id);

		switch(check)
		{
			case -1:
			{
				vec.resize(vec.size()+1);
				int pos = vec.size()-1;

				vec[pos].id = id;
				vec[pos].soket = newsoket;
				vec[pos].transfer_thread = thread(Transfer, id);

				cout << "Сервер соединен с клиентом " << id << endl;

				char OUT[100000];
				strcpy(OUT, ClientsNames(id).c_str());
				write(vec[pos].soket, OUT, sizeof(OUT));

				break;
			}
			default:
			{
				if (write(newsoket, "Имя занято", 20) < 0) cout << "Не удалось записать данные!\n";
				continue;
			}
		}
	}
	
	close(soket);

	for(int i = 0; i < vec.size(); i++) 
	{
		close(vec[i].soket);
		vec[i].transfer_thread.detach();
	}

	return 0;
}


