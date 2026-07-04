#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "stm32f1xx_hal.h" // Funcțiile de bază ale plăcii tale

// Funcțiile pe care le putem apela
void lcd_init(void);   // Pornește ecranul
void lcd_send_cmd(char cmd);  // Trimite o comandă (ex: curăță ecranul)
void lcd_send_data(char data); // Trimite o literă
void lcd_send_string(const char *str); // Trimite un text întreg
void lcd_put_cur(int row, int col); // Mută cursorul
void lcd_clear(void); // Curăță tot ecranul

#endif