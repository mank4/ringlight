#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

#include "scpi/scpi.h"
#include "usbtmc_app.h"

#define PIN_INDICATOR1 8
#define PIN_INDICATOR2 9

#define PIN_LED1 0
#define PIN_LED2 1
#define PIN_LED3 2
#define PIN_LED4 3
#define PIN_LED5 4
#define PIN_LED6 5
#define PIN_LED7 6
#define PIN_LED8 7

#define BTN_DLY 275
#define BTN_P 12
#define BTN_N 14
#define BTN_EN 13
#define BTN_SEC_NUM 28
#define BTN_SEC_SIZE 29
#define BTN_SEC_ORIENT 27

bool enable = true;
uint8_t brightness = 2; //steps from 0 to 9, human perceptions seems to be logarithmic
uint8_t section_num = 1; //number of sections
uint8_t section_size = 8; //size of sections
uint8_t section_orient = 0; //orientation of sections

void led_init()
{
    gpio_set_function(PIN_LED1, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED2, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED3, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED4, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED5, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED6, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED7, GPIO_FUNC_PWM);
    gpio_set_function(PIN_LED8, GPIO_FUNC_PWM);
    
    for(uint i = 0; i < 8; i++) {
        uint slice_num = pwm_gpio_to_slice_num(i);
        pwm_set_wrap(slice_num, 65535);
        pwm_set_gpio_level(i, 0);
        pwm_set_enabled(slice_num, true);
    }
    
    return;
}

void led_set()
{   
    //e^[1-10]/22026*65535
    //2.25^[1-10]/3325.25*65535
    const uint16_t leveltopwm[10] = {44,100,224,505,1136,2557,5753,12946,29127,65535};
    
    uint16_t pwmVal = leveltopwm[brightness];
    if(!enable) {
        pwmVal = 0;
    }
    
    uint16_t pwmVals[8] = {0,0,0,0,0,0,0,0};
    
    for(uint16_t i = 8+section_orient-section_size/2; i < 16+section_orient-section_size/2; i+=(8/section_num)) {
        for(uint16_t j = 0; j < section_size; j++) {
            pwmVals[(i+j)%8] = pwmVal;
        }
    }

    for(uint16_t i = 0; i < 8; i++) {
        pwm_set_gpio_level(i, pwmVals[i]);
    }
    
    return;
}

static scpi_result_t pi_led_enable_set(scpi_t* context) {
    bool value = false;
    
    if(!SCPI_ParamBool(context, &value, true)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    } else {
        enable = value;
        led_set();
    }
    
    return SCPI_RES_OK;
}

static scpi_result_t pi_led_brightness_set(scpi_t* context) {
    uint32_t value = 255;
    SCPI_ParamUInt32(context, &value, true);
    
    if(value < 0 || value > 9) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    } else {
        brightness = value;
        led_set();
    }
    
    return SCPI_RES_OK;
}

static scpi_result_t pi_led_section_num_set(scpi_t* context) {
    uint32_t value = 255;
    SCPI_ParamUInt32(context, &value, true);
    
    if(value != 1 && value != 2 && value != 4) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    } else {
        section_num = value;
        led_set();
    }
    
    return SCPI_RES_OK;
}

static scpi_result_t pi_led_section_size_set(scpi_t* context) {
    uint32_t value = 255;
    SCPI_ParamUInt32(context, &value, true);
    
    if(value < 1 || value > 8) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    } else {
        section_size = value;
        led_set();
    }
    
    return SCPI_RES_OK;
}

static scpi_result_t pi_led_section_orient_set(scpi_t* context) {
    uint32_t value = 255;
    SCPI_ParamUInt32(context, &value, true);
    
    if(value < 0 || value > 7) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    } else {
        section_orient = value;
        led_set();
    }
    
    return SCPI_RES_OK;
}

static scpi_result_t pi_led_individual_set(scpi_t* context) {
    uint32_t value = 255;
    
    SCPI_ParamUInt32(context, &value, true);
    
    if(value < 0 || value > 65535) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
    } else {
        pwm_set_gpio_level(2, value);
    }
    
    return SCPI_RES_OK;
}

#define SCPI_ERROR_QUEUE_SIZE 17
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

//#define SCPI_INPUT_BUFFER_LENGTH 256
//static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

bool scpi_doIndicatorPulse = 0;
absolute_time_t scpi_doIndicatorPulseUntil;

//###########################################
// SCPI callbacks
//###########################################

static scpi_result_t pi_echo(scpi_t* context) {
    char retVal[500] = "hm";
    size_t text_len = 3;
    
    if(!SCPI_ParamCopyText(context, retVal, sizeof(retVal), &text_len, true)) {
        
    }
    
    SCPI_ResultCharacters(context, retVal, text_len);
    
    return SCPI_RES_OK;
}

static scpi_result_t pi_bootsel(scpi_t* context) {
    reset_usb_boot(PIN_INDICATOR1, 0);
    
    return SCPI_RES_OK;
}

//###########################################
// SCPI callback definition
//###########################################

scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    {"*CLS", SCPI_CoreCls, 0},
    {"*ESE", SCPI_CoreEse, 0},
    {"*ESE?", SCPI_CoreEseQ, 0},
    {"*ESR?", SCPI_CoreEsrQ, 0},
    {"*IDN?", SCPI_CoreIdnQ, 0},
    {"*OPC", SCPI_CoreOpc, 0},
    {"*OPC?", SCPI_CoreOpcQ, 0},
    {"*RST", SCPI_CoreRst, 0},
    {"*SRE", SCPI_CoreSre, 0},
    {"*SRE?", SCPI_CoreSreQ, 0},
    {"*STB?", SCPI_CoreStbQ, 0},
    {"*TST?", SCPI_CoreTstQ, 0},
    {"*WAI", SCPI_CoreWai, 0},
    
    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {"SYSTem:ERRor[:NEXT]?", SCPI_SystemErrorNextQ, 0},
    {"SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQ, 0},
    {"SYSTem:VERSion?", SCPI_SystemVersionQ, 0},

    //{"STATus:OPERation?", scpi_stub_callback, 0},
    //{"STATus:OPERation:EVENt?", scpi_stub_callback, 0},
    //{"STATus:OPERation:CONDition?", scpi_stub_callback, 0},
    //{"STATus:OPERation:ENABle", scpi_stub_callback, 0},
    //{"STATus:OPERation:ENABle?", scpi_stub_callback, 0},

    {"STATus:QUEStionable[:EVENt]?", SCPI_StatusQuestionableEventQ, 0},
    //{"STATus:QUEStionable:CONDition?", scpi_stub_callback, 0},
    {"STATus:QUEStionable:ENABle", SCPI_StatusQuestionableEnable, 0},
    {"STATus:QUEStionable:ENABle?", SCPI_StatusQuestionableEnableQ, 0},

    {"STATus:PRESet", SCPI_StatusPreset, 0},
    
    /* probeInterface */
    { .pattern = "ECHO", .callback = pi_echo,},
    { .pattern = "BOOTSEL", .callback = pi_bootsel,},
    
    { .pattern = "LED:INDIvidual", .callback = pi_led_individual_set,},
    { .pattern = "LED:ENable", .callback = pi_led_enable_set,},
    { .pattern = "LED:BRIGHTness", .callback = pi_led_brightness_set,},
    { .pattern = "LED:SECtion:NUMber", .callback = pi_led_section_num_set,},
    { .pattern = "LED:SECtion:SIZE", .callback = pi_led_section_size_set,},
    { .pattern = "LED:SECtion:ORIENTation", .callback = pi_led_section_orient_set,},
    
	SCPI_CMD_LIST_END
};

scpi_t scpi_context;

size_t scpi_write(scpi_t * context, const char * data, size_t len) {
    (void) context;
    usbtmc_app_response(data, len, false);
    return len;
}

scpi_result_t scpi_flush(scpi_t* context) {
    SCPI_RegSetBits(context, SCPI_REG_STB, STB_MAV);
    usbtmc_app_response(NULL, 0, true);
    return SCPI_RES_OK;
}

scpi_interface_t scpi_interface = {
    /*.error = */ NULL,
    /*.write = */ scpi_write,
    /*.control = */ NULL,
    /*.flush = */ scpi_flush,
    /*.reset = */ NULL,
};

//###########################################
// USB callbacks
//###########################################

void usbtmc_app_query_cb(char* data, size_t len)
{
    //use usb internal buffer and SCPI_Parse
    //use scpi library for ieee488.2
    SCPI_Parse(&scpi_context, data, len);
    return;
}

uint8_t usbtmc_app_get_stb_cb(void)
{
    uint8_t status = SCPI_RegGet(&scpi_context, SCPI_REG_STB);
    SCPI_RegClearBits(&scpi_context, SCPI_REG_STB, STB_SRQ);
    return status;
}

void usbtmc_app_clear_stb_cb(void)
{
    SCPI_RegClearBits(&scpi_context, SCPI_REG_STB, 0xff);
}

void usbtmc_app_clear_mav_cb(void)
{
    SCPI_RegClearBits(&scpi_context, SCPI_REG_STB, STB_MAV);
}

void usbtmc_app_set_srq_cb(void)
{
    scpi_context.registers[SCPI_REG_STB] |= STB_SRQ;
    //SCPI_RegSetBits(&scpi_context, SCPI_REG_STB, STB_SRQ);
}

void usbtmc_app_clear_srq_cb(void)
{
    scpi_context.registers[SCPI_REG_STB] &= ~STB_SRQ;
    //SCPI_RegClearBits(&scpi_context, SCPI_REG_STB, STB_SRQ);
}

//indicator light is required by IEEE 488.2
void usbtmc_app_indicator_cb(void)
{
    gpio_put(PIN_INDICATOR1, 1);
    scpi_doIndicatorPulse = true;
    scpi_doIndicatorPulseUntil = make_timeout_time_ms(750);
}

//###########################################
// main application
//###########################################

int main() {
    stdio_init_all();
    
    //led ring
    led_init();
    led_set();
    
    gpio_init(PIN_INDICATOR1);
    gpio_set_dir(PIN_INDICATOR1, GPIO_OUT);
    gpio_put(PIN_INDICATOR1, 1);
    //printf("RP2040 booting...\n");
    sleep_ms(1000);
    gpio_put(PIN_INDICATOR1, 0);
    
    gpio_init(PIN_INDICATOR2);
    gpio_set_dir(PIN_INDICATOR2, GPIO_OUT);
    gpio_put(PIN_INDICATOR2, 0);
    
    gpio_init(BTN_P);
    gpio_init(BTN_N);
    gpio_init(BTN_EN);
    gpio_init(BTN_SEC_NUM);
    gpio_init(BTN_SEC_SIZE);
    gpio_init(BTN_SEC_ORIENT);
    
    gpio_set_dir(BTN_P, GPIO_IN);
    gpio_set_pulls(BTN_P, false, false);
    gpio_set_dir(BTN_N, GPIO_IN);
    gpio_set_pulls(BTN_N, false, false);
    gpio_set_dir(BTN_EN, GPIO_IN);
    gpio_set_pulls(BTN_EN, false, false);
    gpio_set_dir(BTN_SEC_NUM, GPIO_IN);
    gpio_set_pulls(BTN_SEC_NUM, false, false);
    gpio_set_dir(BTN_SEC_SIZE, GPIO_IN);
    gpio_set_pulls(BTN_SEC_SIZE, false, false);
    gpio_set_dir(BTN_SEC_ORIENT, GPIO_IN);
    gpio_set_pulls(BTN_SEC_ORIENT, false, false);
    
    SCPI_Init(&scpi_context,
              scpi_commands,
              &scpi_interface,
              scpi_units_def,
              "mank electronics", "ringlight", "v0.1", "dev",
              NULL, 0, //scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE
    );
    
    usbtmc_app_init();
    
    while (true) {
        //this is not a good coding style but should be ok for this simple project
        if(!gpio_get(BTN_EN)) {
            gpio_put(PIN_INDICATOR2, enable);
            enable = !enable;
            led_set();
            sleep_ms(BTN_DLY);
        }
        if(enable && !gpio_get(BTN_P)) {
            if(brightness < 9) {
                brightness++;
                led_set();
            } else {
                gpio_put(PIN_INDICATOR2, 1);
            }
            sleep_ms(BTN_DLY);
            gpio_put(PIN_INDICATOR2, 0);
        }
        if(enable && !gpio_get(BTN_N)) {
            if(brightness > 0) {
                brightness--;
                led_set();
            } else {
                gpio_put(PIN_INDICATOR2, 1);
            }
            sleep_ms(BTN_DLY);
            gpio_put(PIN_INDICATOR2, 0);
        }
        if(enable && !gpio_get(BTN_SEC_NUM)) {
            //switch between single, double and full
            if(section_num == 1 && section_size == 8) {
                section_num = 1;
                section_size = 2;
            } else if(section_num == 1 && section_size < 8) {
                section_num = 2;
                section_size = 2;
            } else {
                section_num = 1;
                section_size = 8;
            }
            led_set();
            sleep_ms(BTN_DLY);
        }
        if(enable && !gpio_get(BTN_SEC_SIZE)) {
            //switch size of segments
            if(section_num == 1 && section_size < 8) {
                section_size++;
                if(section_size > 6)
                    section_size = 1;
            } else if(section_num == 2) {
                section_size++;
                if(section_size > 2)
                    section_size = 1;
            }
            led_set();
            sleep_ms(BTN_DLY);
        }
        if(enable && !gpio_get(BTN_SEC_ORIENT)) {
            section_orient++;
            if(section_orient > 7)
                section_orient = 0;
            led_set();
            sleep_ms(BTN_DLY);
        }
        
        if(scpi_doIndicatorPulse && get_absolute_time() > scpi_doIndicatorPulseUntil) {
            scpi_doIndicatorPulse = false;
            gpio_put(PIN_INDICATOR1, 0);
        }
        
        usbtmc_app_task_iter();
    }
}
