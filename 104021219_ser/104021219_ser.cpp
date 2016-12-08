#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// file
#include <dirent.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_SIZE 1024
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);

int main(int argc, char** argv) {
	// port number
	if (argc != 2) {
		MY_ERROR("Error: Port not assigned.\n");
	}
	char* argEnd;
	const int serverPort = strtol(argv[1], &argEnd, 10);
	if (serverPort < 0 || serverPort > 65535) {
		MY_ERROR("Error: Invaild port.\n");
	}
	// upload dir
	_mkdir("upload");
	_chdir("upload");
	// create socket
	SOCKET serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddress;
	int clientAddressLen;
	int bytesRead;
	char buf[MAX_SIZE];
	// Load Winsock v2.2
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), (LPWSADATA)&wsadata) != 0) {
		MY_ERROR("Winsock Error\n");
	}
	// (address, type, protocol)
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);   
	// assign server address
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(serverPort);
	// bind server socket
	if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		MY_ERROR("Error: Binding failed.\n");
	}
	// client limits
	if (listen(serverSocket, 3) < 0) {
		MY_ERROR("Error: Exceed maximum 3 connections.\n");
	}
	// listening
	while (1) {
		printf("Listening...\n");
		clientAddressLen = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
		printf("Connection from: %s \n", inet_ntoa(clientAddress.sin_addr));
		// receive
		while ((bytesRead = recv(clientSocket, buf, MAX_SIZE, 0)) > 0) {
			buf[bytesRead] = '\0';
			int bufSize = strlen(buf);
			if (bufSize >= 5 && buf[0] == 'p' && buf[1] == 'u' && buf[2] == 't') {
				char filename[MAX_SIZE];
				memcpy(filename, &buf[4], bufSize - 4);
				FILE* fptr = fopen(filename, "wb");
				do {
					bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
					fwrite(buf, MAX_SIZE, bytesRead, fptr);
				} while (buf[bytesRead - 1] != '\0');
				fclose(fptr);
			} else if (!strncmp(buf, "get", strlen("get"))) {

			} else if (!strncmp(buf, "dir", strlen("dir"))) {
				char info[100000] = " Directory of ./upload\n\n";
				DIR *dir = opendir("."); // upload
				struct dirent *entry;
				struct stat file;
				int totalFile = 0, totalDir = 0;
				long long totalFileSize = 0;
				while ((entry = readdir(dir)) != NULL) {
					char tmp[1000];
					if (entry->d_type == DT_DIR) {
						++totalDir;
						sprintf(tmp, "    <DIR>\t\t\t %s\n",
							//ctime(&file.st_mtime),
							entry->d_name
						);
					} else {
						++totalFile;
						long fileSize = 0;// file.st_size;
						totalFileSize += fileSize;
						sprintf(tmp, "         \t %s\n",
							//ctime(&file.st_mtime),
							//fileSize,
							entry->d_name
						);
					}
					strcat(info, tmp);
				}
				closedir(dir);
				char tmp[1000];
				sprintf(tmp, "\t\t%d File(s)\t%lld bytes\n\t\t%d Dir(s)\n\n",
					totalFile,
					totalFileSize,
					totalDir
				);
				strcat(info, tmp);
				// TODO: >1024
				send(clientSocket, info, strlen(info), 0);
				// recv(clientSocket, buf, MAX_SIZE, 0);
			} else if (!strncmp(buf, "rename", strlen("rename"))) {

			} else {
				// respond ERROR
			}
		}
		closesocket(clientSocket);
	}
	return 0;
}
