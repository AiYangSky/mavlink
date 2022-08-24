/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-20 19:37:17
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-25 00:15:44
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Parameters.h
 */
#ifndef MAVLINK_PARAMETERS_H
#define MAVLINK_PARAMETERS_H

typedef struct
{
    unsigned short curr_count;
    unsigned char req_sys;
    unsigned char req_comp;

    // Timer
    void *timer;

    // Parameters storage
    unsigned short (*Get_numbers)(void);
    unsigned char (*Param_Get_by_index)(unsigned short, char *, void *);
    unsigned char (*Param_Get_by_name)(char *, unsigned short *, void *);
    void *(*Param_Chanege)(char *, unsigned char, void *);
} MAVLINK_PARAMETERS_CB_T;

void Mavlink_Parameters_init(const MAVLINK_PARAMETERS_CB_T *Control_block);

void Mavlink_Parameters_callback(void);

#endif