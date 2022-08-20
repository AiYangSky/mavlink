/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-18 21:57:09
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-20 19:31:30
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Mission.h
 */

#ifndef MAVLINK_MISSION_H
#define MAVLINK_MISSION_H

#include "common/common.h"

#define MAX_RETRY 5

typedef enum
{
    MAVLINK_MISSION_STATUS_IDLE = 1,
    MAVLINK_MISSION_STATUS_UP_START,
    MAVLINK_MISSION_STATUS_UP_LOADING,
    MAVLINK_MISSION_STATUS_DOWN_START,
    MAVLINK_MISSION_STATUS_DOWN_LOADING,
} MAVLINK_MISSION_STATUS_T;

typedef struct
{
    MAVLINK_MISSION_STATUS_T status;
    unsigned short count;
    unsigned short curr_count;
    unsigned char upload_tar_sys;
    unsigned char upload_tar_comp;
    unsigned char upload_tar_chan;
    MAV_MISSION_TYPE type;

    // Timer
    void *timer;
    void (*Os_Timer_creat)(void *, unsigned char, void *);
    void (*Os_Timer_stop_and_reset)(void *);
    unsigned char (*Os_Timer_curr)(void *);

    // Mission storage
    bool (*Mission_Creat)(unsigned short);
    MAV_MISSION_RESULT (*Mission_Append)(mavlink_mission_item_int_t *);
    unsigned short (*Mission_Count)(void);
    void *(*Mission_View)(unsigned short);
    void (*Mission_update)(void);
    bool (*Mission_check)(unsigned short);
    MAV_MISSION_RESULT (*Mission_Clear)(void);
} MAVLINK_MISSION_CB_T;

void Mavlink_Mission_init(void);
void Mavlink_Mission_init_Timer(void *timer,
                                void (*Os_Timer_creat)(void *, unsigned char, void *),
                                void (*Os_Timer_stop_and_reset)(void *),
                                unsigned char (*Os_Timer_curr)(void *));
void Mavlink_Mission_init_Storage(bool (*Mission_Creat)(unsigned short),
                                  MAV_MISSION_RESULT (*Mission_Append)(mavlink_mission_item_int_t *),
                                  unsigned short (*Mission_Count)(void),
                                  void *(*Mission_View)(unsigned short),
                                  void (*Mission_update)(void),
                                  bool (*Mission_check)(unsigned short),
                                  MAV_MISSION_RESULT (*Mission_Clear)(void));
void Mavlink_Mission_item_reached_send(unsigned short count);

#endif // MAVLINK_MISSING_H
