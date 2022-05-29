#ifndef MYCONNECTION_H
#define MYCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include<stdint.h>

#include <map>
#include <list>
#include "mypro_error.h"
#include "../lock/locker.h"
#include "../log/log.h"

void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);


enum msgTypes
{
	CONNECT = 1, CONNACK, PUBLISH, PUBACK, PUBREC, PUBREL,
	PUBCOMP, SUBSCRIBE, SUBACK, UNSUBSCRIBE, UNSUBACK,
	PINGREQ, PINGRESP, DISCONNECT
};


class my_conn
{
public:

    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;


public:
    my_conn() {}
    ~my_conn() {}

public:
    void init(int sockfd, const sockaddr_in &addr, int);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }



private:
	bool mypro_read_packet(uint8_t *packet_type);
    void mypro_connect_packet_handle();
	void mypro_subscribe_packet_handle();
	int mypro_publish_packet_handle();

	void add_write_queue(char *add_buf);

    char *get_line() { return m_read_buf + m_start_line; };


public:
    static int m_epollfd;
    static int m_user_count;
	//static map<string, my_conn*> m_clientId_map;
    int m_state;  //读为0, 写为1

	
	string mypro_client_id;

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];
	char *m_packet_buf;
    int m_read_idx;
    int m_checked_idx;
	uint16_t remain_len;
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;

	
    int m_TRIGMode;
	int m_close_log;

	//MQTT相关
	
	size_t mypro_client_len;

	std::list<char *> m_write_queue;
	std::list<my_conn *> m_publish_list;
	locker write_queue_locker;       //保护请求队列的互斥锁
	



};

#endif
