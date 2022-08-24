/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-11 19:29:53
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-25 01:29:37
 * @FilePath       : \mavlink\example\mavlink_microservices_example.c
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <Windows.h>
#include "Mavlink_Mission.h"
#include "mavlink_route.h"

//用于模拟接收到的二进制数据
unsigned char rec_buffer[2048];

// windostimer 需要的结构
static HWND my_hwwd;
static MSG msg;

void CALLBACK timer_callback(HWND hWnd, UINT nMsg, UINT_PTR nTimerid, DWORD dwTime)
{
}

void windos_timer_look(void)
{

    if (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_TIMER)
        {
            msg.hwnd == my_hwwd;
        }
        TranslateMessage(&msg); // translates virtual-key codes
        DispatchMessage(&msg);  // dispatches message to window
    }
}

// UINT_PTR timer_id[];
void Timer_creat(void *timer, unsigned short time, void (*callback)(unsigned long))
{
    *((UINT_PTR *)(timer)) = SetTimer(NULL, 1, 10, timer_callback);
}

unsigned short mavlink_send(unsigned char chan, unsigned char *data, unsigned short len)
{
    unsigned short i;
    printf("\n\n mavlink chan : %d sent: %d\n", chan, len);

    printf("head: 0X%x,0X%x,0X%x,0X%x,0X%x,0X%x,0X%x,0X%x\n",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6],
           *((unsigned int *)(data + 7)));

    for (i = 8; i < len; i++)
    {
        printf(" 0X%x , ", data[i]);
        if ((i - 8) % 10 == 0)
        {
            printf("\n");
        }
    }
}

unsigned short mavlink_send_chan1(unsigned char *data, unsigned short len)
{
    return mavlink_send(1, data, len);
}
unsigned short mavlink_send_chan2(unsigned char *data, unsigned short len)
{
    return mavlink_send(2, data, len);
}
unsigned short mavlink_send_chan3(unsigned char *data, unsigned short len)
{
    return mavlink_send(3, data, len);
}
unsigned short mavlink_send_chan4(unsigned char *data, unsigned short len)
{
    return mavlink_send(4, data, len);
}

bool mavlin_get_byte(unsigned char chan, unsigned char *data)
{
}

bool mavlin_get_byte_chan1(unsigned char *data)
{
    return mavlin_get_byte(1, data);
}

bool mavlin_get_byte_chan2(unsigned char *data)
{
    return mavlin_get_byte(2, data);
}

bool mavlin_get_byte_chan3(unsigned char *data)
{
    return mavlin_get_byte(3, data);
}

bool mavlin_get_byte_chan4(unsigned char *data)
{
    return mavlin_get_byte(4, data);
} 

int main()
{
    Mavlink_Route_init(1, 1);
    Mavlink_Route_Chan_Set(0, mavlin_get_byte_chan1, mavlink_send_chan1);
    Mavlink_Route_Chan_Set(0, mavlin_get_byte_chan2, mavlink_send_chan2);
    Mavlink_Route_Chan_Set(0, mavlin_get_byte_chan3, mavlink_send_chan3);
    Mavlink_Route_Chan_Set(0, mavlin_get_byte_chan4, mavlink_send_chan4);

    while (1)
    {



        windos_timer_look();
    }
}