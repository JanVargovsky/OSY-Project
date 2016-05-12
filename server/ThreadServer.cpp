#include "ThreadServer.h"

#include <unistd.h>
#include <pthread.h>

#include "../shared/ctespis.h"
#include "../shared/MessageHandler.h"

using namespace std;

struct ThreadStart
{
	int clientSocket;
	ThreadServer * threadServer;
};

ThreadServer::ThreadServer(int port)
	: SocketServer(port, "Vlaknovy server")
{
}

void ThreadServer::ServeNewClient(int clientSocket)
{
	pthread_t thread;
	ThreadStart *ts = new ThreadStart();
	ts->clientSocket = clientSocket;
	ts->threadServer = this;
	if(pthread_create(&thread, NULL, &ThreadServer::ClientListener, ts) != 0)
	{
		printf("Obslouzeni klienta %d selhalo\n", clientSocket);
		delete ts;
	}
	else
		pthread_detach(thread);
}

void *ThreadServer::ClientListener(void * threadStart)
{
	ThreadStart *ts = (ThreadStart*)threadStart;
	int clientSocket = ts->clientSocket;
	ThreadServer * self = ts->threadServer;
	delete (ThreadStart*)threadStart;

	
	MessageHandler messageHandler(clientSocket);
	Message message;
	Message messageServerError(MessageType::Error, EI_Server, ES_Server);

	printf("Klient obslouzen tid=%lu\n", pthread_self());

	// 1. ==> Chci cist (11:Chci cist)
	messageHandler.ReadNext(message);
	if(message.code == CI_PrichaziC && message.text == CS_PrichaziC)
	{
		self->tourniquet.Down();
		self->tourniquet.Up();

		self->mutex.Down();
		if(++*(self->readers) == 1)
			self->db.Down(); // 2. cekani na uvolneni knihovny
		self->mutex.Up();

		// 4. <== Muzes cist (21:Muzes cist)
		message = Message(MessageType::Answer, AI_Cti, AS_Cti);
		messageHandler.Send(message);

		// 5. ==> Chci cist od X do Y (13:Cti od X do Y)
		messageHandler.ReadNextUntil(message, MessageType::Command, CI_Cist);

		// Delay here

		// 6. <== Data (23:Data)
		message = Message(MessageType::Answer, AI_Data, AS_Data); 
		messageHandler.Send(message);

		// 7, 8 dole

		self->mutex.Down();
		if(--*(self->readers) == 0)
			self->db.Up();
		self->mutex.Up();
	}
	// 9. ==> Chci psat (12:Chci psat)
	else if(message.code == CI_PrichaziS)
	{
		self->tourniquet.Down();

		self->db.Down(); // 10. cekani na uvolneni knihovny

		// 12. <== Muzes psat (22:Muzes psat)
		message = Message(MessageType::Answer, AI_Pis, AS_Pis);
		messageHandler.Send(message);

		// 13. ==> Zapis data na X (14:Zapis data na X)
		messageHandler.ReadNextUntil(message, MessageType::Command, CI_Psat);

		// Delay here

		// 14. <== Hotovo (24:Hotovo)
		message = Message(MessageType::Answer, AI_Zapsano, AS_Zapsano);
		messageHandler.Send(message);

		// 15,16 dole

		self->db.Up();

		self->tourniquet.Up();
	}
	else
	{
		messageHandler.Send(messageServerError);
		close(clientSocket);
		printf("Klient konci s chybou tid=%lu\n", pthread_self());
		return (void*)1;
	}

	// 7 nebo 15 ... ==> Konec (15:Konec)
	messageHandler.ReadNextUntil(message, MessageType::Command, CI_Konec);

	// 8 nebo 16 ... <== Nashledanou priste (25:Na shledanou)
	message = Message(MessageType::Answer, AI_Nashledanou, AS_Nashledanou); 
	messageHandler.Send(message);

	printf("Klient konci tid=%lu\n", pthread_self());
	close(clientSocket);
	return (void*)0;
}