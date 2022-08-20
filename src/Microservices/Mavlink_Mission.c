/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-18 21:57:28
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-20 19:31:27
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Mission.c
 */

#include "common/common.h"
#include "mavlink_route.h"
#include "Mavlink_Mission.h"
#include <string.h>

//任务列表应该存在双缓存机制

static MAVLINK_MISSION_CB_T mavlink_mission;
static void Mavlink_Mission_process(unsigned char in_chan, const mavlink_message_t *msg);

/**                                 Initialize Mission CB
 * @description:
 * @return      {*}
 * @note       :                    Call to order
 *                                  {
 *                                      Mavlink_Mission_init();
 *                                      Mavlink_Mission_init_Timer(xxx);
 *                                      Mavlink_Mission_init_Storage(xxx);
 *                                  }
 *                                  The initialization must be done as per flow, or the program will crash.
 */
void Mavlink_Mission_init(void)
{
    Mavlink_Register_process(Mavlink_Mission_process);

    memset(&mavlink_mission, 0, sizeof(MAVLINK_MISSION_CB_T));

    mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
}

/**
 * @description:
 * @param       {void *}            timer   Handle of timer
 * @return      {*}
 * @note       :
 *                                  funtions:
 *                                  void Os_Timer_creat(void *timer, unsigned char out_time, void *callback),
 *                                  void Os_Timer_stop_and_reset(void *timer),
 *                                  unsigned char Os_Timer_curr(void *timer), //Gets the current number of timing cycles
 */
void Mavlink_Mission_init_Timer(void *timer,
                                void (*Os_Timer_creat)(void *, unsigned char, void *),
                                void (*Os_Timer_stop_and_reset)(void *),
                                unsigned char (*Os_Timer_curr)(void *))
{
    mavlink_mission.timer = timer;
    mavlink_mission.Os_Timer_creat = Os_Timer_creat;
    mavlink_mission.Os_Timer_stop_and_reset = Os_Timer_stop_and_reset;
    mavlink_mission.Os_Timer_curr = Os_Timer_curr;
}

/**
 * @description:
 * @return      {*}
 * @note       :                    funtions:
 *                                  bool Mission_Creat(unsigned short number),
 *                                  MAV_MISSION_RESULT Mission_Append(mavlink_mission_item_int_t *mission),
 *                                  unsigned short Mission_Count(void),             //Gets the number of items in the current Mission table
 *                                  void Mission_update(void),                      //Update the mission table in the cache to the current point
 *                                  bool Mission_check(unsigned short),             //Checkout the currently executing mission point
 *                                  MAV_MISSION_RESULT Mission_Clear(void),         //clear current mission table
 */
void Mavlink_Mission_init_Storage(bool (*Mission_Creat)(unsigned short),
                                  MAV_MISSION_RESULT (*Mission_Append)(mavlink_mission_item_int_t *),
                                  unsigned short (*Mission_Count)(void),
                                  void *(*Mission_View)(unsigned short),
                                  void (*Mission_update)(void),
                                  bool (*Mission_check)(unsigned short),
                                  MAV_MISSION_RESULT (*Mission_Clear)(void))
{
    mavlink_mission.Mission_Creat = Mission_Creat;
    mavlink_mission.Mission_Append = Mission_Append;
    mavlink_mission.Mission_Count = Mission_Count;
    mavlink_mission.Mission_View = Mission_View;
    mavlink_mission.Mission_update = Mission_update;
    mavlink_mission.Mission_check = Mission_check;
    mavlink_mission.Mission_Clear = Mission_Clear;
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
    unsigned short len_temp;
    unsigned char i;

    mission_item_reached_temp.seq = count;

    for (i = 0; i < MAVLINK_COMM_NUM_BUFFERS; i++)
    {
        mavlink_msg_mission_current_encode_chan(mavlink_route.sysid, mavlink_route.compid, i,
                                                &mavlink_route.tx_message, &mission_item_reached_temp);

        len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);

        mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
    }
}

/**
 * @description:                            Request a Mission point from the GCS
 * @param       {unsigned short} count      Index of Mission point
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_req_int_send(unsigned short count)
{
    mavlink_mission_request_int_t mission_req_int_temp;
    unsigned short len_temp;

    mission_req_int_temp.mission_type = mavlink_mission.type;
    mission_req_int_temp.seq = count;
    mission_req_int_temp.target_system = mavlink_mission.upload_tar_sys;
    mission_req_int_temp.target_component = mavlink_mission.upload_tar_comp;
    mavlink_msg_mission_request_int_encode_chan(mavlink_route.sysid, mavlink_route.compid, mavlink_mission.upload_tar_chan,
                                                &mavlink_route.tx_message, &mission_req_int_temp);

    len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);

    mavlink_route.chan_cb[mavlink_mission.upload_tar_chan].Send_bytes(mavlink_route.buf_temp, len_temp);
}

/**
 * @description:                                ACK message is returned to the GCS
 * @param       {MAV_MISSION_RESULT} status
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_ack_send(MAV_MISSION_RESULT status)
{
    mavlink_mission_ack_t mission_ack_temp;
    unsigned short len_temp;

    mission_ack_temp.type = status;
    mission_ack_temp.mission_type = mavlink_mission.type;

    mavlink_msg_mission_ack_encode_chan(mavlink_route.sysid, mavlink_route.compid, mavlink_mission.upload_tar_chan,
                                        &mavlink_route.tx_message, &mission_ack_temp);

    len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);

    mavlink_route.chan_cb[mavlink_mission.upload_tar_chan].Send_bytes(mavlink_route.buf_temp, len_temp);
}

/**
 * @description:                            Send mission list point number of items to GCS
 * @param       {unsigned short} count      number of items
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_count_send(unsigned short count)
{
    mavlink_mission_count_t mission_count_temp;
    unsigned short len_temp;

    mission_count_temp.count = count;
    mission_count_temp.mission_type = mavlink_mission.type;
    mission_count_temp.target_system = mavlink_mission.upload_tar_sys;
    mission_count_temp.target_component = mavlink_mission.upload_tar_comp;

    mavlink_msg_mission_count_encode_chan(mavlink_route.sysid, mavlink_route.compid, mavlink_mission.upload_tar_chan,
                                          &mavlink_route.tx_message, &mission_count_temp);

    len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);

    mavlink_route.chan_cb[mavlink_mission.upload_tar_chan].Send_bytes(mavlink_route.buf_temp, len_temp);
}

/**
 * @description:                            Send mission item to GCS
 * @param       {unsigned short} count      Index of items
 * @return      {*}
 * @note       :
 */
static void Mavlink_Mission_item_send(unsigned short count)
{
    mavlink_mission_item_int_t mission_item_temp;
    unsigned short len_temp;

    mavlink_mission.Mission_View(count);

    //填充mission节点
    mission_item_temp.autocontinue = 1;

    mavlink_msg_mission_item_int_encode_chan(mavlink_route.sysid, mavlink_route.compid, mavlink_mission.upload_tar_chan,
                                             &mavlink_route.tx_message, &mission_item_temp);

    len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);

    mavlink_route.chan_cb[mavlink_mission.upload_tar_chan].Send_bytes(mavlink_route.buf_temp, len_temp);
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
    unsigned short len_temp;
    unsigned char i;

    mission_current_temp.seq = count;

    for (i = 0; i < MAVLINK_COMM_NUM_BUFFERS; i++)
    {
        mavlink_msg_mission_current_encode_chan(mavlink_route.sysid, mavlink_route.compid, i,
                                                &mavlink_route.tx_message, &mission_current_temp);

        len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);

        mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
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

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE &&
        mavlink_mission.status != MAVLINK_MISSION_STATUS_UP_START)
    {
        return;
    }

    mavlink_msg_mission_count_decode(msg, &mission_count_temp);

    // miss列表状态赋值
    mavlink_mission.count = mission_count_temp.count;
    mavlink_mission.type = mission_count_temp.mission_type;
    mavlink_mission.curr_count = 0;
    mavlink_mission.upload_tar_chan = in_chan;
    mavlink_mission.upload_tar_sys = msg->sysid;
    mavlink_mission.upload_tar_comp = msg->compid;

    //创建大小为mavlink_mission.count节点的缓存mission表
    mavlink_mission.Mission_Creat(mavlink_mission.count);

    Mavlink_Mission_req_int_send(mavlink_mission.curr_count);

    //开启系统定时器 等待超时 或者下一帧ITEM数据到达
    mavlink_mission.Os_Timer_creat(mavlink_mission.timer, 1500, Mavlink_Mission_up_timerout_callback);

    //回传 MAVLINK_MSG_ID_MISSION_REQUEST_INT 帧
    mavlink_mission.status = MAVLINK_MISSION_STATUS_UP_START;
}

static void Mavlink_Mission_up_timerout_callback(void)
{
    //获取当前触发定时器触发次数
    unsigned char count_temp = mavlink_mission.Os_Timer_curr(mavlink_mission.timer);

    // retry
    if (count_temp < MAX_RETRY)
    {
        Mavlink_Mission_req_int_send(mavlink_mission.curr_count);
    }
    else
    {
        mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
        //关闭并复位超时计时器
        mavlink_mission.Os_Timer_stop_and_reset(mavlink_mission.timer);
    }
}

static void Mavlink_Mission_rec_item(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_item_int_t mission_item_temp;
    MAV_MISSION_RESULT ret;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_UP_LOADING &&
        mavlink_mission.status != MAVLINK_MISSION_STATUS_UP_START)
    {
        return;
    }

    if (mavlink_mission.upload_tar_chan != in_chan)
    {
        return;
    }

    //已接收到数据帧 不需要重传或超时
    mavlink_mission.Os_Timer_stop_and_reset(mavlink_mission.timer);

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
                mavlink_mission.Mission_update();
                //完成一次完整的传输
                Mavlink_Mission_ack_send(MAV_MISSION_ACCEPTED);
                mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
                return;
            }
            else
            {
                Mavlink_Mission_req_int_send(mavlink_mission.curr_count);
                mavlink_mission.Os_Timer_creat(mavlink_mission.timer, 1500, Mavlink_Mission_up_timerout_callback);
            }
        }
        else
        {
            Mavlink_Mission_ack_send(ret);
            mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
        }
    }
    else
    {
        //乱序 重新按现在的索引进行请求
        Mavlink_Mission_req_int_send(mavlink_mission.curr_count);
        mavlink_mission.Os_Timer_creat(mavlink_mission.timer, 1500, Mavlink_Mission_up_timerout_callback);
    }

    mavlink_mission.status = MAVLINK_MISSION_STATUS_UP_LOADING;
}

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
    mavlink_mission.upload_tar_chan = in_chan;
    mavlink_mission.upload_tar_sys = msg->sysid;
    mavlink_mission.upload_tar_comp = msg->compid;

    mavlink_mission.count = mavlink_mission.Mission_Count();
    mavlink_mission.curr_count = 0;

    Mavlink_Mission_count_send(mavlink_mission.count);

    mavlink_mission.Os_Timer_creat(mavlink_mission.timer, 1500, Mavlink_Mission_down_timerout_callback);

    mavlink_mission.status = MAVLINK_MISSION_STATUS_DOWN_START;
}

static void Mavlink_Mission_rec_req_int(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_request_int_t mission_req_int_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_DOWN_START &&
        mavlink_mission.status != MAVLINK_MISSION_STATUS_DOWN_LOADING)
    {
        return;
    }

    if (mavlink_mission.upload_tar_chan != in_chan)
    {
        return;
    }

    mavlink_mission.Os_Timer_stop_and_reset(mavlink_mission.timer);

    mavlink_msg_mission_request_int_decode(msg, &mission_req_int_temp);

    mavlink_mission.curr_count = mission_req_int_temp.seq;

    Mavlink_Mission_item_send(mavlink_mission.curr_count);
    mavlink_mission.Os_Timer_creat(mavlink_mission.timer, 1500, Mavlink_Mission_down_timerout_callback);
    mavlink_mission.status = MAVLINK_MISSION_STATUS_DOWN_LOADING;
}

static void Mavlink_Mission_rec_ack(unsigned char in_chan, const mavlink_message_t *msg)
{
    if (mavlink_mission.status == MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_mission.Os_Timer_stop_and_reset(mavlink_mission.timer);

    mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
}

static void Mavlink_Mission_down_timerout_callback()
{
    unsigned char count_temp = mavlink_mission.Os_Timer_curr(mavlink_mission.timer);

    if (count_temp < MAX_RETRY)
    {
        if (mavlink_mission.status == MAVLINK_MISSION_STATUS_DOWN_START)
        {
            Mavlink_Mission_count_send(mavlink_mission.count);
        }
        else if (mavlink_mission.status == MAVLINK_MISSION_STATUS_DOWN_START)
        {
            Mavlink_Mission_item_send(mavlink_mission.curr_count);
        }
        else
        {
            //不应该运行到这里
        }
    }
    else
    {
        mavlink_mission.status = MAVLINK_MISSION_STATUS_IDLE;
        //关闭并复位超时计时器
        mavlink_mission.Os_Timer_stop_and_reset(mavlink_mission.timer);
    }
}

static void Mavlink_Mission_rec_set_curr(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_set_current_t mission_set_current_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_msg_mission_set_current_decode(msg, &mission_set_current_temp);

    if (mavlink_mission.Mission_check(mission_set_current_temp.seq))
    {
        Mavlink_Mission_curr_send(mission_set_current_temp.seq);
    }
    else
    {
        Mavlink_STATUSTEXT_send(MAV_SEVERITY_WARNING, 0, "check mission error,maintain the original plan\0");
    }
}

static void Mavlink_Mission_rec_clear(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_mission_clear_all_t mission_clear_temp;

    if (mavlink_mission.status != MAVLINK_MISSION_STATUS_IDLE)
    {
        return;
    }

    mavlink_msg_mission_clear_all_decode(msg, &mission_clear_temp);

    mavlink_mission.upload_tar_chan = in_chan;
    mavlink_mission.upload_tar_sys = msg->sysid;
    mavlink_mission.upload_tar_comp = msg->compid;
    mavlink_mission.type = mission_clear_temp.mission_type;

    Mavlink_Mission_ack_send(mavlink_mission.Mission_Clear());
}

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