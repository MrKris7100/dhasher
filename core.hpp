#ifndef __EASYSOCK_CORE
#define __EASYSOCK_CORE

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	#define __EASYSOCK_WINDOWS
	#include <winsock2.h>
	typedef int socklen_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif

#include <map>

namespace easysock {
	#ifdef __EASYSOCK_WINDOWS
		WSADATA _initData;
		typedef SOCKET socket_t;
	#else
		typedef int socket_t;
	#endif
	typedef std::map<std::string, std::string> strmap;

	int lastErrorPlace;
	int lastError;

	bool init() {
		lastErrorPlace = 0;
		lastError = 0;

		#ifdef __EASYSOCK_WINDOWS
			if(_initData.wVersion == 0 && _initData.wHighVersion == 0) {
				int status = WSAStartup(MAKEWORD(2,2), &_initData);
				if(status != 0) {
					lastError = status;
					return false;
				}
			}
		#endif

		return true;
	}

	void exit() {
		#ifdef __EASYSOCK_WINDOWS
			_initData.wVersion = 0;
			_initData.wHighVersion = 0;
			WSACleanup();
		#endif
	}

	std::string toString(easysock::strmap map, std::string delim, std::string append, std::string prepend = "") {
		std::string out = "";
		for(auto const &elem : map) {
			out += prepend + elem.first + delim + elem.second + append;
		}
		return out;
	}
}

#endif
