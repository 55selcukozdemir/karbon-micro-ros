void i2c_master_initt(void);
void write_string(const char * string);


void lcd_yaz_motor_hizi(char *string);
void lcd_yaz_yon(char *string);
void lcd_yaz_servo_acilari(char *acilar1, char *acilar2);
void format_data_to_strings(const int32_t *data, int size, char *str1, char *str2);