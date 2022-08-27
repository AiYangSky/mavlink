/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-18 21:57:09
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-27 19:10:02
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Mission.h
 */

#ifndef MAVLINK_MISSION_H
#define MAVLINK_MISSION_H

#include "common/mavlink.h"
#include "common/common.h"

#define MAX_RETRY 5

    typedef enum
    {
        MAVLINK_MISSION_STATUS_IDLE = 1,
        MAVLINK_MISSION_STATUS_UP_LOADING,
        MAVLINK_MISSION_STATUS_DOWN_START,
        MAVLINK_MISSION_STATUS_DOWN_LOADING,
    } MAVLINK_MISSION_STATUS_T;

    typedef struct
    {
        MAVLINK_MISSION_STATUS_T status;
        unsigned char retry_count;
        unsigned short count;
        unsigned short curr_count;
        unsigned char retry_tar_sys;
        unsigned char retry_tar_comp;
        MAV_MISSION_TYPE type;

        // Timer
        void *timer;

        // Mission storage
        bool (*Mission_Creat)(unsigned short);
        MAV_MISSION_RESULT (*Mission_Append)(mavlink_mission_item_int_t *);
        unsigned short (*Mission_Count)(void);
        void (*Mission_Get)(unsigned short, mavlink_mission_item_int_t *);
        void (*Mission_Update)(void);
        bool (*Mission_Check)(unsigned short);
        MAV_MISSION_RESULT (*Mission_Clear)(void);
    } MAVLINK_MISSION_CB_T;

    void Mavlink_Mission_init(const MAVLINK_MISSION_CB_T *Control_block);
    void Mavlink_Mission_item_reached_send(unsigned short count);
    void Mavlink_Mission_timerout_callback(void);

#endif // MAVLINK_MISSING_H
