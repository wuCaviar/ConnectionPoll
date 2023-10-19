#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

#include "MysqlConn.h"

using namespace std;

class ConnectionPoll
{
public:
    /* 单例模式 */
    static ConnectionPoll* getConnrctPoll();                        // 获取连接池对象
    ConnectionPoll(const ConnectionPoll& obj) = delete;             // 禁止拷贝构造
    ConnectionPoll& operator=(const ConnectionPoll& obj) = delete;  // 禁止赋值构造
    
    shared_ptr<MysqlConn> getConnection();                          // 获取连接
    ~ConnectionPoll();                                              // 析构函数
private:
    ConnectionPoll();                   // 构造函数
    bool parseJsonFile();               // 解析json文件

    void produceConnection();           // 生产者线程
    void recycleConnection();           // 消费者线程
    void addConnection();               // 添加连接

    string m_ip;                        // 数据库ip
    string m_user;                      // 数据库用户名
    string m_passwd;                    // 数据库密码
    string m_dbName;                    // 数据库名
    unsigned short m_port;              // 数据库端口号
    int m_maxConn;                      // 最大连接数
    int m_minConn;                      // 最小连接数
    int m_timeout;                      // 超时时间
    int m_maxIdleTime;                  // 最大空闲时间
    queue<MysqlConn*> m_connectionQ;    // 连接队列
    mutex m_mutexQ;                     // 互斥锁
    condition_variable m_cond;          // 条件变量
};
