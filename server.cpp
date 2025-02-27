#include <winsock2.h>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <functional>

#pragma comment(lib, "ws2_32.lib")

class ThreadPool {
public:
	ThreadPool(size_t numThreads) : stop(false) {
		for (size_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this] {
				while (true) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queueMutex);
						this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty()) {
							return;
						}
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}
					task();
				}
			});
		}
	}
	
	template<class F>
	void enqueue(F&& f) {
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			tasks.emplace(std::forward<F>(f));
		}
		condition.notify_one();
	}
	
	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers) {
			worker.join();
		}
	}
	
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	std::atomic<bool> stop;
};

/*
void handleClient(SOCKET clientSocket) {
	char buffer[1024];
	int bytesRead;
	
	while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
		buffer[bytesRead] = '\0';
		std::cout << "Received: " << buffer << std::endl;
		send(clientSocket, buffer, bytesRead, 0);  // Echo back
	}
	
	closesocket(clientSocket);
}
*/

void handleClient(SOCKET clientSocket) {
	while (1) {
		char lengthBuffer[4];
		int bytesRead = recv(clientSocket, lengthBuffer, sizeof(lengthBuffer), 0);
		if (bytesRead <= 0) {
			closesocket(clientSocket);
			return;
		}
		
		int messageLength = ntohl(*reinterpret_cast<int*>(lengthBuffer));
		char* messageBuffer = new char[messageLength + 1];
		bytesRead = recv(clientSocket, messageBuffer, messageLength, 0);
		if (bytesRead <= 0) {
			delete[] messageBuffer;
			closesocket(clientSocket);
			return;
		}
		
		messageBuffer[messageLength] = '\0';
		std::cout << "Received: " << messageBuffer << std::endl;
		send(clientSocket, messageBuffer, messageLength, 0);  // Echo back
		
		delete[] messageBuffer;
	}

	closesocket(clientSocket);
}


int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed" << std::endl;
		return 1;
	}
	
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed" << std::endl;
		WSACleanup();
		return 1;
	}
	
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(8080);
	
	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Binding failed" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen failed" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	
	ThreadPool pool(4);  // Thread pool with 4 threads
	
	std::cout << "Server is listening on port 8080..." << std::endl;
	
	while (true) {
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed" << std::endl;
			continue;
		}
		pool.enqueue([clientSocket] { handleClient(clientSocket); });
	}
	
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}

