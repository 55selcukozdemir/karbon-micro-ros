#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include <stdio.h>
#include "pins.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
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