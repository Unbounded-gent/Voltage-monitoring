#include <STC15F2K60S2.h>
#include <intrins.h>
#include <iic.h>
#define uint unsigned int
#define uchar unsigned char

uchar code t_display[]={ //   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
    //black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00, 0x40, 0x76, 0x1E, 0x70, 0x38, 0x37, 0x5C, 0x73, 0x3E, 0x78, 0x3d, 0x67, 0x50, 0x37, 0x6e,
    0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF, 0x46};         //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1
uchar code t_address[]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};//位选
uchar table[8]={16,16,16,16,16,16,16,16};
uchar Trg,Cont;
uchar mode;
uint count,err;
uint AD;
uchar Vp;
uchar key_flag,read_flag,led_flag;
uint temp;

//初始关闭继电器蜂鸣器
void init() {
	P2=0xa0;P0=0;P2=0x00;
	P2=0x80;P0=0xff;P2=0x00;
}

void Timer0Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

void Timer1Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x40;		//定时器时钟1T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xCD;		//设置定时初值
	TH1 = 0xD4;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	ET1=1;
}

//矩阵键盘三行代码
void key_read()
	{
		uchar keypress,ReadData;
		P3=0xf0;
		P42=1;P44=1;
		P36=P42;
		P37=P44;
		keypress=P3 & 0xf0;//暂存行值
		P3=0x0f;
		P42=0;P44=0;
		ReadData=(P3|keypress)^0xff;
		Trg=ReadData&(ReadData^Cont);
		Cont=ReadData;
}

void time_0() interrupt 1
{
	static uint i,smg_count,key_count,read_count;
	if(++smg_count>=2)	//动态显示
	{
		smg_count=0;
		P2=0xc0;P0=0x00;P2=0x00;
		P2=0xe0;P0=~t_display[table[i]];P2=0x00;
		P2=0xc0;P0=t_address[i];P2=0x00;
		if(++i>=8)
			i=0;
	}
	if(++key_count>=10)	//按键检测
	{
		key_count=0;
		key_flag=1;
	}
	if(++read_count>=100)	//存储器读取
	{
		read_count=0;
		read_flag=1;
	}
}

void time_1() interrupt 3	//L1灯状态触发条件判断
{
	static uint time_count;
	if(AD<Vp*10)
	{
		time_count++;
		if(time_count>=5000)
			led_flag=1;
	}
	else
	{
		time_count=0;
		led_flag=0;
	}
}

void Led_123()	//LED灯的显示处理
{
	if(led_flag==1)
	{P2 = 0x80;P0 = 0xfe;P2 = 0x00;}
	else
		{P2 = 0x80;P0 = 0xff;P2 = 0x00;}
	
	if(err>=3)
		{P2 = 0x80;P0 = 0xfb;P2 = 0x00;}
	else
		{P2 = 0x80;P0 = 0xff;P2 = 0x00;}
	
	if(count%2==1)
	{P2 = 0x80;P0 = 0xfd;P2 = 0x00;}
	else
	{P2 = 0x80;P0 = 0xff;P2 = 0x00;}
}

void Count_num()		//判断计数的触发条件
{
	static uchar state;
	switch(state)
	{
		case 0:
			if(AD>Vp*10)
			{
				state=1;
			}
			break;
		case 1:
			if(AD<=Vp*10)
			{
				count++;
				state=0;
			}
			break;
	}
}

void main()
{
	init();
	AD_Init();
	Vp=read_24C02(0);
	AD_Get();
	Timer0Init();
	Timer1Init();
	while(1)
	{
		if(key_flag==1)
		{
			key_flag=0;
			Count_num();
			key_read();
				if(Trg==0x28)	//S12-改状态
				{
					err=0;
					mode++;
					if(mode==2)
						write_24C02(0,Vp);
					if(mode>=3)
						mode=0;
				}
				if(Trg==0x24)	//S13-清0
				{
					if(mode==2)
					{
						err=0;
						count=0;
					}
					else
					{
						err++;
					}
				}
				if(Trg==0x18) //S16-加法
				{
					if(mode==1)
					{
						err=0;
						Vp=Vp+5;
						if(Vp>50)
							Vp=0;
					}
					else
					{
						err++;
					}
				}
				if(Trg==0x14) //S17-减法
				{
					if(mode==1)
					{
						err=0;
						Vp=Vp-5;
						if(Vp<0||Vp>50) //<0之后其实值大于50.所以有两个条件
							Vp=50;
					}
					else
					{
						err++;
					}
				}
			}
			if(read_flag==1)	//取电压值
			{
				read_flag=0;
				EA=0;
				temp=AD_Get();
				EA=1;
				AD=temp/255.0*500;
			}
			
			switch(mode)	//不同模式下的显示
			{
				case 0:
					table[0]=25;
					table[1]=table[2]=table[3]=table[4]=16;
					table[5]=AD/100+32;
					table[6]=AD%100/10;
					table[7]=AD%10;
					break;
				case 1:
					table[0]=24;
					table[1]=table[2]=table[3]=table[4]=16;
					table[5]=Vp/10+32;
					table[6]=Vp%10;
					table[7]=0;
					break;
				case 2:
					table[0]=22;
					table[1]=table[2]=16;
					table[3]=(count/10000==0)?16:count/10000;
					table[4]=(count%10000/1000==0)?16:count%10000/1000;
					table[5]=(count%1000/100==0)?16:count%1000/100;
					table[6]=(count%100/10==0)?16:count%100/10;
					table[7]=count%10;
					break;
			}
			Led_123();	//LED灯状态判断
		}
	}