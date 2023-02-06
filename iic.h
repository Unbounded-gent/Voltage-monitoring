#ifndef _IIC_H_
#define _IIC_H_

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
void write_24C02(unsigned char add,unsigned char data1);
unsigned char read_24C02(unsigned char add);
void AD_Init(void);
unsigned char AD_Get(void);

#endif