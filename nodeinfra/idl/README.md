# Define and build jkbuf messages

Use CMake 3.20+ and a C++20 compiler. Message definitions use `.jkbuf` files.

Create `messages/test.jkbuf`:

```cpp
namespace robot {
    message Test {
        u64 sequence;
        string<32> label;
    }
}
```

Create `messages/CMakeLists.txt`:

```cmake
include(BuildJkbuf)
build_jkbuf(NAME test_msg MSG_FILES test.jkbuf)

add_library(messages INTERFACE)
target_link_libraries(messages INTERFACE test_msg)
```

In your application's top-level `CMakeLists.txt`, replace the checkout path:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_robot LANGUAGES C CXX)

add_subdirectory(/path/to/jk_os/nodeinfra nodeinfra)
add_subdirectory(messages)

add_executable(my_node main.cpp)
target_link_libraries(my_node PRIVATE messages)
```

Use the generated header in `main.cpp`:

```cpp
#include "test.h"

int main() {
    jkbuf_robot_Test msg;
    jkbuf_robot_Test_init(&msg);
    msg.f_sequence = 1;
    return jkbuf_robot_Test_valid(&msg) ? 0 : 1;
}
```

Build from your application's root:

```sh
cmake -S . -B build
cmake --build build --target my_node -j4
```

This generates `build/messages/test_msg/test.h` before compiling your app.
The message library is header-only; include paths propagate automatically.
Changes to message files and their included schemas regenerate the headers.
Add more `build_jkbuf(...)` calls and link their targets into `messages` as needed.

To generate a header manually with the built compiler:

```sh
build/nodeinfra/idl/jkbuf -o test.h messages/test.jkbuf
```

See the [RIDL specification](grammar/ridl_spec.md) for types and schema syntax.
