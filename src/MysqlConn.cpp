#include "MysqlConn.h"

MysqlConn::MysqlConn()
{
    m_conn = mysql_init(NULL);               // 初始化连接
    mysql_set_character_set(m_conn, "utf8"); // 设置编码格式
}

MysqlConn::~MysqlConn()
{
    if (m_conn != NULL)
    {
        mysql_close(m_conn); // 关闭连接
    }
    freeResult(); // 释放结果集
}

bool MysqlConn::connect(string user, string passwd, string dbName, string ip, unsigned short port)
{
    MYSQL *ptr = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, NULL, 0); // 连接数据库

    return ptr != nullptr; // 判断是否连接成功
}

bool MysqlConn::update(string sql)
{
    // 执行sql语句
    if (mysql_query(m_conn, sql.c_str()))
    {
        return false;
    }
    return true;
}

bool MysqlConn::query(string sql)
{
    freeResult(); // 释放结果集
    // 执行sql语句
    if (mysql_query(m_conn, sql.c_str()))
    {
        return false;
    }
    m_result = mysql_store_result(m_conn); // 获取结果集
    return true;
}

bool MysqlConn::next()
{
    if (m_result != nullptr)
    {
        m_row = mysql_fetch_row(m_result); // 遍历结果集
        return m_row != nullptr;
    }

    return false;
}

string MysqlConn::value(int index)
{
    int row = mysql_num_rows(m_result); // 获取结果集中的行数
    if (index >= row || index < 0)
    {
        return string();
    }
    char *val = m_row[index];                                    // 获取字段值
    unsigned long length = mysql_fetch_lengths(m_result)[index]; // 获取字段长度
    return string(val, length);                                  // 返回字段值
}

bool MysqlConn::transaction()
{
    return mysql_autocommit(m_conn, false); // 关闭自动提交事务
}

bool MysqlConn::commit()
{
    return mysql_commit(m_conn); // 提交事务
}

bool MysqlConn::rollback()
{
    return mysql_rollback(m_conn); // 回滚事务
}

void MysqlConn::refreshAliveTime()
{
    m_aliveTime = steady_clock::now(); // 刷新起始的空闲时间点   
}

long long MysqlConn::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_aliveTime; // 计算连接存活的总时长
    milliseconds millsec = duration_cast<milliseconds>(res); // 纳秒转换为毫秒
    return millsec.count(); // 返回毫秒数
}

void MysqlConn::freeResult()
{
    if (m_result)
    {
        mysql_free_result(m_result); // 释放结果集
        m_result = nullptr;
    }
}
