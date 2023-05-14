#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <arpa/inet.h>
#include <vector>
#include <ctime>
#include <gmpxx.h>

using namespace std;

int prime[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, \
47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, \
127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, \
197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, \
277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, \
367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, \
449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, \
547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, \
631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, \
727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, \
823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, \
919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997 };

struct Client
{
	int id;
	long open[2];
	long close[2];
};

vector<Client> vec;

char IN[100000], OUT[100000], *id_name;

bool stop = true;
int soket;

long* CreateKeys()
{
	int p = prime[rand() % 168], q = prime[rand() % 168];
	long n = p * q, m = (p - 1) * (q - 1), d = m - 1, e = 10;
	
	for (long i = 2; i <= m; i++)
	{
		if ((m % i == 0) && (d % i == 0))
		{
		    d++;
		    i = 1;
		}
	}
	
	while (true)
	{
		if ((e * d) % m == 1) break;
		else e++;
	}
	
	static long arg[3] = {0,0,0};
	arg[0] = e; arg[1] = d; arg[2] = n;
	return arg;
}

string RSA_encode(string str, int e, int n)
{
	string result = "";

	for(int i = 0; i < str.size(); i++)
	{
		mpz_class res;
		mpz_powm(res.get_mpz_t(), mpz_class((int)str[i]).get_mpz_t(), mpz_class(e).get_mpz_t(), mpz_class(n).get_mpz_t());

		unsigned long int num = mpz_get_ui(res.get_mpz_t());
		result += to_string(num) + " ";
	}

	return result;
}

string RSA_decode(string str, int d, int n)
{
	string result = "";
	int spaces = count(str.begin(), str.end(), ' ');
	
	for(int i = 0; i < spaces; i++)
	{
		int num = stoi(str.substr(0, str.find(' ')));
		str = str.substr(str.find(' ') + 1);

		mpz_class res;
		mpz_powm(res.get_mpz_t(), mpz_class(num).get_mpz_t(), mpz_class(d).get_mpz_t(), mpz_class(n).get_mpz_t());

		unsigned long int number = mpz_get_ui(res.get_mpz_t());
		result += (char)number;
	}

	return result;
}

void ReadMsg()
{
	while(1)
	{
		if (read(soket, IN, sizeof(IN)) < 0) cout<<"Не удалось прочитать сообщение!\n";

		string message(IN);

		if (message[0] == '!')
		{
			bool flag = true;

			string ans = IN;
			ans = ans.substr(7);
			int name = stoi(string(ans.substr(0,ans.find(' '))));
			ans = ans.substr(ans.find(' ')+1);

			for(int i=0;i<vec.size();i++)
			{
				if (vec[i].id == name)
				{
					flag = 0;
					break;
				}
			}

			if (flag)
			{
				long *arg = CreateKeys();
				Client cl;

				long e = stoi(ans.substr(0,ans.find(' ')));
				ans = ans.substr(ans.find(' ')+1);
				long n = stoi(ans);

				cl.id = name;
				cl.open[0] = e;
				cl.open[1] = n;
				cl.close[0] = arg[1];
				cl.close[1] = arg[2];
				vec.push_back(cl);

				string msg = to_string(cl.id) + " !data! " + string(id_name) + " " + to_string(arg[0]) + " " + to_string(arg[2]);
				strcpy(OUT, msg.c_str());
				write(soket, OUT, sizeof(OUT));
				memset(IN, 0, sizeof(IN)); memset(OUT, 0, sizeof(OUT));
			}			
		}
		else if (message[0] == '#' && isdigit(message[1]))
		{
			int id = stoi(message.substr(1));	
			for(auto it = vec.begin(); it!=vec.end(); it++)
			{
				if (it->id == id)
				{
					vec.erase(it);
					break;
				}
			}
		}	
		else if (isdigit(message[0]))
		{
			string msg(IN);
			int id = stoi(msg.substr(0, msg.find(' ')));
			string str = msg.substr(msg.find(' ') + 1);

			cout << "Получено сообщение от " << id << ": " << str << endl;

			long d, n;
			for(int i = 0;i < vec.size(); i++)
			{
				if (vec[i].id == id)
				{
					d = vec[i].close[0];
					n = vec[i].close[1];
					break;
				}
			}
			str = RSA_decode(str, d, n);
			cout << "Сообщение расшифровано: " << str << endl;
			memset(IN, 0, sizeof(IN));
		}
		else if (message.find("\nКлиент")==0) cout << "Список активных клиентов:" << message << endl;
		else if (!message.find("Т") || isalpha(message[0])) cout << "Получено сообщение: " << message << endl;
		memset(IN, 0, sizeof(IN));
	}
}

int main(int argc, char *argv[]) // 1 - port 2 - client's name
{
	id_name = argv[2];
	int port = atoi(argv[1]);
	struct sockaddr_in server;
	struct hostent *serv;
	
	if (argc != 3)
	{
		cout<<"Неверное число аргументов!\n";
		exit(1);
	}
	
	soket = socket(AF_INET, SOCK_STREAM, 0);
	if (soket < 0)
	{
		cout<<"Ошибка создания сокета!\n";
		exit(1);
	}
	else cout<<"Сокет клиента создан\n";
	
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	serv = gethostbyname(argv[1]);//1
	if (!serv)
	{
		cout<<"Нет такого хоста!\n";
		exit(1);
	}
	cout<<"Соединение с сервером с номером порта "<<port<<": ";
	
	if (connect(soket, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		cout<<"Не получилось соединиться с сервером!\n";
		exit(1);
	}
	else cout<<"Соединились с сервером!\n";
		
	if (write(soket, argv[2], sizeof(argv[3])) < 0)//3
	{
		cout<<"Ошибка передачи имени клиента серверу!\n";
		exit(1);
	}
	else cout<<"Имя передано серверу!\n";
	
	cout<<"Соединение установлено!\nЧтобы разорвать соединение, введите #.\nЧтобы узнать список клиентов, введите all.\n";
	
	string ClientsList = "";
	while(!ClientsList.size())
	{	
		read(soket, IN, sizeof(IN));
		ClientsList = IN;	
	}

	if (ClientsList == "Имя занято") 
	{
		cout << ClientsList << "!" << endl;
		close(soket);
		return 0;
	}

	srand(time(0));

	while(ClientsList.find(' ') != string::npos)
	{
		long *arg = CreateKeys();
		Client cl;
		cl.id = stoi(ClientsList.substr(0, ClientsList.find(' ')));

		string msg = to_string(cl.id) + " !data! " + string(argv[2]) + " " + to_string(arg[0]) + " " + to_string(arg[2]);
		ClientsList = ClientsList.substr(ClientsList.find(' ')+1);
		cl.close[0] = arg[1];
		cl.close[1] = arg[2];

		strcpy(OUT, msg.c_str());
		write(soket, OUT, sizeof(OUT));

		string answer = "";
		while(!answer.size())
		{
			read(soket, IN, sizeof(IN));
			answer = IN;
		}

		memset(IN, 0, sizeof(IN)); memset(OUT, 0, sizeof(OUT));

		answer = answer.substr(7);
		answer = answer.substr(answer.find(' ')+1);

		long e = stoi(answer.substr(0,answer.find(' ')));
		answer = answer.substr(answer.find(' ')+1);

		long n = stoi(answer);
		cl.open[0] = e;
		cl.open[1] = n;
		vec.push_back(cl);
	}
	
	thread read_thread(ReadMsg);
		
	while(stop)
	{
		string msg;
		getline(cin, msg);

		if (msg == "#") 
		{
			stop = false;
			vec.clear();
			vec.shrink_to_fit();
		}
		else if (msg != "all")
		{
			int id = stoi(msg.substr(0, msg.find(' ')));
			string str = msg.substr(msg.find(' ') + 1);

			long e = -1, n = -1;

			for(int i = 0; i < vec.size(); i++)
			{
				if (vec[i].id == id)
				{
					e = vec[i].open[0];
					n = vec[i].open[1];
					break;
				}
			}
			if (e != -1)
			{
				str = RSA_encode(str, e, n);
				msg = to_string(id) + " " + string(argv[2]) + " " + str;
				cout<<"Сообщение закодировано: "<<str<<endl;
			}
		}
		strcpy(OUT, msg.c_str());
		
		if(write(soket, OUT, sizeof(OUT)) < 0) cout<<"Не удалось отправить сообщение!\n";
	}
	
	cout << "Соединение с сервером разорвано!\n";
	read_thread.detach();
	close(soket);
	
	return 0;
}

