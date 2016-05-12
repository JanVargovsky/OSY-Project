#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

enum class MessageType : char
{
    Command = 'C',
    Answer = 'A',
    Warning = 'W',
    Error = 'E', 
    Information = 'I',
    NotDefined,
};

bool IsValidMessageType(char c);

class Message
{
    public:
        Message();
        Message(MessageType type, int code, std::string text);

    	MessageType messageType;
    	int code;
    	std::string text;
        
        void ToString(std::string & s);

};

#endif // MESSAGEREADER_H

