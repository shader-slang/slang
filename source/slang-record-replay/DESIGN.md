# Slang Record-Replay System Design

This document describes the architecture and design of the Slang Record-Replay system,
which enables capturing and replaying Slang API calls for debugging, testing, and
determinism verification.

**Last updated:** 2026-02-08

## Overview

The Record-Replay system intercepts all Slang COM interface calls, serializes them to a
binary stream, and can replay them later to reproduce exact API call sequences. This is
useful for:

- **Bug reproduction**: Capture a failing scenario and replay it for debugging
- **Determinism testing**: Verify that replaying calls produces identical results
- **Regression testing**: Compare recorded behavior across code changes

## Architecture

### Core Components

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                          │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Proxy Layer (proxy/*.h)                      │
│  GlobalSessionProxy, SessionProxy, ModuleProxy, etc.            │
│  - Intercepts all COM interface calls                           │
│  - Records inputs/outputs via RECORD_* macros                   │
│  - Delegates to actual Slang implementation                     │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   ReplayContext (replay-context.h)               │
│  - Global singleton managing record/replay state                │
│  - Type-aware serialization (record() overloads)                │
│  - Object handle tracking (COM interface → uint64_t)            │
│  - Proxy ↔ implementation mapping                               │
│  - Playback dispatcher (signature → handler)                    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ReplayStream (replay-stream.h)                │
│  - Memory-backed binary stream                                  │
│  - Optional disk mirroring for crash safety                     │
│  - File loading for playback                                    │
└─────────────────────────────────────────────────────────────────┘
```

### File Organization

```
source/slang-record-replay/
├── DESIGN.md                    # This document
├── WORKFLOW.md                  # Iterative proxy implementation workflow
├── replay-context.h/cpp         # Core serialization and state management
├── replay-stream.h              # Binary stream implementation
├── replay-stream-decoder.h/cpp  # Human-readable stream decoding
├── replay-stream-logger.h       # (EMPTY — planned but not implemented)
├── replay-shared.h              # Ref-count suppression, canonical identity helpers
├── replay-handlers.cpp          # Playback handler registration
└── proxy/
    ├── proxy-base.h/cpp         # Base class for all proxies + wrapObject/unwrapObject
    ├── proxy-macros.h           # Recording macros + playback registration infrastructure
    ├── proxy-global-session.h   # IGlobalSession proxy
    ├── proxy-session.h          # ISession proxy
    ├── proxy-module.h           # IModule proxy
    ├── proxy-component-type.h   # IComponentType proxy
    ├── proxy-entry-point.h      # IEntryPoint proxy
    ├── proxy-compile-request.h  # ICompileRequest proxy
    ├── proxy-compile-result.h   # ICompileResult proxy (all stubs)
    ├── proxy-metadata.h         # IMetadata proxy (all stubs)
    ├── proxy-type-conformance.h # ITypeConformance proxy (all stubs)
    ├── proxy-shared-library.h   # ISlangSharedLibrary proxy
    └── proxy-mutable-file-system.h/cpp  # ISlangMutableFileSystem proxy (fully implemented)

tools/slang-replay/
└── main.cpp                     # Command-line replay utility

tools/slang-unit-test/
├── unit-test-replay-serialization.cpp  # Type round-trip tests
├── unit-test-replay-record.cpp         # End-to-end recording tests (child process)
├── unit-test-replay-playback.cpp       # Playback dispatcher tests
├── unit-test-replay-modes.cpp          # Mode/state machine tests
├── unit-test-replay-integration.cpp    # Byte-level stream verification
├── unit-test-replay-handles.cpp        # Blob + TypeReflection serialization
└── unit-test-replay-filesystem.cpp     # MutableFileSystemProxy tests
```

## Operating Modes

The `ReplayContext` operates in one of four modes:

| Mode | Description |
|------|-------------|
| **Idle** | No operations performed (default when env var not set) |
| **Record** | Writing API calls to the stream |
| **Sync** | Recording while verifying against a reference stream |
| **Playback** | Reading and executing calls from a stream |

## Activation

Recording is enabled via environment variables:

```bash
# Enable recording
SLANG_RECORD_LAYER=1

# Custom recording path (optional)
SLANG_RECORD_PATH=/path/to/recording

# Enable live TTY logging (optional)
SLANG_RECORD_LOG=1
```

It can also be enabled programmatically:

```cpp
slang_enableRecordLayer(true);  // Enable
slang_enableRecordLayer(false); // Disable
```

When enabled, recordings are saved to timestamped folders under `.slang-replays/`:
```
.slang-replays/
└── 2026-02-04_14-30-45-123/
    ├── stream.bin    # Main call data stream
    ├── index.bin     # Call index for quick navigation
    └── files/        # Content-addressed blob storage (SHA1 hashes)
```

## Call Index Stream

The `index.bin` file is a secondary stream written alongside `stream.bin` during
recording. It contains fixed-size entries that allow quick navigation through a replay
without parsing the entire main stream.

### Index Entry Format

Each entry is a `CallIndexEntry` struct (fixed 144 bytes):

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `streamPosition` | uint64_t | 8 bytes | Byte offset in stream.bin where call begins |
| `thisHandle` | uint64_t | 8 bytes | Handle of 'this' pointer (0 for static calls) |
| `signature` | char[] | 128 bytes | Null-terminated function signature |

### Usage

```cpp
// Load a replay with index
ctx.loadReplay("/path/to/recording");

// Get total number of calls
size_t count = ctx.getCallCount();

// Get info about a specific call
const CallIndexEntry* entry = ctx.getCallIndexEntry(5);
printf("Call 5: %s at offset %llu\n", entry->signature, entry->streamPosition);

// Skip to a specific call
ctx.seekToCall(10);  // Position stream at call #10
ctx.executeNextCall();
```

## Proxy Pattern

### ProxyBase Template

All proxies inherit from `ProxyBase<TFirstInterface, TRestInterfaces...>`:

```cpp
template<typename TFirstInterface, typename... TRestInterfaces>
class ProxyBase : public TFirstInterface, public TRestInterfaces..., public RefObject
{
public:
    explicit ProxyBase(ISlangUnknown* actual);

    // ISlangUnknown: ref counting, queryInterface
    // queryInterface delegates to the underlying object to check support,
    // but always returns 'this' (the proxy) on success.

    template<typename T>
    T* getActual() const;  // Access underlying implementation (via queryInterface)

protected:
    ComPtr<ISlangUnknown> m_actual;
};
```

### Object Wrapping

The `wrapObject()` and `unwrapObject()` functions in `proxy-base.cpp` manage proxy
creation:

```cpp
ISlangUnknown* wrapObject(ISlangUnknown* obj);   // Wrap implementation in proxy (for outputs)
ISlangUnknown* unwrapObject(ISlangUnknown* proxy); // Unwrap proxy to implementation (for inputs)
```

Wrapping order matters due to inheritance — more derived types are checked first
(e.g., `IModule` before `IComponentType`).

### Canonical Identity (replay-shared.h)

COM objects can be reached through multiple interface pointers due to multiple
inheritance. The `toSlangUnknown()` helper calls `queryInterface(ISlangUnknown)` to get
a canonical pointer, used as the key for handle tracking. `SuppressRefCountRecording`
suppresses ref-count recording during these internal identity queries.

## Recording Macros

### Core Macros (proxy-macros.h)

```cpp
RECORD_CALL()                    // Begin a method call (locks ctx, records signature + 'this')
RECORD_STATIC_CALL()             // Begin a free/static function call
RECORD_INPUT(arg)                // Record input parameter
RECORD_INPUT_ARRAY(arr, count)   // Record input array
RECORD_INFO(arg)                 // Record informational data (not input/output)
RECORD_OUTPUT(arg)               // Record non-COM output (T* style)
PREPARE_POINTER_OUTPUT(arg)      // Create temp if pointer is null
RECORD_COM_OUTPUT(arg)           // Record COM output (T** style, wraps + records)
RECORD_BLOB_OUTPUT(arg)          // Record blob output (by content hash)
RECORD_COM_RESULT(result)        // Wrap COM result and record it
RECORD_RETURN(result)            // Record return value and return
RECORD_RETURN_VOID()             // No-op for void returns
PROXY_REFCOUNT_IMPL(ProxyType)   // addRef/release overrides that record
```

### Higher-Level Macros

```cpp
RECORD_METHOD_OUTPUT(method, outParam, inputs...)  // SlangResult method(inputs..., T** out)
RECORD_METHOD_RETURN(method, inputs...)            // T method(inputs...)
RECORD_METHOD_VOID(method, inputs...)              // void method(inputs...)
RECORD_METHOD_RETURN_NOARGS(method)                // T method()
```

### Playback Registration

```cpp
REPLAY_REGISTER(ProxyType, methodName)   // Register handler in replay-handlers.cpp
```

This uses `MemberFunctionTraits` and `callMethodWithDefaults` to invoke the proxy method
with default-initialized arguments — the proxy reads actual values from the stream during
the call.

### Example Usage

```cpp
virtual SLANG_NO_THROW SlangResult SLANG_MCALL
createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override
{
    RECORD_CALL();
    RECORD_INPUT(desc);
    auto result = getActual<slang::IGlobalSession>()->createSession(desc, outSession);
    RECORD_COM_OUTPUT(outSession);
    RECORD_RETURN(result);
}
```

## Serialization Format

### Type IDs

Each serialized value is prefixed with a `TypeId` byte:

| TypeId | Value | Description |
|--------|-------|-------------|
| Int8-Int64 | 0x01-0x04 | Signed integers |
| UInt8-UInt64 | 0x05-0x08 | Unsigned integers |
| Float32, Float64 | 0x09-0x0A | Floating point |
| Bool | 0x0B | Boolean |
| String | 0x10 | Null-terminated string with length prefix |
| Blob | 0x11 | Content-hash reference (blob stored in files/ directory) |
| Array | 0x12 | Count + elements |
| ObjectHandle | 0x13 | COM interface as uint64_t handle |
| Null | 0x14 | Null pointer |
| TypeReflectionRef | 0x15 | Type reflection reference (module handle + type name) |
| Error | 0xEE | Error marker |

### Handle System

COM interface pointers are mapped to uint64_t handles:

| Handle Value | Meaning |
|--------------|---------|
| 0 (kNullHandle) | Null pointer |
| 2 (kCustomFileSystemHandle) | User-provided custom file system (not yet registered) |
| 3 (kDefaultFileSystemHandle) | Default file system |
| ≥256 (kFirstValidHandle) | Tracked object handle |

### Call Structure

Each recorded call has this structure:

```
[TypeId::String] [signature length] [signature bytes]
[TypeId::ObjectHandle] [this handle or 0 for static]
[input parameters...]
[output parameters...]
[TypeId::*] [return value]
```

## Blob Serialization

Blobs (`ISlangBlob*`) are NOT tracked as COM objects with handles. Instead they use
content-addressed storage:

1. **Recording**: Compute SHA1 hash of blob content, write content to
   `<replay-dir>/files/<hash>`, record the hash string in the stream.
2. **Playback**: Read hash from stream, load content from `files/<hash>`, create a
   new `RawBlob`.

This deduplicates identical blobs and avoids the need for blob proxy objects.

## Playback System

### Handler Registration

Handlers are registered in `replay-handlers.cpp` via static initialization:

```cpp
// For proxy methods (auto-generated handler)
REPLAY_REGISTER(GlobalSessionProxy, createSession);
REPLAY_REGISTER(SessionProxy, loadModule);

// For static functions (manual handler)
ReplayContext::get().registerHandler("slang_createGlobalSession2",
                                     handle_slang_createGlobalSession2);
```

### Execution

```cpp
ctx.loadReplay("/path/to/recording");
ctx.executeAll();

// Or step-by-step:
while (ctx.hasMoreCalls())
    ctx.executeNextCall();
```

### Playback Dispatch Flow

1. `executeNextCall()` reads the function signature and `this` handle from the stream
2. Looks up the registered `PlaybackHandler` by signature
3. Seeks the stream back to the call start
4. Calls the handler, which invokes the proxy method with default arguments
5. The proxy's `RECORD_CALL()` in playback mode reads the signature and `this` handle
6. Each `RECORD_INPUT()` reads the actual value from the stream
7. The actual Slang API is called with the deserialized arguments
8. Each `RECORD_COM_OUTPUT()` wraps the result and registers it
9. `RECORD_RETURN()` reads the expected return value from the stream

## TypeReflection Serialization

`TypeReflection*` pointers require special handling since they are not COM objects.
They are serialized as a reference to their owning module plus the type's full name:

```
[TypeId::TypeReflectionRef]
[module handle]       # Handle of the IModule that owns this type
[type name string]    # Full name from TypeReflection::getFullName()
```

During playback, the type is recovered by looking up the module from its handle and
calling `module->getLayout()->findTypeByName(typeName)`.

**Known limitation:** Finding the owning module uses internal compiler casts
(`Slang::as<DeclRefType>`, `Slang::asInternal()`), tightly coupling the proxy layer to
compiler internals. For builtin types like `float3`, a fallback iterates all registered
modules to find one whose layout contains the type.

## Adding New API Support

### Step 1: Create or Update Proxy

If the interface doesn't have a proxy, create one:

```cpp
class NewInterfaceProxy : public ProxyBase<slang::INewInterface>
{
public:
    explicit NewInterfaceProxy(slang::INewInterface* actual)
        : ProxyBase(actual) {}

    // Implement each method with recording...
};
```

### Step 2: Implement Methods with Recording

```cpp
virtual ReturnType SLANG_MCALL methodName(ArgType arg, OutType** out) override
{
    RECORD_CALL();
    RECORD_INPUT(arg);
    auto result = getActual<slang::INewInterface>()->methodName(arg, out);
    RECORD_COM_OUTPUT(out);
    RECORD_RETURN(result);
}
```

### Step 3: Register Wrapping (if new interface)

Add to `proxy-base.cpp` in the `wrapObject()` function:

```cpp
TRY_WRAP(slang::INewInterface, NewInterfaceProxy)
```

### Step 4: Register Replay Handler

Add to `replay-handlers.cpp` in `registerAllHandlers()`:

```cpp
REPLAY_REGISTER(NewInterfaceProxy, methodName);
```

### Step 5: Add Type Serialization (if needed)

If the method uses new struct types, add `record()` overloads:

```cpp
// Declaration in replay-context.h
SLANG_API void record(RecordFlag flags, NewStructType& value);

// Implementation in replay-context.cpp
void ReplayContext::record(RecordFlag flags, NewStructType& value)
{
    record(flags, value.field1);
    record(flags, value.field2);
}
```

## Testing

### Unit Tests

Located in `tools/slang-unit-test/`:

| File | Coverage |
|------|----------|
| `unit-test-replay-serialization.cpp` | Round-trip tests for all basic types, enums, structs |
| `unit-test-replay-record.cpp` | End-to-end recording via child process launch of examples |
| `unit-test-replay-playback.cpp` | Playback dispatcher, handler registration, `parseSignature` |
| `unit-test-replay-modes.cpp` | Mode transitions, Idle/Record/Sync/Playback state machine |
| `unit-test-replay-integration.cpp` | Byte-level stream verification of actual API calls |
| `unit-test-replay-handles.cpp` | Blob hash serialization, null handles, TypeReflection |
| `unit-test-replay-filesystem.cpp` | All MutableFileSystemProxy methods |

### Running Tests

```bash
# Build
cmake --workflow --preset debug

# Run all replay tests
./build/Debug/bin/slang-test.exe "slang-unit-test-tool/replay"
```

## slang-replay Command-Line Tool

The `slang-replay` tool (`tools/slang-replay/main.cpp`) provides a command-line
interface for working with recorded API sessions.

### Usage

```bash
slang-replay [options] <path>
```

### Options

| Option | Short | Description |
|--------|-------|-------------|
| `--decode` | `-d` | Decode recording to human-readable text |
| `--raw` | `-R` | Force raw value-by-value output (ignore index.bin) |
| `--replay` | `-r` | Execute the recorded API calls |
| `--verbose` | `-v` | Enable verbose output during replay |
| `--output <file>` | `-o` | Write decoded output to file |
| `--convert-json` | `-cj` | Convert record file to JSON format |
| `--help` | `-h` | Show usage information |

### Examples

```bash
# Decode with structured call-by-call output (recommended)
slang-replay -d .slang-replays/2026-02-04_14-30-45-123/

# Decode with raw value-by-value output
slang-replay -d -R .slang-replays/2026-02-04_14-30-45-123/

# Replay a recording
slang-replay -r .slang-replays/2026-02-04_14-30-45-123/

# Replay with verbose logging
slang-replay -r -v .slang-replays/2026-02-04_14-30-45-123/
```

### Input Formats

The tool accepts either:
- A folder containing `stream.bin` (e.g., `.slang-replays/2026-02-04_14-30-45-123/`)
- A direct path to a `stream.bin` file

## Thread Safety

The `ReplayContext` uses a recursive mutex. All recording operations acquire the lock
via `RECORD_CALL()` which calls `ctx.lock()` and stores the RAII guard.

## Debugging Tips

### Enable TTY Logging

Set `SLANG_RECORD_LOG=1` to see API calls as they're recorded:

```
[REPLAY] GlobalSessionProxy::createSession [this=0x..., handle=256]
[REPLAY] SessionProxy::loadModule [this=0x..., handle=258]
```

### Common Issues

1. **Unimplemented proxy method**: Throws `SLANG_UNIMPLEMENTED_X` at runtime —
   method needs recording support added to the proxy
2. **Handle not found**: Object wasn't properly registered during recording —
   throws `HandleNotFoundException`
3. **Type mismatch**: Stream contains different type than expected —
   throws `TypeMismatchException`

---

## Current State: Proxy Implementation Coverage

The following tables document which proxy methods are implemented vs. stubbed
(`REPLAY_UNIMPLEMENTED_X`). **This is the key area requiring ongoing work.**

### GlobalSessionProxy — partially implemented

| Status | Methods |
|--------|---------|
| Implemented | `createSession`, `findProfile`, `setDefaultDownstreamCompiler`, `getDefaultDownstreamCompiler`, `setLanguagePrelude`, `getLanguagePrelude`, `createCompileRequest`, `checkCompileTargetSupport`, `checkPassThroughSupport`, `findCapability`, `setDownstreamCompilerForTransition`, `getDownstreamCompilerForTransition`, `getCompilerElapsedTime`, `parseCommandLineArguments` |
| Pass-through | `getBuildTagString` (not recorded) |
| Stubbed | `setDownstreamCompilerPath`, `setDownstreamCompilerPrelude`, `getDownstreamCompilerPrelude`, `addBuiltins`, `setSharedLibraryLoader`, `getSharedLibraryLoader`, `compileCoreModule`, `loadCoreModule`, `saveCoreModule`, `setSPIRVCoreGrammar`, `getSessionDescDigest`, `compileBuiltinModule`, `loadBuiltinModule`, `saveBuiltinModule` |

### SessionProxy — partially implemented

| Status | Methods |
|--------|---------|
| Implemented | `getGlobalSession`, `loadModule`, `loadModuleFromSource`, `createCompositeComponentType`, `getTypeLayout`, `getTypeConformanceWitnessSequentialID`, `createTypeConformanceComponentType`, `loadModuleFromIRBlob`, `getLoadedModuleCount`, `getLoadedModule`, `isBinaryModuleUpToDate`, `loadModuleFromSourceString`, `loadModuleInfoFromIRBlob`, `addSearchPath` |
| Stubbed | `specializeType`, `getContainerType`, `getDynamicType`, `getTypeRTTIMangledName`, `getTypeConformanceWitnessMangledName`, `createCompileRequest`, `getDynamicObjectRTTIBytes` |

### ModuleProxy — partially implemented

| Status | Methods |
|--------|---------|
| Implemented | `getLayout`, `getSpecializationParamCount`, `specialize`, `link`, `getTargetCode`, `findEntryPointByName`, `getDefinedEntryPointCount`, `getDefinedEntryPoint`, `serialize`, `getName`, `getFilePath`, `findAndCheckEntryPoint`, `getModuleReflection` |
| Stubbed | `getSession`, `getEntryPointCode`, `getResultAsFileSystem`, `getEntryPointHash`, `getEntryPointHostCallable`, `renameEntryPoint`, `linkWithOptions`, `getTargetMetadata`, `getEntryPointMetadata`, `writeToFile`, `getUniqueIdentity`, `getDependencyFileCount`, `getDependencyFilePath`, `disassemble`, `getTargetCompileResult`, `getEntryPointCompileResult`, `getTargetHostCallable`, `precompileForTarget`, `getPrecompiledTargetCode`, `getModuleDependencyCount`, `getModuleDependency` |

### ComponentTypeProxy — partially implemented

| Status | Methods |
|--------|---------|
| Implemented | `getSession`, `getLayout`, `getSpecializationParamCount`, `getEntryPointCode`, `getEntryPointHash`, `specialize`, `link`, `getEntryPointHostCallable`, `linkWithOptions`, `getTargetCode`, `getTargetMetadata`, `getEntryPointCompileResult` |
| Stubbed | `getResultAsFileSystem`, `renameEntryPoint`, `getEntryPointMetadata`, `getTargetCompileResult`, `getTargetHostCallable`, `precompileForTarget`, `getPrecompiledTargetCode`, `getModuleDependencyCount`, `getModuleDependency` |

### EntryPointProxy — mostly stubbed

| Status | Methods |
|--------|---------|
| Implemented | `getLayout`, `getSpecializationParamCount`, `specialize`, `link`, `getFunctionReflection` |
| Stubbed | `getSession`, `getEntryPointCode`, `getResultAsFileSystem`, `getEntryPointHash`, `getEntryPointHostCallable`, `renameEntryPoint`, `linkWithOptions`, `getTargetCode`, `getTargetMetadata`, `getEntryPointMetadata`, `getTargetCompileResult`, `getEntryPointCompileResult`, `getTargetHostCallable`, `precompileForTarget`, `getPrecompiledTargetCode`, `getModuleDependencyCount`, `getModuleDependency` |

### CompileRequestProxy — partially implemented

Largest proxy with the most methods. Many setter/getter methods for compile options.
Approximately 35 methods implemented, ~40 stubbed.

### CompileResultProxy / MetadataProxy / TypeConformanceProxy — all stubs

**All methods stubbed.** These are pure stub proxies for type-system compatibility.

### SharedLibraryProxy — mostly stubbed

Only `findSymbolAddressByName` is implemented. `castAs` is stubbed.

### MutableFileSystemProxy — fully implemented

All `ISlangFileSystem`, `ISlangFileSystemExt`, and `ISlangMutableFileSystem` methods
have recording/playback support.

---

## Known Bugs

1. **`replay-handlers.cpp` registers handlers for methods that are
   `REPLAY_UNIMPLEMENTED_X` in the proxy.** For example, many `CompileRequestProxy`
   setters and `GlobalSessionProxy` methods are registered via `REPLAY_REGISTER` but
   will throw at runtime when called during playback. The handler registration should
   only include methods that are actually implemented. Mismatches between
   `replay-handlers.cpp` and the proxy headers cause confusing runtime errors during
   playback.

2. **Inconsistent recording in some proxy methods:**
   - `ModuleProxy::getLayout()` and `EntryPointProxy::getLayout()` return raw
     `ProgramLayout*` without recording the return value. If playback depends on
     these results, it will diverge.
   - `EntryPointProxy::getFunctionReflection()` returns a raw pointer without recording.
   - Some methods use `RECORD_INFO()` + manual `return` instead of `RECORD_RETURN()`.
   These inconsistencies mean certain methods can record data during recording but
   produce a stream that can't be played back, because the playback flow expects the
   stream to contain data that was never written.

3. **`notifyDestroyed()` unconditionally prints to stderr** — should be gated behind
   `m_ttyLogging`.

4. **`unit-test-replay-record.cpp` uses hardcoded hashes** that include machine-specific
   file paths. These tests will fail on any machine with different paths.

---

## Proposed Refinements

The following refinements are prioritized by the goals of **simplicity**, **ease of
maintenance**, **minimal 'clever C++'**, and **consistent COM usage**.

### P0: Correctness Fixes

1. **Audit `replay-handlers.cpp` against actual proxy implementations.** Remove
   `REPLAY_REGISTER` entries for methods that are `REPLAY_UNIMPLEMENTED_X`. A mismatch
   here means playback will dispatch to a method that immediately throws. A simple
   grep can verify 1:1 correspondence.

2. **Fix inconsistent recording patterns.** Methods that return values must use
   `RECORD_RETURN()` consistently. Create a checklist of every implemented proxy method
   and verify each follows one of the standard patterns:
   - `RECORD_CALL()` + `RECORD_INPUT(s)` + call + `RECORD_COM_OUTPUT(s)` + `RECORD_RETURN()`
   - `RECORD_METHOD_OUTPUT` / `RECORD_METHOD_RETURN` / `RECORD_METHOD_VOID` macros

3. **Gate `notifyDestroyed()` stderr output behind `m_ttyLogging`.** Currently it
   always prints, polluting stderr in production builds.

### P1: Reduce Template Complexity in proxy-macros.h

The playback infrastructure (`MemberFunctionTraits`, `callWithDefaults`,
`DefaultValue<T>` specializations, `replayHandler` template, `REPLAY_REGISTER` macro)
is the most "clever C++" in the system. It uses variadic templates, `std::tuple`,
`std::index_sequence`, `if constexpr`, and SFINAE-style specializations to auto-generate
playback handlers from method pointer types.

**Problem:** This works, but is hard to debug, hard to extend (e.g., when a method
has a non-default-constructible parameter), and produces opaque compiler errors when
something goes wrong.

**Proposed simplification:** Replace the auto-generated playback dispatch with explicit
per-method handler lambdas. Each handler would be ~3 lines of straightforward code:

```cpp
// Instead of REPLAY_REGISTER(GlobalSessionProxy, findProfile):
ctx.registerHandler("GlobalSessionProxy::findProfile", [](ReplayContext& ctx) {
    auto* proxy = ctx.getCurrentThis<GlobalSessionProxy>();
    const char* name = nullptr;  // will be read from stream by RECORD_INPUT
    proxy->findProfile(name);    // proxy reads from stream via RECORD_INPUT
});
```

This eliminates all of `MemberFunctionTraits`, `DefaultValue`, `callWithDefaults`,
and the `replayHandler` template (~100 lines of dense template code), replacing it
with simple, greppable, debuggable lambdas. The tradeoff is slightly more boilerplate
per method, but each handler is self-documenting and trivially debuggable.

### P2: Consistent COM Usage

The proxy layer mixes COM patterns inconsistently:

- `ProxyBase` inherits from `RefObject` (Slang's ref-counting base) in addition to COM
  interfaces, creating a dual ref-counting path.
- `toSlangUnknown()` / `toSlangInterface()` manually call `queryInterface` then
  immediately `release()`, which is a COM anti-pattern (the returned pointer could
  theoretically be dangled by a concurrent release on another thread, though the
  mutex protects against this in practice).
- `ProxyBase::~ProxyBase()` uses a `static_cast` chain instead of `queryInterface`
  because the ref count is already zero — correct but documents a design tension.
- `MutableFileSystemProxy` has `PROXY_REFCOUNT_IMPL` commented out, with lifetime
  managed externally.

**Proposed:** Document and enforce a single ownership model. If proxies use COM
ref-counting, they should do so consistently. The `toSlangUnknown()` "borrow" pattern
(QI + immediate release) should be documented as intentional and safe only under the
mutex, or replaced with a pattern that doesn't involve transient ownership transfer.

### P3: Delete Dead Code

- **`replay-stream-logger.h`** is empty. Delete it.
- **`kInlineBlobHandle` (handle value 1)** — if defined anywhere, it's unused. Blobs
  are serialized by content hash, not by handle. If this was part of an earlier design,
  remove the constant.

### P4: Simplify the Test Suite

- **`unit-test-replay-record.cpp` hardcoded hashes** are machine-specific (they
  include file paths in the decoded output). Either: (a) don't test the decoded-text
  hash at all (just verify a non-empty decode), or (b) hash only the structural parts
  (signatures, types) while ignoring path-dependent data.
- **Temp file cleanup in filesystem tests** should use RAII guards to ensure cleanup
  on test failure, not just at the happy-path end.
- **Cleanup in record tests** is commented out (`cleanupRecordFiles`). Either enable it
  or document why it's disabled.

### P5: Future — Reduce the Proxy Stub Surface

Many proxy classes are pure stubs (`CompileResultProxy`, `MetadataProxy`,
`TypeConformanceProxy`). For methods that are never called in practice, the
`REPLAY_UNIMPLEMENTED_X` macro provides a clear error. But the large number of stubs
makes it hard to know what's actually working.

**Proposed:** Track coverage by running the slang test suite with replay enabled
and collecting which `REPLAY_UNIMPLEMENTED_X` methods are actually hit. Prioritize
implementing only those methods, rather than trying to achieve full API coverage
upfront. See `WORKFLOW.md` for the iterative approach.
