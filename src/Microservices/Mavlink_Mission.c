/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-18 21:57:28
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-25 00:48:23
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Mission.c
 */

#include "common/mavlink.h"
#include "common/common.h"
#include "mavlink_route.h"
#include "Mavlink_Mission.h"
#include <string.h>

static MAVLINK_MISSION_CB_T mavlink_mission;
static void Mavlink_Mission_process(unsigned char in_chan, const mavlink_message_t *msg);

/**                                 Initialize Mission CB
 * @description:
 * @return      {*}
 * @note       :
 */
void Mavlink_Mission_init(const MAVLINK_MISSION_CB_T *Control_block)
{
    Mavlink_Register_process(Mavlink_Mission_process);

    memset(&mavlink_mission, 0, sizeof(MAVLINK_MISSION_CB_T));
    memcpy(&mavlink_mission, Control_block, sizeof(MAVLINK_MISSION_CB_T));

    mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
}

/**
 * @description:                            Broadcast has arrived at a mission point
 * @param       {unsigned short} count      Index of mission
 * @return      {*}
 * @note       :
 */
void Mavlink_Mission_item_reached_send(unsigned short count)
{
    mavlink_mission_item_reached_t mission_item_reached_temp;

    mission_item_reached_temp.seq = count;

    Mavlink_Route_send(0, 0, (void *)&mission_item_reached_temp, mavlink_msg_mission_item_reached_encode_chan);
}

/**
 * @description:                            Request a Mission point from the GCS
 * @param       {unsigned short} count      Index of Mission point
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_req_int_send(unsigned char tar_sysid, unsigned char tar_compid, unsigned short count)
{
    mavlink_mission_request_int_t mission_req_int_temp;

    mission_req_int_temp.mission_type = mavlink_mission.type;
    mission_req_int_temp.seq = count;
    mission_req_int_temp.target_system = tar_sysid;
    mission_req_int_temp.target_component = tar_compid;

    Mavlink_Route_send(tar_compid, tar_compid, (void *)&mission_req_int_temp,
                       mavlink_msg_mission_request_int_encode_chan);
}

/**
 * @description:                                ACK message is returned to the GCS
 * @param       {MAV_MISSION_RESULT} status
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_ack_send(unsigned char tar_sysid, unsigned char tar_compid, MAV_MISSION_RESULT status)
{
    mavlink_mission_ack_t mission_ack_temp;

    mission_ack_temp.type = status;
    mission_ack_temp.mission_type = mavlink_mission.type;

    Mavlink_Route_send(tar_sysid, tar_compid, (void *)&mission_ack_temp, mavlink_msg_mission_ack_encode_chan);
}

/**
 * @description:                            Send mission list point number of items to GCS
 * @param       {unsigned short} count      number of items
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_count_send(unsigned char tar_sysid, unsigned char tar_compid, unsigned short count)
{
    mavlink_mission_count_t mission_count_temp;

    mission_count_temp.count = count;
    mission_count_temp.mission_type = mavlink_mission.type;
    mission_count_temp.target_system = tar_sysid;
    mission_count_temp.target_component = tar_compid;

    Mavlink_Route_send(tar_sysid, tar_compid, (void *)&mission_count_temp,
                       mavlink_msg_mission_count_encode_chan);
}

/**
 * @description:                            Send mission item to GCS
 * @param       {unsigned short} count      Index of items
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_item_send(unsigned char tar_sysid, unsigned char tar_compid, unsigned short count)
{
    mavlink_mission_item_int_t mission_item_temp;

    // Request and fill the waypoint node information
    mavlink_mission.Mission_Get(count, &mission_item_temp);
    mission_item_temp.target_system = tar_sysid;
    mission_item_temp.target_component = tar_compid;

    Mavlink_Route_send(tar_sysid, tar_compid, (void *)&mission_item_temp,
                       mavlink_msg_mission_item_int_encode_chan);
}

/**
 * @description:                            Broadcast currently executing mission point has changed
 * @param       {unsigned short} count      Index by changed
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_curr_send(unsigned short count)
{
    mavlink_mission_current_t mission_current_temp;

    mission_current_temp.seq = count;

    Mavlink_Route_send(0, 0, (void *)&mission_current_temp,
                       mavlink_msg_mission_current_encode_chan);
}

/**
 * @description: 
 * @return      {*}
 * @note       : 
 */
void Mavlink_Mission_timerout_callback(void)
{
    if (mavlink_mission.retry_count > MAX_RETRY)
    {
        mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
        mavlink_mission.retry_count = 0;
        mavlink_route.Os_Timer_stop_and_reset(mavlink_mission.timer);
    }
    else
    {
        if (mavlink_mission.status == MAVLINK_MISSION_STATUS_UP_LOADING)
        {
            Mavlink_Mission_req_int_send(mavlink_mission.retry_tar_sys, mavlink_mission.retry_tar_comp, mavlink_mission.curr_count);
        }
        else if (mavlink_mission.status == MAVLINK_MISSION_STATUS_DOWN_START)
        {
            Mavlink_Mission_count_send(mavlink_mission.retry_tar_sys, mavlink_mission.retry_tar_comp, mavlink_mission.count);
        }
        else if (mavlink_mission.status == MAVLINK_MISSION_STATUS_DOWN_LOADING)
        {
            Mavlink_Mission_item_send(mavlink_mission.retry_tar_sys, mavlink_mission.retry_tar_comp, mavlink_mission.curr_count);
        }
        else
        {
        }
    }
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_count(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_count_t mission_count_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_msg_mission_count_decode(msg, &mission_count_temp);

    // miss列表状态赋值
    mavlink_mission.count = mission_count_temp.count;
    mavlink_mission.type = mission_count_temp.mission_type;
    mavlink_mission.curr_count = 0;
    mavlink_mission.retry_tar_sys = msg->sysid;
    mavlink_mission.retry_tar_comp = msg->compid;

    //创建大小为mavlink_mission.count节点的缓存mission表
    mavlink_mission.Mission_Creat(mavlink_mission.count);

    Mavlink_Mission_req_int_send(msg->sysid, msg->compid, mavlink_mission.curr_count);

    //开启系统定时器 等待超时 或者下一帧ITEM数据到达
    mavlink_mission.retry_count = 0;
    mavlink_route.Os_Timer_activate(mavlink_mission.timer);

    //回传 MAVLINK_MSG_ID_MISSION_REQUEST_INT 帧
    mavlink_mission.status = MAVLINK_MISSION_STATUS_UP_LOADING;
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_item(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_item_int_t mission_item_temp;
    MAV_MISSION_RESULT ret;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_UP_LOADING)
    {
        return;
    }

    //已接收到数据帧 不需要重传或超时
    mavlink_mission.retry_count = 0;
    mavlink_route.Os_Timer_stop_and_reset(mavlink_mission.timer);

    mavlink_msg_mission_item_int_decode(msg, &mission_item_temp);

    if (mission_item_temp.seq == mavlink_mission.curr_count)
    {
        //这里传入的mission节点不一定正确 需要后续重新确认
        ret = mavlink_mission.Mission_Append(&mission_item_temp);

        if (ret == MAV_MISSION_ACCEPTED)
        {
            mavlink_mission.curr_count++;
            if (mavlink_mission.curr_count == mavlink_mission.count)
            {
                //加载缓存的mission列表至正式列表
                mavlink_mission.Mission_Update();
                //完成一次完整的传输
                Mavlink_Mission_ack_send(msg->sysid, msg->compid, MAV_MISSION_ACCEPTED);
                mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
                return;
            }
            else
            {
                Mavlink_Mission_req_int_send(msg->sysid, msg->compid, mavlink_mission.curr_count);
                mavlink_route.Os_Timer_activate(mavlink_mission.timer);
            }
        }
        else
        {
            Mavlink_Mission_ack_send(msg->sysid, msg->compid, ret);
            mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
        }
    }
    else
    {
        //乱序 重新按当前的索引进行请求
        Mavlink_Mission_req_int_send(msg->sysid, msg->compid, mavlink_mission.curr_count);
        mavlink_route.Os_Timer_activate(mavlink_mission.timer);
    }

    mavlink_mission.status = MAVLINK_MISSION_STATUS_UP_LOADING;
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_req_list(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_request_list_t mission_req_list_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE &&
        mavlink_mission.status != MAVLINK_MISSION_STATUS_DOWN_START)
    {
        return;
    }

    mavlink_msg_mission_request_list_decode(msg, &mission_req_list_temp);

    mavlink_mission.type = mission_req_list_temp.mission_type;
    mavlink_mission.retry_tar_sys = msg->sysid;
    mavlink_mission.retry_tar_comp = msg->compid;

    mavlink_mission.count = mavlink_mission.Mission_Count();
    mavlink_mission.curr_count = 0;

    Mavlink_Mission_count_send(msg->sysid, msg->compid, mavlink_mission.count);

    mavlink_mission.retry_count = 0;
    mavlink_route.Os_Timer_activate(mavlink_mission.timer);

    mavlink_mission.status = MAVLINK_MISSION_STATUS_DOWN_START;
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_req_int(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_request_int_t mission_req_int_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_DOWN_START &&
        mavlink_mission.status != MAVLINK_MISSION_STATUS_DOWN_LOADING)
    {
        return;
    }

    mavlink_mission.retry_count = 0;
    mavlink_route.Os_Timer_stop_and_reset(mavlink_mission.timer);

    mavlink_msg_mission_request_int_decode(msg, &mission_req_int_temp);

    mavlink_mission.curr_count = mission_req_int_temp.seq;

    Mavlink_Mission_item_send(msg->sysid, msg->compid, mavlink_mission.curr_count);
    mavlink_route.Os_Timer_activate(mavlink_mission.timer);
    mavlink_mission.status = MAVLINK_MISSION_STATUS_DOWN_LOADING;
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_ack(unsigned char in_chan, const mavlink_message_t *msg)
{
    if (mavlink_mission.status == MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_mission.retry_count = 0;
    mavlink_route.Os_Timer_stop_and_reset(mavlink_mission.timer);

    mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_set_curr(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_set_current_t mission_set_current_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_msg_mission_set_current_decode(msg, &mission_set_current_temp);

    if (mavlink_mission.Mission_Check(mission_set_current_temp.seq))
    {
        Mavlink_Mission_curr_send(mission_set_current_temp.seq);
    }
    else
    {
        Mavlink_STATUSTEXT_send(MAV_SEVERITY_WARNING, 0, "check mission error,maintain the original plan\0");
    }
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_rec_clear(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_clear_all_t mission_clear_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_msg_mission_clear_all_decode(msg, &mission_clear_temp);

    mavlink_mission.retry_count = 0;
    mavlink_route.Os_Timer_stop_and_reset(mavlink_mission.timer);

    Mavlink_Mission_ack_send(msg->sysid, msg->compid, mavlink_mission.Mission_Clear());
}

/**
 * @description:
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_process(unsigned char in_chan, const mavlink_message_t *msg)
{
    switch (msg->msgid)
    {
    // upload msgs
    case MAVLINK_MSG_ID_MISSION_COUNT: //空闲则 开启一次任务传输

        Mavlink_Mission_rec_count(in_chan, msg);
        break;

    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
        //接收一个任务节点 并请求下一个节点 存在超时及重发
        //接收完毕所有节点后 回复ack
        Mavlink_Mission_rec_item(in_chan, msg);
        break;

        // download msgs
    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        Mavlink_Mission_rec_req_list(in_chan, msg);
        break;

    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        Mavlink_Mission_rec_req_int(in_chan, msg);
        break;

    case MAVLINK_MSG_ID_MISSION_ACK:
        Mavlink_Mission_rec_ack(in_chan, msg);
        break;

    // set current msg
    case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
        Mavlink_Mission_rec_set_curr(in_chan, msg);
        break;

    // del mission msg
    case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
        Mavlink_Mission_rec_clear(in_chan, msg);
        break;

    default:
        break;
    }
}