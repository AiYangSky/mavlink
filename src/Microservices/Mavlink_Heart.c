/*
 * @Description    : Mavlink microservices of Heart & Connection
 * @Author         : Aiyangsky
 * @Date           : 2022-08-11 20:01:11
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-24 16:04:20
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Heart.c
 */

#include "common/mavlink.h"
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

    heartbeat.type = MAV_TYPE_QUADROTOR;
    heartbeat.autopilot = MAV_AUTOPILOT_PX4;

    heartbeat.base_mode = Get_Base_mode();
    heartbeat.custom_mode = Get_Custom_mode();
    heartbeat.system_status = Get_SYS_Mode();

    Mavlink_Route_send(0, 0, (void *)&heartbeat, mavlink_msg_heartbeat_encode_chan);
}
