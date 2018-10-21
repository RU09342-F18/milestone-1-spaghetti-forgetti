#include <msp430.h>


/**
 * main.c
 */

extern void LEDSetup();
extern void timerSetup();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    LEDSetup();
    timerSetup();

    //UART CODE
    P4SEL |= (BIT4+BIT5);       // P4.4 & 4.5 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;        //State machine reset + small clock initialization
    UCA1CTL1 |= UCSSEL_2;       //SMCLK
    UCA1BR0 = 6;                //9600 baud
    UCA1BR1 = 0;                //9600 baud
    UCA1MCTL |= UCBRS_0 + UCBRF_13 + UCOS16;
    UCA1CTL1 &= ~UCSWRST;       // Initialize USCI state machine
    UCA1IE |= UCRXIE;           // Enable USCI_A0 RX interrupt

    __bis_SR_register(LPM0_bits + GIE);   //Low power mode w/ global interrupt enabled
    __no_operation();
}

void LEDSetup(void)
{
    P1DIR |= (BIT2 + BIT3 + BIT4);  //Set outputs for LEDs
    P1SEL |= (BIT2 + BIT3 + BIT4);  //GPIO set
}
void timerSetup(void)
{
    TA0CTL = TASSEL_2 + MC_1 + OUTMOD_7;        //SMCLK + up-mode + reset/set
    TA0CCR0 = 0xFF;      //Full Cycle

    TA0CCTL1 = OUTMOD_2;  //toggle
    TA0CCTL2 = OUTMOD_2;  //toggle
    TA0CCTL3 = OUTMOD_2;  //toggle
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
      case USCI_NONE:break;                             // Vector 0 - no interrupt
      case USCI_UCRXIFG: // Vector 2 - RXIFG
          switch(bitcounter)
              {
              case 0:
                  while(!(UCA1IFG & UCTXIFG));
                  buffer = UCA1RXBUF;           //Store num. of bytes
                  UCA1TXBUF = UCA1RXBUF - 3;  //Takes 3 bytes off the code and transmit
                  __no_operation();           //Pauses the clock for a moment
                  break;

              case 1:
                  TA0CCR1 = (UCA1RXBUF);      //Sets RED section
                  break;

              case 2:
                  TA0CCR2 = (UCA1RXBUF);      //Sets Green section
                  break;

              case 3:
                  TA0CCR3 = (UCA1RXBUF);      //Sets Blue section
                  break;

              default:
                  while(!(UCA1IFG & UCTXIFG));
                  UCA1TXBUF = UCA1RXBUF;      //Transmit bytes to next node
                  break;
              }
              if(bitcounter != buffer)
              {
                  bitcounter += 1;
               }
              else if (bitcounter == buffer)
              {
                   bitcounter = 0;
              }
        break;
        case USCI_UCTXIFG:break;
        default: break;
      }
}
