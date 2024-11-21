# Telemetry-Firmware-FE11
Viewing live graphs from sensor data. Dashboard sends data via WiFi to TelemTransceiver which is wired to a laptop for viewing.

CAN--> STM32 --UART--> ESP32 --WIFI--> ESP32 --Serial--> Laptop

Plotting uses pyserial and matplotlib python packages. Press any key to pause and resume plotting.

Plotting command usage: ```python plot_serial_data.py <port_name>```

Web server and camera stream uses OpenCV, mjpeg-streamer, JustPy, matplotlib, and numpy python packages.