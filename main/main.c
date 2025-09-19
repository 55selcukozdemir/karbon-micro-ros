#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"


#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>
#include "esp32_serial_transport.h"



#define GPIO_OUTPUT_IO_0    CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1    CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))

#define SENSOR_PIN GPIO_NUM_4

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}


#define STRING_BUFFER_LEN 50

rcl_publisher_t test;

rcl_subscription_t front_wheel_control_subscriber;
rcl_subscription_t back_wheel_control_subscriber;

std_msgs__msg__String outcoming_test;

std_msgs__msg__Int32 incoming_front_wheel_angel;
std_msgs__msg__Int32 incoming_back_wheel_angel;

static int pulse_count = 0;
// ISR fonksiyonu
static void IRAM_ATTR sensor_isr_handler(void* arg) {
	pulse_count++;  // her tetikleme bir "pulse"
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	const char *msg_str = "Merhaba ESP32!";
    size_t len = strlen(msg_str);
    if(len < outcoming_test.data.capacity) {
        strcpy(outcoming_test.data.data, msg_str);
        outcoming_test.data.size = len;
    }
    RCSOFTCHECK(rcl_publish(&test, &outcoming_test, NULL));
}

void front_wheel_callback (const void * msgin){
	const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *) msgin;

	if(incoming_front_wheel_angel.data != msg->data){
		incoming_front_wheel_angel.data = msg->data;
		// Veri yaz
		const char *msg = "Merhaba ESP32!";
		size_t len = strlen(msg);
		if(len < outcoming_test.data.capacity) {
			strcpy(outcoming_test.data.data, msg);
			outcoming_test.data.size = len;
		}
		RCSOFTCHECK(rcl_publish(&test, &outcoming_test, NULL));
	}
}

void back_wheel_callback (const void * msgin){
	const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *) msgin;

	if(incoming_back_wheel_angel.data != msg->data){
		incoming_back_wheel_angel.data = msg->data;
		// Veri yaz
		const char *msg = "Merhaba ESP32!";
		size_t len = strlen(msg);
		if(len < outcoming_test.data.capacity) {
			strcpy(outcoming_test.data.data, msg);
			outcoming_test.data.size = len;
		}
		RCSOFTCHECK(rcl_publish(&test, &outcoming_test, NULL));
	}
}

void micro_ros_task(void * arg)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SENSOR_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // sensöre göre gerekebilir
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE    // düşen kenarda tetikle
    };
    gpio_config(&io_conf);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(SENSOR_PIN, sensor_isr_handler, NULL);

	
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;
	
	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	RCCHECK(rcl_init_options_init(&init_options, allocator));

	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "car_drive", "/car", &support));

	RCCHECK(rclc_subscription_init_best_effort(&front_wheel_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "/car/car_front_wheel"));
	RCCHECK(rclc_subscription_init_best_effort(&back_wheel_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "/car/car_back_wheel"));

	RCCHECK(rclc_publisher_init_best_effort(&test, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "/car/test"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default2(&timer, &support, RCL_MS_TO_NS(timer_timeout), timer_callback, true));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));
	RCCHECK(rclc_executor_add_subscription(&executor, &front_wheel_control_subscriber, &incoming_front_wheel_angel, front_wheel_callback, ON_NEW_DATA));
	RCCHECK(rclc_executor_add_subscription(&executor, &back_wheel_control_subscriber, &incoming_back_wheel_angel, back_wheel_callback, ON_NEW_DATA));


	// Statik buffer oluştur
	char outcoming_test_buffer[STRING_BUFFER_LEN];
	// Mesaj alanına buffer’ı ata
	outcoming_test.data.data = outcoming_test_buffer;
	outcoming_test.data.capacity = STRING_BUFFER_LEN;

	while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
		usleep(100000);
	}

	// free resources
	RCCHECK(rcl_subscription_fini(&front_wheel_control_subscriber, &node));
	RCCHECK(rcl_subscription_fini(&back_wheel_control_subscriber, &node));
	RCCHECK(rcl_node_fini(&node));

  	vTaskDelete(NULL);
}

static uart_port_t uart_port = UART_NUM_0;

void app_main(void)
{
#if defined(RMW_UXRCE_TRANSPORT_CUSTOM)
	rmw_uros_set_custom_transport(
		true,
		(void *) &uart_port,
		esp32_serial_open,
		esp32_serial_close,
		esp32_serial_write,
		esp32_serial_read
	);
#else
#error micro-ROS transports misconfigured
#endif  // RMW_UXRCE_TRANSPORT_CUSTOM

    xTaskCreate(micro_ros_task,
            "uros_task",
            CONFIG_MICRO_ROS_APP_STACK,
            NULL,
            CONFIG_MICRO_ROS_APP_TASK_PRIO,
            NULL);
}