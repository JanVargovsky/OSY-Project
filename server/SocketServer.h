#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <string>

#include "../shared/Semaphore.h"

class SocketServer
{
    public:
        ~SocketServer();

        virtual void Run();
    protected:
    	SocketServer(int port, std::string serverName);

    	const int port;
    	const std::string serverName;

    	int serverSocket;
    	int *readers, readersId;
    	Semaphore mutex, db, tourniquet;

    	virtual void ServeNewClient(int clientSocket) = 0;
    private:
    	void Init();
};

#endif // SOCKETSERVER_H
