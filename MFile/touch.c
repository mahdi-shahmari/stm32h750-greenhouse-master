#include "touch.h" 
#include "lcd.h"
#include "delay.h"
#include "stdlib.h"
#include "math.h"
#include "gui.h"	    
#include "qspi_w25q64.h"

extern QSPI_HandleTypeDef hqspi;

#define TP_CALIB_FLASH_ADDR   0x7FF000UL   /* last 4K sector of the 8MB chip */
#define TP_CALIB_MAGIC        0xA5         /* marks "valid calibration present" */

typedef struct
{
    float   xfac;
    float   yfac;
    int16_t xoff;
    int16_t yoff;
    uint8_t touchtype;
    uint8_t magic;
} TP_CalibData_t;   /* 14 bytes total */

_m_tp_dev tp_dev=
{
  TP_Init,
  TP_Scan,
  TP_Adjust,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,	  	 		
  0,
  0,	  	 		
};					
//The default is data with touchtype=0.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

/*****************************************************************************
* @name       :void TP_Write_Byte(u8 num)   
* @date       :2018-08-09 
* @function   :Write a byte data to the touch screen IC with SPI bus
* @parameters :num:Data to be written
* @retvalue   :None
******************************************************************************/  	 			    					   
void TP_Write_Byte(u8 num)    
{  
  u8 count=0;   
  for(count=0;count<8;count++)  
  { 	  
    if(num&0x80)TDIN(1);  
    else TDIN(0);   
    num<<=1;    
    TCLK(0); 
    delay_us(1);
    TCLK(1);		//Active on rising edge	        
  }		 			    
}

/*****************************************************************************
* @name       :u16 TP_Read_AD(u8 CMD)	  
* @date       :2018-08-09 
* @function   :Reading adc values from touch screen IC with SPI bus
* @parameters :CMD:Read command,0xD0 for x,0x90 for y
* @retvalue   :Read data
******************************************************************************/    
u16 TP_Read_AD(u8 CMD)	  
{ 	 
  u8 count=0; 	  
  u16 Num=0; 
  TCLK(0);		//First pull down the clock 	 
  TDIN(0); 	//Pull down the data line
  TCS(0); 		//Select the touch screen IC
  TP_Write_Byte(CMD);//Send command word
  delay_us(6);//The longest conversion time of ADS7846 is 6us
  TCLK(0); 	     	    
  delay_us(1);    	   
  TCLK(1);		//Give 1 clock, clear BUSY
  delay_us(1);    
  TCLK(0); 	     	    
  for(count=0;count<16;count++)//Read 16-bit data, only the upper 12 bits are valid 
  { 				  
    Num<<=1; 	 
    TCLK(0);	//Active on falling edge  	    	   
    delay_us(1);    
    TCLK(1);
    if(DOUT)Num++; 		 
  }  	
  Num>>=4;   	//Only the upper 12 bits are valid.
  TCS(1);		//Release selection	 
  return(Num);  
  //#endif
}

#define READ_TIMES 5 	//Read times
#define LOST_VAL 1	  	//Drop value
/*****************************************************************************
* @name       :u16 TP_Read_XOY(u8 xy)  
* @date       :2018-08-09 
* @function   :Read the touch screen coordinates (x or y),
Read the READ_TIMES secondary data in succession 
and sort the data in ascending order,
Then remove the lowest and highest number of LOST_VAL 
and take the average
* @parameters :xy:Read command(CMD_RDX/CMD_RDY)
* @retvalue   :Read data
******************************************************************************/  
u16 TP_Read_XOY(u8 xy)
{
  u16 i, j;
  u16 buf[READ_TIMES];
  u16 sum=0;
  u16 temp;
  for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
  for(i=0;i<READ_TIMES-1; i++)//Sort
  {
    for(j=i+1;j<READ_TIMES;j++)
    {
      if(buf[i]>buf[j])//Ascending
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }	  
  sum=0;
  for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
  temp=sum/(READ_TIMES-2*LOST_VAL);
  return temp;   
} 

/*****************************************************************************
* @name       :u8 TP_Read_XY(u16 *x,u16 *y)
* @date       :2018-08-09 
* @function   :Read touch screen x and y coordinates,
The minimum value can not be less than 100
* @parameters :x:Read x coordinate of the touch screen
y:Read y coordinate of the touch screen
* @retvalue   :0-fail,1-success
******************************************************************************/ 
u8 TP_Read_XY(u16 *x,u16 *y)
{
  u16 xtemp,ytemp;			 	 		  
  xtemp=TP_Read_XOY(CMD_RDX);
  ytemp=TP_Read_XOY(CMD_RDY);	  												   
  //if(xtemp<100||ytemp<100)return 0;//Reading failed
  *x=xtemp;
  *y=ytemp;
  return 1;//Successful reading
}

#define ERR_RANGE 50 //tolerance scope 
/*****************************************************************************
* @name       :u8 TP_Read_XY2(u16 *x,u16 *y) 
* @date       :2018-08-09 
* @function   :Read the touch screen coordinates twice in a row, 
and the deviation of these two times can not exceed ERR_RANGE, 
satisfy the condition, then think the reading is correct, 
otherwise the reading is wrong.
This function can greatly improve the accuracy.
* @parameters :x:Read x coordinate of the touch screen
y:Read y coordinate of the touch screen
* @retvalue   :0-fail,1-success
******************************************************************************/ 
u8 TP_Read_XY2(u16 *x,u16 *y) 
{
  u16 x1,y1;
  u16 x2,y2;
  u8 flag;    
  flag=TP_Read_XY(&x1,&y1);   
  if(flag==0)return(0);
  flag=TP_Read_XY(&x2,&y2);	   
  if(flag==0)return(0);   
  if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//Two samples before and after are within +-50
     &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
  {
    *x=(x1+x2)/2;
    *y=(y1+y2)/2;
    return 1;
  }else return 0;	  
} 

/*****************************************************************************
* @name       :void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
* @date       :2018-08-09 
* @function   :Draw a touch point,Used to calibrate							
* @parameters :x:Read x coordinate of the touch screen
y:Read y coordinate of the touch screen
color:the color value of the touch point
* @retvalue   :None
******************************************************************************/  
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
  POINT_COLOR=color;
  LCD_DrawLine(x-12,y,x+13,y);//Horizontal line
  LCD_DrawLine(x,y-12,x,y+13);//Vertical line
  LCD_DrawPoint(x+1,y+1);
  LCD_DrawPoint(x-1,y+1);
  LCD_DrawPoint(x+1,y-1);
  LCD_DrawPoint(x-1,y-1);
  gui_circle(x,y,POINT_COLOR,6,0);//Draw center circle
}	

/*****************************************************************************
* @name       :void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
* @date       :2018-08-09 
* @function   :Draw a big point(2*2)					
* @parameters :x:Read x coordinate of the point
y:Read y coordinate of the point
color:the color value of the point
* @retvalue   :None
******************************************************************************/   
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
  POINT_COLOR=color;
  LCD_DrawPoint(x,y);//Center point 
  LCD_DrawPoint(x+1,y);
  LCD_DrawPoint(x,y+1);
  LCD_DrawPoint(x+1,y+1);	 	  	
}	

/*****************************************************************************
* @name       :u8 TP_Scan(u8 tp)
* @date       :2018-08-09 
* @function   :Scanning touch event				
* @parameters :tp:0-screen coordinate 
1-Physical coordinates(For special occasions such as calibration)
* @retvalue   :Current touch screen status,
0-no touch
1-touch
******************************************************************************/  					  
u8 TP_Scan(u8 tp)
{			   
  if(PEN==0)//Button pressed
  {
    if(tp)TP_Read_XY2(&tp_dev.x,&tp_dev.y);//Read physical coordinates
    else if(TP_Read_XY2(&tp_dev.x,&tp_dev.y))//Read screen coordinates
    {
      tp_dev.x=tp_dev.xfac*tp_dev.x+tp_dev.xoff;//Convert the result to screen coordinates
      tp_dev.y=tp_dev.yfac*tp_dev.y+tp_dev.yoff;  
    } 
    if((tp_dev.sta&TP_PRES_DOWN)==0)//Has not been pressed before
    {		 
      tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//Key press  
      tp_dev.x0=tp_dev.x;//Record the coordinates of the first press
      tp_dev.y0=tp_dev.y;  	   			 
    }			   
  }
  else
  {
    if(tp_dev.sta&TP_PRES_DOWN)//Was pressed before
    {
      tp_dev.sta&=~(1<<7);//Mark button release	
    }else//Has not been pressed before
    {
      tp_dev.x0=0;
      tp_dev.y0=0;
      tp_dev.x=0xffff;
      tp_dev.y=0xffff;
    }	    
  }
  return tp_dev.sta&TP_PRES_DOWN;//Return to the current touch screen state
}

//////////////////////////////////////////////////////////////////////////	 
//The base address of the address range stored in the EEPROM, occupying 13 bytes (RANGE: SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 40
/*****************************************************************************
* @name       :void TP_Save_Adjdata(void)
* @date       :2018-08-09 
* @function   :Save calibration parameters		
* @parameters :None
* @retvalue   :None
******************************************************************************/ 										    
void TP_Save_Adjdata(void)
{
    TP_CalibData_t calib;
    calib.xfac      = tp_dev.xfac;
    calib.yfac      = tp_dev.yfac;
    calib.xoff      = (int16_t)tp_dev.xoff;
    calib.yoff      = (int16_t)tp_dev.yoff;
    calib.touchtype = tp_dev.touchtype;
    calib.magic     = TP_CALIB_MAGIC;

    HAL_QSPI_Abort(&hqspi);                          /* ensure indirect mode */
    QSPI_W25Qxx_SectorErase(TP_CALIB_FLASH_ADDR);     /* must erase before write */
    QSPI_W25Qxx_WriteBuffer((uint8_t*)&calib, TP_CALIB_FLASH_ADDR, sizeof(calib));
    QSPI_W25Qxx_MemoryMappedMode();  
}

/*****************************************************************************
* @name       :u8 TP_Get_Adjdata(void)
* @date       :2018-08-09 
* @function   :Gets the calibration values stored in the EEPROM		
* @parameters :None
* @retvalue   :1-get the calibration values successfully
0-get the calibration values unsuccessfully and Need to recalibrate
******************************************************************************/ 	
u8 TP_Get_Adjdata(void)
{
   TP_CalibData_t calib;

    HAL_QSPI_Abort(&hqspi);
    QSPI_W25Qxx_ReadBuffer((uint8_t*)&calib, TP_CALIB_FLASH_ADDR, sizeof(calib));

    if (calib.magic == TP_CALIB_MAGIC)
    {
        tp_dev.xfac      = calib.xfac;
        tp_dev.yfac      = calib.yfac;
        tp_dev.xoff      = calib.xoff;
        tp_dev.yoff      = calib.yoff;
        tp_dev.touchtype = calib.touchtype;

        if (tp_dev.touchtype) { CMD_RDX = 0X90; CMD_RDY = 0XD0; }
        else                  { CMD_RDX = 0XD0; CMD_RDY = 0X90; }

        return 1;
    }
    return 0;   /* erased flash reads as 0xFF, so magic won't match ? recalibrate */
0;
}	

//Prompt string
//const u8* TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";

/*****************************************************************************
* @name       :void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
* @date       :2018-08-09 
* @function   :Display calibration results	
* @parameters :x0:the x coordinates of first calibration point
y0:the y coordinates of first calibration point
x1:the x coordinates of second calibration point
y1:the y coordinates of second calibration point
x2:the x coordinates of third calibration point
y2:the y coordinates of third calibration point
x3:the x coordinates of fourth calibration point
y3:the y coordinates of fourth calibration point
fac:calibration factor 
* @retvalue   :None
******************************************************************************/ 	 					  
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{	  
//  POINT_COLOR=RED;
//  LCD_ShowString(40,140,16,"x1:",1);
//  LCD_ShowString(40+80,140,16,"y1:",1);
//  LCD_ShowString(40,160,16,"x2:",1);
//  LCD_ShowString(40+80,160, 16,"y2:",1);
//  LCD_ShowString(40,180, 16,"x3:",1);
//  LCD_ShowString(40+80,180, 16,"y3:",1);
//  LCD_ShowString(40,200, 16,"x4:",1);
//  LCD_ShowString(40+80,200, 16,"y4:",1);  
//  LCD_ShowString(40,220, 16,"fac is:",1);     
//  LCD_ShowNum(40+24,140,x0,4,16);		//Display value
//  LCD_ShowNum(40+24+80,140,y0,4,16);	//Display value
//  LCD_ShowNum(40+24,160,x1,4,16);		//Display value
//  LCD_ShowNum(40+24+80,160,y1,4,16);	//Display value
//  LCD_ShowNum(40+24,180,x2,4,16);		//Display value
//  LCD_ShowNum(40+24+80,180,y2,4,16);	//Display value
//  LCD_ShowNum(40+24,200,x3,4,16);		//Display value
//  LCD_ShowNum(40+24+80,200,y3,4,16);	//Display value
//  LCD_ShowNum(40+56,220,fac,3,16); 	//Display the value, the value must be within the range of 95~105.
}

/*****************************************************************************
* @name       :u8 TP_Get_Adjdata(void)
* @date       :2018-08-09 
* @function   :Calibration touch screen and Get 4 calibration parameters
* @parameters :None
* @retvalue   :None
******************************************************************************/ 		 
void TP_Adjust(void)
{								 
  u16 pos_temp[4][2];//Coordinate cache value
  u8  cnt=0;	
  u16 d1,d2;
  u32 tem1,tem2;
  double fac; 	
  u16 outtime=0;
  cnt=0;				
  POINT_COLOR=BLUE;
  BACK_COLOR =WHITE;
  LCD_Clear(WHITE);//Clear screen   
  POINT_COLOR=RED;//red 
  LCD_Clear(WHITE);//Clear screen 	   
  POINT_COLOR=BLACK;
//  LCD_ShowString(10,40,16,"Please use the stylus click",1);//Show message
//  LCD_ShowString(10,56,16,"the cross on the screen.",1);//Show message
//  LCD_ShowString(10,72,16,"The cross will always move",1);//Show message
//  LCD_ShowString(10,88,16,"until the screen adjustment",1);//Show message
//  LCD_ShowString(10,104,16,"is completed.",1);//Show message
  
  TP_Drow_Touch_Point(20,20,RED);//Plot 1 
  tp_dev.sta=0;//Eliminate the trigger signal 
  tp_dev.xfac=0;//xfac is used to mark whether it has been calibrated, so it must be cleared before calibration! to avoid errors	 
  while(1)//If it is not pressed for 10 seconds, it will automatically exit
  {
    tp_dev.scan(1);//Scan physical coordinates
    if((tp_dev.sta&0xc0)==TP_CATH_PRES)//The button is pressed once (the button is now released.)
    {	
      outtime=0;		
      tp_dev.sta&=~(1<<6);//The mark button has been processed.
      
      pos_temp[cnt][0]=tp_dev.x;
      pos_temp[cnt][1]=tp_dev.y;
      cnt++;	  
      switch(cnt)
      {			   
      case 1:						 
        TP_Drow_Touch_Point(20,20,WHITE);				//Clear point 1 
        TP_Drow_Touch_Point(lcddev.width-20,20,RED);	//Plot 2
        break;
      case 2:
        TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	//Clear point 2
        TP_Drow_Touch_Point(20,lcddev.height-20,RED);	//Plot 3
        break;
      case 3:
        TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);			//Clear point 3
        TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);	//Plot 4
        break;
      case 4:	 //All four points have been obtained
        //Equal opposite sides
        tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
        tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
        tem1*=tem1;
        tem2*=tem2;
        d1=sqrt(tem1+tem2);//Get the distance of 1,2
        
        tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
        tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
        tem1*=tem1;
        tem2*=tem2;
        d2=sqrt(tem1+tem2);//Get the distance of 3,4
        fac=(float)d1/d2;
        if(fac<0.95||fac>1.05||d1==0||d2==0)//Unqualified
        {
          cnt=0;
          TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//Clear point 4
          TP_Drow_Touch_Point(20,20,RED);								//Plot 1
          TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//Display Data   
          continue;
        }
        tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
        tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
        tem1*=tem1;
        tem2*=tem2;
        d1=sqrt(tem1+tem2);//Get the distance of 1,3
        
        tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
        tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
        tem1*=tem1;
        tem2*=tem2;
        d2=sqrt(tem1+tem2);//Get the distance of 2,4
        fac=(float)d1/d2;
        if(fac<0.95||fac>1.05)//Unqualified
        {
          cnt=0;
          TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//Clear point 4
          TP_Drow_Touch_Point(20,20,RED);								//Plot 1
          TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//Display Data   
          continue;
        }//Correct
        
        //Diagonal equal
        tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
        tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
        tem1*=tem1;
        tem2*=tem2;
        d1=sqrt(tem1+tem2);//Get a distance of 1,4
	
        tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
        tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
        tem1*=tem1;
        tem2*=tem2;
        d2=sqrt(tem1+tem2);//Get the distance of 2,3
        fac=(float)d1/d2;
        if(fac<0.95||fac>1.05)//Unqualified
        {
          cnt=0;
          TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//Clear point 4
          TP_Drow_Touch_Point(20,20,RED);								//Plot 1
          TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//Display Data   
          continue;
        }//Correct
        //Calculation results
        tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//Get xfac		 
        tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//Get xoff
        
        tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//Get yfac
        tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//Get yoff  
        if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//The touch screen is the opposite of the preset.
        {
          cnt=0;
          TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//Clear point 4
          TP_Drow_Touch_Point(20,20,RED);								//Plot 1
          //LCD_ShowString(40,26, 16,"TP Need readjust!",1);
          tp_dev.touchtype=!tp_dev.touchtype;//Modify touch screen type.
          if(tp_dev.touchtype)//X,Y direction is opposite to screen
          {
            CMD_RDX=0X90;
            CMD_RDY=0XD0;	 
          }else				   //X,Y direction is the same as the screen
          {
            CMD_RDX=0XD0;
            CMD_RDY=0X90;	 
          }			    
          continue;
        }		
        POINT_COLOR=BLUE;
        LCD_Clear(WHITE);//Clear screen
        //LCD_ShowString(35,110, 16,"Touch Screen Adjust OK!",1);//Calibration completed
        delay_ms(1000);
        TP_Save_Adjdata();  
        LCD_Clear(WHITE);//Clear screen   
        return;//Calibration completed				 
      }
    }
    delay_ms(10);
    outtime++;
    if(outtime>1000)
    {
      TP_Get_Adjdata();
      break;
    } 
  }
}		

/*****************************************************************************
* @name       :u8 TP_Init(void)
* @date       :2018-08-09 
* @function   :Initialization touch screen
* @parameters :None
* @retvalue   :0-no calibration
1-Has been calibrated
******************************************************************************/  
u8 TP_Init(void)
{			    		   
  
  GPIO_InitTypeDef GPIO_Initure;	//GPIO
  __HAL_RCC_GPIOE_CLK_ENABLE();			//Turn on the GPIOH clock
  __HAL_RCC_GPIOA_CLK_ENABLE();			//Turn on the GPIOI clock
  
  //SCK
  GPIO_Initure.Pin=GPIO_PIN_1;            //PH6
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //Push-pull output
  GPIO_Initure.Pull=GPIO_PULLUP;          //pull up
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     //high speed
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //initialization
  
  //CS  MOSI
  GPIO_Initure.Pin=GPIO_PIN_5|GPIO_PIN_6; //PI3,8
  HAL_GPIO_Init(GPIOE,&GPIO_Initure);     //initialization
  
  //PEN
  GPIO_Initure.Pin=GPIO_PIN_6;            //PH7
  GPIO_Initure.Mode=GPIO_MODE_INPUT;      //enter
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);     //initialization
  
  //PG3
  GPIO_Initure.Pin=GPIO_PIN_4;            //PG3
  HAL_GPIO_Init(GPIOE,&GPIO_Initure);     //initialization
  
  TP_Read_XY(&tp_dev.x,&tp_dev.y);//First read initialization	 
  if(TP_Get_Adjdata())return 0;//Already calibrated
  else			   //Not calibrated?
  { 										    
    LCD_Clear(WHITE);//Clear screen
    TP_Adjust();  //Screen calibration 
    TP_Save_Adjdata();	 
  }			
  TP_Get_Adjdata();	
  return 1; 									 
}



