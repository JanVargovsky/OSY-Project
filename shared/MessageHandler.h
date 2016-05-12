#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <vector>

#include "Message.h"

class MessageHandler
{
    public:
        MessageHandler(int fd);
        ~MessageHandler();

        bool Send(Message & message);
        bool ReadNext(Message & message);
        bool ReadNextUntil(Message & message, MessageType expectedType, int expectedCode);
    private:
        const int fd;
        const unsigned int MAX_LINE_LENGTH = 256;
        std::vector<char> buffer;
        bool isOpen;

        bool ReadToBuffer();

};

#endif // MESSAGEHANDLER_H
