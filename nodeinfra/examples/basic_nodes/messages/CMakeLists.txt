include(BuildJkbuf)
build_jkbuf(NAME temperature_msg MSG_FILES temperature.jkbuf)

add_library(messages INTERFACE)
target_link_libraries(messages INTERFACE temperature_msg)
