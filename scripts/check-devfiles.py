#!/usr/bin/env python3
"""Check that an OS image's development files work on the image itself.

    check-devfiles.py <root>

<root> is the assembled root filesystem (build-rootfs.sh runs this on it).
What a program built on jk_os goes through, checked against the files that
are really in <root> (paths in the files are the device's: /usr/...):

  CMake packages (usr/lib/cmake, usr/share/cmake, ...):
    - every imported target's files (IMPORTED_LOCATION): libraries, and
      tools such as moc, qmake or qmlcachegen
    - its include directories and the absolute libraries it links
    - the Qt and KDE packages it depends on (Qt's tool and target
      dependencies, find_dependency(Qt6.../KF6...)): Qt6Qml without
      Qt6QmlTools fails find_package(Qt6 COMPONENTS Qml)
  pkg-config files (usr/lib/pkgconfig, usr/share/pkgconfig):
    - the -I and -L directories, the -l libraries and the absolute paths
      in Cflags and Libs
    - the modules in Requires and Requires.private
  Development links (usr/lib/lib*.so): none dangling.

A path left from the build machine (/home/.../build/...) doesn't exist in
<root>, so it shows up as missing. Prints each problem and exits 1 if any.
"""
import glob
import os
import re
import shlex
import sys

root = os.path.abspath(sys.argv[1]) if len(sys.argv) == 2 else sys.exit(__doc__)
problems = []


def rel(path):
    return os.path.relpath(path, root)


def on_root(path):
    """The image's file for a path as the device sees it (/usr/...)."""
    return os.path.join(root, path.lstrip("/"))


def problem(where, what):
    problems.append(f"{rel(where)}: {what}")


# ------------------------------------------------------------------ CMake

cmake_dirs = [d for d in glob.glob(on_root("/usr/lib/cmake")) + glob.glob(on_root("/usr/lib/*/cmake"))
              + glob.glob(on_root("/usr/share/cmake*")) if os.path.isdir(d)]
packages = set()
for d in cmake_dirs:
    for name in os.listdir(d):
        packages.add(name)

PREFIX_STEP = re.compile(r'get_filename_component\(_IMPORT_PREFIX "\$\{_IMPORT_PREFIX\}" PATH\)')
LOCATION = re.compile(r'IMPORTED_LOCATION(?:_\w+)? "([^"]*)"')
INCLUDES = re.compile(r'INTERFACE_INCLUDE_DIRECTORIES "([^"]*)"')
LINKS = re.compile(r'INTERFACE_LINK_LIBRARIES "([^"]*)"')
QT_DEPS = re.compile(r'set\(__qt_\w+?_(?:tool|target)_deps "([^"]*)"\)')
FIND_DEP = re.compile(r'find_dependency\(\s*((?:Qt6|KF6)\w*)')


def import_prefix(directory):
    """_IMPORT_PREFIX of the export files in <directory>: the directory, up
    as many levels as its *Targets.cmake says (on the device)."""
    for f in glob.glob(os.path.join(directory, "*.cmake")):
        with open(f, errors="replace") as fh:
            steps = len(PREFIX_STEP.findall(fh.read()))
        if steps:
            p = "/" + rel(directory)
            for _ in range(steps):
                p = os.path.dirname(p)
            return p
    return None


def entries(value):
    """A CMake list's plain entries: generator expressions ($<...>) and
    other variables are left out, as they depend on the consumer."""
    for e in value.split(";"):
        if e and "$<" not in e and ">" not in e and "${" not in e.replace("${_IMPORT_PREFIX}", ""):
            yield e


for d in cmake_dirs:
    for directory, _, files in os.walk(d):
        cmake_files = [f for f in files if f.endswith(".cmake")]
        if not cmake_files:
            continue
        prefix = import_prefix(directory)
        for name in cmake_files:
            path = os.path.join(directory, name)
            with open(path, errors="replace") as fh:
                text = fh.read()
            # Branches CMake never takes (KF6's "if(0) find_dependency(...)").
            text = re.sub(r"^\s*if\(0\)\s*$.*?^\s*endif\(\)", "", text, flags=re.M | re.S)
            if prefix is not None and "_IMPORT_PREFIX" in text:
                sub = lambda e: e.replace("${_IMPORT_PREFIX}", prefix)
                for value in LOCATION.findall(text):
                    for e in entries(value):
                        if not os.path.lexists(on_root(sub(e))):
                            problem(path, f"imported file {sub(e)} is missing")
                for value in INCLUDES.findall(text):
                    for e in entries(value):
                        if not os.path.isdir(on_root(sub(e))):
                            problem(path, f"include directory {sub(e)} is missing")
                for value in LINKS.findall(text):
                    for e in entries(value):
                        if sub(e).startswith("/") and not os.path.lexists(on_root(sub(e))):
                            problem(path, f"linked library {sub(e)} is missing")
            for value in QT_DEPS.findall(text):
                # "Qt6Core\\;6.11.2;Qt6DBus\\;6.11.2": name and version per entry
                for dep in re.split(r"(?<!\\);", value):
                    pkg = dep.split("\\")[0]
                    if pkg and pkg not in packages:
                        problem(path, f"depends on CMake package {pkg}, which isn't there")
            for pkg in FIND_DEP.findall(text):
                if pkg not in packages and pkg != "Qt6":
                    problem(path, f"find_dependency({pkg}): no such CMake package")

# ------------------------------------------------------------- pkg-config

pc_dirs = [d for d in glob.glob(on_root("/usr/lib/pkgconfig")) + glob.glob(on_root("/usr/lib/*/pkgconfig"))
           + glob.glob(on_root("/usr/share/pkgconfig")) if os.path.isdir(d)]
pc_files = {}
for d in pc_dirs:
    for f in glob.glob(os.path.join(d, "*.pc")):
        pc_files.setdefault(os.path.basename(f)[:-3], f)
lib_dirs = [on_root("/usr/lib")] + glob.glob(on_root("/usr/lib/*-linux-gnu"))
VAR = re.compile(r"\$\{(\w+)\}")
OPERATORS = {"=", "<", ">", "<=", ">=", "!="}


def expand(value, variables, depth=0):
    if depth > 20:
        return value
    out = VAR.sub(lambda m: variables.get(m.group(1), ""), value)
    return expand(out, variables, depth + 1) if VAR.search(out) else out


def required(value):
    """Module names in a Requires line (versions and operators left out)."""
    tokens = value.replace(",", " ").split()
    names, skip = [], False
    for t in tokens:
        if skip:
            skip = False
        elif t in OPERATORS:
            skip = True
        else:
            names.append(t)
    return names


for name, path in sorted(pc_files.items()):
    variables, fields = {"pc_sysrootdir": "/"}, {}
    with open(path, errors="replace") as fh:
        for line in fh:
            line = line.strip()
            m = re.match(r"^([\w.]+)\s*=\s*(.*)$", line)
            if m:
                variables[m.group(1)] = expand(m.group(2), variables)
                continue
            m = re.match(r"^([\w.]+):\s*(.*)$", line)
            if m:
                fields[m.group(1)] = expand(m.group(2), variables)
    for key in ("Cflags", "Cflags.private", "Libs", "Libs.private"):
        try:
            tokens = shlex.split(fields.get(key, ""))
        except ValueError:
            tokens = fields.get(key, "").split()
        search = list(lib_dirs)
        for t in tokens:
            if t.startswith("-I") and len(t) > 2:
                if not os.path.isdir(on_root(t[2:])):
                    problem(path, f"{key}: include directory {t[2:]} is missing")
            elif t.startswith("-L") and len(t) > 2:
                if not os.path.isdir(on_root(t[2:])):
                    problem(path, f"{key}: library directory {t[2:]} is missing")
                search.append(on_root(t[2:]))
            elif t.startswith("-Wl,-rpath-link,"):
                d = t[len("-Wl,-rpath-link,"):]
                if not os.path.isdir(on_root(d)):
                    problem(path, f"{key}: library directory {d} is missing")
            elif t.startswith("-l") and len(t) > 2 and key == "Libs":
                lib = t[2:]
                # -l:<file> names the file itself (-l:liblua5.4.a).
                names = [lib[1:]] if lib.startswith(":") else [f"lib{lib}.so", f"lib{lib}.a"]
                if not any(os.path.exists(os.path.join(s, n)) for s in search for n in names):
                    problem(path, f"{key}: library -l{lib} is missing")
            elif t.startswith("/") and (".so" in t or t.endswith(".a")):
                if not os.path.exists(on_root(t)):
                    problem(path, f"{key}: {t} is missing")
    for key in ("Requires", "Requires.private"):
        for mod in required(fields.get(key, "")):
            if mod not in pc_files:
                problem(path, f"{key}: no pkg-config module {mod}")

# ------------------------------------------------------- development links

for d in lib_dirs:
    for f in glob.glob(os.path.join(d, "lib*.so")):
        if os.path.islink(f) and not os.path.exists(f):
            problem(f, f"dangling development link (-> {os.readlink(f)})")

checked = f"{len(cmake_dirs)} CMake directories, {len(pc_files)} pkg-config modules"
if problems:
    print(f"check-devfiles: {len(problems)} problem(s) in {checked}:", file=sys.stderr)
    for p in problems:
        print("  " + p, file=sys.stderr)
    sys.exit(1)
print(f"check-devfiles: development files complete ({checked})")
