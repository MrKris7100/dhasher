#ifndef __EASYSOCK_TCP
#define __EASYSOCK_TCP

#include <string>
#include "core.hpp"

namespace easysock {
	namespace tcp {
		socket_t createSocket() {
			easysock::lastError = 0;
			easysock::lastErrorPlace = 0;

			socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
			if(sock == INVALID_SOCKET) {
				easysock::lastErrorPlace = 31;
				#ifdef __EASYSOCK_WINDOWS
					easysock::lastError = WSAGetLastError();
				#else
					easysock::lastError = errno;
				#endif
				return INVALID_SOCKET;
			} else {
				return sock;
			}
		}

		std::string getipfromaddr(std::string addr) {
			easysock::lastError = 0;
			easysock::lastErrorPlace = 0;

			struct hostent* host = gethostbyname(addr.c_str());
			if(host == NULL) {
				easysock::lastErrorPlace = 32;
				#ifdef __EASYSOCK_WINDOWS
					easysock::lastError = WSAGetLastError();
				#else
					easysock::lastError = h_errno;
				#endif
			} else {
				struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
				for(int i = 0; addr_list[i] != NULL; i++) {
					// Return the first one IP
					return inet_ntoa(*addr_list[i]);
				}
			}

			return addr;
		}

		class Client {
		private:
			socket_t socket;
		public:
			Client(socket_t socket) {
				this->socket = socket;
			}

			~Client() {
				#ifdef __EASYSOCK_WINDOWS
					shutdown(this->socket, SD_SEND);
					closesocket(this->socket);
				#else
					close(this->socket);
				#endif
			}

			int write(std::string data) {
				easysock::lastError = 0;
				easysock::lastErrorPlace = 0;

				int status = send(this->socket, data.c_str(), data.length(), 0);
				if(status == SOCKET_ERROR) {
					easysock::lastErrorPlace = 12;
					#ifdef __EASYSOCK_WINDOWS
						easysock::lastError = WSAGetLastError();
					#else
						easysock::lastError = errno;
					#endif
					return -1;
				} else {
					return status;
				}
			}

			std::string read(int &status) {
				easysock::lastError = 0;
				easysock::lastErrorPlace = 0;

				std::string buf = std::string(9999, '\0');
				status = recv(this->socket, &buf[0], 9999, 0);
				if(status == SOCKET_ERROR) {
					easysock::lastErrorPlace = 13;
					#ifdef __EASYSOCK_WINDOWS
						easysock::lastError = WSAGetLastError();
					#else
						easysock::lastError = errno;
					#endif
					status = -1;
					return "";
				} else {
					buf.resize(status);
					return buf;
				}
			}

			std::string read() {
				int status;
				return read(status);
			}

			std::string readAll() {
				std::string resp;
				int status;
				do {
					resp += read(status);
					if(status == SOCKET_ERROR) {
						return "";
					}
				} while(status > 0);
				return resp;
			}
		};

		Client* connect(std::string addr, int port) {
			easysock::lastError = 0;
			easysock::lastErrorPlace = 0;

			socket_t sock = createSocket();
			if(sock == INVALID_SOCKET) return nullptr;

			struct sockaddr_in client;
			client.sin_family = AF_INET;
			client.sin_addr.s_addr = inet_addr(getipfromaddr(addr).c_str());
			client.sin_port = htons(port);

			if(connect(sock, (struct sockaddr*)&client, sizeof(client)) == SOCKET_ERROR) {
				easysock::lastErrorPlace = 11;
				#ifdef __EASYSOCK_WINDOWS
					easysock::lastError = WSAGetLastError();
				#else
					easysock::lastError = errno;
				#endif
				return nullptr;
			} else {
				return new Client(sock);
			}
		}

		class Server {
		private:
			socket_t socket;
		public:
			Server(socket_t socket) {
				this->socket = socket;
			}

			~Server() {
				#ifdef __EASYSOCK_WINDOWS
					closesocket(this->socket);
				#else
					close(this->socket);
				#endif
			}

			Client* listen() {
				easysock::lastError = 0;
				easysock::lastErrorPlace = 0;

				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				socket_t sock = accept(this->socket, (struct sockaddr*)&client, &len);
				if(sock == INVALID_SOCKET) {
					easysock::lastErrorPlace = 23;
					#ifdef __EASYSOCK_WINDOWS
						easysock::lastError = WSAGetLastError();
					#else
						easysock::lastError = errno;
					#endif
					return nullptr;
				} else {
					return new Client(sock);
				}
			}
		};

		Server* createServer(int port, std::string addr = "0.0.0.0") {
			easysock::lastError = 0;
			easysock::lastErrorPlace = 0;

			socket_t sock = createSocket();
			if(sock == INVALID_SOCKET) return nullptr;

			struct sockaddr_in server;
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = inet_addr(getipfromaddr(addr).c_str());
			server.sin_port = htons(port);

			if(bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
				easysock::lastErrorPlace = 21;
				#ifdef __EASYSOCK_WINDOWS
					easysock::lastError = WSAGetLastError();
				#else
					easysock::lastError = errno;
				#endif
				return nullptr;
			} else if(listen(sock, 3) == SOCKET_ERROR) {
				easysock::lastErrorPlace = 22;
				#ifdef __EASYSOCK_WINDOWS
					easysock::lastError = WSAGetLastError();
				#else
					easysock::lastError = errno;
				#endif
				return nullptr;
			} else {
				return new Server(sock);
			}
		}
	}
}

#endif
