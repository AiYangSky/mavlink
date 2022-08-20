/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-12 12:14:54
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-18 22:02:19
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Heart.h
 */

#ifndef MAVLINK_HEART_H
#define MAVLINK_HEART_H

void Mavlink_Hreat(unsigned char chan);

__attribute__((weak)) unsigned char Get_SYS_Mode(void);
__attribute__((weak)) unsigned char Get_Custom_mode(void);
__attribute__((weak)) unsigned char Get_Base_mode(void);



#endif //

