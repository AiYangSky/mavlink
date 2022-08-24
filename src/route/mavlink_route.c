/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-12 12:11:18
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-24 16:22:50
 * @FilePath       : \mavlink\src\route\mavlink_route.c
 */

#include "minimal/minimal.h"
#include "common/common.h"
#include "mavlink_route.h"

//同一个工程中只能存在一个路由 这是由mavlink底层决定的
MAVLINK_ROUTE_CB_T mavlink_route;

/**
 * @description:
 * @param       {unsigned char} sysid           Local system ID
 * @param       {unsigned char} compid          Local Component ID
 * @return      {*}
 * @note       :
 */
void Mavlink_Route_init(unsigned char sysid, unsigned char compid)
{
    mavlink_route.compid = compid;
    mavlink_route.sysid = sysid;

    mavlink_route.route_nums = 0;
    memset(mavlink_route.route_list, 0, sizeof(mavlink_route.route_list));
    memset(mavlink_route.Process, 0, 4 * MAX_MAVLINK_PROCESS);
}

/**
 * @description:                                Set the input and output interfaces of the MAVLink channel
 * @param       {unsigned char} chan            Index of channel
 * @param       {bool} *Get_byte                Interface of input
 * @param       {unsigned short} *Send_bytes    Interfaces of output
 * @return      {*}
 * @note       :
 */
void Mavlink_Chan_Set(unsigned char chan,
                      bool (*Get_byte)(unsigned char *),
                      unsigned short (*Send_bytes)(unsigned char *, unsigned short))
{
    mavlink_route.chan_cb[chan].Get_byte = Get_byte;
    mavlink_route.chan_cb[chan].Send_bytes = Send_bytes;
}

/**
 * @description:                                The MAVLink dataframe is issued via the MAvLink route
 * @param       {unsigned char} sysid           target sysid    <0> broadcast
 * @param       {unsigned char} compid          target compid   <0> broadcast
 * @param       {void} *msg                     mavlink pack msg
 * @return      {*}
 * @note       :
 * funtion:
 *              unsigned short (*Pack)(unsigned char, unsigned char, unsigned char, mavlink_message_t *, const void *)
 * Prototype:
 *              unsigned short mavlink_msg_##msgname##_encode_chan(unsigned char sysid, unsigned char compid, unsigned char chan,
 *                                                                 mavlink_message_t *txmsg, const void *msg)
 */
bool Mavlink_Route_send(unsigned char tar_sysid, unsigned char tar_compid, const void *msg,
                        unsigned short (*Pack)(unsigned char, unsigned char, unsigned char, mavlink_message_t *, const void *))
{
    unsigned short len_temp;
    unsigned char i;
    bool sent_flag[MAVLINK_COMM_NUM_BUFFERS];
    bool ret = false;

    mavlink_route.Mutex_Get(mavlink_route.mutex);

    memset(sent_flag, 0, sizeof(sent_flag));

    // broadcast
    if (tar_sysid == 0)
    {
        for (i = 0; i < MAVLINK_COMM_NUM_BUFFERS; i++)
        {
            Pack(mavlink_route.sysid, mavlink_route.compid, i, &mavlink_route.tx_message, msg);
            len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);
            mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
        }
        ret = true;
    }
    else
    {
        for (i = 0; i < mavlink_route.route_nums; i++)
        {
            if (mavlink_route.route_list[i].sysid == tar_sysid)
            {
                // broadcast of system
                if (tar_compid == 0)
                {
                    // once sent
                    if (sent_flag[i] == false)
                    {
                        Pack(mavlink_route.sysid, mavlink_route.compid, i, &mavlink_route.tx_message, msg);
                        len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);
                        mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
                        sent_flag[i] == true;
                        ret = true;
                    }
                }
                // p2p sent
                else if (mavlink_route.route_list[i].compid == tar_compid)
                {
                    Pack(mavlink_route.sysid, mavlink_route.compid, i, &mavlink_route.tx_message, msg);
                    len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, &mavlink_route.tx_message);
                    mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
                    ret = true;
                    break;
                }
            }
        }
    }

    mavlink_route.Mutex_Put(mavlink_route.mutex);

    return ret;
}

/**
 * @description:                                Register a callback to process mavlink frame
 * @param       {void} *Process                 callback to process MAVLink data frame
 * @return      {*}
 * @note       :
 */
bool Mavlink_Register_process(void (*Process)(unsigned char, const mavlink_message_t *))
{
    unsigned short i;
    bool ret = false;
    for (i = 0; i < MAX_MAVLINK_PROCESS; i++)
    {
        if (mavlink_route.Process[i] == NULL)
        {
            mavlink_route.Process[i] = Process;
            ret = true;
            break;
        }
    }
    return ret;
}

/**
 * @description:                                Sends a warning string to the system
 * @param       {MAV_SEVERITY} status           Warning Severity
 * @param       {unsigned short} id             Event id
 * @param       {unsigned char} *str
 * @return      {*}
 * @note       :
 */
void Mavlink_STATUSTEXT_send(MAV_SEVERITY status, unsigned short id, char *str)
{
    mavlink_statustext_t mission_statustext_temp;
    unsigned char i = 0;

    mission_statustext_temp.severity = status;
    mission_statustext_temp.id = id;

    while (strlen(str) > 50)
    {
        memcpy(mission_statustext_temp.text, str, 50);
        mission_statustext_temp.chunk_seq = i;
        i++;
        str += 50;
        Mavlink_Route_send(0, 0, (void *)&mission_statustext_temp, mavlink_msg_statustext_encode_chan);
    }
    memcpy(mission_statustext_temp.text, str, strlen(str) + 1);
    Mavlink_Route_send(0, 0, (void *)&mission_statustext_temp, mavlink_msg_statustext_encode_chan);
}

/**
 * @description:                                Mavlink route detection
 * @param       {unsigned char} in_chan         Index of input channel
 * @param       {mavlink_message_t} *message    MAVLink data frame
 * @return      {*}
 * @note       : The precondition for establishing the MAVLink routing table is that the
 *               complete MAVLink data frame sent by the channel has been received and
 *               the peer ID identification is confirmed.
 *               The macro MAX_MAVLINK_ROUTE defines the maximum value of the routing table.
 */
static void Mavlink_Route_check(unsigned char in_chan, const mavlink_message_t *message)
{
    unsigned char i;

    // System broadcast domain
    if (message->sysid == 0)
    {
        return;
    }

    // Component broadcast domain
    if (message->sysid == mavlink_route.sysid && message->compid == MAV_COMP_ID_ALL)
    {
        return;
    }

    for (i = 0; i < mavlink_route.route_nums; i++)
    {
        if (mavlink_route.route_list[i].chan == in_chan &&
            mavlink_route.route_list[i].sysid == message->sysid &&
            mavlink_route.route_list[i].compid == message->compid)
        {
            if (mavlink_route.route_list[i].type == 0 && message->msgid == MAVLINK_MSG_ID_HEARTBEAT)
            {
                mavlink_route.route_list[i].type = mavlink_msg_heartbeat_get_type(message);
            }
            break;
        }
    }

    if (i < MAX_MAVLINK_ROUTE && i == mavlink_route.route_nums)
    {
        mavlink_route.route_list[i].chan == in_chan;
        mavlink_route.route_list[i].sysid == message->sysid;
        mavlink_route.route_list[i].compid == message->compid;
        if (message->msgid == MAVLINK_MSG_ID_HEARTBEAT)
        {
            mavlink_route.route_list[i].type = mavlink_msg_heartbeat_get_type(message);
        }
        mavlink_route.route_nums++;
    }
}

/**
 * @description:                                    Mavlink route processing
 * @param       {unsigned char} in_chan             Index of input channel
 * @param       {mavlink_message_t} *message        MAVLink data frame
 * @return      {*}
 * @note       :
 */
static bool Mavlink_Route_process(unsigned char in_chan, const mavlink_message_t *message)
{
    short tar_sysid = -1;
    short tar_compid = -1;
    mavlink_msg_entry_t *msg_entry;
    unsigned char i;
    unsigned short len_temp;
    bool ret = true;

    // Lockback?
    if (message->sysid == mavlink_route.sysid && message->compid == mavlink_route.compid)
    {
        ret = false;
        return ret;
    }

    Mavlink_Route_check(in_chan, message);

    switch (message->msgid)
    {
    // Type of forwarding not required
    case MAVLINK_MSG_ID_RADIO_STATUS:
    case MAVLINK_MSG_ID_ADSB_VEHICLE:
        return ret;
        break;
    default:
        break;
    }

    // Extract the target address of the message
    msg_entry = mavlink_get_msg_entry(message->msgid);

    if (msg_entry != NULL)
    {
        if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM)
        {
            tar_sysid = _MAV_RETURN_uint8_t(message, msg_entry->target_system_ofs);
        }
        if (msg_entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT)
        {
            tar_compid = _MAV_RETURN_uint8_t(message, msg_entry->target_component_ofs);
        }
    }
    else // Unidentifiable information is broadcast directly
    {
        ret = false;
    }

    len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, message);

    if (tar_sysid == mavlink_route.sysid)
    {
        if (tar_compid == mavlink_route.compid)
        {
        }
        else if (tar_compid == 0 || tar_compid == -1)
        {
            for (i = 0; i < mavlink_route.route_nums; i++)
            {
                if (mavlink_route.route_list[i].sysid == tar_sysid)
                {
                    mavlink_route.chan_cb[mavlink_route.route_list[i].chan].Send_bytes(mavlink_route.buf_temp, len_temp);
                }
            }
        }
        else
        {
            for (i = 0; i < mavlink_route.route_nums; i++)
            {
                if (mavlink_route.route_list[i].sysid == tar_sysid &&
                    mavlink_route.route_list[i].compid == tar_compid)
                {
                    ret = false;
                    mavlink_route.chan_cb[mavlink_route.route_list[i].chan].Send_bytes(mavlink_route.buf_temp, len_temp);
                    break;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < MAVLINK_COMM_NUM_BUFFERS; i++)
        {
            if (i = in_chan)
            {
                continue;
            }
            mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
        }
    }

    return ret;
}

/**
 * @description:
 * @param       {mavlink_message_t} *message
 * @return      {*}
 * @note       :
 */
static void Mavlink_Date_process(unsigned char in_chan, const mavlink_message_t *message)
{
    unsigned short i;
    for (i = 0; i < MAX_MAVLINK_PROCESS; i++)
    {
        if (mavlink_route.Process[i] != NULL)
        {
            mavlink_route.Process[i](in_chan, message);
        }
        else
        {
            break;
        }
    }
}

/**
 * @description:                                Framework for processing MAVLink data frames
 * @param       {unsigned char} status          Status of the frame
 * @param       {unsigned char} in_chan         Index of input channel
 * @param       {mavlink_message_t} *message    MAVLink data frame
 * @return      {*}
 * @note       :
 */
static void Mavlink_Process_Handle(unsigned char status, unsigned char in_chan, const mavlink_message_t *message)
{
    unsigned char i;
    unsigned short len_temp;

    switch (status)
    {
    case MAVLINK_FRAMING_OK:
        if (Mavlink_Route_process(in_chan, message))
        {
            Mavlink_Date_process(in_chan, message);
        }
        break;

    case MAVLINK_FRAMING_BAD_CRC:
    case MAVLINK_FRAMING_BAD_SIGNATURE:
        //无法验证CRC或者签名的数据帧 直接广播转发
        len_temp = mavlink_msg_to_send_buffer(mavlink_route.buf_temp, message);
        for (i = 0; i < MAVLINK_COMM_NUM_BUFFERS; i++)
        {
            if (i = in_chan)
            {
                continue;
            }
            mavlink_route.chan_cb[i].Send_bytes(mavlink_route.buf_temp, len_temp);
        }
        break;

    default:
        break;
    }
}

/**
 * @description:                                Mavlink reception and decoding framework
 * @return      {*}
 * @note       :                                Continuous operation at lower system priority is recommended
 */
void Mavlink_Rec_Handle(void)
{
    unsigned char byte_temp;
    unsigned char ret;
    unsigned char i;

    for (i = 0; i < MAVLINK_COMM_NUM_BUFFERS; i++)
    {
        if (mavlink_route.chan_cb[i].Get_byte(&byte_temp))
        {
            ret = mavlink_frame_char(i, byte_temp, &mavlink_route.message_temp, &mavlink_route.status_temp);

            if (ret)
            {
                //处理数据
                Mavlink_Process_Handle(ret, i, &mavlink_route.message_temp);
            }
        }
    }
}