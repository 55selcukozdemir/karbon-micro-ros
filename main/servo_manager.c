#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include <stdio.h>
#include "pins.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE

#define LEDC_MODE_SERVOS LEDC_SPEED_MODE_MAX
#define LEDC_RES LEDC_TIMER_16_BIT
#define SERVO_FREQ_HZ 50 // Servo için frekans

// Servo açısını duty değerine çevir
uint32_t servo_angle_to_duty(float angle)
{
    // 0° = 0.5ms, 180° = 2.5ms
    float pulse_ms = 0.5 + (angle / 180.0) * 2.0;
    float period_ms = 1000.0 / SERVO_FREQ_HZ; // 20ms
    uint32_t duty = (uint32_t)((pulse_ms / period_ms) * ((1 << LEDC_RES) - 1));
    return duty;
}

// Servo ve LEDC başlat
void servo_init()
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = SERVO_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t front_channel_cfg = {
        .gpio_num = FRONT_SERVO,
        .speed_mode = LEDC_MODE,
        .channel = FRONT_SERVO_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&front_channel_cfg);

    ledc_channel_config_t back_channel_cfg = {
        .gpio_num = BACK_SERVO,
        .speed_mode = LEDC_MODE,
        .channel = BACK_SERVO_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&back_channel_cfg);

    //  servo
    ledc_channel_config_t servo_1_channel_cfg = {
        .gpio_num = SERVO_1,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_1_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_1_channel_cfg);

    ledc_channel_config_t servo_2_channel_cfg = {
        .gpio_num = SERVO_2,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_2_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_2_channel_cfg);

    ledc_channel_config_t servo_3_channel_cfg = {
        .gpio_num = SERVO_3,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_3_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_3_channel_cfg);

    ledc_channel_config_t servo_4_channel_cfg = {
        .gpio_num = SERVO_4,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_4_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_4_channel_cfg);

    ledc_channel_config_t servo_5_channel_cfg = {
        .gpio_num = SERVO_5,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_5_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_5_channel_cfg);

    ledc_channel_config_t servo_6_channel_cfg = {
        .gpio_num = SERVO_6,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_6_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_6_channel_cfg);

    ledc_channel_config_t servo_7_channel_cfg = {
        .gpio_num = SERVO_7,
        .speed_mode = LEDC_MODE_SERVOS,
        .channel = SERVO_7_CHANEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&servo_7_channel_cfg);
}

void back_wheel_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE, BACK_SERVO_CHANEL, duty);
    ledc_update_duty(LEDC_MODE, BACK_SERVO_CHANEL);
}

void front_wheel_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE, FRONT_SERVO_CHANEL, duty);
    ledc_update_duty(LEDC_MODE, FRONT_SERVO_CHANEL);
}

void servo_1_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_1_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_1_CHANEL);
}

void servo_2_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_2_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_2_CHANEL);
}

void servo_3_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_3_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_3_CHANEL);
}

void servo_4_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_4_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_4_CHANEL);
}

void servo_5_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_5_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_5_CHANEL);
}

void servo_6_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_6_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_6_CHANEL);
}

void servo_7_angle(float angle)
{
    uint32_t duty = servo_angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE_SERVOS, SERVO_7_CHANEL, duty);
    ledc_update_duty(LEDC_MODE_SERVOS, SERVO_7_CHANEL);
}