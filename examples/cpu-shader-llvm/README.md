Running Slang compute shaders on CPU via LLVM
=============================================

This example demonstrates two different ways of running Slang compute shaders
on CPUs by using the direct-to-LLVM emitter. The process is a bit different than
running shaders on GPUs because there is no graphics API involved. You are
directly responsible for calling into the shader code.

The samples demonstrate two suggested workflows for running shaders on CPUs:

* `jit.cpp` demonstrates usage of the JIT compilation feature by compiling the
  shader during runtime.
* `link.cpp` statically links to a shader that is compiled during build time.

**NOTE:** This README uses Khronos terminology for clarity, where
*work item* = "thread", *workgroup* = "thread group". This is to avoid mixing
work items with CPU threads, as they do not actually map to each other.

## `jit.cpp`: JIT shader compilation during runtime

This sample program demonstrates compiling a compute shader during runtime and
running the shader using the Slang C++ API.

### Why or why not JIT compile

**Slang API:** If you are already using the Slang API in your program, this
approach may be more straightforward to use.

**Faster iteration:** JIT compilation enables hot reloading the Slang shader as
you can reload and compile new shaders in your program without re-linking the
host program itself.

**Exact optimization for host CPU:** JIT compilation allows LLVM to optimize the
shader for the host's CPU architecture (akin to `-march=native` in C compilers).
This means that extensions like AVX512 can be utilized when they happen to be
available instead of imposing a hard requirement on them being available.

**Compilation delay:** As compiling occurs during runtime, loading a shader 
causes a delay. 

**Need to ship Slang libraries:** For JIT compilation, your host program must
ship with the Slang libraries, including `slang-llvm.dll/so`, which is fairly
big due to including LLVM (around 100 MB).

## `link.cpp`: ahead-of-time shader compilation

This program uses `slangc` during build time to produce object code from the
shader source. This object code is then linked with the host program.

### Why or why not compile ahead of time

**No need to ship Slang libraries:** The Slang compiler is only being used
during build time. This approach can reduce the memory footprint and loading
times compared to the JIT method that has to ship with Slang libraries.

**No loading times:** No compiling work occurs during runtime.

**Embedded systems support:** Ahead-of-time compilation allows cross-compiling
Slang code for platforms that cannot host the Slang compiler themselves.

**No Slang API:** If you are not using the Slang API, this approach allows you
to keep using the `slangc` standalone compiler. Correspondingly, reflection
information is not available.

**More complex build configuration:** You need your build system to run `slangc`
to produce object code and link it as part of your program.

**Slow iteration times:** In order to run a shader you've just changed, you need
to re-compile it into object code and re-link your whole program again.

## Running the compute shader from the host program

With either approach, the shader is compiled into native CPU machine code. The
host program is then able to call the shader in the following ways:

* **Recommended:** Per workgroup
    - One function call runs a single workgroup invocation
* **Not recommended:** Compute dispatch
    - Runs the specified number of workgroups in a single-threaded loop
* **DO NOT USE:** Per work item
    - Runs a single work item without considering others.

The per workgroup approach is recommended and demonstrated in this sample,
because it's both very flexible and performant. It allows LLVM to vectorize work
items onto SIMD lanes, yet it allows the user to implement multithreaded
execution in their preferred manner, as they can dispatch workgroups from any
thread.

The per work item approach should not be used, because it cannot work with
barriers and shared memory once they become supported on the CPU targets. It
also prevents many vectorization opportunities, causing worse performance on
almost any modern CPU and shader.

## Advanced topics

This section lists advice for very specific needs and is not necessary to read
for basic use.

### Debugging

You should be able to use a debugger with the resulting code and step through
the Slang code, as long as you ran `slangc` with debug symbols enabled
(`-g1`, `-g2` or `-g3`). You should also disable optimizations with `-O0` to
make the debugging experience more straightforward; workgroup-level
vectorization can make the program difficult-to-impossible to follow in a
debugger, and `-O0` prevents that from happening.

### Passing parameters to LLVM

You can pass parameters directly to LLVM using the `-Xllvm` flag, e.g.
`-Xllvm -float-abi=soft`. To set multiple flags, you can use the
`-Xllvm... <flags> -X.` syntax. Available LLVM options can be viewed with
`llc --help` or `llc --help-hidden`.

### Cross compilation

`slangc` can be used to cross-compile shaders for CPUs other than the host. You
can set the [target triple](https://clang.llvm.org/docs/CrossCompilation.html#target-triple)
with `-llvm-target-triple <target-triple>`. To set a
[specific CPU type](https://clang.llvm.org/docs/CrossCompilation.html#cpu-fpu-abi)
and enable all the extensions it has, use `-llvm-cpu <cpuname>`. To enable
individual extensions, use `-llvm-features <comma,separated,list>`. The list of
features that may exist on an architecture can be viewed with
`llc -march=<arch> -mattr=help`.

For example, when targeting the RP2350 microcontroller with ARM Cortex M33
CPU cores, the following flags can be used:

```bash
slangc -target shader-object-code -emit-cpu-via-llvm -llvm-target-triple thumb-v8-eabi -llvm-cpu cortex-m33 -llvm-features armv8-m.main,fp-armv8,dsp -Xllvm... -function-sections -data-sections -float-abi=soft --stack-size-section -X. shader.slang -o shader.o
```

If you want to cross compile for a target other than X86 or ARM CPUs, you may
need to compile Slang with your own LLVM build. The official `slang-llvm.dll/so`
distribution only contains support for some of the most common CPU architectures.
For example, if you wish to target RISC-V, you need to have an LLVM build with
support for that architecture and build Slang with
`-DSLANG_SLANG_LLVM_FLAVOR=USE_SYSTEM_LLVM`.

### Inspecting LLVM IR

If you wish to run your own custom passes on the LLVM IR produced by `slangc`,
debug the LLVM emitter, or inspect the IR for any other reason, you can use the
`llvm-shader-ir` and `llvm-host-ir` targets. They correspond to
`shader-object-code` and `host-object-code`, but output the IR before running
codegen but after optimization.
