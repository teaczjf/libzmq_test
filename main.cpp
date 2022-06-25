#include <stdio.h>

#include <iostream>
#include "my_easylogging.h"
#include "normal_typedef.h"
#include "zmq.h"

using namespace std;
#define MAX_COM_NUM 5
typedef struct zmq_init_param
{
    UINT32 id;
    string ipaddress;
    UINT16 port;
    string role;
    string topic;
};

//线程处理函数
void print()
{
    usleep(1000 * 1000);
    LOG(INFO) << "子线程运行喽........";
}
#if 1
class server
{
private:
    zmq_init_param m_pub_param;
    zmq_init_param m_sub_param;

    /// ZMQ used for internal thread comunication
    void *m_zmq_context[MAX_COM_NUM];
    void *m_zmq_socket[MAX_COM_NUM];

    /// poll data
    zmq_pollitem_t m_poll_items[MAX_COM_NUM];

    /// server ZMQ pub & sub  for service
    void *m_zmq_server_pub;
    void *m_zmq_server_pub_socket;
    void *m_zmq_server_sub;
    void *m_zmq_server_sub_socket;

public:
    server(/* args */);
    ~server();
    static void *server_handle(void *pthis);
    static void *server_handle2(void *pthis);

    thread *m_ph_server;
    thread *m_ph_server2;

    UINT8 InitInternalThreadCommunication();
    server *m_pObj;
};

server::server(/* args */)
{
    /// Init zmq pub
    m_pub_param.id = 1;
    m_pub_param.ipaddress = "127.0.0.1";
    m_pub_param.port = 6666;
    m_pub_param.role = "pub";
    m_pub_param.topic = "pub_topic";
    m_pObj = this;
    m_zmq_server_pub = zmq_ctx_new();
    m_zmq_server_pub_socket = zmq_socket(m_zmq_server_pub, ZMQ_PUB);

    char endpoint[20];
    memset(endpoint, 0, 20);
    sprintf((char *)endpoint, "tcp://%s:%d", m_pub_param.ipaddress.data(), m_pub_param.port);
    int ret = zmq_bind(m_zmq_server_pub_socket, (char *)endpoint);

    if (Lret_success != ret)
    {
        LOG(ERROR) << "OdlxCarServer::InitServer>>m_zmq_server_pub_socket zsock_bind error " << endpoint;
    }
    else
    {
        LOG(INFO) << "server  m_zmq_server_pub_socket zsock_bind success " << endpoint;
    }

    /// Init zmq sub
    m_sub_param.id = 2;
    m_sub_param.ipaddress = "127.0.0.1";
    m_sub_param.port = 8888;
    m_sub_param.role = "sub";
    m_sub_param.topic = "client_pub_topic";
    m_zmq_server_sub = zmq_ctx_new();
    m_zmq_server_sub_socket = zmq_socket(m_zmq_server_sub, ZMQ_SUB);
    memset(endpoint, 0, 20);
    sprintf((char *)endpoint, "tcp://%s:%d", m_sub_param.ipaddress.data(), m_sub_param.port);

    ret = zmq_connect(m_zmq_server_sub_socket, (char *)endpoint);
    if (Lret_success != ret)
    {
        LOG(ERROR) << "server  zmq_connect  FALIE " << endpoint;
    }
    else
    {
        LOG(INFO) << "server   zmq_connect success " << endpoint;
    }
    zmq_setsockopt(m_zmq_server_sub_socket, ZMQ_SUBSCRIBE, m_sub_param.topic.data(), m_sub_param.topic.size());
    int water_mark = 0;
    zmq_setsockopt(m_zmq_server_sub_socket, ZMQ_RCVHWM, &water_mark, sizeof(water_mark));

    InitInternalThreadCommunication();
    // m_ph_server = new thread(server_handle, (void *)this);

    m_ph_server2 = new thread(server_handle2, (void *)this);
}

server::~server()
{
}
/**
 * @brief 和device 一对多通讯的
 * @param  pthis            My Param doc
 * @return void*
 */
void *server::server_handle(void *pthis)
{
    if (NULL == pthis)
    {
        return NULL;
    }
    server *pObj = (server *)pthis;
    usleep(1000 * 1000);

    LOG(INFO) << "server_handle线程运行喽........";

    char topic_name[256] = {0};
    char payload[8192] = {0};
    while (TRUE)
    {
        /// using poll, not loop
        int cnt = zmq_poll(pObj->m_poll_items, MAX_COM_NUM, -1);
        LOG_EVERY_N(1, INFO) << "TxHandler:::<<<<<<<<<<<<<<<OdlxCarServer::TxHandler 还活着============================ " << endl;

        for (int i = 0; i < MAX_COM_NUM; i++)
        {
            if (pObj->m_poll_items[i].revents & ZMQ_POLLIN)
            {
                int poll_num = i;

                memset(topic_name, 0, sizeof(topic_name));
                memset(payload, 0, sizeof(payload));
                int size = zmq_recv(pObj->m_zmq_socket[i], topic_name, sizeof(topic_name), 0);
                // if (poll_num == 4)
                // {
                LOG_EVERY_N(1, DEBUG) << " size ==" << size << "TxHandler:::<<<<<<<<<<<<<<<解析topic_name " << topic_name;
                // }
                size = zmq_recv(pObj->m_zmq_socket[i], payload, sizeof(payload), 0);
                // if (poll_num == 4)
                // {
                LOG_EVERY_N(1, DEBUG) << " size ==" << size << "TxHandler:::<<<<<<<<<<<<<<<解析payload == " << payload;
                //     continue;
                // }

                /// packet switch , we can save to DB \
      /// use zmq server to send to web devicea
                usleep(1 * 1000); //测试发现这里不给以延时发送会失败。

                // zmq_send(pObj->m_zmq_server_pub_socket, pObj->m_pub_param.topic.data(), pObj->m_pub_param.topic.size(), ZMQ_SNDMORE);
                // int length = zmq_send(pObj->m_zmq_server_pub_socket, payload, size, 0);
            }
        }
    }
}
/**
 * @brief  用来和client 一对一通讯的
 * @param  pthis            My Param doc
 * @return void*
 */
void *server::server_handle2(void *pthis)
{
    if (NULL == pthis)
    {
        return NULL;
    }
    server *pObj = (server *)pthis;
    usleep(1000 * 1000);

    LOG(INFO) << "server_handle2线程运行喽........";

    char topic_name[256] = {0};
    char payload[8192] = {0};
    while (TRUE)
    {

        char topic_name[256] = {0};
        char payload[8192] = {0};
        memset(topic_name, 0, sizeof(topic_name));
        memset(payload, 0, sizeof(payload));
        int size = zmq_recv(pObj->m_zmq_server_sub_socket, topic_name, sizeof(topic_name), 0);
        LOG_EVERY_N(1, DEBUG) << " size ==" << size << "TxHandler:::<<<<<<<<<<<<<<<解析topic_name " << topic_name;

        size = zmq_recv(pObj->m_zmq_server_sub_socket, payload, sizeof(payload), 0);
        LOG_EVERY_N(1, DEBUG) << " size ==" << size << "TxHandler:::<<<<<<<<<<<<<<<解析payload == " << payload;

        // /// using poll, not loop
        // LOG_EVERY_N(1, INFO) << "TxHandler:::<<<<<<<<<<<<<<<OdlxCarServer::TxHandler 还活着============================ " << endl;
    }
}

UINT8 server::InitInternalThreadCommunication()
{
    UINT8 ret = Lret_success;
    int water_mark = 0;
    /// every modbus device need a zmq pub
    /// zmq parameter can read from json
    for (int i = 0; i < MAX_COM_NUM; i++)
    {
        /// Init zmq sub
        m_zmq_context[i] = zmq_ctx_new();
        m_zmq_socket[i] = zmq_socket(m_zmq_context[i], ZMQ_SUB);
        CHAR endpoint[20] = {0};
        sprintf((char *)endpoint, "tcp://%s:%d", "127.0.0.1", 9999 + i);
        int ret = zmq_connect(m_zmq_socket[i], (char *)endpoint);
        zmq_setsockopt(m_zmq_socket[i], ZMQ_SUBSCRIBE, "InitInternalThreadComunication", strlen("InitInternalThreadComunication"));
        zmq_setsockopt(m_zmq_socket[i], ZMQ_RCVHWM, &water_mark, sizeof(water_mark));

        if (Lret_success != ret)
        {
            LOG(ERROR) << "OdlxCarServer::InitInternalThreadCommunication>>ZMQ_SUB zsock_connect error " << endpoint << endl;
            ret = Lret_fail;
        }
        else
        {
            LOG_IF(1, INFO) << "ZMQ_SUB zsock_connect success " << endpoint << endl;
        }

        m_poll_items[i].socket = m_zmq_socket[i];
        m_poll_items[i].fd = 0;
        m_poll_items[i].events = ZMQ_POLLIN;
        m_poll_items[i].revents = 0;
    }

    return ret;
}
#endif
class device
{
private:
    /* data */
private:
    /// ZMQ use for internal thread comunication
    /// modbus collect data pub to server
    void *m_zmq_context;

    void *m_zmq_socket;

public:
    device(UINT8 id);
    ~device();
    static void *device_handle(void *pthis);
    thread *m_ph_device;
    UINT8 InitInternalThreadCommunication();

    zmq_init_param m_pub_param;
    zmq_init_param m_sub_param;
    UINT8 m_modbus_id;
    device *m_pObj;
};

device::device(UINT8 id)
{
    m_modbus_id = id;
    m_pObj = this;

    InitInternalThreadCommunication();
    m_ph_device = new thread(device_handle, (void *)this);
}

device::~device()
{
}

// void device::device_handle(device &pthis)
void *device::device_handle(void *pthis)
{
    if (NULL == pthis)
    {
        return NULL;
    }
    device *pObj = (device *)pthis;

    usleep(1000 * 1000);
    LOG(INFO) << "device_handle线程运行喽........";
    while (1)
    {
        zmq_send(pObj->m_zmq_socket, "InitInternalThreadComunication111111111111111111111", strlen("InitInternalThreadComunication111111111111111111111"), ZMQ_SNDMORE);
        int length = zmq_send(pObj->m_zmq_socket, to_string(pObj->m_modbus_id).data(), 1, 0);
        LOG_EVERY_N(1, INFO) << " <<<<<<<<<<<<<<<device_handle线程  packet  长度== " << length << endl;
        usleep(1000 * 1000);
    }
}

UINT8 device::InitInternalThreadCommunication()
{
    /// Init zmq
    INT32 ret = Lret_success;

    m_zmq_context = zmq_ctx_new();
    m_zmq_socket = zmq_socket(m_zmq_context, ZMQ_PUB);

    int water_mark = 0;
    zmq_setsockopt(m_zmq_socket, ZMQ_SNDHWM, &water_mark, sizeof(water_mark));

    CHAR endpoint[40] = {0};
    sprintf((char *)endpoint, "tcp://%s:%d", "127.0.0.1", 9999 + m_modbus_id);
    ret = zmq_bind(m_zmq_socket, (char *)endpoint);

    if (Lret_success != ret)
    {
        LOG(ERROR) << "OdlxCarModbus::InitInternalThreadCommunication>>ZMQ_PUB zsock_bind error " << endpoint << "m_modbus_id = " << (UINT32)m_modbus_id;

        ret = Lret_fail;
    }
    else
    {
        LOG(INFO) << "ZMQ_PUB zsock_bind success " << endpoint << "m_modbus_id = " << (UINT32)m_modbus_id;
    }

    return ret;
}

class client
{
private:
    /* data */
private:
    /// ZMQ use for internal thread comunication
    /// modbus collect data pub to server
    void *m_zmq_context;

    void *m_zmq_socket;

public:
    client(UINT8 id);
    ~client();
    static void *client_handle(void *pthis);
    thread *m_ph_client;
    UINT8 InitInternalThreadCommunication();

    zmq_init_param m_pub_param;
    zmq_init_param m_sub_param;

    /// client ZMQ pub & sub  for service
    void *m_zmq_client_pub;
    void *m_zmq_client_pub_socket;
    void *m_zmq_client_sub;
    void *m_zmq_client_sub_socket;

    UINT8 m_modbus_id;
    client *m_pObj;
};

client::client(UINT8 id)
{
    /// public
    m_pObj = this;
    m_pub_param.id = 1;
    m_pub_param.ipaddress = "127.0.0.1";
    m_pub_param.port = 8888;
    m_pub_param.role = "pub";
    m_pub_param.topic = "client_pub_topic";
    m_pObj = this;

    m_zmq_client_pub = zmq_ctx_new();
    m_zmq_client_sub = zmq_ctx_new();

    m_zmq_client_pub_socket = NULL;
    m_zmq_client_sub_socket = NULL;

    m_zmq_client_pub_socket = zmq_socket(m_zmq_client_pub, ZMQ_PUB);
    CHAR endpoint[40] = {0};
    sprintf((char *)endpoint, "tcp://%s:%d", m_pub_param.ipaddress.data(), m_pub_param.port);
    int ret = zmq_bind(m_zmq_client_pub_socket, (char *)endpoint);

    if (Lret_success != ret)
    {
        LOG(ERROR) << "client ZMQ_PUB zsock_bind error " << endpoint;
    }
    else
    {
        LOG(INFO) << "clientZMQ_PUB zsock_bind success " << endpoint;
    }

    /// sub

    m_sub_param.id = 2;
    m_sub_param.ipaddress = "127.0.0.1";
    m_sub_param.port = 6666;
    m_sub_param.role = "sub";
    m_sub_param.topic = "sub_topic";
    m_zmq_client_sub_socket = zmq_socket(m_zmq_client_sub, ZMQ_SUB);
    memset(endpoint, 0, 20);
    sprintf((char *)endpoint, "tcp://%s:%d", m_sub_param.ipaddress.data(), m_sub_param.port);

    ret = zmq_connect(m_zmq_client_sub_socket, (char *)endpoint);
    if (Lret_success != ret)
    {
        LOG(ERROR) << "client ZMQ_SUB zmq_connect error " << endpoint;
    }
    else
    {
        LOG(INFO) << "client ZMQ_SUB zmq_connect  success " << endpoint;
    }
    zmq_setsockopt(m_zmq_client_sub_socket, ZMQ_SUBSCRIBE, m_sub_param.topic.data(), m_sub_param.topic.size());
    int water_mark = 0;
    zmq_setsockopt(m_zmq_client_sub_socket, ZMQ_RCVHWM, &water_mark, sizeof(water_mark));

    // InitInternalThreadCommunication();
    m_ph_client = new thread(client_handle, (void *)this);
}

client::~client()
{
}

// void client::client_handle(client &pthis)
/**
 * @brief 与server一对一通讯
 * @param  pthis            My Param doc
 * @return void*
 */
void *client::client_handle(void *pthis)
{
    if (NULL == pthis)
    {
        return NULL;
    }
    client *pObj = (client *)pthis;

    usleep(1000 * 1000);
    LOG(INFO) << "client_handle线程运行喽........";
    while (1)
    {
        zmq_send(pObj->m_zmq_client_pub_socket, "client_pub_topi1c", strlen("client_pub_topi1c"), ZMQ_SNDMORE);
        int length = zmq_send(pObj->m_zmq_client_pub_socket, "this client message", strlen("this client message"), 0);
        LOG_EVERY_N(1, INFO) << " <<<<<<<<<<<<<<<client_handle线程  packet  长度== " << length << endl;
        usleep(1000 * 1000);
    }
}

UINT8 client::InitInternalThreadCommunication()
{
    /// Init zmq
    INT32 ret = Lret_success;

    m_zmq_context = zmq_ctx_new();
    m_zmq_socket = zmq_socket(m_zmq_context, ZMQ_PUB);

    int water_mark = 0;
    zmq_setsockopt(m_zmq_socket, ZMQ_SNDHWM, &water_mark, sizeof(water_mark));

    CHAR endpoint[40] = {0};
    sprintf((char *)endpoint, "tcp://%s:%d", "127.0.0.1", 9999 + m_modbus_id);
    ret = zmq_bind(m_zmq_socket, (char *)endpoint);

    if (Lret_success != ret)
    {
        LOG(ERROR) << "OdlxCarModbus::InitInternalThreadCommunication>>ZMQ_PUB zsock_bind error " << endpoint << "m_modbus_id = " << (UINT32)m_modbus_id;

        ret = Lret_fail;
    }
    else
    {
        LOG(INFO) << "ZMQ_PUB zsock_bind success " << endpoint << "m_modbus_id = " << (UINT32)m_modbus_id;
    }

    return ret;
}
// #define HDDD
int main()
{
    easylogginginit(); //日志初始化
    // log_test();        //日志自测函数   里面可以看细节的用法
    //创建线程
    client client_test(1);
    usleep(1000 * 1000);

    server server_test;
    // device device_test1(1);
    // device device_test2(2);

    // server_test.InitInternalThreadCommunication();
    // device_test.InitInternalThreadCommunication();

    LOG(INFO) << "主线程运行喽.........";
    while (1)
    {
        usleep(10);
    }
    return 0;
}