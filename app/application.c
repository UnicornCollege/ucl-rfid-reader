#include <application.h>
#include <usb_talk.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// UART instance
#define UC_SFRFIDUSBREADER_TAG_ID_SIZE 12
#define UC_SFRFIDUSBREADER_RX_BUFFER_SIZE 16

typedef struct uc_sfrfidusbreader_t uc_sfrfidusbreader_t;

struct uc_sfrfidusbreader_t
{
    bc_scheduler_task_id_t _task_id;
    bc_uart_channel_t _uart_channel;
    size_t _buffer_length;
    size_t _tag_id_length;
    uint8_t _tag_id[UC_SFRFIDUSBREADER_TAG_ID_SIZE];
};

#define UC_SFRFIDUSBREADER_DELAY 1000

void application_init(void)
{
    usb_talk_init();

    //bc_module_core_pll_enable();

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize UART 
    uc_sfrfidusbreader_t *uc_sfrfidusbreader;
    uc_sfrfidusbreader = malloc(sizeof(uc_sfrfidusbreader_t));

    uc_sfrfidusbreader->_uart_channel = BC_UART_UART1;
    uc_sfrfidusbreader->_tag_id[UC_SFRFIDUSBREADER_TAG_ID_SIZE] = 0;
    uc_sfrfidusbreader->_buffer_length = UC_SFRFIDUSBREADER_RX_BUFFER_SIZE;
    uc_sfrfidusbreader->_tag_id_length = UC_SFRFIDUSBREADER_TAG_ID_SIZE;

    bc_uart_init(uc_sfrfidusbreader->_uart_channel, BC_UART_CONFIG_9600_8N1);

    uc_sfrfidusbreader->_task_id = bc_scheduler_register(uc_sfrfidusbreader_task, uc_sfrfidusbreader, UC_SFRFIDUSBREADER_DELAY);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
    }
}

void uc_sfrfidusbreader_task(void *param)
{
    uc_sfrfidusbreader_t *self = param;
    uint8_t _buffer[16] = {0};
    uint8_t _usb[12] = {0};
    char _tag_id[12];

    memset(_usb, 0, 12);

    bc_uart_read(self->_uart_channel, _buffer, self->_buffer_length, BC_TICK_INFINITY);

    for (int i = 0; i < 12; i++) {
        _usb[i] = _buffer[i+1];
    }

    memcpy(self->_tag_id, _usb, self->_tag_id_length);
    //usb_talk_publish_rfid_tag("", _usb);

    bc_scheduler_plan_current_relative(UC_SFRFIDUSBREADER_DELAY);   
}
