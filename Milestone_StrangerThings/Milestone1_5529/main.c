#include <msp430.h> 


/**
 * main.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	LEDSetup();
	timerSetup();

	//UART CODE
	P3SEL |= (BIT3+BIT4);
    UCA0CTL1 |= (UCSWRST); //State machine reset + small clock initialization
    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 = 6;                //9600 baud *Determined by TI calculator(http://processors.wiki.ti.com/index.php/USCI_UART_Baud_Rate_Gen_Mode_Selection)
    UCA0BR1 = 0;            //9600 baud *Determined by TI calculator(http://processors.wiki.ti.com/index.php/USCI_UART_Baud_Rate_Gen_Mode_Selection)
    UCA0MCTL |= UCBRS_0; // Modulation
    UCA0MCTL |= UCBRF_13;
    UCA0MCTL |= UCOS16;
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
    UCA0IE |= UCRXIE; // Enable USCI_A0 RX interrupt

    _BIS_SR(LPM0_bits + GIE);   //Low power mode w/ global interrupt enabled
    while(1)
    {

    }
}

void LEDSetup(void)
{
    P1DIR |= (BIT2 + BIT3 + BIT4);  //Set outputs for LEDs
    P1SEL |= (BIT2 + BIT3 + BIT4);  //
}
void timerSetup(void)
{
    TA0CTL = TASSEL_2 + ID_2 + MC_1;        //SMCLK + Internal Divider of 4 + upmode
    TA0CCR0 = 255;      //Full Cycle
    TA0CCR1 = 150;        //Red LED
    TA0CCR2 = 0;        //Green LED
    TA0CCR3 = 0;        //Blue LED
    TA0CCTL1 = OUTMOD_7;  //reset/set
    TA0CCTL2 = OUTMOD_7;  //reset/set
    TA0CCTL3 = OUTMOD_7;  //reset/set
}

volatile int bitcounter = 0;
#pragma vector=USCI_A0_VECTOR       //Interrupt vector definition
__interrupt void USCI_A0(void)       //Interrupt occurs anytime an input/output occurs
{
    switch(__even_in_range(UCA0IV, USCI_UCTXIFG))
    {
        case USCI_NONE:
            break;
        case USCI_UCRXIFG:
            switch(bitcounter)
            {
            case 0:
                while(!(UCA0IFG & UCTXIFG))
                UCA0TXBUF = UCA0RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
                __no_operation();           //Pauses the clock for a moment
                break;

            case 1:
                TA0CCR1 = (UCA0RXBUF);      //Sets RED section
                break;

            case 2:
                TA0CCR2 = (UCA0RXBUF);      //Sets RED section
                break;

            case 3:
                TA0CCR3 = (UCA0RXBUF);      //Sets RED section
                break;

            default:
                UCA0TXBUF = UCA0RXBUF - 3;  //Takes 3 bytes off the code for LEDs and leaves the rest
                UCA0TXBUF = UCA0RXBUF;      //Transmit bytes to next board
            }
            if(UCA0RXBUF != 0x0D)
            {
                bitcounter += 1;
            }
            else if(UCA0RXBUF == 0x0D)
            {
                bitcounter = 0;
            }
            break;
        case USCI_UCTXIFG:  //Do nothing if transmitting
            break;
        default:
            break;
    }
}
