// UART - async receiver
#include <xc.h>

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

void delay(int time)
{
	int i, j;
	for (i = 0; i < time; i++)
	{
		for (j = 0; j < 100; j++);
	}
}

void asyncReceiverConfig (void){
	// receiver config
	SPBRG = 0x19; // 9.6K baud rate @ FOSC=4MHz, asynchronous high speed
				// (see formula in Table 10-1 data sheet)
	SYNC = 0; // asynchronous mode (TXSTA reg)
	SPEN = 1; // enable serial port (RCSTA reg)
	RX9 = 0; // 8-bit reception (TXSTA reg)
	BRGH = 1; // asynchronous high-speed (TXSTA reg)
	CREN = 1; // enable continuous receive (RCSTA reg)
}

void portsConfig(void) {
    TRISB = 0x00; // output to 7 segment (wholeNum)
    TRISD = 0x00; // output to 7 segment (decimalNum)
    PORTB = 0; // initial to low
    PORTD = 0; // initial to low

    TRISC7 = 1; // input as uart receiver
}

void main (void) {
    unsigned int wholeNum, decimalNum;
    int data = 0;

    asyncReceiverConfig();
    portsConfig();

    while(1) {
		delay (50);

		while(!RCIF);
        data = RCREG;

        wholeNum = (data >> 4) & 0x0F; // extract upper nibble
        decimalNum = data & 0x0F; // extract lower nibble

        // display to 2 7 segments
        PORTB = wholeNum;
        PORTD = decimalNum;
    }
}