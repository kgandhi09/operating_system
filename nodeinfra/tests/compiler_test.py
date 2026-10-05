"""End-to-end compiler tests, including compilation of generated C99 and C++20."""
import pathlib
import re
import subprocess
import sys
import tempfile

jkbuf, cc, cxx, includes = sys.argv[1:]

def run(args, ok=True):
    result = subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=20)
    if ok and result.returncode:
        raise AssertionError(f"{args}: {result.stdout}\n{result.stderr}")
    if not ok and result.returncode == 0:
        raise AssertionError(f"unexpected success: {args}")
    return result

with tempfile.TemporaryDirectory(prefix="jkbuf-test-") as tmp:
    root = pathlib.Path(tmp)
    def compile_schema(text, filename="input", ok=True):
        src, out = root / (filename + ".ridl"), root / (filename + ".h")
        src.write_text(text)
        result = run([jkbuf, "-o", str(out), str(src)], ok)
        return out, result

    (root / "common.ridl").write_text("namespace common { message Header { u64 stamp; } }")
    source = '''#include "common.ridl"
#include "common.ridl"
// comments are accepted
namespace demo {
  /* forward reference and nested composites */
  message Sample {
    common.Header header;
    Later later;
    array<vector<bool, 4>, 2> flags;
    string<32> label;
    vector<f64, 8> values;
  }
  message Later { i32 value; }
  message Empty {}
}'''
    out, _ = compile_schema(source)
    repeat, _ = compile_schema(source, "repeat")
    assert out.read_bytes() == repeat.read_bytes(), "generation is not deterministic"
    sibling, _ = compile_schema('#include "common.ridl"\nmessage Sibling { common.Header header; string<32> label; }', "sibling")
    body = '''#include "input.h"
#include "sibling.h"
int main(void) {
  jkbuf_demo_Sample msg;
  jkbuf_demo_Sample_init(&msg);
  if (!jkbuf_demo_Sample_valid(&msg)) return 1;
  msg.f_values.length = 9;
  if (jkbuf_demo_Sample_valid(&msg)) return 2;
  msg.f_values.length = 0;
  msg.f_flags.data[0].length = 1;
  msg.f_flags.data[0].data[0] = 2;
  if (jkbuf_demo_Sample_valid(&msg)) return 3;
  msg.f_flags.data[0].data[0] = 1;
  if (!jkbuf_demo_Sample_valid(&msg)) return 4;
  return 0;
}'''
    for compiler, extension, standard in ((cc, "c", "c99"), (cxx, "cpp", "c++20")):
        file = root / ("generated_test." + extension)
        file.write_text(body + "\n")
        exe = root / ("generated_test_" + extension)
        run([compiler, "-std=" + standard, "-Wall", "-Wextra", "-Werror", "-pedantic", "-I", includes, str(file), "-o", str(exe)])
        run([str(exe)])

    cases = [
        ('message A { u32 x }', "expected ';'"),
        ('message A { Missing x; }', 'unknown type'),
        ('message A { u32 x; u32 x; }', 'duplicate field'),
        ('message A {} message A {}', 'duplicate message'),
        ('message A { A self; }', 'recursive'),
        ('message A { B b; } message B { A a; }', 'recursive'),
        ('message A { string<0> x; }', 'bound'),
        ('message A { vector<u8, 999999999999999999999999> x; }', 'bound'),
        ('message A { array<u64, 65536> x; }', 'payload'),
        ('message A { array<u8, 65535> x; u64 y; }', 'payload'),
        ('message A { @ x; }', 'invalid token'),
        ('#include "missing.ridl"', 'include not found'),
        ('#include "bad\tpath"', 'invalid token'),
        ('#include "unterminated', 'invalid token'),
        ('/* unterminated', 'invalid token'),
        ('namespace a { message b_c {} } namespace a_b { message c {} }', 'collision'),
        ('namespace a {', 'expected'),
        ('message A {} message A_init {}', 'collision'),
        ('message boolean_valid {}', 'collision'),
        ('message A { ' + 'array<' * 70 + 'u8' + ',1>' * 70 + ' x; }', 'nesting'),
    ]
    for i, (text, error) in enumerate(cases):
        _, result = compile_schema(text, f"invalid_{i}", False)
        assert error in result.stderr, result.stderr
        assert re.search(r":\d+:\d+:", result.stderr), result.stderr

    (root / "cycle.ridl").write_text('#include "cycle.ridl"')
    result = run([jkbuf, "-o", str(root / "cycle.h"), str(root / "cycle.ridl")], False)
    assert "cyclic include" in result.stderr
    before = (root / "input.ridl").read_bytes()
    run([jkbuf, "-o", str(root / "input.ridl"), str(root / "input.ridl")], False)
    assert before == (root / "input.ridl").read_bytes()

    # Same logical schema gives the same identifier after whitespace changes.
    first, _ = compile_schema('message M { u32 n; }', 'first')
    second, _ = compile_schema('/* comment */ message M {\n u32 n;\n}', 'second')
    changed, _ = compile_schema('message M { u64 n; }', 'changed')
    schema = lambda p: re.search(r'jkbuf_M_SCHEMA UINT64_C\((\d+)\)', p.read_text()).group(1)
    assert schema(first) == schema(second) != schema(changed)
    conflict = root / 'conflict.c'
    conflict.write_text('#include "first.h"\n#include "changed.h"\n')
    result = run([cc, '-c', str(conflict), '-o', str(root / 'conflict.o')], False)
    assert 'Conflicting_RIDL_schema' in result.stderr

    # -I search and include-within-namespace have explicit scope.
    inc = root / 'includes'; inc.mkdir()
    (inc / 'inner.ridl').write_text('message Inner { u8 value; }')
    src = root / 'scoped.ridl'; src.write_text('namespace outer { #include "inner.ridl" message M { Inner x; } }')
    target = root / 'scoped.h'
    run([jkbuf, '-I', str(inc), '-o', str(target), str(src)])
    assert 'jkbuf_outer_Inner' in target.read_text()
    # .jkbuf is the user-facing extension; legacy RIDL inputs remain accepted.
    src = root / 'message.jkbuf'
    src.write_text(source)
    target, depfile = root / 'message.h', root / 'message.d'
    run([jkbuf, '--depfile', str(depfile), '-o', str(target), str(src)])
    assert target.read_bytes() == out.read_bytes()
    assert str(src) in depfile.read_text()
    assert str(root / 'common.ridl') in depfile.read_text()
    assert depfile.read_text().startswith(str(target) + ':')
    original_schema = (root / 'common.ridl').read_bytes()
    for destination in (src, root / 'common.ridl', target):
        run([jkbuf, '--depfile', str(destination), '-o', str(target), str(src)], False)
    run([jkbuf, '-o', str(root / 'common.ridl'), str(src)], False)
    assert (root / 'common.ridl').read_bytes() == original_schema
    assert target.read_bytes() == out.read_bytes()
print('PASS: parser, includes, diagnostics, bounds, cycles, schema IDs, generated C99/C++20 validation')
