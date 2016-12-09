# Introduction to Computer Networks Lab1
###### 104021219 鄭余玄

## Description
Implement a simple FTP model using Winsock.
The server creates a FTP server and allow users (clients) to upload and download files.
The client is able to access the FTP server and upload and download files.


## Usage
For server,
```
./104021219_ser.exe <Port>
```
For client,
```
./104021219_cli.exe <IPv4> <Port>
```

### Command functions
- put <filename>
- get <filename>
- dir
- rename <old filename> <new filename>

### Remark
- each packet is less than 1024 bytes
-  handle illegal-command exceptions

## Build Environment
- Windows x64
- Visual Studio 14.0
- C++