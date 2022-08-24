/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-20 19:36:48
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-25 00:15:49
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Parameters.c
 */

#include "common/mavlink.h"
#include "common/common.h"
#include "mavlink_route.h"
#include "Mavlink_Parameters.h"

static MAVLINK_PARAMETERS_CB_T mavlink_params;
static void Mavlink_Parameters_process(unsigned char in_chan, const mavlink_message_t *msg);

/**                                 Initialize Parameters CB
 * @description:
 * @return      {*}
 * @note       :
 */
void Mavlink_Parameters_init(const MAVLINK_PARAMETERS_CB_T *Control_block)
{
    Mavlink_Register_process(Mavlink_Parameters_process);
    memset((void *)&mavlink_params, 0, sizeof(MAVLINK_PARAMETERS_CB_T));
    memcpy(&mavlink_params, Control_block, sizeof(MAVLINK_PARAMETERS_CB_T));
}

/**
 * @description:                        The parameter is found based on it's index and packaged to be sent
 * @param       {unsigned char} chan    The input MAVlink channel number
 * @param       {unsigned short} index  index of parameter
 * @return      {*}
 * @note       :
 */
static bool Mavlink_Parameters_value_send_by_index(unsigned char tar_sys, unsigned char tar_comp, unsigned short index)
{
    mavlink_param_value_t param_value_temp;

    memset((unsigned char *)&param_value_temp.param_value, 0, 4);
    param_value_temp.param_type = mavlink_params.Param_Get_by_index(index, param_value_temp.param_id,
                                                                    (void *)&param_value_temp.param_value);
    if (param_value_temp.param_type == 0)
    {
        return false;
    }

    param_value_temp.param_index = index;
    param_value_temp.param_index = mavlink_params.Get_numbers();

    Mavlink_Route_send(tar_sys, tar_comp, (void *)&param_value_temp, mavlink_msg_param_value_encode_chan);
}

/**
 * @description:                        The parameter is found based on it's name and packaged to be sent
 * @param       {unsigned char} chan    The input MAVlink channel number
 * @param       {char} *name            name of parameter
 * @return      {*}
 * @note       :
 */
static bool Mavlink_Parameters_value_send_by_name(unsigned char tar_sys, unsigned char tar_comp, char *name)
{
    mavlink_param_value_t param_value_temp;

    memset((unsigned char *)&param_value_temp.param_value, 0, 4);
    param_value_temp.param_type = mavlink_params.Param_Get_by_name(name, &param_value_temp.param_index,
                                                                   (void *)&param_value_temp.param_value);
    if (param_value_temp.param_type == 0)
    {
        return false;
    }

    memcpy(param_value_temp.param_id, name, 16);
    param_value_temp.param_index = mavlink_params.Get_numbers();

    Mavlink_Route_send(tar_sys, tar_comp, (void *)&param_value_temp, mavlink_msg_param_value_encode_chan);
}

/**
 * @description:                            Interval for parameter sending tasks
 * @return      {*}
 * @note       :
 */
void Mavlink_Parameters_callback(void)
{
    if (mavlink_params.curr_count < mavlink_params.Get_numbers())
    {
        Mavlink_Parameters_value_send_by_index(mavlink_params.req_sys, mavlink_params.req_comp, mavlink_params.curr_count);
        mavlink_params.curr_count++;
    }
    else
    {
        mavlink_route.Os_Timer_stop_and_reset(mavlink_params.timer);
    }
}

/**
 * @description:                            Responding to a parameter list request
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Parameters_rec_req_list(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_param_request_list_t param_list_temp;

    mavlink_msg_param_request_list_decode(msg, &param_list_temp);

    mavlink_params.curr_count = 0;
    mavlink_params.req_sys = msg->sysid;
    mavlink_params.req_comp = msg->compid;

    if (mavlink_params.curr_count < mavlink_params.Get_numbers())
    {
        Mavlink_Parameters_value_send_by_index(mavlink_params.req_sys, mavlink_params.req_comp, mavlink_params.curr_count);
        mavlink_params.curr_count++;
        mavlink_route.Os_Timer_activate(mavlink_params.timer);
    }
}

/**
 * @description:                            Responding to individual parameter requests
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
static void Mavlink_Parameters_rec_req_read(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_param_request_read_t param_read_temp;
    bool ret;

    mavlink_msg_param_request_read_decode(msg, &param_read_temp);

    if (param_read_temp.param_index == -1)
    {
        ret = Mavlink_Parameters_value_send_by_name(msg->sysid, msg->compid, param_read_temp.param_id);
    }
    else
    {
        ret = Mavlink_Parameters_value_send_by_index(msg->sysid, msg->compid, param_read_temp.param_index);
    }

    if (!ret)
    {
        Mavlink_STATUSTEXT_send(MAV_SEVERITY_NOTICE, 2, "Requested parameter does not exist\0");
    }
}

/**
 * @description:                            Responds to a modification of a parameter
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :
 */
void Mavlink_Parameters_rec_set(unsigned char in_chan, const mavlink_message_t *msg)
{
    mavlink_param_set_t param_set_temp;

    mavlink_msg_param_set_decode(msg, &param_set_temp);

    if (mavlink_params.Param_Chanege(param_set_temp.param_id, param_set_temp.param_type,
                                     (void *)&param_set_temp.param_value) != NULL)
    {
        Mavlink_Parameters_value_send_by_name(msg->sysid, msg->compid, param_set_temp.param_id);
    }
    else
    {
        Mavlink_STATUSTEXT_send(MAV_SEVERITY_NOTICE, 2, "Requested parameter does not exist\0");
    }
}

/**
 * @description:                            Mavlink_Parameters_process
 * @param       {unsigned char} in_chan
 * @param       {mavlink_message_t} *msg
 * @return      {*}
 * @note       :                            call in Mavlink_Date_process()
 */
static void Mavlink_Parameters_process(unsigned char in_chan, const mavlink_message_t *msg)
{
    switch (msg->msgid)
    {
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        Mavlink_Parameters_rec_req_list(in_chan, msg);
        break;
    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        Mavlink_Parameters_rec_req_read(in_chan, msg);
        break;
    case MAVLINK_MSG_ID_PARAM_SET:
        Mavlink_Parameters_rec_set(in_chan, msg);
        break;

    default:
        break;
    }
}