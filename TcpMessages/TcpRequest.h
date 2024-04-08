#pragma once

#include "../types.h"
#include <vector>
using namespace std;

/**
 * @brief TCP request builder/helper class.
 */
class TcpRequest
{
public:
	/**
	 * Constructor parsing a request buffer into a TcpRequest object
	 * @param buffer The buffer containing the request
	 */
	TcpRequest(char* buffer);

	String commandName;
	vector<String> commandArgs;
	String id;

private:
	/**
	 * Parses the request buffer into a vector of arguments (words space separated)
	 * @param buffer The buffer containing the request
	 * @return The vector of arguments
	 */
	vector<String> getArgs(char* buffer);
};
