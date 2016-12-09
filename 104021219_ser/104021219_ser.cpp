#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
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
			// parse filename
			char filename[MAX_SIZE];
			strcpy(filename, &buf[4]);
			// check if server has the file(name)
			// [WTF]: yes; [LGTM]: no
			FILE* fptr = fopen(filename, "rb");
			if (fptr != NULL) {
				send(clientSocket, "WTF", 3, 0);
				fclose(fptr);
			}
			else {
				send(clientSocket, "LGTM", 4, 0);
			}
			// check client decision
			bytesRead = recv(clientSocket, buf, MAX_SIZE, 0);
			// [OK]: put file; [G8]: cancel put file
			if (bytesRead >= 2 && buf[0] == 'O' && buf[1] == 'K') {
				int fileSize = 0;
				fptr = fopen(filename, "wb");
				// remove 2 bytes check codes
				memmove(buf, buf + 2, bytesRead);
				fwrite(buf, 1, bytesRead - 2, fptr);
				fileSize += bytesRead - 2;
				while (1) {
					memset(buf, 0, MAX_SIZE);
					bytesRead = recv(clientSocket, buf, MAX_SIZE, 0);
					if (bytesRead > 0) {
						fwrite(buf, 1, bytesRead, fptr);
						fileSize += bytesRead;
					}
					else break;
				}
				fclose(fptr);
				printf(
					"[%s] put %s success (%d bytes).\n",
					inet_ntoa(clientAddress.sin_addr),
					filename,
					fileSize
				);
			}
			else {
				printf("[%s] put %s fail.\n", inet_ntoa(clientAddress.sin_addr), filename);
			}
		}
		else if (bytesRead >= 5 && buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') {
			// parse filename
			char filename[MAX_SIZE];
			strcpy(filename, &buf[4]);
			FILE* fptr = fopen(filename, "rb");
			// check if server has the file
			// [WTF]: no; [LGTM]: yes
			if (fptr == NULL) {
				send(clientSocket, "WTF", 3, 0);
				printf("[%s] get %s fail.\n", inet_ntoa(clientAddress.sin_addr), filename);
			}
			else {
				send(clientSocket, "LGTM", 4, 0);
				int fileSize = 0;
				while (1) {
					memset(buf, 0, MAX_SIZE);
					bytesRead = fread(buf, 1, MAX_SIZE, fptr);
					if (bytesRead > 0) {
						send(clientSocket, buf, bytesRead, 0);
						fileSize += bytesRead;
					}
					else break;
				}
				fclose(fptr);
				printf(
					"[%s] get %s success (%d bytes).\n",
					inet_ntoa(clientAddress.sin_addr),
					filename,
					fileSize
				);
			}
		}
		else if (bytesRead == 3 && buf[0] == 'd' && buf[1] == 'i' && buf[2] == 'r') {
			// dir to ./GBY.txt
			_chdir("..");
			system("dir upload>GBY.txt");
			FILE* fptr = fopen("GBY.txt", "rb");
			while ((bytesRead = fread(buf, 1, MAX_SIZE, fptr)) > 0) {
				send(clientSocket, buf, bytesRead, 0);
				memset(buf, 0, MAX_SIZE);
			}
			fclose(fptr);
			system("del GBY.txt");
			printf("[%s] dir success.\n", inet_ntoa(clientAddress.sin_addr));
			_chdir("upload");
		}
		else if (
			bytesRead >= 8 &&
			buf[0] == 'r' &&
			buf[1] == 'e' &&
			buf[2] == 'n' &&
			buf[3] == 'a' &&
			buf[4] == 'm' &&
			buf[5] == 'e'
		) {
			// parse filename1 filename2
			buf[bytesRead] = '\0';
			int valid = 1;
			strtok(buf, " ");
			char* filename1 = strtok(NULL, " "), *filename2 = NULL;
			if (filename1 == NULL) valid = 0;
			if (valid) filename2 = strtok(NULL, " ");
			if (filename2 == NULL) valid = 0;
			char ret[100];
			if (!valid) strcpy(ret, " command");
			// check if server has filename1 and filename2
			FILE* fptr = fopen(filename1, "rb");
			if (fptr == NULL) valid = 0;
			else fclose(fptr);
			fptr = fopen(filename2, "rb");
			if (fptr != NULL) {
				valid = 0;
				fclose(fptr);
			}
			if (!valid) strcpy(ret, " file");
			// [G8]: rename success
			if (valid) {
				char cmd[MAX_SIZE];
				sprintf(cmd, "ren %s %s", filename1, filename2);
				system(cmd);
				send(clientSocket, "G8", 2, 0);
				printf("[%s] rename success.\n", inet_ntoa(clientAddress.sin_addr));
			}
			else {
				send(clientSocket, ret, sizeof(ret), 0);
				printf("[%s] Invalid: %s.\n", inet_ntoa(clientAddress.sin_addr), ret);
			}
		}
		closesocket(clientSocket);
	}
	return 0;
}
