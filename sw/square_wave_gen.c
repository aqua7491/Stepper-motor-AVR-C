#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL 



void delay_ms(uint16_t count) {
  while(count--) {
    _delay_ms(1);
  }
} 

void InitADC()
{
 //set prescaller to 128
 ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
 // Select Vref=AVcc
 // Avcc(+5v) as voltage reference
 ADMUX |= (1<<REFS0);
 ADMUX &= ~(1<<REFS1);
 //ADC in free-running  mode
 ADCSRB &= ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0));
 //Signal source, in this case is the free-running
 ADCSRA |= (1<<ADATE);
 //Power up the ADC
 ADCSRA |= (1<<ADEN);
 //Start converting
 ADCSRA |= (1<<ADSC);
}

uint16_t ReadADC(uint8_t ADCchannel)
{
 //select ADC channel with safety mask
 ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
 //single conversion mode
 ADCSRA |= (1<<ADSC);
 // wait until ADC conversion is complete
 while( ADCSRA & (1<<ADSC) );
 return ADC;
}

int main (void)
{
  double delay;
  int16_t adc_value;
 
  // sense LEDs output mode
  DDRD |= (1<<PD2);
  DDRD |= (1<<PD4);

  /* set output mode 
   * PD5=clock, PD6=enable, PD7=cw/ccw */
  DDRD |= (1<<PD5);
  DDRD |= (1<<PD6);
  DDRD |= (1<<PD7);

  // put ON control pins
  PORTD |= (1<<PD5);
  PORTD |= (1<<PD6);
  PORTD |= (1<<PD7);

  //initialize ADC
  InitADC();

  while (1) {

      //reading potentiometer value
      //adc_value = ReadADC(0);
      adc_value = ADCW;

      if ((adc_value > 700) || (adc_value < 300)) {

        // enable stepper driver
        PORTD |= (1<<PD6);

        if (adc_value > 700) 
          // sense of rotation, bit = 1
          PORTD |= (1<<PD7);
          // sense LEDs
          PORTD |= (1<<PD2);
          PORTD &= ~(1<<PD4);

        if (adc_value < 300) 
          // invert sense of rotation, bit = 0
          PORTD &= ~(1<<PD7);
          // sense LEDs
          PORTD &= ~(1<<PD2);
          PORTD |= (1<<PD4);
        
        // swap clock pin
        PORTD ^= (1<<PD5);
        delay_ms(8);

      } else {

        // disable stepper driver
        PORTD &= ~(1<<PD5);
        PORTD &= ~(1<<PD6);

        // sense LEDs off
        PORTD &= ~(1<<PD2);
        PORTD &= ~(1<<PD4);
        __no_operation();
      }
  }

  return 0;
}


