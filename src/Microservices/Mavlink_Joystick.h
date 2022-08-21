/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-22 01:37:50
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-22 01:51:36
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Joystick.h
 */

#ifndef MAVLINK_JOYSTICK_H
#define MAVLINK_JOYSTICK_H

#include "common/mavlink.h"
#include "common/common.h"

void Mavlink_Joystick_init(void (*joystick_Send)(mavlink_manual_control_t *));

#endif