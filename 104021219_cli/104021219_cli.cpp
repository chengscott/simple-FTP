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
			printf("Invalid: Command is too long.\n");
			continue;
		}
		// connection
		serverSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
			MY_ERROR("Connect Error\n");
		}
		// command
		if (bufSize >= 5 && buf[0] == 'p' && buf[1] == 'u' && buf[2] == 't') {
			// put to ./upload
			_chdir("upload");
			int cancel = 0;
			// parse filename
			char filename[MAX_SIZE];
			strcpy(filename, &buf[4], bufSize - 3);
			int fileSize = 0;
			// check if client has the file
			FILE* fptr = fopen(filename, "rb");
			if (fptr == NULL) {
				printf("Invaild: %s is not found!\n", filename);
				cancel = 1;
			}
			// check if server has the file(name)
			if (!cancel) {
				send(serverSocket, buf, strlen(buf) + 1, 0);
				memset(buf, 0, MAX_SIZE);
				bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
				// [WTF]: server has the file(name)
				if (bytesRead == 3 && strcmp(buf, "WTF") == 0) {
					printf("Overwrite %s in server? [Y/n]", filename);
					char res;
					scanf("%c", &res);
					// [G8]: cancel put file
					if (res == 'n') {
						send(serverSocket, "G8", 2, 0);
						cancel = 1;
					}
				}
			}
			if (!cancel) {
				// [OK]: put file
				send(serverSocket, "OK", 2, 0);
				while (1) {
					memset(buf, 0, MAX_SIZE);
					bufSize = fread(buf, 1, MAX_SIZE, fptr);
					if (bufSize > 0) {
						send(serverSocket, buf, bufSize, 0);
						fileSize += bufSize;
					}
					else break;
				}
				printf("put %s success (%d bytes).\n", filename, fileSize);
			}
			if (fptr != NULL) fclose(fptr);
			_chdir("..");
		} else if (bufSize >= 5 && buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') {
			// get to ./download
			_chdir("download");
			int cancel = 0;
			// parse filename
			char filename[MAX_SIZE];
			strcpy(filename, &buf[4], bufSize - 3);
			// check if client has the file(name)
			FILE* fptr = fopen(filename, "rb");
			if (fptr != NULL) {
				printf("Overwrite current %s file? [Y/n]", filename);
				char res;
				scanf("%c", &res);
				if (res == 'n') cancel = 1;
				fclose(fptr);
			}
			if (!cancel) {
				// check if server has the file
				send(serverSocket, buf, strlen(buf) + 1, 0);
				memset(buf, 0, MAX_SIZE);
				bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
				// [WTF]: server does not have the file
				if (bytesRead == 3 && strcmp(buf, "WTF") == 0) {
					cancel = 1;
					printf("Invalid: %s file not found in server.\n", filename);
				}
			}
			if (!cancel) {
				int fileSize = 0;
				fptr = fopen(filename, "wb");
				while (1) {
					memset(buf, 0, MAX_SIZE);
					bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
					if (bytesRead > 0) {
						fwrite(buf, 1, bytesRead, fptr);
						fileSize += bytesRead;
					}
					else break;
				}
				printf("get %s success (%d bytes).\n", filename, fileSize);
				fclose(fptr);
			}
			_chdir("..");
		} else if (bufSize == 3 && buf[0] == 'd' && buf[1] == 'i' && buf[2] == 'r') {
			send(serverSocket, "dir", 3, 0);
			// get temp file to ./GBY.txt
			FILE* fptr;
			fptr = fopen("GBY.txt", "wb");
			while ((bytesRead = recv(serverSocket, buf, MAX_SIZE, 0)) > 0) {
				fwrite(buf, 1, bytesRead, fptr);
				memset(buf, 0, MAX_SIZE);
			}
			fclose(fptr);
			// show dir result
			fptr = fopen("GBY.txt", "r");
			char c;
			while ((c = getc(fptr)) != EOF) putchar(c);
			fclose(fptr);
			// remove temp file ./GBY.txt
			system("del GBY.txt");
		}
		else if (
			bufSize >= 8 &&
			buf[0] == 'r' &&
			buf[1] == 'e' &&
			buf[2] == 'n' &&
			buf[3] == 'a' &&
			buf[4] == 'm' &&
			buf[5] == 'e'
		) {
			send(serverSocket, buf, bufSize, 0);
			char GBY[MAX_SIZE];
			bytesRead = recv(serverSocket, GBY, MAX_SIZE, 0);
			// [G8]: success
			if (bytesRead == 2 && GBY[0] == 'G' && GBY[1] == '8')
				printf("%s success.\n", buf);
			else printf("Invalid: %s.\n", buf);
		}
		closesocket(serverSocket);
		memset(buf, 0, MAX_SIZE);
	}
	return 0;
}
