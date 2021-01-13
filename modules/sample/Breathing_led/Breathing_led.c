#include "Breathing_led.h"

//线程相关
#define THREAD_PRIORITY    25
#define THREAD_STACK_SIZE  512
#define THREAD_TIMESLICE   20

/*
PWM3  IO
CH2---PB5  red
CH3---PB0  green
CH4---PB1  blue
*/
#define PWM3_DEVICE_NAME "pwm3"
#define PWM3_DEVICE_CHANNEL2 2
#define PWM3_DEVICE_CHANNEL3 3
#define PWM3_DEVICE_CHANNEL4 4

//周期时间为 0.5ms (毫秒)，则 period 值为 500000ns（纳秒），输出频率为 2KHz，占空比为 pulse / period，pulse 值不能超过 period
#define PWM3_PERIOD 50000    //周期变化单位ns，此处是0.05ms
#define PWM3_STEP   195     //pwm变化量   50000/256 亮度步长 rgb
struct rt_device_pwm *pwm_dev;   //PWM设备句柄

// /* 呼吸灯曲线表 */
extern uint8_t breath_led_table[][3];

static void Breathing_led_entry(void *parameter)    //线程入口函数
{
    static uint32_t pulse_r,pulse_g,pulse_b,count;
    uint8_t r,g,b;      //RGB三色值
    pulse_r=0;pulse_g=0;pulse_b=0;
    count = 0;
    while(1)
    {
        r=breath_led_table[count][0];       //每次都查颜色变化表给PWM赋值
        g=breath_led_table[count][1];
        b=breath_led_table[count][2];
        pulse_r = PWM3_PERIOD - (PWM3_STEP * r);   
        pulse_g = PWM3_PERIOD - (PWM3_STEP * g);
        pulse_b = PWM3_PERIOD - (PWM3_STEP * b);
        rt_pwm_set(pwm_dev,PWM3_DEVICE_CHANNEL2,PWM3_PERIOD,pulse_r);
        rt_pwm_set(pwm_dev,PWM3_DEVICE_CHANNEL3,PWM3_PERIOD,pulse_g);
        rt_pwm_set(pwm_dev,PWM3_DEVICE_CHANNEL4,PWM3_PERIOD,pulse_b);
        count++;
        count = ( count > 300 )? 0 : count;
        rt_thread_mdelay(10);
    }
}

static int Breathing_led_start()      //线程初始化函数
{
    //初始化PWM设备
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM3_DEVICE_NAME);
    if(pwm_dev == RT_NULL)
    {
        rt_kprintf("cannot find pwmdev\n");
        return RT_ERROR;
    }

	rt_pwm_set(pwm_dev,PWM3_DEVICE_CHANNEL2,PWM3_PERIOD,PWM3_PERIOD); //初始PWM默认值,电路板上LED低电平亮起，因此初始化要全部拉高
    rt_pwm_set(pwm_dev,PWM3_DEVICE_CHANNEL3,PWM3_PERIOD,PWM3_PERIOD); //初始PWM默认值
    rt_pwm_set(pwm_dev,PWM3_DEVICE_CHANNEL4,PWM3_PERIOD,PWM3_PERIOD); //初始PWW默认值

    rt_pwm_enable(pwm_dev,PWM3_DEVICE_CHANNEL2);   //使能PWM通道
    rt_pwm_enable(pwm_dev,PWM3_DEVICE_CHANNEL3);   //使能PWM通道
    rt_pwm_enable(pwm_dev,PWM3_DEVICE_CHANNEL4);   //使能PWM通道

    rt_thread_t tid;
    //线程创建
    tid=rt_thread_create("Breathing_led",
                          Breathing_led_entry,
                          RT_NULL,
                          THREAD_STACK_SIZE,
                          THREAD_PRIORITY,
                          THREAD_TIMESLICE);
    if(tid !=RT_NULL)
    {
        rt_thread_startup(tid);         //线程启动
    }
    else
    {
        return RT_ERROR;
    }
    return RT_EOK;
}
MSH_CMD_EXPORT(Breathing_led_start,Breathing_led_start);