#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include<stdio.h>
#include<string.h>
#include<winsock.h>
#include<windows.h>
#include<pthread.h>
#include<math.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
SOCKET hsock;
SOCKADDR_IN saddr;

struct thread_params{
	FILE *fp;
	int start;
	int block_count;
};

SOCKET getsocket(){
	SOCKET hsock;
	hsock = socket(AF_INET, SOCK_STREAM, NULL);
	if (hsock == INVALID_SOCKET)
		printf("Socket error");
	return hsock;
}

void send_data(char *buff){
	hsock = getsocket();

	if (connect(hsock, (SOCKADDR *)&saddr, sizeof(saddr)) == SOCKET_ERROR)
		printf("connection Error");

	if (send(hsock, buff, 1024, 0) == SOCKET_ERROR)
		printf("Error sending data\n");
	//printf("Data sent is :%s\n", buff);
}

char * receive_data(){
	int len,buff_len=1024;
	char *buff;
	buff = (char *)malloc(sizeof(char)* 1024);
	memset(buff, 0, sizeof(char)* 1024);
	if ((len=recv(hsock, buff, buff_len, 0)) == SOCKET_ERROR)
		printf("Error in receiving data");
	//buff[len] = '\0';
	//printf("Received data is :%s", buff);
	return buff;
}

void * download_file(void* tp){
	struct thread_params *params = (struct thread_params *)tp;
	char buff[1024], *reply;
	int i,size;
	//fseek(params->fp, params->start, SEEK_SET);
	for (i = 0; i < params->block_count; i++){
		memset(buff, 0, 1024);
		_itoa(params->start + (i * 1024), buff, 10);
		pthread_mutex_lock(&mutex1);
		send_data(buff);
		reply = receive_data();
		//send_data(params->s, "ok");
		size = atoi(reply);
		if (size < 1024){
			printf("We are at the end\n");
			printf("");
		}
		//printf("%d %d\n", size,i);
		reply = receive_data();
		pthread_mutex_unlock(&mutex1);
		//send_data(params->s, "ok");
		fseek(params->fp, params->start + (i * 1024), SEEK_SET);
		fwrite(reply, sizeof(char), size, params->fp);
		fflush(params->fp);
	}
	return tp;
}

int main(){
	pthread_t p[4];
	SOCKET s=0;
	WSADATA wsdata;
	char buff[100],*reply;
	int error = WSAStartup(0x0202, &wsdata);
	if (error){
		printf("Error in wsa startup\n");
		getchar();
	}

	if (wsdata.wVersion != 0x0202)
		printf("wsa Error");

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("127.0.0.2");
	saddr.sin_port = htons(6656);
/*	s = socket(AF_INET, SOCK_STREAM, NULL);
	if (s == INVALID_SOCKET)
		printf("socket Error");
	if (connect(s, (SOCKADDR *)&saddr, sizeof(saddr)) == SOCKET_ERROR)
		printf("connection Error");
	else
		printf("Connection successful");*/
	
	int size, block_nums,total_blocks,i,prev_strt,prev_blocks;
	float temp;
	FILE *fp;
	char filename[100];
	struct thread_params *tp;
	printf("Enter the full path of file to be downloaded\n");
	scanf("%[^\n]s", buff);
	send_data(buff);
	receive_data();
	reply = receive_data();
	send_data("ok");
	fflush(stdin);
	printf("enter the name for downloading file");
	scanf("%[^\n]s", filename);
	size = atoi(reply);
	temp = (float)size /(float) 1024;
	total_blocks = (int)ceil(temp);
	temp = (float)total_blocks / (float)4;
	block_nums = ceil(temp);
	prev_strt = 0;
	prev_blocks = 0;
	printf("Downloading.....\n");
	for (i = 0; i < 4; i++){
		tp = (struct thread_params *)malloc(sizeof(struct thread_params));
		fp = fopen(filename, "wb");
		tp->fp = fp;
		tp->start = (i == 0) ? prev_strt : (prev_strt + (prev_blocks * 1024));
		prev_strt = tp->start;
		if (i != 3)
			tp->block_count = block_nums;
		else
			tp->block_count = total_blocks;
		prev_blocks = tp->block_count;
		pthread_create(&p[i], NULL, download_file, tp);
		total_blocks = total_blocks - block_nums;
		if (i != 3){
			temp = (float)total_blocks / (float)(3 - i);
			block_nums = ceil(temp);
		}
	}
	for (i = 0; i < 4; i++)
		pthread_join(p[i], NULL);
	system("cls");
	printf("download successfull\n");
	fflush(stdin);
	getchar();
	//buff[0] = -1; buff[1] = '\0';
	//send_data(buff);
	return 0;
}