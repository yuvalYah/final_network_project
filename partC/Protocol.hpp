/*This class creat a message protocol*/
#pragma once
#include <iostream>
#include <cstring>
#include <string>
using namespace std;

static int setID = 1;
namespace Net{
    
class Protocol{
    private:
        int _msgID; 
        int _sourcID;
        int _destID;
        int _trailingMsg;
        int _funcID;
        char _payload[492];
        // char *_payload;//[492];


    public:
        //constructors
        Protocol(int sourcID, int DestID, int trailing,  int funcID,const char* payload): 
            _msgID(setID++), _sourcID(sourcID), _destID(DestID), _funcID(funcID), _trailingMsg(trailing)
            {
                memset(&_payload, 0, sizeof(_payload));
                strcpy(_payload, payload);
        }
        //constructor from a string to a message protocol
        Protocol(std::string protocol){
            int ind =protocol.find(",");
            _msgID = std::stoi (protocol.substr(0,ind));

            std::string newstring = protocol.substr(ind+1); ind =newstring.find(",");
            _sourcID =std::stoi (newstring.substr(0, ind));

            newstring = newstring.substr(ind+1); ind =newstring.find(",");
            _destID=std::stoi (newstring.substr(0, ind));

            newstring = newstring.substr(ind+1); ind =newstring.find(",");
            _trailingMsg =std::stoi (newstring.substr(0, ind));

            newstring = newstring.substr(ind+1); ind =newstring.find(",");
            _funcID =std::stoi (newstring.substr(0, ind));

            std::string payload = newstring.substr(ind+1);
            strcpy(_payload ,&payload[0]);
        }
        //method to convert string to char*
        operator const char *() 
        { 
            
            const char* str;
            string paytemp(_payload);
            string str2 = to_string(_msgID) + "," + to_string(_sourcID) + ',' +to_string(_destID) + 
                ',' + to_string(_trailingMsg) + ',' + to_string(_funcID) + ',';
            
            str2+=paytemp;

            str = str2.c_str();
            
            return str; 
        } 
        //get methods
        int get_msgID() {return _msgID;}
        int get_srcID() {return _sourcID;}
        int get_destID() {return _destID;}
        int get_trailing() {return _trailingMsg;}
        int get_funcID() {return _funcID;}
        std::string get_payload() { return _payload;}

};
}