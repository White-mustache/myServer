#ifndef MYSERVER_H
#define MYSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./threadpool/threadpool.h"
#include "./myProtocol/my_conn.h"


const int MAX_FD = 1000;           //最大文件描述符
const int MAX_EVENT_NUMBER = 500; //最大事件数
//const int TIMESLOT = 5;             //最小超时单位

class MyServer
{
public:
	MyServer();
	~MyServer();

	void init(int port, const char *SERIP, int thread_num, int trigmode);

	void thread_pool();

	void log_write();
	void trig_mode();
	void eventListen();
	void eventLoop();
	void addClient(int connfd, struct sockaddr_in client_address);
	bool dealClinetData();
	void dealWithRead(int sockfd);
	void dealWithWrite(int sockfd);

public:
	const char* serverIP;
	int m_port;

	int m_close_log;
	
    //int m_pipefd[2];
    int m_epollfd;
    my_conn *user_boards;

	//线程池相关
	threadpool<my_conn> *m_pool;
    int m_thread_num;

    //epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

	
};

#endif

