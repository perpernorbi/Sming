/*
 * telnetServer.cpp
 *
 *  Created on: 18 apr. 2015
 *      Author: Herman
 */

#include "TelnetServer.h"
#include "TcpServer.h"

TelnetServer::TelnetServer() : TcpServer()
{
	// TODO Auto-generated constructor stub
}

TelnetServer::~TelnetServer()
{
	// TODO Auto-generated destructor stub
}

void TelnetServer::enableDebug(bool reqStatus)
{
	telnetDebug = reqStatus;
	if(telnetDebug && curClient) /* only setSetDebug when already connected */
	{
		Debug.setDebug(DebugPrintCharDelegate(&TelnetServer::wrchar, this));
	} else {
		Debug.setDebug(Serial);
	}
}

void TelnetServer::enableCommand(bool reqStatus)
{
#if ENABLE_CMD_EXECUTOR
	if(reqStatus && curClient && !commandExecutor) {
		commandExecutor = new CommandExecutor(curClient);
	}
	if(!reqStatus && commandExecutor) {
		delete commandExecutor;
		commandExecutor = nullptr;
	}
#endif
	telnetCommand = reqStatus;
}
void TelnetServer::onClient(TcpClient* client)
{
	debug_d("TelnetServer onClient %s", client->getRemoteIp().toString().c_str());

	TcpServer::onClient(client);

	if(curClient) {
		debug_d("TCP Client already connected");
		client->sendString("Telnet Client already connected\r\n");
		client->close();
	} else {
		curClient = client;
		curClient->setTimeOut(USHRT_MAX);
		curClient->sendString("Welcome to Sming / ESP6266 Telnet\r\n");
		if(telnetCommand) {
#if ENABLE_CMD_EXECUTOR
			commandExecutor = new CommandExecutor(client);
#endif
		}
		if(telnetDebug) {
			Debug.setDebug(DebugPrintCharDelegate(&TelnetServer::wrchar, this));
		}
		Debug.printf("This is debug after telnet start\r\n");
	}
}

void TelnetServer::onClientComplete(TcpClient& client, bool succesfull)
{
	if(&client == curClient) {
#if ENABLE_CMD_EXECUTOR
		delete commandExecutor;
		commandExecutor = nullptr;
#endif
		curClient = nullptr;
		debug_d("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str());
	} else {
		debug_d("Telnet server unconnected client close");
	}

	debug_d("TelnetServer onClientComplete %s", client.getRemoteIp().toString().c_str());
	TcpServer::onClientComplete(client, succesfull);
	Debug.setDebug(Serial);
}

void TelnetServer::wrchar(char c)
{
	char ca[2];
	ca[0] = c;
	curClient->write(ca, 1);
}

bool TelnetServer::onClientReceive(TcpClient& client, char* data, int size)
{
	debug_d("TelnetServer onClientReceive : %s, %d bytes \r\n", client.getRemoteIp().toString().c_str(), size);
	debug_d("Data : %s", data);
#if ENABLE_CMD_EXECUTOR
	if(commandExecutor) {
		commandExecutor->executorReceive(data, size);
	}
#endif
	return true;
}
