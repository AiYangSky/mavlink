/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-12 12:11:31
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-24 16:23:27
 * @FilePath       : \mavlink\src\route\mavlink_route.h
 */

#include "mavlink_types.h"
#include "mavlink_helpers.h"

#ifndef __MAVLINK_ROUTE_H
#define __MAVLINK_ROUTE_H

#define MAX_MAVLINK_ROUTE 16
#define MAX_MAVLINK_PROCESS 50

typedef struct
{
    unsigned char index;
    // Callback function of link the hardware implementation
    bool (*Get_byte)(unsigned char *);
    unsigned short (*Send_bytes)(unsigned char *, unsigned short);
} MAVLINK_ROUTE_CHAN_T;

typedef struct
{
    unsigned char type;
    unsigned char chan;
    unsigned char sysid;
    unsigned char compid;
} MAVLINK_ROUTE_LIST_T;

typedef struct
{
    unsigned char sysid;
    unsigned char compid;

    MAVLINK_ROUTE_CHAN_T chan_cb[MAVLINK_COMM_NUM_BUFFERS];
    
    unsigned char route_nums;
    MAVLINK_ROUTE_LIST_T route_list[MAX_MAVLINK_ROUTE];

    //buffer
    mavlink_status_t status_temp;
    mavlink_message_t message_temp;
    mavlink_message_t tx_message;
    unsigned char buf_temp[MAVLINK_MAX_PACKET_LEN];

    //hock funtions list
    void (*Process[MAX_MAVLINK_PROCESS])(unsigned char, const mavlink_message_t *);

    //mutex
    void *mutex;
    void (*Mutex_Get)(void *);
    void (*Mutex_Put)(void *);

    //timer 
    void (*Os_Timer_creat)(void *, unsigned short, void *);
    void (*Os_Timer_stop_and_reset)(void *);
    unsigned char (*Os_Timer_curr)(void *); 
} MAVLINK_ROUTE_CB_T;

extern MAVLINK_ROUTE_CB_T mavlink_route;

void Mavlink_Route_init(unsigned char sysid, unsigned char compid);
void Mavlink_Chan_Set(unsigned char chan,
                      bool (*Get_byte)(unsigned char *),
                      unsigned short (*Send_bytes)(unsigned char *, unsigned short));
bool Mavlink_Route_send(unsigned char tar_sysid, unsigned char tar_compid, const void *msg,
                        unsigned short (*Pack)(unsigned char, unsigned char, unsigned char, mavlink_message_t *, const void *));
bool Mavlink_Register_process(void (*Process)(unsigned char, const mavlink_message_t *));
void Mavlink_Rec_Handle(void);
void Mavlink_STATUSTEXT_send(MAV_SEVERITY status, unsigned short id, char *str);

#endif //__MAVLINK_ROUTE_H