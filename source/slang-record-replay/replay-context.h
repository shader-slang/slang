#pragma once

#include "replay-stream.h"
#include "../core/slang-memory-arena.h"

#include <slang.h>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>

namespace SlangRecord {

using Slang::MemoryArena;

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

/// Unified serializer for binary I/O during record/replay.
/// Provides a uniform API for both reading and writing serialized data.
/// Owns its own ReplayStream and MemoryArena internally.
/// 
/// Recording is automatically enabled if SLANG_RECORD_LAYER=1 environment
/// variable is set, or can be explicitly enabled/disabled at runtime.
class ReplayContext
{
public:
    /// Get the global singleton instance (for recording).
    /// Thread-safe. The singleton is always in writing mode.
    static ReplayContext& get();

    /// Create a writing context (for recording).
    /// Automatically enabled if SLANG_RECORD_LAYER=1 is set.
    ReplayContext();

    /// Create a reading context (for replay) from existing data.
    /// Reading contexts are always active.
    ReplayContext(const void* data, size_t size);

    /// Check if recording/replay is active.
    /// For writing contexts, this is controlled by enable()/disable() or env var.
    /// For reading contexts, this always returns true.
    bool isActive() const { return m_isActive; }

    /// Enable recording (for writing contexts).
    void enable() { m_isActive = true; }

    /// Disable recording (for writing contexts).
    void disable() { m_isActive = false; }

    bool isReading() const { return m_stream.isReading(); }
    bool isWriting() const { return !isReading(); }
    ReplayStream& getStream() { return m_stream; }
    const ReplayStream& getStream() const { return m_stream; }
    MemoryArena& getArena() { return m_arena; }

    /// Lock the context for thread-safe access.
    /// Returns an RAII lock guard.
    std::unique_lock<std::recursive_mutex> lock() { return std::unique_lock<std::recursive_mutex>(m_mutex); }

    /// Reset the context to initial state (clears stream and arena).
    void reset();

    // Basic types
    void serialize(int8_t& value);
    void serialize(int16_t& value);
    void serialize(int32_t& value);
    void serialize(int64_t& value);
    void serialize(uint8_t& value);
    void serialize(uint16_t& value);
    void serialize(uint32_t& value);
    void serialize(uint64_t& value);
    void serialize(float& value);
    void serialize(double& value);
    void serialize(bool& value);
    void serialize(const char*& str);

    // Blob data (void* + size)
    void serializeBlob(const void*& data, size_t& size);

    // Arrays with count - calls serialize() on each element
    template<typename T, typename CountT>
    void serializeArray(const T*& arr, CountT& count);

    // Object handles (COM interface pointers mapped to IDs)
    void serializeHandle(uint64_t& handleId);

    // Enum types - serialize as int32_t
    template<typename EnumT> void serializeEnum(EnumT& value);

    // Slang enum types
    void serialize(SlangSeverity& value);
    void serialize(SlangBindableResourceType& value);
    void serialize(SlangCompileTarget& value);
    void serialize(SlangContainerFormat& value);
    void serialize(SlangPassThrough& value);
    void serialize(SlangArchiveType& value);
    void serialize(SlangFloatingPointMode& value);
    void serialize(SlangFpDenormalMode& value);
    void serialize(SlangLineDirectiveMode& value);
    void serialize(SlangSourceLanguage& value);
    void serialize(SlangProfileID& value);
    void serialize(SlangCapabilityID& value);
    void serialize(SlangMatrixLayoutMode& value);
    void serialize(SlangStage& value);
    void serialize(SlangDebugInfoLevel& value);
    void serialize(SlangDebugInfoFormat& value);
    void serialize(SlangOptimizationLevel& value);
    void serialize(SlangEmitSpirvMethod& value);
    void serialize(slang::CompilerOptionName& value);
    void serialize(slang::CompilerOptionValueKind& value);
    void serialize(slang::ContainerType& value);
    void serialize(slang::SpecializationArg::Kind& value);
    void serialize(SlangLanguageVersion& value);
    void serialize(slang::BuiltinModuleName& value);

    // POD and complex structs
    void serialize(SlangUUID& value);
    void serialize(slang::CompilerOptionValue& value);
    void serialize(slang::CompilerOptionEntry& value);
    void serialize(slang::PreprocessorMacroDesc& value);
    void serialize(slang::TargetDesc& value);
    void serialize(slang::SessionDesc& value);
    void serialize(slang::SpecializationArg& value);
    void serialize(SlangGlobalSessionDesc& value);

private:
    void serializeRaw(void* data, size_t size);
    void serializeTypeId(TypeId id);
    void writeTypeId(TypeId id);
    TypeId readTypeId();
    void expectTypeId(TypeId expected);

    std::recursive_mutex m_mutex;
    ReplayStream m_stream;
    MemoryArena m_arena;
    bool m_isActive;
};

// Template implementations

template<typename T, typename CountT>
void ReplayContext::serializeArray(const T*& arr, CountT& count)
{
    if (isWriting())
    {
        writeTypeId(TypeId::Array);
        uint64_t arrayCount = static_cast<uint64_t>(count);
        m_stream.write(&arrayCount, sizeof(arrayCount));
        for (uint64_t i = 0; i < arrayCount; ++i)
            serialize(const_cast<T&>(arr[i]));
    }
    else
    {
        expectTypeId(TypeId::Array);
        uint64_t arrayCount;
        m_stream.read(&arrayCount, sizeof(arrayCount));
        count = static_cast<CountT>(arrayCount);
        if (arrayCount > 0)
        {
            T* buf = m_arena.allocateArray<T>(static_cast<size_t>(arrayCount));
            for (uint64_t i = 0; i < arrayCount; ++i)
            {
                new (&buf[i]) T{};
                serialize(buf[i]);
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
void ReplayContext::serializeEnum(EnumT& value)
{
    int32_t v = static_cast<int32_t>(value);
    serialize(v);
    if (isReading())
        value = static_cast<EnumT>(v);
}

} // namespace SlangRecord
