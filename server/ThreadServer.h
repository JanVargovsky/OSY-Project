#ifndef THREADSERVER_H
#define THREADSERVER_H

#include "SocketServer.h"

class ThreadServer : public SocketServer
{
    public:
    	ThreadServer(int port);

    	virtual void ServeNewClient(int clientSocket);
    private:
    	static void* ClientListener(void * threadStart);
};

#endif // THREADSERVER_H
