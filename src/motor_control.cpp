/* 
 * The following code is responsible for the control of the window opener.
 * The window opener is a servo motor that is controlled by a PWM signal.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "room_data.h"
#include "weather_data.h"

static const char *TAG = "MOTOR_CONTROL";

//The data below is from the datasheet of the servo motor
#define SERVO_MIN_PULSEWIDTH_US 500  //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2500  //Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE        -90   //Minimum angle
#define SERVO_MAX_DEGREE        90    //Maximum angle

#define SERVO_PULSE_GPIO             13       //GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  //1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    //20000 ticks, 20ms

extern Room_data Internal_room_data;
extern Weather_data Weather;

//This function takes an angle as an input and creates a PWM signal out of it as an output
static inline uint32_t create_pwm_signal(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void motor_control_task(void *params)
{
    //Setting up the timer using the MCPWM peripheral
    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {};
    timer_config.group_id = 0;
    timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    timer_config.resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ;
    timer_config.period_ticks = SERVO_TIMEBASE_PERIOD;
    timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    //Setting up the operator using the MCPWM peripheral
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {};
    operator_config.group_id = 0;
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    //Establish a link between the timer's timing signals and the operator
    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    //Setting up a comparator
    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {};
    comparator_config.flags.update_cmp_on_tez = true;
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    //Setting up a signal generator
    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {};
    generator_config.gen_gpio_num = SERVO_PULSE_GPIO;
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    //Set the initial position of the motor to the one read from the NVS
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(Internal_room_data.get_window_deg())));
    ESP_LOGI(TAG, "Window angle set to: %f", Internal_room_data.get_window_deg());

    //Sets the generation action on timer and compare events
    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    //Enabling and starting the timer
    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    while (1) {

        //Manual mode
        if (!Internal_room_data.get_is_auto())
        {
            //Open the window as much as the user likes
            ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(Internal_room_data.get_window_deg())));
        }
        //Automatic mode
        else
        {
            //If the room temperature is higher than the desired and outside temperatures, open the window to cool down the room
            if (Internal_room_data.get_internal_temperature() > Internal_room_data.get_desired_temperature())
            {
                if(Internal_room_data.get_internal_temperature() > Weather.get_temp())
                {   
                    //Open the window
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(90)));
                }
                else
                {   //Close the window
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(-90)));
                }
            }
            //If the room temperature is lower than the desired and outside temperature, open the window to heat up the room
            else if (Internal_room_data.get_internal_temperature() < Internal_room_data.get_desired_temperature())
            {
                if(Internal_room_data.get_internal_temperature() < Weather.get_temp())
                {   
                    //Open the window
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(90)));
                }
                else
                {   //Close the window
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(-90)));
                }
            }
            else
            {   //Close the window
                ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, create_pwm_signal(-90)));
            }

        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}