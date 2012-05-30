/*
Copyright (c) 2012, fabiodive@gmail.com All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, self
list of conditions and the following disclaimer. Redistributions in binary
form must reproduce the above copyright notice, self list of conditions and
the following disclaimer in the documentation and/or other materials
provided with the distribution. Neither the name of fabiodive@gmail.com nor
the names of its contributors may be used to endorse or promote products
derived from self software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

fabiodive@gmail.com

*/


#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdlib.h>


///////////////////////////////////////////////////////////
// Defines
//

// CPU CLOCK
#define F_CPU 16000000UL 

// USART
#define BAUDRATE 57600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)


///////////////////////////////////////////////////////////
// Functions declaration
//

// Initialize USART
void USART_init(void){
  UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
  UBRR0L = (uint8_t)(BAUD_PRESCALLER);
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  UCSR0C = (3<<UCSZ00);
}

// RX
unsigned char USART_receive(void){
  while(!(UCSR0A & (1<<RXC0)));
  return UDR0;
}

// TX
void USART_send(unsigned char data){
  while(!(UCSR0A & (1<<UDRE0)));
  UDR0 = data;
}

// TX string
void USART_putstring(char* StringPtr){
  while(*StringPtr != 0x00){
    USART_send(*StringPtr);
    StringPtr++;
  }
  // Newline for pc serial monitoring
  USART_send('\n');
}

// Delay function, canonical _delay_ms 
// doesn't accept variables but constants
void delay_ms(uint16_t count) {
  while(count--) {
    _delay_ms(1);
  }
} 

// ADC Init
void InitADC() {
  // set prescaler to 128
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
  // Select Vref=AVcc
  // Avcc(+5v) as voltage reference
  ADMUX |= (1<<REFS0);
  ADMUX &= ~(1<<REFS1);
  // ADC in free-running  mode
  ADCSRB &= ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0));
  // Signal source, in this case is the free-running
  ADCSRA |= (1<<ADATE);
  // Power up the ADC
  ADCSRA |= (1<<ADEN);
  // Start converting
  ADCSRA |= (1<<ADSC);
}

uint16_t ReadADC(uint8_t ADCchannel) {
  // select ADC channel with safety mask
  ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
  // single conversion mode
  ADCSRA |= (1<<ADSC);
  // wait until ADC conversion is complete
  while( ADCSRA & (1<<ADSC) );
  return ADC;
}


///////////////////////////////////////////////////////////
// MAIN Function
//

int main (void) {

  uint16_t delay;
  uint16_t adc_value;

  // String[] is in fact an array but when we put the text 
  // between the " " symbols the compiler threats it as a 
  // String and automatically puts the null termination 
  // character in the end of the text
 
  //char String[]="Hello world!!";
  char String[16];
 
  // Call the USART initialization code
  USART_init();
 
  // Initialize ADC
  InitADC();

  // Set sense LEDs in output mode
  DDRD |= (1<<PD3);
  DDRD |= (1<<PD4);

  // Set output mode of stepper driver pins
  // PD5=clock, PD6=enable, PD7=cw/ccw
  DDRD |= (1<<PD5);
  DDRD |= (1<<PD6);
  DDRD |= (1<<PD7);

  // Set ON stepper driver pins
  PORTD |= (1<<PD5);
  PORTD |= (1<<PD6);
  PORTD |= (1<<PD7);

  // Main Loop 
  for (;;) {

    // reading potentiometer value
    // adc_value = ReadADC(0);
    adc_value = ADCW;

    // I define two ranges where the stepper motor is enabled
    if ((adc_value > 611) || (adc_value < 412)) {

      // enable stepper driver
      PORTD |= (1 << PD6);

      // This is the higher range
      if (adc_value > 611) { 
        // sense of rotation, bit = 1
        PORTD |= (1 << PD7);
        // sense LEDs
        PORTD |= (1 << PD3);
        PORTD &= ~(1 << PD4);
        
        // OK integer trunked result during division
        // are deci-millisecond
        // The result is reverted.
        delay = 9 - ((adc_value - 612) / 51);
      }

      // This is the lower range
      if (adc_value < 412) {
        // invert sense of rotation, bit = 0
        PORTD &= ~(1 << PD7);
        // sense LEDs
        PORTD &= ~(1 << PD3);
        PORTD |= (1 << PD4);

        delay = 1 + (adc_value / 51);
      }
      
      // swap clock pin
      PORTD ^= (1 << PD5);

      // convert integer delay to string 
      itoa(delay, String, 10);
      // sending the value to the monitoring station
      USART_putstring(String);

      // calling delay
      delay_ms(delay);

    // if here, not in enabled ranges
    } else {
      // disabling stepper driver
      PORTD &= ~(1 << PD5);
      PORTD &= ~(1 << PD6);

      // sense LEDs off
      PORTD &= ~(1 << PD3);
      PORTD &= ~(1 << PD4);
    }
  }

  return 0;
}

