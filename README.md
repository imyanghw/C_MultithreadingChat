# C_MultithreadingChat
멀티쓰레드를 활용한 채팅 프로그램
> 쓰레드의 생성 및 실행흐름을 제어하여 이를 활용한 간단한 채팅프로그램을 제작

## 개발환경 및 사용버전
- Linux Ubuntu
- C Language

## 구현내용
- 모든 클라이언트는 전체채팅 메시지를 볼 수 있음
- 여러 client가 접속하여 채팅이 가능함
- 최대 메시지 버퍼: 100
- 채팅방 닉네임 최대길이 : 13
- Client based on TCP multiprocess with I/O routine split

## server.c
코드 컴파일
> "[실행파일 이름] [ip 주소] [port 번호]" 입력

## client.c
코드 컴파일
> 클라이언트들은 각각 "[실행파일 이름] [IP 주소] [port 번호] [이름]" 입력하여 서버와 연결<
> 전송하고자 하는 메시지 입력


## Screenshots


## 프로젝트 고찰
문제점


해결


프로그램 장/단점
