#include <avr/io.h>
#include "avr/interrupt.h"
#include "bit_def.h"
#include "util/delay.h"
#include "serial.h"

#define recieved_data PIND	//הצהרה על הפורט לקבלת הנתונים המפוענחים	
    
char recieved_measures[10]={3,5,5,5,5,5,5,5,5,5}; //מערך הנתונים שנקלט
int l=0,i;
char send_data[5]={0,0,0,0,0}; //מערך השידור למחשב האישי
short temp;

//----------------פונקצית פסיקה חיצונית 1  -----------
ISR (INT1_vect)
{
	GICR  = 0b00000000;	//ביטול פסיקה חיצונית 1
	_delay_ms(1000);
	/*בדיקה שהדגל סנכרון שלנו התקבל נכונה ורק אז להמשיך לקבלת המידע*/
	recieved_measures[0]=recieved_data&0xf0; //מיסוך חלקו התחתון-4סיביות- של הנתון הראשון והכנסתו למערך- סהכ 8 סיביוץ
	if(recieved_measures[0]!=0xf0) //אם לא מתקיים לך לסוף ותתחיל מההתחלה
		goto sof;
	_delay_ms(2000);
	recieved_measures[1]=recieved_data&0xf0; // בדיקת  חלקו העליון -4 סיביות -של הדגל -סהכ 8 סיביות
	if(recieved_measures[1]!=0xf0)
		goto sof;
	_delay_ms(2000);
	for(l=2;l<10;l++)
	{
		recieved_measures[l]=recieved_data&0xf0; //הכנסת 4 סיביות של כל בית בהתקבל
		_delay_ms(2000);
	}
	/*הכנסת הנתונים למערך בצורה של 8 סיביות בתקשורת טורית ניתן לשדר 8 סיביות בניגוד לשיודר למקודד*/
	send_data[0]=0xaa;
	send_data[1]=recieved_measures[2]|(recieved_measures[3]>>4);
	send_data[2]=recieved_measures[4]|(recieved_measures[5]>>4);
	send_data[3]=recieved_measures[6]|(recieved_measures[7]>>4);
	send_data[4]=recieved_measures[8]|(recieved_measures[9]>>4);
	USART_Transmit_Buffer(send_data,5); //השידור למחשב האישי
sof: GICR  = 0b10000000;	//איפשור פסיקה חיצונית  1
}

void USART_Init (void)
{
	/* Set baud rate */ 
	UBRRH = 0x00;  
	UBRRL = 207;	//9600 -- baud-rate- ערכה ישנה
	/* Enable receiver and transmitter */
	UCSRB = (1<<TXEN); //אפשר שידור טורי
	UCSRA |= (1<<U2X); //הגדלת קצב ההעברה פי 2
	/*//נכתוב את URSEL ל1
	 על מנת לכתוב לאוגר 
	ה- UCSRC
	ולא לאוגר ה- UBRRH
	עבודה עם 8 סיביות מידע*/
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
		USART_Transmit(*data); //שלח את הערך של המצביע
		data++; //הגדך את המצביע באחד כלומר עבור לתא הבא במערך
		len--;
	}
}

//-----------------------------------תוכנית ראשית------------------------------
int main (void)
{
	DDRD=0x02; //קביעת ההדק השני כפלט - הדק השידור למחשב האישי
	USART_Init(); 
	MCUCR = 0b00001100; //פסיקה חיצונית 1 פעילה בעליה
	GICR  = 0b10000000;	//איפשור פסיקה חיצונית  1 
	sei();	            //איפשור סיבית פסיקות גלובלי - חובה לאפשר כללי את הפסיקות
	while(1);

	return 0;
}
