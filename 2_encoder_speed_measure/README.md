# 编码器测速

## 如何使用:

1. 配置 syscfg,初始化相关引脚组,把 2 个 AB 相引脚初始化

![](picture_library\Screenshot_2025-07-23_102229.png)

1. 在 main.c 中添加初始化代码

```c
NVIC_EnableIRQ(USER_GPIO_INT_IRQN);//使能中断
```

2. 在 (main.c) 中添加测速代码

```c
int32_t _encoder_l_count = 0;
uint8_t x,y,z,sum;
void GROUP1_IRQHandler(void)
{
	switch(DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
	{
		case USER_GPIO_INT_IIDX:
		if(DL_GPIO_getEnabledInterruptStatus(GPIOA, USER_GPIO_PULSE_A_PIN))
		{
			x=1;//A中断，则x=1;B中断，则x=0;

			//中断发生时，A相高电平，y=1；反之y=0;
			if(DL_GPIO_readPins(GPIOA,USER_GPIO_PULSE_A_PIN)) y=1;
			else y=0;
			//中断发生时，B相高电平，z=1；反之z=0;
			if(DL_GPIO_readPins(GPIOA,USER_GPIO_PULSE_B_PIN)) z=1;
			else z=0;

			sum=x+y+z;//求和判断转动方向，偶数正转，奇数反转
			if(sum==0||sum==2) _encoder_l_count++;
			else _encoder_l_count--;

			DL_GPIO_clearInterruptStatus(GPIOA, USER_GPIO_PULSE_A_PIN);
		}

		if(DL_GPIO_getEnabledInterruptStatus(GPIOA, USER_GPIO_PULSE_B_PIN))
		{
			x=0;//A中断，则x=1;B中断，则x=0;
			//中断发生时，A相高电平，y=1；反之y=0;
			if(DL_GPIO_readPins(GPIOA,USER_GPIO_PULSE_A_PIN)) y=1;
			else y=0;
			//中断发生时，B相高电平，z=1；反之z=0;
			if(DL_GPIO_readPins(GPIOA,USER_GPIO_PULSE_B_PIN)) z=1;
			else z=0;

			sum=x+y+z;//求和判断转动方向，偶数正转，奇数反转
			if(sum==0||sum==2) _encoder_l_count++;
			else _encoder_l_count--;

			DL_GPIO_clearInterruptStatus(GPIOA, USER_GPIO_PULSE_B_PIN);
		}

		break;
	}
}
```
