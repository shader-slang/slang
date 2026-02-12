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

## Unit Tests

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

## Thread Safety

The `ReplayContext` uses a recursive mutex. All recording operations acquire the lock
via `RECORD_CALL()` which calls `ctx.lock()` and stores the RAII guard.
