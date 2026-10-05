include(BuildJkbuf)
build_jkbuf(NAME sensor_msg MSG_FILES sensor.jkbuf)

add_library(example_messages INTERFACE)
target_link_libraries(example_messages INTERFACE sensor_msg)
