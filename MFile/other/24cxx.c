/*****************************************************************************************************/
/***********************************  www.kavirElectronic.ir  *****************************************/
/*****************************************************************************************************/
#include "24cxx.h"
#include "delay.h"

//Initialize the IIC interface
void AT24CXX_Init(void)
{
  IIC_Init();//IICinitialization
}
//Read a data at the specified address of AT24CXX
//ReadAddr: the address to start reading
//Return value: the data read
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
  u8 temp=0;		  	    																 
  IIC_Start();  
  if(EE_TYPE>AT24C16)
  {
    IIC_Send_Byte(0XA0);	   //Send write command
    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr>>8);//Send high address	    
  }else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //Send device address 0XA0, write data   
  IIC_Wait_Ack(); 
  IIC_Send_Byte(ReadAddr%256);   //Send low address
  IIC_Wait_Ack();	    
  IIC_Start();  	 	   
  IIC_Send_Byte(0XA1);           //Enter receiving mode			   
  IIC_Wait_Ack();	 
  temp=IIC_Read_Byte(0);		   
  IIC_Stop();//Generate a stop condition	    
  return temp;
}
//Write a data in AT24CXX specified address
//WriteAddr: the destination address for writing data
//DataToWrite: the data to be written
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
  IIC_Start();  
  if(EE_TYPE>AT24C16)
  {
    IIC_Send_Byte(0XA0);	    //Send write command
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr>>8);//Send high address	  
  }else IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //Send device address 0XA0, write data	 
  IIC_Wait_Ack();	   
  IIC_Send_Byte(WriteAddr%256);   //Send low address
  IIC_Wait_Ack(); 	 										  		   
  IIC_Send_Byte(DataToWrite);     //Send byte							   
  IIC_Wait_Ack();  		    	   
  IIC_Stop();//Generate a stop condition 
  delay_ms(10);	 
}
//Start writing data of length Len at the specified address in AT24CXX
//This function is used to write 16bit or 32bit data.
//WriteAddr: the address to start writing
//DataToWrite: the first address of the data array
//Len: The length of the data to be written 2, 4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
  u8 t;
  for(t=0;t<Len;t++)
  {
    AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
  }												    
}

//Start reading data of length Len from the specified address in AT24CXX
//This function is used to read 16bit or 32bit data.
//ReadAddr: the address to start reading
//Return value: data
//Len: The length of the data to be read 2,4
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{  	
  u8 t;
  u32 temp=0;
  for(t=0;t<Len;t++)
  {
    temp<<=8;
    temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
  }
  return temp;												    
}
//Check if AT24CXX is normal
//The last address (255) of 24XX is used here to store the logo word.
//If you use other 24C series, this address needs to be modified
//Return 1: Detection failed
//Return 0: detection is successful
u8 AT24CXX_Check(void)
{
  u8 temp;
  temp=AT24CXX_ReadOneByte(255);//Avoid writing AT24CXX every time you boot		   
  if(temp==0X55)return 0;		   
  else//Exclude the first initialization
  {
    AT24CXX_WriteOneByte(255,0X55);
    temp=AT24CXX_ReadOneByte(255);	  
    if(temp==0X55)return 0;
  }
  return 1;											  
}

//Start reading the specified number of data at the specified address in AT24CXX
//ReadAddr: The address to start reading is 0~255 for 24c02
//pBuffer: the first address of the data array
//NumToRead: The number of data to be read
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
  while(NumToRead)
  {
    *pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
    NumToRead--;
  }
}  
//Start writing the specified number of data at the specified address in AT24CXX
//WriteAddr: The address to start writing is 0~255 for 24c02
//pBuffer: the first address of the data array
//NumToWrite: the number of data to be written
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
  while(NumToWrite--)
  {
    AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
    WriteAddr++;
    pBuffer++;
  }
}
