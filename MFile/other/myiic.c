/*****************************************************************************************************/
/***********************************  www.kavirElectronic.ir  *****************************************/
/*****************************************************************************************************/
#include "myiic.h"
#include "delay.h"

//IIC initialization
void IIC_Init(void)
{
  GPIO_InitTypeDef GPIO_Initure;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();   //Enable GPIOH clock
  
  //PH4,5Initial settings
  GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11;
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //Push-pull output
  GPIO_Initure.Pull=GPIO_PULLUP;          //pull up
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;    //fast
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);
  
  IIC_SDA(1);
  IIC_SCL(1);  
}

//Generate IIC start signal
void IIC_Start(void)
{
  SDA_OUT();     //sda line output
  IIC_SDA(1);	  	  
  IIC_SCL(1);
  delay_us(4);
  IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
  delay_us(4);
  IIC_SCL(0);//Clamp the I2C bus, ready to send or receive data
}	  
//Generate IIC stop signal
void IIC_Stop(void)
{
  SDA_OUT();//sda line output
  IIC_SCL(0);
  IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
  delay_us(4);
  IIC_SCL(1); 
  IIC_SDA(1);//Send I2C bus end signal
  delay_us(4);							   	
}
//Waiting for the response signal
//Return value: 1, failed to receive response
// 0, the response is received successfully
u8 IIC_Wait_Ack(void)
{
  u8 ucErrTime=0;
  SDA_IN();      //SDA set as input  
  IIC_SDA(1);delay_us(1);	   
  IIC_SCL(1);delay_us(1);	 
  while(READ_SDA)
  {
    ucErrTime++;
    if(ucErrTime>250)
    {
      IIC_Stop();
      return 1;
    }
  }
  IIC_SCL(0);//Clock output 0 	   
  return 0;  
} 
//Generate ACK response
void IIC_Ack(void)
{
  IIC_SCL(0);
  SDA_OUT();
  IIC_SDA(0);
  delay_us(2);
  IIC_SCL(1);
  delay_us(2);
  IIC_SCL(0);
}
//No ACK response		    
void IIC_NAck(void)
{
  IIC_SCL(0);
  SDA_OUT();
  IIC_SDA(1);
  delay_us(2);
  IIC_SCL(1);
  delay_us(2);
  IIC_SCL(0);
}					 				     
//IIC sends a byte
//Return whether the slave responds
//1, there is a response
//0, no response		  
void IIC_Send_Byte(u8 txd)
{                        
  u8 t;   
  SDA_OUT(); 	    
  IIC_SCL(0);//Pull down the clock to start data transmission
  for(t=0;t<8;t++)
  {              
    IIC_SDA((txd&0x80)>>7);
    txd<<=1; 	  
    delay_us(2);   //All three delays are necessary for TEA5767
    IIC_SCL(1);
    delay_us(2); 
    IIC_SCL(0);	
    delay_us(2);
  }	 
} 	    
//Read 1 byte, when ack=1, send ACK, ack=0, send nACK 
u8 IIC_Read_Byte(unsigned char ack)
{
  unsigned char i,receive=0;
  SDA_IN();//SDA set as input
  for(i=0;i<8;i++ )
  {
    IIC_SCL(0); 
    delay_us(2);
    IIC_SCL(1);
    receive<<=1;
    if(READ_SDA)receive++;   
    delay_us(1); 
  }					 
  if (!ack)
    IIC_NAck();//Send nACK
  else
    IIC_Ack(); //Send ACK   
  return receive;
}


