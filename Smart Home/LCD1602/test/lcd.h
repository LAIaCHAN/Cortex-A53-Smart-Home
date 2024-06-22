#ifndef __LCD_H
#define __LCD_H
typedef unsigned char           uint8_t;    
typedef unsigned short int      uint16_t;    
typedef unsigned int            uint32_t;    

void BSP_LCD1602_Init(int fd);
void BSP_LCD1602_WriteBit(int fd,uint8_t *buf, uint8_t len);
void BSP_LCD1602_ReadStatus(int fd);
void BSP_LCD1602_WriteCommand(int fd,unsigned char WCLCD, unsigned char BuysC); //BuysC为0时忽略忙检测
void BSP_LCD1602_WriteData(int fd,unsigned char WDLCD);
void DisplayOneChar(int fd,unsigned char X, unsigned char Y, unsigned char DData);
void DisplayListChar(int fd,unsigned char X, unsigned char Y, unsigned char *DData);

#endif
