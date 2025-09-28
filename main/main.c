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

#define GPIO_OUTPUT_IO_0 CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1 CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))

#define SENSOR_PIN GPIO_NUM_4

#define RCCHECK(fn)                                                                      \
	{                                                                                    \
		rcl_ret_t temp_rc = fn;                                                          \
		if ((temp_rc != RCL_RET_OK))                                                     \
		{                                                                                \
			printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
			vTaskDelete(NULL);                                                           \
		}                                                                                \
	}
#define RCSOFTCHECK(fn)                                                                    \
	{                                                                                      \
		rcl_ret_t temp_rc = fn;                                                            \
		if ((temp_rc != RCL_RET_OK))                                                       \
		{                                                                                  \
			printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
		}                                                                                  \
	}

#define STRING_BUFFER_LEN 50

rcl_publisher_t test;

rcl_subscription_t front_wheel_control_subscriber;
rcl_subscription_t back_wheel_control_subscriber;

std_msgs__msg__String outcoming_test;

std_msgs__msg__Int32 incoming_front_wheel_angel;
std_msgs__msg__Int32 incoming_back_wheel_angel;

int counter = 0;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
	(void)last_call_time;
	if (timer != NULL)
	{
		sprintf(outcoming_test.data.data, "Hello from micro-ROS #%d", counter++);
		outcoming_test.data.size = strlen(outcoming_test.data.data);
		RCSOFTCHECK(rcl_publish(&test, &outcoming_test, NULL));
	}
}

void front_wheel_callback(const void *msgin)
{
	const std_msgs__msg__Int32 * incoming_front_wheel_angel = (const std_msgs__msg__Int32 *)msgin;
}

void back_wheel_callback(const void *msgin)
{
	const std_msgs__msg__Int32 * incoming_back_wheel_angel = (const std_msgs__msg__Int32 *)msgin;
}

void micro_ros_task(void *arg)
{

	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	RCCHECK(rcl_init_options_init(&init_options, allocator));

	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "car_drive", "", &support));
	RCCHECK(rclc_publisher_init_default(&test, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "test2"));

	RCCHECK(rclc_subscription_init_default(&front_wheel_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "car_front_wheel"));
	RCCHECK(rclc_subscription_init_default(&back_wheel_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "car_back_wheel"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(timer_timeout), timer_callback));

	// create executor
	rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
	RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));
	RCCHECK(rclc_executor_add_subscription(&executor, &front_wheel_control_subscriber, &incoming_front_wheel_angel, &front_wheel_callback, ON_NEW_DATA));
	RCCHECK(rclc_executor_add_subscription(&executor, &back_wheel_control_subscriber, &incoming_back_wheel_angel, &back_wheel_callback, ON_NEW_DATA));

	// Fill the array with a known sequence
	outcoming_test.data.data = (char *)malloc(STRING_BUFFER_LEN * sizeof(char));
	outcoming_test.data.size = 0;
	outcoming_test.data.capacity = STRING_BUFFER_LEN;
	rclc_executor_spin(&executor);

	// free resources
	RCCHECK(rcl_subscription_fini(&front_wheel_control_subscriber, &node));
	RCCHECK(rcl_subscription_fini(&back_wheel_control_subscriber, &node));
	RCCHECK(rcl_publisher_fini(&test, &node));
	RCCHECK(rcl_node_fini(&node));

	vTaskDelete(NULL);
}

static uart_port_t uart_port = UART_NUM_0;
void app_main(void)
{
#if defined(CONFIG_MICRO_ROS_ESP_NETIF_WLAN) || defined(CONFIG_MICRO_ROS_ESP_NETIF_ENET)
    ESP_ERROR_CHECK(uros_network_interface_initialize());
#endif

    //pin micro-ros task in APP_CPU to make PRO_CPU to deal with wifi:
    xTaskCreate(micro_ros_task,
            "uros_task",
            CONFIG_MICRO_ROS_APP_STACK,
            NULL,
            CONFIG_MICRO_ROS_APP_TASK_PRIO,
            NULL);
}
