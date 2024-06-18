# Project to demonstrate a ROP attack on FreeRTOS and the raspberry pi pico w

With thanks to racka98 https://github.com/racka98/PicoW-FreeRTOS-Template and Tony Smith https://blog.smittytone.net/2021/10/31/how-to-send-data-to-a-raspberry-pi-pico-via-usb/ for setting up the raspberry pi pico program with the serial over usb so the attack can be performed  
By overriding the return address on the stack the program is able to recall a function that should only be called once leading to functionality the developer didn't expect  
The block_data positioned in perform_task is overrwritten when the raspberry pi pico recieves a block from the python program with the new block data and if the block is too big, it can overrwrite more of the stack than it's supposed to  
This would also work without the FreeRTOS kernel since it doesn't provide many memory protection mechanisms for its programs  
This branch also uses a canary to protect the stack but overwrites it when sending the evil payload  

## Project Layout
sender -> directory holding the python program to send messages to the pico
example -> directory holding the program to be uploaded to the pico
freertos -> directory holding the FreeRTOS kernel used to build the program with

## Building
Inside this repo call 'git submodule --init' to initialise the FreeRTOS submodule in this repo
First clone the pico-sdk from https://github.com/raspberrypi/pico-sdk and call 'git update submodule --init' inside it  
Then set PICO_SDK_PATH to be this directory  
For the raspberry pi pico program, go into the example directory and type 'cmake -B build' then 'cmake --build build'  
Then connect the raspberry pi pico w to your device while holding down the BOOTSEL button  
Copy 'TEST_PICO_FREERTOS.uf2' inside the build directory (should have been created from the cmake instructions above) to the raspberry pi  
Disconnect the raspberry pi pico from your device (should now start running the program)  
For the python program, go into the sender directory  
Type 'python send_messages.py'  

## Running
While the program is running, the first 3 messages sent will not be malicious (but if they are longer than 32 bytes bad things may happen)  
Then the payload is sent and the raspberry pi pico will call an undesired function that it shouldn't call  
If payloads are sent too quickly, the pico may not read them individually and errors may occurre  

## Changing the program
To find the address of the function to call, on a unix like system type 'readelf -a build/TEST_PICO_FREERTOS.elf' and look for where the function is in memory  
In the python program, change the last four bytes of data in the 'send_deadly_block' function to be this address but in reverse byte order (since the program executes in little endian format)  
Also, type 'arm-none-eabi-objdump -drS build/TEST_PICO_FREERTOS.elf' and look for 'perform_task' and check what value it subtracts from sp in the function. Make sure the for loop in 'send_deadly_block' repeats this number of times  

## Disclaimer
**This code is provided for educational purposes only and under no circumstances will I be held responsible if it's misused**
