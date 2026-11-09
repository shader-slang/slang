# Debugging

You can debug `[host]` execution with `slangi` in a terminal or VS Code. Both support
breakpoints, stepping, and inspection of host values and GPU resources while stopped.

## CLI debugging

`slangi -debug` provides a GDB-style command interface. Source programs compile with
debug information automatically; `-api` selects the GPU backend.

```sh
slangi -debug -api d3d12 main.slang
# HGVM debugger. Type help for commands.
```

### Breakpoints and stepping

You can break at a source line or function, step into calls with `step`, step over them
with `next`, and return to the caller with `finish`. `stepi` and `nexti` step VM instructions.

For example, this `main.slang` fills a buffer on the GPU. A breakpoint on line 11 stops
host execution before the result is printed.

```slang
import slang.runtime;

[host]
void main()
{
    let values = RWStructuredBuffer<int>.alloc(4);
    Grid(4).dispatch((uint i) =>
    {
        values[i] = int(i) * 2;
    });
    print(values[3]); // 6
}
```

```gdb
break main.slang:11   # Breakpoint 1 (1 locations).
run                  # Breakpoint 1, #0 main at .../main.slang:11:5
```

### Value and buffer inspection

`print` inspects variables, fields, indexed elements, and pointer dereferences.
Structured buffers support both individual element reads and a preview of their contents.
At the breakpoint above, the buffer contains four values:

```gdb
print values[3]       # $1 = 6
print values         # $2 = RWStructuredBuffer<int> (count = 4, stride = 4, bytes = 16, ...)
                     #      {[0] = 0, [1] = 2, [2] = 4, [3] = 6}
whatis values        # type = RWStructuredBuffer<int>
```

`backtrace` shows the call stack, including inline frames. `frame N`, `up`, and `down`
select the frame used by `info locals`, `info args`, and `print`.

```gdb
backtrace            # #0 main at .../main.slang:11:5
continue             # Prints 6; Program exited normally.
quit
```

### Precompiled programs

Precompiled HGBC supports the same debugger when compiled with `-g2`.
Source display and source breakpoints also depend on the original source files being available.

```sh
slangc main.slang -target hgbc -g2 -o main.hgbc
slangi -debug -api d3d12 main.hgbc
# HGVM debugger. Type help for commands.
```

## VS Code / DAP setup

VS Code debugging is available in the `h-slang` branch of
[slang-vscode-extension](https://github.com/saipraveenb25/h-slang-vscode-extension/tree/h-slang).
Its `slang-hgvm` adapter connects to `slangi -dap`; the ordinary Marketplace extension may
lack this adapter. The extension's
[development setup](https://github.com/saipraveenb25/h-slang-vscode-extension/blob/h-slang/CONTRIBUTING.md)
covers dependencies and the **Run Native Extension** development launch.

The workspace's `.vscode/settings.json` selects matching `slangd` and `slangi` executables
from a [native build](installation.md#build-slang). For a Windows build:

```json
{
  "slang.enableExperimentalFeatures": true,
  "slang.slangdLocation": "C:/path/to/slang/build/windows-vs2022-dev/RelWithDebInfo/bin/slangd.exe",
  "slang.slangiLocation": "C:/path/to/slang/build/windows-vs2022-dev/RelWithDebInfo/bin/slangi.exe"
}
```

`.vscode/launch.json` selects the host entry point, backend, and module search paths.
This configuration starts `main.slang` and pauses at entry:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "slang-hgvm",
      "request": "launch",
      "name": "Debug h-slang",
      "program": "${workspaceFolder}/main.slang",
      "cwd": "${workspaceFolder}",
      "entryPoint": "main",
      "includePaths": ["${workspaceFolder}"],
      "api": "d3d12",
      "stopOnEntry": true
    }
  ]
}
```

`includePaths` can include your application's package directories. When omitted,
it defaults to `slang.additionalSearchPaths`.

## Debugger features

VS Code provides source and function breakpoints, pause/continue, and step into/over/out.
The Call Stack, Variables, Watch, and hover views inspect the selected host frame,
including inline frames.

<!-- Replace this placeholder with a VS Code screenshot of main.slang stopped at print. -->
<figure class="tooling-screenshot-placeholder">
  <strong>VS Code: breakpoints and stepping</strong>
  <figcaption>Screenshot to be added: source, call stack, locals, and a watch for values[3].</figcaption>
</figure>

| Feature            | Available inspection                                                               |
| ------------------ | ---------------------------------------------------------------------------------- |
| Watches and hover  | Variables, fields, literal indices, and pointer dereferences                       |
| Structured buffers | Individual elements and paged expansion of larger buffers                          |
| Nested data        | Struct fields, arrays, matrix rows, and inline vectors                             |
| Resource metadata  | Buffer count/stride/size, texture dimensions/format/mips, and window/surface state |
| Texture images     | **Open Mip as EXR** on a texture mip in the Variables view                         |

Buffer inspection reads back the requested elements and converts their device layout for
display. For the program above, these Watch expressions show the computed values:

```slang
values[0] // 0
values[1] // 2
values[2] // 4
values[3] // 6
```

Textures expose their mip levels in Variables. **Open Mip as EXR** captures the selected
mip and opens it with the configured EXR viewer. The snapshot remains available after
continuing or ending the session.

<!-- Replace this placeholder with a VS Code screenshot of buffer expansion and an EXR mip. -->
<figure class="tooling-screenshot-placeholder">
  <strong>VS Code: resource inspection</strong>
  <figcaption>Screenshot to be added: expanded buffer elements and a texture mip opened as EXR.</figcaption>
</figure>

## Rough edges

**Host debugging.** Stepping controls the host program, not GPU shader invocations. Debugging
waits after each GPU operation by default. `-gpu-execution buffered` in the CLI, or
`"gpuExecution": "buffered"` in VS Code, retains normal batching. Resource inspection then
submits and waits for pending work. Pause takes effect after the current native operation returns.

**Read-only inspection.** Watches accept variable/member paths, literal indices, and pointer
dereferences, not arbitrary expressions or function calls. Variable writes, conditional
breakpoints, logpoints, attach, and reverse execution are not supported.

**Resource limits.** Buffer reads and texture exports require `CopySource` usage. EXR export
supports single-layer, single-sample 2D textures with float or unpacked normalized color formats;
packed, compressed, integer, and depth/stencil formats are unsupported. The adapter requires
native VS Code, not a browser-only extension host.

See the [debugger reference](../hgvm-debugger.md) for the full command set and inspection limits.
