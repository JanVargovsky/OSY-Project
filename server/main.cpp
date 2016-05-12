#include <stdio.h>
#include <signal.h>
#include <string>

#include "SocketServer.h"
#include "ForkServer.h"
#include "ThreadServer.h"

using namespace std;

int DEBUG = 1;

void cleanInit();
void clean();

SocketServer * server;

enum ServerType
{
	Invalid = -1,
	Thread  = 1,
	Process = 2,
	NotSelected = 0,
};

void printHelp()
{
	printf("-p ... procesovy server (implicitni hodnota)\n");
	printf("-t ... vlaknovy server\n");
	printf("--help ... zobrazi tuhle napovedu\n");
}

int main(int n, char *args[])
{
	// 1 ... port
	// 2 ... process/thread
	if(n < 2)
	{
		printf("Ocekavam port serveru a [-p nebo -t] pro volbu typu serveru\n");
		printHelp();
		return 1;
	}

	string tmp = args[1];
	if(tmp == "-h" || tmp == "--help")
	{
		printHelp();
		return 0;
	}

	int port = stoi(tmp);
	ServerType serverType = ServerType::NotSelected;

	if(n == 3)
	{
		tmp = args[2];
		if(tmp == "p" || tmp == "-p")
			serverType = ServerType::Process;
		else if(tmp == "-t" || tmp == "-t")
			serverType = ServerType::Thread;
		else
			serverType = ServerType::Invalid;
	}

	if(serverType == ServerType::Invalid)
	{
		printf("Spatny typ serveru.\n");
		return 1;
	}
	else if(serverType == ServerType::Process || serverType == ServerType::NotSelected)
		server = new ForkServer(port);
	else if(serverType == ServerType::Thread)
		server = new ThreadServer(port);

	//cleanInit();
	server->Run();

	clean();
	return 0;
}

void cleanInit()
{
	signal( SIGINT, [](int code){ exit( 1 ); });
  	signal( SIGPIPE, [](int code){ exit( 1 ); });

  	if(atexit(clean))
	{
		printf("Clean init selhal\n");
		exit(1);	
	}
}

void clean()
{
	delete server;
}
