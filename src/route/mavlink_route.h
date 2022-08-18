/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-12 12:11:31
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-18 18:43:01
 * @FilePath       : \mavlink\src\route\mavlink_route.h
 */

#include "mavlink_types.h"
#include "mavlink_helpers.h"

#define MAX_MAVLINK_ROUTE 16
#define MAX_MAVLINK_PROCESS 256

void Mavlink_Route_init(unsigned char sysid, unsigned char compid);
void Mavlink_Chan_Set(unsigned char chan,
                      bool (*Get_byte)(unsigned char *),
                      unsigned short (*Send_bytes)(unsigned char *, unsigned short));
bool Mavlink_Register_process(void (*Process)(const mavlink_message_t *));
void Mavlink_Rec_Handle(void);