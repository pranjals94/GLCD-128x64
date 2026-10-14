#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <string.h>

/* =========================================================
   I2C SLAVE ADDRESS
   ========================================================= */

#define SLAVE_ADDRESS 0x08
#define BUFFER_SIZE   25

/* =========================================================
   COMMANDS
   ========================================================= */

#define CMD_STRING8   0x01
#define CMD_STRING16  0x02
#define CMD_CLEAR     0x03
#define CMD_16by16Icon  0x04
#define CMD_GLCD_reset 0x05
#define CMD_GLCD_Backlight_Pwm 0x06

extern volatile uint8_t received_buffer[BUFFER_SIZE];
extern volatile uint8_t received_count;
extern volatile uint8_t data_ready;

/* =========================================================
   INITIALIZE I2C SLAVE
   ========================================================= */

void TWI_Init(void);
void make_msg(uint8_t *row, uint8_t *column);

#endif