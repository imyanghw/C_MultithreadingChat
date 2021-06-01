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
- 최대 접속자 수: 10
- 채팅방 닉네임 최대길이: 13
- Client based on TCP multiprocess with I/O routine split

## server.c
코드 컴파일
> "[실행파일 이름] [port 번호]" 입력

## client.c
코드 컴파일
> 클라이언트들은 각각 "[실행파일 이름] [IP 주소] [port 번호] [이름]" 입력하여 서버와 연결<br>
> "@[수신자 이름] [메시지 내용]"을 입력해서 전송하고자 하는 메시지 입력<br>

## Screenshots
<img src="https://user-images.githubusercontent.com/76413580/120250918-c6cd9700-c2ba-11eb-9b00-da9cf634fcd7.PNG"></image>

## 프로젝트 고찰
프로그램 장/단점
- I/O 멀티플렉싱을 활용하여 소켓을 한데 묶어서 최소한의 리소스 및 프로세스를 이용해서 데이터를 송수신함<br>
- 멀티프로세스 방식보다 효율적임<br>
- select 멀티플렉싱 기법은 동시접속자의 수가 한계가 있음<br>
- select 함수는 호출할 때마다 관찰대상에 대한 정보를 매번 운영체제에게 전달하기때문에 부담이 많이 따름<br>
