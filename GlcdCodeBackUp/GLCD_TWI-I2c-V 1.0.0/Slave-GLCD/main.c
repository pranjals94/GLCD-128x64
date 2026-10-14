#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#include "i2c.h"
#include "glcd.h"

volatile uint32_t millis_count = 0;


/* =========================================================
   BUTTON SETTINGS
   ========================================================= */

#define BUTTON_PORT     PORTC
#define BUTTON_PIN_REG  PINC
#define BUTTON_DDR      DDRC
#define BUTTON_PIN      PC3

#define DEBOUNCE_TIME   30      // 30 ms
#define LONG_PRESS_TIME 1000    // 1 second


// timer 2 for backlight pwm
void LED_PWM_Init(void)
{
	// PB3 (OC2) as output
	DDRB |= (1 << PB3);

	// Fast PWM mode, non-inverting output
	TCCR2 = (1 << WGM20) |
	(1 << WGM21) |
	(1 << COM21) |
	(1 << CS21);    // Prescaler = 8

	OCR2 = 192; 
}
/*
OCR2 = 0;    // OFF
OCR2 = 64;   // Approximately 25% brightness
OCR2 = 128;  // Approximately 50% brightness
OCR2 = 192;  // Approximately 75% brightness
OCR2 = 255;  // Maximum
*/

/* =========================================================
   TIMER1 -> 1 ms TIME BASE
   ATmega8 @ 16 MHz

   16 MHz / 64 = 250 kHz
   250 kHz / 250 = 1000 Hz
   1000 Hz = 1 ms
   ========================================================= */

void Millis_Init(void)
{
    /* CTC mode */
    TCCR1B |= (1 << WGM12);

    /* Prescaler = 64 */
    TCCR1B |= (1 << CS11) | (1 << CS10);

    /* 250 timer counts = 1 ms */
    OCR1A = 249;

    /* Enable Timer1 Compare A interrupt */
    TIMSK |= (1 << OCIE1A);

    /* Enable global interrupts */
    sei();
}


/* =========================================================
   TIMER1 COMPARE A ISR
   ========================================================= */

ISR(TIMER1_COMPA_vect)
{
    millis_count++;
}


/* =========================================================
   ATOMIC MILLIS READ
   ========================================================= */

uint32_t millis(void)
{
    uint32_t m;

    cli();

    m = millis_count;

    sei();

    return m;
}
//---------------------------
/* =========================================================
   BUTTON
   PC3 = INPUT
   Active LOW
   Button connected between PC3 and GND
   ========================================================= */

#define BUTTON_PIN PC3

void Button_Init(void)
{
    /* PC3 as input */
    DDRC &= ~(1 << BUTTON_PIN);

    /* Enable internal pull-up */
    PORTC |= (1 << BUTTON_PIN);
}


/* =========================================================
   BUTTON EVENT
   Returns:
      0 = no press
      1 = short press
      2 = long press

   Short press = < 1 second
   Long press  = >= 1 second
   ========================================================= */

uint8_t Button_GetEvent(void)
{
    uint16_t press_time = 0;

    /* Button not pressed */
    if (PINC & (1 << BUTTON_PIN))
    {
        return 0;
    }

    /* Debounce */
    _delay_ms(DEBOUNCE_TIME);

    /* Check again */
    if (PINC & (1 << BUTTON_PIN))
    {
        return 0;
    }

    /* Button is pressed */
    while (!(PINC & (1 << BUTTON_PIN)))
    {
        _delay_ms(10);

        press_time += 10;

        /*
         * Long press detected.
         *
         * We can return immediately after
         * 1 second instead of waiting for release.
         */
        if (press_time >= LONG_PRESS_TIME)
        {	
			GLCD_Icon16x16(0,112,2);// power button
			 /* Button is pressed */
			 while (!(PINC & (1 << BUTTON_PIN)))
			 {
				 _delay_ms(DEBOUNCE_TIME);
			 }
			 
            /* Release debounce */
            _delay_ms(DEBOUNCE_TIME);
            return 2;       // LONG PRESS
        }
    }

    /* Release debounce */
    _delay_ms(DEBOUNCE_TIME);

    return 1;               // SHORT PRESS
}



/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
	_delay_ms(15); // let power stablize
    uint32_t last_millis = 0;

    uint8_t button_event;

    uint8_t row;
    uint8_t column;
	
	LED_PWM_Init();
	
    /* Initialize I2C slave */
    TWI_Init();
	
	GLCD_reset(); // hardware reset
	_delay_ms(5);

    /* Initialize GLCD */
    GLCD_Init();

    /* Initialize 1 ms timer */
    Millis_Init();

    /* Initialize PC3 button */
    Button_Init();

    /* -----------------------------------------------------
       Initial display
       ----------------------------------------------------- */
	
    GLCD_String16(0, 10, "Graphical");

	for(uint8_t i=0;i<6;i++){
		uint8_t col =i*17;
		GLCD_Icon16x16(2, col, i);// display icons
	}
    GLCD_String16(4, 61, "Display");
	GLCD_String8(6, 0, "Use level shifters in");
	GLCD_String8(7, 0, "SCL-SDA for 3.3v MCU");
    _delay_ms(5000);
    GLCD_Clear();

    /* =====================================================
       MAIN LOOP
       ===================================================== */

    while (1)
    {

        /* =================================================
           BUTTON
           ================================================= */

        button_event = Button_GetEvent();

        /* -------------------------------------------------
           SHORT PRESS
           ------------------------------------------------- */

        if (button_event == 1)
        {
            uint8_t screen = 0;
			/*
             * Put your SHORT PRESS code here
             */
            GLCD_DisplayScreen(screen);
			
            while(1){
			
			button_event = Button_GetEvent();
			if (button_event == 1){ // short press
				GLCD_Clear();
				screen++;
				if (screen >= 4)
				{
					GLCD_Clear();
					break;
				}
				GLCD_DisplayScreen(screen);
				
			}else if(button_event == 2) // long press
			{
				GLCD_Clear();
				break;
			}
			}
        }



        /* =================================================
           I2C DATA
           ================================================= */

        if (data_ready)
        {
            data_ready = 0;

            last_millis = millis();

            switch (received_buffer[0])
            {
                case CMD_STRING8:

                    make_msg(&row, &column);
					GLCD_SyncControllers();
                    GLCD_String8(row, column, received_buffer);

                    break;


                case CMD_STRING16:

                    make_msg(&row, &column);
					GLCD_SyncControllers();
                    GLCD_String16(row, column, received_buffer);

                    break;


                case CMD_CLEAR:
					GLCD_SyncControllers();
                    GLCD_Clear();

                    break;


                case CMD_16by16Icon:

                    make_msg(&row, &column);
					GLCD_SyncControllers();
                    GLCD_Icon16x16(
                        row,
                        column,
                        (uint8_t)atoi((char *)received_buffer)
                    );

                    break;
					
				case CMD_GLCD_reset:
					GLCD_reset();
					GLCD_Init();// re initialize
				
					break;
				
				case CMD_GLCD_Backlight_Pwm:
					make_msg(&row, &column);
					OCR2 = (uint8_t)atoi((char *)received_buffer);
				break;


                default:

                    /* Unknown command */

                    break;
            }


            /* Reset after processing */
            received_count = 0;
        }


        /* =================================================
           NO SIGNAL TIMEOUT
           ================================================= */

        else
        {
            if (millis() - last_millis > 10000)
            {
				GLCD_SyncControllers();
                No_Signal();
                GLCD_Clear();
            }
        }
    }
}