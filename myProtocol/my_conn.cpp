#include "my_conn.h"


#include <fstream>

//locker m_lock;
map<string, void *> m_clientId_map;


//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
		//event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//从内核时间表删除描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        //event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
		event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int my_conn::m_user_count = 0;
int my_conn::m_epollfd = -1;


//关闭连接，关闭一个连接，客户总量减一
void my_conn::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close %d\n", m_sockfd);
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}
//初始化连接,外部调用初始化套接字地址
void my_conn::init(int sockfd, const sockaddr_in &addr, int TRIGMode)
{
    m_sockfd = sockfd;
    m_address = addr;

	addfd(m_epollfd, sockfd, true, m_TRIGMode);
    m_user_count++;
	
    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    m_TRIGMode = TRIGMode;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
	m_state = 0;
	remain_len = 0;
	m_close_log = 0;

	
    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
	

}
bool my_conn::read_once()
{
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;
	
    //LT读取数据
    if (0 == m_TRIGMode)
    {
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        return true;
    }
    //ET读数据
    else
    {
        while (true)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}
bool my_conn::write()
{
	write_queue_locker.lock();
	while(!m_write_queue.empty())
	{
		char *tmp = m_write_queue.front();
		send(m_sockfd, tmp, sizeof(*tmp), 0);
		m_write_queue.pop_front();
		//如果转发多个，需要记录剩余状态，进行删除。
		delete tmp;
	}
	write_queue_locker.unlock();
	m_read_idx = 0;
	m_checked_idx = 0;
	remain_len = 0;
	modfd( m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);

	return true;
}

/**解析报文，解析出控制报文的类型和剩余长度,剩余长度字节固定为2，范围0~65535**/
int my_conn::mypro_read_packet(uint8_t *packet_type)
{
	if(m_read_idx <= 3)
		RETURN_ERROR(M_BUFFER_TOO_SHORT_ERROR);
	*packet_type = (*m_read_buf & 0xF0) >> 4;
	remain_len = (uint16_t)(*(m_read_buf + 1));

	RETURN_ERROR(M_SUCCESS_ERROR);
}

/**接收发布报文, 转发报文给订阅该报文的客户端**/
int my_conn::mypro_publish_packet_handle()
{
	
	if(!m_publish_list.empty())
	{
		char *publish_buf = (char *)malloc(m_read_idx);
		std::list<my_conn *>::iterator it;
 		for(it = m_publish_list.begin(); it != m_publish_list.end(); it ++)
 		{
  			my_conn *tmp = *it;
			tmp->add_write_queue(publish_buf);
 		}
	}
	else
	{
		LOG_INFO("id[%s] no be subscribed",mypro_client_id.c_str());
	}
	RETURN_ERROR(M_SUCCESS_ERROR);
}

/**接收订阅报文，将订阅信息插入相应列表**/
void my_conn::mypro_subscribe_packet_handle()
{
	uint8_t id_len = (uint8_t)(*(m_read_buf + 3));
	//MAP中根据id找到对应my_conn
	string tmp_id(m_read_buf + 4, (size_t)id_len);
	std::map<string, void *>::iterator it;
	it = m_clientId_map.find(tmp_id);
	if(it == m_clientId_map.end())
	{
		LOG_INFO("subscribe no match tmp_id[%s]",tmp_id.c_str());
	}
	else
	{
		my_conn *tmp_conn = (my_conn *)(it->second);
		//线程安全吗 
		my_conn *now_conn = this;
		tmp_conn->m_publish_list.push_back(now_conn);
		LOG_INFO("subscribe match, id[%s] subscribe id[%s], insert this 0x%x",mypro_client_id.c_str(), tmp_id.c_str(), this);
	}
}

/**接收连接报文，id和my_conn通过map对应起来**/
void my_conn::mypro_connect_packet_handle()
{
	uint8_t id_len = (uint8_t)(*(m_read_buf + 3));
	string tmp_id(m_read_buf + 4, (size_t)id_len);
	mypro_client_id = tmp_id;
	m_clientId_map[mypro_client_id] = (void *)this;
	LOG_INFO("connect success, socketfd[%d] id[%s] id_len[%d] this[0x%x] mypro_client_id[%s]",m_sockfd, m_read_buf + 4, id_len, this, mypro_client_id.c_str());
}


/**将写报文添加到写队列**/
void my_conn::add_write_queue(char *add_buf)
{
	write_queue_locker.lock();
	m_write_queue.push_back(add_buf);
	write_queue_locker.unlock();
	//激活写操作
	modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
}


void my_conn::process()
{
	m_write_idx = 0;
	uint8_t packet_type = 0;

	int ret = mypro_read_packet(&packet_type);
	
	switch(packet_type)
	{
		case 0:
			if(M_BUFFER_TOO_SHORT_ERROR == ret)
				//LOG_ERROR("M_BUFFER_TOO_SHORT_ERROR");
			break;
		case CONNECT:
			mypro_connect_packet_handle();
			break;
		case PUBLISH:
			ret = mypro_publish_packet_handle();
			break;
		case SUBSCRIBE:
			mypro_subscribe_packet_handle();
			break;
	}
	
	
    modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
}


