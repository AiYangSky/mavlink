/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-21 20:27:20
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-22 01:29:21
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Command.h
 */

#ifndef MAVLINK_COMMAND_H
#define MAVLINK_COMMAND_H

#include "common/common.h"

typedef struct
{
    unsigned short long_time_command;
    unsigned char long_time_sysid;
    unsigned char long_time_compid;
    unsigned char long_time_chan;

    // Timer
    void *timer;
    void (*Os_Timer_creat)(void *, unsigned short, void *);
    void (*Os_Timer_stop_and_reset)(void *);
    unsigned char (*Os_Timer_curr)(void *);

    MAV_RESULT (*Commandint_Load)(mavlink_command_int_t *);
    MAV_RESULT (*Commandlong_Load)(mavlink_command_long_t *);
    MAV_RESULT (*Commandlong_Append)(mavlink_command_long_t *);
    MAV_RESULT (*Command_Cancel)(void);
    unsigned char (*Command_Process)(void);
} MAVLINK_COMMAND_CB_T;

void Mavlink_Command_init(const MAVLINK_COMMAND_CB_T *Control_block);
void Mavlink_Command_completed(void);

#endif