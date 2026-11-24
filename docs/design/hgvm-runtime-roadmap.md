# HGVM Runtime, Interop, and AOT Roadmap

Status: design agreed on 20 November 2026. Phase 0a is implemented; later phases are not. Each
phase below lists its deliverables, the HGBC opcodes it deletes, and its acceptance criteria, so the
roadmap can be executed phase by phase. Decisions marked _settled_ were confirmed in design
discussion; items under [Open questions](#open-questions) still need a decision before the phase
that depends on them.

## Motivation

HGVM currently implements operating-system and GPU features as dedicated HGBC instructions. Consider
setting a window title. The feature is spread over the Lua opcode table
(`source/slang/slang-hgbc-opcodes.lua`, `SetWindowTitle` = `0x76`), the validator, the stager, a
`StagedRecord` struct, the record builder, the interpreter's `executeSetWindowTitle`, a virtual
method on `IHGVMHostServices`, the WASM bridge (`slangvm-browser-host.cpp`), the JavaScript host
(`browser-host.js`), and an `hgbc_asm` wrapper in `simple_window.slang`. Roughly 30 of the 184
opcodes are of this kind, host object kinds are a closed enum (`HGVMHostObjectKind`), and every such
instruction hand-writes its own marshalling (for example `executePollInputEvent` writes event fields
by byte offset).

This roadmap replaces that structure with one general interop mechanism, moves OS and GPU features
into ordinary Slang runtime libraries, and shrinks the VM core so that a future HGBC to LLVM
ahead-of-time (AOT) compiler only has to translate a small instruction set.

## Target architecture

```
+------------------------- Slang runtime libraries (.slang) -------------------------+
| os.window / os.input / os.time / os.fs / image / console / thread                  |
| gpu.context / buffer / texture / dispatch / raster / raytracing / surface          |
|      | [DllImport]            | [JSImport(async?)]          | [COM]                  |
+------+------------------------+-----------------------------+------------------------+
| Native providers: SDL3, system DLLs, hgvm-gpu (flat C API over slang-rhi, static)  |
| Web providers: window.js, WebGPU glue (main thread)                                |
+------------------------------------------------------------------------------------+
+------------------------------------ VM core ISA -----------------------------------+
| arithmetic, control flow, memory (T* values are raw host addresses),               |
| Call / CallNative / CallNativeIndirect, Retain / Release / FixLifetime,            |
| object allocation and free, PlatformSwitch, IMPT / EXPT sections,                  |
| EMBD shader-module data, debug instructions                                        |
+------------------------------------------------------------------------------------+
  Interpreter: browser, teaching, debugging, semantic reference
  HGBC -> LLVM AOT: product (native executables, wasm32); optional ORC JIT later
```

## Principles

These rules apply to every phase and are part of each design review.

1. **All semantics live in HGBC.** The AOT compiler translates HGBC, not `StagedRecord`. Staging may
   optimize but must not carry meaning that HGBC does not state; otherwise the AOT compiler has to
   reimplement it and the two engines drift apart.
2. **One canonical representation per value.** For example, interop pointers use the existing `T*`
   rather than a parallel pointer type, and Box metadata lives in one `TypeDescriptor` rather than
   being copied into each object.
3. **The interpreter is the semantic reference.** Every engine-visible behavior (deinit order,
   error codes, interop results) is checked by differential tests between the interpreter and
   other engines.
4. **Every phase deletes the ad-hoc opcodes it replaces** and keeps the full test suite passing.
   HGBC has no cross-version compatibility guarantee, so opcode values can be reused.
5. **Do not preclude multithreading or async/await.** No interpreter-global "current call" state,
   no library API that assumes a single thread, and asynchronous library operations return
   `Future<T>`.

## Settled decisions

### Runtime libraries replace ad-hoc instructions

Window, input, timer, byte stream, image codec, and console output become Slang runtime libraries.
GPU context, resource allocation, dispatch, rasterization, ray tracing, and presentation surfaces
also move into a library (see [GPU as a library](#gpu-as-a-library)). The VM keeps only the core ISA
shown above.

### Native interop through `[DllImport]`

Slang already supports `[DllImport]` (`core.meta.slang`, `IRDllImportDecoration`,
`slang-ir-dll-import.cpp`) and `[DllExport]` for the C++ backend, with marshalling rules in
`NativeCallMarshallingContext` (`slang-ir-marshal-native-call.cpp`). HGBC reuses the front end and
the marshalling rules but not `generateDllImportFuncs`, which emits C++ code.

- The compiler emits marshalling as explicit HGBC instructions. Consider
  `[DllImport("SDL3")] void* SDL_CreateWindow(NativeString title, int w, int h, uint64 flags)`
  called with a `String`: the compiler emits a string-borrow instruction followed by
  `OpCallNative`, whose operands are C ABI values only. The interpreter, the LLVM translator, and
  the JavaScript glue then share one simple call form.
- HGBC adds an `IMPT` section with `OpImportLibrary` and `OpImportFunction`, plus `OpCallNative`
  and `OpCallNativeIndirect` (call through a function pointer, used for COM and callbacks).
- Each import carries a complete C signature record: integer signedness and width, calling
  convention, `bool` as one zero-extended byte, no variadic functions, and parameter attributes
  (`nocapture`, `readonly`, `nounwind`). The same record format serves exports later.
- Library resolution happens at load time, in order: the static built-in registry, host-registered
  providers, then manifest and search-path lookup. Only reachable imports are bound. A missing
  library or symbol fails the load with the referencing function and the searched locations.
  `DefaultSharedLibraryLoader` already applies platform file-name decoration, so `"SDL3"` resolves
  to `SDL3.dll`, `libSDL3.so`, or `libSDL3.dylib`.
- Built-in libraries that ship inside the VM (for example `hgvm-gpu`) register typed thunks that a
  build-time generator produces from their Slang declarations. They need no libffi and work in the
  WASM build. External DLLs use libffi call stubs prepared per signature during staging.
- Loading arbitrary native code is gated by an `allowNativeLibraries` VM setting and an optional
  library allowlist.

### Web interop through `[JSImport]`

`[JSImport]` declares a JavaScript implementation, either inline (similar to Emscripten `EM_JS`) or
as a named export of a `.js` file shipped next to the module. Inline sources are stored as HGBC
literals; the WASM VM instantiates all imports of a program as one ES module. A file-based import
is the fallback for pages with a strict Content Security Policy.

- Values are converted, not shared: numbers, `BigInt` for 64-bit integers, JavaScript strings, and
  plain objects for blittable structs. `out`/`inout` values are returned as an array
  `[result, out1, out2, ...]`.
- JavaScript-owned objects are referenced through ids kept in a JavaScript-side table; the Slang
  wrapper class releases the id in its `__deinit`.
- Imports that may suspend are declared `[JSImport(async)]` (see
  [Asynchronous JavaScript](#asynchronous-javascript)).
- JavaScript imports are main-thread-only by default.

### Platform dispatch is folded when the VM loads

The Slang front end only sees the single `hgbc` target, so `__target_switch` cannot select a
platform. Instead:

- Capability atoms `hgvm_windows`, `hgvm_linux`, `hgvm_macos`, `hgvm_web`, and the alias
  `hgvm_native` describe the host platform. `[DllImport]` implies `hgvm_native`; `[JSImport]`
  implies `hgvm_web`; `[require(...)]` narrows further.
- A new statement `__platform_switch { case hgvm_web: ... case hgvm_native: ... }` is checked like
  `__target_switch` (capabilities narrow inside each case) but lowers to an opaque terminator
  `IRPlatformSwitch` that survives to HGBC as `OpPlatformSwitch`. SCCP and CFG simplification must
  treat all of its edges as live, as they already do for `IRTargetSwitch`.
- At load, the VM computes reachability from exports and embedded entry points, following only the
  edge of each `OpPlatformSwitch` that matches the host. Only reachable functions and blocks are
  staged, the switch becomes a direct branch, and only reachable imports are bound. A reachable
  reference to a function whose platform requirement excludes the host is a load error with the
  reference chain. Exports with a platform requirement are omitted from the export table on other
  platforms.
- The AOT compiler folds the same switch from the target triple; LLVM `globaldce` removes the rest.

Consider a window wrapper:

```slang
[DllImport("SDL3")] void* SDL_CreateWindow(NativeString title, int w, int h, uint64 flags);
[DllImport("SDL3")] void SDL_DestroyWindow(void* window);
[JSImport("window.js", "create")] uint jsCreateWindow(String title, uint w, uint h);
[JSImport("window.js", "release")] void jsReleaseWindow(uint id);

public class Window
{
    uint64 handle; // native pointer value, or JavaScript table id on the web

    __init(String title, uint w, uint h)
    {
        __platform_switch
        {
        case hgvm_web:    handle = jsCreateWindow(title, w, h);
        case hgvm_native: handle = uint64(SDL_CreateWindow(title, int(w), int(h), 0));
        }
    }

    __deinit
    {
        __platform_switch
        {
        case hgvm_web:    jsReleaseWindow(uint(handle));
        case hgvm_native: SDL_DestroyWindow((void*)handle);
        }
    }
}
```

On the web the VM never stages the native case and never tries to load SDL3; on desktop it never
instantiates `window.js`.

### Pointers and memory

- Interop uses the default `T*`, consistent with Slang's CPU targets and the existing C++ backend
  `[DllImport]`. In host code `T*` points to host memory. Device addresses in host code will use a
  distinct opaque type later (for example `DeviceAddress<T>`), and the host-to-device layout
  conversion rejects host `T*` fields.
- **Invariant:** every value of an address type (`Ptr`, `Out`, `InOut`, `Ref`, `BorrowIn`) is a raw
  host address (tag zero) that stays valid for the lifetime of its storage: frame addresses until
  the function returns, heap addresses while the object lives, literal addresses until the program
  unloads. Tagged addresses appear only as staged operands and internal VM handles.
- To make that possible, the frame stack never moves. A `FrameStack` interface hides two
  implementations: a reserved virtual-address region committed on demand on native hosts, and
  page-stable segments on the web (needed because several suspended execution contexts can exist
  there). Stack overflow fails the VM with the call chain instead of growing without bound. Today
  `stack.growToCount` can reallocate and `FrameAbsolute` addresses are offsets for that reason.
- String and binary literals used as data move out of the growable instruction storage into a
  stable per-program data segment, so a borrowed `NativeString` from a literal stays valid.

### FFI type rules

| Kind                                               | Rule                                                                                                                                              |
| -------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| Scalars, vectors, matrices (row-major)             | Passed by value                                                                                                                                   |
| Blittable structs and arrays of them               | Allowed when the staged layout has no managed leaves and no VM addresses; their VM layout equals the target C natural layout, asserted at staging |
| `in String`                                        | Borrowed as a NUL-terminated `const char*` for the duration of the call, zero-copy; literal storage is NUL-terminated by invariant                |
| `out` / `inout`                                    | Passed as a pointer to the caller's storage, no copy; aliasing (`f(a, a)`) is documented                                                          |
| `T*`, `NativeString`                               | Raw values; the VM does not manage their lifetime                                                                                                 |
| `[COM]` interfaces                                 | Allowed; ownership follows `AddRef`/`Release`                                                                                                     |
| Classes, structs with managed fields, `out String` | Rejected at compile time                                                                                                                          |
| Structs by value                                   | Deferred; pass by pointer. Clang can generate thunks later if needed                                                                              |

Native code must not retain pointers it receives after the call returns, and must not unwind
(C++ exceptions, `longjmp`) through VM frames. Returned `const char*` values are converted to
`String` only by an explicit copy; freeing them follows the API's own rule, written in the
wrapper. Pointer graphs returned by C are read through `T*` and deep-copied into Slang types
promptly, or kept behind a class that owns the root pointer and frees it in `__deinit`. Pointer
graphs passed to C are built in a `NativeArena` class that frees its allocations in `__deinit`.
`CPtr` width follows the target pointer size; `__sizeOf<T*>()` reports the target value.

### Ownership and reference counting

The compiler, not the stager, owns reference counting, because the AOT compiler and the interpreter
must execute identical ownership operations and because only Slang IR knows ownership semantics
well enough to optimize them.

- New instructions `OpRetain`, `OpRelease`, `OpFixLifetime`, `OpFreeObject`, and later
  `OpIsUnique`. Struct values are released field by field in IR; HGBC `OpRelease` applies to one
  reference (class, String, Box, RuntimeArray, COM reference).
- Conventions recorded in function signatures: parameters are borrowed (+0), results are owned
  (+1), and a `consuming` parameter transfers ownership.
- The insertion and optimization pass runs late in the IR pipeline, after a reserved slot for
  coroutine lowering. It removes retain/release pairs, forwards ownership at the last use, and
  infers borrows.
- Lifetimes: a named local whose type transitively has an observable deinit (a class with
  `__deinit`, a COM reference) keeps a lexical lifetime and is released at scope end, so guard
  objects behave as expected. Other values may be released after their last use. This mirrors
  Swift's lexical lifetimes.
- Each class gets a compiler-generated destroy function: call `__deinit`, release each managed
  field, free the object. `OpRelease` means "decrement; if zero, call the type's destroy". The
  stager's `OwningSlot` cleanup and the VM's recursive release through `managedLeafOffsets` are
  removed.
- `__deinit` runs synchronously on the thread that performs the zero-reaching release. Releases on
  threads without a VM execution context (foreign threads, the Python garbage collector) enqueue
  the object for a VM thread. Thread-affine resources are handled by their library, for example a
  window's `__deinit` posts destruction to the main thread.
- `this` must not escape from `__deinit`, checked at compile time, so the runtime needs no
  resurrection handling.
- Deep releases switch from recursion to a thread-local worklist past a depth threshold.
- There is no cycle collector; cycles leak until `weak` references exist.
- VM close releases outstanding host-held values explicitly; it does not sweep the heap.

In the interpreter, `OpRelease` behaves like a potential call: when the count reaches zero it
pushes the destroy function's frame and continues at the next record afterward. In AOT the release
is an inline decrement plus a conditional call:

```llvm
  %rc = load i64, ptr %obj
  %shared = icmp slt i64 %rc, 0
  br i1 %shared, label %atomic, label %local
local:
  %n = sub i64 %rc, 1
  store i64 %n, ptr %obj
  %zero = icmp eq i64 %n, 0
  br i1 %zero, label %destroy, label %cont
destroy:
  call void @Window.__destroy(ptr %obj)   ; __deinit, release fields, free
  br label %cont
atomic:                                    ; cold
  call void @hgvm_release_shared(ptr %obj, ptr @Window.__destroy)
  br label %cont
```

To keep releases from blocking LLVM optimizations, the compiler computes a transitive effect
summary per destroy function; types whose destroy only frees memory get
`memory(argmem: readwrite, inaccessiblemem: readwrite)` on the call. Allocation and free functions
carry `allockind`, `alloc-family`, and `allocsize`. Release branches are not marked cold by default
(short-lived temporaries usually do reach zero); uniqueness and escape analysis in Slang IR resolve
the known cases instead.

### Object header and type descriptors

Every heap object uses one header:

```c
struct ObjectHeader {
    uint64_t rc;                  // count; negative means shared (atomic) mode; room for weak side table
    const TypeDescriptor* type;   // compiler-generated, in the program's read-only data
};
```

`TypeDescriptor` holds the destroy function, a mark-shared function, the canonical type identity
(the address-based identity already used by Python interop), the parent class, size and alignment,
and the debug layout. The type pointer is required because classes can inherit (a `Base` reference
may hold a `Derived`), downcasts need the dynamic type, publishing to another thread must find each
object's mark-shared function, foreign-thread releases have no static type, and the debugger
inspects arbitrary objects. The C++ virtual table pointer, `HeapObjectKind`, the per-object copy of
`managedLeafOffsets`, and the debug-only event-tracking fields (moved to a side table keyed by
address) are removed. The AOT compiler still calls destroy directly for classes without subclasses
in the linked program, and objects that do not escape can be stack-allocated or scalar-replaced.

### Interface values and boxing

Witness tables do not exist at run time. `slang-ir-lower-dynamic-dispatch-insts.cpp` lowers an
interface value to a tagged union (`kIROp_HGVMTaggedUnionType`, tag = the witness table's unique id)
whose payload is a Box, and lowers method calls to dispatch functions built by `createDispatchFunc`
that `switch` on the tag. In HGBC this appears as `OpTaggedUnionType`, `OpBox`/`OpUnbox`, and the
tagged-union instructions. `OpBox` allocates an immutable heap copy of the concrete value;
`OpMakeTaggedUnion` builds the 16-byte value `{caseIndexPlusOne, caseTag, payload}` without
allocating. Boxing happens when a concrete value converts to an interface type and when owned
erasure (`lowerHGBCOwnedPayloads`) replaces `PackAnyValue`, `UnpackAnyValue`, or `Reinterpret`.

The switch-based closed-world dispatch stays, because it is the best form for AOT. Phase 0b
improves boxing:

- A class value is its own box (the unified header already carries the count and type), so
  converting a class to an interface allocates nothing.
- Non-mutating method calls borrow the payload inside the box instead of `OpUnbox` copying it.
- Mutation checks `OpIsUnique` and modifies the box in place when unique (copy-on-write).
- Box metadata comes from `TypeDescriptor`; the unbox type check compares descriptor pointers.
- Inline storage for small payloads without managed fields (as in Swift existential containers)
  is optional and decided by AOT measurements.

COM interfaces are excluded from this lowering (`isEffectivelyComPtrType`) and use real vtables.
Values of an interface type created by one loaded program with a conformance unknown to another
program are rejected by the existing public-id mapping; open-world conformances need runtime
witness tables and are an open question.

### COM and error codes

- COM import: each COM requirement records an explicit `ComVTableSlot(index)` decoration at the
  declaration (IUnknown occupies 0 to 2, then declaration order with inherited interfaces first),
  because COM's ABI is positional while witness-table access must remain key-based. `lowerComMethods`
  runs for `hgbc` and expands calls to a vtable load plus `OpCallNativeIndirect`. A COM reference is a
  managed leaf whose retain and release call `AddRef` and `Release`. Casting between COM interfaces
  calls `QueryInterface`; failure yields null.
- `throws` maps to native error codes when the error type is an error-code type (an `enum : int32`
  conforming to a marker interface, with `HResult` and `SlangResult` provided):
  `T f(args) throws E` becomes `int32 f(args, T* out)` and a negative code is an error.
  `[PreserveSig]` disables the transformation for APIs that need success codes such as `S_FALSE`.
  Other error types keep the existing status-plus-two-out-parameters convention.
- COM export (a Slang class implementing a COM interface for native code) is deferred with the
  other exports (see below). It requires a COM-callable wrapper whose `AddRef`/`Release` forward to
  the class object's count and whose `QueryInterface(IID_IUnknown)` returns a stable identity.

### Callbacks and reentrancy

A narrow callback feature comes before general exports. FFI signatures may take a function-typed
parameter when the C API also takes a `void* userdata`; the compiler passes a static trampoline and
a retained closure. Native interpreter builds use one generated trampoline per callback signature,
the web build passes a JavaScript wrapper function that re-enters the VM, and AOT passes real
function pointers. APIs without a userdata parameter are rejected until exports exist. The
registration owner releases the closure (for example `SDL_RemoveEventWatch` in a wrapper's
`__deinit`; JavaScript wrappers have `dispose()` with a `FinalizationRegistry` backstop).

Same-thread reentrancy is allowed. A re-entry boundary is an error-isolation boundary: a callee
that `throws` returns its error code; `fail()` in a non-throwing callee aborts with a message rather
than unwinding through native frames. A thread without an execution context that calls into the VM
gets one attached automatically.

Libraries expose callback-style APIs to users while polling at the FFI boundary where possible:

```slang
public void run(Window window, IWindowHandler handler)
{
    while (processWindowEvents(window))
    {
        InputEvent e;
        while (pollInputEvent(window, e))
            dispatchEvent(handler, e);
        handler.onFrame(timer.tick());
    }
}
```

Synchronous OS callbacks are needed only for cases such as redrawing during a native live-resize
modal loop.

### Exports

`[DllExport]` and COM export are implemented together with AOT, where they cost little: an export
is a `dllexport` function, a COM export is a static vtable of thunks. The export record format (an
`ExportKind::NativeFunction` entry with the shared C signature record) is defined in Phase 1 so the
AOT work needs no format change. Exports follow the same type rules as imports, plus an opaque
handle convention for classes handed to C (a class must implement a `[COM]` interface to cross the
boundary). The compiler generates a C header (C structs with `_Static_assert` sizes, C-style
`lpVtbl` and C++ declarations for COM interfaces) and `<lib>_init(config)` / `<lib>_shutdown()`;
initialization never happens in `DllMain`. `fail()` inside a non-throwing export aborts; `throws`
maps to error codes; `<lib>_last_error()` reports the message. On wasm32 an export that may suspend
is wrapped in `WebAssembly.promising`. Interpreter-side exports (libffi closures) are optional
later work.

### Threading

The VM will support shared-memory multithreading.

- Program code, staged records (read-only after finalize; breakpoint bits use atomic writes),
  type descriptors, and the heap are shared. Each OS thread runs its own execution context with
  its own frame stack and current-call state.
- Objects start in local mode with non-atomic counts. Publishing an object to another thread
  (`Thread.spawn` captures, channel send, storing into a global, storing into a field of a shared
  object, handing it to foreign code that may use it on another thread) marks its reachable graph
  shared, switching to atomic counts, as in Lean 4. Storing a managed reference into a shared
  object's field has a write barrier. Without threads (single-threaded WASM) objects never become
  shared and no atomic instruction is executed. Today `RefObject` is always atomic
  (`slang-smart-pointer.h`), which is one reason objects get their own header.
- Data races on plain fields are undefined behavior in the first version. Libraries provide
  `Atomic<T>`, `Mutex`, `Channel`, and `Thread.spawn`; sendability checking is future work.
- Host globals are shared and use thread-safe one-time initialization.
- Thread-affine APIs are marked `[MainThread]` (window functions, JavaScript imports by default)
  and checked at run time in debug builds.
- `hgvm-gpu` is thread-safe: submission is locked internally and command encoders are per thread.
- On the web, threads exist only in cross-origin-isolated pages; `Thread.isSupported()` reports it.
  JavaScript imports and WebGPU are main-thread-only.

### Asynchronous JavaScript

- JSPI is required (the WASM runtime already checks for `WebAssembly.Suspending` and
  `WebAssembly.promising` in `javascript/slangvm/src/index.js`); there is no Asyncify fallback.
- Only imports declared `[JSImport(async)]` are wrapped as suspending. A non-async import that
  returns a Promise is an error. `callBrowserHost` currently awaits any Promise and must change so
  the interpreter and AOT agree.
- Slang has no `async`/`await` syntax yet. The compiler infers which functions may suspend; exports
  that may suspend are wrapped with `WebAssembly.promising`. Indirect calls are assumed to suspend
  unless an export asserts `[JSExport(sync)]`, which traps if violated.
- A suspension is a cooperative yield: inbound calls that arrive while one context is suspended run
  in a new execution context on the same thread.
- `async` is valid only on JavaScript imports; native imports block.

To stay compatible with future language-level `async`/`await`:

- A call to an async import from synchronous code means "await immediately"; existing code keeps
  its meaning when `await` arrives.
- Coroutines will be lowered in Slang IR to state machines whose frames are class objects, before
  the reference-counting pass.
- Library asynchronous operations return `Future<T>` with `wait()` now and `await` later.
- Executors resume tasks on their original thread by default; task migration would be an implicit
  publication point.
- Debug information must not assume that every frame lives on the stack.

### GPU as a library

- `hgvm-gpu` is a C++ library over slang-rhi with a flat C API and opaque handles. It owns devices
  and queues, the buffer pool, resource-state tracking and automatic barriers, deferred readback,
  pipeline caches, acceleration structures, and shader binding tables. Event tracking and statistics
  are reported through callbacks. A flat API (rather than COM) keeps it usable from the WASM build,
  where COM export is unavailable; it is linked statically and registered through the built-in
  thunk generator.
- Slang wrapper classes own the handles and release them in `__deinit`. Asynchronous completion
  (readback, `mapAsync`, `onSubmittedWorkDone`) returns `Future<T>`; `Defer<T>` and `OpWaitDefer`
  become a library future.
- Shader code stays in HGBC as data (`OpSlangModule`, `OpSlangEntryPoint`,
  `OpSpecializeSlangEntryPoint`); AOT products embed precompiled per-backend artifacts instead.
- A dispatched lambda is split by the compiler into an entry-point reference and a generated Slang
  packing function that writes the captures into argument bytes. That function replaces the VM's
  `LayoutConverter`, so AOT can optimize it.
- Large pipeline-state records such as `OpGraphicsPipelineState` become blittable Slang descriptor
  structs passed to the library, which creates and caches pipelines.
- Presentation surfaces move to the library and take a native window handle (or canvas) from the
  window library.

### Execution engines

- The interpreter remains the engine for the browser, teaching, and debugging, and the semantic
  reference for differential tests.
- AOT translates HGBC to LLVM IR (through slang-llvm's builder): SSA values via `mem2reg`/SROA,
  internal linkage, precise attributes on runtime helpers, runtime fast paths as bitcode for LTO,
  cold error paths, exact numeric semantics (no `nsw`, guarded division, saturating float-to-int,
  no fast-math). Products are native executables and wasm32 bundles; an ORC JIT for desktop may
  follow.
- Generics are fully specialized and interface dispatch is a closed-world `switch`, which LLVM
  turns into jump tables and inlines.

## Phases

```
0a -> 0b -> 1 -> 2 -+-> 3
                    +-> 4 -> 5 -> 6 (JIT, interpreter exports, async/await)
                    +-> 6 (multithreading may start after Phase 2)
```

### Phase 0a: runtime invariants (no behavior change)

Deliverables:

- `FrameStack` with the native reserved-region implementation and the segmented web
  implementation; stack-overflow diagnostics with the call chain.
- Stable per-program literal data segment; NUL-terminated literal invariant.
- Address-type values become raw host addresses, asserted where produced.
- Interpreter "current call" state moves into an execution-context object.
- Fix the known `marshalNativeCall` bugs (see [Known pre-existing bugs](#known-pre-existing-bugs)).

Acceptance: the existing suite passes; new tests cover deep recursion failure and literal pointer
stability across a second program load.

Implementation notes:

- `FrameStack` (`slang-hgvm-frame-stack.h`) is selected by `Desc::frameStackKind`; the default is
  the reserved region on native hosts and segments on WebAssembly. `Desc::frameStackByteLimit`
  (64 MiB by default) bounds each execution context's stack.
- `AddressKind::Heap` is renamed `AddressKind::Native`, because tag zero now names any raw host
  address (frame, heap, or literal), and `FrameAbsolute` is removed. `FrameRelative`,
  `ConstHeap`, and the instruction kinds remain staged operands and internal handles only.
- String and binary literal bytes live in each program's `LiteralSegment`; scalar literals stay in
  the instruction storage, where only staged operands read them.
- The execution context owns the frame stack and the active invocation; the interpreter has a
  single main context until Phase 6 adds one per thread.
- The front end rejects source-level recursion for the `hgbc` target, so the deep-recursion tests
  are C++ unit tests (`tools/gfx-unit-test/hgvm-frame-stack.cpp`) that retarget a call in the
  serialized HGBC, as the debugger's recursion test already does.
- The `marshalNativeCall` fixes are in a separate change to master (see
  [Known pre-existing bugs](#known-pre-existing-bugs)), so this branch picks them up with the next
  merge from master.

### Phase 0b: explicit ownership and deinit

Deliverables:

- `OpRetain`, `OpRelease`, `OpFixLifetime`, `OpFreeObject`; ownership conventions in signatures;
  the late IR pass that inserts and optimizes them.
- `__deinit` in the front end, compiler-generated destroy functions, the `this`-escape check.
- Unified object header and `TypeDescriptor`; Box uses descriptor copy/destroy functions.
- Boxing improvements: class-as-box, borrowed payload access, copy-on-write with `OpIsUnique`.
- The stager stops inserting ownership operations; `OwningSlot` cleanup is deleted.

Acceptance: tests pin deinit order (FIFO by release), lexical lifetimes of guard objects, last-use
release of plain values, deep-list release without stack overflow, and boxing without extra
allocation for classes.

### Phase 1: interop core

Deliverables:

- `IMPT` section, `OpImportLibrary`, `OpImportFunction`, `OpCallNative`, `OpCallNativeIndirect`,
  the C signature record, and the export record format.
- HGBC marshalling pass reusing `NativeCallMarshallingContext`; FFI type rules in the validator.
- Library resolution chain, libffi stubs, static built-in registry and thunk generator,
  `allowNativeLibraries`.
- `[JSImport]` (inline and file), `[JSImport(async)]`, WASM instantiation and conversions; change
  `callBrowserHost` to reject unexpected Promises.
- Platform capability atoms, `__platform_switch`, `OpPlatformSwitch`, load-time reachability.
- `T*` loads and stores on native memory; `NativeArena`.
- Port the timer and console output.
- FFI test support: an in-repo C test library (for example `tests/hgvm-ffi/ffi-test-lib.c`) that
  slang-test builds with its downstream C compiler and that HGVM tests import, covering the ABI
  cases under [Testing strategy](#testing-strategy).

Deleted opcodes: `TimerType`, `CreateTimer`, `TimerElapsed`, `ResetTimer`, `Print`.

Acceptance: the timer and console libraries run on desktop and in the browser from one HGBC file;
missing-library and wrong-platform diagnostics are tested.

### Phase 2: OS libraries

Deliverables:

- Error-code `throws` convention and `[PreserveSig]`.
- Window and input library (SDL3 on desktop, `window.js` on the web) with polling at the FFI
  boundary and a Slang handler API.
- File byte streams and image codecs; asynchronous operations return `Future<T>`.

Deleted opcodes: `WindowType`, `CreateWindow`, `ProcessWindowEvents`, `PollInputEvent`,
`CloseWindow`, `SetWindowTitle`, `ByteStreamType`, `ByteStreamFromFile`, `CloseByteStream`,
`ByteStreamGetSize`, `ByteStreamRead`, `ByteStreamWrite`, `ImageDecode`, `ImageEncode`. Byte-stream
and image operations with device endpoints may keep a library-internal path until Phase 4. The
window part of `IHGVMHostServices` is removed.

Acceptance: existing window demos run unchanged in behavior on desktop and web.

### Phase 3: COM import, reentrancy, callbacks

Deliverables:

- `ComVTableSlot` decorations; `lowerComMethods` for `hgbc`; COM managed leaf; `QueryInterface`
  casts.
- Same-thread reentrancy with error isolation; automatic context attachment for foreign threads.
- Userdata callbacks (native trampolines, JavaScript wrappers); inbound calls during a JSPI
  suspension run in a new context.
- Synchronous redraw during native live resize; synchronous browser event handling where needed.

Acceptance: a COM import test against Slang's own `ISlangBlob`; reentrancy tests that check
borrowed pointers stay valid across nested calls; callback lifetime tests.

### Phase 4: GPU as a library

Deliverables, in order (each step deletes its opcodes):

1. `hgvm-gpu` C API design (object model, command recording granularity, futures).
2. Buffers: allocation, readback, copy; `Defer` becomes `Future`. Candidates: `AllocateResource`,
   `StructuredBufferGetDimensions`, `ReadBackBufferElement`, `AsyncReadBackBufferElement`,
   `DeferType`, `WaitDefer`, `UploadBufferStorageType`, `UploadBufferStore`, `UploadBufferUploadTo`.
3. Compute dispatch: lambda splitting and generated packing functions. Candidates: `MakeDispatcher`,
   `LayoutConverter`, `TypeLayout`.
4. Textures, blit, clear, surfaces. Candidates: `TextureGetDimensions`, `BlitTexture2D`,
   `ClearTexture`, `SurfaceType`, `CreateSurface`, `ResizeSurface`, `GetSurfaceDimensions`,
   `WaitForSurface`, `AcquireSurfaceTexture`, `TryAcquireSurfaceTexture`, `PresentSurface`,
   `TryPresentSurface`.
5. Graphics pipelines and draws. Candidates: `GraphicsPipelineState`, `GraphicsColorTargetState`,
   `GraphicsAttachments`, `MakeGraphicsPipeline`, `GraphicsDrawType`, `MakeGraphicsDraw`,
   `GraphicsDrawParametersType`, `MakeGraphicsDrawParameters`.
6. Ray tracing. Candidates: `RaytracingAccelerationStructureType`,
   `RaytracingBottomLevelAccelerationStructureType`, `AllocAccelerationStructure`,
   `BuildAccelerationStructure`, `MakeRayTracer`, `CreateRaytracingScene`, `RaytracingPipelineType`,
   `MakeRaytracingPipeline`, `SetRayMiss`, `RayHitGroup`, `SetRayHitGroup`, `DispatchRays`.

Resource type opcodes (`RWStructuredBufferType`, `StructuredBufferType`, `Texture2DType`,
`RenderTarget2DType`, `DescriptorHandleType`) and descriptor-handle conversions are decided in
step 1, because device code still refers to these types.

Acceptance: the VM core ISA matches the target architecture; GPU tests and demos pass on Vulkan
and D3D12 in CI and on WebGPU in the browser.

### Phase 5: AOT

Deliverables:

- HGBC to LLVM translation of the core ISA, ownership lowering (inline fast paths, destroy effect
  summaries, allocator attributes), `OpCallNative` with signature attributes, `OpPlatformSwitch`
  folding.
- `[DllExport]` with real symbols, generated C headers, init/shutdown; COM export with static
  vtables.
- wasm32 output with JavaScript glue and JSPI wrapping of imports and exports.
- Precompiled shader artifacts; static linking of `hgvm-gpu` and libraries.
- Differential tests run on the interpreter and AOT; FFI tests also compare against the C++ backend.
  Export tests use C drivers: a slang-test directive builds a module with one engine and calls the
  driver's `int run_test(GetExportFn getExport)` with that engine's exports.
- Fast allocator (thread-local size classes, for example mimalloc).

Acceptance: the differential suite passes on both engines; benchmark results are tracked in CI.

### Phase 6: later work, ordered by need

- Multithreading implementation: contexts per thread, shared mode, write barrier, `Thread`,
  `Mutex`, `Channel`, web threads. Depends only on the Phase 0 invariants and may start after
  Phase 2.
- Language-level `async`/`await`.
- ORC JIT on desktop.
- Interpreter-side exports (libffi closures, COM-callable wrappers, callbacks without userdata).
- `weak` references; sendability checking; escape analysis improvements.

## Testing strategy

Three layers, from fastest to most authoritative:

1. **In-Slang round trips.** A module exports a function and imports it back with
   `[DllImport("", "name")]`, as in `tests/cpu-program/class-com.slang`. Fast, but caller and callee
   share the marshalling code, so symmetric bugs cancel out.
2. **C drivers.** A C shared library compiled by the downstream compiler that slang-test already
   uses calls the exports through `getExport` and includes the generated header, so layouts are
   checked by `_Static_assert`. A small in-repo C test library (for example
   `tests/hgvm-ffi/ffi-test-lib.c`) provides import targets and callback entry points.
3. **Differential runs.** The same tests run on the interpreter, the C++ backend (as an AOT
   stand-in for the FFI subset), and AOT once it exists.

ABI cases to cover: sign and zero extension of 8- and 16-bit integers; interleaved integer and
floating-point arguments; stack-passed arguments beyond the register count; pointers, `out`/`inout`
write-back, struct arrays, null pointers; UTF-8 strings; error-code `throws`; COM reference counting
and identity; reentrancy; calls and releases from other threads; calls after VM close (closures
must detect it and abort with a message); `fail()` isolation (death tests).

Performance: a benchmark suite (binary-trees, n-body, spectral-norm, fannkuch, string processing, an
interface-dispatch microbenchmark, and a representative h-slang host workload) compared against C#
NativeAOT and Swift `-O`. Before AOT exists, compiling host code through the C++ backend with clang
`-O2` gives an early estimate of what the IR shape allows.

## Performance expectations

| Workload                       | Versus C# NativeAOT | Versus Swift `-O` | Reason                                    |
| ------------------------------ | ------------------- | ----------------- | ----------------------------------------- |
| Numeric, vector, matrix        | Equal or faster     | Equal or faster   | Vector types map to LLVM vectors          |
| Generic code                   | Equal or faster     | Faster            | Whole-program specialization              |
| Interface dispatch             | Faster              | Faster            | Closed-world switch, inlinable            |
| Allocation-heavy object graphs | Slower              | Similar           | Reference counting versus generational GC |
| String processing              | Similar             | Similar           | Depends on runtime-library fast paths     |

The decisive factor for typical code is the quality of the ownership optimizer, followed by escape
analysis, the allocator, and the boxing improvements. The ownership optimizer is a long-term project
with its own staffing, not a side task of Phase 5.

## Design review checklist

- Does any semantics live only in the stager?
- Does the change add single-threaded assumptions or interpreter-global state?
- Do asynchronous library operations return `Future<T>`?
- Does the ownership pass still run after the coroutine-lowering slot?
- Does a guard or special case handle a shape whose producer should be fixed instead?

## Open questions

| Question                                                                                                                               | Needed before              |
| -------------------------------------------------------------------------------------------------------------------------------------- | -------------------------- |
| `hgvm-gpu` object model and command-recording granularity                                                                              | Phase 4                    |
| AOT shader precompilation matrix (backends, specializations) and its EMBD format                                                       | Phase 5                    |
| Worker-thread calls to JavaScript imports: reject or proxy to the main thread                                                          | Phase 6 threading          |
| Sendability checking in the language                                                                                                   | Phase 6                    |
| Open-world interface conformances across separately loaded programs or AOT modules (runtime witness tables for interfaces marked open) | When plugins are needed    |
| Inline small payloads in interface values                                                                                              | Phase 5, after measurement |

## Known pre-existing bugs

Both are fixed on master by shader-slang/slang#14679.

- `NativeCallMarshallingContext::marshalNativeCall` passes `resultVar` twice for a `Result` return,
  so the error out-parameter receives the wrong address. The branch is currently unreachable:
  `lowerErrorHandling` runs at the end of lower-to-IR, before `generateDllImportFuncs`, so an
  imported or exported `throws` function passes the lowered `ResultType` struct by value. The
  error-code `throws` convention of Phase 2 has to run its marshalling before error handling is
  lowered, or lower `Result` into that convention directly.
- The same function does not copy `out`/`inout` arguments back after the call (marked `TODO`).
  Only `String` pointees need a copy: other pointees already have their native representation and
  pass the caller's storage directly. The `[DllExport]` wrapper still lacks the mirror-image
  conversion for `out String`, which needs an ownership rule for the returned `const char*`.

## Source map

| Area                          | Files                                                                                                               |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Opcode schema                 | `source/slang/slang-hgbc-opcodes.lua`, `include/slang-hgbc-format.h`                                                |
| HGBC emission                 | `source/slang/slang-emit-hgbc.cpp`                                                                                  |
| Owned erasure and boxing      | `source/slang/slang-ir-hgbc-owned-erasure.cpp`, `source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp`            |
| Native marshalling            | `source/slang/slang-ir-marshal-native-call.cpp`, `slang-ir-dll-import.cpp`, `slang-ir-dll-export.cpp`               |
| COM lowering                  | `source/slang/slang-ir-com-interface.cpp`, `slang-ir-lower-com-methods.cpp`                                         |
| Staging and execution         | `source/slang-hgvm/slang-hgvm-interpreter.cpp`, `slang-hgvm-staged-records.h`, `slang-hgvm-staged-record-builder.*` |
| Addresses and heap objects    | `source/slang-hgvm/slang-hgvm-types.h`                                                                              |
| Frame stack                   | `source/slang-hgvm/slang-hgvm-frame-stack.h`, `slang-hgvm-frame-stack.cpp`                                          |
| Host services (to be removed) | `source/slang-hgvm/slang-hgvm.h` (`IHGVMHostServices`)                                                              |
| WASM bridge                   | `source/slangvm-wasm/slangvm-browser-call.cpp`, `slangvm-browser-host.cpp`, `javascript/slangvm/src/`               |
| Runtime libraries             | `source/standard-modules/host/runtime/`                                                                             |
| Python interop design         | `docs/design/hgvm-python-interop.md`                                                                                |
