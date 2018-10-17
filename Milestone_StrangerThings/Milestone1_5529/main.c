#include <msp430.h> 


/**
 * main.c
 */

extern void LEDSetup();
extern void timerSetup();
extern void resetColor();

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	LEDSetup();
	timerSetup();

	//UART CODE
	P4SEL |= (BIT4+BIT5);
    UCA1CTL1 |= UCSWRST; //State machine reset + small clock initialization
    UCA1CTL1 |= UCSSEL_2;
    UCA1BR0 = 6;                //9600 baud *Determined by TI calculator(http://processors.wiki.ti.com/index.php/USCI_UART_Baud_Rate_Gen_Mode_Selection)
    UCA1BR1 = 0;            //9600 baud *Determined by TI calculator(http://processors.wiki.ti.com/index.php/USCI_UART_Baud_Rate_Gen_Mode_Selection)
    UCA1MCTL |= UCBRS_0 + UCBRF_13 + UCOS16;
    UCA1CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
    UCA1IE |= UCRXIE; // Enable USCI_A0 RX interrupt

    __bis_SR_register(LPM0_bits + GIE);   //Low power mode w/ global interrupt enabled
    __no_operation();
}

void LEDSetup(void)
{
    P1DIR |= (BIT2 + BIT3 + BIT4);  //Set outputs for LEDs
    P1SEL |= (BIT2 + BIT3 + BIT4);  //GPIO
}
void timerSetup(void)
{
    TA0CTL = TASSEL_2 + MC_1 + OUTMOD_7;        //SMCLK + upmode + reset/set
    TA0CCR0 = 0xFF;      //Full Cycle
    TA0CCR1 = 255;        //Red LED
    TA0CCR2 = 0;        //Green LED
    TA0CCR3 = 0;        //Blue LED
    TA0CCTL1 = OUTMOD_2;  //toggle
    TA0CCTL2 = OUTMOD_2;  //toggle
    TA0CCTL3 = OUTMOD_2;  //toggle
}

void resetColor()
{
    TA0CCR1 = 0x00; // Sets TA0CCR1
    TA0CCR2 = 0x00; // Sets TA0CCR2
    TA0CCR3 = 0x00; // Sets TA0CCR3
}

volatile int bitcounter = 0;
volatile int buffer = 0;

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA1IV,4))
      {
      case 0:break;                             // Vector 0 - no interrupt
      case 2: // Vector 2 - RXIFG
          switch(bitcounter)
              {
              case 0:
                  while(!(UCA1IFG & UCTXIFG));
                  //bitcounter += 1;
                  buffer = UCA1RXBUF;
                  UCA1TXBUF = UCA1RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
                  __no_operation();           //Pauses the clock for a moment
                  break;

              case 1:
                  TA0CCR1 = (UCA1RXBUF);      //Sets RED section
                  //bitcounter += 1;
                  break;

              case 2:
                  TA0CCR2 = (UCA1RXBUF);      //Sets Green section
                  //bitcounter += 1;
                  break;

              case 3:
                  TA0CCR3 = (UCA1RXBUF);      //Sets Blue section
                  //bitcounter += 1;
                  break;

              default:
                  while(!(UCA1IFG & UCTXIFG));
                  //UCA1TXBUF = UCA1RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
                  UCA1TXBUF = UCA1RXBUF;      //Transmit bytes to next board
                  break;
              }
              if(bitcounter != buffer - 1)
              {
                  bitcounter += 1;
               }
              else if (bitcounter == buffer - 1)
              {
                   //At the end of the message reset count in anticipation for the next message
                   bitcounter = 0;
              }
        break;
        case 4:break;
        default: break;
      }
}

/*
#pragma vector=USCI_A1_VECTOR       //Interrupt vector definition
__interrupt void USCI_A1_ISR(void)       //Interrupt occurs anytime an input/output occurs
{
    switch(bitcounter)
    {
    case 0:
        //while(!(UCA1IFG & UCTXIFG));
        bitcounter += 1;
        //bitcounter = UCA1RXBUF;
        UCA1TXBUF = UCA1RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
        //__no_operation();           //Pauses the clock for a moment
        break;

    case 1:
        TA0CCR1 = (UCA1RXBUF);      //Sets RED section
        bitcounter += 1;
        break;

    case 2:
        TA0CCR2 = (UCA1RXBUF);      //Sets Green section
        bitcounter += 1;
        break;

    case 3:
        TA0CCR3 = (UCA1RXBUF);      //Sets Blue section
        bitcounter += 1;
        break;

    default:
        //while(!(UCA1IFG & UCTXIFG));
        //UCA1TXBUF = UCA1RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
        UCA1TXBUF = UCA1RXBUF;      //Transmit bytes to next board
        if(UCA1RXBUF == 0x0D)
        {
            bitcounter = 0;
        }
        break;
    }

}*/
/*
{
     case 0:break;                             // Vector 0 - no interrupt
     case 2: // Vector 2 - RXIFG
       switch(bitcounter)
           {
           case 0:
               UCA1TXBUF = UCA1RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
               buffer = UCA1RXBUF;
               bitcounter++;
               break;

           case 1:
               TA0CCR1 = (UCA1RXBUF);      //Sets RED section
               bitcounter++;
               break;

           case 2:
               TA0CCR2 = (UCA1RXBUF);      //Sets Green section
               bitcounter++;
               break;

           case 3:
               TA0CCR3 = (UCA1RXBUF);      //Sets Blue section
               bitcounter++;
               break;

           default:
               if(bitcounter <= buffer)
               {
                   UCA1TXBUF = UCA1RXBUF;      //Transmit bytes to next board
                   bitcounter++;
               }
               else
               {
                   bitcounter = 1;
                   UCA1TXBUF = UCA1RXBUF - 3;
                   buffer = UCA1RXBUF;
                   resetColor();
               }
               break;
           }*/
