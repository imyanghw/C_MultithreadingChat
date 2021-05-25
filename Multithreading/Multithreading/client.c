#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h> //주소변환 기능사용 
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 13

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

void *sendMsg(void *arg);
void *recvMsg(void *arg);
void error_handling(char *msg);

int main(int argc, char *argv[])
{
	pthread_t snd_t, rcv_t;
	struct sockaddr_in serv_addr;
	void *return_t;
	int sock;

	//input error
	if (argc != 4) {
		printf("Wrong input. Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	sprintf(name, "[%s]", argv[3]);
	//create socket
	sock = socket(PF_INET, SOCK_STREAM, 0); //연결 지향형 socket생성
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //주소체계 IPv4
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //입력받는 첫번째 값을 IP주소에 넣기
	serv_addr.sin_port = htons(atoi(argv[2])); //입력받는 두번째 값을 Port번호에 넣기

	//error
	if (connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("connecting error occurred");
	} //클라이언트 -> 서버로의 연결 시도

	//쓰레드 생성
	pthread_create(&snd_t, NULL, sendMsg, (void*)&sock); //송신 쓰레드
	pthread_create(&rcv_t, NULL, recvMsg, (void*)&sock); //수신 쓰레드
	pthread_join(snd_t, &return_t);
	pthread_join(rcv_t, &return_t);

	close(sock);
	return 0;
}

// sending thread
void *sendMsg(void *arg) {
	int sock = *((int *)arg); //소켓을 받음
	char name_msg[NAME_SIZE + BUF_SIZE];

	write(sock, name, strlen(name));

	while (1) {
		fgets(msg, BUF_SIZE, stdin);
		//exit
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
			close(sock);
			exit(0);
		}

		int isPos = 0; //@수신자 이름, 메시지 내용을 입력하여 특정 상대에게 메시지 전송
		for (int i = 0; i < BUF_SIZE; i++) {
			if (msg[i] == '@') {
				isPos = 1;
				break;
			}
		}
		if (isPos == 0) {
			printf("Target user not found!\n");
			continue;
		}

		sprintf(name_msg, "%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg)); //서버로 메시지를 보냄
	}
	return NULL;
}

//reading thread
void *recvMsg(void *arg) {
	int sock = *((int *)arg); //호출받은 소켓 연결
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len; //메시지 길이 변수

	while (1) {
		//get message from server
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1); //메시지 수신(읽기)

		if (str_len == -1) {
			return (void *)-1;
		}

		char cur_name[NAME_SIZE];
		int clnt_name_size = 0;
		int idx = 0;

		//get receiver's name
		while (idx < BUF_SIZE) {
			if (name_msg[idx] == '@') {
				idx++;
				break;
			}
			idx++;
		}//@ 구분으로 인원수 파악

		for (int j = 0; name_msg[idx + j] != 32; j++) {
			cur_name[j] = name_msg[idx + j];
			clnt_name_size++;
		}

		//check if this user is receiver
		int isPos = 1;
		for (int j = 0; j < clnt_name_size; j++) {
			if (name[j + 1] != cur_name[j]) {
				isPos = 0;
				break;
			}
		}
		if (cur_name[0] == 'a' && cur_name[1] == 'l' && cur_name[2] == 'l') {
			isPos = 1;
		}

		//print message if this user is rigth receiver
		if (isPos == 1) {
			name_msg[str_len] = 0;

			char fnl_msg[BUF_SIZE + NAME_SIZE];
			int cur = 0;

			//get sender's name and message
			for (int j = 0; j < str_len; j++) {
				if (name_msg[j] == '@') {
					while (name_msg[j] != 32) {
						j++;
					}
					j++;
				}
				fnl_msg[cur++] = name_msg[j];
			}
			fnl_msg[cur] = 0;
			//print
			fputs(fnl_msg, stdout);
		}
	}
	return NULL;
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}