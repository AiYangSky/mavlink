/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-22 01:37:26
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-22 01:51:20
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Joystick.c
 */

#include "common/mavlink.h"
#include "common/common.h"
#include "mavlink_route.h"
#include "Mavlink_Joystick.h"

static void (*Joystick_Send)(mavlink_manual_control_t *);
static void Mavlink_Joystick_process(unsigned char in_chan, const mavlink_message_t *msg);

void Mavlink_Joystick_init(void (*joystick_Send)(mavlink_manual_control_t *))
{
    Joystick_Send = joystick_Send;
    Mavlink_Register_process(Mavlink_Joystick_process);
}

static void Mavlink_Joystick_rec(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_manual_control_t manusl_temp;

    mavlink_msg_manual_control_decode(msg, &manusl_temp);
    Joystick_Send(&manusl_temp);
}

static void Mavlink_Joystick_process(unsigned char in_chan, const mavlink_message_t *msg)
{
    switch (msg->sysid)
    {
    case MAVLINK_MSG_ID_MANUAL_CONTROL:
        Mavlink_Joystick_rec(in_chan, msg);
        break;

    default:
        break;
    }
}
