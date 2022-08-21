/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-20 19:37:17
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-22 01:34:13
 * @FilePath       : \mavlink\src\Microservices\Mavlink_Parameters.h
 */
#ifndef MAVLINK_PARAMETERS_H
#define MAVLINK_PARAMETERS_H

typedef struct
{
    unsigned short number;
    unsigned short curr_count;
    unsigned char in_chan;

    // Timer
    void *timer;
    void (*Os_Timer_creat)(void *, unsigned short, void *);
    void (*Os_Timer_stop_and_reset)(void *);
    unsigned char (*Os_Timer_curr)(void *);

    // Parameters storage
    unsigned short (*Get_numbers)(void);
    unsigned char (*Param_Get_by_index)(unsigned short, char *, void *);
    unsigned char (*Param_Get_by_name)(char *, unsigned short *, void *);
    void *(*Param_Chanege)(char *, unsigned char, void *);
} MAVLINK_PARAMETERS_CB_T;

void Mavlink_Parameters_init(const MAVLINK_PARAMETERS_CB_T *Control_block);

#endif