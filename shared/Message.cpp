#include "Message.h"

using namespace std;

bool IsValidMessageType(char c)
{
	c = toupper(c);
    return c == 'C' || c == 'A' || c == 'W' || c == 'E' || c == 'I';
}

Message::Message() : 
	Message(MessageType::NotDefined, -1, "")
{
    
}

Message::Message(MessageType type, int code, std::string text) :
	messageType(type), code(code), text(text)
{

}

void Message::ToString(string & s)
{
    s = static_cast<char>(messageType);
    if(code != -1)
        s+= to_string(code);
    s += ':' + text;
}