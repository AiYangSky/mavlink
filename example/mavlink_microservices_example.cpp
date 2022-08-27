/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-11 19:29:53
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-28 00:30:13
 * @FilePath       : \mavlink\example\mavlink_microservices_example.cpp
 */

#include <QCoreApplication>

#include <QObject>
#include <QDebug.h>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include <QTime>

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern "C"
{
#include "Mavlink_Mission.h"
#include "mavlink_route.h"
#include "Mavlink_Heart.h"
#include "Mavlink_Parameters.h"
#include "Parameters.h"
#include "Mavlink_Mission.h"
#include "Mavlink_Command.h"
}

#include "mavlink_microservices_example.h"

//接收缓冲区 用于和qgc交互
QQueue<unsigned char> mavlink_buffer;

Route *test;

unsigned short mavlink_send(unsigned char chan, unsigned char *data, unsigned short len)
{
    if (chan == 0)
    {
        if (test->tcp_qgc != NULL)
        {
            test->tcp_qgc->write((const char *)data, len);
        }
    }
}

unsigned short mavlink_send_chan1(unsigned char *data, unsigned short len)
{
    return mavlink_send(0, data, len);
}

unsigned short mavlink_send_chann(unsigned char *data, unsigned short len)
{
    return len;
}

bool mavlin_get_byte(unsigned char chan, unsigned char *data)
{
    bool ret = false;

    if (mavlink_buffer.size() != 0)
    {
        ret = true;
        *data = mavlink_buffer.dequeue();
    }

    return ret;
}

bool mavlin_get_byte_chan1(unsigned char *data)
{
    return mavlin_get_byte(0, data);
}

bool mavlin_get_byte_chann(unsigned char *data)
{
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    test = new Route;

    return a.exec();
}

void Timer_activate(void *timer)
{
    ((QTimer *)timer)->start(250);
}

void Timer_stop_and_reset(void *timer)
{
    ((QTimer *)timer)->stop();
}

QMutex mutex;
void mutex_get(void *mutex)
{
    ((QMutex *)mutex)->lock();
}

void mutex_put(void *mutex)
{
    ((QMutex *)mutex)->unlock();
}

void Route::heart_send(void)
{
    Mavlink_Hreat();
}

void Route::Params_handle(void)
{
    Mavlink_Parameters_callback();
}

PARAMETERS_CB_T parameters_test;
unsigned char simulation_ROM[1024];
unsigned char simulation_RAM[1024];

unsigned short checksum(unsigned char *data, unsigned int size)
{
    unsigned short Sum;
    while (size)
    {
        Sum += *data;
        data++;
        size--;
    }
    return Sum;
}

bool Read_From_ROM(unsigned char *dst, unsigned int offset, unsigned int size)
{
    memcpy(dst, simulation_ROM + offset, size);

    return true;
}

bool Write_2_ROM(unsigned char *src, unsigned int offset, unsigned int size)
{
    memcpy(simulation_ROM + offset, src, size);

    return true;
}

unsigned short Param_list_number(void)
{
    return parameters_test.table_info.used_number;
}

unsigned char Param_list_by_index(unsigned short index, char *name, void *value)
{
    return Parameters_Get_by_index(&parameters_test, index, name, value);
}

unsigned char Param_Get_by_name(char *name, unsigned short *index, void *value)
{
    return Parameters_Get_by_name(&parameters_test, name, index, value);
}

void *Param_Chanege(char *name, unsigned char type, void *value)
{
    return Parameters_Chanege(&parameters_test, name, (PARAMETERS_TYPE_T)type, value);
}

QList<mavlink_mission_item_int_t> mission_list;
QList<mavlink_mission_item_int_t> mission_list_temp;

bool Mission_Creat(unsigned short count)
{
    mission_list_temp.clear();
    return true;
}

MAV_MISSION_RESULT Mission_Append(mavlink_mission_item_int_t *node)
{
    MAV_MISSION_RESULT ret = MAV_MISSION_ACCEPTED;
    mission_list_temp.append(*node);
    return ret;
}

unsigned short Mission_Count(void)
{
    return mission_list.size();
}

void Mission_Get(unsigned short index, mavlink_mission_item_int_t *node)
{
    *node = mission_list.at(index);
}

void Mission_Update(void)
{
    mission_list.clear();
    for (unsigned short i = 0; i < mission_list_temp.size(); i++)
    {
        mission_list.append(mission_list_temp.at(i));
    }
}

bool Mission_Check(unsigned short index)
{
    return true;
}

MAV_MISSION_RESULT Mission_Clear(void)
{
    mission_list.clear();
    return MAV_MISSION_ACCEPTED;
}

void Route::Mission_handle(void)
{
    Mavlink_Mission_timerout_callback();
}

void Route::Command_handle(void)
{
    Mavlink_Command_callback();
}

MAV_RESULT Commandint_Load(unsigned char sysid, unsigned char compid, mavlink_command_int_t *msg)
{
    return MAV_RESULT_ACCEPTED;
}

MAV_RESULT Commandlong_Load(unsigned char sysid, unsigned char compid, mavlink_command_long_t *msg)
{
    return MAV_RESULT_ACCEPTED;
}

MAV_RESULT Commandlong_Append(unsigned char sysid, unsigned char compid, mavlink_command_long_t *msg)
{
    return MAV_RESULT_ACCEPTED;
}

MAV_RESULT Command_Cancel(void)
{
    return MAV_RESULT_ACCEPTED;
}

unsigned char Command_Process(void)
{
    return 255;
}

Route::Route(QObject *parent) : QObject(parent)
{
    // route 部分初始化
    Mavlink_Route_init(1, 1);
    Mavlink_Route_Chan_Set(0, mavlin_get_byte_chan1, mavlink_send_chan1);
    for (unsigned char i = 1; i < MAVLINK_COMM_NUM_BUFFERS; i++)
    {
        Mavlink_Route_Chan_Set(i, mavlin_get_byte_chann, mavlink_send_chann);
    }
    Mavlink_Route_Mutex_Set(&mutex, mutex_get, mutex_put);
    Mavlink_Route_timer_Set(Timer_activate, Timer_stop_and_reset);

    this->heart = new QTimer;
    this->tcp_server = new QTcpServer;
    this->rec_handle = new QTimer;
    this->Params = new QTimer;
    this->Mission = new QTimer;
    this->Command = new QTimer;

    //参数部分初始化
    MAVLINK_PARAMETERS_CB_T param_init_stru;
    param_init_stru.timer = this->Params;
    param_init_stru.Get_numbers = Param_list_number;
    param_init_stru.Param_Get_by_index = Param_list_by_index;
    param_init_stru.Param_Get_by_name = Param_Get_by_name;
    param_init_stru.Param_Chanege = Param_Chanege;
    Mavlink_Parameters_init(&param_init_stru);

    //参数初始化
    memset(simulation_ROM, EMPTY_BYTE, sizeof(simulation_ROM));
    memset(simulation_RAM, EMPTY_BYTE, sizeof(simulation_RAM));
    Parameters_Init(&parameters_test, "TEST\0", simulation_RAM, 0, 1024,
                    Read_From_ROM, Write_2_ROM, checksum);

    //添加参数
    float value_1 = 1024.0f;
    Parameters_Creat(&parameters_test, "TEST1\0", PARAMETERS_TYPE_F32, (void *)&value_1);
    uint32_t value_2 = 1000;
    Parameters_Creat(&parameters_test, "TEST2\0", PARAMETERS_TYPE_UINT32, (void *)&value_2);
    int32_t value_3 = -2000;
    Parameters_Creat(&parameters_test, "TEST3\0", PARAMETERS_TYPE_INT32, (void *)&value_3);
    int16_t value_4 = -3000;
    Parameters_Creat(&parameters_test, "TEST4\0", PARAMETERS_TYPE_INT16, (void *)&value_4);
    uint16_t value_5 = 4000;
    Parameters_Creat(&parameters_test, "TEST5\0", PARAMETERS_TYPE_UINT16, (void *)&value_5);
    uint8_t value_6 = 6;
    Parameters_Creat(&parameters_test, "TEST6\0", PARAMETERS_TYPE_UINT8, (void *)&value_6);
    int8_t value_7 = -10;
    Parameters_Creat(&parameters_test, "TEST7\0", PARAMETERS_TYPE_INT8, (void *)&value_7);

    //航点部分
    MAVLINK_MISSION_CB_T mission_init_stru;
    mission_init_stru.timer = this->Mission;
    mission_init_stru.Mission_Creat = Mission_Creat;
    mission_init_stru.Mission_Append = Mission_Append;
    mission_init_stru.Mission_Count = Mission_Count;
    mission_init_stru.Mission_Get = Mission_Get;
    mission_init_stru.Mission_Update = Mission_Update;
    mission_init_stru.Mission_Check = Mission_Check;
    mission_init_stru.Mission_Clear = Mission_Clear;
    Mavlink_Mission_init(&mission_init_stru);

    //命令部分
    MAVLINK_COMMAND_CB_T command_init_stru;
    command_init_stru.timer = this->Command;
    command_init_stru.Commandint_Load = Commandint_Load;
    command_init_stru.Commandlong_Load = Commandlong_Load;
    command_init_stru.Commandlong_Append = Commandlong_Append;
    command_init_stru.Command_Cancel = Command_Cancel;
    command_init_stru.Command_Process = Command_Process;
    Mavlink_Command_init(&command_init_stru);

    tcp_server->listen(QHostAddress::Any, 5760);
    //心跳包发送 目前伪造了一个心跳
    connect(this->heart, &QTimer::timeout, this, Route::heart_send);
    //定时调用解析循环 解码并处理mavlink数据帧
    connect(this->rec_handle, &QTimer::timeout, this, Route::recv_handle);
    // QGC通信链接 暂时只维护一个scoket
    connect(this->tcp_server, &QTcpServer::newConnection, this, Route::TcpnewConnect);
    //参数协议的重发栈
    connect(this->Params, &QTimer::timeout, this, Route::Params_handle);
    //航点协议的重发栈
    connect(this->Mission, &QTimer::timeout, this, Route::Mission_handle);
    //命令协议的状态栈
    connect(this->Command, &QTimer::timeout, this, Route::Command_handle);

    //心跳 1HZ
    this->heart->start(1000);
    //解码1ms一次 一次最多1000字节
    this->rec_handle->start(1);

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void Route::TcpnewConnect()
{
    if (tcp_qgc == NULL)
    {
        tcp_qgc = tcp_server->nextPendingConnection();
        connect(this->tcp_qgc, &QTcpSocket::readyRead, this, Route::Tcprec);
        printf("tcp connect!");
    }
}

void Route::Tcprec(void)
{
    QByteArray temp = tcp_qgc->readAll();

    //在vscode环境下有警告 但不影响结果
    // QByteArray 需要这样才可正常按u8使用
    uint8_t Tcp_Data[temp.size()];
    memcpy(Tcp_Data, temp, temp.size());

    for (unsigned short i = 0; i < temp.size(); i++)
    {
        mavlink_buffer.enqueue(Tcp_Data[i]);
    }
}

void Route::recv_handle()
{
    unsigned short i = 1000;
    while (i--)
    {
        Mavlink_Rec_Handle();
    }
}

Route::~Route()
{
}
