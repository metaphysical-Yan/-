#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")


/*
int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed" << std::endl;
		return 1;
	}
	
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed" << std::endl;
		WSACleanup();
		return 1;
	}
	
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed" << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	
	std::string message;
	char buffer[1024];
	while (true) {
		std::cout << "Enter message: ";
		std::getline(std::cin, message);
		
		send(clientSocket, message.c_str(), message.size(), 0);
		
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			std::cout << "Received from server: " << buffer << std::endl;
		}
	}
	
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
*/

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed" << std::endl;
		return 1;
	}
	
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed" << std::endl;
		WSACleanup();
		return 1;
	}
	
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed" << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	
	std::string message;
	char buffer[1024];
	while (true) {
		std::cout << "Enter message (type 'exit' to quit): ";
		std::getline(std::cin, message);
		
		if (message == "exit") {
			break;
		}
		
		// Send message length (4 bytes) first
		int messageLength = message.size();
		int networkLength = htonl(messageLength);
		send(clientSocket, reinterpret_cast<char*>(&networkLength), sizeof(networkLength), 0);
		
		// Send the actual message
		send(clientSocket, message.c_str(), messageLength, 0);
		
		// Receive the echo from the server
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';  // Null-terminate the received data
			std::cout << "Received from client: " << buffer << std::endl;
		}
	}
	
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}

