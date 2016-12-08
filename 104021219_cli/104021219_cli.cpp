#include <winsock2.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_SIZE 1024
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);

int main(int argc, char** argv) {
	// port number
	if (argc != 3) {
		MY_ERROR("Error: Invaild argument (104021219_cli.exe <IPv4> <Port>).\n");
	}
	char* argEnd;
	const char* serverIP = argv[1];
	// TODO: check valid IPv4
	const int serverPort = strtol(argv[2], &argEnd, 10);
	if (serverPort < 0 || serverPort > 65535) {
		MY_ERROR("Error: Invaild Port range.\n");
	}
	// dir
	_mkdir("upload");
	_mkdir("download");
	// create socket
	SOCKET serverSocket;
	struct sockaddr_in serverAddress;
	int bytesRead;
	char buf[MAX_SIZE];
	// Load Winsock v2.2
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), (LPWSADATA)&wsadata) != 0) {
		MY_ERROR("Winsock Error\n");
	}
	// bind to socket
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(serverIP);
	serverAddress.sin_port = htons(serverPort);
	// listening
	while (gets(buf)) {
		// validate command
		int bufSize = strlen(buf);
		if (bufSize >= MAX_SIZE) {
			printf("Invalid: Command is too long.");
			continue;
		}
		// connection
		serverSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
			MY_ERROR("Connect Error\n");
		}
		// command
		if (bufSize >= 5 && buf[0] == 'p' && buf[1] == 'u' && buf[2] == 't') {
			_chdir("upload");
			char filename[MAX_SIZE];
			memcpy(filename, &buf[4], bufSize - 3);
			FILE* fptr = fopen(filename, "rb");
			if (fptr == NULL) printf("Invaild: %s is not found!", filename);
			else {
				send(serverSocket, buf, strlen(buf) + 1, 0);
				while (1) {
					memset(buf, 0, MAX_SIZE);
					bufSize = fread(buf, 1, MAX_SIZE, fptr);
					if (bufSize > 0) {
						send(serverSocket, buf, bufSize, 0);
						printf("Send %d bytes.\n", bufSize);
					}
					else break;
				}
				printf("%s is sent.\n", filename);
			}
			_chdir("..");
		} else if (bufSize == 3 && buf[0] == 'd' && buf[1] == 'i' && buf[2] == 'r') {
			send(serverSocket, "dir", 3, 0);
			// TODO: read size
			bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
			buf[bytesRead] = '\0';
			printf("%s\n", buf);
		}
		closesocket(serverSocket);
		memset(buf, 0, MAX_SIZE);
	}
	return 0;
}