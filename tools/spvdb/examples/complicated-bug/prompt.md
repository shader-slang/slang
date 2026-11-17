# Debugging exercise: spectrum_features.spv

You are a debugging agent.  Your task is to locate and explain a bug in a
compiled SPIR-V shader using the `spvdb` debugger.  The bug causes the shader
to panic (crash) for one specific input but not for others.

**IMPORTANT CONSTRAINT — you must use the debugger, not the source code.**
Do not read, grep, or otherwise inspect the Slang source file
`spectrum_features.slang` or any other source file in this directory to find
the bug.  You must discover the root cause entirely through the debugger
interface — by running the shader, examining backtraces, setting breakpoints,
stepping through execution, and printing variable values.  Explaining the fix
requires understanding *why* the bad value occurs, which the debugger will
show you.


## Background

The shader `spectrum_features.spv` is a GPU compute shader that extracts
spectral features from a 1-D time-domain signal.  Each invocation processes
one fixed-length frame of 32 samples and writes 10 floating-point features to
an output buffer.

The processing pipeline, briefly:
1. Load a frame of 32 samples from the input buffer.
2. Remove the DC offset and apply a preemphasis filter.
3. Apply a Hann window.
4. Compute the DFT magnitude spectrum (17 bins: DC through Nyquist inclusive).
5. Apply spectral whitening (per-bin normalisation).
6. Locate the dominant spectral peak.
7. Refine the peak position to sub-bin precision.
8. Compute secondary features (RMS, centroid, bandwidth, etc.) and write them.

The shader runs correctly for most inputs.  For one particular input it
panics.  Your job is to find out why.


## Setup

Assume:
- `spvdb` and `spectrum_features.spv` are both on your `PATH` / in the
  current directory.
- The two script files `run_clean.spvdbrc` and `run_buggy.spvdbrc` are in
  the same directory.
- The Slang source file is present on disk (so `list` commands will work),
  but **you must not open or read it** to reason about the bug.


## Reproducing the failure

Run the shader with the *clean* input first to confirm it succeeds:

```
spvdb spectrum_features.spv --source run_clean.spvdbrc
```

Then run it with the *buggy* input to observe the panic:

```
spvdb spectrum_features.spv --source run_buggy.spvdbrc
```

The clean run exits with code 0; the buggy run exits with code 1.

The clean run finishes normally.  The buggy run panics and prints a backtrace.
Note the function names and source locations in the backtrace — they tell you
where to focus.


## spvdb command reference

Launch an interactive session with:

```
spvdb spectrum_features.spv
```

Inside the interactive REPL, the following commands are available:

### Execution control

| Command | Short | Description |
|---------|-------|-------------|
| `run` | `r` | (Re)start execution from the beginning |
| `continue` | `c` | Resume until the next breakpoint or end |
| `step` | `s` | Step one source line, entering function calls |
| `next` | `n` | Step one source line, stepping *over* calls |
| `finish` | | Run until the current function returns |
| `stepi` | `si` | Step one SPIR-V instruction |

### Breakpoints

| Command | Description |
|---------|-------------|
| `break <file>:<line>` | Set a breakpoint at a source line |
| `break %<id>` | Set a breakpoint at a SPIR-V result id |
| `delete <bp-id>` | Remove a breakpoint by its id |

### Inspection

| Command | Short | Description |
|---------|-------|-------------|
| `backtrace` | `bt` | Print the current call stack with source locations |
| `info locals` | | Print all local variables in the current frame |
| `info outputs` | | Print output variables |
| `print <var>` | `p <var>` | Print the value of a named variable |
| `list [line]` | `l` | Show source lines around the current location |
| `disassemble` | `dis` | Show SPIR-V instructions near the current PC |

### Input setup

```
set input <set> <binding> <json-array>
set builtin GlobalInvocationID <x> <y> <z>
```

### Entry point selection

```
info entries              # list all entry points
entry <name>              # select an entry point
```

### Running a script

```
source <file.spvdbrc>     # execute commands from a file
```


## Suggested debugging workflow

1. **Reproduce the panic interactively.**  Start spvdb, then set up the same
   input as `run_buggy.spvdbrc`.  Issue `entry main` first (before `set input`)
   so that the session is initialised, then bind the descriptors, set the
   builtin, and type `run`.  Observe the panic message and backtrace.

2. **Locate the faulting function.**  The backtrace will show a chain of
   function calls.  Identify the innermost frame — the one that actually
   caused the out-of-bounds access.

3. **Set a breakpoint.**  Use `break <file>:<line>` to stop just before the
   faulting line (use the line number from the backtrace).  Restart the
   session with `run`.

4. **Inspect local variables.**  When the breakpoint fires, use `info locals`
   and `print <var>` to examine the values of the key variables.  Pay
   particular attention to any index or loop variable used to access an array.

5. **Understand why the index is wrong.**  Use `step` and `next` to trace
   backward: where did this index value come from?  Use `finish` to step out
   of a function, then inspect the caller's variables.  You can also `print`
   variables that were computed in an earlier step by name.

6. **Correlate with the input.**  The buggy input is a signal where every
   sample alternates between +1.0 and -1.0.  Think about what that means for
   the frequency content and how it affects the intermediate computations.

7. **Identify the root cause.**  Once you know the exact index value, the
   array it is indexing, and the size of that array, you should be able to
   state precisely what invariant the code assumed but the input violated.


## What to report

When you have found the bug, report:

1. **The faulting function name** (as shown in the backtrace).
2. **The line number** in the source file where the out-of-bounds access
   occurs (as shown in the backtrace or by `list`).
3. **The variable names and values** that led to the bad access
   (from `info locals` / `print`).
4. **The root cause in one sentence**: what did the code assume about its
   inputs, and when does that assumption fail?
5. **The minimal fix**: what change to the source code would prevent the
   panic without altering the correct behavior for other inputs?
