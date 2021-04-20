/*
*********************************************************************************************************
*                            VFD-CON Controller User Functions
*
*                          (c) Copyright 2017, MARUENG Co., Ltd., Daejeon, South Korea
*                                           All Rights Reserved
*
* File : VFDCON_Sub.h
* By   : DongHyun Kwon
*********************************************************************************************************
*/

#define LCD_DATABUS	*((unsigned char *)0x2000)	
#define LCD_CONTROL	*((unsigned char *)0x2100)


void MCU_initialize(void)			/* initialize ATmege128 MCU */
{ 
  //MCUCR	= 0x80;	// enable external memory and I/O
  //XMCRA = 0x44;	// 0x1100-0x7FFF=1 wait, 0x8000-0xFFFF=0 wait
  //XMCRB = 0x80;	// enable bus keeper, use A8~A15 as address


	DDRA	= 0xFF;
	PORTA	= 0x00;

	DDRB	= 0xFF;
	PORTB	= 0x00;

	DDRC	= 0x00;
	PORTC	= 0x00;


	DDRD	= 0xF3;
	PORTD	= 0xF3;

	DDRE	= 0xFE;
	PORTE	= 0x00;

  DDRF	= 0x00;
  PORTF	= 0xFF;


  DDRG	= 0x13;
  PORTG	= 0x03;

}


void Delay_nop(void)		/* time delay for us */
{ 

}


void Delay_us(unsigned char time_us)		/* time delay for us */
{ register unsigned char i;

  for(i = 0; i < time_us; i++)			// 4 cycle +
    { asm volatile(" PUSH  R0 ");		// 2 cycle +
      asm volatile(" POP   R0 ");		// 2 cycle +
//    asm volatile(" PUSH  R0 ");		// 2 cycle +
//    asm volatile(" POP   R0 ");		// 2 cycle +
      asm volatile(" PUSH  R0 ");		// 2 cycle +
      asm volatile(" POP   R0 ");		// 2 cycle = 16 cycle = 1 us for 16MHz
    }
}


void Delay_ms(unsigned int time_ms)		/* time delay for ms */
{ register unsigned int i;

  for(i = 0; i < time_ms; i++)
    { Delay_us(250);
      Delay_us(250);
      Delay_us(250);
      Delay_us(150);
    }
}



void TX0_char(unsigned char data)		/* transmit a character by USART0 */
{
  while((UCSR0A & 0x20) == 0x00);		// data register empty ?
  UDR0 = data;
}


void TX1_char(unsigned char data)		/* transmit a character by USART1 */
{
  while((UCSR1A & 0x20) == 0x00);		// data register empty ?
  UDR1 = data;
}


void LCD_Reset(void)		/* write a command(instruction) to text LCD */
{
	cbi(PORTG,1); // VFD_nSEL
	cbi(PORTG,0); // VFD_nWR
	PORTA = 0x1F;
	Delay_us(1);
	sbi(PORTG,0); // VFD_nWR
	sbi(PORTG,1); // VFD_nSEL
	PORTA = 0x00;
	while((inp(PING) & 0x08) == 0x08); // LCD Texe Time

}


void LCD_Position(unsigned char command)		/* write a command(instruction) to text LCD */
{
	cbi(PORTG,1); // VFD_nSEL
	cbi(PORTG,0); // VFD_nWR
	PORTA = 0x10;
	Delay_us(1);
	sbi(PORTG,0); // VFD_nWR
	sbi(PORTG,1); // VFD_nSEL
	while((inp(PING) & 0x08) == 0x08); // LCD Texe Time
	
	cbi(PORTG,1); // VFD_nSEL
	cbi(PORTG,0); // VFD_nWR
	PORTA = command;
	Delay_us(1);
	sbi(PORTG,0); // VFD_nWR
	sbi(PORTG,1); // VFD_nSEL
	while((inp(PING) & 0x08) == 0x08); // LCD Texe Time

}

void LCD_command(unsigned char command)		/* write a command(instruction) to text LCD */
{
	cbi(PORTG,1); // VFD_nSEL
	cbi(PORTG,0); // VFD_nWR
	PORTA = command;
	Delay_us(1);
	sbi(PORTG,0); // VFD_nWR
	sbi(PORTG,1); // VFD_nSEL
	while((inp(PING) & 0x08) == 0x08); // LCD Texe Time
}


void LCD_data(unsigned char data)		/* display a character on text LCD */
{
	cbi(PORTG,1); // VFD_nSEL
	cbi(PORTG,0); // VFD_nWR
	if(data < 0x20)
	{
		PORTA = 0x20;
	}else{
		PORTA = data;
	}
	Delay_us(1);
	sbi(PORTG,0); // VFD_nWR
	sbi(PORTG,1); // VFD_nSEL
	while((inp(PING) & 0x08) == 0x08); // LCD Texe Time
	
}

void LCD_string(unsigned char command, unsigned char *string) /* display a string on LCD */
{
  LCD_Position(command);				// start position of string
  while(*string != '\0')			// display string
    { LCD_data(*string);
       string++;
    }
}


void Beep(void)
{
	sbi(PORTG,4);
	Delay_ms(50);
	cbi(PORTG,4);
}

void Beep_Long(void)
{
	sbi(PORTG,4);
	Delay_ms(100);
	cbi(PORTG,4);
}


unsigned char EEPROM_read(unsigned int address) /* read from EEPROM */
{
  while(EECR & (1<<EEWE));                      // if EEWE = 1, wait
  EEAR = address;                               // if EEWE = 0, read
  EECR |= (1<<EERE);                            // EERE = 1
  return EEDR;                                  // return data
}

void EEPROM_write(unsigned int address, unsigned char byte) /* write to EEPROM */
{
  while(EECR & (1<<EEWE));                      // if EEWE = 1, wait
  EEAR = address;                               // if EEWE = 0, write
  EEDR = byte;
  EECR |= (1<<EEMWE);                           // EEMWE = 1
  EECR |= (1<<EEWE);                            // EEWE = 1
}


