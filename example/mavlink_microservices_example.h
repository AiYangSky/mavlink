/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-25 16:18:32
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-27 23:41:52
 * @FilePath       : \mavlink\example\mavlink_microservices_example.h
 */

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>

extern "C"
{
#include "Mavlink_Mission.h"
#include "mavlink_route.h"
#include "Mavlink_Heart.h"
}

class Route : public QObject
{
    Q_OBJECT
private:
private slots:
    void heart_send(void);
    void TcpnewConnect(void);
    void Tcprec(void);
    void recv_handle(void);
    void Params_handle(void);
    void Mission_handle(void);
    void Command_handle(void);

public:
    QTcpSocket *tcp_qgc;

private:
    QTimer *heart;
    QTimer *rec_handle;
    QTimer *Params;
    QTimer *Mission;
    QTimer *Command;
    QTcpServer *tcp_server;

public:
    explicit Route(QObject *parent = nullptr);
    ~Route();
};
