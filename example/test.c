#include <FreeRTOS.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <task.h>

#define MESSAGE_PAUSE 900
#define BLOCK_SIZE 32

void task(void* params);
void start_task();

typedef struct
{
    uint16_t checksum;
    uint16_t len;
    char data[];
} USBBlock;

int counter = 0;

void count_func();

// main program
int main()
{
    stdio_init_all();
    printf("Starting Program!\n");
    count_func();
    start_task();
    return 0;
}

// create task to use with FreeRTOS
void start_task()
{
    // func, name, stack size, args, priority, handle
    xTaskCreate(task, "Test Task", 1024, NULL, 1, NULL);
    vTaskStartScheduler();
    while(true){} // halt
}

// get packet of data
void get_block(USBBlock* block)
{
    block->checksum = 0;
    block->len = 0;
    char* buffer = (char*)block;
    uint16_t index = 0;
    int c = 0;
    while(true)
    {
        c = getchar_timeout_us(100);
        if(c != PICO_ERROR_TIMEOUT)
        {
            if(c == 0x55) // wait until have start of packet
            {
                break;
            }
        }
        else
        {
            return;
        }
    }
    while(true)
    {
        c = getchar_timeout_us(100);
        if(c != PICO_ERROR_TIMEOUT)
        {
            buffer[index++] = c & 0xff;
        }
        else
        {
            break;
        }
    }
    uint16_t checksum = block->checksum;
    for(uint16_t i = 0; i < block->len - 2; i++)
    {
        checksum += buffer[i + 2];
    }
    // check have all the data
    if(checksum == 0)
    {
        printf("ACK\r\n");
    }
    else
    {
        printf("Err\r\n");
    }
}

// should only be called once
void count_func()
{
    if(counter == 0)
    {
        printf("Everything is ok\r\n");
        counter++;
    }
    else
    {
        // something bad has happened if we get here
        do
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // have flash but slower
            printf("All chaos has broken loose! Mwah ha ha ha...\r\n");
            vTaskDelay(MESSAGE_PAUSE * 5);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(MESSAGE_PAUSE * 5);
        } while(true);    
    }
}

// task to be performed (definitely no bugs here)
void __attribute__ ((noinline)) perform_task()
{
    char block_data[BLOCK_SIZE]; // reserve space for block
    // perform flash and during off check for messages
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    get_block((USBBlock*)block_data);
    vTaskDelay(MESSAGE_PAUSE);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(MESSAGE_PAUSE);
}

// perform the task repeatedly
void task(void* params)
{
    bool connected = true;
    if(cyw43_arch_init())
    {
        printf("Error initing wifi\r\n");
        connected = false;
    }
    while(connected)
    {
        perform_task();
    }
}
