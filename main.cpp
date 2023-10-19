#include <iostream>
#include <memory>
#include <thread>

#include "MysqlConn.h"
#include "ConnectionPoll.h"

using namespace std;

// 单线程测试：使用/不使用连接池
// 多线程测试：使用/不使用连接池

void op1(int begin, int end){
    for (int i = begin; i < end; i++){
        MysqlConn conn;
        conn.connect("root", "7464736", "test", "192.168.3.7");
        char sql[1024] = {0};
        sprintf(sql, "insert into user values(%d, 'zhangsan', 20)", i);
        conn.update(sql);
    }
}

void op2(ConnectionPoll *poll, int begin, int end){
    for (int i = begin; i < end; i++){
        shared_ptr<MysqlConn> conn = poll->getConnection();
        char sql[1024] = {0};
        sprintf(sql, "insert into user values(%d, 'zhangsan', 20)", i);
        conn->update(sql);
    }
}

void test1(){
#if 0
    steady_clock::time_point begin = steady_clock::now();
    op1(0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << " 非连接池，单线程，用时： " << length.count() << " 纳秒， " 
        << length.count() / 1000000 << " 毫秒 " << endl;

#else
    ConnectionPoll *poll = ConnectionPoll::getConnrctPoll();
    steady_clock::time_point begin = steady_clock::now();
    op2(poll, 0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << " 非连接池，单线程，用时： " << length.count() << " 纳秒， " 
        << length.count() / 1000000 << " 毫秒 " << endl;

#endif
}

void test2(){
#if 0
    MysqlConn conn;
    conn.connect("root", "7464736", "test", "192.168.3.7");
    steady_clock::time_point begin = steady_clock::now();
    thread t1(op1, 0, 1000);
    thread t2(op1, 1000, 2000);
    thread t3(op1, 2000, 3000);
    thread t4(op1, 3000, 4000);
    thread t5(op1, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << " 非连接池，多线程，用时： " << length.count() << " 纳秒， " 
        << length.count() / 1000000 << " 毫秒 " << endl;

#else
    ConnectionPoll *poll = ConnectionPoll::getConnrctPoll();
    steady_clock::time_point begin = steady_clock::now();
    thread t1(op2, poll, 0, 1000);
    thread t2(op1, poll, 1000, 2000);
    thread t3(op1, poll, 2000, 3000);
    thread t4(op1, poll, 3000, 4000);
    thread t5(op1, poll, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << " 连接池，多线程，用时： " << length.count() << " 纳秒， " 
        << length.count() / 1000000 << " 毫秒 " << endl;

#endif
}

int query(){
    MysqlConn conn;
    conn.connect("root", "7464736", "test", "192.168.3.7");
    string sql = "insert into user values(1, 'zhangsan', 20)";
    bool flag = conn.update(sql);
    cout << "flag value = " << flag << endl;

    sql = "select * from user";
    conn.query(sql);
    while (conn.next()){
        cout << conn.value(0) << ", " << conn.value(1) << ", " << conn.value(2) << endl;
    }
    return 0;
}

int main(){
    // query();
    // test1();
    // test2();
    return 0;
}