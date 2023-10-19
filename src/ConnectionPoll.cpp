#include "ConnectionPoll.h"
#include "jsoncpp/json.h"
#include <fstream>
#include <thread>

using namespace Json;

ConnectionPoll *ConnectionPoll::getConnrctPoll()
{
    static ConnectionPoll poll; // 静态局部变量，程序结束时会自动释放
    return &poll;               // 返回连接池对象
}

shared_ptr<MysqlConn> ConnectionPoll::getConnection()
{
    unique_lock<mutex> locker(m_mutexQ); // 加锁
    // 如果连接池里的连接数大于等于最大连接数，就等待
    while (m_connectionQ.empty()){
        // 等待
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout))){
            if (m_connectionQ.empty()){
                // return nullptr;
                continue; // 继续等待
            }
        }
    }

    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn){
        lock_guard<mutex> locker(m_mutexQ); // 加锁
        conn->refreshAliveTime();           // 刷新起始的空闲时间点
        m_connectionQ.push(conn);           // 添加连接
    }); // 获取连接

    m_connectionQ.pop(); // 弹出连接
    m_cond.notify_all(); // 唤醒所有等待的线程
    return connptr;
}

ConnectionPoll::~ConnectionPoll()
{
    while (!m_connectionQ.empty()){
        MysqlConn *conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

ConnectionPoll::ConnectionPoll()
{
    // 加载json文件
    if (!parseJsonFile()){
        // cout << "parse json file failed!" << endl;
        return;
    }

    for (int i = 0; i < m_minConn; i++){
        addConnection();
    }

    // 创建第一个线程,生成连接池里的连接
    // this指针指向当前对象
    thread producer(&ConnectionPoll::produceConnection, this);

    // 创建第二个线程,销毁连接池里的连接
    thread recycler(&ConnectionPoll::recycleConnection, this);

    // 等待线程结束 分离线程
    producer.detach();
    recycler.detach();
}

bool ConnectionPoll::parseJsonFile()
{
    ifstream ifs("dbconf.json"); // 打开json文件
    Reader reader;               // 创建json解析对象
    Value root;                  // 创建json根节点
    reader.parse(ifs, root);     // 解析json文件
    if (root.isObject()){
        m_ip = root["ip"].asString();               // 获取ip
        m_user = root["username"].asString();           // 获取用户名
        m_passwd = root["passwd"].asString();       // 获取密码
        m_dbName = root["dbName"].asString();       // 获取数据库名
        m_port = root["port"].asInt();              // 获取端口号
        m_maxConn = root["maxConn"].asInt();        // 获取最大连接数
        m_minConn = root["minConn"].asInt();        // 获取最小连接数
        m_timeout = root["timeout"].asInt();        // 获取超时时间
        m_maxIdleTime = root["maxIdleTime"].asInt();// 获取最大空闲时间
        return true;
    }
    return false;
}

void ConnectionPoll::produceConnection()
{
    while (true){
        unique_lock<mutex> locker(m_mutexQ); // 加锁
        // 如果连接池里的连接数大于等于最大连接数，就等待
        while (m_connectionQ.size() >= m_minConn)
        {
            m_cond.wait(locker); // 等待
        }
        addConnection(); // 添加连接
        m_cond.notify_all(); // 唤醒所有消费者线程
    }
}

void ConnectionPoll::recycleConnection()
{
    while (true){
        this_thread::sleep_for(chrono::seconds(1)); // 休眠1秒
        // this_thread::sleep_for(chrono::milliseconds(500)); // 休眠500毫秒 1000毫秒等于1秒
        unique_lock<mutex> locker(m_mutexQ); // 加锁
        while (m_connectionQ.size() > m_minConn)
        {
            MysqlConn *conn = m_connectionQ.front();
            if (conn->getAliveTime() >= m_maxIdleTime)
            {
                m_connectionQ.pop();
                delete conn;
            }
            else
            {
                break;
            }         
        }
    }
}

void ConnectionPoll::addConnection()
{
    MysqlConn *conn = new MysqlConn;
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
    conn->refreshAliveTime();
    m_connectionQ.push(conn);
}
