# Slang Record-Replay System Design

This document describes the architecture and design of the Slang Record-Replay system, which enables capturing and replaying Slang API calls for debugging, testing, and determinism verification.

## Overview

The Record-Replay system intercepts all Slang COM interface calls, serializes them to a binary stream, and can replay them later to reproduce exact API call sequences. This is useful for:

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
├── replay-context.h/cpp         # Core serialization and state management
├── replay-stream.h              # Binary stream implementation
├── replay-stream-decoder.h/cpp  # Human-readable stream decoding
├── replay-handlers.cpp          # Playback handler registration
└── proxy/
    ├── proxy-base.h/cpp         # Base class for all proxies
    ├── proxy-macros.h           # Recording macros (RECORD_CALL, etc.)
    ├── proxy-global-session.h   # IGlobalSession proxy
    ├── proxy-session.h          # ISession proxy
    ├── proxy-module.h           # IModule proxy
    ├── proxy-component-type.h   # IComponentType proxy
    ├── proxy-entry-point.h      # IEntryPoint proxy
    ├── proxy-compile-request.h  # ICompileRequest proxy
    ├── proxy-blob.h             # ISlangBlob proxy
    └── ...                      # Other interface proxies

tools/slang-replay/
└── main.cpp                     # Command-line replay utility
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

When enabled, recordings are saved to timestamped folders under `.slang-replays/`:
```
.slang-replays/
└── 2026-02-04_14-30-45-123/
    └── stream.bin
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
    
    // ISlangUnknown implementation (ref counting, queryInterface)
    
    template<typename T>
    T* getActual() const;  // Access underlying implementation
    
protected:
    ComPtr<ISlangUnknown> m_actual;
};
```

### Object Wrapping

The `wrapObject()` and `unwrapObject()` functions manage proxy creation:

```cpp
// Wrap an implementation in a proxy (for outputs)
ISlangUnknown* wrapObject(ISlangUnknown* obj);

// Unwrap a proxy to get the implementation (for inputs)
ISlangUnknown* unwrapObject(ISlangUnknown* proxy);
```

Wrapping order matters due to inheritance - more derived types are checked first.

## Recording Macros

### Core Macros

```cpp
// Begin a method call - records signature and 'this' pointer
RECORD_CALL()

// Record input parameters
RECORD_INPUT(arg)

// Record COM output parameters (T** style)
RECORD_COM_OUTPUT(outParam)

// Record return value
RECORD_RETURN(result)
RECORD_RETURN_VOID()
```

### Higher-Level Macros

```cpp
// Pattern: SlangResult method(inputs..., T** outObject)
RECORD_METHOD_OUTPUT(method, outParam, inputs...)

// Pattern: T method(inputs...) - with return value
RECORD_METHOD_RETURN(method, inputs...)

// Pattern: void method(inputs...) - void methods
RECORD_METHOD_VOID(method, inputs...)

// Pattern: T method() - no args
RECORD_METHOD_RETURN_NOARGS(method)
```

### Example Usage

```cpp
virtual SLANG_NO_THROW SlangResult SLANG_MCALL
createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override
{
    RECORD_CALL();                                           // Begin call
    RECORD_INPUT(desc);                                      // Record input
    auto result = getActual<slang::IGlobalSession>()->createSession(desc, outSession);
    RECORD_COM_OUTPUT(outSession);                           // Wrap & record output
    RECORD_RETURN(result);                                   // Record return value
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
| Blob | 0x11 | Binary data with size prefix |
| Array | 0x12 | Count + elements |
| ObjectHandle | 0x13 | COM interface as uint64_t handle |
| Null | 0x14 | Null pointer |
| Error | 0xEE | Error marker |

### Handle System

COM interface pointers are mapped to uint64_t handles:

| Handle Value | Meaning |
|--------------|---------|
| 0 (kNullHandle) | Null pointer |
| 1 (kInlineBlobHandle) | User-provided blob (data serialized inline) |
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

## Playback System

### Handler Registration

Handlers are registered in `replay-handlers.cpp`:

```cpp
// For proxy methods
REPLAY_REGISTER(GlobalSessionProxy, createSession);
REPLAY_REGISTER(SessionProxy, loadModule);

// For static functions (manual handler)
ReplayContext::get().registerHandler("slang_createGlobalSession2", handle_slang_createGlobalSession2);
```

### Execution

```cpp
// Load a recording
ctx.loadReplay("/path/to/recording");

// Execute all calls
ctx.executeAll();

// Or execute one at a time
while (ctx.hasMoreCalls()) {
    ctx.executeNextCall();
}
```

## Adding New API Support

### Step 1: Create or Update Proxy

If the interface doesn't have a proxy, create one:

```cpp
class NewInterfaceProxy : public ProxyBase<slang::INewInterface>
{
public:
    SLANG_COM_INTERFACE(/* unique GUID */)
    
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

Add to `proxy-base.cpp`:

```cpp
TRY_WRAP(slang::INewInterface, NewInterfaceProxy)
```

### Step 4: Register Replay Handler

Add to `replay-handlers.cpp`:

```cpp
REPLAY_REGISTER(NewInterfaceProxy, methodName);
```

### Step 5: Add Type Serialization (if needed)

If the method uses new struct types, add `record()` overloads in `replay-context.h/cpp`:

```cpp
// Declaration in header
SLANG_API void record(RecordFlag flags, NewStructType& value);

// Implementation in cpp
void ReplayContext::record(RecordFlag flags, NewStructType& value)
{
    record(flags, value.field1);
    record(flags, value.field2);
    // ...
}
```

## Testing

### Unit Tests

Located in `tools/slang-unit-test/`:

- `unit-test-replay-context.cpp` - Tests serialization round-trips
- `unit-test-record-replay.cpp` - Tests end-to-end recording/replay

### Running Tests

```bash
# Build
cmake --workflow --preset debug

# Run record-replay tests
./build/Debug/bin/slang-test.exe "slang-unit-test-tool/RecordReplay"
./build/Debug/bin/slang-test.exe "slang-unit-test-tool/replayContext"
```

## slang-replay Command-Line Tool

The `slang-replay` tool (`tools/slang-replay/main.cpp`) provides a command-line interface for working with recorded API sessions.

### Building

The tool is built as part of the standard Slang build:

```bash
cmake --workflow --preset debug
# Binary is at: build/Debug/bin/slang-replay.exe
```

### Usage

```bash
slang-replay [options] <record-file>
```

### Options

| Option | Short | Description |
|--------|-------|-------------|
| `--decode` | `-d` | Decode binary stream.bin to human-readable text |
| `--replay` | `-r` | Execute the recorded API calls |
| `--verbose` | `-v` | Enable verbose output during replay |
| `--output <file>` | `-o` | Write decoded output to file instead of stdout |
| `--convert-json` | `-cj` | Convert record file to JSON format |
| `--help` | `-h` | Show usage information |

### Examples

```bash
# Decode a recording to see what API calls were made
slang-replay -d .slang-replays/2026-02-04_14-30-45-123/stream.bin

# Decode and save to file
slang-replay -d -o calls.txt .slang-replays/2026-02-04_14-30-45-123/stream.bin

# Replay a recording (execute all recorded API calls)
slang-replay -r .slang-replays/2026-02-04_14-30-45-123/

# Replay with verbose logging
slang-replay -r -v .slang-replays/2026-02-04_14-30-45-123/
```

### Input Formats

The tool accepts either:
- A folder containing `stream.bin` (e.g., `.slang-replays/2026-02-04_14-30-45-123/`)
- A direct path to the `stream.bin` file

## Debugging Tips

### Enable TTY Logging

Set `SLANG_RECORD_LOG=1` to see API calls as they're recorded:

```
[RECORD] GlobalSessionProxy::createSession this=0x100
[RECORD] SessionProxy::loadModule this=0x101
```

### Decode Stream Files

Use `ReplayStreamDecoder` to inspect recordings:

```cpp
String decoded = ReplayStreamDecoder::decodeFile("stream.bin");
printf("%s\n", decoded.getBuffer());
```

### Common Issues

1. **Unimplemented proxy method**: If a method throws `SLANG_UNIMPLEMENTED_X`, it needs recording support
2. **Handle not found**: Object wasn't properly registered during recording
3. **Type mismatch**: Stream contains different type than expected (version mismatch?)

## Thread Safety

The `ReplayContext` uses a recursive mutex. All recording operations acquire the lock via `RECORD_CALL()` which calls `ctx.lock()`.

## Future Enhancements

- [ ] Full replay verification (compare outputs during replay)
- [ ] Differential recording (only record changes)
- [ ] Network transport for remote debugging
- [ ] Compression for large recordings
- [ ] More complete API coverage
