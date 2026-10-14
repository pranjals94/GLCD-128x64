#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "i2c.h"

volatile uint8_t received_buffer[BUFFER_SIZE];
volatile uint8_t received_count = 0;
volatile uint8_t data_ready = 0;


void TWI_Init(void)
{
    // ATmega8 slave address = 0x08
    TWAR = (SLAVE_ADDRESS << 1);

    // Enable TWI + interrupt + ACK
    TWCR =
        (1 << TWEN) |
        (1 << TWIE) |
        (1 << TWEA);

    sei();
}


ISR(TWI_vect)
{
    uint8_t status = TWSR & 0xF8;

    switch (status)
    {
        /*
         * Own SLA+W received
         */
        case 0x60:

            received_count = 0;
            data_ready = 0;

            TWCR =
                (1 << TWINT) |
                (1 << TWEN)  |
                (1 << TWEA)  |
                (1 << TWIE);

            break;


        /*
         * Data received, ACK returned
         */
        case 0x80:

            if (received_count < BUFFER_SIZE)
            {
                received_buffer[received_count] = TWDR;
                received_count++;
            }

            TWCR =
                (1 << TWINT) |
                (1 << TWEN)  |
                (1 << TWEA)  |
                (1 << TWIE);

            break;


        /*
         * STOP condition received
         */
        case 0xA0:

            data_ready = 1;

            TWCR =
                (1 << TWINT) |
                (1 << TWEN)  |
                (1 << TWEA)  |
                (1 << TWIE);

            break;


        /*
         * SLA+R received
         */
        case 0xA8:

            TWDR = 0xAA;

            TWCR =
                (1 << TWINT) |
                (1 << TWEN) |
                (1 << TWEA)  |
                (1 << TWIE);

            break;


        /*
         * Data transmitted, ACK received
         */
        case 0xB8:

            TWDR = 0xAA;

            TWCR =
                (1 << TWINT) |
                (1 << TWEN) |
                (1 << TWEA) |
                (1 << TWIE);

            break;


        /*
         * Bus error
         */
        case 0x00:

            /*
             * Reset TWI peripheral
             */
            TWCR = 0;

            TWCR =
                (1 << TWEN) |
                (1 << TWIE) |
                (1 << TWEA);

            received_count = 0;
            data_ready = 0;

            break;


        /*
         * No relevant state information
         */
        case 0xF8:

            TWCR =
                (1 << TWINT) |
                (1 << TWEN)  |
                (1 << TWEA)  |
                (1 << TWIE);

            break;


        /*
         * Unknown state
         */
        default:

            /*
             * Reinitialize TWI
             */
            TWCR = 0;

            TWCR =
                (1 << TWEN) |
                (1 << TWIE) |
                (1 << TWEA);

            received_count = 0;
            data_ready = 0;

            break;
    }
}



void make_msg(uint8_t *row, uint8_t *column){
	if (received_count >= 4)
	{
		*row     = received_buffer[1];
		*column  = received_buffer[2];

			uint8_t msg_length = received_count - 3;

			// Move message from index 3 to index 0
			for (uint8_t i = 0; i < msg_length; i++)
			{
				received_buffer[i] = received_buffer[i + 3];
			}

			// Add NULL terminator
			received_buffer[msg_length] = '\0';
	}

}