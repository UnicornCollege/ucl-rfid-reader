#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

char rfid_buffer[16];
char rfid_tag_id[13];
int rfid_bytes_left = 0;

void uart_handler(bc_uart_channel_t channel, bc_uart_event_t event, void *event_param)
{
    (void) channel;
    (void) event_param;

    if (event == BC_UART_EVENT_ASYNC_READ_TIMEOUT)
    {
        bc_led_pulse(&led, 1000);
    }

    if (event == BC_UART_EVENT_ASYNC_READ_DATA)
    {
        while (true)
        {
            char character;

            if (bc_uart_async_read(BC_UART_UART1, &character, 1) == 0)
            {
                break;
            }

            if (rfid_bytes_left == 0)
            {
                if (character == '\x02')
                {
                    rfid_bytes_left = 16;
                }
                else
                {
                    continue;
                }
            }

            if ((rfid_bytes_left == 3 && character != '\r') ||
                (rfid_bytes_left == 2 && character != '\n') ||
                (rfid_bytes_left == 1 && character != '\x03'))
            {
                rfid_bytes_left = 0;
                continue;
            }

            rfid_buffer[16 - rfid_bytes_left--] = character;

            if (rfid_bytes_left == 0)
            {
                bc_led_pulse(&led, 100);
                strncpy(rfid_tag_id, &rfid_buffer[1], 12);
                rfid_tag_id[12] = '\0';
                usb_talk_publish_rfid_tag(rfid_tag_id);
            }
        }
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
       usb_talk_publish_push_button("", 0);
    }
}

void application_init(void)
{
    usb_talk_init();

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

     // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    static uint8_t write_buffer[64];
    static uint8_t read_buffer[64];
    static bc_fifo_t write_fifo;
    static bc_fifo_t read_fifo;

    bc_fifo_init(&write_fifo, write_buffer, sizeof(write_buffer));
    bc_fifo_init(&read_fifo, read_buffer, sizeof(read_buffer));

    bc_uart_init(BC_UART_UART1, BC_UART_CONFIG_9600_8N1);
    bc_uart_set_event_handler(BC_UART_UART1, uart_handler, NULL);
    bc_uart_set_async_fifo(BC_UART_UART1, &write_fifo, &read_fifo);
    bc_uart_async_read_start(BC_UART_UART1, 5000);

}