#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
using namespace std;

#define BUFFER_SIZE 1024

struct Config
{
	string name;
	int number_of_peer;
	vector <string> peers;
	string repo;
};

struct Commit
{
	uint32_t number_of_commit , number_of_file , number_of_add , number_of_modify , number_of_copy , number_of_delete , commit_size;
	vector <string> new_file , modified , deleted;
	vector <pair <string , string> > copied;
	unordered_map <string , string> file_list;
	// timestamp
};

typedef struct Config Config;
typedef struct Commit Commit;

void read_config(string file_name , Config &config)
{
	ifstream config_file(file_name);
	string line;

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_1(line);
	input_1 >> config.name;
	config.name = "/tmp/mp2-" + config.name + ".sock";

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_2(line);
	string peer;
	config.number_of_peer = 0;
	while (input_2 >> peer)
	{
		peer = "/tmp/mp2-" + peer + ".sock";
		config.peers.push_back(peer);
		config.number_of_peer++;
	}

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_3(line);
	input_3 >> config.repo;

	return;
}

void init_socket(Config &config , int &server_socket , vector <int> &peer_socket)
{
	struct sockaddr_un address;

	unlink(config.name.c_str());
	server_socket = socket(AF_UNIX , SOCK_STREAM , 0);
	memset(&address , 0 , sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path , config.name.c_str() , sizeof(address.sun_path) - 1);
	bind(server_socket , (const struct sockaddr *)&address , sizeof(struct sockaddr_un));
	listen(server_socket , 20);

	for (int i = 0 ; i < config.number_of_peer ; i++)
		peer_socket[i] = socket(AF_UNIX , SOCK_STREAM , 0);

	return;
}

int connect_socket(Config &config , vector <int> &peer_socket , int index)
{
	struct sockaddr_un address;
	memset(&address , 0 , sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path , config.peers[index].c_str() , sizeof(address.sun_path) - 1);
	return connect(peer_socket[index] , (const struct sockaddr *)&address , sizeof(struct sockaddr_un));
}

void close_socket(Config &config , int &server_socket , vector <int> &peer_socket)
{
	close(server_socket);
	unlink(config.name.c_str());

	for (int i = 0 ; i < config.number_of_peer ; i++)
	{
		close(peer_socket[i]);
		unlink(config.peers[i].c_str());
	}

	return;
}

void read_command(string &command , vector <string> &arguments)
{
	string line;
	getline(cin , line);
	istringstream input(line);
	input >> command;
	arguments.clear();
	string argument;
	while (input >> argument)
		arguments.push_back(argument);
	return;
}

bool check_file_exist(string file_name , Config &config)
{
	struct stat temp;
	if (file_name[0] == '@')
		file_name = config.repo + "/" + file_name.substr(1);
	return stat(file_name.c_str() , &temp) == 0;
}

bool simple_copy(string source , string destination , Config &config)
{
	if (source[0] == '@' && !check_file_exist(source , config))
		return false;
	else
	{
		if (source[0] == '@')
			source = config.repo + "/" + source.substr(1);

		FILE *in_file = fopen(source.c_str() , "rb");

		if (!in_file)
		{
			cout << "fail\n";
			return true;
		}

		FILE *out_file = fopen(destination.c_str() , "wb");
		char buffer_1[BUFFER_SIZE + 1] = {0};
		int byte;

		while ((byte = fread(buffer_1 , sizeof(char) , BUFFER_SIZE , in_file)) > 0)
			fwrite(buffer_1 , sizeof(char) , byte , out_file);

		cout << "success\n";

		fclose(in_file);
		fclose(out_file);
	}

	return true;
}

int main(int argc , char **argv)
{
	Config config;
	read_config(argv[1] , config);

	string command;
	vector <string> arguments(2);
	string source , destination;
	int status = 0;
	vector <int> peer_status(config.number_of_peer , 0);
	int byte;
	char buffer_1[BUFFER_SIZE + 1] = {0} , buffer_2[BUFFER_SIZE + 1] = {0};
	int target;
	FILE *in_file , *out_file;
	bool finish = false;

	int server_socket;
	vector <int> peer_socket(config.number_of_peer) , client_socket;
	vector <bool> connected(config.number_of_peer , false);
	init_socket(config , server_socket , peer_socket);

	int max_fd = sysconf(_SC_OPEN_MAX);
	fd_set read_set , write_set;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_SET(STDIN_FILENO , &read_set);
	FD_SET(server_socket , &read_set);

	while (!finish)
	{
		for (int i = 0 ; i < config.number_of_peer ; i++)
			if (!connected[i] && connect_socket(config , peer_socket , i) != -1)
			{
				connected[i] = true;
				FD_SET(peer_socket[i] , &read_set);
				FD_SET(peer_socket[i] , &write_set);
			}

		fd_set working_read_set , working_write_set;
		memcpy(&working_read_set , &read_set , sizeof(fd_set));
		memcpy(&working_write_set , &write_set , sizeof(fd_set));

		if (select(max_fd , &working_read_set , &working_write_set , NULL , NULL) == 0)
			continue;

		if (FD_ISSET(STDIN_FILENO , &working_read_set))
		{
			read_command(command , arguments);
			status = 1;
		}

		if (FD_ISSET(server_socket , &working_read_set))
		{
			int fd = accept(server_socket , NULL , NULL);
			client_socket.push_back(fd);
			FD_SET(fd , &read_set);
			FD_SET(fd , &write_set);
		}

		for (int i = 0 ; i < config.number_of_peer ; i++)
		{
			if (FD_ISSET(peer_socket[i] , &working_read_set))
			{
				recv(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
				if (strcmp(buffer_1 , "success") == 0)
				{
					if (command == "cp" || command == "mv")
					{
						status = 3;
						peer_status[i] = 2;
						target = peer_socket[i];
						out_file = fopen(destination.c_str() , "wb");
					}
					else if (command == "rm")
					{
						cout << "success\n";
						status = 0;
					}
				}
				else if (strcmp(buffer_1 , "fail") == 0)
				{
					peer_status[i] = 2;
				}
				else if (peer_socket[i] == target)
				{
					byte = atoi(buffer_1);

					if (byte == 0)
					{
						fclose(out_file);
						cout << "success\n";
						status = 0;
					}
					else
					{
						recv(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						fwrite(buffer_1 , sizeof(char) , byte , out_file);
					}
				}
			}

			if (FD_ISSET(peer_socket[i] , &working_write_set))
			{
				if ((command == "cp" || command == "mv" || command == "rm") && status == 2 && peer_status[i] == 0)
				{
					strcpy(buffer_1 , command.c_str());
					send(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					if (source[0] == '@')
						source = source.substr(1);
					strcpy(buffer_1 , source.c_str());
					send(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					peer_status[i] = 1;
				}
				else if (command == "exit" && status == 2 && peer_status[i] == 0)
				{
					strcpy(buffer_1 , command.c_str());
					send(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					peer_status[i] = 1;
				}
			}
		}

		int number_of_client = client_socket.size();
		for (int i = 0 ; i < number_of_client ; i++)
		{
			if (FD_ISSET(client_socket[i] , &working_read_set))
			{
				recv(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
				if (strcmp(buffer_1 , "cp") == 0 || strcmp(buffer_1 , "mv") == 0 || strcmp(buffer_1 , "rm") == 0)
				{
					command = buffer_1;
					recv(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					source = config.repo + "/" + buffer_1;
					status = 4;
					target = client_socket[i];
				}
				else if (strcmp(buffer_1 , "exit") == 0)
				{
					FD_CLR(client_socket[i] , &read_set);
					FD_CLR(client_socket[i] , &write_set);
					close(client_socket[i]);
					client_socket.erase(client_socket.begin() + i);
				}
			}

			if (FD_ISSET(client_socket[i] , &working_write_set))
			{
				if ((command == "cp" || command == "mv") && status == 4 && client_socket[i] == target)
				{
					if (!check_file_exist(source , config))
					{
						cout << "not exist : " << source << "\n";
						strcpy(buffer_1 , "fail");
						send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						status = 0;
					}
					else
					{
						cout << "exist : " << source << "\n";
						strcpy(buffer_1 , "success");
						send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						status = 5;
						in_file = fopen(source.c_str() , "rb");
					}
				}
				else if ((command == "cp" || command == "mv") && status == 5 && client_socket[i] == target)
				{
					byte = fread(buffer_2 , sizeof(char) , BUFFER_SIZE , in_file);
					sprintf(buffer_1 , "%d" , byte);
					send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					if (byte > 0)
						send(client_socket[i] , buffer_2 , BUFFER_SIZE , 0);
					else
					{
						fclose(in_file);
						if (command == "mv")
							remove(source.c_str());
						status = 0;
					}
				}
				else if (command == "rm" && status == 4 && client_socket[i] == target)
				{
					if (!check_file_exist(source , config))
					{
						strcpy(buffer_1 , "fail");
					}
					else
					{
						remove(source.c_str());
						strcpy(buffer_1 , "success");
					}

					send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					status = 0;
				}
			}
		}

		if ((command == "cp" || command == "mv") && status == 1)
		{
			source = arguments[0];
			destination = arguments[1];

			if (destination[0] == '@')
				destination = config.repo + "/" + destination.substr(1);

			if (simple_copy(source , destination , config))
			{
				if (command == "mv")
				{
					if (source[0] == '@')
						source = config.repo + "/" + source.substr(1);
					remove(source.c_str());
				}

				status = 0;
			}
			else
			{
				status = 2;
				for (int i = 0 ; i < config.number_of_peer ; i++)
					peer_status[i] = 0;
			}
		}
		else if (command == "rm" && status == 1)
		{
			source = arguments[0];
			source = source.substr(1);

			if (check_file_exist(source , config))
			{
				remove(source.c_str());
				status = 0;
			}
			else
			{
				status = 2;
				for (int i = 0 ; i < config.number_of_peer ; i++)
					peer_status[i] = 0;
			}
		}
		else if (command == "exit" && status == 1)
		{
			status = 2;
			for (int i = 0 ; i < config.number_of_peer ; i++)
				peer_status[i] = 0;
		}
		else if (command == "exit" && status == 2)
		{
			finish = true;
			for (int i = 0 ; i < config.number_of_peer && finish ; i++)
				if (peer_status[i] == 0)
					finish = false;
		}
	}

	close_socket(config , server_socket , peer_socket);
	cout << "bye\n";
	return 0;
}