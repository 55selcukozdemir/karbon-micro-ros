#include "driver/ledc.h"
#include "pins.h"

#define DC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define DC_TIMER_NUM LEDC_TIMER_0

void dc_init()
{
    // LEDC PWM timer ayarı
    ledc_timer_config_t ledc_timer = {
        .speed_mode = DC_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT, // 0-1023 duty
        .freq_hz = 15000,                     // 20 kHz PWM (motor için sessiz çalışır)
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    // front dc motor
    // RPWM kanalı
    ledc_channel_config_t front_channel_r = {
        .gpio_num = FRONT_MOTOR_RPWM,
        .speed_mode = DC_SPEED_MODE,
        .channel = FRONT_MOTOR_RPWM_CHANEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0};
    ledc_channel_config(&front_channel_r);

    // LPWM kanalı
    ledc_channel_config_t front_channel_l = {
        .gpio_num = FRONT_MOTOR_RPWM,
        .speed_mode = DC_SPEED_MODE,
        .channel = FRONT_MOTOR_RPWM_CHANEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0};
    ledc_channel_config(&front_channel_l);

    // back dc motor
    // RPWM kanalı
    ledc_channel_config_t back_channel_r = {
        .gpio_num = BACK_MOTOR_RPWM,
        .speed_mode = DC_SPEED_MODE,
        .channel = BACK_MOTOR_RPWM_CHANEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0};
    ledc_channel_config(&back_channel_r);

    // LPWM kanalı
    ledc_channel_config_t back_channel_l = {
        .gpio_num = BACK_MOTOR_RPWM,
        .speed_mode = DC_SPEED_MODE,
        .channel = BACK_MOTOR_RPWM_CHANEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0};
    ledc_channel_config(&back_channel_l);
}

void front_wheel(float duty)
{
    if (duty < 0)
    {
        ledc_set_duty(DC_SPEED_MODE, FRONT_MOTOR_LPWM_CHANEL, duty * -1);
        ledc_update_duty(DC_SPEED_MODE, FRONT_MOTOR_LPWM_CHANEL);
        ledc_set_duty(DC_SPEED_MODE, FRONT_MOTOR_RPWM_CHANEL, 0);
        ledc_update_duty(DC_SPEED_MODE, FRONT_MOTOR_RPWM_CHANEL);
    }
    else
    {
        ledc_set_duty(DC_SPEED_MODE, FRONT_MOTOR_RPWM_CHANEL, duty);
        ledc_update_duty(DC_SPEED_MODE, FRONT_MOTOR_RPWM_CHANEL);
        ledc_set_duty(DC_SPEED_MODE, FRONT_MOTOR_LPWM_CHANEL, 0);
        ledc_update_duty(DC_SPEED_MODE, FRONT_MOTOR_LPWM_CHANEL);
    }
}

void back_wheel(float duty)
{
    if (duty < 0)
    {
        ledc_set_duty(DC_SPEED_MODE, BACK_MOTOR_LPWM_CHANEL, duty * -1);
        ledc_update_duty(DC_SPEED_MODE, BACK_MOTOR_LPWM_CHANEL);
        ledc_set_duty(DC_SPEED_MODE, BACK_MOTOR_RPWM_CHANEL, 0);
        ledc_update_duty(DC_SPEED_MODE, BACK_MOTOR_RPWM_CHANEL);
    }
    else
    {
        ledc_set_duty(DC_SPEED_MODE, BACK_MOTOR_RPWM_CHANEL, duty);
        ledc_update_duty(DC_SPEED_MODE, BACK_MOTOR_RPWM_CHANEL);
        ledc_set_duty(DC_SPEED_MODE, BACK_MOTOR_LPWM_CHANEL, 0);
        ledc_update_duty(DC_SPEED_MODE, BACK_MOTOR_LPWM_CHANEL);
    }
}