#include "driver/ledc.h"

#ifndef PINS_H
#define PINS_H

// front and back motor pins
#define FRONT_MOTOR_LPWM                4
#define FRONT_MOTOR_LPWM_CHANEL         LEDC_CHANNEL_0

#define FRONT_MOTOR_RPWM                5
#define FRONT_MOTOR_RPWM_CHANEL         LEDC_CHANNEL_1

#define BACK_MOTOR_LPWM                 12
#define BACK_MOTOR_LPWM_CHANEL          LEDC_CHANNEL_2

#define BACK_MOTOR_RPWM                 13
#define BACK_MOTOR_RPWM_CHANEL          LEDC_CHANNEL_3

// direction control pins
#define FRONT_SERVO                     21
#define FRONT_SERVO_CHANEL              LEDC_CHANNEL_4

#define BACK_SERVO                      22
#define BACK_SERVO_CHANEL               LEDC_CHANNEL_5

// LCD screen pins
#define LCD_I2C_MASTER_SDA_IO           18
#define LCD_I2C_MASTER_SDA_IO_CHANNEL   LEDC_CHANNEL_6

#define LCD_I2C_MASTER_SCL_IO           19
#define LCD_I2C_MASTER_SCL_IO_CHANNEL   LEDC_CHANNEL_7

// Servolar
#define SERVO_1                      14
#define SERVO_1_CHANEL               LEDC_CHANNEL_0

#define SERVO_2                      15
#define SERVO_2_CHANEL               LEDC_CHANNEL_1

#define SERVO_3                      16
#define SERVO_3_CHANEL               LEDC_CHANNEL_2

#define SERVO_4                      17
#define SERVO_4_CHANEL               LEDC_CHANNEL_3

#define SERVO_5                      23
#define SERVO_5_CHANEL               LEDC_CHANNEL_4

#define SERVO_6                      25
#define SERVO_6_CHANEL               LEDC_CHANNEL_5

#define SERVO_7                      26
#define SERVO_7_CHANEL               LEDC_CHANNEL_6


// keyboard
#define ROW_1 34
#define ROW_2 35
#define ROW_3 36
#define ROW_4 39

#define COL_1 16
#define COL_2 17
#define COL_3 21

#endif
