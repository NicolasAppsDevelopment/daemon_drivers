#include "TcpRequest.h"
#include "../drivererror.h"

TcpRequest::TcpRequest(char* buffer)
{
	vector<String> args = getArgs(buffer);
	if (args.size() <= 1)
	{
		throw DriverError("Invalid request");
	}
	this->id = args[0];
	this->commandName = args[1];
	this->commandArgs = vector<String>(args.begin() + 2, args.end());
}

vector<String> TcpRequest::getArgs(char* buffer) {
    char separator = ' ';
    String str(buffer);
    str.erase(str.find_last_not_of(" \n\r\t") + 1);
    vector<String> args;

    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= (int)str.size(); i++) {

        // If we reached the end of the word or the end of the input.
        if (str[i] == separator || i == (int)str.size()) {
            endIndex = i;
            String temp;
            temp.append(str, startIndex, endIndex - startIndex);
            args.push_back(temp);
            startIndex = endIndex + 1;
        }
    }

    return args;
}