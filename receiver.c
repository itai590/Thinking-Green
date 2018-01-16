#include <avr/io.h>
#include "avr/interrupt.h"
#include "bit_def.h"
#include "util/delay.h"
#include "serial.h"

#define recieved_data PIND	//����� �� ����� ����� ������� ���������	
    
char recieved_measures[10]={3,5,5,5,5,5,5,5,5,5}; //���� ������� �����
int l=0,i;
char send_data[5]={0,0,0,0,0}; //���� ������ ����� �����
short temp;

//----------------������� ����� ������� 1  -----------
ISR (INT1_vect)
{
	GICR  = 0b00000000;	//����� ����� ������� 1
	_delay_ms(1000);
	/*����� ����� ������ ���� ����� ����� ��� �� ������ ����� �����*/
	recieved_measures[0]=recieved_data&0xf0; //����� ���� ������-4������- �� ����� ������ ������� �����- ��� 8 ������
	if(recieved_measures[0]!=0xf0) //�� �� ������ �� ���� ������ �������
		goto sof;
	_delay_ms(2000);
	recieved_measures[1]=recieved_data&0xf0; // �����  ���� ������ -4 ������ -�� ���� -��� 8 ������
	if(recieved_measures[1]!=0xf0)
		goto sof;
	_delay_ms(2000);
	for(l=2;l<10;l++)
	{
		recieved_measures[l]=recieved_data&0xf0; //����� 4 ������ �� �� ��� ������
		_delay_ms(2000);
	}
	/*����� ������� ����� ����� �� 8 ������ ������� ����� ���� ���� 8 ������ ������ ������ ������*/
	send_data[0]=0xaa;
	send_data[1]=recieved_measures[2]|(recieved_measures[3]>>4);
	send_data[2]=recieved_measures[4]|(recieved_measures[5]>>4);
	send_data[3]=recieved_measures[6]|(recieved_measures[7]>>4);
	send_data[4]=recieved_measures[8]|(recieved_measures[9]>>4);
	USART_Transmit_Buffer(send_data,5); //������ ����� �����
sof: GICR  = 0b10000000;	//������ ����� �������  1
}

void USART_Init (void)
{
	/* Set baud rate */ 
	UBRRH = 0x00;  
	UBRRL = 207;	//9600 -- baud-rate- ���� ����
	/* Enable receiver and transmitter */
	UCSRB = (1<<TXEN); //���� ����� ����
	UCSRA |= (1<<U2X); //����� ��� ������ �� 2
	/*//����� �� URSEL �1
	 �� ��� ����� ����� 
	�- UCSRC
	��� ����� �- UBRRH
	����� �� 8 ������ ����*/
	UCSRC = (1<<URSEL)|(3<<UCSZ0); 

}

void USART_Transmit (unsigned char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );
	/* Put data into buffer, sends the data */
	UDR = data;
}

void USART_Transmit_Buffer (unsigned char *data,unsigned char len)
{
	while(len)
	{
		USART_Transmit(*data); //��� �� ���� �� ������
		data++; //���� �� ������ ���� ����� ���� ��� ��� �����
		len--;
	}
}

//-----------------------------------������ �����------------------------------
int main (void)
{
	DDRD=0x02; //����� ���� ���� ���� - ��� ������ ����� �����
	USART_Init(); 
	MCUCR = 0b00001100; //����� ������� 1 ����� �����
	GICR  = 0b10000000;	//������ ����� �������  1 
	sei();	            //������ ����� ������ ������ - ���� ����� ���� �� �������
	while(1);

	return 0;
}
