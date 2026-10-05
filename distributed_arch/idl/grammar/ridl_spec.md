# RIDL Specification

Version: 0.1 (draft)

RIDL (Robotics Interface Definition Language) is a schema language for
distributed robotics systems. Its design goals are bounded memory usage,
portable serialization, embedded and Linux compatibility, and static code
generation without runtime reflection.

This document describes the syntax in [ridl.ebnf](ridl.ebnf) and the lexical
rules implemented by the [lexer](../lexer/lexer.cpp). The grammar is the
syntax baseline; implementation gaps are listed in section 9. Parsing,
semantic validation, serialization, and code generation are not implemented.

## 1. File Structure

A `.ridl` file contains zero or more include, namespace, or message
declarations. Declarations may appear in any order syntactically. An empty
file is valid syntax; declaration order and name resolution semantics remain
to be defined.

```ridl
namespace robot.motion {
    message MotorState {
        u32 rpm;
        f32 current;
        f32 voltage;
    }
}
```

Messages may also appear directly at file scope. There are no `package`,
`import`, or `enum` declarations in the current language.

## 2. Namespaces

A namespace has a simple or dot-qualified name and a brace-delimited body:

```ridl
namespace robot.localization {
    message Pose2D {
        f32 x;
        f32 y;
        f32 theta;
    }
}
```

Namespace bodies may contain includes, messages, and nested namespaces.
Empty namespace bodies are allowed. There is no trailing semicolon after
the closing brace. Rules for reopening namespaces, resolving nested names,
and mapping namespaces to generated code remain to be defined.

## 3. Includes

The grammar defines an include declaration as `#include` followed by a
quoted path, with no trailing semicolon:

```ridl
#include "common/header.ridl"
```

Includes are allowed at file scope and inside namespaces. Their intended
purpose is to reference definitions from other files. Search paths,
duplicate includes, include cycles, and the namespace context of included
declarations remain to be defined.

**Implementation gap:** the lexer does not yet recognize `#include` as a
token, even though `KwInclude` exists in the token type enum. This example
is grammar-defined syntax that still needs lexer support.

## 4. Messages and Fields

A message has a simple identifier as its name and contains zero or more
fields. Each field consists of a type expression, a simple identifier, and
a semicolon:

```ridl
message MotorState {
    u32 rpm;
    string<64> name;
}
```

Empty messages are allowed by the grammar. Messages cannot contain nested
declarations. There is no trailing semicolon after a message's closing brace.

Fields have no numeric IDs, default values, or `=` syntax. For example,
`u32 rpm = 1;` is not valid in this version. Duplicate-name validation and
other semantic checks remain to be defined and implemented.

## 5. Types

A field type is a primitive type, a named type, a fixed array, a bounded
vector, or a bounded string.

### Primitive Types

| Type | Description             |
| ---- | ----------------------- |
| bool | boolean                 |
| u8   | unsigned 8-bit integer  |
| u16  | unsigned 16-bit integer |
| u32  | unsigned 32-bit integer |
| u64  | unsigned 64-bit integer |
| i8   | signed 8-bit integer    |
| i16  | signed 16-bit integer   |
| i32  | signed 32-bit integer   |
| i64  | signed 64-bit integer   |
| f32  | 32-bit floating point   |
| f64  | 64-bit floating point   |

These describe the intended value types; their wire encoding is not defined.

### Named Types

A type reference may be a simple or dot-qualified identifier:

```ridl
message Pose2D {
    f32 x;
    f32 y;
}

message RobotState {
    Pose2D pose;
    common.Header header;
}
```

The grammar accepts these references. Checking that they resolve to declared
types requires a future semantic validation stage; `common.Header` above
would need a corresponding declaration.

### Composite Types

The following are field declarations, to be placed inside a message:

```ridl
array<f32, 3> accel;
vector<f32, 128> ranges;
string<64> frame_id;
array<vector<u8, 32>, 4> packets;
```

- `array<T, N>` describes a fixed array of `N` elements.
- `vector<T, N>` describes a vector with a maximum capacity of `N` elements.
- `string<N>` describes a string with a maximum length of `N`.
- Array and vector element types may be any type expression, including
  another composite type or a named type.

Every composite type requires an explicit decimal integer bound. Bare
`string`, unbounded vectors, and expressions such as `string<32 + 32>` are
not valid types. The grammar accepts zero and leading zeros; whether zero
bounds are semantically valid, maximum supported bounds, and overflow checks
remain to be defined. String encoding and whether length counts bytes or
characters are also undecided.

## 6. Identifiers and Keywords

Identifiers are case-sensitive and follow `[A-Za-z][A-Za-z0-9_]*`. They must
start with an ASCII letter; underscores and digits are allowed only after
the first character. A qualified identifier is one or more identifiers
separated by dots, such as `robot.motion.Pose2D`.

The lexer recognizes these reserved words:

```text
namespace message array vector string
bool u8 u16 u32 u64 i8 i16 i32 i64 f32 f64
```

These words cannot be used as identifiers or components of qualified names.
The grammar's identifier rule describes the spelling; the lexer's keyword
classification distinguishes reserved words from identifiers.

`package`, `import`, `enum`, and bare `include` are currently ordinary
identifiers, not declaration keywords. The grammar's include introducer is
the complete spelling `#include`.

## 7. Literals

Integer literals consist of one or more decimal digits: `[0-9]+`. They are
used for composite-type bounds. Signs, hexadecimal notation, floating-point
literals, and arithmetic expressions are not part of this syntax. The lexer
preserves the digits as text; it does not convert or range-check the value.

String literals are enclosed in double quotes and are used for include
paths. The grammar permits printable characters other than a double quote
or newline between the quotes, including an empty string. There are no
escape sequences: a backslash is an ordinary character and cannot escape
a quote. Whether an include path is usable is a separate semantic check.

The lexer returns string contents without the surrounding quotes. A string
that reaches a newline (`\n`) or end of input without a closing quote produces
an `Unknown` token. The lexer does not yet enforce the grammar's restriction
to printable characters; other control characters can currently occur in
a string token.

## 8. Whitespace, Punctuation, and Comments

Spaces, tabs, carriage returns, and newlines are whitespace. Whitespace may
separate tokens and has no declaration-level meaning; the lexer emits
`Whitespace` tokens that the parser will need to skip. Whitespace cannot
split an identifier or keyword into parts.

The implemented punctuation tokens are `{`, `}`, `<`, `>`, `;`, `,`, and `.`.
Unrecognized characters produce `Unknown` tokens. Tokens carry a file name
and start/end line and column positions for diagnostics.

Comments are not part of the current grammar. Neither `//` nor `/* ... */`
is recognized as a comment by the lexer. The `SingleLineComment` and
`MultiLineComment` enum entries are placeholders, not implemented features.
The `(* ... *)` comments in the EBNF file document the grammar itself and
are not RIDL source syntax.

## 9. Implementation Status

| Component                                                                           | Status                                                                                |
| ----------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| Keywords, identifiers, integers, strings, whitespace, punctuation, source locations | Implemented in the lexer, with the string restriction gap noted above                 |
| `#include` token recognition                                                        | Defined in the grammar; missing from the lexer                                        |
| AST                                                                                 | Basic schema, namespace, include, and message nodes; field and type nodes are missing |
| Parser                                                                              | Not implemented                                                                       |
| Name resolution and semantic validation                                             | Not implemented                                                                       |
| Wire format, serialization, and code generation                                     | Not implemented                                                                       |

Tokenizing a source file does not yet establish that it is a valid schema.

## 10. Memory, Wire Format, and Compatibility Goals

The intended generated runtime should support static allocation and avoid
heap allocation, exceptions, RTTI, and runtime reflection. Mandatory bounds
on arrays, vectors, and strings provide syntax toward that goal. Full memory
guarantees also require semantic rules for named types and recursive type
references; these are not yet defined. These runtime goals do not prohibit
dynamic allocation in the host-side compiler.

A compact, endian-safe binary format with predictable decoding is a design
goal. No wire-format specification or implementation is currently present
in this repository.

Schema evolution remains a design goal, not a supported guarantee. The
current syntax has no field IDs or reserved-ID declarations. Compatibility
rules for adding, removing, renaming, reordering, or changing fields and
container bounds must be designed alongside the wire format. No such change
is currently guaranteed to preserve compatibility.

## 11. Future Work

The intended first code-generation target is portable C99, with optional
C++ wrappers and Python bindings later. These targets are not implemented.

Possible language extensions, outside the current grammar, include:

- Comments and enums.
- Field IDs and reserved IDs, if required by the compatibility design.
- RPC/service definitions.
- Unions/variants and optional fields.
- Schema reflection, zero-copy transport metadata, and QoS annotations.
