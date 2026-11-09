# HGVM debugger

`slangi program.slang` uses HGVM by default and requires a `[host]` entry point.
Use `slangi -legacy program.slang` to run an ordinary entry point with the legacy bytecode
interpreter. `-hgvm` explicitly selects the default engine. `-legacy` cannot be combined
with HGVM debugging, tracing, statistics, GPU execution modes, or precompiled `.hgbc` input.

Run `slangi -debug program.slang` to start an interactive HGVM debug session. Source inputs
are compiled with standard debug information. Precompiled `.hgbc` inputs work too; compile
with `slangc -target hgbc -g2` to retain source locations and inline scopes. `-debug` can be
combined with `-trace-all` or `-trace-n N`.

Debug sessions submit and wait after each GPU operation by default, including dispatches,
draws, uploads, and copies. Use `-gpu-execution buffered` to preserve normal command batching,
or `-gpu-execution synchronous` to select waiting explicitly. `-gpu-execution default` selects
synchronous execution when debugging and buffered execution otherwise. These options also work
with precompiled HGBC. Synchronous execution reduces CPU/GPU overlap; it does not step inside
GPU kernels.

The command syntax follows GDB for the supported subset:

| Command | Action |
| --- | --- |
| `run`, `r` | Run or restart the selected entry point. |
| `start`, `starti` | Run and stop before the first executable staged record. |
| `break FILE:LINE`, `break LINE`, `break FUNCTION`, `b ...` | Set a source or function breakpoint. A bare line uses the selected frame's file. Quote a file path with single quotes if needed. |
| `info breakpoints` | List breakpoint numbers, enabled state and resolved locations. |
| `delete [ID]`, `disable [ID]`, `enable [ID]` | Edit one breakpoint, or all breakpoints when ID is omitted. |
| `continue`, `c` | Resume until a breakpoint or exit. |
| `step [N]`, `s [N]` | Advance by source lines, entering calls. |
| `next [N]`, `n [N]` | Advance by source lines, stepping over calls, including inline calls. |
| `stepi [N]`, `si [N]` | Step executable staged records. |
| `nexti [N]`, `ni [N]` | Step executable staged records, stepping over physical calls. |
| `finish`, `fin` | Run until the innermost logical frame returns. |
| `backtrace`, `bt`, `where` | Show the logical stack, marking inline frames with `(inlined)`. |
| `frame [N]`, `f [N]`, `up [N]`, `down [N]` | Select and display a logical frame. |
| `info locals`, `info args` | Inspect variables in the selected logical frame. |
| `print EXPR`, `p EXPR` | Read a variable, member, indexed element, or pointer dereference. |
| `whatis EXPR` | Show the expression's declared type. |
| `list`, `l` | Show source around the selected frame. |
| `help`, `h`, `quit`, `q` | Show help or leave the debugger. |

An empty command repeats the previous command. Prompts and debugger output go to stderr;
program output and optional tracing remain on stdout. Quitting or closing stdin while stopped
unwinds managed host values and discards unsubmitted device work. Work already submitted to the
GPU cannot be cancelled by the debugger.

Source breakpoints resolve to recorded line ranges and block entries, including inline call
sites. Unresolved locations remain pending. Function breakpoints currently refer to physical
HGVM functions; use a source breakpoint to stop inside an inline function. Multiple locations
may resolve to one source breakpoint. Generated operations without line information remain
available to instruction stepping. No source location is synthesized for them.

`info breakpoints` uses the GDB columns `Num`, `Type`, `Disp`, `Enb`, `Address`, and `What`.
Addresses are hexadecimal offsets in 32-bit words from the staged program's base. A breakpoint
with multiple locations has a `<MULTIPLE>` parent row and numbered location rows such as `1.1`
and `1.2`, each with its own offset and available source location. Unresolved breakpoints display
`<PENDING>` and their original location specification. Enable, disable, and delete commands
currently operate on the parent breakpoint number; individual location controls are not implemented.

Variable paths support `value`, `value.field`, `items[3]`, `*pointer`, and `pointer->field`.
Indices are non-negative integer constants. Reference parameters display their current pointee;
ordinary locals display the latest `DebugValue` snapshot. A suspended caller's snapshot can lag
behind writes through an alias until its next update. Values before their first update may contain
garbage; there is no availability tracking. Debug copies do not keep managed objects alive.
Heap reads reject handles that no longer refer to a live owned object. Aggregate previews expand
up to three levels and sixteen children per level; strings are limited to 256 bytes. Use a more
specific path to inspect elements outside a preview.

Vectors and matrix rows display inline, for example `(100, 200)`. Aggregate summaries use
Slang type names without module prefixes, preserving namespaces and generic arguments. Native
objects show read-only properties: buffers include element count, stride, byte size, and usage;
textures include dimensions, format, mip/layer/sample counts, and usage. Native windows and
surfaces show their current state and configuration. These summaries inspect cached descriptors
only while stopped; they do not read GPU contents, wait for completion, or process window events.

Structured buffers expand as indexed arrays in VS Code, advertising their element count before
any readback. Expanding a page reads only that range. Struct fields, nested arrays, matrix rows,
and inline vectors use the same display rules as host values, after converting from the buffer's
actual device layout. Expressions such as `buffer[17].position` also read individual elements.
In the CLI, `print buffer` previews the first sixteen elements; `info locals` and `whatis` do not
read buffer contents. Buffer snapshots are shared by aliases while stopped and discarded on
resume. Inspection in buffered mode first submits and waits for preceding GPU work.

Buffer contents are read-only and require `CopySource` usage. Null or expired objects and failed
readbacks produce inspection errors without aborting the program. A stop retains at most 64 MiB
of converted buffer data; a child request returns at most 16,384 entries. Larger requests report
an error asking for a smaller page rather than silently truncating the array. Managed objects
inside device-buffer elements are not supported.

This version does not implement general expression evaluation, conditional breakpoints,
the `tbreak` command, CLI asynchronous interruption, argument passing, explicit `list`
locations, or `finish` for an outer selected frame. Source accuracy depends on compiler debug
information. `start` and `starti` currently share the same entry stop, which can be inside an
inline function when that is the entry point's first executable instruction.

## Interpreter and session contracts

`DebugSession` owns an interpreter and implements `IStepControl` and `IDebugHandler`. A frontend
prepares a `Program`, edits breakpoints, and calls `run` with an `IDebugSessionListener`. The
listener receives actual stops and returns the next command. This interface has no CLI or DAP
dependencies. Calls are synchronous and serialized on the executing thread.

`Desc::gpuExecution` accepts `GPUExecutionMode::Default`, `Buffered`, or `Synchronous`.
`DebugSession` resolves `Default` to `Synchronous`; an ordinary `Interpreter` resolves it to
`Buffered`. Explicit choices are preserved. The interpreter's `getDebugBufferInfo()` inspects
only cached state; `readDebugBufferElements()` performs checked, synchronous readback into
canonical host-layout bytes during a stopped callback.

Interpreter callbacks are optional and borrowed for the interpreter's lifetime. Step control
runs before the first executable record, then returns `Step`, `Continue`, or `Stop`. Continuing
from a breakpoint executes that instruction once; reaching it again stops again. The callback
receives the current record's breakpoint bit directly, including when stepping or stopping at
entry. The session looks up the breakpoint number only on a hit. Breakpoint addresses contain a
staged program ID and a 32-bit word offset. They must identify executable
record boundaries in a live staged program. They are not portable across interpreter instances
or restaging. `setBreakpoint` changes only the record's break bit and requires execution to be
stopped. It does not validate arbitrary offsets.

Scope directives run before the next executable instruction. A scope callback replaces the
current call's scope index and inline-site count, both carried directly in the staged record.
Function entry saves that context, and function exit restores
it after managed-value cleanup. Scope zero means unknown scope, not function exit. The session
retains compact `CallState` entries in `m_callStack`, with scope indices, inline-site counts,
call-site addresses and call IDs. Logical stack entries identify inline frames when the stack is
inspected; interpreter frames remain private. Variable declarations and canonical type layouts
live in the debug map.
DebugValue records copy bytes into non-owning frame slots, preserving typed field/index paths for
partial updates. No value callbacks, availability flags, or scope-transition resets are involved.
The session resolves variable visibility and reads values only on request while stopped, through
opaque active-call IDs and interpreter range checks. Heap reachability is computed lazily from
owning frame slots at a stop, without adding allocation callbacks.

The ordinary interpreter loop is a separate template specialization with no per-record debugger
checks. In continue mode the debug loop checks the break bit and delivers scope updates; there
is no per-instruction step callback. It also checks an atomic pending-control flag so a frontend
can request interruption without instruction stepping. Scope updates and function entry/exit use only scalar depth
changes and call-stack push/pop operations, without debug-map lookups.
`nexti` over a call and `finish` use temporary continuation breakpoints and run with `Continue`
between them. `next` also arms source/block boundaries at eligible inline depths. Block-entry
hits detect backward edges so a loop on one source line still stops on its next iteration.
Temporary hits check the live stack depth and invocation ID; a user breakpoint at the same address
always wins.
Temporary ownership is removed on a visible stop or run completion without clearing user bits.

`findInlineContinuations` isolates the inline continuation policy. It scans the current physical
function's staged scope setters and collects the first executable record after each final setter
whose inline-site count meets the requested depth. This is a conservative set, not a prediction
of which branch will execute; it includes earlier blocks and multiple exits. Consecutive setters
are combined, and value updates execute before the breakpoint. Physical caller continuations
cover returns without scope setters. All this work runs while stopped; scope callbacks still only
update scalar state. Explicit continuation metadata could replace the scan later.

`step` uses cached range bounds, numeric source keys prepared during staging, and a binary range
lookup when crossing a boundary. Stack expansion, path normalization, breakpoint
resolution, source-file reads and formatting happen during preparation, breakpoint edits or
actual stops, never in the scope callback.

## Writing debugger tests

Debugger CLI tests use the existing `slang-test` directive syntax:

```slang
//TEST:DEBUGGER(mode=CLI, prefix=CLI):

[host]
void main()
{
    //CLI-SEND: break '$file':[[@LINE+1]]
    print(42);
    //CLI-CHECK: Breakpoint 1, #0 main at {{.*}}:[[@LINE-1]]:
}

//CLI-SEND: run
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 42
//CLI-CHECK-NEXT: }
```

`PREFIX-SEND:` comments are collected in source order. The runner sends each command when the
CLI reaches a prompt, then closes stdin after the final command. An empty `SEND` sends an empty
line, exercising command repetition. `$file` expands to the test file's path; `[[@LINE]]` and
`[[@LINE+N]]` / `[[@LINE-N]]` expand relative to the SEND comment's line. Quote `$file` when using
it in a breakpoint so paths containing spaces work.

The runner invokes LLVM FileCheck with `PREFIX-CHECK`, so normal `-NEXT`, `-NOT`, `-DAG`, captures,
and line expressions are available. It checks the standard `slang-test` output envelope: result
code, stderr, then stdout. Checks preserve order within each stream; they do not assume ordering
between stdout and stderr. Nonzero exits, unused commands and timeouts fail independently of
FileCheck. Failures save a `.actual` file. The timeout uses the runner's existing
`SLANG_TEST_RPC_TIMEOUT_MS` setting.

Multiple headers can select different prefixes in one source file, or reuse a script against
precompiled bytecode with `precompile=2` (`slangc -g2`) or `precompile=0` (no debug information).
The temporary HGBC file is compiled within that test and removed afterward. Arguments after the
header's final colon go to `slangi`; entry selection and include paths also go to the precompiler.

Run the suite with `slang-test tests/compute/hgvm/debugger/`. The debugger always uses a standalone
`slangi` process, including when the surrounding suite uses `-use-test-server`.

## Debug Adapter Protocol

`slangi -dap` serves one debug session over stdin/stdout using DAP `Content-Length` framing.
The native Slang VS Code extension registers debug type `slang-hgvm` and launches this mode.
Select a development executable with the extension's `slang.slangiLocation` setting, or use the
bundled executable. For example:

```json
{
    "type": "slang-hgvm",
    "request": "launch",
    "name": "Debug HGVM",
    "program": "${workspaceFolder}/main.slang",
    "cwd": "${workspaceFolder}",
    "entryPoint": "main",
    "includePaths": ["${workspaceFolder}"],
    "api": "vulkan",
    "gpuExecution": "default",
    "stopOnEntry": true
}
```

Launch compiles source with standard debug information or loads an existing `.hgbc`. Execution
starts after `configurationDone`, allowing breakpoints to be installed first. Source and function
breakpoints, continue, step-in/over/out, pause, terminate, stacks, arguments, locals, watches, and
hover evaluation are supported. Expression syntax is the same read-only subset as the CLI.
There is one HGVM thread with logical inline frames; GPU kernels are not stepped. Missing source
information is left absent. Precompiled programs need their own debug information and source files.

The optional `gpuExecution` launch property accepts `default`, `buffered`, or `synchronous`.
Omitting it uses synchronous debugging. Set it to `buffered` to retain ordinary batching while
still allowing explicit buffer inspection and texture exports to synchronize with the GPU.

The adapter reads framed messages on a reader thread and processes them on the execution thread.
`DebugSession::requestControl()` is the only cross-thread session method. The interpreter observes
its atomic flag before the next executable record, after inline scope and debug-value updates.
`IDebugSessionListener::onControl()` can apply breakpoint updates while execution is internally
stopped and return `Resume` without replacing the active stepping plan. It can instead return
`Pause` for a visible stop, or `Stop` to unwind managed values. A blocked native operation must
return before pause or graceful termination can take effect. Disconnecting the transport requests
the same cleanup. All protocol output is serialized on the execution thread; program print output
uses `IHGVMOutputSink` to emit DAP `output` events.

DAP source breakpoint updates replace one source's set in a single resolution pass, retaining IDs
for unchanged lines. Function breakpoint updates replace the function set. An unresolved breakpoint
is unverified; conditional breakpoints and logpoints are rejected rather than treated as unconditional.
The adapter does not advertise attach, variable writes, exception breakpoints, or reverse execution.

The value API can return a bounded recursive CLI preview or a shallow value with an opaque
reference. `getValueChildren()` reads one level and supports indexed ranges, allowing DAP to expand
past the CLI preview's size and depth limits. References contain interpreter-checked locations and
opaque call IDs, and are discarded on resume. DAP frame and variable IDs are also invalidated on
resume and are not reused by later stops. No value inspection runs during ordinary execution.

DAP tests use `//TEST:DEBUGGER(mode=DAP, prefix=DAP):` and `//DAP-SEND:` comments. Each SEND is a
JSON request without `seq` or `type`; the runner supplies those and waits for the matching response.
`wait stopped`, `wait initialized`, and `wait terminated` wait for events, including ones that
arrived before the preceding response. `capture frame body.stackFrames.0.id` saves an integer from
the latest response, usable as `$frame` in later JSON requests. `$file` expands to the source path
and `$program` to the source or precompiled HGBC path; both are escaped for use inside JSON strings.
`[[@LINE+N]]` works as in CLI tests. FileCheck checks the decoded JSON transcript with `DAP-CHECK`.

## Texture inspection

In the Variables view, expand a texture to list `mip[0]`, `mip[1]`, and so on. Each mip shows its
actual dimensions and expands to width, height, and format. Listing these entries does not submit
GPU commands or read pixels. Right-click a mip and choose **Open Mip as EXR** to capture its contents.
The extension opens a temporary `.exr` with VS Code's configured editor association; install or
configure an EXR viewer if necessary. Snapshots remain available after continuing or ending the
debug session and are removed when their last editor closes. Files left open when VS Code exits
remain in the operating system's temporary directory.

Export completes pending GPU work, including in buffered mode, and reads only the selected mip.
The current implementation supports single-layer, single-sample 2D textures with CopySource usage.
Float16/32 and unpacked normalized 8/16-bit color formats are converted to uncompressed RGBA32Float
EXR. BGRA8 is reordered to RGBA; missing RGB channels are zero and missing alpha is one. sRGB RGB
channels are linearized, alpha is unchanged, and HDR values are preserved without tone mapping.
Packed, compressed, integer, and depth/stencil formats are unsupported. A mip's padded readback must
fit within 256 MiB; larger mips report an error before GPU work is submitted. Export failures do not
terminate the debugged program.

The native adapter exposes the custom DAP request `hgvm/exportTextureMip`, with a mip's
`variablesReference` and an absolute `destinationPath`. The destination must not already exist.
The response succeeds only after the EXR has been flushed and closed; partial files are removed on
failure. The operation runs on the execution thread while stopped. Mip references expire on resume,
just like other variable references. File handling stays in the adapter; `DebugSession` accepts a
sequential byte writer and has no dependency on VS Code or filesystem paths.
