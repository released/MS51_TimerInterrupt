/*_____ I N C L U D E S ____________________________________________________*/
#include "MS51_16K.h"

#include	"project_config.h"



/*_____ D E C L A R A T I O N S ____________________________________________*/
volatile uint8_t u8TH0_Tmp = 0;
volatile uint8_t u8TL0_Tmp = 0;

volatile uint8_t u8TH1_Tmp = 0;
volatile uint8_t u8TL1_Tmp = 0;

volatile uint8_t u8TH2_Tmp = 0;
volatile uint8_t u8TL2_Tmp = 0;

//UART 0
bit BIT_TMP;
bit BIT_UART;
bit uart0_receive_flag=0;
unsigned char uart0_receive_data;

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;

/*_____ M A C R O S ________________________________________________________*/
#define SYS_CLOCK 								(24000000ul)


/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , Enable);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("compare_buffer finish \r\n");	
		set_flag(flag_error , Disable);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}



void send_UARTString(uint8_t* Data)
{
	#if 1
	uint16_t i = 0;

	while (Data[i] != '\0')
	{
		#if 1
		SBUF = Data[i++];
		#else
		UART_Send_Data(UART0,Data[i++]);		
		#endif
	}

	#endif

	#if 0
	uint16_t i = 0;
	
	for(i = 0;i< (strlen(Data)) ;i++ )
	{
		UART_Send_Data(UART0,Data[i]);
	}
	#endif

	#if 0
    while(*Data)  
    {  
        UART_Send_Data(UART0, (unsigned char) *Data++);  
    } 
	#endif
}

void send_UARTASCII(uint16_t Temp)
{
    uint8_t print_buf[16];
    uint16_t i = 15, j;

    *(print_buf + i) = '\0';
    j = (uint16_t)Temp >> 31;
    if(j)
        (uint16_t) Temp = ~(uint16_t)Temp + 1;
    do
    {
        i--;
        *(print_buf + i) = '0' + (uint16_t)Temp % 10;
        (uint16_t)Temp = (uint16_t)Temp / 10;
    }
    while((uint16_t)Temp != 0);
    if(j)
    {
        i--;
        *(print_buf + i) = '-';
    }
    send_UARTString(print_buf + i);
}


void GPIO_Init(void)
{
	P12_PUSHPULL_MODE;		
	P11_PUSHPULL_MODE;		
	P30_PUSHPULL_MODE;	
}

void set_interrupt_priority(void)
{
	set_IP_PT0;
	set_IPH_PT0H;

	set_IP_PT1;
	set_IPH_PT1H;
	set_TCON_TR1;
 		
	set_EIP_PT2;
	set_EIPH_PT2H;
}

void Timer2_IRQHandler(void)
{
	P30 = ~P30;
}

void Timer2_ISR(void) interrupt 5
{
    _push_(SFRS);	

    TH2 = u8TH2_Tmp;
    TL2 = u8TL2_Tmp;
	
    clr_T2CON_TF2;
	
	Timer2_IRQHandler();

    _pop_(SFRS);	
}

void TIMER2_Init(void)
{
	uint16_t res = 0;

	/*
		formula : 16bit 
		(0xFFFF+1 - target)  / (24MHz/psc) = time base 

	*/	
	const uint16_t target_freq = 100;

	TIMER2_DIV_128;
	TIMER2_Auto_Reload_Delay_Mode;
	
	u8TH2_Tmp = HIBYTE(65536 - (SYS_CLOCK/128/target_freq));
	u8TL2_Tmp = LOBYTE(65536 - (SYS_CLOCK/128/target_freq)); 

    TH2 = u8TH2_Tmp;
    TL2 = u8TL2_Tmp;

    ENABLE_TIMER2_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_T2CON_TR2;                                  //Timer0 run
}

void Timer1_IRQHandler(void)
{
	P11 ^= 1;
}

void Timer1_ISR(void) interrupt 3
{
    _push_(SFRS);	
	
    TH1 = u8TH1_Tmp;
    TL1 = u8TL1_Tmp;
    clr_TCON_TF1;
	
	Timer1_IRQHandler();

    _pop_(SFRS);	
}

void TIMER1_Init(void)
{
	uint16_t res = 0;

	/*
		formula : 16bit 
		(0xFFFF+1 - target)  / (24MHz/psc) = time base 

	*/	
	const uint16_t target_freq = 100;

	ENABLE_TIMER1_MODE1;	// mode 0 : 13 bit , mode 1 : 16 bit
    TIMER1_FSYS_DIV12;
	
	u8TH1_Tmp = HIBYTE(65536 - (SYS_CLOCK/12/target_freq));
	u8TL1_Tmp = LOBYTE(65536 - (SYS_CLOCK/12/target_freq)); 

    TH1 = u8TH1_Tmp;
    TL1 = u8TL1_Tmp;

    ENABLE_TIMER1_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR1;                                  //Timer0 run
}

void Timer0_IRQHandler(void)
{

	tick_counter();

	if ((get_tick() % 1000) == 0)
	{
//    	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
		P12 ^= 1;

	}

	if ((get_tick() % 50) == 0)
	{

	}		
}

void Timer0_ISR(void) interrupt 1        // Vector @  0x0B
{
    _push_(SFRS);	
	
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    clr_TCON_TF0;
	
	Timer0_IRQHandler();

    _pop_(SFRS);	
}

void TIMER0_Init(void)	// 1000 Hz
{
	uint16_t res = 0;

	/*
		formula : 16bit 
		(0xFFFF+1 - target)  / (24MHz/psc) = time base 

	*/	
	const uint16_t target_freq = 1000;

	ENABLE_TIMER0_MODE1;	// mode 0 : 13 bit , mode 1 : 16 bit
    TIMER0_FSYS_DIV12;
	
	u8TH0_Tmp = HIBYTE(65536 - (SYS_CLOCK/12/target_freq));
	u8TL0_Tmp = LOBYTE(65536 - (SYS_CLOCK/12/target_freq)); 

    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;

    ENABLE_TIMER0_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR0;                                  //Timer0 run
}


void Serial_ISR (void) interrupt 4 
{
    _push_(SFRS);

    if (RI)
    {   
      uart0_receive_flag = 1;
      uart0_receive_data = SBUF;
      clr_SCON_RI;                                         // Clear RI (Receive Interrupt).
    }
    if  (TI)
    {
      if(!BIT_UART)
      {
          TI = 0;
      }
    }

    _pop_(SFRS);	
}

void UART0_Init(void)
{
	#if 1
	const unsigned long u32Baudrate = 115200;
	P06_QUASI_MODE;    //Setting UART pin as Quasi mode for transmit
	
	SCON = 0x50;          //UART0 Mode1,REN=1,TI=1
	set_PCON_SMOD;        //UART0 Double Rate Enable
	T3CON &= 0xF8;        //T3PS2=0,T3PS1=0,T3PS0=0(Prescale=1)
	set_T3CON_BRCK;        //UART0 baud rate clock source = Timer3

	RH3    = HIBYTE(65536 - (SYS_CLOCK/16/u32Baudrate));  
	RL3    = LOBYTE(65536 - (SYS_CLOCK/16/u32Baudrate));  
	
	set_T3CON_TR3;         //Trigger Timer3
	
	set_IE_ES;
	ENABLE_GLOBAL_INTERRUPT;

	set_SCON_TI;
	BIT_UART=1;
	#else	
    UART_Open(SYS_CLOCK,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF; 
	#endif
}


void MODIFY_HIRC_24(void)
{
	unsigned char u8HIRCSEL = HIRC_24;
    unsigned char data hircmap0,hircmap1;
//    unsigned int trimvalue16bit;
    /* Check if power on reset, modify HIRC */
    SFRS = 0 ;
	#if 1
    IAPAL = 0x38;
	#else
    switch (u8HIRCSEL)
    {
      case HIRC_24:
        IAPAL = 0x38;
      break;
      case HIRC_16:
        IAPAL = 0x30;
      break;
      case HIRC_166:
        IAPAL = 0x30;
      break;
    }
	#endif
	
    set_CHPCON_IAPEN;
    IAPAH = 0x00;
    IAPCN = READ_UID;
    set_IAPTRG_IAPGO;
    hircmap0 = IAPFD;
    IAPAL++;
    set_IAPTRG_IAPGO;
    hircmap1 = IAPFD;
    clr_CHPCON_IAPEN;

	#if 0
    switch (u8HIRCSEL)
    {
		case HIRC_166:
		trimvalue16bit = ((hircmap0 << 1) + (hircmap1 & 0x01));
		trimvalue16bit = trimvalue16bit - 15;
		hircmap1 = trimvalue16bit & 0x01;
		hircmap0 = trimvalue16bit >> 1;

		break;
		default: break;
    }
	#endif
	
    TA = 0xAA;
    TA = 0x55;
    RCTRIM0 = hircmap0;
    TA = 0xAA;
    TA = 0x55;
    RCTRIM1 = hircmap1;
    clr_CHPCON_IAPEN;
    PCON &= CLR_BIT4;
}


void SYS_Init(void)
{
    MODIFY_HIRC_24();

    ALL_GPIO_QUASI_MODE;
    ENABLE_GLOBAL_INTERRUPT;                // global enable bit	
}

void main (void) 
{
    SYS_Init();

    UART0_Init();
	GPIO_Init();
	TIMER0_Init();
	TIMER1_Init();
	TIMER2_Init();

	set_interrupt_priority();
		
    while(1)
    {

		
    }
}



