#pragma once

#include "replay-stream.h"
#include "../core/slang-blob.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-list.h"
#include "../core/slang-memory-arena.h"

#include <slang.h>

#include <cstdint>
#include <cstring>
#include <mutex>

namespace SlangRecord {

using Slang::Dictionary;
using Slang::List;
using Slang::MemoryArena;
using Slang::String;

/// Handle constants for interface tracking.
/// Handles 0-255 are reserved for special meanings.
constexpr uint64_t kNullHandle = 0;          ///< Null pointer
constexpr uint64_t kInlineBlobHandle = 1;    ///< User-provided blob serialized inline
constexpr uint64_t kFirstValidHandle = 0x100; ///< First handle for tracked objects

/// Exception thrown when trying to record an untracked interface.
class UntrackedInterfaceException : public Slang::Exception
{
public:
    UntrackedInterfaceException(ISlangUnknown* obj)
        : Slang::Exception("Attempted to record untracked interface")
        , m_object(obj)
    {
    }
    ISlangUnknown* getObject() const { return m_object; }
private:
    ISlangUnknown* m_object;
};

/// Exception thrown when a handle is not found during playback.
class HandleNotFoundException : public Slang::Exception
{
public:
    HandleNotFoundException(uint64_t handle)
        : Slang::Exception(String("Handle not found: ") + String(handle))
        , m_handle(handle)
    {
    }
    uint64_t getHandle() const { return m_handle; }
private:
    uint64_t m_handle;
};

/// Operating mode for the replay system.
enum class Mode : uint8_t
{
    Idle,       ///< No data captured, operations are no-ops
    Record,     ///< Writing data to a stream
    Sync,       ///< Writing data and comparing to reference stream for determinism verification
    Playback,   ///< Reading data from a stream
};

/// Flags indicating the role of a value being recorded.
/// Used to determine replay verification behavior.
enum class RecordFlag : uint8_t
{
    None = 0,           ///< No special handling
    Input = 1 << 0,     ///< Function input argument (verify on replay)
    Output = 1 << 1,    ///< Function output argument (capture on replay)
    InOut = Input | Output, ///< Input/output argument
    ReturnValue = 1 << 2,   ///< Function return value (capture on replay)
    ThisPtr = 1 << 3,   ///< 'this' pointer for method calls
};

inline RecordFlag operator|(RecordFlag a, RecordFlag b)
{
    return static_cast<RecordFlag>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline RecordFlag operator&(RecordFlag a, RecordFlag b)
{
    return static_cast<RecordFlag>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline bool hasFlag(RecordFlag flags, RecordFlag flag)
{
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

/// Type identifiers for serialized values.
enum class TypeId : uint8_t
{
    Int8 = 0x01, Int16 = 0x02, Int32 = 0x03, Int64 = 0x04,
    UInt8 = 0x05, UInt16 = 0x06, UInt32 = 0x07, UInt64 = 0x08,
    Float32 = 0x09, Float64 = 0x0A, Bool = 0x0B,
    String = 0x10, Blob = 0x11, Array = 0x12, ObjectHandle = 0x13, Null = 0x14,
    Error = 0xEE,  ///< Error marker - indicates an exception occurred
};

SLANG_API const char* getTypeIdName(TypeId id);

/// Exception thrown when type mismatch occurs during deserialization.
class TypeMismatchException : public Slang::Exception
{
public:
    SLANG_API TypeMismatchException(TypeId expected, TypeId actual);
    SLANG_API TypeId getExpected() const { return m_expected; }
    SLANG_API TypeId getActual() const { return m_actual; }
private:
    TypeId m_expected, m_actual;
};

/// Exception thrown when data mismatch occurs during sync mode verification.
class DataMismatchException : public Slang::Exception
{
public:
    SLANG_API DataMismatchException(size_t offset, size_t size);
    SLANG_API size_t getOffset() const { return m_offset; }
    SLANG_API size_t getSize() const { return m_size; }
private:
    size_t m_offset, m_size;
};

/// Unified serializer for binary I/O during record/replay.
/// Provides a uniform API for both reading and writing serialized data.
/// Owns its own ReplayStream and MemoryArena internally.
/// 
/// The context operates in one of four modes:
/// - Idle: No operations performed (default when env var not set)
/// - Record: Writing data to a stream
/// - Sync: Writing data and comparing to reference stream for determinism
/// - Playback: Reading data from a stream
class ReplayContext
{
public:
    /// Get the global singleton instance (for recording).
    /// Thread-safe. The singleton starts in Idle or Record mode based on env var.
    SLANG_API static ReplayContext& get();

    /// Create an idle context.
    /// Will switch to Record mode if SLANG_RECORD_LAYER=1 is set.
    SLANG_API ReplayContext();

    /// Create a playback context from existing data.
    SLANG_API ReplayContext(const void* data, size_t size);

    /// Create a sync context that records while verifying against reference data.
    SLANG_API ReplayContext(const void* referenceData, size_t referenceSize, bool syncMode);

    /// Destructor - must be in DLL to properly free Dictionary memory.
    SLANG_API ~ReplayContext();

    /// Get the current operating mode.
    SLANG_API Mode getMode() const { return m_mode; }

    /// Check if the context is active (not Idle).
    SLANG_API bool isActive() const
    {
        return m_mode != Mode::Idle;
    }

    /// Set the operating mode.
    /// When entering Record mode, sets up mirror file for crash-safe capture.
    /// When leaving Record mode, closes the mirror file.
    SLANG_API void setMode(Mode mode);

    /// Convenience methods for common mode checks.
    SLANG_API bool isIdle() const { return m_mode == Mode::Idle; }
    SLANG_API bool isRecording() const { return m_mode == Mode::Record; }
    SLANG_API bool isSyncing() const { return m_mode == Mode::Sync; }
    SLANG_API bool isPlayback() const { return m_mode == Mode::Playback; }

    /// Enable recording (sets mode to Record if currently Idle).
    SLANG_API void enable();

    /// Disable recording (sets mode to Idle).
    SLANG_API void disable();

    // =========================================================================
    // TTY Logging (Live Trace)
    // =========================================================================

    /// Enable or disable live TTY logging of recorded calls.
    /// When enabled, prints call signatures to stderr as they are recorded.
    /// Can also be enabled via SLANG_RECORD_LOG=1 environment variable.
    SLANG_API void setTtyLogging(bool enable);

    /// Check if TTY logging is enabled.
    SLANG_API bool isTtyLogging() const { return m_ttyLogging; }

    /// Record an error marker in the stream.
    /// Call this when an exception occurs to mark the error location in the stream.
    SLANG_API void recordError(const char* message);

    // =========================================================================
    // Replay Directory Management
    // =========================================================================

    /// Set the base directory for replays (default: ".slang-replays").
    /// Must be called before enabling recording.
    SLANG_API void setReplayDirectory(const char* path);

    /// Get the current replay base directory.
    SLANG_API const char* getReplayDirectory() const;

    /// Get the path to the current recording session folder.
    /// Returns nullptr if not currently recording.
    SLANG_API const char* getCurrentReplayPath() const;

    /// Load a replay from a folder path (reads stream.bin).
    /// Switches to Playback mode on success.
    SLANG_API SlangResult loadReplay(const char* folderPath);

    /// Load the most recent replay from the replay directory.
    /// Switches to Playback mode on success.
    SLANG_API SlangResult loadLatestReplay();

    /// Find the most recent replay folder in the given directory.
    /// Returns empty string if no replays found.
    SLANG_API static String findLatestReplayFolder(const char* baseDir);

    // =========================================================================

    /// Legacy compatibility - maps to Record/Playback modes.
    SLANG_API bool isReading() const { return m_mode == Mode::Playback; }
    SLANG_API bool isWriting() const { return m_mode == Mode::Record || m_mode == Mode::Sync; }

    SLANG_API ReplayStream& getStream() { return m_stream; }
    SLANG_API const ReplayStream& getStream() const { return m_stream; }
    SLANG_API MemoryArena& getArena() { return m_arena; }

    /// Lock the context for thread-safe access.
    /// Returns an RAII lock guard.
    SLANG_API std::unique_lock<std::recursive_mutex> lock() { return std::unique_lock<std::recursive_mutex>(m_mutex); }

    /// Reset the context to initial state (clears streams and arena, mode becomes Idle).
    SLANG_API void reset();

    /// Switch to play mode, clearing all local state and resetting stream to 0, but
    /// keeping the stream data present so we can replay what's just happened
    SLANG_API void switchToPlayback();

    /// Switch to sync mode, copying recorded data to reference stream and resetting
    /// for verification pass
    SLANG_API void switchToSync();

    // Basic types
    SLANG_API void record(RecordFlag flags, int8_t& value);
    SLANG_API void record(RecordFlag flags, int16_t& value);
    SLANG_API void record(RecordFlag flags, int32_t& value);
    SLANG_API void record(RecordFlag flags, int64_t& value);
    SLANG_API void record(RecordFlag flags, uint8_t& value);
    SLANG_API void record(RecordFlag flags, uint16_t& value);
    SLANG_API void record(RecordFlag flags, uint32_t& value);
    SLANG_API void record(RecordFlag flags, uint64_t& value);
    SLANG_API void record(RecordFlag flags, float& value);
    SLANG_API void record(RecordFlag flags, double& value);
    SLANG_API void record(RecordFlag flags, bool& value);
    SLANG_API void record(RecordFlag flags, const char*& str);

    // Blob data (void* + size)
    SLANG_API void recordBlob(RecordFlag flags, const void*& data, size_t& size);

    // Arrays with count - calls record() on each element
    template<typename T, typename CountT>
    void recordArray(RecordFlag flags, const T*& arr, CountT& count);

    /// Parse a function signature to extract "ClassName::methodName" format.
    /// Works with both MSVC __FUNCSIG__ and GCC/Clang __PRETTY_FUNCTION__.
    /// Returns the normalized signature, or the original if parsing fails.
    SLANG_API static const char* parseSignature(const char* signature, char* buffer, size_t bufferSize);

    /// Begin recording a method call.
    /// Records the function signature and 'this' pointer as a tracked handle.
    template<typename T>
    void beginCall(const char* signature, T* thisPtr)
    {
        ensureInitialized();
        if (!isActive())
            return;
        // Parse and record the normalized signature
        char normalizedSig[256];
        const char* parsed = parseSignature(signature, normalizedSig, sizeof(normalizedSig));
        
        // Log to TTY if enabled
        if (m_ttyLogging)
            logCall(parsed, thisPtr);
        
        record(RecordFlag::Input, parsed);
        // Record the 'this' pointer as a handle
        // Use reinterpret_cast to avoid ambiguity with multiple inheritance from ISlangUnknown
        ISlangUnknown* obj = reinterpret_cast<ISlangUnknown*>(thisPtr);
        recordInterfaceImpl<ISlangUnknown>(RecordFlag::Input, obj);
    }

    /// Begin recording a static/free function call.
    /// Records only the function signature.
    void beginStaticCall(const char* signature)
    {
        ensureInitialized();
        if (!isActive())
            return;
        char normalizedSig[256];
        const char* parsed = parseSignature(signature, normalizedSig, sizeof(normalizedSig));
        
        // Log to TTY if enabled
        if (m_ttyLogging)
            logCall(parsed, nullptr);
        
        record(RecordFlag::Input, parsed);
        uint64_t nh = kNullHandle;
        recordHandle(RecordFlag::Input, nh);
    }

    /// Register a proxy-implementation pair.
    /// Call this when wrapping an implementation with a proxy.
    SLANG_API void registerProxy(ISlangUnknown* proxy, ISlangUnknown* implementation);

    /// Register an interface object and get its handle.
    /// Used when creating proxy objects to register them for handle tracking.
    SLANG_API uint64_t registerInterface(ISlangUnknown* obj);

    /// Get or create a proxy for an implementation.
    /// If a proxy already exists, returns it. Otherwise returns nullptr.
    SLANG_API ISlangUnknown* getExistingProxy(ISlangUnknown* implementation);

    /// Get the next handle value that will be assigned.
    /// Useful for testing to know how many objects have been registered.
    SLANG_API uint64_t getNextHandle() const { return m_nextHandle; }

    /// Check if an object is registered.
    SLANG_API bool isInterfaceRegistered(ISlangUnknown* obj) const;

    /// Get handle for an object (throws if not registered).
    SLANG_API uint64_t getHandleForInterface(ISlangUnknown* obj) const;

    /// Get object for a handle (returns nullptr if not found).
    SLANG_API ISlangUnknown* getInterfaceForHandle(uint64_t handle) const;

    // Enum types - record as int32_t
    template<typename EnumT> void recordEnum(RecordFlag flags, EnumT& value);

    // Slang enum types
    SLANG_API void record(RecordFlag flags, SlangSeverity& value);
    SLANG_API void record(RecordFlag flags, SlangBindableResourceType& value);
    SLANG_API void record(RecordFlag flags, SlangCompileTarget& value);
    SLANG_API void record(RecordFlag flags, SlangContainerFormat& value);
    SLANG_API void record(RecordFlag flags, SlangPassThrough& value);
    SLANG_API void record(RecordFlag flags, SlangArchiveType& value);
    SLANG_API void record(RecordFlag flags, SlangFloatingPointMode& value);
    SLANG_API void record(RecordFlag flags, SlangFpDenormalMode& value);
    SLANG_API void record(RecordFlag flags, SlangLineDirectiveMode& value);
    SLANG_API void record(RecordFlag flags, SlangSourceLanguage& value);
    SLANG_API void record(RecordFlag flags, SlangProfileID& value);
    SLANG_API void record(RecordFlag flags, SlangCapabilityID& value);
    SLANG_API void record(RecordFlag flags, SlangMatrixLayoutMode& value);
    SLANG_API void record(RecordFlag flags, SlangStage& value);
    SLANG_API void record(RecordFlag flags, SlangDebugInfoLevel& value);
    SLANG_API void record(RecordFlag flags, SlangDebugInfoFormat& value);
    SLANG_API void record(RecordFlag flags, SlangOptimizationLevel& value);
    SLANG_API void record(RecordFlag flags, SlangEmitSpirvMethod& value);
    SLANG_API void record(RecordFlag flags, slang::CompilerOptionName& value);
    SLANG_API void record(RecordFlag flags, slang::CompilerOptionValueKind& value);
    SLANG_API void record(RecordFlag flags, slang::ContainerType& value);
    SLANG_API void record(RecordFlag flags, slang::SpecializationArg::Kind& value);
    SLANG_API void record(RecordFlag flags, SlangLanguageVersion& value);
    SLANG_API void record(RecordFlag flags, slang::BuiltinModuleName& value);

    // POD and complex structs
    SLANG_API void record(RecordFlag flags, SlangUUID& value);
    SLANG_API void record(RecordFlag flags, slang::CompilerOptionValue& value);
    SLANG_API void record(RecordFlag flags, slang::CompilerOptionEntry& value);
    SLANG_API void record(RecordFlag flags, slang::PreprocessorMacroDesc& value);
    SLANG_API void record(RecordFlag flags, slang::TargetDesc& value);
    SLANG_API void record(RecordFlag flags, slang::SessionDesc& value);
    SLANG_API void record(RecordFlag flags, slang::SpecializationArg& value);
    SLANG_API void record(RecordFlag flags, SlangGlobalSessionDesc& value);

    // COM interface pointers - handle tracking is done internally
    SLANG_API void record(RecordFlag flags, ISlangBlob*& obj);
    SLANG_API void record(RecordFlag flags, ISlangFileSystem*& obj);
    SLANG_API void record(RecordFlag flags, ISlangFileSystemExt*& obj);
    SLANG_API void record(RecordFlag flags, ISlangMutableFileSystem*& obj);
    SLANG_API void record(RecordFlag flags, ISlangSharedLibrary*& obj);
    SLANG_API void record(RecordFlag flags, slang::IGlobalSession*& obj);
    SLANG_API void record(RecordFlag flags, slang::ISession*& obj);
    SLANG_API void record(RecordFlag flags, slang::IModule*& obj);
    SLANG_API void record(RecordFlag flags, slang::IComponentType*& obj);
    SLANG_API void record(RecordFlag flags, slang::IEntryPoint*& obj);
    SLANG_API void record(RecordFlag flags, slang::ITypeConformance*& obj);
    SLANG_API void record(RecordFlag flags, slang::ICompileRequest*& obj);

    // Object handles (COM interface pointers mapped to IDs)
    // Public for testing purposes
    SLANG_API void recordHandle(RecordFlag flags, uint64_t& handleId);

private:
    void recordRaw(RecordFlag flags, void* data, size_t size);
    void recordTypeId(TypeId id);
    void writeTypeId(TypeId id);
    TypeId readTypeId();
    TypeId readTypeIdFromReference();
    void expectTypeId(TypeId expected);

    /// Record a COM interface pointer (internal implementation).
    template<typename T>
    void recordInterfaceImpl(RecordFlag flags, T*& obj);

    std::recursive_mutex m_mutex;
    ReplayStream m_stream;          ///< Main stream for record/playback
    ReplayStream m_referenceStream; ///< Reference stream for sync mode comparison
    MemoryArena m_arena;
    Mode m_mode;
    List<uint8_t> m_compareBuffer; ///< Reusable buffer for sync comparisons

    // Handle tracking: maps objects to handles and back
    Dictionary<ISlangUnknown*, uint64_t> m_objectToHandle;
    Dictionary<uint64_t, ISlangUnknown*> m_handleToObject;
    uint64_t m_nextHandle = kFirstValidHandle;

    // Proxy tracking: maps proxies to implementations and back
    Dictionary<ISlangUnknown*, ISlangUnknown*> m_proxyToImpl;
    Dictionary<ISlangUnknown*, ISlangUnknown*> m_implToProxy;

    // Replay directory management
    String m_replayDirectory = ".slang-replays";  ///< Base directory for replays
    String m_currentReplayPath;                    ///< Current recording session folder

    // TTY logging
    bool m_ttyLogging = false;  ///< Whether to log calls to stderr
    
    // Deferred initialization (to avoid global init order issues with CharEncoding)
    bool m_initialized = false;          ///< True after ensureInitialized() has run
    
    /// Ensure deferred initialization has completed.
    /// Called on first actual use to avoid global init order issues.
    SLANG_API void ensureInitialized();
    
    /// Log a call to stderr (used when m_ttyLogging is enabled).
    SLANG_API void logCall(const char* signature, void* thisPtr);

    /// Set up mirror file for crash-safe capture when entering Record mode.
    void setupRecordingMirror();
    
    /// Close mirror file when leaving Record mode.
    void closeRecordingMirror();
    
    /// Generate a timestamp folder name (e.g., "2026-02-04_14-30-45-123").
    static String generateTimestampFolderName();

    // ==========================================================================
    // Playback Dispatcher
    // ==========================================================================
public:
    /// Function type for registered playback handlers.
    /// The handler is called in playback mode and should call the appropriate
    /// record() methods to read arguments from the stream, then execute the call.
    using PlaybackHandler = void(*)(ReplayContext& ctx);

    /// Register a playback handler for a function signature.
    /// The signature should match what __FUNCSIG__ or __PRETTY_FUNCTION__ produces.
    SLANG_API void registerHandler(const char* signature, PlaybackHandler handler);

    /// Execute the next recorded call from the stream.
    /// Reads the function signature, looks up the handler, and calls it.
    /// Returns true if a call was executed, false if at end of stream.
    SLANG_API bool executeNextCall();

    /// Execute all recorded calls until end of stream.
    SLANG_API void executeAll();

    /// Check if there are more calls to execute.
    SLANG_API bool hasMoreCalls() const { return !m_stream.atEnd(); }

    /// Get the 'this' handle for the current call being executed.
    /// Only valid within a playback handler.
    SLANG_API uint64_t getCurrentThisHandle() const { return m_currentThisHandle; }

    /// Get the 'this' pointer for the current call, cast to the given type.
    /// Only valid within a playback handler.
    template<typename T>
    T* getCurrentThis()
    {
        if (m_currentThisHandle == kNullHandle)
            return nullptr;
        return static_cast<T*>(getInterfaceForHandle(m_currentThisHandle));
    }

private:
    // Map from function signature to handler
    Dictionary<String, PlaybackHandler> m_handlers;
    
    // Current 'this' handle during playback execution
    uint64_t m_currentThisHandle = kNullHandle;
};

// Template implementations

template<typename T, typename CountT>
void ReplayContext::recordArray(RecordFlag flags, const T*& arr, CountT& count)
{
    if (m_mode == Mode::Idle) return;
    if (isWriting())
    {
        recordTypeId(TypeId::Array);
        uint64_t arrayCount = static_cast<uint64_t>(count);
        record(flags, arrayCount);
        for (uint64_t i = 0; i < arrayCount; ++i)
            record(flags, const_cast<T&>(arr[i]));
    }
    else
    {
        expectTypeId(TypeId::Array);
        uint64_t arrayCount;
        record(flags, arrayCount);
        count = static_cast<CountT>(arrayCount);
        if (arrayCount > 0)
        {
            T* buf = m_arena.allocateArray<T>(static_cast<size_t>(arrayCount));
            for (uint64_t i = 0; i < arrayCount; ++i)
            {
                new (&buf[i]) T{};
                record(flags, buf[i]);
            }
            arr = buf;
        }
        else
        {
            arr = nullptr;
        }
    }
}

template<typename EnumT>
void ReplayContext::recordEnum(RecordFlag flags, EnumT& value)
{
    if (m_mode == Mode::Idle) return;
    int32_t v = static_cast<int32_t>(value);
    record(flags, v);
    if (isReading())
        value = static_cast<EnumT>(v);
}

template<typename T>
void ReplayContext::recordInterfaceImpl(RecordFlag flags, T*& obj)
{
    if (m_mode == Mode::Idle) return;

    bool isInput = hasFlag(flags, RecordFlag::Input) || hasFlag(flags, RecordFlag::ThisPtr);
    bool isOutput = hasFlag(flags, RecordFlag::Output) || hasFlag(flags, RecordFlag::ReturnValue);

    if (isWriting())
    {
        // Recording mode
        if (isInput)
        {
            // Handle null
            if (obj == nullptr)
            {
                uint64_t handle = kNullHandle;
                recordHandle(flags, handle);
                return;
            }

            // Special case: ISlangBlob may be user-provided data
            if (auto blob = dynamic_cast<ISlangBlob*>(static_cast<ISlangUnknown*>(obj)))
            {
                // Record blob contents inline if not already tracked
                if (!isInterfaceRegistered(blob))
                {
                    // Write inline blob marker
                    uint64_t handle = kInlineBlobHandle;
                    recordHandle(flags, handle);
                    
                    // Record blob data
                    const void* data = blob->getBufferPointer();
                    size_t size = blob->getBufferSize();
                    recordBlob(flags, data, size);
                    return;
                }
            }

            // Normal case: look up handle for tracked object
            uint64_t handle = getHandleForInterface(static_cast<ISlangUnknown*>(obj));
            recordHandle(flags, handle);
        }
        else if (isOutput)
        {
            // Output: register object and record handle
            uint64_t handle = registerInterface(static_cast<ISlangUnknown*>(obj));
            recordHandle(flags, handle);
        }
    }
    else
    {
        // Playback mode
        if (isInput)
        {
            // Read handle
            uint64_t handle = kNullHandle;
            recordHandle(flags, handle);

            if (handle == kNullHandle)
            {
                obj = nullptr;
            }
            else if (handle == kInlineBlobHandle)
            {
                // Read inline blob data and create a new blob
                const void* data = nullptr;
                size_t size = 0;
                recordBlob(flags, data, size);
                
                // Create a RawBlob from the data (arena-allocated during recordBlob)
                Slang::ComPtr<ISlangBlob> blob = Slang::RawBlob::create(data, size);
                obj = static_cast<T*>(static_cast<ISlangUnknown*>(blob.detach()));
            }
            else
            {
                obj = static_cast<T*>(getInterfaceForHandle(handle));
            }
        }
        else if (isOutput)
        {
            // Output: register object and record handle
            uint64_t handle = registerInterface(static_cast<ISlangUnknown*>(obj));
            recordHandle(flags, handle);
        }
    }
}

} // namespace SlangRecord
