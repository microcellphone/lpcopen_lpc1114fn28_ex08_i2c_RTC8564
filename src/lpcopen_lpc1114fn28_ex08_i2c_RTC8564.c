/*
===============================================================================
 Name        : lpcopen_lpc1114fn28_ex08_i2c_RTC8564.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "rtc8564.h"
#include <xprintf.h>

// TODO: insert other definitions and declarations here
void UART_IRQHandler(void)
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	unsigned char Buf;
	uint8_t d;
	int i, cnt;


	IIRValue = Chip_UART_ReadIntIDReg(LPC_USART);
	IIRValue >>= 1;			/* skip pending bit in IIR */
	IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */

	if (IIRValue == IIR_RLS){		/* Receive Line Status */
		LSRValue = Chip_UART_ReadLineStatus(LPC_USART);
		/* Receive Line Status */
		if (LSRValue & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE | UART_LSR_BI)){
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			UARTStatus = LSRValue;
			Dummy = Chip_UART_ReadByte(LPC_USART);	/* Dummy read on RX to clear interrupt, then bail out */
			return;
		}
		if (LSRValue & UART_LSR_RDR){	/* Receive Data Ready */
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			Buf = Chip_UART_ReadByte(LPC_USART);
		}
	}else if (IIRValue == IIR_RDA){	/* Receive Data Available */
		/* Receive Data Available */
		i = RxBuff.wi;
		cnt = RxBuff.ct;
		while (Chip_UART_ReadLineStatus(LPC_USART) & UART_LSR_RDR) {	/* Get all data in the Rx FIFO */
			d = Chip_UART_ReadByte(LPC_USART);
			if (cnt < BUFF_SIZE) {	/* Store data if Rx buffer is not full */
				RxBuff.buff[i++] = d;
				i %= BUFF_SIZE;
				cnt++;
			}
		}
		RxBuff.wi = i;
		RxBuff.ct = cnt;
	}else if (IIRValue == IIR_CTI){	/* Character timeout indicator */
		/* Character Time-out indicator */
		UARTStatus |= 0x100;		/* Bit 9 as the CTI error */
	}else if (IIRValue == IIR_THRE){	/* THRE, transmit holding register empty */
		cnt = TxBuff.ct;
		if(cnt){/* There is one or more byte to send */
			i = TxBuff.ri;
			for (d = 16; d && cnt; d--, cnt--){	/* Fill Tx FIFO */
				Chip_UART_SendByte(LPC_USART, TxBuff.buff[i++]);
				i %= BUFF_SIZE;
			}
			TxBuff.ri = i;
			TxBuff.ct = cnt;
		}else{
			TxBuff.act = 0; /* When no data to send, next putc() must trigger Tx sequense */
		}
	}
	return;
}

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    // TODO: insert code here
    TS_RTC_INFO rtc;


    SysTick_Config(SystemCoreClock/1000 - 1); /* Generate interrupt each 1 s   */
    IOCON_Config_Request();
    UART_Config_Request(115200);
    xdev_out(uart0_putc);
    xdev_in(uart0_getc);
    xprintf ("lpcopen_lpc1114fn28_ex08_i2c_RTC8564\n") ;

    RTC8564_Config_Request();
    RTC8564_Initialize_Request( 1, 19, 3, 10, MON, 10, 53, 30);
    RTC8564_Get_Current_Time_Request ( &rtc );
    xprintf("20%02d/%02d/%02d(%s)\n", rtc.bRTC_year, rtc.bRTC_mon, rtc.bRTC_day, Get_Week_String(rtc.bRTC_week));

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
    	Delay(1000);
       RTC8564_Get_Current_Time_Request ( &rtc );
//       xprintf("20%02d/%02d/%02d(%s)\n", rtc.bRTC_year, rtc.bRTC_mon, rtc.bRTC_day, Get_Week_String(rtc.bRTC_week));
       xprintf("%02d:%02d:%02d\r", rtc.bRTC_hour, rtc.bRTC_min, rtc.bRTC_sec);
		  i++ ;
        // "Dummy" NOP to allow source level single
        // stepping of tight while() loop
        __asm volatile ("nop");
    }
    return 0 ;
}
