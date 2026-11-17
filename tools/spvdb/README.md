# spvdb — SPIR-V Shader Debugger

**spvdb** is a source-level debugger and interpreter for SPIR-V shader modules.
It executes shaders on the CPU, surfacing breakpoints, single-stepping,
local-variable inspection, and call-stack reconstruction from
`NonSemantic.Shader.DebugInfo.100` debug information.

Supported execution models: **GLCompute**, **Vertex**, **Fragment**.

---

## Features

- Source-level breakpoints by file and line number, or by SPIR-V result-id
- Single-step by source line (step into / step over / step out) or by instruction
- Inspect local variables, output variables, and descriptor buffer contents
- Reconstruct inlined call stacks via `DebugInlinedAt`
- Bind descriptor-set buffers as JSON arrays, images from PNG/BMP/JPEG/TGA/HDR
- Set built-in values (`GlobalInvocationId`, `FragCoord`, …) and specialization constants
- Interactive REPL (via [replxx](https://github.com/AmokHuginnsson/replxx)) and a
  scriptable batch front-end for use with test frameworks

---

## Building

### Prerequisites

| Tool | Minimum version |
|------|----------------|
| CMake | 3.20 |
| C++ compiler with C++20 support | GCC 11 / Clang 14 / MSVC 19.29 |
| `spirv-as` (for assembling `.spvasm` test fixtures) | any recent SPIRV-Tools |

### Cloning

```bash
git clone --recurse-submodules https://github.com/zangold-nv/spvdb.git
cd spvdb
```

If you already cloned without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

### Standalone build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

This produces:
- `build/cli/spvdb` — interactive debugger
- `build/tools/spvdb-test/spvdb-test` — scriptable test front-end
- `build/tests/` — Catch2 unit and integration tests

### Running the tests

```bash
cd build
ctest --output-on-failure
```

### Embedding in another project

Add spvdb as a subdirectory; the `libspvdb` static library target is the only
thing exposed to the parent:

```cmake
add_subdirectory(third_party/spvdb)
target_link_libraries(my_target PRIVATE spvdb)
```

When included this way the CLI, test, and tool targets are suppressed and
`SPVDB_SKIP_STB_IMAGE_IMPL` is set automatically to avoid duplicate symbol
errors with the parent's own stb_image build.

---

## Interactive CLI

```
spvdb [<path.spv> [<entry_point>]]
```

### File and session setup

| Command | Description |
|---------|-------------|
| `file <path.spv>` | Load a SPIR-V module |
| `info entries` | List available entry points |
| `entry <name>` | Select an entry point |
| `set input <set> <binding> <json>` | Bind a descriptor-set buffer (JSON array) |
| `set input loc <location> <json>` | Bind a vertex/fragment input by `Location` decoration |
| `set builtin <name> <value>` | Set a built-in value (e.g. `GlobalInvocationId`, `FragCoord`) |
| `set specconst <id> <value>` | Override a specialization constant by `SpecId` |

### Execution

| Command | Aliases | Description |
|---------|---------|-------------|
| `run` | `r` | Start execution from the entry point |
| `continue` | `c` | Continue until the next breakpoint or exit |
| `step` | `s` | Step one source line (steps *into* calls) |
| `next` | `n` | Step one source line (steps *over* calls) |
| `finish` | | Run until the current function returns |
| `stepi` | `si` | Step one SPIR-V instruction |

### Breakpoints

| Command | Description |
|---------|-------------|
| `break <file>:<line>` | Set a source breakpoint |
| `break %<id>` | Set a result-id breakpoint |
| `info breakpoints` | List all breakpoints |
| `delete <bp-id>` | Remove a breakpoint |

File paths are matched as suffixes, so `break main.slang:42` matches
`/any/path/to/main.slang:42`.

### Inspection

| Command | Aliases | Description |
|---------|---------|-------------|
| `info locals` | | Print local variables at the current location |
| `info outputs` | | Print output and storage-buffer variables |
| `print <name>` | `p` | Evaluate and print a single variable |
| `backtrace` | `bt`, `where` | Print the current call stack |
| `list [line]` | `l` | Show source lines around the current location |
| `disassemble` | `dis` | Show SPIR-V instructions near the current PC |

### Example session

```
$ spvdb shader.spv
(spvdb) entry main
(spvdb) set input 0 0 [1, 2, 3, 4]
(spvdb) break shader.slang:27
Breakpoint 1 set at shader.slang:27
(spvdb) run
stopped: Breakpoint 1 at shader.slang:27
(spvdb) info locals
local: a = 7
local: b = 5
(spvdb) next
stopped: Step at shader.slang:28
(spvdb) continue
stopped: EntryFinished
(spvdb) info outputs
output: result = [12]
(spvdb) quit
```

---

## C++ API

Add `lib/api/spvdb.h` (and optionally `lib/api/spvdb_session.h` for the
session type) to your include path and link against `libspvdb`.

### Quick example

```cpp
#include "api/spvdb.h"

// Load a SPIR-V module from a file.
auto mod = spvdb::load_module_from_file("shader.spv");
if (!mod) { /* handle error */ }

// Create a debuggable session for the "main" entry point.
auto sess = spvdb::create_session(*mod, "main", {});
if (!sess) { /* handle error */ }

// Bind a storage-buffer descriptor: set=0, binding=0, data=[7, 5, 0].
spvdb::set_descriptor_json(*sess, 0, 0, "[7, 5, 0]");

// Set a breakpoint and run.
spvdb::set_breakpoint(*sess, "shader.slang", 27);
auto reason = spvdb::run(*sess);          // StopReason::Breakpoint

// Inspect locals.
for (auto& [name, val] : spvdb::local_variables(*sess))
    std::cout << name << " = " << spvdb::value_to_string(val) << "\n";

// Continue to completion.
reason = spvdb::run(*sess);               // StopReason::EntryFinished

// Read back the output buffer.
auto result = spvdb::read_descriptor(*sess, 0, 2);
```

### Key types

| Type | Description |
|------|-------------|
| `spvdb::Module` | Loaded SPIR-V module (immutable, shareable) |
| `spvdb::Session` | Mutable execution state for one invocation |
| `spvdb::SessionOptions` | Memory initialisation (`init_pattern`, `init_value`) |
| `spvdb::Value` | Tagged union: Bool, Int32/64, UInt32/64, Float32/64, Composite, Pointer |
| `spvdb::StopReason` | `Breakpoint`, `Step`, `EntryReached`, `EntryFinished`, `Panic` |
| `spvdb::SourceLocation` | `file`, `line`, `column` |
| `spvdb::Result<T>` | Success/error wrapper; check with `if (r)` or `r.error()` |

### API reference (selected functions)

```cpp
// Module loading
Result<Module>   load_module(std::span<const uint32_t> words);
Result<Module>   load_module_from_file(const std::string& path);
std::vector<EntryPoint> list_entry_points(const Module&);

// Session creation & setup
Result<Session>  create_session(const Module&, const std::string& entry,
                                const SessionOptions& = {});
void set_spec_constant(Session&, uint32_t spec_id, uint32_t value);
void set_descriptor(Session&, uint32_t set, uint32_t binding,
                    std::vector<uint32_t> data);
void set_descriptor_json(Session&, uint32_t set, uint32_t binding,
                         const std::string& json);
void set_image(Session&, uint32_t set, uint32_t binding,
               const std::string& image_path);
void set_input_location(Session&, uint32_t location, Value value);
void set_builtin(Session&, uint32_t builtin_id, Value value);

// Breakpoints
uint32_t set_breakpoint(Session&, const std::string& file, uint32_t line);
uint32_t set_breakpoint_at_id(Session&, uint32_t result_id);
void     remove_breakpoint(Session&, uint32_t bp_id);

// Execution
StopReason run(Session&);
StopReason step(Session&);
StopReason step_over(Session&);
StopReason step_out(Session&);
StopReason step_instruction(Session&);
std::string panic_message(const Session&);

// Inspection
SourceLocation current_location(const Session&);
std::vector<std::pair<std::string,Value>> local_variables(const Session&);
std::vector<std::pair<std::string,Value>> output_variables(const Session&);
Result<Value> evaluate_variable(const Session&, const std::string& name);
Result<Value> read_descriptor(const Session&, uint32_t set, uint32_t binding);
std::vector<StackFrame> backtrace(const Session&);
```

---

## Project layout

```
spvdb/
├── lib/
│   ├── api/            Public C++ API (spvdb.h, spvdb_session.h)
│   ├── core/           Value and Result types
│   ├── parser/         SPIR-V binary parser
│   ├── module/         Module and instruction representation
│   └── interpreter/    Execution engine + GLSL.std.450 + image sampler
├── cli/                Interactive REPL (replxx-based)
├── tests/
│   ├── unit/           Catch2 unit tests
│   ├── spirv/          Hand-written SPIR-V assembly test fixtures
│   └── integration/    Integration tests
└── third_party/
    ├── SPIRV-Headers/  (submodule) SPIR-V opcode headers
    ├── Catch2/         (submodule) unit test framework
    ├── nlohmann_json/  (submodule) JSON parsing for descriptors
    ├── replxx/         (submodule) readline-style REPL
    └── stb/            stb_image for image descriptor loading
```

---

## Code style

All C++ sources are formatted with **clang-format** using the `.clang-format`
file at the repository root.  The easiest way to reformat or verify is via the
CMake targets added by the standalone build:

```bash
cmake -B build
cmake --build build --target format        # reformat in-place
cmake --build build --target format-check  # exit non-zero if any file differs
```

If you prefer to invoke clang-format directly, run it from the repository root
so it picks up `.clang-format` automatically:

```bash
# Reformat all sources (excludes third_party and build)
find cli lib tests \( -name '*.cpp' -o -name '*.h' \) \
    | xargs clang-format -i
```

The `format-check` target (or the equivalent `--dry-run --Werror` invocation)
is suitable for use in CI.

---

## Static analysis

The standalone build adds a `tidy` target that runs **clang-tidy** across all
first-party sources (`cli/`, `lib/`, `tests/`).

```bash
cmake -B build
cmake --build build --target tidy
```

`clang-tidy` uses `compile_commands.json` (generated automatically in the build
directory by CMake) to resolve include paths and compiler flags, so the build
directory must exist before running the target — a bare `cmake -B build` without
a full build is sufficient.

If you want to run clang-tidy directly on a single file:

```bash
clang-tidy -p build lib/interpreter/interpreter.cpp
```
