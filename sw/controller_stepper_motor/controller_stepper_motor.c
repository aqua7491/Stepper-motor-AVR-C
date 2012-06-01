/*
Copyright (c) 2012, fabiodive - fabiodive@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, self
list of conditions and the following disclaimer. Redistributions in binary
form must reproduce the above copyright notice, self list of conditions and
the following disclaimer in the documentation and/or other materials
provided with the distribution. Neither the name of fabiodive nor
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
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>


///////////////////////////////////////////////////////////
// Defines
//

// CPU CLOCK
#define F_CPU 16000000UL 

// USART
#define BAUDRATE 57600
// Set the clock for the desired baud rate
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

// encoder conversion activation threshold
// below it the interrupt get discharged
#define TH_ENABLE 10

// actual encoder steps
volatile uint16_t encoder_steps = 0;

// this toggle between 
volatile uint8_t previous_dir = 0; 

// memory buttons flag
volatile uint8_t mem_buttons_flag = 0;

uint16_t delay;

// ADC converted value from potentiometer 
// used for speed, 0 - 1023
uint16_t adc_value;

// String[] is in fact an array but when we put the text 
// between the " " symbols the compiler threats it as a 
// String and automatically puts the null termination 
// character in the end of the text

//char String[]="Hello world!!";
//char String[16];
 

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
  USART_send('\r');
  USART_send('\n');
}

// Delay function, canonical _delay_ms 
// doesn't accept variables but just constants
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
  //ADMUX &= ~(1<<REFS1);
  // ADC in free-running  mode
  //ADCSRB &= ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0));
  // Signal source, in this case is the free-running
  //ADCSRA |= (1<<ADATE);
  // Power up the ADC
  ADCSRA |= (1<<ADEN);
  // Do a conversion
  ADCSRA |= (1<<ADSC);
}

uint16_t ReadADC(uint8_t ADCchannel) {
  // Clear the channel selection
  ADMUX &= 0xF0;
  // select ADC channel with safety mask
  // maximum selection: 0b1111
  ADMUX |= (ADCchannel & 0x0F);
  // single conversion mode
  ADCSRA |= (1<<ADSC);
  // wait until ADC conversion is complete
  while( ADCSRA & (1<<ADSC) );
  return ADCW;
}


///////////////////////////////////////////////////////////
// Interrupts service routines
//

// Triggered by changing state of pin PB0
// Encoder phase A pin
ISR (PCINT0_vect) {

 /* 
 
 Those marked with * are the needed conditions

 *if two bits are equals and previous_dir = 0
    same direction as before

  if two bits are equals and previous_dir = 1
    new direction is opposite

  if two bits are differents and previous_dir = 0
    new direction is opposite

 *if two bits are differents and previous_dir = 1
    same direction as before

 */

  uint8_t phase_a;
  uint8_t phase_b;

  phase_a = (PINB & (1<<PB0));
  phase_b = (PINB & (1<<PB1)) >>1;

  /*
  USART_send(phase_a+48);
  USART_send('-');
  USART_send(phase_b+48);
  USART_send('-');
  USART_send(previous_dir+48);
  USART_send('-');
  USART_send(encoder_steps+48);
  USART_send('\r');
  USART_send('\n');
  */

  // if actual direction is equal to the previous
  if ( !((phase_a ^ phase_b) ^ previous_dir) ) {
    encoder_steps++;
    // This way we should be sure, a real encoder
    // rotation is begun
    if ( encoder_steps > TH_ENABLE ) {
      // enable stepper driver
      PORTD |= (1 << PD6);
      if (phase_a == phase_b) {
        PORTD &= ~(1 << PD3);
        PORTD |= (1 << PD4);
      } else {
        PORTD |= (1 << PD3);
        PORTD &= ~(1 << PD4);
      }
 
      // one motor step forward
      //uint8_t i;
      //for (i = 0; i < 4; i++) {
        // swap clock pin
        PORTD ^= (1 << PD5);
        _delay_ms(1);
        PORTD ^= (1 << PD5);
        _delay_ms(1);
      //}
      // I begin from 0 again to count
      encoder_steps = 0;
    }
  // Not over the threshold, this interrupt was the beginning of a
  // opposite encoder rotation or not a real command
  } else {
    // Toggle the direction of rotation
    // on stepper motor driver pin
    PORTD ^= (1 << PD7);
    // and the memory flag aswell
    if (previous_dir == 0) {
      previous_dir = 1;
    } else {
      previous_dir = 0;
    }
    // reset encoder counter
    encoder_steps = 0;
  }
}


// Triggered by changing state of pin PC1
// Memory buttons
ISR (PCINT1_vect) {
  mem_buttons_flag = 1;
}


///////////////////////////////////////////////////////////
// MAIN Function
//

int main (void) {

  // Call the USART initialization code
  USART_init();
 
  // Initialize ADC
  InitADC();

  // Push button LED
  DDRD |= (1<<PD2);

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

  // Set encoder A and B pins as input
  DDRB &= ~(1 << PB0);
  DDRB &= ~(1 << PB1);

  // Set pull-up resistor on encoder
  // A, B input pins
  PORTB |= (1<<PB0);
  PORTB |= (1<<PB1);

  // Set memory push button pin as input
  DDRC &= ~(1 << PC1);

  // Set pull-up resistor on push
  // button input pin
  PORTC |= (1<<PC1);

  // Enable interrupt on PCI0 vector
  PCICR |= (1<<PCIE0);
  // Enable interrupt on PCI1 vector
  PCICR |= (1<<PCIE1);
  // Enable interrupt for PCINT0 - PB0
  PCMSK0 |= (1<<PCINT0);
  // Enable interrupt for PCINT9 - PC1
  PCMSK1 |= (1<<PCINT9);

  // Enable interrupts
  sei();

  //--

  // Main Loop 
  for (;;) {

    if ( mem_buttons_flag ) {
      // reading potentiometer value
      // for speed value
      adc_value = ReadADC(0);

      if ( PINC & (1<<PC1) ) {
      // do something
      } else {
        mem_buttons_flag = 0;
      }
    }

    // disabling stepper driver
    PORTD &= ~(1 << PD5);
    PORTD &= ~(1 << PD6);

    // sense LEDs off
    PORTD &= ~(1 << PD3);
    PORTD &= ~(1 << PD4);
  }

  return 0;
}

