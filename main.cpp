#include "myserver.h"


int main(int argc, char *argv[])
{
	//端口号,默认9000
    int PORT = 9090;
	const char *SERIP = "192.168.46.128";
    //触发组合模式,默认listenfd LT + connfd LT
    int TRIGMode = 0;
    //listenfd触发模式，默认LT
    int LISTENTrigmode = 0;
    //connfd触发模式，默认LT
    int CONNTrigmode = 0;
    //线程池内的线程数量,默认8
    int thread_num = 4;
	//日志写入方式，默认同步
    int LOGWrite = 0;


	MyServer server;
	server.init(PORT, SERIP, thread_num, TRIGMode);

	//日志
    server.log_write();
	server.thread_pool();
	server.trig_mode();
	server.eventListen();
	server.eventLoop();

	return 0;

}

