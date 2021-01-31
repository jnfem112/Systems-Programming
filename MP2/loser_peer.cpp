#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
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
		peer = "/tmp/mp2-" + peer + ".sock"
		config.peers.push_back(peer);
		config.number_of_peer++;
	}

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_3(line);
	input_3 >> config.repo;

	return;
}

void init_socket(Config &config , int &listen_socket , vector <int> &peer_socket)
{
	struct sockaddr_un address;

	unlink(config.name.c_str());
	listen_socket = socket(AF_UNIX , SOCK_STREAM , 0);
	memset(&address , 0 , sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path , config.name.c_str() , sizeof(address.sun_path) - 1);
	bind(listen_socket , (const struct sockaddr *)&server_address , sizeof(struct sockaddr_un));
	listen(listen_socket , 20);

	for (int i = 0 ; i < config.number_of_peer ; i++)
	{
		peer_socket[i] = socket(AF_UNIX , SOCK_STREAM , 0)
		memset(&address , 0 , sizeof(struct sockaddr_un));
		address.sun_family = AF_UNIX;
		strncpy(address.sun_path , config.peers[i].c_str() , sizeof(address.sun_path) - 1);
		connect(peer_socket[i] , (const struct sockaddr *)&address , sizeof(struct sockaddr_un));
	}

	return;
}

void close_socket(Config &config , int &listen_socket , vector <int> &peer_socket)
{
	close(listen_socket);
	for (int i = 0 ; i < config.number_of_peer ; i++)
		close(peer_socket[i]);
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

bool simple_copy(string source , string destination , Config &config)
{
	stat temp;

	if (source[0] == '@' && stat((config.repo + "/" + source.substr(1)).c_str() , temp) != 0)
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

		while ((byte = fread(buffer , sizeof(char) , BUFFER_SIZE , in_file)) > 0)
			fwrite(buffer , sizeof(char) , byte , out_file);

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

	int listen_socket;
	vector <int> peer_socket;
	init_socket(config , listen_socket , peer_socket);

	int max_fd = sysconf(_SC_OPEN_MAX);
	fd_set read_set , write_set;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_SET(STDIN_FILENO , &read_set);
	FD_SET(socket_in , &read_set);

	string command;
	vector <string> arguments(2);
	int status = 0;
	vector <int> peer_status(config.number_of_peer , 0);
	string source , destination;
	int byte;
	char buffer[BUFFER_SIZE + 1] = {0};

	while (true)
	{
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

		for (int fd = 3 ; fd <= max_fd ; fd++)
		{
			if (FD_ISSET(fd , &working_read_set))
			{
				// TODO
			}

			if (FD_ISSET(fd , &working_write_set))
			{
				// TODO
			}
		}

		if (command == "cp" && status == 1)
		{
			source = arguments[0];
			destination = arguments[1];

			if (destination[0] == '@')
				destination = config.repo + "/" + destination.substr(1);

			if (simple_copy(source , destination , config))
				status = 0;
			else
			{
				// TODO
			}
		}
	}

	close_socket(config , listen_socket , peer_socket);
	return 0;
}