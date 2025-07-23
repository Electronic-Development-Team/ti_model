# eeprom 读写

## 如何使用:

1. 配置 syscfg,初始化相关引脚组,PA28,PA31

2. 添加 systick 延时相关文件

3. 添加 eeprom 相关文件

4. 在主函数中调用 eeprom 相关函数

```c
bsp_eeprom_init(); //EEPROM初始化
```

## 常用接口

### 写入

函数原型:

-   void WriteFlashParameter(uint16_t Label,float WriteData);
-   void WriteFlashParameter_Two(uint16_t Label,float WriteData1,float WriteData2);
-   void WriteFlashParameter_Three(uint16_t Label,float WriteData1,float WriteData2,float WriteData3);

eg:

-   WriteFlashParameter(0,eeprom_write[0]);
-   WriteFlashParameter_Two(1,eeprom_write[1],eeprom_write[2]);
-   WriteFlashParameter_Three(3,eeprom_write[3],eeprom_write[4],eeprom_write[5]);

### 读取

函数原型:

-   void ReadFlashParameterOne(uint16_t Label,float \*ReadData);
-   void ReadFlashParameterTwo(uint16_t Label,float *ReadData1,float *ReadData2);
-   void ReadFlashParameterThree(uint16_t Label,float *ReadData1,float *ReadData2,float \*ReadData3);

eg:

-   ReadFlashParameterThree(0,&eeprom_read[0],&eeprom_read[1],&eeprom_read[2]);

-   ReadFlashParameterTwo(3,&eeprom_read[3],&eeprom_read[4]);

-   ReadFlashParameterOne(5,&eeprom_read[5]);
