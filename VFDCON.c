/*
*********************************************************************************************************
*                            VFD-CON R2 Controller Main Functions
*
*                          (c) Copyright 2016, MARUENG co., ltd., Daejeon, South Korea
*                                           All Rights Reserved
*
* File : VFDCON.C
* By   : DongHyun Kwon
*********************************************************************************************************

*********************************************************************************************************
  ATMEGA128 Fuse Bits
  /BootLock12 /BootLock11  /BootLock02  /BootLock01  /Lock2 /Lock1
                       /M103C  /WDTON
  /OCDEN  /JTAGEN *CKOPT  *EESAVE /BOOTSZ1  /BOOTSZ0  /BOOTRST
  *BODLEVEL /BODEN  *SUT1 /SUT0 /CKSEL3 /CKSEL2 /CKSEL1 /CKSEL0
*********************************************************************************************************      
*/

// External 11.0592MHz


#define Led_Delay	1
#define Serial_Max	1000



#include <avr/io.h>
#include <VFDCON_Sub.h>
#include <avr/interrupt.h>
#include <avr/math.h>
#include <avr/signal.h>
#include <string.h>
#include <stdlib.h>


unsigned int  i,j,Time_Count=10;
unsigned char Key_Input,Key_Flag=0;

unsigned int Key_Count;
unsigned char Time_Flag,Led_Count,Led_Delay_Count;
unsigned char Key_Toggle;

unsigned char KEY_BUF[14],Check_Sum,Key_Data;


unsigned int 	i_count,j_count,Buf_Start_Count,Read_Length,Head_Count,Serial_Count1,Buf_Start_Count;
//unsigned char RxD_BUF1[Serial_Max],RxD_Info_BUF[54],Serial_Flag,Serial_OK_Flag1,Ser_Check_Sum;
unsigned char RxD_BUF1[Serial_Max],RxD_Info_BUF[94],Serial_Flag,Serial_OK_Flag1,Ser_Check_Sum;
unsigned char Serial_Reset_Count1;
unsigned char LED_Data;
unsigned char LCD_LINE1_BUF[20],LCD_LINE2_BUF[20],LCD_LINE3_BUF[20],LCD_LINE4_BUF[20];

unsigned char Key_Push_Count,UHD_Enable,UHD_Key_Flag,Reset_Key_Count;


unsigned char LCD_LINE1_TEMP[20],LCD_LINE2_TEMP[20],LCD_LINE3_TEMP[20],LCD_LINE4_TEMP[20];
unsigned char LCD_Serial_Time_Count,Temp_Buf_Count;

unsigned char Flow_Count;

unsigned char System_Fault_Count,System_Run_Okay;

unsigned char LCD_Replay_Count;




SIGNAL(SIG_UART0_RECV)
{

}	// end of SIGNAL(SIG_UART0_RECV)



SIGNAL(SIG_UART1_RECV)
{
	RxD_BUF1[Serial_Count1]=UDR1;
//	TX0_char(UDR1);
	Serial_Count1++;
	if(Serial_Count1 > Serial_Max)
	{
		Serial_Count1=0;
	}
	Serial_Reset_Count1=0;
	Serial_Flag=1;
	
	//LCD_Serial_Time_Count=0;

}	// end of SIGNAL(SIG_UART1_RECV)



SIGNAL(SIG_OUTPUT_COMPARE1A)            //OC1A interrupt function 
{
  Time_Flag=1;
	PORTE=PORTE^0x08;
	//Serial_Reset_Count1 ++;
	LCD_Serial_Time_Count ++;
	
	if(System_Run_Okay ==1)
	{
		System_Fault_Count++;
	}
	
	if(Serial_Reset_Count1 >= 20){	Serial_Reset_Count1=20; }
	if(LCD_Serial_Time_Count >= 250){	LCD_Serial_Time_Count= 250; }
	if(System_Fault_Count >= 200){	System_Fault_Count= 200; }


}	// end of SIGNAL(SIG_OUTPUT_COMPARE1A)




void Serial1_Buf_Processing(void)
{
	if((Serial_Flag ==1)&& (Serial_Count1 >= 94))
	{
		for(Head_Count=Buf_Start_Count;Head_Count<Serial_Count1;Head_Count++)
		{
			if((RxD_BUF1[Head_Count] == 0x3A) && (RxD_BUF1[Head_Count+1] == 'T') && (RxD_BUF1[Head_Count+2] == 'C') && (RxD_BUF1[Head_Count+3] == 'S') && (RxD_BUF1[Head_Count+52] == 0x0D) && (RxD_BUF1[Head_Count+53] == 0x0A)&& (Serial_Count1 >= 94))
			{
				for(j_count=0;j_count<94;j_count++)
				{
					RxD_Info_BUF[j_count]=RxD_BUF1[Head_Count+j_count];
					RxD_BUF1[Head_Count+j_count]=0x00;
				}
				Serial_OK_Flag1 =1;
				Buf_Start_Count = Head_Count + 94;
				Head_Count =Serial_Max;
				
			}else{
				
				//Serial_Count1 =0;
				
			}
		}

	}//if(Serial_Flag ==1)
}//void Serial1_Buf_Processing(void)


void Serial1_Check(void)
{
	if(Serial_OK_Flag1 ==1)
	{
		Serial_OK_Flag1=0;		

		if((RxD_Info_BUF[0] == 0x3A)&&(RxD_Info_BUF[1] == 'T')&&(RxD_Info_BUF[2] =='C')&&(RxD_Info_BUF[3] =='S'))//TCS Setting
		{
			sbi(PORTE,2);
			
			Serial_Reset_Count1=0;
			System_Fault_Count =0;
			System_Run_Okay =1;
			
			
			LCD_Serial_Time_Count =0;
			Temp_Buf_Count =0;
			
			Ser_Check_Sum=0;
			
			for(i=0;i<91;i++)
			{
				Ser_Check_Sum = Ser_Check_Sum+RxD_Info_BUF[i];
			}
	
			if(Ser_Check_Sum == RxD_Info_BUF[91])
			{
				//Beep();
				LED_Data = RxD_Info_BUF[5];
				for(i=0;i<20;i++)
				{
					LCD_LINE1_BUF[i]=RxD_Info_BUF[6+i];
					LCD_LINE2_BUF[i]=RxD_Info_BUF[26+i];
					LCD_LINE3_BUF[i]=RxD_Info_BUF[46+i];
					LCD_LINE4_BUF[i]=RxD_Info_BUF[66+i];

				}

				if((LED_Data & 0x01) == 0x01){	sbi(PORTB,0);	}else{	cbi(PORTB,0);	}	
				if((LED_Data & 0x02) == 0x02){	sbi(PORTB,1);	}else{	cbi(PORTB,1);	}	
				if((LED_Data & 0x04) == 0x04){	sbi(PORTB,2);	}else{	cbi(PORTB,2);	}	
				if((LED_Data & 0x08) == 0x08){	sbi(PORTB,3);	}else{	cbi(PORTB,3);	}	
				if((LED_Data & 0x10) == 0x10){	sbi(PORTB,4);	}else{	cbi(PORTB,4);	}	
				if((LED_Data & 0x20) == 0x20){	sbi(PORTB,5);	}else{	cbi(PORTB,5);	}	

	
				//LCD Line 1 Display
				LCD_Position(0x00);				// start position of string
				for(i=0;i<20;i++)
				{
					LCD_data(LCD_LINE1_BUF[i]);
				}
	
				//LCD Line 2 Display
				LCD_Position(0x14);				// start position of string
				for(i=0;i<20;i++)
				{
					LCD_data(LCD_LINE2_BUF[i]);
				}

				//LCD Line 3 Display
				LCD_Position(0x28);				// start position of string
				for(i=0;i<20;i++)
				{
					LCD_data(LCD_LINE3_BUF[i]);
				}
	
				//LCD Line 4 Display
				LCD_Position(0x3C);				// start position of string
				for(i=0;i<20;i++)
				{
					LCD_data(LCD_LINE4_BUF[i]);
				}
	
			}
			
			cbi(PORTE,2);

			

		}

		//Serial_Count1 =0;
		for(i=0;i<94;i++){	RxD_Info_BUF[i]=0x00;	}
		Serial_Reset_Count1=0;
		
	}
	
}//void Serial1_Check(void)


int main(void)
{ 

  MCU_initialize(); // initialize MCU
  Delay_ms(500);

  sbi(PORTE,1);//sbi(C2-disable),cbi(C2-enable)
  Delay_ms(100);
  cbi(PORTE,1);//sbi(C2-disable),cbi(C2-enable)
  
  PORTD	= 0x00;
	PORTB	= 0xFF;
	
	Beep();
	
	
	LCD_Reset();
	Delay_ms(10);
	

	LCD_Position(0x00);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	
	LCD_Position(0x14);
	LCD_data(0x7F);LCD_data(0x7F);
	LCD_string(0x16,"  SYSTEM START  ");
	LCD_data(0x7F);LCD_data(0x7F);

	LCD_Position(0x28);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(' ');LCD_data(' ');
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(' ');LCD_data(' ');LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	
	LCD_Position(0x3C);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);LCD_data(0x7F);
	LCD_data(0x7F);LCD_data(0x7F);

 	PORTD	= 0xF3;
 	Delay_ms(500);
  PORTD	= 0x00;	

// Timer 1 setting
  TCCR1A = 0x00;                                // CTC mode(4)
  TCCR1B = 0x0C;                                // 11.0592MHz/256/(1+21599) = 2Hz       4319 = 10Hz   43199 = 1Hz
  OCR1AH = (4319  >> 8);
  OCR1AL = 4319 & 0xFF;
  TCNT1H = 0x00;                                // clear Timer/Counter1
  TCNT1L = 0x00;
  TIMSK	 = 0x10;                                 // enable OC1A interrupt
  TIFR	 = 0x00;                                  // clear all interrupt flags

/*
// USART0 Setting 
  UCSR0A = 0x00;
  UCSR0B = 0x98;
  UCSR0C = 0x06;				// 8data, no parity, 1 stop
  UBRR0H = (5 >> 8);				// 19200(35) 115200(5)baud rate
  UBRR0L = 5 & 0xFF;
*/


// USART1 Setting 
  UCSR1A = 0x00;
  UCSR1B = 0x98;
  UCSR1C = 0x06;				// 8data, no parity, 1 stop
  UBRR1H = (5 >> 8);				// 19200(35) 115200(5)baud rate
  UBRR1L = 5 & 0xFF;


	Led_Count=0;

	Beep();
	Beep();

 	Key_Toggle=0;
 	Reset_Key_Count=0;
 	System_Fault_Count =0;
 	System_Run_Okay =0;
 	
 	
 	PORTB=0x00;
	PORTE=0x0C;

	Key_Data=0x30;

///KEY_BUF
	KEY_BUF[0]=0x3A;KEY_BUF[1]=0x4B;KEY_BUF[2]=0x45;KEY_BUF[3]=0x59;KEY_BUF[4]=0x06;
	KEY_BUF[5]=Key_Data;
	KEY_BUF[6]=0x00;KEY_BUF[7]=0x00;KEY_BUF[8]=0x00;KEY_BUF[9]=0x00;KEY_BUF[10]=0x00;

	Check_Sum =0;
	for(i=0;i<11;i++)
	{
		Check_Sum = Check_Sum+KEY_BUF[i];
	}
	
	KEY_BUF[11]=Check_Sum;// Check Sum
	KEY_BUF[12]=0x0D;KEY_BUF[13]=0x0A;
	
	//for(i=0;i<14;i++){		TX1_char(KEY_BUF[i]);}
	
///KEY_BUF

	LCD_string(0x00,"Model:MDE-80H       ");
	LCD_string(0x14,"Ensemble Multiplexer");
	LCD_string(0x28,"      Ver:2018.05.18");
	LCD_string(0x3C,"--------------------");



  sei();
	
while(1)
	{

	Delay_ms(5);
	// 0x1D-Up, 0x17-Down, 0x1E-Left, 0x1B-Right, 0x0F-Enter
	Key_Input= inp(PINF) & 0x1F;

	if(Key_Input != 0x1E)//Enter(1RU)
	{
		Key_Push_Count=0;
	}

	if(Key_Input != 0x0B)//Left+Right(1RU)
	{
		Reset_Key_Count=0;
		
	}else if(Key_Input == 0x0B){//Left+Right(1RU)

		sbi(PORTD,1);
		sbi(PORTD,4);
		cbi(PORTD,5);
		sbi(PORTD,6);
		cbi(PORTD,7);
	}

	if(Key_Input == 0x1F)
	{
		Key_Flag=0;
		//Key_Push_Count=0;
		cbi(PORTD,1);
		cbi(PORTD,4);
		cbi(PORTD,5);
		cbi(PORTD,6);
		cbi(PORTD,7);
		
	}else{

		if(Key_Flag ==0)
		{

				LCD_Reset();
				Delay_ms(1);
				
				LCD_string(0x00,"Model:MDE-80H       ");
				LCD_string(0x14,"Ensemble Multiplexer");
				LCD_string(0x28,"      Ver:2018.05.18");
				LCD_string(0x3C,"--------------------");

			if(Key_Input == 0x1E)//Enter(1RU)
			{
				
				cbi(PORTD,1);
				sbi(PORTD,4);
				sbi(PORTD,5);
				sbi(PORTD,6);
				sbi(PORTD,7);

				//Beep();
				Key_Push_Count=0;
				Key_Count=0;
				Led_Count=0;
				Key_Flag=1;
				
				Key_Data =0x31;
				
				///KEY_BUF
				KEY_BUF[0]=0x3A;KEY_BUF[1]=0x4B;KEY_BUF[2]=0x45;KEY_BUF[3]=0x59;KEY_BUF[4]=0x06;
				KEY_BUF[5]=Key_Data;
				KEY_BUF[6]=0x00;KEY_BUF[7]=0x00;KEY_BUF[8]=0x00;KEY_BUF[9]=0x00;KEY_BUF[10]=0x00;
			
				Check_Sum =0;
				for(i=0;i<11;i++)
				{
					Check_Sum = Check_Sum+KEY_BUF[i];
				}
				
				KEY_BUF[11]=Check_Sum;// Check Sum
				KEY_BUF[12]=0x0D;KEY_BUF[13]=0x0A;
				
				for(i=0;i<14;i++){		TX1_char(KEY_BUF[i]);}
				
				LCD_Serial_Time_Count =0;
				Temp_Buf_Count =0;
				
				///KEY_BUF
				
			}else if(Key_Input == 0x1D){//Up(1RU)

				sbi(PORTD,1);
				cbi(PORTD,4);
				sbi(PORTD,5);
				sbi(PORTD,6);
				sbi(PORTD,7);
	
				//Beep();
				Key_Count=0;
				Led_Count=0;
				Key_Flag=1;
				Key_Data =0x32;
				
				///KEY_BUF
				KEY_BUF[0]=0x3A;KEY_BUF[1]=0x4B;KEY_BUF[2]=0x45;KEY_BUF[3]=0x59;KEY_BUF[4]=0x06;
				KEY_BUF[5]=Key_Data;
				KEY_BUF[6]=0x00;KEY_BUF[7]=0x00;KEY_BUF[8]=0x00;KEY_BUF[9]=0x00;KEY_BUF[10]=0x00;
			
				Check_Sum =0;
				for(i=0;i<11;i++)
				{
					Check_Sum = Check_Sum+KEY_BUF[i];
				}
				
				KEY_BUF[11]=Check_Sum;// Check Sum
				KEY_BUF[12]=0x0D;KEY_BUF[13]=0x0A;
				
				for(i=0;i<14;i++){		TX1_char(KEY_BUF[i]);}
				
				LCD_Serial_Time_Count =0;
				Temp_Buf_Count =0;
				///KEY_BUF

			
			}else if(Key_Input == 0x1B){//Left(1RU)

				sbi(PORTD,1);
				sbi(PORTD,4);
				cbi(PORTD,5);
				sbi(PORTD,6);
				sbi(PORTD,7);
	
				//Beep();
				Key_Count=0;
				Led_Count=0;
				Key_Flag=1;
				Key_Data =0x33;
				
				///KEY_BUF
				KEY_BUF[0]=0x3A;KEY_BUF[1]=0x4B;KEY_BUF[2]=0x45;KEY_BUF[3]=0x59;KEY_BUF[4]=0x06;
				KEY_BUF[5]=Key_Data;
				KEY_BUF[6]=0x00;KEY_BUF[7]=0x00;KEY_BUF[8]=0x00;KEY_BUF[9]=0x00;KEY_BUF[10]=0x00;
			
				Check_Sum =0;
				for(i=0;i<11;i++)
				{
					Check_Sum = Check_Sum+KEY_BUF[i];
				}
				
				KEY_BUF[11]=Check_Sum;// Check Sum
				KEY_BUF[12]=0x0D;KEY_BUF[13]=0x0A;
				
				for(i=0;i<14;i++){		TX1_char(KEY_BUF[i]);}
				
				LCD_Serial_Time_Count =0;
				Temp_Buf_Count =0;
				///KEY_BUF
								
			}else if(Key_Input == 0x17){//Down(1RU)

				sbi(PORTD,1);
				sbi(PORTD,4);
				sbi(PORTD,5);
				cbi(PORTD,6);
				sbi(PORTD,7);
	
				//Beep();
				Key_Count=0;
				Led_Count=0;
				Key_Flag=1;
				Key_Data =0x34;
				
				///KEY_BUF
				KEY_BUF[0]=0x3A;KEY_BUF[1]=0x4B;KEY_BUF[2]=0x45;KEY_BUF[3]=0x59;KEY_BUF[4]=0x06;
				KEY_BUF[5]=Key_Data;
				KEY_BUF[6]=0x00;KEY_BUF[7]=0x00;KEY_BUF[8]=0x00;KEY_BUF[9]=0x00;KEY_BUF[10]=0x00;
			
				Check_Sum =0;
				for(i=0;i<11;i++)
				{
					Check_Sum = Check_Sum+KEY_BUF[i];
				}
				
				KEY_BUF[11]=Check_Sum;// Check Sum
				KEY_BUF[12]=0x0D;KEY_BUF[13]=0x0A;
				
				for(i=0;i<14;i++){		TX1_char(KEY_BUF[i]);}
				LCD_Serial_Time_Count =0;
				Temp_Buf_Count =0;
				///KEY_BUF

			}else if(Key_Input == 0x0F){//Right(1RU)

				sbi(PORTD,1);
				sbi(PORTD,4);
				sbi(PORTD,5);
				sbi(PORTD,6);
				cbi(PORTD,7);
	
				//Beep();
				Key_Count=0;
				Led_Count=0;
				Key_Flag=1;
				Key_Data =0x35;
				
				///KEY_BUF
				KEY_BUF[0]=0x3A;KEY_BUF[1]=0x4B;KEY_BUF[2]=0x45;KEY_BUF[3]=0x59;KEY_BUF[4]=0x06;
				KEY_BUF[5]=Key_Data;
				KEY_BUF[6]=0x00;KEY_BUF[7]=0x00;KEY_BUF[8]=0x00;KEY_BUF[9]=0x00;KEY_BUF[10]=0x00;
			
				Check_Sum =0;
				for(i=0;i<11;i++)
				{
					Check_Sum = Check_Sum+KEY_BUF[i];
				}
				
				KEY_BUF[11]=Check_Sum;// Check Sum
				KEY_BUF[12]=0x0D;KEY_BUF[13]=0x0A;
				
				for(i=0;i<14;i++){		TX1_char(KEY_BUF[i]);}
				LCD_Serial_Time_Count =0;
				Temp_Buf_Count =0;
				///KEY_BUF
				
			}
							
		}
  }

	if(Time_Flag ==1)
	{
		Time_Flag=0;
		Time_Count++;
		Key_Push_Count++;
		Reset_Key_Count++;
 		Key_Count++;
 		Serial_Reset_Count1 ++;
 		LCD_Replay_Count++;
 		
 		
 		if(Key_Push_Count == 30)
 		{
			Beep_Long();
			System_Run_Okay =0;
			System_Fault_Count=0;
 	
 		}

 		if(LCD_Replay_Count == 50)
 		{
			LCD_Replay_Count=0;
			//LCD_Reset();
			//Delay_ms(1);
			
			LCD_string(0x00,"Model:MDE-80H       ");
			LCD_string(0x14,"Ensemble Multiplexer");
			LCD_string(0x28,"      Ver:2018.05.18");
			LCD_string(0x3C,"--------------------"); 			
 		}


		if((Reset_Key_Count == 30))
 		{
			Beep_Long();

			System_Run_Okay =0;
			System_Fault_Count=0;

 		}
 		
 		if(Key_Push_Count > 100)
 		{
				Key_Push_Count =100;
 		}
 
		if(Reset_Key_Count > 100)
 		{
				Reset_Key_Count =100;
 		} 		
 		
 		if(Key_Count >= 20)
 		{
	 		Led_Delay_Count++;
			Key_Count=20;
 			
 			if(Led_Delay_Count >=2)
 			{			
	 			Led_Delay_Count=0;

	 		}
 	
 		}else{

 		}


		if(Time_Count >= 1)
		{
			
			Time_Count = 0;
			
		}
		
	}


	Serial1_Buf_Processing();
	Serial1_Check();

	if(Serial_Reset_Count1 == 2)
	{
		//cbi(PORTE,2);
		Serial_Count1 =0;
		Buf_Start_Count =0;
		Serial_Flag =0;
		Serial_Reset_Count1=10;

		//for(i_count=0;i_count<Serial_Max;i_count++){	RxD_BUF1[i_count]=0x00;	}	
		//sbi(PORTE,2);
	}	

	}		// end of while
}			// end of main





