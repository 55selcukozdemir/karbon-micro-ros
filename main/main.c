#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "uros_network_interfaces.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <std_msgs/msg/string.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>
#include "esp32_serial_transport.h"
#include "screen_manager.h"
#include "servo_manager.h"

#include "pins.h"

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
std_msgs__msg__String outcoming_test;

rcl_subscription_t hiz_control_subscriber;
rcl_subscription_t yon_control_subscriber;
rcl_subscription_t servolar_control_subscriber;

std_msgs__msg__Int32 incoming_hiz;
std_msgs__msg__Int32MultiArray incoming_yon;
std_msgs__msg__Int32MultiArray incoming_servolar;

// ---------------------------
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

// --------------------------------------
void hiz_callback(const void *msgin)
{
	const std_msgs__msg__Int32 *incoming_back_wheel_angel = (const std_msgs__msg__Int32 *)msgin;
	ESP_LOGI("sub", "hiz");
	char str[12];
	snprintf(str, sizeof(str), "Hiz:%04ld", incoming_back_wheel_angel->data);
	lcd_yaz_motor_hizi(str);
}
void yon_callback(const void *msgin)
{
	ESP_LOGI("sub", "yon");
	const std_msgs__msg__Int32MultiArray *msg = (const std_msgs__msg__Int32MultiArray *)msgin;
	char temp[6];
	char str1[12];
	strcat(str1, "|Yon:");
	for (int i = 0; i < msg->data.size; i++)
	{
		if(i == 0){
			snprintf(temp, sizeof(temp), "%03ld", msg->data.data[i]);
		}
		else{
			snprintf(temp, sizeof(temp), "-%03ld", msg->data.data[i]);
		}
		strcat(str1, temp); // str1'e veriyi ekle
	}
	ESP_LOGI("sub", "%s", str1);
	lcd_yaz_yon(str1);
}

void servolar_callback(const void *msgin)
{
	ESP_LOGI("sub", "servo");
	const std_msgs__msg__Int32MultiArray *msg = (const std_msgs__msg__Int32MultiArray *)msgin;

	char str1[21]; // İlk string, 20 karakter sınırı + null byte
	char str2[21]; // İkinci string, 20 karakter sınırı + null byte
	format_data_to_strings(msg->data.data, msg->data.size, str1, str2);
	lcd_yaz_servo_acilari(str1, str2);
}
// --------------------------------------

void micro_ros_task(void *arg)
{
	i2c_master_initt();
	servo_init();
	write_string("basladi");
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	RCCHECK(rcl_init_options_init(&init_options, allocator));

#ifdef CONFIG_MICRO_ROS_ESP_XRCE_DDS_MIDDLEWARE
	rmw_init_options_t *rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

	// Static Agent IP and port can be used instead of autodisvery.
	RCCHECK(rmw_uros_options_set_udp_address(CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT, rmw_options));
	// RCCHECK(rmw_uros_discover_agent(rmw_options));
#endif

	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "car_drive", "", &support));
	RCCHECK(rclc_publisher_init_default(&test, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "test_micro_ros"));

	RCCHECK(rclc_subscription_init_default(&hiz_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "hiz"));
	RCCHECK(rclc_subscription_init_default(&yon_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray), "yon"));
	RCCHECK(rclc_subscription_init_default(&servolar_control_subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray), "servolar"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(timer_timeout), timer_callback));

	// create executor
	rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
	RCCHECK(rclc_executor_init(&executor, &support.context, 4, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));
	RCCHECK(rclc_executor_add_subscription(&executor, &hiz_control_subscriber, &incoming_hiz, &hiz_callback, ON_NEW_DATA));
	RCCHECK(rclc_executor_add_subscription(&executor, &yon_control_subscriber, &incoming_yon, &yon_callback, ON_NEW_DATA));
	RCCHECK(rclc_executor_add_subscription(&executor, &servolar_control_subscriber, &incoming_servolar, &servolar_callback, ON_NEW_DATA));

	// Fill the array with a known sequence
	outcoming_test.data.data = (char *)malloc(STRING_BUFFER_LEN * sizeof(char));
	outcoming_test.data.size = 0;
	outcoming_test.data.capacity = STRING_BUFFER_LEN;

	// Init the memory of your array in order to provide it to the executor.
	// If a message from ROS comes and it is bigger than this, it will be ignored, so ensure that capacities here are big enought.
	incoming_yon.data.capacity = 2;
	incoming_yon.data.size = 0;
	incoming_yon.data.data = (int32_t *)malloc(incoming_yon.data.capacity * sizeof(int32_t));

	incoming_servolar.data.capacity = 8;
	incoming_servolar.data.size = 0;
	incoming_servolar.data.data = (int32_t *)malloc(incoming_yon.data.capacity * sizeof(int32_t));

	write_string("");
	while (1)
	{
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
		usleep(10000);
	}

	// free resources
	RCCHECK(rcl_subscription_fini(&hiz_control_subscriber, &node));
	RCCHECK(rcl_subscription_fini(&yon_control_subscriber, &node));
	RCCHECK(rcl_subscription_fini(&servolar_control_subscriber, &node));
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

	// i2c_master_initt();
	// pin micro-ros task in APP_CPU to make PRO_CPU to deal with wifi:
	xTaskCreate(micro_ros_task,
				"uros_task",
				CONFIG_MICRO_ROS_APP_STACK,
				NULL,
				CONFIG_MICRO_ROS_APP_TASK_PRIO,
				NULL);
}
