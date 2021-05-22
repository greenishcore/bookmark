#include <reg51.h>				
#define uchar unsigned char
#define uint  unsigned int				

uchar data digital[4];					//四个数码管显示
uchar data default_ew=20;  			//东西默认设置
uchar data default_ns=30;				//南北默认设置
uchar data dirction_ew=20;			//设置东西方向的时间
uchar data dirction_ns=30;			//设置南北方向的时间
int n;
uchar data countt0,countt1;	//两个定时器中断次数
//定义6组开关

sbit  k1=P3^5;		//时间加
sbit  k2=P3^6;		//时间减
sbit  k3=P3^4;		//确认
sbit  k4=P3^7;      //切换方向
sbit  k5=P3^1;		//禁止
sbit  k6=P1^5;		//夜间模式


sbit red_ns=P2^6;		//南北红灯
sbit yellow_ns=P2^5;	//南北黄灯
sbit green_ns=P2^4;     //南北绿灯

sbit red_ew=P2^3;		//东西红灯
sbit yellow_ew=P2^2;	//东西黄灯
sbit green_ew=P2^1;		//东西绿灯
		
bit dirction=0;				//方向切标志 =1时，南北；=0时，东西
bit ew_ns=0;			//东西南北控制
bit flash=0;			//闪烁
bit night_mode=0;			//夜间黄灯闪烁

uchar code character[11]={	//共阴极字型码
	0x3f,  //--0
	0x06,  //--1
	0x5b,  //--2
	0x4f,  //--3
	0x66,  //--4
	0x6d,  //--5
	0x7d,  //--6
	0x07,  //--7
	0x7f,  //--8
	0x6f,  //--9
	0x00   //--NULL
};
void delay(int ms);			//延时
void key();					//按键扫描
void display();				//显示
void init();   				//开机初始化

void main()
{
	TMOD=0X11;	   				//定时器设置
	TH1=0X3C;		
	TL1=0XB0;
	TH0=0X3C;							//定时器0置初值 0.05S
	TL0=0XB0;
	EA=1;									//开总中断
	ET0=1;								//定时器0中断开启
	ET1=1;					   		//定时器1中断开启
	TR0=1;								//启动定时0
	TR1=0;						 		//关闭定时1
	EX0=1;								//开外部中断0
	EX1=1;								//开外部中断1
    init();							//开机初始化
	P2=0Xc3;							// 开始默认状态，东西绿灯，南北黄灯   
    default_ns=default_ew+5; 			//默认南北通行时间比东西多5秒
	while(1)							  //主循环
	{	 
		key(); 							//按键扫描
		display(); 					//显示程序
	}	
}
void key(void)					//按键扫描
{	 
	if(k1!=1)							
	{
		display();       				//调用显示
		if(k1!=1)							//如果确定按下
		{
		  	TR0=0;	       				//关定时器	
			flash=0;					//闪烁标志位关
			P2=0x00;					//灭显示
			TR1=0;							//启动定时1
			if(dirction==0)					//设置键按下
				dirction_ew++;   		//南北加1S
			else
				dirction_ns++;    		//东西加1S
			if(dirction_ns==100)
				dirction_ns=1;
			if(	dirction_ew==100)
				dirction_ew=1;   		//加到100置1
			default_ns=dirction_ns ; 		//设置的数值赋给东西南北
			default_ew=dirction_ew;			
		  	do
		 	{ 
				display();				 //调用显示，用于延时
			}
			while(k1!=1);			 //等待按键释放
		}
	}

	if(k2!=1)							//当K2(时间减)按键按下时
	{		
		display();       				//调用显示，用于延时消抖  
		if(k2!=1)						//如果确定按下
		{	
		 	TR0=0;         				//关定时器0	
			flash=0;					//闪烁标志位关
			P2=0x00;					//灭显示
			TR1=0;						//关定时器1	
			if(dirction==0)
				dirction_ew--;  			//南北减1S
			else
				dirction_ns--;  			//东西减1S
			if(dirction_ns==0)
				dirction_ns=99;	
			if(	dirction_ew==0 )
				dirction_ew=99;   		//减到1重置99
			default_ns=dirction_ns ;	 		//设置的数值赋给东西南北
			default_ew=dirction_ew;
		  	do	
		    { 
				display();       		//调用显示
			}
			while(k2!=1);			   //等待按键释放
		}
	}

	if(k3!=1)							
	{	
		display();       				//调用显示，用于延时消抖
		if(k3!=1)						//如果确定按下
		{
			TR0=1;  						//启动定时器0
			default_ns=dirction_ns;				//从中断回复，仍显示设置过的数值
			default_ew=dirction_ew;				//显示设置过的时间
			TR1=0;							//关定时器1	
			if(dirction==0)						//时间倒时到0时
			{ 
				P2=0X00;					 //灭显示
				green_ew=1;						//东西绿灯亮
				red_ns=1;					//南北红灯亮
				default_ns=default_ew+5; 			//回到初值
			}
			else 
			{ 
				P2=0x00;					//南北绿灯，东西红灯
				green_ns=1;
				red_ew=1;
				default_ew=default_ns+5; 
			}
		}
	}
	 
	if(k4!=1)							
    {
		display();       				//调用显示，用于延时消抖
		if(k4!=1)						//如果确定按下
		{  
		 	TR0=0;						//关定时器0	
			dirction=!dirction;					//取反dirction标志位，以切换调节方向
			TR1=0;						//关定时器1
			ew_ns=dirction;
			do
			{
				display(); 			   //调用显示，用于延时
			}				
			while(k4!=1);			  //等待按键释放
		}
	}	

	if(k5!=1)							
	{
	   	display();       				//调用显示
	   	if(k5!=1)						//如果确定按下
	   	{ 	
	    	TR0=0;						//关定时器
			P2=0x00;					//灭显示
			red_ew=1;
			red_ns=1;					//全部置红灯
			TR1=0;
			default_ew=00;					//四个方向的时间都为00
			default_ns=00;
			do
			{
				display(); 				//调用显示，用于延时
			}	
			while(k5!=1);			//等待按键释放	
	   	}
	}
	if(k6!=1)							
	{
	   	display();       				//调用显示，用于延时消抖 
	   	if(k6!=1)						//如果确定按下
	   	{ 	
	    	TR0=0;						//关定时器
			P2=0x00;
			TR1=1;
			default_ew=00;					//四个方向的时间都为00
			default_ns=00;
			do
			{
				display(); 			  //调用显示，用于延时
			}	
			while(k6!=1);			//等待按键释放	
	   	}
	}
}
void display(void) //显示程序
{		
	digital[1]=default_ns/10; 		//数码管 东西秒十位
	digital[2]=default_ns%10; 		//数码管 东西秒个位
	digital[3]=default_ew/10; 		//数码管 南北秒十位
	digital[0]=default_ew%10; 		//数码管 南北秒个位		
	P1=0xff;          		// 熄灭
	P0=0x00;				      //熄灭
	P1=0xfe;           		//东西十位数码管
	P0=character[digital[1]];		//东西时间十位的数码管字形			
	delay(1);				//延时
	P1=0xff;				//熄灭
	P0=0x00;				//熄灭
						   
	P1=0xfd;             	//东西个位数码管
	P0=character[digital[2]];		 //东西时间个位的数码管字形
	delay(1);				 //延时
	P1=0xff;				//熄灭
	P0=0x00;				//熄灭
	P1=0Xfb;		  		//南北十位数码管
	P0=character[digital[3]];		//南北时间十位的数码管字形
	delay(1);				//延时
	P1=0xff;				 //熄灭
	P0=0x00;				 //熄灭
	P1=0Xf7;				 //南北个位数码管
	P0=character[digital[0]];		//南北时间个位的数码管字形
	delay(1);				//延时
}

void time0(void) interrupt 1 using 1  	//定时中断子程序
{
	TH0=0X3C;							//重赋初值
	TL0=0XB0;							//12m晶振50ms//重赋初值
	TR0=1;								//重新启动定时器
	countt0++;							//软件计数加1
	if(countt0==10)						//加到10也就是半秒
	{
		if((default_ns<=5)&&(ew_ns==0)&&(flash==1))  		//东西黄灯闪		
        {
			green_ew=0;
			yellow_ew=0;
		}		   				
	    if((default_ew<=5)&&(ew_ns==1)&&(flash==1))  		//南北黄灯闪		
        {  
			green_ns=0;
			yellow_ns=0;
		}	
	}
		
	if(countt0==20)	                  	// 定时器中断次数=20时（即1秒时）
	{	countt0=0;						//清零计数器
		default_ew--;						//东西时间减1
		default_ns--;						//南北时间减1

		if((default_ns<=5)&&(ew_ns==0)&&(flash==1))  		//东西黄灯闪		
        {
			green_ew=0;
			yellow_ew=1;
		}		   				
	    if((default_ew<=5)&&(ew_ns==1)&&(flash==1))  		//南北黄灯闪		
        {  
			green_ns=0;
			yellow_ns=1;
		}		 						
		if(default_ew==0&&default_ns==5) 		//当东西倒计时到0时，重置5秒，用于黄灯闪烁时间   
		{
			default_ew=5;
			flash=1;
		}
		if(default_ns==0&&default_ew==5)		//当南北倒计时到0时，重置5秒，用于黄灯闪烁时间   
		{
			default_ns=5;
			flash=1;
		}
		if(ew_ns==0&&default_ns==0)			//当黄灯闪烁时间倒计时到0时，
		{
			P2=0x00;					//重置东西南背方向的红绿灯
			green_ns=1;
			red_ew=1;
			ew_ns=!ew_ns;
			flash=0;
			default_ns=dirction_ns;			//重赋南北方向的起始值
			default_ew=dirction_ns+5;		//重赋东西方向的起始值
		}		
		if(ew_ns==1&&default_ew==0)			//当黄灯闪烁时间到
		{
			P2=0X00;					//重置东西南北的红绿灯状态
			green_ew=1;					 //东西绿灯亮
			red_ns=1;					 //南北红灯亮
			ew_ns=!ew_ns;				 //取反
			flash=0;					//闪烁
			default_ew=dirction_ew;			//重赋东西方向的起始值
			default_ns=dirction_ew+5;		//重赋南北方向的起始值
		}
	}	
}
void time1(void) interrupt 3 	//定时中断子程序
{
	TH1=0X3C;							//重赋初值
	TL1=0XB0;							//12m晶振50ms//重赋初值
	countt1++;							//软件计数加1
	if(countt1==10)					   // 定时器中断次数=10时（即0.5秒）
	{
			yellow_ns=0;				//南北黄灯灭
			yellow_ew=0;				//东西黄灯灭
	}
	if(countt1==20)	                  	// 定时器中断次数=20时（即1秒时）
	{	countt1=0;						//清零计数器
			yellow_ns=1;					//南北黄灯亮
			yellow_ew=1;			  	//东西黄灯亮
	}	
}


//外部中断0
void int0(void) interrupt 0 using 1	   //只允许东西通行
{
	TR0=0;								//关定时器0
	TR1=0;								//关定时器1
	P2=0x00;							//灭显示
	green_ew=1;							//东西方向置绿灯
	red_ns=1;							//南北方向为红灯
	default_ew=00;							//四个方向的时间都为00
	default_ns=00;	
}

//外部中断1
void int1(void) interrupt 2 using 1	  	 //只允许南北通行 
{
	TR0=0;								//关定时器0
	TR1=0;							   //关定时器1
	P2=0x00;							//灭显示
	green_ns=1;							//置南北方向为绿灯
	red_ew=1;							//东西方向为红灯
	default_ns=00;							//四个方向的时间都为00
	default_ew=00;
}
void init()//开机的初始化
{ 
	for(n=0;n<50;n++)	//循环显示----50次
   	{
	    P0=0x40;  
	    P1=0xfe;	//第一位显示
		delay(1);	//延时
	    P1=0xfd;	//第二位显示
		delay(1);	//延时
		P1=0Xfb;	//第三位显示
		delay(1);	//延时
		P1=0Xf7;	//第四位显示
		delay(1);	//延时
	    P1 = 0xff;	//灭显示
	}
}

void delay(int ms)		//延时程序
{
	uint j,k;
	for(j=0;j<ms;j++)			//延时ms
	   for(k=0;k<124;k++);		//大约1毫秒的延时
}