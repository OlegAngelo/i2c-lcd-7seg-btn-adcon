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
int wholeNum, decimalNum, tempNum;
int d_value = 0;
char buffer[16];

void delay(int time)
{
	int i, j;
	for (i = 0; i < time; i++)
	{
		for (j = 0; j < 100; j++);
	}
}

void portConfig(void) {
    TRISD = 0x00; // data for LCD
    //PORTD = 0x00; // initial to low
	TRISB = 0x00; // instructions for LCD
	//PORTB = 0x80;
	//RB4 = 0;
	//RB5 = 1;
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

void initLCD()
{
	delay(50);

	instCtrl(0x3C); // function set: 8-bit; dual-line
	instCtrl(0x38); // display off
	instCtrl(0x01); // display clear
	instCtrl(0x06); // entry mode: increment; shift off
	instCtrl(0x0E); // display on; cursor off; blink off
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


void adcConfig(void) {
    // configure for ADC
    ADCON1 = 0x80; // result register: right Justified, clock: FOSC/8
                   // all ports in PORTA are analog
                   // VREF+=VDD, VREF-=VSS
    ADCON0 = 0x41; // clock: FOSC/8 analog channel: AN0
                   // A/D conversion: STOP, A/D module: ON
    ADIE = 1;      // A/D conversion complete interrupt enable (PIE1 reg)
    ADIF = 0;      // reset interrupt flag (PIR1 reg)
    PEIE = 1;      // enable all peripheral interrupt (INTCON reg)
    GO = 1;        // start A/D conversion (ADCON0 reg)
    GIE = 1;       // enable all unmasked interrupts (INTCON reg)
}
unsigned char configDisplayValue(unsigned char whole, unsigned char decimal)
{
    // (upper nibble = whole, lower nibble = decimal)
    return ((whole<<4) | (decimal & 0x0F)); 
}
void interrupt ISR(void)
{
    GIE = 0; // disable all unmasked interrupts (INTCON reg)

    if (ADIF == 1) // checks CCP1 interrupt flag
    {
        ADIF = 0; // clears interrupt flag (INTCON reg)
		
	
    
        d_value = ((ADRESH << 8) + ADRESL); // read ADRESH, move to correct position, read ADRESL

        // convert adc to voltage value
        voltageOutput = (d_value / 1023.0) * 5.0; // 1023 because 10 bits ADRESH + ADRESL
		voltageOutput += 0.02;

		wholeNum = (int)voltageOutput;
		decimalNum = (int)((voltageOutput - wholeNum) * 10);
		
		delay(50);

		instCtrl(0x02);
       voltageOutput = configDisplayValue(decimalNum, wholeNum);
		sprintf(buffer, "VOLTAGE:%4.1f ", voltageOutput);
        
		for(char *p = buffer; *p; p++) {
			 dataCtrl(*p);
		}
		//PORTD = wholeNum;
		//dataCtrl(wholeNum);
		//PORTD =1;
	//	dataCtrl('.');

		//delay(200);
		//PORTD = decimalNum;
		//dataCtrl('2');

	//	instCtrl(0xC0);

	//	delay(10);
       // voltageOutput = configDisplayValue(tempNum);
    }

    delay(200); // delay to get the hold capacitor charged
    GO = 1; // restart A/D conversion (ADCON0 reg)
    GIE = 1; // enable all unmasked interrupts (INTCON reg)
}


void main(void)
{
    portConfig();

	delay(50);
	initLCD();

/*
	dataCtrl('V');
	dataCtrl('O');
	dataCtrl('L');
	dataCtrl('T');
	dataCtrl('A');
	dataCtrl('G');
	dataCtrl('E');
*/
	//instCtrl(0xC0); // set cursor next line below voltage

	delay(100);

   	adcConfig();


	delay(100);
    while(1)
    {
    }
}