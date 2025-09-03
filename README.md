# Door Closer ESP32

An ESP32-based automatic door closer system that uses a linear actuator to automatically close doors when they're left open. The system includes a WiFi web server that can receive door state events from external applications (like iOS apps) and automatically control the door closing mechanism.

## Features

- **WiFi-enabled**: Connects to your local WiFi network for remote control
- **Automatic door closing**: Uses a linear actuator to physically close doors
- **Web server interface**: Simple HTTP endpoints for door state management
- **Safety features**: Built-in initialization sequence and timing controls
- **Serial monitoring**: Comprehensive logging for debugging and monitoring

## Hardware Requirements

### Core Components
- **ESP32 Development Board** (any ESP32 variant)
- **L298N H-Bridge Motor Driver** for controlling the linear actuator
- **12V Linear Actuator** (200mm stroke recommended)
- **12V Power Supply** for the actuator
- **Breadboard and jumper wires** for connections

### Pin Connections
| ESP32 Pin | L298N Connection | Purpose |
|-----------|------------------|---------|
| Pin 25    | IN1              | Forward/Extend control |
| Pin 26    | IN2              | Reverse/Retract control |
| 3.3V      | VCC              | Logic power (if needed) |
| GND       | GND              | Common ground |

## Circuit Diagram

```
ESP32                    L298N                    Linear Actuator
+----+                   +----+                   +----+
|    |                   |    |                   |    |
| 25 |------------------>|IN1 |------------------>|    |
|    |                   |    |                   |    |
| 26 |------------------>|IN2 |------------------>|    |
|    |                   |    |                   |    |
|GND |------------------>|GND |------------------>|    |
+----+                   +----+                   +----+
                              |
                              v
                          12V Power Supply
```

## Software Setup

### Prerequisites
- Arduino IDE with ESP32 board support
- Required libraries: `WiFi.h` (included with ESP32 core)

### Configuration
1. Open `closer/closer.cpp` in Arduino IDE
2. Update WiFi credentials:
   ```cpp
   const char *ssid = "Your WiFi name";
   const char *password = "Your WiFi password";
   ```
3. Verify pin assignments match your hardware setup
4. Upload the code to your ESP32

## Usage

### Initialization
When the ESP32 starts up, it automatically:
1. Connects to WiFi
2. Starts the web server
3. Runs a 23-second initialization sequence to retract the actuator to a known position
4. Becomes ready to receive door events

### Web Server Endpoints

The ESP32 creates a simple web server with these endpoints:

- **GET /** - Status page showing the door controller is ready
- **GET /open** - Triggers the door closing sequence
- **GET /closed** - Logs that the door is closed (informational only)

### Door Closing Sequence

When `/open` is triggered, the system automatically:
1. **Extends** the actuator for 20 seconds to push the door closed
2. **Retracts** the actuator for 20 seconds to return to neutral position
3. **Stops** and waits for the next door event

Total cycle time: **40 seconds**

### Example Usage

#### Using curl
```bash
# Trigger door closing
curl http://[ESP32_IP_ADDRESS]/open

# Check status
curl http://[ESP32_IP_ADDRESS]/
```

#### Using a web browser
Navigate to `http://[ESP32_IP_ADDRESS]/open` to trigger door closing

#### iOS App Integration
The system is designed to work with iOS apps that can send HTTP requests when door sensors detect an open door.

## Serial Monitor Output

The ESP32 provides detailed logging via Serial Monitor (115200 baud):

```
L298N H-Bridge initialized
Connecting to Your WiFi name
.....
WiFi connected.
IP address: 192.168.1.100
Door controller server started!
Initializing: RETRACTING actuator to known position (23 seconds)...
Initialization complete - Actuator in retracted position
Ready to receive door events!
New Client.
open
Door is OPEN! Activating linear actuator...
Starting linear actuator: EXTENDING to close door
Linear actuator: EXTENDING back to neutral (20 seconds)
Door closing sequence complete - actuator stopped
Total cycle time: 40 seconds
```

## Safety Features

- **Initialization sequence**: Ensures the actuator starts from a known position
- **Prevent multiple activations**: Won't start a new closing sequence while one is in progress
- **Timing controls**: Prevents the actuator from running indefinitely
- **Hardware protection**: Uses H-bridge for proper motor control

## Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Verify SSID and password are correct
   - Check WiFi signal strength
   - Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

2. **Actuator Not Moving**
   - Check power supply voltage (should be 12V)
   - Verify pin connections
   - Check Serial Monitor for error messages

3. **Door Closing Too Fast/Slow**
   - Adjust timing values in the code (currently 20 seconds per phase)
   - Consider actuator stroke length and door weight

### Debug Mode
Enable detailed logging by monitoring the Serial output at 115200 baud.

## Customization

### Timing Adjustments
Modify these values in the code to match your specific setup:
```cpp
if (elapsed >= 23000)  // 23 seconds initialization
if (elapsed >= 20000)  // 20 seconds for full stroke
```

### Pin Changes
Update these constants if using different ESP32 pins:
```cpp
const int IN1_PIN = 25;  // Change as needed
const int IN2_PIN = 26;  // Change as needed
```

## License

This project is open source. Feel free to modify and distribute as needed.

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the Serial Monitor output
3. Verify hardware connections
4. Check WiFi network compatibility

---

**Note**: This system is designed for residential use with standard interior doors. For commercial or heavy-duty applications, consider using industrial-grade components and additional safety mechanisms.

