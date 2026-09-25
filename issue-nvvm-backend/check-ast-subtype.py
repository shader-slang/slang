#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Check every registered AST subtype pair against native C++ inheritance.

Run after building the matching native Linux Ninja compiler, with no concurrent build:
    python3 issue-nvvm-backend/check-ast-subtype.py --output build/ast-subtype-proof

The standalone executable links configured compiler objects without exporting a test API.
Generated registry membership supplies class names only; C++ supplies inheritance truth.
Use objects built from the current headers and configuration. Invalid tags are deliberately
not executed against assertion-disabled objects; their unchanged assertion is source-audited.
"""

import argparse
import hashlib
import json
import re
import shlex
import subprocess
from pathlib import Path


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def generate_proof(build, output):
    """Expand registry names into compiler-checked inheritance and typed construction tests."""
    registry = build / "source/slang/fiddle/slang-ast-forward-declarations.h.fiddle"
    classes = re.findall(r"^\s*class (\w+);", registry.read_text(), re.MULTILINE)
    if not classes or len(set(classes)) != len(classes):
        raise ValueError("The generated AST registry must contain unique class names")
    template = Path(__file__).with_name("ast-subtype-proof.cpp.in")
    replacements = {
        "INDEX_ASSERTIONS": "\n".join(
            f"static_assert(int({name}::kType) == {index});"
            for index, name in enumerate(classes)
        ),
        "CREATORS": "\n".join(f"    &createNode<{name}>," for name in classes),
        "INHERITANCE": "\n".join(
            "    {"
            + ",".join(f"__is_base_of({target},{source})" for target in classes)
            + "},"
            for source in classes
        ),
        "TARGET_CHECKS": "\n".join(f"    checkTarget<{name}>(nodes);" for name in classes),
    }
    source = template.read_text()
    for marker, text in replacements.items():
        pattern = rf"^[ \t]*// GENERATED:{marker}$"
        source, count = re.subn(pattern, lambda _: text, source, flags=re.MULTILINE)
        if count != 1:
            raise ValueError(f"Expected exactly one template marker: {marker}")
    (output / "proof.cpp").write_text(source)
    return len(classes)


def configured_commands(build, config, output):
    """Reuse native configured compiler flags, objects and link libraries for the test main."""
    database = json.loads((build / "compile_commands.json").read_text())
    entries = [
        entry for entry in database
        if entry["file"].endswith("/slang-ast-boilerplate.cpp")
        and f"/{config}/" in entry["output"]
    ]
    if len(entries) != 1:
        raise ValueError("Expected one configured AST boilerplate compile command")
    entry = entries[0]
    command = entry.get("arguments") or shlex.split(entry["command"])
    flags = []
    index = 0
    while index < len(command):
        option = command[index]
        if option in ("-o", "-c", "-include"):
            index += 2
            continue
        if option == "-Winvalid-pch" or re.fullmatch(r"-O\w*|-g\w*", option):
            index += 1
            continue
        flags.append(option)
        index += 1
    compile_command = flags + [
        "-O0", "-c", str(output / "proof.cpp"), "-o", str(output / "proof.o")
    ]

    # This harness supports native Linux multi-config Ninja builds. Retain the actual target's
    # objects and library ordering; omit only its shared-library RPATH for this standalone binary.
    lines = (build / f"CMakeFiles/impl-{config}.ninja").read_text().splitlines()
    targets = [
        index for index, line in enumerate(lines)
        if line.startswith(f"build {config}/lib/libslang-compiler.so.")
        and ": CXX_SHARED_LIBRARY" in line
    ]
    if len(targets) != 1:
        raise ValueError("Expected one native Ninja compiler shared-library target")
    target = targets[0]
    object_names = lines[target].split(": ", 1)[1].split(" | ")[0].split()[1:]
    if any("$" in name or not name.endswith(".o") for name in object_names):
        raise ValueError("Unsupported escaped or non-object Ninja target input")
    objects = [str(build / name) for name in object_names]
    properties = {}
    for line in lines[target + 1:]:
        if not line.startswith("  "):
            break
        key, value = line.strip().split(" = ", 1)
        properties[key] = value
    libraries = []
    for option in shlex.split(properties["LINK_LIBRARIES"]):
        if option.startswith("-Wl,-rpath,"):
            continue
        if "$" in option:
            raise ValueError(f"Unsupported Ninja library expansion: {option}")
        libraries.append(option if option.startswith("-") else str(build / option))
    link_command = [
        command[0], str(output / "proof.o"), *objects, *libraries,
        "-o", str(output / "proof")
    ]
    return compile_command, link_command, Path(entry["directory"])


def run_logged(label, command, output, directory):
    """Retain exact commands and diagnostics, including failed bounded compiler invocations."""
    write_json(output / f"{label}-command.json", command)
    with (output / f"{label}.log").open("w") as log:
        subprocess.run(
            command, cwd=directory, stdout=log, stderr=subprocess.STDOUT,
            timeout=600, check=True
        )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--build-dir", default="build", type=Path)
    parser.add_argument("--config", default="RelWithDebInfo")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "proof.cpp").exists():
        raise ValueError("Use a new output directory to preserve prior proof evidence")
    count = generate_proof(build, output)
    print(f"Generated {count} classes and {count * count} inheritance pairs", flush=True)
    compile_command, link_command, directory = configured_commands(build, args.config, output)
    run_logged("compile", compile_command, output, directory)
    run_logged("link", link_command, output, directory)
    result = subprocess.run([str(output / "proof")], capture_output=True, timeout=180)
    (output / "result.log").write_bytes(result.stdout + result.stderr)
    print((result.stdout + result.stderr).decode(errors="replace"))
    result.check_returncode()
    summary = json.loads(result.stdout.decode().strip().splitlines()[-1])
    summary["proof_source_sha256"] = hashlib.sha256((output / "proof.cpp").read_bytes()).hexdigest()
    summary["configuration"] = args.config
    summary["build_dir"] = str(build)
    write_json(output / "results.json", summary)


if __name__ == "__main__":
    main()
