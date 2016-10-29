#include "sys.h"
#include "usart.h"		
#include "delay.h"	
//#include "led.h"  
//#include "oled.h"  
//#include "ds18b20.h"   
#include "rc522.h"
//#include "hc05.h"
#include "usart2.h"
#include "string.h"

/*OLED引脚信息   ：RST-PC(13);DC-PB(9);SCL---D0-PC(15);SDA---D1-PC(14);*/
/*DS18B20引脚信息：DQ-PA(0);*/
/*RC522引脚信息  : PB0（片选SDA） 1（复位RST）   PB13(SCK) 14(MISO) 15(MOSI)  RQ悬空*/
/*LED引脚信息    : PA8 PD2*/

u8 writeData[16]={1, 2, 3, 4, 0};
u8 str[16];
u8 UID[4],Temp[4];//--卡的第0扇区第0块;块0的前4个字节是UID（序列号），第5个字节是卡UID的校验位，剩下的是厂商数据。
u8 RF_Buffer[18]                                       			   ;
u8 Password_Buffer[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}              ; // Mifare One 缺省密码
unsigned char s=0x08;

char          MBRX[30]                                             ;
char          MBKeyTP[30]                                          ;
unsigned char RFID[16];	
char          Event                                                ;
u8 DISP_MODE,i                                          		   ; // 编辑控件显示模式
u8 des_on       = 0                                    			   ; // DES加密标志
void Key_TP_Task(void)                                             ;
void display();
void display2();
#define LED0 PAout(5)// PB5
 
/*void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(GPIOB, &GPIO_InitStructure);		
	
}*/
 u8 h1[8];//---定义为8时候最后字符出现乱码
   u8 h2[13]="Welcome Back";
   u8 shuzi[]={"0123456789abcdef"};
	 
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(GPIOA, &GPIO_InitStructure);	
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
	
}
int main(void)
{		
	 char* u;
		 char* e;
	u8 status2;	 
	u8 CT[2]={1,1};
	u8 SN1[4]={0,0,0,0};
u8 reclen=0;
  u8 j=0;
  //Stm32_Clock_Init(9);//系统时钟设置
	delay_init();		//延时初始化
	uart_init(9600); //串口1初始化  
USART2_Init(9600);
LED_Init();
//  HC05_Init();
	//OLED_Init(); 
	//LED_Init();	        //初始化LED-A8(LEDA8) D2(LEDD2)	
	InitRc522();
	PcdReset();			//复位RC522
  PcdAntennaOn();		//开启天线发射 
	printf("INIT\n");
																										//开始并没有发送数据，那么上电之后接收的一大部分数据从哪里来
	while(1)
	{	    	    
		//printf("Request\n");
		//delay_ms(2000);
		//HC05_Set_Cmd("AT+ROLE=0");
		
		delay_ms(200);
	/*	printf("set role\n");
		HC05_Set_Cmd("AT+NAME=simonheheda");   //还是一样的名字，这里没有变
		delay_ms(200);
		printf("set name\n");
		HC05_Set_Cmd("AT+UART=9600,0,0");
		delay_ms(200);
		printf("set bound\n");  */
		//j=HC05_Get_Role();
		//if(j!=2)
		//{ HC05_Set_Cmd("AT+RESET");	LED1=1;delay_ms(200);LED1=0;delay_ms(200);}    //一直闪烁，曾经一次LED0亮，一直在复位，发送的是16进制数，但是不是正确的
		//else 
		
			//LED1=1;
		//delay_ms(2000);
		printf("read card\n");
		status2=PcdRequest(0x52,Temp);////寻卡,输出为卡类型----
		//printf("next\n");
		if(status2==MI_OK) 
		{
			status2 = PcdAnticoll(UID);  //防冲撞处理，输出卡片序列号，4字节--第0扇区第0块前4个字节是UID（序列号）
			/*if(status==MI_OK)
			{
			status=MI_ERR;
				printf("slow\n");
			LED0=1;
			//	delay_ms(2500);
				//LED0=0;
			}*/
			//do{LED0=1;status = PcdAnticoll(UID);}while(status!=MI_OK);
			printf("slow\n");LED0=0;
		}
		if(PcdAnticoll(UID)==MI_ERR)   {LED0=1;}
	else
		LED0=0;
			//LED0=0;
		if(status2==MI_OK) 
		{
		 //LED0=0;
			display();
			status2=PcdRead(s,RFID);   //读取数据
			display2(RFID);
			/* LED0=1;
			delay_ms(1000);
			LED0=0;
			delay_ms(1000);*/
			//delay_ms(5000);
			j=1;
		}
		//status2=MI_ERR;
	 delay_ms(1000);
		//LEDD2=1;LEDA8=1;           //-操作一秒后自动触发高电平
	  
	 if(USART2_RX_STA&0X8000)        //判断是否接收完成  为什么接收不在中断完成
	 {
		 reclen=USART2_RX_STA&0X7FFF;	//得到数据长度    不是USART_RX_STA&0x3fff;吗
		  	USART2_RX_BUF[reclen]=0;	 	//加入结束符
		 printf("%s\n",(const char*)USART2_RX_BUF);
			if(reclen==9||reclen==8) 		//控制DS1检测
			{
				if(strcmp((const char*)USART2_RX_BUF,"ON")==0)LED0=1;	//打开LED1
				if(strcmp((const char*)USART2_RX_BUF,"OFF")==0)LED0=0;//关闭LED1
			}
 			USART2_RX_STA=0;	 
	 }
	 //int t;
	 if(j==1)
	 {
		 j=0;
		 
		// printf("ready to printf\n");   
		 //u2_printf("hello\n");       //
		 //sprintf(u,"%s",h1);                      //在这里加上去之后只能检测一次卡
		 //printf("u sprintf ok\n");
		 //sprintf(e,"%s",RFID);
		// printf("e sprintf ok\n");
		  u2_printf((char*)h1); 
		 
		 //printf("u printf ok\n");
		// u2_printf((char*)e);
	 }
	}
}
void display()
{
  

printf("%s\n",h2);
   h1[0]=shuzi[(UID[0]&0xf0)>>4];//BCD码，再去找对应的数字
   h1[1]=shuzi[UID[0]&0x0f];
   h1[2]=shuzi[(UID[1]&0xf0)>>4];//BCD码，再去找对应的数字
   h1[3]=shuzi[UID[1]&0x0f];
   h1[4]=shuzi[(UID[2]&0xf0)>>4];//BCD码，再去找对应的数字
   h1[5]=shuzi[UID[2]&0x0f];
   h1[6]=shuzi[(UID[3]&0xf0)>>4];//BCD码，再去找对应的数字
   h1[7]=shuzi[UID[3]&0x0f];
   //h1[8]='\n';
  // OLED_Display_On();
   
  //OLED_ShowString(00,10,"Card:"); 
  // OLED_ShowString(50,10,h1);    //--我自己增加的OLED显示数组函数
  //OLED_ShowString(20,50,h2);    //--我自己增加的OLED显示数组函数
   
  // OLED_Refresh_Gram();
printf("Card:\n");
printf("%s\n",h1);
	// sprintf(u,"%s",h1);  //在这里加上去之后只能检测一次卡
//u2_printf(u); 
   
  /*  switch(h1[0])//----仅取一位实验作用
   {
     case '4':LEDA8=0;
              break;          //--显示8个序列号码
     case 'f':LEDD2=0;
              break;       //--显示8个序列号码
    default:;
   }*/
   
}

void display2(u8 *p)
{
	 //显示卡的卡号，以十六进制显示
{
	u8 num[9];
	u8 i;

	for(i=0;i<4;i++)
	{
		num[i*2]=p[i]/16;
		num[i*2]>9?(num[i*2]+='7'):(num[i*2]+='0');
		num[i*2+1]=p[i]%16;
		num[i*2+1]>9?(num[i*2+1]+='7'):(num[i*2+1]+='0');
	}
	num[8]=0;
	//POINT_COLOR=RED;	  
	//LCD_ShowString(x,y,200,16,16,"The Card ID is:");	
	printf("The Card ID is:\n");
	//DisplayString(x,y+16,num,charColor,bkColor);
 //	for(i=0;i<8;i++)
	//{
			printf("%s\n",num);
	//}
}

}















