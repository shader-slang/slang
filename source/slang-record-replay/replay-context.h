#pragma once

#include "replay-stream.h"
#include "handle-tracker.h"
#include "../core/slang-memory-arena.h"

#include <slang.h>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace SlangRecord {

using Slang::MemoryArena;

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
};

const char* getTypeIdName(TypeId id);

/// Exception thrown when type mismatch occurs during deserialization.
class TypeMismatchException : public std::runtime_error
{
public:
    TypeMismatchException(TypeId expected, TypeId actual);
    TypeId getExpected() const { return m_expected; }
    TypeId getActual() const { return m_actual; }
private:
    TypeId m_expected, m_actual;
};

/// Exception thrown when data mismatch occurs during sync mode verification.
class DataMismatchException : public std::runtime_error
{
public:
    DataMismatchException(size_t offset, size_t size);
    size_t getOffset() const { return m_offset; }
    size_t getSize() const { return m_size; }
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
    static ReplayContext& get();

    /// Create an idle context.
    /// Will switch to Record mode if SLANG_RECORD_LAYER=1 is set.
    ReplayContext();

    /// Create a playback context from existing data.
    ReplayContext(const void* data, size_t size);

    /// Create a sync context that records while verifying against reference data.
    ReplayContext(const void* referenceData, size_t referenceSize, bool syncMode);

    /// Get the current operating mode.
    Mode getMode() const { return m_mode; }

    /// Check if the context is active (not Idle).
    bool isActive() const { return m_mode != Mode::Idle; }

    /// Set the operating mode.
    void setMode(Mode mode) { m_mode = mode; }

    /// Convenience methods for common mode checks.
    bool isIdle() const { return m_mode == Mode::Idle; }
    bool isRecording() const { return m_mode == Mode::Record; }
    bool isSyncing() const { return m_mode == Mode::Sync; }
    bool isPlayback() const { return m_mode == Mode::Playback; }

    /// Enable recording (sets mode to Record if currently Idle).
    void enable() { if (m_mode == Mode::Idle) m_mode = Mode::Record; }

    /// Disable recording (sets mode to Idle).
    void disable() { m_mode = Mode::Idle; }

    /// Legacy compatibility - maps to Record/Playback modes.
    bool isReading() const { return m_mode == Mode::Playback; }
    bool isWriting() const { return m_mode == Mode::Record || m_mode == Mode::Sync; }

    ReplayStream& getStream() { return m_stream; }
    const ReplayStream& getStream() const { return m_stream; }
    MemoryArena& getArena() { return m_arena; }

    /// Lock the context for thread-safe access.
    /// Returns an RAII lock guard.
    std::unique_lock<std::recursive_mutex> lock() { return std::unique_lock<std::recursive_mutex>(m_mutex); }

    /// Reset the context to initial state (clears streams and arena, mode becomes Idle).
    void reset();

    // Basic types
    void record(RecordFlag flags, int8_t& value);
    void record(RecordFlag flags, int16_t& value);
    void record(RecordFlag flags, int32_t& value);
    void record(RecordFlag flags, int64_t& value);
    void record(RecordFlag flags, uint8_t& value);
    void record(RecordFlag flags, uint16_t& value);
    void record(RecordFlag flags, uint32_t& value);
    void record(RecordFlag flags, uint64_t& value);
    void record(RecordFlag flags, float& value);
    void record(RecordFlag flags, double& value);
    void record(RecordFlag flags, bool& value);
    void record(RecordFlag flags, const char*& str);

    // Blob data (void* + size)
    void recordBlob(RecordFlag flags, const void*& data, size_t& size);

    // Arrays with count - calls record() on each element
    template<typename T, typename CountT>
    void recordArray(RecordFlag flags, const T*& arr, CountT& count);

    // Object handles (COM interface pointers mapped to IDs)
    void recordHandle(RecordFlag flags, uint64_t& handleId);

    /// Record a COM interface pointer.
    /// During recording:
    /// - Input: looks up handle (throws UntrackedInterfaceException if not registered)
    /// - Output: registers object and assigns handle
    /// During playback:
    /// - Input: looks up object by handle
    /// - Output: verifies handle matches (object should already be registered)
    /// 
    /// Special case: ISlangBlob passed as input may contain user data and is recorded inline.
    template<typename T>
    void recordInterface(RecordFlag flags, T*& obj);

    /// Register an interface that was just created.
    /// Call this after creating an object to track it.
    /// Returns the assigned handle.
    uint64_t registerInterface(ISlangUnknown* obj);

    /// Register a proxy-implementation pair.
    /// Call this when wrapping an implementation with a proxy.
    void registerProxy(ISlangUnknown* proxy, ISlangUnknown* implementation);

    /// Get or create a proxy for an implementation.
    /// If a proxy already exists, returns it. Otherwise returns nullptr.
    ISlangUnknown* getExistingProxy(ISlangUnknown* implementation);

    /// Check if an object is registered.
    bool isInterfaceRegistered(ISlangUnknown* obj) const;

    /// Get handle for an object (throws if not registered).
    uint64_t getHandleForInterface(ISlangUnknown* obj) const;

    /// Get object for a handle (throws if not registered).
    ISlangUnknown* getInterfaceForHandle(uint64_t handle) const;

    // Enum types - record as int32_t
    template<typename EnumT> void recordEnum(RecordFlag flags, EnumT& value);

    // Slang enum types
    void record(RecordFlag flags, SlangSeverity& value);
    void record(RecordFlag flags, SlangBindableResourceType& value);
    void record(RecordFlag flags, SlangCompileTarget& value);
    void record(RecordFlag flags, SlangContainerFormat& value);
    void record(RecordFlag flags, SlangPassThrough& value);
    void record(RecordFlag flags, SlangArchiveType& value);
    void record(RecordFlag flags, SlangFloatingPointMode& value);
    void record(RecordFlag flags, SlangFpDenormalMode& value);
    void record(RecordFlag flags, SlangLineDirectiveMode& value);
    void record(RecordFlag flags, SlangSourceLanguage& value);
    void record(RecordFlag flags, SlangProfileID& value);
    void record(RecordFlag flags, SlangCapabilityID& value);
    void record(RecordFlag flags, SlangMatrixLayoutMode& value);
    void record(RecordFlag flags, SlangStage& value);
    void record(RecordFlag flags, SlangDebugInfoLevel& value);
    void record(RecordFlag flags, SlangDebugInfoFormat& value);
    void record(RecordFlag flags, SlangOptimizationLevel& value);
    void record(RecordFlag flags, SlangEmitSpirvMethod& value);
    void record(RecordFlag flags, slang::CompilerOptionName& value);
    void record(RecordFlag flags, slang::CompilerOptionValueKind& value);
    void record(RecordFlag flags, slang::ContainerType& value);
    void record(RecordFlag flags, slang::SpecializationArg::Kind& value);
    void record(RecordFlag flags, SlangLanguageVersion& value);
    void record(RecordFlag flags, slang::BuiltinModuleName& value);

    // POD and complex structs
    void record(RecordFlag flags, SlangUUID& value);
    void record(RecordFlag flags, slang::CompilerOptionValue& value);
    void record(RecordFlag flags, slang::CompilerOptionEntry& value);
    void record(RecordFlag flags, slang::PreprocessorMacroDesc& value);
    void record(RecordFlag flags, slang::TargetDesc& value);
    void record(RecordFlag flags, slang::SessionDesc& value);
    void record(RecordFlag flags, slang::SpecializationArg& value);
    void record(RecordFlag flags, SlangGlobalSessionDesc& value);

private:
    void recordRaw(RecordFlag flags, void* data, size_t size);
    void recordTypeId(TypeId id);
    void writeTypeId(TypeId id);
    TypeId readTypeId();
    TypeId readTypeIdFromReference();
    void expectTypeId(TypeId expected);

    std::recursive_mutex m_mutex;
    ReplayStream m_stream;          ///< Main stream for record/playback
    ReplayStream m_referenceStream; ///< Reference stream for sync mode comparison
    MemoryArena m_arena;
    Mode m_mode;
    std::vector<uint8_t> m_compareBuffer; ///< Reusable buffer for sync comparisons

    // Handle tracking: maps objects to handles and back
    std::unordered_map<ISlangUnknown*, uint64_t> m_objectToHandle;
    std::unordered_map<uint64_t, ISlangUnknown*> m_handleToObject;
    uint64_t m_nextHandle = kFirstValidHandle;

    // Proxy tracking: maps proxies to implementations and back
    std::unordered_map<ISlangUnknown*, ISlangUnknown*> m_proxyToImpl;
    std::unordered_map<ISlangUnknown*, ISlangUnknown*> m_implToProxy;
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
void ReplayContext::recordInterface(RecordFlag flags, T*& obj)
{
    if (m_mode == Mode::Idle) return;

    bool isInput = hasFlag(flags, RecordFlag::Input) || hasFlag(flags, RecordFlag::ThisPtr);
    bool isOutput = hasFlag(flags, RecordFlag::Output) || hasFlag(flags, RecordFlag::ReturnValue);

    if (isWriting())
    {
        // Recording mode
        if (isInput)
        {
            // Input: look up handle for existing object
            // Special case: ISlangBlob may be user-provided data
            if (auto blob = dynamic_cast<ISlangBlob*>(static_cast<ISlangUnknown*>(obj)))
            {
                // Record blob contents inline if not already tracked
                if (!isInterfaceRegistered(blob))
                {
                    // Write a special "inline blob" marker (handle 0 with data following)
                    uint64_t handle = kNullHandle;
                    recordHandle(flags, handle);
                    
                    // Record blob data
                    const void* data = blob->getBufferPointer();
                    size_t size = blob->getBufferSize();
                    recordBlob(flags, data, size);
                    return;
                }
            }

            // Normal case: look up handle
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
            // Read handle and look up object
            uint64_t handle = kNullHandle;
            recordHandle(flags, handle);

            if (handle == kNullHandle)
            {
                // Could be null or inline blob
                // For now, assume null
                obj = nullptr;
            }
            else
            {
                obj = static_cast<T*>(getInterfaceForHandle(handle));
            }
        }
        else if (isOutput)
        {
            // Read handle - object should already be registered via registerInterface
            uint64_t handle = kNullHandle;
            recordHandle(flags, handle);

            if (handle != kNullHandle)
            {
                // Verify object is registered with this handle
                auto it = m_handleToObject.find(handle);
                if (it == m_handleToObject.end())
                {
                    throw HandleNotFoundException(handle);
                }
                obj = static_cast<T*>(it->second);
            }
            else
            {
                obj = nullptr;
            }
        }
    }
}

} // namespace SlangRecord
