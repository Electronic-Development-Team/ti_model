# SPI OLED 使用指南

## 如何使用:

1. 配置 syscfg,初始化硬件 spi
   ![](picture_library/Screenshot%202025-07-23%20100016.png)

2. 复制 code_library 文件夹到工程中
3. 添加头文件`drv_oled.h`

4. 在 main.c 中添加初始化代码

```c
oled_init();//oled显示屏初始化
```

## 常用接口

-   void display_6_8_string(unsigned char x,unsigned char y,char ch[])
