#include <application.h>
#include <usb_talk.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// UART instance
#define UC_SFRFIDUSBREADER_TX_FIFO_BUFFER_SIZE 16
#define UC_SFRFIDUSBREADER_RX_FIFO_BUFFER_SIZE 16

typedef struct uc_sfrfidusbreader_t uc_sfrfidusbreader_t;

struct uc_sfrfidusbreader_t
{
    bc_scheduler_task_id_t _task_id;
    bc_uart_channel_t _uart_channel;
    bc_fifo_t _tx_fifo;
    bc_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[UC_SFRFIDUSBREADER_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[UC_SFRFIDUSBREADER_RX_FIFO_BUFFER_SIZE];
    size_t _length;
    char *_buffer;
};

#define UC_SFRFIDUSBREADER_DELAY 1000

void application_init(void)
{
    usb_talk_init();

     bc_module_core_pll_enable();

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize UART 
    uc_sfrfidusbreader_t *uc_sfrfidusbreader;
    uc_sfrfidusbreader = malloc(sizeof(uc_sfrfidusbreader_t));

    //memset(uc_sfrfidusbreader, 0, sizeof(*uc_sfrfidusbreader));

    uc_sfrfidusbreader->_uart_channel = BC_UART_UART0;
    bc_fifo_init(&uc_sfrfidusbreader->_tx_fifo, uc_sfrfidusbreader->_tx_fifo_buffer, sizeof(uc_sfrfidusbreader->_tx_fifo_buffer));
    bc_fifo_init(&uc_sfrfidusbreader->_rx_fifo, uc_sfrfidusbreader->_rx_fifo_buffer, sizeof(uc_sfrfidusbreader->_rx_fifo_buffer));
    uc_sfrfidusbreader->_length = UC_SFRFIDUSBREADER_TX_FIFO_BUFFER_SIZE;

    bc_uart_param_t uart_param =
    {
        .baudrate = BC_UART_BAUDRATE_9600_BD
    };

    bc_uart_init(uc_sfrfidusbreader->_uart_channel, &uart_param, &uc_sfrfidusbreader->_tx_fifo, &uc_sfrfidusbreader->_rx_fifo);

    uc_sfrfidusbreader->_task_id = bc_scheduler_register(uc_sfrfidusbreader_task, uc_sfrfidusbreader, UC_SFRFIDUSBREADER_DELAY);

    usb_talk_start();
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
    (void) param;
    size_t _length = 16;
    int8_t _buffer[16] = {0};

    bc_uart_read(self->_uart_channel, _buffer, _length, 0);

    bc_scheduler_plan_current_relative(UC_SFRFIDUSBREADER_DELAY);   
}
