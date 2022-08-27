/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-21 20:27:20
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-25 19:04:06
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Command.h
 */

#ifndef MAVLINK_COMMAND_H
#define MAVLINK_COMMAND_H

#include "common/mavlink.h"
#include "common/common.h"

    typedef struct
    {
        unsigned short long_req_command;
        unsigned char long_req_sysid;
        unsigned char long_req_compid;

        // Timer
        void *timer;

        MAV_RESULT (*Commandint_Load)(unsigned char, unsigned char, mavlink_command_int_t *);
        MAV_RESULT (*Commandlong_Load)(unsigned char, unsigned char, mavlink_command_long_t *);
        MAV_RESULT (*Commandlong_Append)(unsigned char, unsigned char, mavlink_command_long_t *);
        MAV_RESULT (*Command_Cancel)(void);
        unsigned char (*Command_Process)(void);
    } MAVLINK_COMMAND_CB_T;

    void Mavlink_Command_init(const MAVLINK_COMMAND_CB_T *Control_block);
    void Mavlink_Command_completed(void);
    void Mavlink_Command_callback(void);

#endif
