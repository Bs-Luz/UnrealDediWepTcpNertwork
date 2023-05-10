#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

//---C
#include <WinSock2.h>
//---C++
#include <iostream>
#include <string.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
//---Plugin Library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "vs14/mysqlcppconn.lib")

using namespace std;

const string DBIP = "192.168.0.15:3306";					//MySQL IP
const string DBID = "sihoon";								//MySQL ID
const string DBPass = "qwer1234";							//MySQL Pass
sql::Driver* DB_Driver = nullptr;							//MySQL 드라이버
sql::Connection* DB_Connection = nullptr;					//MySQL 데이터베이스 연결
sql::Statement* DB_Statement = nullptr;						//MySQL 데이터베이스에 대한 SQL 을 실행하는데 사용
sql::PreparedStatement* DB_PreparedStatement = nullptr;		//MySQL 데이터베이스에 미리 컴파일된 SQL 을 실행하는데 사용
sql::ResultSet* DB_ResultSet = nullptr;						//MySQL 데이터베이스에서 반환된 결과 집합

const int MAXPlayer = 2;		//데디서버에 들어갈 수 있는 최대 플레이어 수

#pragma pack(push, 1)
struct MyData
{
	int PlayerNum;
	uint16_t ServerPort;
	char IP[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DBInfoData
{
	uint16_t ServerPort;
	char IP[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DBDatas
{
	uint16_t ServerPort;
	char IP[16];
	uint16_t ServerPlayer;
};
#pragma pack(pop)

void SendToDB(sql::Statement* _statement, string IPAddr, int PortNum, int PlayerNum, int Server_State)
{
	// DB에 입력될 데이터 값
	string IP = IPAddr;
	string stringPort = to_string(PortNum);
	string stringPlayerNumber = to_string(PlayerNum);
	string server_state = to_string(Server_State);

	// 쿼리문 세팅
	string query = "INSERT INTO server_list (ip_address, port_number, player_number, server_state) VALUES ('" + IP + "', " + stringPort + ", " + stringPlayerNumber + ", " + server_state + ") ON DUPLICATE KEY UPDATE player_number=" + stringPlayerNumber + ";";

	try
	{
		// 쿼리문 날리기
		_statement->execute(query);
	}
	catch (sql::SQLException err)
	{
		cout << "DB Send Error" << endl;
		exit(-2);
	}
}

void ReceiveFromDB(sql::Statement* _statement, sql::ResultSet*& _resultset)
{
	// 쿼리문 세팅
	string query = "SELECT * from server_list";

	try {
		// 실행하고 결과 받아오는게 있을 때 executeQuery 사용하자(결과값은 ResultSet* 타입이다)
		_resultset = _statement->executeQuery(query);
	}
	catch (sql::SQLException err)
	{
		cout << "DB Receive Error" << endl;
	}

}


int main()
{
	//---MySQL
	try
	{
		DB_Driver = get_driver_instance();	//mysql 드라이버를 가져와서 연결
		DB_Connection = DB_Driver->connect(DBIP, DBID, DBPass);	//데이터베이스 접속
		DB_Connection->setSchema("server_info");
		DB_Statement = DB_Connection->createStatement();
		cout << "--->DB" << endl;
	}
	catch (sql::SQLException e)
	{
		cout << "--->NOT DB" << endl;
		return -1;
	}

	//---WinSock
	WSAData wsaData;

	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);		//winsock 초기화
	if (Result != 0) { cout << "WinSock2 Error"; return -1; }	//초기화 안되면 에러처리

	SOCKET TCPServerSocket = socket(PF_INET, SOCK_STREAM, 0);	//가상의 socket 만들기
	if (TCPServerSocket == INVALID_SOCKET) { cout << "Can't make Socket" << GetLastError(); return -1; }

	SOCKADDR_IN TCPServerSocketAddr;	//소켓 주소 구조체 만들기
	memset(&TCPServerSocketAddr, NULL, sizeof(TCPServerSocketAddr));	//초기화
	TCPServerSocketAddr.sin_family = AF_INET;				//IPv4
	TCPServerSocketAddr.sin_port = htons(10101);			//port
	TCPServerSocketAddr.sin_addr.s_addr = INADDR_ANY;		//모두 수신

	//socket 연결
	//mysql 에도 bind 라는 api 가 있기때문에 ::bind 를 쓰면 소켓bind 를 사용한다.
	Result = ::bind(TCPServerSocket, (SOCKADDR*)&TCPServerSocketAddr, sizeof(TCPServerSocketAddr));
	if (Result == SOCKET_ERROR) { cout << "Can't bind" << GetLastError(); return -1; }

	//server 열기
	Result = listen(TCPServerSocket, 0);
	if (Result == SOCKET_ERROR) { cout << "Can't listen" << GetLastError(); return -1; }

	//fd  
	fd_set ReadSockets;
	fd_set CopyReadSockets;
	FD_ZERO(&ReadSockets);
	FD_SET(TCPServerSocket, &ReadSockets);

	//반복
	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 100;

	//무한루프
	while (1)
	{
		CopyReadSockets = ReadSockets;			//복제
		int ChangeCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);	//수신 가능한 데이터가 있는 소켓 갯수
		if (ChangeCount == 0) continue;	//수신이 가능한 소켓이 없다, 다시돌려
		else
		{
			for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
			{
				if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
				{
					if (ReadSockets.fd_array[i] == TCPServerSocket)
					{
						SOCKADDR_IN DediServerSockAddr;
						memset(&DediServerSockAddr, NULL, sizeof(DediServerSockAddr));
						int DediServerSockAddrLength = sizeof(DediServerSockAddr);

						SOCKET DediServerSocket = accept(TCPServerSocket, (SOCKADDR*)&DediServerSockAddr, &DediServerSockAddrLength);
						FD_SET(DediServerSocket, &ReadSockets);
					}
					else
					{
						char Buffer[1024] = { 0, };
						int RecvBytes = recv(ReadSockets.fd_array[i], Buffer, sizeof(int), 0);
						Buffer[RecvBytes] = '\0';	//버퍼의 마지막에 null추가
						if (RecvBytes == 0)	//데이터를 수신하지 않음
						{
							closesocket(ReadSockets.fd_array[i]);
							FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
						}
						else if (RecvBytes < 0)	//에러 
						{
							cout << "Error : " << GetLastError();
							closesocket(ReadSockets.fd_array[i]);
							FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
						}
						else
						{
							int number;
							memcpy(&number, Buffer, sizeof(int));
							//20230426 받은 데이터 : 서버
							if (number == 1)
							{
								char DediServerBuffer[1024] = { 0, };  
								int DediServerRecvBytes = recv(ReadSockets.fd_array[i], DediServerBuffer, sizeof(DediServerBuffer), 0);
								DediServerBuffer[DediServerRecvBytes] = '\0';	//버퍼의 마지막에 null추가
								MyData data;
								memcpy(&data, DediServerBuffer, sizeof(MyData));
								cout << "----------DediServer----------" << endl;
								cout << "PlayerNum : " << data.PlayerNum << endl;
								cout << "ServerPort : " << data.ServerPort << endl;
								cout << "IP : " << data.IP << endl;

								//20230426 DB연결
								cout << "----------DBInfo----------" << endl;
								SendToDB(DB_Statement, data.IP, data.ServerPort, data.PlayerNum, 1);
							}
							//20230426 받은 데이터 : 클라
							else
							{
								cout << "----------ClientServer----------" << endl;
								
								//1. db에서 포트번호랑 ip 받기
								ReceiveFromDB(DB_Statement, DB_ResultSet);
								DBInfoData DBData;
								memset(&DBData, 0, sizeof(DBData));
								//20230502
								DBDatas dbDatas[10];
								memset(&dbDatas, 0, sizeof(dbDatas));

								int k = 0;
								while (DB_ResultSet->next())
								{
									//20230502
									strncpy(dbDatas[k].IP, DB_ResultSet->getString("ip_address").c_str(), sizeof(dbDatas[k].IP));
									dbDatas[k].ServerPort = DB_ResultSet->getInt("port_number");
									dbDatas[k].ServerPlayer = DB_ResultSet->getInt("player_number");
									k++;
								}
								//cout 
								for (int n = 0; n < 4; n++)
								{
									cout << "DB------------------------------" << endl;
									cout << dbDatas[n].IP << endl;
									cout << dbDatas[n].ServerPort << endl;
									cout << dbDatas[n].ServerPlayer << endl;
									cout << "--------------------------------" << endl;
								}

								//20230502
								for(int j = 0; j < 10; j++)
								{
									//조건1. 서버가 맨처음 7777 이어야할것
									//조건2. 7777서버가 풀일때 7778 서버로 들어가야할것
									if (dbDatas[j].ServerPort == 7777)
									{
										if (dbDatas[j].ServerPlayer <= MAXPlayer)
										{
											strncpy(DBData.IP, dbDatas[j].IP, sizeof(DBData.IP));
											cout << DBData.IP << endl;
											DBData.ServerPort = dbDatas[j].ServerPort;
											cout << DBData.ServerPort << endl;

											//2. 받은 정보를 클라에 넘겨주기
											char DBBuffer[1024] = { 0, };
											//데이터 복사
											memcpy(DBBuffer, &DBData, sizeof(DBInfoData));
											int bytesSent = 0;
											cout << "DB > Client" << endl;
											//데이터 넘겨주기
											send(ReadSockets.fd_array[i], DBBuffer, sizeof(DBData), bytesSent);
											cout << "------------------------------------" << endl;
											break;
										}
										else 
										{
											cout << "1 else" << endl;
										}
									}
									else if(dbDatas[j].ServerPort == 7778)
									{
										if (dbDatas[j].ServerPlayer <= MAXPlayer)
										{
											strncpy(DBData.IP, dbDatas[j].IP, sizeof(DBData.IP));
											cout << DBData.IP << endl;
											DBData.ServerPort = dbDatas[j].ServerPort;
											cout << DBData.ServerPort << endl;

											//2. 받은 정보를 클라에 넘겨주기
											char DBBuffer[1024] = { 0, };
											//데이터 복사
											memcpy(DBBuffer, &DBData, sizeof(DBInfoData));

											//--
											int bytesSent = 0;
											cout << "DB > Client" << endl;
											//데이터 넘겨주기
											send(ReadSockets.fd_array[i], DBBuffer, sizeof(DBData), bytesSent);
											cout << "------------------------------------" << endl;
											break;
										}
										else
										{
											cout << "2 else" << endl;
										}
									}
								}

							}
						}
					}
				}
			}
		}
	}

	closesocket(TCPServerSocket);		//소켓삭제
	WSACleanup();	//해제

	return 0;
}

