# Slang Record/Replay System

## Overview

The Slang Record/Replay system captures Slang API calls made by an application during execution, serializing them to a binary stream that can later be replayed independently of the original application. This enables:

- **Reproducible bug reports**: Users can record a session and share it with developers
- **Determinism testing**: Verify that the same API calls produce the same results
- **Performance profiling**: Replay API calls without application overhead
- **Debugging**: Step through API calls in isolation

## Architecture

The system consists of three main components:

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application                               │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Proxy Layer                                 │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐               │
│  │GlobalSession│ │   Session   │ │   Module    │  ...          │
│  │   Proxy     │ │    Proxy    │ │    Proxy    │               │
│  └─────────────┘ └─────────────┘ └─────────────┘               │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     ReplayContext                                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ ReplayStream │  │  Handle Map  │  │ MemoryArena  │          │
│  │ (binary I/O) │  │ (obj↔handle) │  │  (playback)  │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   Actual Slang Implementation                    │
└─────────────────────────────────────────────────────────────────┘
```

### 1. Proxy Layer (`proxy/`)

Each Slang COM interface has a corresponding proxy class that wraps the real implementation:

| Proxy Class | Wraps Interface |
|-------------|-----------------|
| `GlobalSessionProxy` | `slang::IGlobalSession` |
| `SessionProxy` | `slang::ISession` |
| `ModuleProxy` | `slang::IModule` |
| `EntryPointProxy` | `slang::IEntryPoint` |
| `ComponentTypeProxy` | `slang::IComponentType` |
| `CompileRequestProxy` | `slang::ICompileRequest` |
| `BlobProxy` | `ISlangBlob` |
| ... | ... |

Proxies inherit from `ProxyBase<TInterface, ...>`, a variadic template that:
- Implements `ISlangUnknown` ref-counting via `RefObject`
- Handles `queryInterface()` delegation to the underlying object
- Provides `getActual<T>()` to access the wrapped implementation

The `wrapObject()` function in `proxy-base.cpp` wraps any Slang COM interface in its appropriate proxy type, handling inheritance order correctly (derived before base).

### 2. ReplayContext

The central serialization engine that handles all recording and playback. It operates in one of four modes:

| Mode | Description |
|------|-------------|
| `Idle` | No operations (default when recording disabled) |
| `Record` | Writing API calls to the stream |
| `Sync` | Recording while verifying against a reference stream (determinism testing) |
| `Playback` | Reading and executing recorded API calls |

Key features:

#### Type-Tagged Serialization
Every value is prefixed with a `TypeId` tag for type safety:
```cpp
enum class TypeId : uint8_t {
    Int8, Int16, Int32, Int64,      // Signed integers
    UInt8, UInt16, UInt32, UInt64,  // Unsigned integers
    Float32, Float64, Bool,          // Primitives
    String, Blob, Array,             // Variable-length
    ObjectHandle, Null               // References
};
```

#### Handle-Based Object Tracking
COM interface pointers are serialized as stable 64-bit handles:
```cpp
constexpr uint64_t kNullHandle = 0;          // Null pointer
constexpr uint64_t kInlineBlobHandle = 1;    // Inline blob data
constexpr uint64_t kFirstValidHandle = 0x100; // First tracked object
```

The context maintains bidirectional maps:
- `m_objectToHandle`: `ISlangUnknown* → uint64_t`
- `m_handleToObject`: `uint64_t → ISlangUnknown*`

#### RecordFlag System
Flags indicate how each value should be handled during record/playback:
```cpp
enum class RecordFlag : uint8_t {
    None = 0,
    Input = 1 << 0,      // Verify on replay
    Output = 1 << 1,     // Capture on replay  
    InOut = Input|Output,
    ReturnValue = 1 << 2,
    ThisPtr = 1 << 3
};
```

#### Unified `record()` API
A single `record()` method handles both reading and writing based on mode:
```cpp
void record(RecordFlag flags, int32_t& value);
void record(RecordFlag flags, const char*& str);
void record(RecordFlag flags, slang::ISession*& obj);
void record(RecordFlag flags, slang::SessionDesc& desc);
// ... many overloads
```

### 3. ReplayStream

A simple memory-backed binary stream for efficient I/O:
- Writing: Appends to an in-memory buffer
- Reading: Reads from buffer with position tracking
- Optional file mirroring for crash-safe recording
- Can be loaded from/saved to disk

## Recording Workflow

### 1. Activation
Recording is activated via environment variable `SLANG_RECORD_LAYER=1` or programmatically:
```cpp
slang_enableRecordLayer(true);
```

### 2. Proxy Wrapping
When `slang_createGlobalSession2()` is called with recording enabled, the returned `IGlobalSession` is wrapped in a `GlobalSessionProxy`. Subsequent objects created through the API are also wrapped.

### 3. Method Recording
Each proxy method uses macros to record the call:

```cpp
// In GlobalSessionProxy::createSession()
virtual SlangResult createSession(const SessionDesc& desc, ISession** outSession) override
{
    RECORD_CALL();                              // Lock context, record signature + 'this'
    RECORD_INPUT(desc);                         // Record input parameters
    auto result = getActual()->createSession(desc, outSession);
    RECORD_OUTPUT(outSession);                  // Wrap and record output object
    RECORD_RETURN(result);                      // Record return value
}
```

The `RECORD_*` macros expand to:
```cpp
auto& _ctx = ReplayContext::get();
auto _lock = _ctx.lock();                       // Thread-safe locking
_ctx.beginCall(__FUNCSIG__, this);              // Record function signature + this handle
_ctx.record(RecordFlag::Input, desc);           // Serialize SessionDesc
auto result = getActual()->createSession(desc, outSession);
*outSession = wrapObject(*outSession);          // Wrap in proxy
_ctx.record(RecordFlag::Output, *outSession);   // Record output handle
_ctx.record(RecordFlag::ReturnValue, result);
return result;
```

### 4. Special Cases

#### Inline Blobs
User-provided `ISlangBlob` objects (e.g., shader source) are not tracked by handle. Instead, their contents are serialized inline:
```cpp
[kInlineBlobHandle][blob_size][blob_data...]
```

#### Complex Structs
Structures like `SessionDesc` have custom `record()` overloads that serialize each field:
```cpp
void ReplayContext::record(RecordFlag flags, slang::SessionDesc& value)
{
    record(flags, value.structureSize);
    recordArray(flags, value.targets, value.targetCount);
    record(flags, value.flags);
    // ... all fields
}
```

## Playback Workflow

### 1. Handler Registration
Before playback, handlers must be registered for each function signature:
```cpp
ctx.registerHandler("IGlobalSession::createSession(...)", 
    [](ReplayContext& ctx) {
        // Read signature (already consumed)
        // Read this handle
        auto* thisPtr = ctx.getCurrentThis<IGlobalSession>();
        
        // Read inputs
        SessionDesc desc;
        ctx.record(RecordFlag::Input, desc);
        
        // Execute call
        ISession* session;
        auto result = thisPtr->createSession(desc, &session);
        
        // Verify/record outputs
        ctx.record(RecordFlag::Output, session);
        ctx.record(RecordFlag::ReturnValue, result);
    });
```

### 2. Execution
```cpp
ctx.switchToPlayback();
while (ctx.executeNextCall()) {
    // Each call reads its signature, looks up handler, executes
}
```

### 3. Verification
During playback, output values and return values are compared against recorded data. Mismatches throw `DataMismatchException`.

## Binary Format

Each recorded call follows this structure:

```
┌─────────────────────────────────────────────────┐
│ String: Function signature (e.g., __FUNCSIG__)  │
├─────────────────────────────────────────────────┤
│ ObjectHandle: 'this' pointer handle             │
├─────────────────────────────────────────────────┤
│ Input parameters (type-tagged)                  │
├─────────────────────────────────────────────────┤
│ Output parameters (type-tagged)                 │
├─────────────────────────────────────────────────┤
│ Return value (type-tagged)                      │
└─────────────────────────────────────────────────┘
```

Each value is prefixed with its TypeId:
```
[TypeId:1 byte][Data:N bytes]
```

Variable-length types include size:
```
String: [TypeId::String][length:4][chars:length]
Blob:   [TypeId::Blob][size:8][data:size]
Array:  [TypeId::Array][count:8][elements...]
```

## Thread Safety

- `ReplayContext::lock()` returns a `std::unique_lock<std::recursive_mutex>`
- All proxy methods acquire the lock via `RECORD_CALL()`
- The recursive mutex allows nested API calls

## API Functions

Public C API for controlling recording:

```cpp
SLANG_API void slang_enableRecordLayer(bool enable);
SLANG_API bool slang_isRecordLayerEnabled();
SLANG_API void slang_getRecordLayerData(const void** outData, size_t* outSize);
SLANG_API void slang_clearRecordLayer();
SLANG_API void* slang_getReplayContext();  // For advanced use
```

## Files

| File | Purpose |
|------|---------|
| `replay-context.h` | Core serialization context |
| `replay-context.cpp` | Implementation of serialization |
| `replay-stream.h` | Binary stream I/O |
| `proxy/proxy-base.h` | Base template for proxy classes |
| `proxy/proxy-base.cpp` | `wrapObject()` implementation |
| `proxy/proxy-macros.h` | Recording macros |
| `proxy/proxy-*.h` | Individual proxy class definitions |

## Roadmap

The following sections describe the remaining work to complete the record/replay system, organized by priority and with proposed implementation approaches.

---

### Phase 1: Handler Registration & Testing Infrastructure

#### 1.1 Handler Registration Macro (Priority: HIGH)

**Goal**: Simple one-liner registration of playback handlers.

**Current State**: `registerHandler()` exists but registration is manual.

**Proposed Approach**:
```cpp
// In proxy-macros.h - define a registration macro
#define REPLAY_REGISTER(InterfaceType, methodName) \
    static struct _Replay_##InterfaceType##_##methodName##_Registrar { \
        _Replay_##InterfaceType##_##methodName##_Registrar() { \
            ReplayContext::get().registerHandler( \
                REPLAY_SIGNATURE(InterfaceType, methodName), \
                &InterfaceType##Proxy::replay_##methodName); \
        } \
    } _replay_registrar_##InterfaceType##_##methodName

// Usage in proxy header:
class GlobalSessionProxy {
    static void replay_createSession(ReplayContext& ctx);
};
REPLAY_REGISTER(GlobalSession, createSession);
```

**Implementation Steps**:
1. Add `REPLAY_SIGNATURE()` macro to generate consistent signature strings
2. Add `replay_*` static methods to proxy classes (mirror the instance methods)
3. Add `REPLAY_REGISTER()` macro with static initialization
4. Create a single `RegisterAllHandlers()` call or rely on static init

**Files**: `proxy-macros.h`, each `proxy-*.h`

---

#### 1.2 Testing Infrastructure (Priority: HIGH)

**Goal**: Verify the replay system works correctly without circular dependencies.

**Problem**: How do we verify playback produces correct results without using the system we're testing?

**Proposed Approach - Three-Phase Testing**:

```cpp
SLANG_UNIT_TEST(replayRoundTrip_createSession)
{
    // Phase 1: Baseline - no recording
    slang::IGlobalSession* baselineGlobal = nullptr;
    slang::ISession* baselineSession = nullptr;
    {
        slang_enableRecordLayer(false);
        slang_createGlobalSession2(&desc, &baselineGlobal);
        baselineGlobal->createSession(sessionDesc, &baselineSession);
        // Extract identifying info (pointer addresses, internal state hashes, etc.)
    }
    
    // Phase 2: Record
    slang::IGlobalSession* recordedGlobal = nullptr;
    slang::ISession* recordedSession = nullptr;
    {
        slang_enableRecordLayer(true);
        slang_createGlobalSession2(&desc, &recordedGlobal);
        recordedGlobal->createSession(sessionDesc, &recordedSession);
        
        // Verify proxy wrapping - the actual objects should match baseline behavior
        auto* proxy = static_cast<GlobalSessionProxy*>(recordedGlobal);
        auto* actualGlobal = proxy->getActual<slang::IGlobalSession>();
        SLANG_CHECK(verifyEquivalent(actualGlobal, baselineGlobal));
    }
    
    // Phase 3: Playback
    {
        auto& ctx = ReplayContext::get();
        ctx.switchToPlayback();
        ctx.executeAll();
        
        // Retrieve objects from the handle map
        auto* replayedGlobal = ctx.getInterfaceForHandle(0x100);
        auto* replayedSession = ctx.getInterfaceForHandle(0x101);
        
        // Verify they match baseline (compare internal state, not pointers)
        SLANG_CHECK(verifyEquivalent(replayedGlobal, baselineGlobal));
        SLANG_CHECK(verifyEquivalent(replayedSession, baselineSession));
    }
}
```

**Verification Helpers**:
```cpp
// Compare two sessions by checking their configuration matches
bool verifyEquivalent(slang::ISession* a, slang::ISession* b);

// Compare global sessions
bool verifyEquivalent(slang::IGlobalSession* a, slang::IGlobalSession* b);

// For modules: compare by name, file path, entry points
bool verifyEquivalent(slang::IModule* a, slang::IModule* b);
```

**Implementation Steps**:
1. Add `verifyEquivalent()` helpers for key interface types
2. Write round-trip tests for basic operations (createGlobalSession, createSession)
3. Expand to more complex operations (loadModule, compile)
4. Add sync-mode tests to verify determinism

**Files**: `unit-test-replay-context.cpp` (expand), new `unit-test-replay-roundtrip.cpp`

---

### Phase 2: TypeReflection Support

#### 2.1 TypeReflection Handle Mapping (Priority: MEDIUM-HIGH)

**Problem**: `TypeReflection*` is a raw pointer, not a COM interface. Cannot wrap in proxies.

**Key Insight**: `TypeReflection` can be cast to `DeclRefType`, which has a `DeclRef` containing module info and type path.

**Proposed Approach - Path-Based Identification**:
```cpp
// New TypeId for type reflection
enum class TypeId : uint8_t {
    // ... existing ...
    TypeReflectionPath = 0x20,  // Module handle + mangled name
};

// Recording a TypeReflection*
void ReplayContext::record(RecordFlag flags, slang::TypeReflection*& type)
{
    if (isWriting())
    {
        if (type == nullptr)
        {
            recordTypeId(TypeId::Null);
            return;
        }
        
        recordTypeId(TypeId::TypeReflectionPath);
        
        // Get the module this type belongs to
        auto* declRefType = (DeclRefType*)type;
        auto declRef = declRefType->getDeclRef();
        auto* module = declRef.getModule();
        
        // Record module handle (should already be registered)
        uint64_t moduleHandle = getHandleForInterface(module);
        recordHandle(flags, moduleHandle);
        
        // Record the mangled/qualified type name
        String typeName = getMangledTypeName(type);  // Helper to get unique name
        record(flags, typeName);
    }
    else  // Playback
    {
        expectTypeId(TypeId::TypeReflectionPath);
        
        // Read module handle
        uint64_t moduleHandle;
        recordHandle(flags, moduleHandle);
        auto* module = static_cast<slang::IModule*>(getInterfaceForHandle(moduleHandle));
        
        // Read type name
        const char* typeName = nullptr;
        record(flags, typeName);
        
        // Look up type in module's reflection
        type = lookupTypeByName(module, typeName);
    }
}

// Helper: Get a unique identifier for a type
String getMangledTypeName(slang::TypeReflection* type);

// Helper: Look up a type by name in a module
slang::TypeReflection* lookupTypeByName(slang::IModule* module, const char* name);
```

**Implementation Steps**:
1. Add `getMangledTypeName()` using existing Slang name mangling infrastructure
2. Add `lookupTypeByName()` using module's type system
3. Add `record()` overload for `TypeReflection*`
4. Test with specialize() calls that use TypeReflection

**Files**: `replay-context.h`, `replay-context.cpp`

---

### Phase 3: Command Index Stream

#### 3.1 Separate Index Stream (Priority: MEDIUM)

**Goal**: Fast seeking through recorded commands without parsing the entire data stream.

**Problem**: Currently must read entire stream to find a specific call.

**Proposed Format - Two-Stream Design**:
```
Main Stream (data.bin):
┌──────────────────────────────────────────────┐
│ Call 0: [signature][this][args...][result]   │ offset 0
├──────────────────────────────────────────────┤
│ Call 1: [signature][this][args...][result]   │ offset 847
├──────────────────────────────────────────────┤
│ Call 2: ...                                  │ offset 1203
└──────────────────────────────────────────────┘

Index Stream (index.bin):
┌────────────────────────────────────────────────────────────┐
│ Entry 0: [signature_hash:4][this_handle:8][offset:8][tid:4]│
├────────────────────────────────────────────────────────────┤
│ Entry 1: [signature_hash:4][this_handle:8][offset:8][tid:4]│
├────────────────────────────────────────────────────────────┤
│ ...                                                        │
└────────────────────────────────────────────────────────────┘
```

**Proposed API**:
```cpp
struct CommandIndexEntry {
    uint32_t signatureHash;  // Fast lookup without string compare
    uint64_t thisHandle;
    uint64_t dataOffset;     // Byte offset in main stream
    uint32_t threadId;       // For multithreaded filtering
};

class ReplayContext {
    ReplayStream m_indexStream;  // Separate from m_stream
    
    // Write index entry after each call
    void endCall(uint64_t callStartOffset);
    
    // Seek to a specific call by index
    void seekToCall(size_t callIndex);
    
    // Filter calls by thread
    void setThreadFilter(uint32_t threadId);
    
    // Get call count without parsing data
    size_t getCallCount() const;
};
```

**Implementation Steps**:
1. Add `m_indexStream` to ReplayContext
2. Add `endCall()` to write index entry (signature hash + this + offset + thread)
3. Modify `RECORD_CALL()` to save start offset, `RECORD_RETURN` to call endCall
4. Add seeking/filtering APIs

**Files**: `replay-context.h`, `replay-context.cpp`, `proxy-macros.h`

---

### Phase 4: File System Wrapper

#### 4.1 Recording File System (Priority: MEDIUM)

**Goal**: Capture all file access during recording, replay without actual files.

**Challenge**: User can set their own file system, which we must wrap while keeping our wrapper on top.

**Proposed Design - Wrapper Stack**:
```
┌──────────────────────────────────────────────┐
│      RecordingFileSystemWrapper (ours)       │ ← Always on top
│  Records all access to ReplayContext         │
├──────────────────────────────────────────────┤
│      User's Custom File System (optional)    │ ← User-provided
├──────────────────────────────────────────────┤
│      Default Slang File System               │
└──────────────────────────────────────────────┘
```

**Implementation**:
```cpp
class RecordingFileSystemWrapper : public ISlangMutableFileSystem {
    ComPtr<ISlangFileSystem> m_inner;  // User's or default FS
    ReplayContext& m_ctx;
    
    // Track file contents by hash
    Dictionary<String, size_t> m_pathToContentHash;
    
    SlangResult loadFile(const char* path, ISlangBlob** outBlob) override {
        // Record the request
        m_ctx.beginStaticCall("FileSystem::loadFile");
        m_ctx.record(RecordFlag::Input, path);
        
        // Load from inner
        auto result = m_inner->loadFile(path, outBlob);
        
        // Record the result (content gets stored by hash)
        if (SLANG_SUCCEEDED(result) && *outBlob) {
            m_ctx.recordFileContent(*outBlob);  // Stores by hash
        }
        m_ctx.record(RecordFlag::ReturnValue, result);
        
        return result;
    }
};

// When user calls setFileSystem():
void SessionProxy::setFileSystem(ISlangFileSystem* fs) {
    // Unwrap our existing wrapper
    auto* ourWrapper = dynamic_cast<RecordingFileSystemWrapper*>(currentFS);
    
    // Set user's FS as the inner FS of our wrapper
    ourWrapper->setInner(fs);
    
    // Our wrapper stays on top - don't replace it
}
```

**File Content Storage**:
```cpp
// In ReplayContext:
Dictionary<Hash, List<uint8_t>> m_fileContents;  // Hash → content
Dictionary<String, Hash> m_pathToHash;           // Path → hash (LUT)

void recordFileContent(ISlangBlob* blob) {
    Hash hash = computeHash(blob);
    if (!m_fileContents.containsKey(hash)) {
        m_fileContents[hash] = copyBlobData(blob);
    }
    // Record just the hash in the stream
    record(RecordFlag::Output, hash);
}
```

**Implementation Steps**:
1. Create `RecordingFileSystemWrapper` class
2. Add file content storage to ReplayContext (by hash)
3. Hook into session creation to install wrapper
4. Handle setFileSystem() to re-wrap
5. Playback: create in-memory FS from stored content

**Files**: New `proxy/proxy-recording-file-system.h`, `replay-context.h`

---

### Phase 5: Lifetime Management

#### 5.1 Reference Counting Tracking (Priority: MEDIUM)

**Goal**: Destroy objects at the correct time during playback.

**Proposed Approach**:
```cpp
// Record addRef/release calls
void ReplayContext::recordAddRef(ISlangUnknown* obj) {
    beginStaticCall("ISlangUnknown::addRef");
    recordInterfaceImpl(RecordFlag::Input, obj);
    uint32_t newCount = obj->addRef();
    record(RecordFlag::Output, newCount);  // For debugging
}

void ReplayContext::recordRelease(ISlangUnknown* obj) {
    beginStaticCall("ISlangUnknown::release");
    recordInterfaceImpl(RecordFlag::Input, obj);
    uint32_t newCount = obj->release();
    record(RecordFlag::Output, newCount);
    
    if (newCount == 0) {
        // Object destroyed - remove from handle map
        unregisterInterface(obj);
    }
}

// In ProxyBase:
uint32_t release() override {
    auto& ctx = ReplayContext::get();
    if (ctx.isActive()) {
        ctx.recordRelease(this);
    }
    return RefObject::releaseReference();
}
```

**Debug Mode - RefCount Verification**:
```cpp
// During recording, store refcount with each handle use
void recordHandle(RecordFlag flags, uint64_t& handleId) {
    recordTypeId(TypeId::ObjectHandle);
    recordRaw(flags, &handleId, sizeof(handleId));
    
    #ifdef SLANG_REPLAY_DEBUG
    if (isWriting() && handleId != kNullHandle) {
        auto* obj = getInterfaceForHandle(handleId);
        uint32_t refCount = getRefCount(obj);  // Query without incrementing
        record(flags, refCount);
    }
    #endif
}
```

**Implementation Steps**:
1. Override `addRef()`/`release()` in ProxyBase to record calls
2. Add `unregisterInterface()` for cleanup
3. During playback, track expected refcounts and verify on release
4. Optional: Debug mode that records refcount with every handle use

**Files**: `proxy-base.h`, `replay-context.h`

---

### Phase 6: Multithreading Support

#### 6.1 Thread ID Recording (Priority: MEDIUM-LOW)

**Goal**: Record thread context for each call, enable single-thread replay.

**Proposed Approach**:
```cpp
// Extend index entry with thread ID
struct CommandIndexEntry {
    uint32_t signatureHash;
    uint64_t thisHandle;
    uint64_t dataOffset;
    uint32_t threadId;  // Added
};

// In RECORD_CALL():
uint32_t threadId = getCurrentThreadId();
record(RecordFlag::Input, threadId);

// Playback filtering:
class ReplayContext {
    std::optional<uint32_t> m_threadFilter;
    
    void setThreadFilter(uint32_t threadId) { m_threadFilter = threadId; }
    void clearThreadFilter() { m_threadFilter = std::nullopt; }
    
    bool executeNextCall() {
        // Skip calls from other threads if filter set
        while (!atEnd()) {
            auto entry = peekNextIndexEntry();
            if (!m_threadFilter || entry.threadId == *m_threadFilter) {
                return executeCall(entry);
            }
            skipCall(entry);
        }
        return false;
    }
};
```

**Implementation Steps**:
1. Add `getCurrentThreadId()` platform abstraction
2. Record thread ID in index stream
3. Add thread filtering to playback
4. Test with multithreaded recording

**Files**: `replay-context.h`, `replay-context.cpp`

---

### Phase 7: Disk Storage Format

#### 7.1 Replay Folder Structure (Priority: MEDIUM-LOW)

**Goal**: Save replays to disk with all dependencies.

**Proposed Format**:
```
replay-20260204-143022/
├── manifest.json        # Metadata, version, timestamps
├── data.bin             # Main command stream
├── index.bin            # Command index stream
├── files/               # Captured file contents (by hash)
│   ├── a1b2c3d4.bin    
│   ├── e5f6g7h8.bin
│   └── ...
└── file-map.json        # Path → hash lookup table
```

**manifest.json**:
```json
{
    "version": "1.0",
    "slangVersion": "2026.1.1",
    "timestamp": "2026-02-04T14:30:22Z",
    "platform": "windows-x86_64",
    "commandCount": 1234,
    "threadIds": [1, 2, 3]
}
```

**API**:
```cpp
class ReplayContext {
    SlangResult saveToFolder(const char* path);
    static ReplayContext loadFromFolder(const char* path);
};
```

**Implementation Steps**:
1. Define folder structure and manifest format
2. Add `saveToFolder()` - writes streams and files
3. Add `loadFromFolder()` - static factory method
4. Add file content collection during recording
5. Add tests for save/load round-trip

**Files**: `replay-context.h`, `replay-context.cpp`, new `replay-io.h`

---

### Phase 8: slang-replay Tool Update

#### 8.1 Revive Playback Utility (Priority: LOW)

**Current State**: `tools/slang-replay/main.cpp` references old headers that no longer exist.

**Proposed Approach**:
```cpp
// Simplified new main.cpp
int main(int argc, char* argv[]) {
    Options options = parseArgs(argc, argv);
    
    // Load replay from folder
    auto ctx = ReplayContext::loadFromFolder(options.replayPath);
    
    // Register all handlers
    RegisterAllHandlers(ctx);
    
    // Options
    if (options.threadId) ctx.setThreadFilter(*options.threadId);
    if (options.startCall) ctx.seekToCall(*options.startCall);
    
    // Execute
    while (ctx.hasMoreCalls()) {
        if (options.verbose) {
            printf("Call %zu: %s\n", ctx.getCurrentCallIndex(), 
                   ctx.peekNextSignature());
        }
        ctx.executeNextCall();
    }
    
    printf("Replay complete: %zu calls executed\n", ctx.getExecutedCallCount());
    return 0;
}
```

**Command Line**:
```bash
slang-replay replay-folder/
slang-replay --thread=1 replay-folder/     # Single thread only
slang-replay --start=100 --count=50 replay-folder/  # Range
slang-replay --to-json output.json replay-folder/   # Export
```

**Implementation Steps**:
1. Remove old header references
2. Rewrite using new ReplayContext API
3. Add command-line options for filtering/seeking
4. Add JSON export for debugging

**Files**: `tools/slang-replay/main.cpp`

---

### Phase 9: Full API Coverage

#### 9.1 Expand Proxy Coverage (Priority: ONGOING)

**Goal**: Cover entire Slang API surface.

**Approach**:
1. Start with examples from `unit-test-record-replay.cpp`
2. Add recording to each proxy method incrementally
3. Test each method with round-trip tests
4. Ultimate goal: Record entire test suite, replay successfully

**Current Coverage** (estimate):
- [ ] IGlobalSession: ~20% (createSession done)
- [ ] ISession: ~10%
- [ ] IModule: ~5%
- [ ] IComponentType: ~5%
- [ ] IEntryPoint: ~5%
- [ ] ICompileRequest: ~0%
- [ ] File system: 0%

**Priority Order for Methods**:
1. `IGlobalSession::createSession`
2. `ISession::loadModule`
3. `ISession::createCompositeComponentType`
4. `IComponentType::link`
5. `IComponentType::getEntryPointCode`
6. ... (expand based on test coverage needs)

---

### Phase 10: Stretch Goals

#### 10.1 Replay Viewer UI (Priority: STRETCH)

**Goal**: Cross-platform UI to browse and analyze replays.

**Proposed Stack**: Python + PyQt6 (cross-platform, easy to develop)

**Features**:
- Tree view of recorded calls
- Parameter inspection
- Call timeline visualization
- Search/filter by method name
- Jump to specific calls
- Export selection

**Basic Structure**:
```python
# replay_viewer.py
import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QTreeView
from PyQt6.QtCore import QAbstractItemModel

class ReplayModel(QAbstractItemModel):
    def __init__(self, manifest_path):
        # Load manifest.json and index.bin
        pass
    
    def data(self, index, role):
        # Return call info for display
        pass

class ReplayViewer(QMainWindow):
    def __init__(self, replay_path):
        super().__init__()
        self.model = ReplayModel(replay_path)
        self.tree = QTreeView()
        self.tree.setModel(self.model)
        self.setCentralWidget(self.tree)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    viewer = ReplayViewer(sys.argv[1])
    viewer.show()
    app.exec()
```

**Implementation Steps**:
1. Create Python bindings for reading replay format
2. Build basic tree view of calls
3. Add parameter inspection panel
4. Add search/filter functionality
5. Package as standalone app

**Files**: New `tools/replay-viewer/` directory

---

## Implementation Order Summary

| Phase | Priority | Effort | Dependencies |
|-------|----------|--------|--------------|
| 1.1 Handler Registration | HIGH | Small | None |
| 1.2 Testing Infrastructure | HIGH | Medium | 1.1 |
| 2.1 TypeReflection Support | MEDIUM-HIGH | Medium | None |
| 3.1 Command Index Stream | MEDIUM | Medium | None |
| 4.1 File System Wrapper | MEDIUM | Large | None |
| 5.1 Lifetime Management | MEDIUM | Medium | None |
| 6.1 Multithreading | MEDIUM-LOW | Small | 3.1 |
| 7.1 Disk Storage | MEDIUM-LOW | Medium | 3.1, 4.1 |
| 8.1 slang-replay Tool | LOW | Small | 7.1 |
| 9.1 Full API Coverage | ONGOING | Large | 1.1, 1.2 |
| 10.1 Viewer UI | STRETCH | Large | 7.1 |

**Recommended Sprint Order**:
1. **Sprint 1**: 1.1 + 1.2 (Get basic round-trip testing working)
2. **Sprint 2**: 2.1 + 9.1-partial (TypeReflection + expand API coverage)
3. **Sprint 3**: 3.1 + 6.1 (Index stream + threading support)
4. **Sprint 4**: 4.1 (File system - largest single piece)
5. **Sprint 5**: 5.1 + 7.1 (Lifetime + disk storage)
6. **Sprint 6**: 8.1 + 9.1-continued (Tool + more API coverage)
7. **Ongoing**: 10.1 (Viewer can be developed in parallel)

---

## Future Enhancements (Post-MVP)

- **Compression**: zstd compression for stored data
- **Diffing**: Compare two replays to find behavioral differences
- **Selective Recording**: Record only specific API subsets
- **Streaming Replay**: Process without loading entire file
- **Remote Replay**: Send replay to server for analysis
- **Automatic Regression Detection**: Compare replay outputs across Slang versions

