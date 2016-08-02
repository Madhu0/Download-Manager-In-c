#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include<stdio.h>
#include<string.h>
#include<winsock.h>
#include<windows.h>

SOCKADDR_IN client_adr;
SOCKET csock,sock;

void send_data(char *buff){
	if (send(csock, buff,1024,0) == SOCKET_ERROR)
		printf("Error sending data\n");
	//printf("Data sent is :%s\n", buff);
}

char * receive_data(){
	char *buff;
	int cl;
	cl = sizeof(SOCKADDR_IN);
	buff = (char *)malloc(sizeof(char)* 1024);
	int len;

	csock = accept(sock, (SOCKADDR *)&client_adr, &cl);

	if ((len=recv(csock, buff, 1024, 0)) == SOCKET_ERROR)
		printf("Error in receiving data");
	if (len!=1024)
		buff[len] = '\0';
	//printf("Received data is :%s\n", buff);
	return buff;
}

void handle_requests(FILE *fp,int file_end){
	char buff[1024], *reply,size_buff[1024];
	int pos,size=0;
	while (1){
		file_end = file_end - 1024;
		reply=receive_data();
		//send_data(csock,"ok");
 		pos=atoi(reply);
		fseek(fp, pos, SEEK_SET);
		size=fread(buff, 1, 1024, fp);
		printf("%d\n", file_end);
		_itoa(size, size_buff, 10);
		send_data(size_buff);
		//receive_data(csock);
		send_data(buff);
		free(reply);
		if (file_end <= 0)
			return;
	}
}

void initialize(){
	int size;
	FILE *fp;
	char *reply, buff[1024];
	reply = receive_data();
	send_data("ok");
	fp = fopen(reply, "rb");
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	_itoa(size, buff, 10);
	send_data(buff);
	receive_data();
	handle_requests(fp,size);
}

int main(){
	int cl;
	char buff[100];
	WSADATA wsdata;
	SOCKADDR_IN adr;
	int error = WSAStartup(0x0202, &wsdata);
	if (error){
		printf("Error in wsa startup\n");
		getchar();
	}

	if (wsdata.wVersion != 0x0202)
		printf("wsa Error");

	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = inet_addr("127.0.0.2");
	adr.sin_port = htons(6656);

	sock = socket(AF_INET, SOCK_STREAM, NULL);
	if (sock == INVALID_SOCKET)
		printf("Socket eroor");
	if (bind(sock, (SOCKADDR *)&adr, sizeof(adr)) == SOCKET_ERROR)
		printf("binding error");
	printf("listening....\n");
	if(listen(sock, 10)==-1)
		printf("listening error");

/*	csock=accept(s, (SOCKADDR *)&client_adr, &cl);
	printf("received connection from %s\n",inet_ntoa(client_adr.sin_addr));*/
	initialize();
	fflush(stdin);
	//getchar();
}