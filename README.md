
# mavlink

    This is an implementation of part of the MaVLink
    microservices, which are currently supported as follows:

* [mavlink route](http://mavlink.io/zh/guide/routing.html)
* [Heartbeat/Connection Protocol](http://mavlink.io/zh/services/heartbeat.html)
* [Mission Protocol](http://mavlink.io/zh/services/mission.html)
* [Parameter Protocol](http://mavlink.io/zh/services/parameter.html)
* [Command Protocol](http://mavlink.io/zh/services/command.html)

# How to use

    A brief routine based on QT is provided:
    example/mavlink_microservices_example.cpp

## Build routines

```bash
$ cmake -B .build

$ cmake --build .build
```

    You should choose "Kate - MinGW Makefiles" as the build tool 
    and add QT to system PATH(or set CMAKE_PREFIX_PATH in CMakeLists).

    And add the following libraries to the project:
    [Parameters](https://github.com/AiYangSky/Parameters)
    It implements a simple parameter management function.

## note

    The standard mavlink interface functions are modified for easier implementation:
```C
    static inline uint16_t mavlink_msg_name_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_heartbeat_t* heartbeat)
```
    to
```C
    static inline uint16_t mavlink_msg_heartbeat_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const void* heartbeat)
```
    This change was made by modifying the mavlink generator script.
    