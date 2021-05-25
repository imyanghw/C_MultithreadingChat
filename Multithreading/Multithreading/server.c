#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h> //�ּҺ�ȯ ��ɻ�� 

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

	serv_sock = socket(PF_INET, SOCK_STREAM, 0); //���� ������ socket����

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //�ּ�ü�� IPv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //socket�� IP�ּ� �Ҵ�
	serv_addr.sin_port = htons(atoi(argv[1])); //socket�� PORT �Ҵ�

	if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("bind() error");
	} //�ּ����� �Ҵ�

	if (listen(serv_sock, 5) == -1) {
		error_handling("listen() error");
	} //�����û ���	

	FD_ZERO(&fds); //fd_set�� ������ ��Ʈ�� 0���� �ʱ�ȭ
	FD_SET(serv_sock, &fds); //fd�� ���޵� ���� ��ũ���� ������ ����� �ش��ϴ� ��Ʈ�� 1�� ����
	fd_max = serv_sock;

	while (1) {
		cpy_fds = fds; //select �Լ�ȣ���� ������ ��ȭ�� ���� ���� ��ũ������ ��ġ�� ������ ������ ��ġ�� ��Ʈ���� 0���� ����
		//������ ������ ���ؼ� ���纻�� ����

		//Ÿ�Ӿƿ� ������ ���� �ڵ� : select �Լ� ȣ������ timeval ����ü ������ �ʱ�ȭ�� �ݺ�
		timeout.tv_sec = 5; //���μ��� ������(�� ����)
		timeout.tv_usec = 5000;//���μ��� ������(����ũ�� �ʴ���)

		//select �Լ� ȣ��
		if ((fd_num = select(fd_max + 1, &cpy_fds, 0, 0, &timeout)) == -1) {
			break;
		} //���� ��ũ���� ���� 0�������� �����ϱ� ������ 1�� ������
		if (fd_num == 0) {
			continue;
		}

		//check every fd
		for (int i = 0; i < fd_max + 1; i++) {
			if (!FD_ISSET(i, &cpy_fds)) {
				continue;
			} //���� ��ũ���� ������ ������ ����� ��ȯ

			//connection request
			if (i == serv_sock) {
				addr_size = sizeof(clnt_addr);
				clnt_sock =
					accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
				//���� ������ ���º�ȭ�� ������ �����û�� ���� ������ ������ ����
				clnt_socks[clnt_cnt++] = clnt_sock;
				FD_SET(clnt_sock, &fds); //fd_set�� ���� fds�� Ŭ���̾�Ʈ�� ����� ������ ���� ��ũ���� ������ ���
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
					//������ �����Ͱ� EOF�� ��쿡�� ������ �����ϰ� ���� fds���� �ش������� �����ϴ� ������ ��ħ

					clnt_checks[i] = 0;
					for (int j = 0; j < NAME_SIZE; j++) {
						clnt_names[i][j] = 0;
					}
				}//���º�ȭ�� �߻��� ������ ���� ������ �ƴ� ��쿡 ����

				//get client name after connect request
				else if (clnt_checks[i] == 0) {
					//client �̸��� null�� ���
					for (int j = 0; j < NAME_SIZE; j++) {
						clnt_names[i][j] = 0;
					}
					//client �̸��� ������
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
				}//������ �����Ͱ� ���ڿ��� ��� echo

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