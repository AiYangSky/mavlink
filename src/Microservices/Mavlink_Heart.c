/*
 * @Description    : Mavlink microservices of Heart & Connection
 * @Author         : Aiyangsky
 * @Date           : 2022-08-11 20:01:11
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-18 21:59:48
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Heart.c
 */

#include "minimal/mavlink_msg_heartbeat.h"
#include "common/common.h"
#include "mavlink_route.h"
#include "Mavlink_Heart.h"

__attribute__((weak)) unsigned char Get_Base_mode(void)
{
}

__attribute__((weak)) unsigned char Get_Custom_mode(void)
{
}

__attribute__((weak)) unsigned char Get_SYS_Mode(void)
{
}

void Mavlink_Hreat(unsigned char chan)
{
    static mavlink_heartbeat_t heartbeat;
    mavlink_message_t msg;

    unsigned short msgLength;
    unsigned char msgBuffer[MAVLINK_MSG_ID_0_LEN + 25];

    heartbeat.type = MAV_TYPE_QUADROTOR;
    heartbeat.autopilot = MAV_AUTOPILOT_PX4;

    heartbeat.base_mode = Get_Base_mode();
    heartbeat.custom_mode = Get_Custom_mode();
    heartbeat.system_status = Get_SYS_Mode();

    mavlink_msg_heartbeat_encode_chan(mavlink_route.sysid, mavlink_route.compid, chan, &msg, &heartbeat);
    msgLength = mavlink_msg_to_send_buffer(msgBuffer, &msg);
    mavlink_route.chan_cb[chan].Send_bytes(msgBuffer, msgLength);
}
