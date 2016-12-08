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
	if (listen(serverSocket, 1) < 0) {
		MY_ERROR("Error: Exceed maximum 1 connection.\n");
	}
	// listening
	printf("Listening...\n");
	while (1) {
		clientAddressLen = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
		// receive
		bytesRead = recv(clientSocket, buf, MAX_SIZE, 0);
		if (bytesRead >= 5 && buf[0] == 'p' && buf[1] == 'u' && buf[2] == 't') {
			char filename[MAX_SIZE];
			strcpy(filename, &buf[4]);
			FILE* fptr = fopen(filename, "wb");
			// TODO: warning cover, filesize, timing
			int fileLen = strlen(filename);
			memmove(buf, buf + 5 + fileLen, bytesRead);
			fwrite(buf, 1, bytesRead - 5 - fileLen, fptr);
			while (1) {
				memset(buf, 0, MAX_SIZE);
				bytesRead = recv(clientSocket, buf, MAX_SIZE, 0);
				if (bytesRead > 0) {
					fwrite(buf, 1, bytesRead, fptr);
				}
				else break;
			}
			fclose(fptr);
			printf("[%s] put %s success.\n", inet_ntoa(clientAddress.sin_addr), filename);
		}
		else if (bytesRead >= 5 && buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') {
			char filename[MAX_SIZE];
			strcpy(filename, &buf[4]);
			FILE* fptr = fopen(filename, "rb");
			if (fptr == NULL) {
				send(clientSocket, "WTF", 3, 0);
			}
			else {
				send(clientSocket, "LGTM", 4, 0);
				while (1) {
					memset(buf, 0, MAX_SIZE);
					bytesRead = fread(buf, 1, MAX_SIZE, fptr);
					if (bytesRead > 0) {
						send(clientSocket, buf, bytesRead, 0);
					}
					else break;
				}
				printf("[%s] get %s success.\n", inet_ntoa(clientAddress.sin_addr), filename);
			}
		}
		else if (!strncmp(buf, "dir", strlen("dir"))) {
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
				}
				else {
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
		}
		else if (bytesRead == 6 && strcmp(buf, "rename") == 0) {

		}
		else {
			// respond ERROR
		}
		closesocket(clientSocket);
	}
	return 0;
}
