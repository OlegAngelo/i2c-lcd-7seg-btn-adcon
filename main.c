// UART - async transmitter
#include <xc.h>
#include <stdio.h>

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

float voltageOutput = 0;
unsigned int wholeNum, decimalNum;
int d_value = 0;
char data[20];

void delay(int time)
{
	int i, j;
	for (i = 0; i < time; i++)
	{
		for (j = 0; j < 100; j++);
	}
}

void instCtrl(unsigned char data)
{
	PORTD = data; // load data to port c since my port c is the output pins
	RB5 = 0;	  // set RS to 0 (instruction reg)
	RB6 = 0;	  // set RW to 0 (write)
	RB7 = 1;	  // set E to 1
	delay(5);
	RB7 = 0; // set E to 0 (strobe)
}

void lcdConfig()
{
	delay(50);

	instCtrl(0x3C); // function set: 8-bit; dual-line
	instCtrl(0x38); // display off
	instCtrl(0x01); // display clear
	instCtrl(0x06); // entry mode: increment; shift off
	instCtrl(0x0C); // display on; cursor off; blink off
}

void dataCtrl(unsigned char data)
{
	PORTD = data; // load data to PORT C since its our output
	RB5 = 1;	  // set RS to 1 (data reg)
	RB6 = 0;	  // set RW to 0 (write)
	RB7 = 1;	  // set E to 1
	delay(5);
	RB7 = 0; // set E to 0 (strobe)
}

void interrupt ISR(void)
{
    GIE = 0; // disable all unmasked interrupts (INTCON reg)

    if (ADIF == 1) // checks CCP1 interrupt flag
    {
		char voltage[] = "VOLTAGE:";

        ADIF = 0; // clears interrupt flag (INTCON reg)
		
        d_value = ((ADRESH << 8) + ADRESL); // read ADRESH, move to correct position, read ADRESL

        // convert adc to voltage value
        voltageOutput = (d_value / 1023.0) * 5.0; // 1023 because 10 bits ADRESH + ADRESL
		voltageOutput += 0.02;

		wholeNum = (int)voltageOutput;
		decimalNum = (int)((voltageOutput - wholeNum) * 10);
		
		instCtrl(0x02);
		for (char *w = voltage; *w; w++) dataCtrl(*w);

		instCtrl(0xC0);
		sprintf(data, "%d.%d ", wholeNum, decimalNum);
		for (char *p = data; *p; p++) dataCtrl(*p);

    	delay(100); // delay to get the hold capacitor charged
    	GO = 1; // restart A/D conversion (ADCON0 reg)
    }

	// intf needs to be outside so it doesnt depend on the pot to be changed
	// in order to transfer data
	if (INTF) { // check rb0 interrupt
		INTF = 0;
		RC0 ^= 1; // led to test if interrupt rb0 is working

		unsigned char transmitData = (wholeNum << 4) | (decimalNum & 0x0F);

		while(!TRMT);
		TXREG = transmitData;
	}


    GIE = 1; // enable all unmasked interrupts (INTCON reg)
}

void interruptConfig(void) {
	OPTION_REG = 0xC4; // 1100 0100
	INTE = 1; // int enable
	INTF = 0; // int flag clear
}

void adcConfig(void) {
    // configure for ADC
    ADCON1 = 0x80; // result register: right Justified, clock: FOSC/8
                   // all ports in PORTA are analog
                   // VREF+=VDD, VREF-=VSS
    ADCON0 = 0x41; // clock: FOSC/8 analog channel: AN0
                   // A/D conversion: STOP, A/D module: ON
    ADIE = 1;      // A/D conversion complete interrupt enable (PIE1 reg)
    ADIF = 0;      // reset interrupt flag (PIR1 reg)
    GO = 1;        // start A/D conversion (ADCON0 reg)
}

void asyncTransmitterConfig(void){
	// transmitter config
 	SPBRG = 0x19; // 9.6K baud rate @ FOSC=4MHz, asynchronous high speed
 			      // (see formula in Table 10-1 data sheet)
 	SYNC = 0; // asynchronous mode (TXSTA reg)
 	SPEN = 1; // enable serial port (RCSTA reg)
 	TX9 = 0; // 8-bit transmission (TXSTA reg)
 	BRGH = 1; // asynchronous high-speed (TXSTA reg)
 	TXEN = 1; // transmit enable (TXSTA reg)
}

void portsConfig(void) {
    TRISD = 0x00; // data for LCD
	TRISB = 0x01; // instructions for LCD & rb0 as input for button
	TRISC = 0x00; // output to led to test rb0 int
				  // output to mcu2 (master)
}

void main(void)
{
    portsConfig();
	lcdConfig();
   	adcConfig();

	asyncTransmitterConfig();

	interruptConfig(); // rb0 int

	RB0 = 0; // set led to low
	
    PEIE = 1;      // enable all peripheral interrupt (INTCON reg)
    GIE = 1;       // enable all unmasked interrupts (INTCON reg)

    while(1)
    {
    }
}