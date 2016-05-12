#include "SocketServer.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string>

using namespace std;

SocketServer::SocketServer(int port, string serverName) : 
	port(port),
	serverName(serverName),
	mutex(Semaphore(0x100, 1)),
	db(Semaphore(0x200, 1)),
	tourniquet(Semaphore(0x300, 1)),
	readersId(-1)
{
	Init();
}

SocketServer::~SocketServer()
{
	close(this->serverSocket);

	if(this->readersId != -1)
		shmctl(this->readersId, IPC_RMID, NULL);

	printf("%s na portu %d ukoncen.\n", this->serverName.c_str(), this->port);
}

void SocketServer::Init()
{
	// vytvoreni socketu
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(this->serverSocket == -1)
	{
		printf("Nemuzu vytvorit socket.\n");
		exit(1);
	}

	in_addr addr_any = { INADDR_ANY };
	sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons( port );
	srv_addr.sin_addr = addr_any;

	// socket smi znovu okamzite pouzit jiz drive pouzite cislo portu
	int opt = 1;
	if (setsockopt( this->serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) < 0)
		printf("Nelze nastavit vlastnosti socketu.\n");

	// prirazeni adresy a portu socketu
	if (bind( this->serverSocket, (const sockaddr * ) &srv_addr, sizeof( srv_addr ) ) < 0)
	{
		printf("Prirazeni adresy selhalo.\n");
		exit(1);
	}

	// aplikace bude naslouchat na zadanem portu
	if ( listen( this->serverSocket, 1 ) < 0 )
	{
		printf("Nelze naslouchat na pozadovanem portu.\n");
		exit(1);
	}

	// pripraveni sdilene pameti
	if( (this->readersId = shmget(0x400, sizeof(int), 0600)) < 0)
	{
		if( (this->readersId = shmget(0x400, sizeof(int), 0600 | IPC_CREAT)) < 0)
		{
			printf("Nelze vytvorit sdilenou pamet.\n");
			exit(1);
		}
	}
	readers = (int*)shmat(this->readersId, 0, 0);
	*readers = 0;
}

void SocketServer::Run()
{
	printf("%s posloucha na portu %d.\n", this->serverName.c_str(), this->port);

	char buf[ 100 ];
	sockaddr_in rsa;
	int rsa_size = sizeof( rsa );
	while(1)
	{
		printf("Cekam na klienta ...\n");
		int sock_client = accept(this->serverSocket,(sockaddr*)&rsa, (socklen_t*)&rsa_size);

		if (sock_client == -1)
			printf("Spojeni se nezdarilo.\n");
		else
			ServeNewClient(sock_client);
	}
}
