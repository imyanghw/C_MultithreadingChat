#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h> //주소변환 기능사용 

#define BUF_SIZE 100
#define NAME_SIZE 13
#define MAX_CLNT 10

void error_handling(char *msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][NAME_SIZE];
int clnt_checks[MAX_CLNT];

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;

	char buf[BUF_SIZE];
	struct sockaddr_in serv_addr, clnt_addr;
	struct timeval timeout;
	int fd_max, str_len, fd_num;
	fd_set fds, cpy_fds;

	for (int i = 0; i < MAX_CLNT; i++)
	{
		clnt_checks[i] = 0;
	}

	socklen_t addr_size;
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0); //연결 지향형 socket생성

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //주소체계 IPv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //socket의 IP주소 할당
	serv_addr.sin_port = htons(atoi(argv[1])); //socket의 PORT 할당

	if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("bind() error");
	} //주소정보 할당

	if (listen(serv_sock, 5) == -1) {
		error_handling("listen() error");
	} //연결요청 대기	

	FD_ZERO(&fds); //fd_set형 변수의 비트를 0으로 초기화
	FD_SET(serv_sock, &fds); //fd로 전달된 파일 디스크립터 정보를 등록함 해당하는 비트를 1로 설정
	fd_max = serv_sock;

	while (1) {
		cpy_fds = fds; //select 함수호출이 끝나면 변화가 생긴 파일 디스크립터의 위치를 제외한 나머지 위치의 비트들은 0으로 설정
		//원본의 유지를 위해서 복사본에 저장

		//타임아웃 설정을 위한 코드 : select 함수 호출전에 timeval 구조체 변수의 초기화를 반복
		timeout.tv_sec = 5; //프로세스 잠재우기(초 단위)
		timeout.tv_usec = 5000;//프로세스 잠재우기(마이크로 초단위)

		//select 함수 호출
		if ((fd_num = select(fd_max + 1, &cpy_fds, 0, 0, &timeout)) == -1) {
			break;
		} //파일 디스크립터 값이 0에서부터 시작하기 때문에 1을 더해줌
		if (fd_num == 0) {
			continue;
		}

		//check every fd
		for (int i = 0; i < fd_max + 1; i++) {
			if (!FD_ISSET(i, &cpy_fds)) {
				continue;
			} //파일 디스크립터 정보가 있으면 양수를 반환

			//connection request
			if (i == serv_sock) {
				addr_size = sizeof(clnt_addr);
				clnt_sock =
					accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
				//서버 소켓의 상태변화가 맞으면 연결요청에 대한 수락의 과정을 진행
				clnt_socks[clnt_cnt++] = clnt_sock;
				FD_SET(clnt_sock, &fds); //fd_set형 변수 fds에 클라이언트와 연결된 소켓의 파일 디스크립터 정보를 등록
				if (fd_max < clnt_sock)
					fd_max = clnt_sock;
			}
			//read message
			else {
				str_len = read(i, buf, BUF_SIZE);
				//close request
				if (str_len == 0) {
					FD_CLR(i, &fds);
					close(i);
					printf("@%s has left the chat\n", clnt_names[i]);
					//수신한 데이터가 EOF인 경우에는 소켓을 종료하고 변수 fds에서 해당정보를 삭제하는 과정을 거침

					clnt_checks[i] = 0;
					for (int j = 0; j < NAME_SIZE; j++) {
						clnt_names[i][j] = 0;
					}
				}//상태변화가 발생한 소켓이 서버 소켓이 아닌 경우에 실행

				//get client name after connect request
				else if (clnt_checks[i] == 0) {
					//client 이름이 null일 경우
					for (int j = 0; j < NAME_SIZE; j++) {
						clnt_names[i][j] = 0;
					}
					//client 이름을 가져옴
					for (int j = 1; buf[j] != ']'; j++) {
						clnt_names[i][j - 1] = buf[j];
					}
					printf("User @%s has joined the chat\n", clnt_names[i]);
					clnt_checks[i] = 1;
				}

				//send message to all clients
				else {
					for (int j = 0; j < clnt_cnt; j++) {
						write(clnt_socks[j], buf, str_len);
					}
				}//수신한 데이터가 문자열인 경우 echo

				//set buf as 0
				for (int j = 0; j < BUF_SIZE; j++) {
					buf[j] = 0;
				}
			}
		}
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}