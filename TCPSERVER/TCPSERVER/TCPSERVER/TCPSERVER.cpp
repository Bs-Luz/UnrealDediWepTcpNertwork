#include "iostream"
//socket 통신을 위한 헤더
#include <WinSock2.h>

using namespace std;
//WinSock2.h를 사용하기 위한 라이브러리(VS 내장)
#pragma comment (lib, "ws2_32.lib")

int main()
{
	const int DEDICATED_SERVER_PORT = 7777;

	WSAData wsaData;

	//WinSock 초기화
	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		cout << "WinSock2 Error" << endl;

		return -1;
	}

	//SOCKET 생성
	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (ServerSocket == INVALID_SOCKET)
	{
		cout << "소켓 만들 수 없음(create)" << GetLastError() << endl;
		//에러 번호 호출을 위한 함수(GetLastError)
		return -1;
	}

	//SOCKET 연결(bind)
	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	//서버 IP를 넣어주기위한 초기화 작업 0으로 초기화
	ServerSockAddr.sin_family = AF_INET; //IPv4형식의 아이피 사용
	ServerSockAddr.sin_port = htons(10101); //접속할 포트 설정
	ServerSockAddr.sin_addr.s_addr = INADDR_ANY;

	Result = bind(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	//실제 IP와 만든 SOCKET을 사용해도 되?
	if (Result == SOCKET_ERROR)
	{
		cout << "소켓 만들 수 없음(bind)" << GetLastError() << endl;
		//에러 번호 호출을 위한 함수(GetLastError)
		return -1;
	}

	//통신 연결 대기 (Listen)
	Result = listen(ServerSocket, 0);

	if (Result == SOCKET_ERROR)
	{
		cout << "소켓 만들 수 없음(listen)" << GetLastError() << endl;
		//에러 번호 호출을 위한 함수(GetLastError)
		return -1;
	}

	//Accept
	SOCKADDR_IN ClientSockAddr;
	memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
	int ClientSockAddrLength = sizeof(ClientSockAddr);
	//TCP 방식에서 사용하는 길이만 사용할 수 있도록 길이 지정
	int ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);

	if (ClientSocket == SOCKET_ERROR)
	{
		cout << "소켓 만들 수 없음(Accept)" << GetLastError() << endl;
		//에러 번호 호출을 위한 함수(GetLastError)
		return -1;
	}

	//Send/Recv 정보를 보내고 받기
	const char* data = "TCP 서버에서 보내는 데이터입니다.";
	int dataLength = strlen(data);
	int sentBytes = send(ClientSocket, data, dataLength, 0);
	if (sentBytes == SOCKET_ERROR) {
		cout << "데이터 전송 실패" << endl;


		//소켓 닫기
		closesocket(ClientSocket);
		closesocket(ServerSocket);

		//WinSock 닫기
		WSACleanup();
	}
}