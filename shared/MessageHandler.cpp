#include "MessageHandler.h"

#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <fcntl.h>

#include "Message.h"

using namespace std;

extern int DEBUG;

MessageHandler::MessageHandler(int fd) :fd(fd)
{
	isOpen = fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

MessageHandler::~MessageHandler()
{
	close(fd);
}

bool MessageHandler::ReadNextUntil(Message & message, MessageType expectedType, int expectedCode)
{
	while(isOpen)
	{
		if(ReadNext(message))
		{
			if(message.messageType == expectedType && message.code == expectedCode)
				return true;
		}
	}

	return false;
}

bool MessageHandler::ReadNext(Message & m)
{
	if(!isOpen)
		return false;

	size_t index = index = find(buffer.begin(), buffer.end(), '\n') - buffer.begin();

	if(index == buffer.size())
	{
		while(1)
		{
			// closed fd
			if(!ReadToBuffer() && !isOpen)
				return false;

			index = find(buffer.begin(), buffer.end(), '\n') - buffer.begin();
			// finally found '\n'
			if(index != buffer.size())
				break;
		}
	}
	
	// found
	// NOTE: ignores \n
	string message = string(buffer.begin(), buffer.begin() + index);

	if(DEBUG) printf("DEBUG: %s\n", message.c_str());
	
	// remove it from the buffer
	buffer.erase(buffer.begin(), buffer.begin() + index + 1);
	
	// has to contain "?:\n" - 3 chars, but \n is not part of the text
	if(message.size() < 2 || message.size() > MAX_LINE_LENGTH)
	    return false;
	
	index = 0;
	// ?
	if(!IsValidMessageType(message[index]))
	    return false;
	m.messageType = static_cast<MessageType>(message[index++]);
	
	// [NN]
	if(isdigit(message[index]) && isdigit(message[index + 1]))
	{
	    m.code = 10 * (message[index] - '0') + message[index+1] - '0';
	    index += 2;
	}
	
	// ':'
	if(message[index++] != ':')
	    return false;
	
	// [text]
	m.text = message.substr(index);

	m.ToString(message);
	printf("Prijato: %s\n", message.c_str());
	
	return true;
}

bool MessageHandler::Send(Message & message)
{
	string str;
	message.ToString(str);
	str += '\n';

	int l = write(fd, str.c_str(), str.size());

	printf("Poslano: %s", str.c_str());

	return l > 0;
}

bool MessageHandler::ReadToBuffer()
{
	char buf[MAX_LINE_LENGTH];
	int size = read(fd, buf, MAX_LINE_LENGTH);
	
	// check if its equal to 0 -> fd is closed
	if(size == 0)
	{
		isOpen = false;
	    return 0;
	}

	if(size < 0)
	{
		// TODO: check it from errno
		isOpen = false;
		return 0;
	}
	
	buf[size] = '\0';
	if(DEBUG) printf("NOVA DATA BUFFER (%d): %s\n", size, buf);
	string line = buf;
	buffer.insert(buffer.end(), line.begin(), line.end());

	return 1;
}