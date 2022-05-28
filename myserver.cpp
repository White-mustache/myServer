#include "myserver.h"

MyServer::MyServer()
{
	user_boards = new my_conn[MAX_FD];
	
}

MyServer::~MyServer()
{
	close(m_epollfd);
	close(m_listenfd);

	delete[] user_boards;
	delete m_pool;
}

void MyServer::init(int port, const char *SERIP, int thread_num, int trigmode)
{
	m_port = port;
	serverIP = SERIP;
	m_thread_num = thread_num;
	m_TRIGMode = trigmode;
}

void MyServer::trig_mode()
{
    //LT + LT
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}

void MyServer::log_write()
{
	m_close_log = 0;
	//同步写日志
	Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
}


void MyServer::thread_pool()
{
    //线程池
    m_pool = new threadpool<my_conn>(m_thread_num);
}



void MyServer::eventListen()
{
	m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(m_listenfd >= 0);

	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton( AF_INET, serverIP, &address.sin_addr);
    address.sin_port = htons(m_port);

    struct linger tmp = { 1, 0 };
    setsockopt( m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof( tmp ) );

    //int flag = 1;
	
	//设置复用，closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket
    //setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

	epoll_event events[MAX_EVENT_NUMBER];
	m_epollfd = epoll_create(5);
	assert(m_epollfd != -1);

	my_conn::m_epollfd = m_epollfd;
	addfd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
	//ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
}

void MyServer::addClient(int connfd, struct sockaddr_in client_address)
{
    user_boards[connfd].init(connfd, client_address,m_CONNTrigmode);

}


bool MyServer::dealClinetData()
{
	LOG_INFO("deal Clinet Data");
	struct sockaddr_in client_address;
	socklen_t client_addr_len = sizeof(client_address);
	if(0 == m_LISTENTrigmode)
	{
		int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addr_len);
		if(connfd < 0)
		{
			LOG_ERROR("%s:errno is:%d", "accept error", errno);
			return false;
		}
		if(my_conn::m_user_count >= MAX_FD)
		{
			LOG_ERROR("%s", "Internal server busy");
			return false;
		}
		addClient(connfd, client_address);
	}
	else
	{
		while(1)
		{
			int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addr_len);
			if (connfd < 0)
			{
				LOG_ERROR("%s:errno is:%d", "accept error", errno);
				return false;
			}
			if (my_conn::m_user_count >= MAX_FD)
			{
				LOG_ERROR("%s", "Internal server busy");
				return false;
			}
			addClient(connfd, client_address);
		}
	}
	return true;
}
void MyServer::dealWithRead(int sockfd)
{
	if(user_boards[sockfd].read_once())
	{
		LOG_INFO("deal with the client(%s), id[%s]", inet_ntoa(user_boards[sockfd].get_address()->sin_addr), user_boards[sockfd].mypro_client_id.c_str());
		m_pool->append_p(user_boards + sockfd);
	}
}

void MyServer::dealWithWrite(int sockfd)
{
	if(user_boards[sockfd].write())
	{
		LOG_INFO("send data to the client(%s), id[%s]", inet_ntoa(user_boards[sockfd].get_address()->sin_addr), user_boards[sockfd].mypro_client_id.c_str());
	}

}




void MyServer::eventLoop()
{
	
	while(1)
	{
		int num = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if(num < 0 && errno != EINTR)
		{
			LOG_ERROR("%s", "epoll failure");
			break;
		}

		for(int i = 0; i < num; i ++)
		{
			int sockfd = events[i].data.fd;
			//处理新到的客户链接
			if(sockfd == m_listenfd)
			{
				bool flag = dealClinetData();
				if(false == flag)
					continue;
			}
			else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				//服务器端关闭连接
				user_boards[sockfd].close_conn(1);
			}

			//处理客户连接上接收到的数据
			else if(events[i].events & EPOLLIN)
			{
				dealWithRead(sockfd);
			}
			else if(events[i].events & EPOLLOUT)
			{
				dealWithWrite(sockfd);
			}
		}
			
	}
}


