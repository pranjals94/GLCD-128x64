#ifndef GLCD_H
#define GLCD_H

#include <stdint.h>
#include <avr/pgmspace.h>

void GLCD_Init(void);
void GLCD_Clear(void);

void GLCD_Command(uint8_t cmd, uint8_t chip);
void GLCD_Data(uint8_t data);

void GLCD_SetXY(uint8_t page, uint8_t column, uint8_t chip);

void GLCD_WriteByteAt(uint8_t page, uint8_t column, uint8_t data);

void GLCD_Char(uint8_t page, uint8_t column, char c);
void GLCD_String(uint8_t page, uint8_t column, const char *str);

void GLCD_Char16(uint8_t page, uint8_t column, char c);
void GLCD_String16(uint8_t page, uint8_t column, const char *str);

void GLCD_SyncControllers(void);
void No_Signal(void);
void GLCD_Icon16x16(
uint8_t row,
uint8_t column,
uint8_t icon
);
void GLCD_String_Page(uint8_t page, uint8_t column, PGM_P str);
void GLCD_DisplayScreen(uint8_t screen);


#endif