#ifndef FORKSERVER_H
#define FORKSERVER_H

#include "SocketServer.h"

class ForkServer : public SocketServer
{
    public:
    	ForkServer(int port);

    	virtual void ServeNewClient(int clientSocket);
};

#endif // FORKSERVER_H
