# 野火指南者开发板-PWM呼吸灯

## 1    环境

RT_Thread  ENV开发工具，rt_thread源码包，MDK5

## 2   概述

### 2.1   功能实现

本实例使用RT-Thread源码包中的BSP移植到野火指南者开发板，过程中配置了PWM，实现了板载RGB呼吸灯的效果。

通过控制对应PWM通道，使得板载LED灯实现亮度变化以及色彩变化，程序中由颜色变化表来“驱动”灯的亮起状态。

### 2.2  引脚连接

|  LED  | 引脚 | PWM通道  |
| :---: | :--: | :------: |
| LED_R | PB5  | TIM3_CH2 |
| LED_G | PB0  | TIM3_CH3 |
| LED_B | PB1  | TIM3_CH4 |

### 2.3   实际效果

![](Doc\breathing_led.gif)

## 3  驱动配置

在RT-Thread源码包中我们使用STM32的库文件，编译构建关于野火指南针开发板的BSP。

### 3.1  使用STM32-CubeMX工具生成配置代码

在目录下的board目录中找到CubeMX_Config文件夹，打开文件夹下的CubeMX_Config.ioc文件，自动进入CubeMX工程。

左边目录中选择Timers -> TIM3，在TIM3 Mode and Configuration选项页中，将需要用到的Channel选择成为PWM Generation CH*。此处使用Channel2、Channel3和Channel4。

配置好后如图：

![](Doc\CubeMX_Config1.PNG)

**注意：**这里TIM3_CH2默认的引脚可能是PA7，在右边的芯片图片中找到PB5引脚，单击后选择TIM3_CH2，就能变成上图中的PB5了。

然后点击右上角的GENERATE CODE，即可自动生成配置代码。

在board目录下，CubeMX_Config-->Src的stm32f1xx_hal_msp.c中可以看到PB0、PB1和PB5的配置代码已经自动生成。

### 3.2  修改Kconfig文件

在board目录下有Kconfig文件，在文件中添加如下代码：

<img src="Doc\Kconfig.PNG" style="zoom:67%;" />

添加好后保存。

### 3.3  用ENV工具重新配置工程

在目录下打开ENV工具，输入menuconfig，选择Hardware Drivers Config --> On-chip Peripheral Drivers 即可看到目前支持的驱动。

<img src="Doc\menuconfig.PNG" style="zoom:50%;" />

勾选 Enable timer --> 勾选 Enable TIM3

勾选 Enable pwm -->  勾选 Enable timer3 output pwm  --> Enable PWM3 channel2、*channel3和 *channel4。

保存退出。

然后输入命令

```
scons --target = mdk5
```

配置编译成功后，用MDK5开发工具打开工程。打开工程后，我们可以查看Driver目录下是否有新的驱动配置文件加入。

<img src="Doc\Driver.PNG" style="zoom:67%;" />

## 4  实现

此部分主要是模块的Breathing_Led.h头文件与Breathing_Led.c源文件的编写，统一存放在Application路径下。

### 4.1  使用 rt_thread 创建线程

这里利用 rt_thread 创建出线程去维护PWM输出的变化。

关于线程具体用法参照[RT_Thread文档中心-内核-线程管理](https://www.rt-thread.org/document/site/programming-manual/thread/thread/)。

1. 线程初始化函数：查找PWM设备，初始化PWM设备，配置PWM输出的初始状态，使能PWM设备；创建线程，配置线程相关参数，配置线程入口函数。
2. 线程入口函数：不断调整PWM的输出状态。

在初始化函数中创建线程，调用初始化函数时，线程创建并启动，自动进入入口函数执行任务。

### 4.2  使用rt_thread中的PWM设备管理接口

详细用法参照[RT_Thread文档中心-设备和驱动-PWM设备](https://www.rt-thread.org/document/site/programming-manual/device/pwm/pwm/)

| **函数**         | **描述**                              |
| ---------------- | ------------------------------------- |
| rt_device_find() | 根据 PWM 设备名称查找设备获取设备句柄 |
| rt_pwm_set()     | 设置 PWM 周期和脉冲宽度               |
| rt_pwm_enable()  | 使能 PWM 设备                         |
| rt_pwm_disable() | 关闭 PWM 设备                         |

实现RGB呼吸灯原理在于调整PWM输出占空比进而控制RGB的亮度。

<img src="Doc\PWM1.PNG" style="zoom:67%;" />

### 4.3  全彩呼吸灯

呼吸的特性是一种类似图中的指数曲线过程，吸气是指数上升过程，呼气是指数下降过程，成年人吸气呼气整个过程持续约3秒。

<img src="Doc\呼吸灯波形.PNG" style="zoom:50%;" />

在单色LED实现呼吸灯时，我们依据此波形为亮度构建了一张**表1**（indexWave），此表中有300个元素，代表每一个10ms内LED的亮度，只需要查**表1**后将PWM占空比设置成对应值即可，一次完整的过程在3s左右。

而实现全彩呼吸灯时，由于有三通道颜色同时亮起，直接进行RGB模型上的均匀变化同时无法控制亮度。因此使用HSV颜色模型，HSV模型中分为色调（H），饱和度（S），明度（V）三个参数，依据**表1**的变化，生成对应的**HSV变化表**，然后利用HSV转RGB算法，生成一个有三百份数据的**RGB变化表**（在路径下有生成RGB表的对应Python脚本）。

然后每次都依据**RBG变化表**来对PWM进行配置，实现色彩均匀变化的全彩呼吸灯。

### 4.4  观察现象

程序下载成功后，通过串口给操作系统发送命令：

```
Breathing_led_start
```

即可观察到呼吸灯现象。

## 5  注意事项

1. 使用CubeMX工具配置PWM时，使能TIM3_CH2时的引脚默认为PA7，这时需要在右边图形界面中找到PB5引脚设置成TIM3_CH2输出，这时候引脚配置就能变成PB5了。
2. PWM占空比利用pulse来控制，一次脉冲周期是由period来控制，占空比是pules/period。此时pules / period是指一次周期中，输出高电平的占比。而实际电路中，LED是共阳极的，因此需要引脚输出低电平才能点亮LED，所以实际设置中需要逆向一次。即（period-pulse）/ period才是亮起的占比。
3. LED呼吸灯的变化是依靠已生成的表来进行的，此实例中只是一种呼吸灯方式，可以按需修改，但是只能遵循RGB的色彩设置模式。
4. 由于main.c中例程是红灯闪烁，对呼吸灯现象会有所影响。