#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include "loser.h"
using namespace std;

int main(int argc , char **argv)
{
	if (strcmp(argv[1] , "status") == 0)
	{
		chdir(argv[2]);
		status();
	}

	if (strcmp(argv[1] , "commit") == 0)
	{
		chdir(argv[2]);
		commit();
	}

	if (strcmp(argv[1] , "log") == 0)
	{
		chdir(argv[3]);
		log(atoi(argv[2]));
	}

	return 0;
}
