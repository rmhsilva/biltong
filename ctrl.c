/**
 * Because of stupid trimpots
 */

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../iocompat.h"

// Frequency adjustment, results in ~25kHz PWM.
#define FREQ_ADJ  160
#define PWM_MAX   0.9*FREQ_ADJ   // Max PWM (of 255) allowed for fan = 90%
#define PWM_MIN   0.1*FREQ_ADJ   // And min

// Restrict (clamp) value
#define CLAMP(val,max,min) ((val<min)? min : (val>max)? max : val)

void init() {
  
  // Setup PWM with non-prescaled clock source
  // TCCR1A = TIMER1_PWM_INIT;  // Enables OCR1A
  TCCR1B |= _BV(CS11);
  GTCCR   = _BV(PWM1B) | _BV(COM1B1);  // Enables OCR1B

  // Set PWM duty cycle to 50%
  OCR1B = TIMER1_TOP / 2;

  // Set PWM frequency (kinda)
  OCR1C = FREQ_ADJ;

  // Enable PWM output
  DDROC = _BV(PB4);

  // Enable ADC pin pullup resistor
  PORTB = _BV(3);

  // Enable ADC
  ADCSRA = _BV(ADEN);

  // Initialise (take first reading)
  ADCSRA |= _BV(ADSC);

  // Enable free running mode
  ADCSRA |= _BV(ADATE);

  // x128 Prescaler for ADC clock
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  // Select correct ADC input
  ADMUX = _BV(MUX0) | _BV(MUX1);
}


int main(int argc, char const *argv[])
{
  uint16_t voltage;

  init();

  // Update speed every second.
  // Yes this could be done better using interrupts, but this took 2 seconds.
  for (;;) {
    voltage = ADCL; 
    voltage |= ADCH << 8;

    // Get a duty cycle (0..0xFF) from a relative val (0..0x3FF)
    OCR1B = CLAMP(voltage >> 2, PWM_MAX, PWM_MIN);

    _delay_ms(1000);
  }

  return 0;
}