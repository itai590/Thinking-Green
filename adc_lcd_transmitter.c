#include <avr/io.h>
#include "avr/interrupt.h"
#include "bit_def.h"
#include "util/delay.h"

#define en_lcd PORTC_0      //PORTC_0 display enable 
#define rs	   PORTC_1		
#define data_port	PORTB   
#define TE     PORTC_6          

short temp1=0,temp2=0,gas1=0,gas2=0;
unsigned char c_temp1=0,c_temp2=0,c_gas1=0,c_gas2=0,measure[10]; 
int m,n,concentration,gas_in_ppm,l;
float Vin,R0,Rs,ratio,ratio_resistor_value;


void instruction_out_lcd(unsigned char ins_code)
{
	rs=0; 
	data_port=ins_code;  
	en_lcd=1; 
	en_lcd=0;  
	_delay_ms(5);  
}


void data_out_lcd(unsigned char data_out)
{
	if(data_out>=0x80) 
		data_out-=0x40; 
	rs=1; 
	data_port=data_out; 
	en_lcd=1; 
	en_lcd=0;   
	_delay_ms(5);
}

//--display int
void init_lcd()			
{
	 unsigned char j;
	 for(j=0;j<4;j++)
	 {
		 instruction_out_lcd(0x38); 
	 }
	 instruction_out_lcd(0x0C);
	 instruction_out_lcd(0x01); 
	 instruction_out_lcd(0x0C); 
	 instruction_out_lcd(0x97); 
	 instruction_out_lcd(0x04); 
}


void lcd_line1( char str[])
{
	unsigned char j;
	instruction_out_lcd(0x97); 
	instruction_out_lcd(0x04); 
    for(j=0;j<24;j++) 
		data_out_lcd(str[j]);
}


void lcd_line2( char str[])
{
    unsigned char j;
	instruction_out_lcd(0xD7); 
	instruction_out_lcd(0x04); 
    for(j=0;j<24;j++) 
	{
		data_out_lcd(str[j]);
	}
}


void temp_display1 ()
{
	instruction_out_lcd(0x8D);
	instruction_out_lcd(0x6); 
	temp1=(temp1*25)/10; 
	c_temp1=temp1/10; 
	
	data_out_lcd(temp1/100+0x30); 
	data_out_lcd(temp1%100/10+0x30);
	data_out_lcd('.');
	data_out_lcd(temp1%100%10+0x30);
	data_out_lcd('C');
}


void temp_display2 ()
{
	instruction_out_lcd(0xCD); 
	instruction_out_lcd(0x6); 
	temp2=(temp2*25)/10; 
	c_temp2=temp2/10;
	
	data_out_lcd(temp2/100+0x30); 
	data_out_lcd(temp2%100/10+0x30); 
	data_out_lcd('.');
	data_out_lcd(temp2%100%10+0x30); 
	data_out_lcd('þC');
}


void gas_display1 (int gas_ppm)
{
	instruction_out_lcd(0x80); 
    instruction_out_lcd(0x6);
	if(gas_ppm==10000) 
	{
		gas_ppm=(gas_ppm)/100;
		data_out_lcd(gas_ppm/100+0x30);
		data_out_lcd(gas_ppm%100/10+0x30);
		data_out_lcd('.');
		data_out_lcd(gas_ppm%100%10+0x30);
		data_out_lcd('þK');
		data_out_lcd('p');
	}

	else
	{
		gas_ppm=(gas_ppm)/100;
		data_out_lcd(' ');
		data_out_lcd(gas_ppm%100/10+0x30);
		data_out_lcd('.');
		data_out_lcd(gas_ppm%100%10+0x30);
		data_out_lcd('þK');
		data_out_lcd('p');
		data_out_lcd('p');
		data_out_lcd('m');
	}
}


void gas_display2 (int gas_ppm)
{
	instruction_out_lcd(0xC0); 
	instruction_out_lcd(0x6); 

	if(gas_ppm==10000) 
	{
		gas_ppm=(gas_ppm)/100;
		data_out_lcd(gas_ppm/100+0x30);
		data_out_lcd(gas_ppm%100/10+0x30);
		data_out_lcd('.');
		data_out_lcd(gas_ppm%100%10+0x30);
		data_out_lcd('þK');
		data_out_lcd('p');
	}

	else
	{
		gas_ppm=(gas_ppm)/100;
		data_out_lcd(' ');
		data_out_lcd(gas_ppm%100/10+0x30);
		data_out_lcd('.');
		data_out_lcd(gas_ppm%100%10+0x30);
		data_out_lcd('þK');
		data_out_lcd('p');
		data_out_lcd('p');
		data_out_lcd('m');
   }
}


void adc_init(void)
{
	
	REFS1:0  */
	ADMUX = 0b11000000;

	ADCSRA = 0x86;
}


unsigned short adcConvert (unsigned char Ch)
{
	unsigned short Val;		
	ADMUX &=0xc0;			
	ADMUX|=Ch;				
	ADCSRA|=0x40;			
	while(!(ADCSRA&0x10));	
	Val=ADC;				
	return Val;
}


float resistor_ratio(float adc_value)
{
	R0=((5/0.5)-1)/2;
	Vin=adc_value*2.56/1024;  
	Rs=(5/Vin)-1; 
	ratio=Rs/R0; 
	return ratio;
}

//---------------- ppm--------------------------------------
int pollution(float resistor_ratio)
{ 
	if(resistor_ratio>=2) m=0;                            //200ppm
	if(resistor_ratio<2 && resistor_ratio>=1.7) m=1;      //300ppm
	if(resistor_ratio<1.7 && resistor_ratio>=1.4) m=2;    //400ppm
	if(resistor_ratio<1.4 && resistor_ratio>=1.3) m=3;    //500ppm
	if(resistor_ratio<1.3 && resistor_ratio>=1.2) m=4;    //600Pppm
	if(resistor_ratio<1.2 && resistor_ratio>=1.15) m=5;   //700ppm
	if(resistor_ratio<1.15 && resistor_ratio>=1.12) m=6;  //800ppm
	if(resistor_ratio<1.12 && resistor_ratio>=1.06) m=7;  //900ppm
	if(resistor_ratio<1.06 && resistor_ratio>=1.0) m=8;   //1000ppm
	if(resistor_ratio<1.0 && resistor_ratio>=0.74) m=9;   //2000ppm
	if(resistor_ratio<0.74 && resistor_ratio>=0.64) m=10; //3000ppm
	if(resistor_ratio<0.64 && resistor_ratio>=0.57) m=11; //4000ppm
	if(resistor_ratio<0.57 && resistor_ratio>=0.52) m=12; //5000ppm
	if(resistor_ratio<0.52 && resistor_ratio>=0.47) m=13; //6000ppm
	if(resistor_ratio<0.47 && resistor_ratio>=0.45) m=14; //7000ppm
	if(resistor_ratio<0.45 && resistor_ratio>=0.42) m=15; //8000ppm
	if(resistor_ratio<0.42 && resistor_ratio>=0.41) m=16; //9000ppm
	if(resistor_ratio<0.41) m=17;                         //10000ppm
	int ppm_array[18]={200,300,400,500,600,700,800,900,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};
	concentration=ppm_array[m];

	return concentration;
}

void send_measures (void)
{
	measure[0]=0xf0;
	measure[1]=0xf0;
	measure[2]=c_temp1&0xf0;
	measure[3]=c_temp1<<4;
	measure[4]=c_gas1&0xf0;
	measure[5]=c_gas1<<4;
	measure[6]=c_temp2&0xf0;
	measure[7]=c_temp2<<4;
	measure[8]=c_gas2&0xf0;
	measure[9]=c_gas2<<4;
	for(l=0;l<10;l++)
	{
		PORTD=measure[l]; 
		TE=0;            
		_delay_ms(500);  
		TE=1;           
		_delay_ms(500);
	}
}


int main (void)
{
	DDRD=0xff;     
	DDRC=0xc3;		
	DDRB=0xff;		
	init_lcd();		
	TE=1;           
	adc_init();		
	lcd_line1("temp1:       gas1:        ");
	lcd_line2("temp2:       gas2:        ");
	float temp_array[13] ={1.32,1.25,1.18,1.1,1.05,1.02,1.0,0.98,0.97,0.95,0.92,0.91,0.9};

	while(1)
	{
		temp1=	adcConvert (0);
    	n=((temp1/4)/5)+2;
    	temp_display1 ();	

    	gas1=	adcConvert (1); 
    	ratio_resistor_value=resistor_ratio(gas1);
    	ratio_resistor_value=ratio_resistor_value/temp_array[n];
    	gas_in_ppm=pollution(ratio_resistor_value);
		c_gas1=gas_in_ppm/100;
    	gas_display1(gas_in_ppm);

    	temp2=	adcConvert (2); 
    	n=((temp2/4)/5)+2;
    	temp_display2 ();

    	gas2=	adcConvert (3); 
    	ratio_resistor_value=resistor_ratio(gas2);
        ratio_resistor_value=ratio_resistor_value/temp_array[n];
    	gas_in_ppm=pollution(ratio_resistor_value);
        c_gas2=gas_in_ppm/100;
    	gas_display2(gas_in_ppm);

		send_measures();

		_delay_ms(10000);

	}
	return 0;
}
