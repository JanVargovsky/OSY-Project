#include "ForkServer.h"

#include <unistd.h>

#include "../shared/ctespis.h"
#include "../shared/MessageHandler.h"

using namespace std;

ForkServer::ForkServer(int port)
	: SocketServer(port, "Processovy server")
{
}

void ForkServer::ServeNewClient(int clientSocket)
{
	pid_t pid;
	if((pid = fork()) < 0)
	{
		printf("Obslouzeni klienta %d selhalo\n", clientSocket);
		return;
	}
	else if(pid == 0)
	{
		MessageHandler messageHandler(clientSocket);
		Message message;
		Message messageServerError(MessageType::Error, EI_Server, ES_Server);

		printf("Klient obslouzen pid=%d\n", getpid());

		// 1. ==> Chci cist (11:Chci cist/12:Chci psat)
		messageHandler.ReadNext(message);
		if(message.code == CI_PrichaziC && message.text == CS_PrichaziC)
		{
			tourniquet.Down();
			tourniquet.Up();

			mutex.Down();
			if(++*readers == 1)
				db.Down(); // 2. cekani na uvolneni knihovny
			mutex.Up();

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

			mutex.Down();
			if(--*readers == 0)
				db.Up();
			mutex.Up();
		}
		// 9. ==> Chci psat (12:Chci psat)
		else if(message.code == CI_PrichaziS)
		{
			tourniquet.Down();

			db.Down(); // 10. cekani na uvolneni knihovny

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

			db.Up();

			tourniquet.Up();
		}
		else
		{
			messageHandler.Send(messageServerError);
			close(clientSocket);
			printf("Klient konci s chybou pid=%d\n", getpid());
			_exit(1);
		}

		// 7 nebo 15 ... ==> Konec (15:Konec)
		messageHandler.ReadNextUntil(message, MessageType::Command, CI_Konec);

		// 8 nebo 16 ... <== Nashledanou priste (25:Na shledanou)
		message = Message(MessageType::Answer, AI_Nashledanou, AS_Nashledanou); 
		messageHandler.Send(message);

		printf("Klient konci pid=%d\n", getpid());
		close(clientSocket);
		close(this->serverSocket);
		_exit(0);
	}
}
