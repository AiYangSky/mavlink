/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-21 20:27:59
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-25 00:48:14
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Command.c
 */

#include "common/mavlink.h"
#include "common/common.h"
#include "mavlink_route.h"
#include "Mavlink_Command.h"
#include <string.h>

static MAVLINK_COMMAND_CB_T mavlink_command;
static void Mavlink_Command_process(unsigned char in_chan, const mavlink_message_t *msg);

void Mavlink_Command_init(const MAVLINK_COMMAND_CB_T *Control_block)
{
    Mavlink_Register_process(Mavlink_Command_process);
    memset((void *)&mavlink_command, 0, sizeof(MAVLINK_COMMAND_CB_T));
    memcpy(&mavlink_command, Control_block, sizeof(MAVLINK_COMMAND_CB_T));
}

void Mavlink_Command_completed(void)
{
    mavlink_command.long_req_command = 0;
    mavlink_route.Os_Timer_stop_and_reset(mavlink_command.timer);
}

void Mavlink_Command_callback(void)
{
    mavlink_command_ack_t command_ack_temp;

    command_ack_temp.result = MAV_RESULT_IN_PROGRESS;
    command_ack_temp.command = mavlink_command.long_req_command;
    command_ack_temp.target_system = mavlink_command.long_req_sysid;
    command_ack_temp.target_component = mavlink_command.long_req_compid;
    command_ack_temp.progress = mavlink_command.Command_Process();
    command_ack_temp.result_param2 = 0;

    Mavlink_Route_send(mavlink_command.long_req_sysid, mavlink_command.long_req_compid,
                       (void *)&command_ack_temp, mavlink_msg_command_ack_encode_chan);
}

static void Mavlink_Command_rec_int(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_command_int_t command_int_temp;
    mavlink_command_ack_t command_ack_temp;

    mavlink_msg_command_int_decode(msg, &command_int_temp);
    command_ack_temp.result = mavlink_command.Commandint_Load(msg->sysid, msg->compid, &command_int_temp);

    if (command_ack_temp.result == MAV_RESULT_IN_PROGRESS)
    {
        mavlink_command.long_req_command = command_int_temp.command;
        mavlink_command.long_req_sysid = msg->sysid;
        mavlink_command.long_req_compid = msg->compid;
        mavlink_route.Os_Timer_activate(mavlink_command.timer);
    }

    command_ack_temp.command = command_int_temp.command;
    command_ack_temp.target_system = msg->sysid;
    command_ack_temp.target_component = msg->compid;
    command_ack_temp.progress = 0;
    command_ack_temp.result_param2 = 0;

    Mavlink_Route_send(msg->sysid, msg->compid,
                       (void *)&command_ack_temp, mavlink_msg_command_ack_encode_chan);
}

static void Mavlink_Command_rec_long(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_command_long_t command_long_temp;
    mavlink_command_ack_t command_ack_temp;

    mavlink_msg_command_long_decode(msg, &command_long_temp);

    if (command_long_temp.confirmation == 0)
    {
        command_ack_temp.result = mavlink_command.Commandlong_Load(msg->sysid, msg->compid, &command_long_temp);
    }
    else
    {
        command_ack_temp.result = mavlink_command.Commandlong_Append(msg->sysid, msg->compid, &command_long_temp);
    }

    if (command_ack_temp.result == MAV_RESULT_IN_PROGRESS && command_long_temp.confirmation == 0)
    {
        mavlink_command.long_req_command = command_long_temp.command;
        mavlink_command.long_req_sysid = msg->sysid;
        mavlink_command.long_req_compid = msg->compid;
        mavlink_route.Os_Timer_activate(mavlink_command.timer);
    }

    command_ack_temp.command = command_long_temp.command;
    command_ack_temp.target_system = msg->sysid;
    command_ack_temp.target_component = msg->compid;
    command_ack_temp.progress = 0;
    command_ack_temp.result_param2 = 0;

    Mavlink_Route_send(msg->sysid, msg->compid,
                       (void *)&command_ack_temp, mavlink_msg_command_ack_encode_chan);
}

static void Mavlink_Command_rec_cancel(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_command_cancel_t command_cancel_temp;
    mavlink_command_ack_t command_ack_temp;

    mavlink_msg_command_cancel_decode(msg, &command_cancel_temp);

    command_ack_temp.result = mavlink_command.Command_Cancel();
    command_ack_temp.command = command_cancel_temp.command;
    command_ack_temp.progress = 0;
    command_ack_temp.result_param2 = 0;
    command_ack_temp.target_system = msg->sysid;
    command_ack_temp.target_component = msg->compid;

    Mavlink_Command_completed();

    Mavlink_Route_send(msg->sysid, msg->compid,
                       (void *)&command_ack_temp, mavlink_msg_command_ack_encode_chan);
}

static void Mavlink_Command_process(unsigned char in_chan, const mavlink_message_t *msg)
{
    switch (msg->msgid)
    {
    case MAVLINK_MSG_ID_COMMAND_INT:
        Mavlink_Command_rec_int(in_chan, msg);
        break;
    case MAVLINK_MSG_ID_COMMAND_LONG:
        Mavlink_Command_rec_long(in_chan, msg);
        break;
    case MAVLINK_MSG_ID_COMMAND_CANCEL:
        Mavlink_Command_rec_cancel(in_chan, msg);
        break;

    default:
        break;
    }
}
