"""Build real C/C++ consumers through source and relocated installed packages."""
import pathlib
import subprocess
import sys
import tempfile
import time

cmake, source, package_build, cc, cxx, generator = sys.argv[1:]


def run(*args, ok=True):
    result = subprocess.run(args, text=True, capture_output=True, timeout=90)
    if (result.returncode == 0) != ok:
        raise AssertionError(f"{args}:\n{result.stdout}\n{result.stderr}")
    return result.stdout + result.stderr


with tempfile.TemporaryDirectory(prefix="jkbuf cmake ") as tmp:
    root = pathlib.Path(tmp)
    prefix = root / "install"
    run(cmake, "--install", package_build, "--prefix", str(prefix))
    relocated = root / "relocated install"
    prefix.rename(relocated)
    project = root / "consumer"
    messages = project / "messages"
    for directory in ("left", "right", "shared schemas"):
        (messages / directory).mkdir(parents=True)
    leaf = messages / "shared schemas" / "leaf.jkbuf"
    leaf.write_text("namespace common { message Leaf { u32 value; } }\n")
    (messages / "shared schemas" / "common.jkbuf").write_text(
        '#include "leaf.jkbuf"\nnamespace common { message Header { Leaf leaf; } }\n'
    )
    left = messages / "left" / "message.jkbuf"
    original = '#include "common.jkbuf"\nnamespace left { message Sample { common.Header header; } }\n'
    left.write_text(original)
    (messages / "right" / "message.jkbuf").write_text(
        "namespace right { message Sample { bool active; } }\n"
    )
    (messages / "legacy.ridl").write_text("message Legacy { u8 value; }\n")
    (messages / "CMakeLists.txt").write_text('''include(BuildJkbuf)
build_jkbuf(NAME samples MSG_FILES left/message.jkbuf right/message.jkbuf
  INCLUDE_DIRS "shared schemas")
build_jkbuf(NAME legacy MSG_FILES legacy.ridl)
add_library(messages INTERFACE)
target_link_libraries(messages INTERFACE samples legacy)
''')
    apps = project / "apps"
    apps.mkdir()
    body = '''#include "left/message.h"
#include "right/message.h"
#include "legacy.h"
#include <stdio.h>
int main(void) {
  jkbuf_left_Sample msg;
  jkbuf_left_Sample_init(&msg);
  if (!jkbuf_left_Sample_valid(&msg)) return 1;
  printf("%zu\\n", sizeof(msg.f_header.f_leaf.f_value));
  return 0;
}
'''
    (apps / "main.c").write_text(body)
    (apps / "main.cpp").write_text(body)
    (apps / "CMakeLists.txt").write_text('''add_executable(consumer_c main.c)
add_executable(consumer_cpp main.cpp)
target_link_libraries(consumer_c PRIVATE messages)
target_link_libraries(consumer_cpp PRIVATE messages)
''')

    def configure(build, extra=(), ok=True):
        return run(cmake, "-S", str(project), "-B", str(build), "-G", generator,
                   f"-DCMAKE_C_COMPILER={cc}", f"-DCMAKE_CXX_COMPILER={cxx}",
                   f"-DCMAKE_PREFIX_PATH={relocated}", *extra, ok=ok)

    def build_consumers(build, ok=True):
        return run(cmake, "--build", str(build), "--target", "consumer_c", "consumer_cpp",
                   "--parallel", "4", ok=ok)

    def check_consumers(build, expected):
        for executable in ("consumer_c", "consumer_cpp"):
            assert run(str(build / "apps" / executable)).strip() == expected

    def write_project(setup):
        (project / "CMakeLists.txt").write_text(f'''cmake_minimum_required(VERSION 3.20)
project(jkbuf_consumer LANGUAGES C CXX)
{setup}
add_subdirectory(messages)
add_subdirectory(apps)
''')

    write_project('find_package(jk_nodeinfra CONFIG REQUIRED)')
    build = root / "installed build"
    configure(build)
    build_consumers(build)
    check_consumers(build, "4")
    generated = build / "messages" / "samples" / "left" / "message.h"
    timestamp = generated.stat().st_mtime_ns
    build_consumers(build)
    assert generated.stat().st_mtime_ns == timestamp, "no-op build regenerated headers"

    # Included schemas are discovered by the compiler, including transitive -I inputs.
    time.sleep(1.1)  # Make also needs to observe this on coarse timestamp filesystems.
    leaf.write_text("namespace common { message Leaf { u64 value; } }\n")
    build_consumers(build)
    assert generated.stat().st_mtime_ns != timestamp, "included schema was not tracked"
    check_consumers(build, "8")
    timestamp = generated.stat().st_mtime_ns
    build_consumers(build)
    assert generated.stat().st_mtime_ns == timestamp

    generated.unlink()
    build_consumers(build)
    check_consumers(build, "8")
    time.sleep(1.1)
    left.write_text("message Invalid { Missing field; }\n")
    assert "unknown type" in build_consumers(build, ok=False)
    left.write_text(original)
    build_consumers(build)

    # Never run an imported target-architecture compiler during a cross build.
    cross = root / "cross build"
    assert "JKBUF_HOST_EXECUTABLE" in configure(cross, ("-DCMAKE_SYSTEM_NAME=Linux",), ok=False)
    configure(cross, ("-DCMAKE_SYSTEM_NAME=Linux",
                      f"-DJKBUF_HOST_EXECUTABLE={relocated / 'bin' / 'jkbuf'}"))
    build_consumers(cross)
    check_consumers(cross, "8")

    # The same user CMake files also work with a source checkout added as a sibling.
    write_project(f'''set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(JK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("{source}" nodeinfra)''')
    source_build = root / "source build"
    configure(source_build)
    build_consumers(source_build)
    check_consumers(source_build, "8")

print(f"PASS: {generator}: .jkbuf libraries, C/C++, includes, incremental builds, relocation, cross-host selection, source checkout")
