import serial
import time
import random

module = serial.Serial("COM4", 9600, timeout=0.2)

speed = 60
oil_temp = 140
coolant_temp = 130
fuel_pressure = 100
distance = 300

module.flush()

message = 5
data = 0

while True:
    speed += random.randint(-10, 10)
    speed %= 255

    oil_temp = coolant_temp + random.randint(-2, 2)
    oil_temp %= 255

    coolant_temp += random.randint(-1, 1)
    coolant_temp %= 255

    fuel_pressure += random.randint(-5, 5)
    fuel_pressure %= 255

    distance += 1;

    if module.in_waiting > 0:
        message = module.readline()
        message=int(message)
        module.reset_input_buffer()
        print(message)

    if message == 13:
        data = speed
    elif message == 5:
        data = coolant_temp
    elif message == 92:
        data = oil_temp
    elif message == 10:
        data = fuel_pressure
    elif message == 49:
        data = distance
            
    time.sleep(1);
    #module.flush();
    string = str(data) + "\0";

    module.write(bytes(string, 'utf-8'))
    print ("data: ", data)


module.flush()
module.close()
    
