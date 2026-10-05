# RIDL specification for jkbuf

Version: **0.2 — local shared-memory milestone** (2026-10-05).

RIDL describes bounded message types, stored in `.jkbuf` files (legacy `.ridl`
files are also accepted). `jkbuf` is the C++20 compiler for this
language. It implements lexing, parsing, semantic validation, and generation
of C99-compatible native-layout structs with C++ message traits. The local
C++ node runtime uses those types directly in shared memory. No ROS code or
schema tooling is used.

The syntax is defined in [ridl.ebnf](ridl.ebnf). Networking, portable wire
serialization, RPC definitions, and schema evolution are future work.

## File structure

A file contains include, namespace, and message declarations in any order.
Empty files, namespaces, and messages are accepted. There is no trailing
semicolon after a namespace or message body.

```ridl
#include "common/header.jkbuf"
namespace robot.motion {
    message MotorState {
        common.Header header;
        u32 rpm;
        f32 current;
        string<64> name;
    }
}
```

Namespaces can nest and reopen. Namespaces and includes may appear within
namespaces; messages contain fields, not nested declarations. Each field is
`type name;`. Field IDs, default values, enums, unions, services, and optional
fields are not part of this version.

## Includes and name resolution

`#include` is one token followed by a quoted path. Paths are resolved relative
to the including file first, then through `-I` directories in command-line
order. Canonical filesystem paths identify files. Repeated includes of the
same file in the same namespace are deduplicated; cyclic includes fail.
An included file's declarations are collected in the including namespace.
The same file may therefore be instantiated in distinct namespace contexts.

Forward references are supported. Type lookup tries the current namespace,
then each enclosing namespace, then the root. Qualified names follow the
same outward search. Unresolved references, duplicate fully qualified
messages, duplicate fields, and recursive by-value type dependencies fail.
Includes and diagnostics retain their original file names and locations.

## Types and bounds

| Syntax | Meaning and generated representation |
| --- | --- |
| `bool` | `uint8_t`; validators accept only 0 or 1 |
| `u8`, `u16`, `u32`, `u64` | Corresponding unsigned fixed-width integer |
| `i8`, `i16`, `i32`, `i64` | Corresponding signed fixed-width integer |
| `f32`, `f64` | Native `float` and `double` on the supported Linux ABI |
| `Name` or `namespace.Name` | Embedded, by-value message |
| `array<T, N>` | Struct with inline `T data[N]` |
| `vector<T, N>` | Struct with `uint32_t length` and inline `T data[N]` |
| `string<N>` | Struct with `uint32_t length` and inline `uint8_t data[N]` |

Types can nest, for example `array<vector<u8, 32>, 4>`. Every bound is a
positive decimal integer in **1..65536**. Zero, overflow, signs, hexadecimal
notation, and bound expressions fail. Leading zeros are accepted.

The compiler checks the complete native layout, including alignment and
padding, against a **65536-byte** limit. This also rejects huge nested
containers even when each individual bound is legal. Primitive alignment
is its width on the supported x86_64/aarch64 ABI; generated compile-time
size checks reject an incompatible layout.

Strings count bytes and have no implicit NUL terminator or UTF-8 validation.
Validators check bounds, boolean values, and nested messages. For vectors,
only the active `length` elements are validated. Fixed arrays validate all
elements. All-zero initialization is valid for every generated message.
Empty messages contain one reserved byte so they are valid C99 structs.

## Lexical rules

Identifiers are case-sensitive ASCII `[A-Za-z][A-Za-z0-9_]*`; qualified names
use dots. Keywords are `namespace`, `message`, `array`, `vector`, `string`,
and the primitive type names. `#include` is the include introducer; bare
`include` is an identifier.

Integer tokens preserve their decimal spelling; the parser validates and
converts bounds. Quoted include paths contain only printable ASCII bytes
other than a quote. There are no escape sequences; backslash is an ordinary
character. Empty include paths and unterminated or control-character-containing
string literals fail.

Spaces, tabs, carriage returns, and newlines are trivia. Both `//` line
comments and non-nesting `/* ... */` block comments are accepted. Unterminated
block comments fail. Comments may separate tokens, but cannot split tokens.
Unrecognized characters fail with file/line/column diagnostics.

## Compiler interface

```sh
jkbuf [-I directory] [--depfile output.d] -o output.h input.jkbuf
```

An output header contains all transitively included definitions, ordered so
by-value dependencies are defined first. Output is deterministic for the same
logical declarations. Compilation validates input before opening output, and
refuses to overwrite any input file. The optional `--depfile` writes a Make-style
dependency file listing the root schema and all transitively included files.
The header and dependency file must be distinct. CMake's `BuildJkbuf` module
uses this information to regenerate message headers when schemas change;
see [Message libraries with CMake](../../README.md#message-libraries-with-cmake).

`robot.motion.MotorState` becomes `jkbuf_robot_motion_MotorState`. Field
`rpm` becomes `f_rpm`. Prefixing fields avoids C/C++ keyword conflicts.
Generated-name collisions, including namespace flattening and generated
helper/function names, are rejected. The `jkbuf_detail_` prefix is reserved
for compiler-generated container types.

Each message supplies:

- A pointer-free native-layout struct.
- `TYPE_init(TYPE *)` for zero initialization.
- `TYPE_valid(const TYPE *)` for validation.
- `TYPE_SCHEMA`, a deterministic 64-bit FNV-1a fingerprint of the versioned
  logical definition, including ordered fields and referenced schemas.
- `jk::MessageTraits<TYPE>` in C++ for typed node endpoints.

Repeated included types across generated headers are guarded; conflicting
schemas for the same generated type produce a compile error. C++ consumers
need `jk/message.hpp` from the runtime's include directory. C consumers need
only standard C99 headers. The current publisher/subscriber API is C++.

Schema fingerprints identify compatibility; they are not cryptographic
identities. The local runtime also compares native size and alignment.
Renaming/reordering/changing fields or referenced message definitions changes
the fingerprint. There is no automatic compatibility conversion.

Limits: 1 MiB per source file, 8 MiB total loaded source, 1024 messages,
64 levels of syntactic nesting/include nesting/type dependency depth, and
512 bytes of collected namespace scope. Host-side parsing/code generation
uses dynamic allocation; generated message storage and validators do not.

## Current scope

Implemented: grammar, lexer, AST, recursive-descent parser, include handling,
name resolution, semantic/bounds checks, deterministic C-compatible generation,
C++ message traits, and local shared-memory publisher/subscriber examples.
Tests compile and run generated headers as C99 and C++20, check invalid schemas,
and exercise the node runtime in separate processes.

Deferred: portable wire encoding/decoding, TCP transport, schema evolution,
Python bindings, reflection, service definitions, and advanced language
features. Native structs must not be sent as a network protocol by dumping
their raw bytes.
