# PICOW_A3 DATA SENDER

The PICOWA3 rendering resource page requires relevant data, such as CPU maximum frequency, current frequency, temperature, etc. 
For more information that needs to be transmitted, please refer to:[resource.h](https://github.com/AyaSanae/PICOW_A3_DATA_SENDER/blob/Linux/resource.h)

The SENDER will send data every two seconds.
Data conversion is achieved by transforming the resc_serialize structure into a byte array.

When compiling, please link the Bluetooth library, for example:
            gcc -o data_serialize_send data_serialize_send.c -lbluetooth


