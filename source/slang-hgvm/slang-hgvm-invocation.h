#pragma once

#include "core/slang-basic.h"
#include "slang-hgbc-format.h"

#include <cstring>

namespace Slang::HGVM
{
class IHGVMHostServices;

enum class HGVMHostObjectKind
{
    Window,
    Surface,
};

/// A typed object whose representation and lifetime are owned by an interpreter host.
class HGVMHostObject : public RefObject
{
public:
    HGVMHostObject(HGVMHostObjectKind kind, IHGVMHostServices* owner)
        : m_kind(kind), m_owner(owner)
    {
        SLANG_RELEASE_ASSERT(owner);
    }

    HGVMHostObjectKind getKind() const { return m_kind; }
    IHGVMHostServices* getOwner() const { return m_owner; }

private:
    HGVMHostObjectKind m_kind;
    IHGVMHostServices* m_owner;
};

/// Identifies the scalar semantics embedded in a staged basic operation.
///
/// Consider `OpAdd Float16 ...`: the enum selects the 16-bit floating-point wrapper and its
/// two-byte storage without an HGBC type lookup. A standalone four-bit value uses one canonical
/// byte in interpreter storage; packed resource layouts are handled by their transfer operations.
enum class ScalarType : uint32_t
{
    Bool,
    Int4,
    UInt4,
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Float4E2M1,
    Float8E4M3,
    Float8E5M2,
    Float16,
    BFloat16,
    Float32,
    Float64,
};

/// Describes the physical interpreter layout selected for a scalar type.
///
/// For example, `Float64` has an eight-byte size and alignment, while `Float4E2M1` has a four-bit
/// logical width but occupies one byte as a standalone HGVM value.
struct ScalarLayout
{
    uint32_t logicalBitWidth = 0;
    uint32_t byteSize = 0;
    uint32_t byteAlignment = 1;
};

/// Returns the canonical VM scalar layout, independent of shader buffer layouts.
ScalarLayout getScalarLayout(ScalarType type);

/// Describes a concrete function using IDs scoped to its owning Program.
struct FunctionSignature
{
    HGBCId resultType = 0;
    List<HGBCId> parameterTypes;
};

/// Host-facing metadata for one canonical VM type. Names remain available after VM close;
/// canonical addresses and payloads are deliberately hidden behind native handles.
struct HostValueType
{
    enum class Kind
    {
        Void,
        Scalar,
        Aggregate,
        HostObject,
    };

    Kind kind = Kind::Void;
    ScalarType scalarType = ScalarType::Bool;
    HGVMHostObjectKind hostObjectKind = HGVMHostObjectKind::Window;
    size_t byteSize = 0;
    String name;
};

class InterpreterCore;
class Value;
struct Argument;

/// An immutable function binding whose signature and entry point are resolved once.
/// The owning VM can invalidate the binding without waiting for this handle's destruction.
class BoundFunction : public RefObject
{
public:
    const String& getName() const;
    ConstArrayView<HostValueType> getParameterTypes() const;
    const HostValueType& getResultType() const;
    bool isValid() const;

    /// Calls this binding on its owning VM using the same checks as Interpreter::invoke.
    /// The binding does not keep the VM alive; close or destruction makes later calls fail.
    /// Failure leaves outResult unchanged, including when it is also an input argument.
    SlangResult operator()(ConstArrayView<Argument> arguments, Value& outResult, String& outError)
        const;

private:
    friend class InterpreterCore;
    BoundFunction() = default;
    RefPtr<RefObject> m_state;
};

/// Owns a VM result. Copies share native storage; its last owner releases the managed fields.
/// Passing the value into a function copies and retains those fields with ordinary VM semantics.
/// VM close clears the payload immediately. Default construction represents void; scalar access
/// validates both liveness and scalar kind. Referenced class objects can still be mutated by Slang.
class Value
{
public:
    bool isValid() const;
    bool isVoid() const;
    const HostValueType& getType() const;
    SlangResult getScalarData(ScalarType type, void* destination, size_t size, String& outError)
        const;

private:
    friend class InterpreterCore;
    RefPtr<RefObject> m_state;
};

/// A scalar with an explicit native kind, or a retained native value handle. Raw aggregate bytes
/// cannot enter through this API, so callers cannot forge VM references or canonical identities.
struct Argument
{
    enum class Kind
    {
        Scalar,
        Value,
        HostObject,
    };

    Kind kind = Kind::Scalar;
    ScalarType scalarType = ScalarType::Bool;
    uint8_t scalarBytes[sizeof(double)] = {};
    Value valueStorage;
    RefPtr<HGVMHostObject> hostObject;

    static Argument fromScalar(ScalarType type, const void* data, size_t size)
    {
        SLANG_RELEASE_ASSERT(
            data && size == getScalarLayout(type).byteSize && size <= sizeof(scalarBytes));
        Argument result;
        result.scalarType = type;
        std::memcpy(result.scalarBytes, data, size);
        return result;
    }

    static Argument scalar(bool value)
    {
        uint8_t bits = value ? 1 : 0;
        return fromScalar(ScalarType::Bool, &bits, sizeof(bits));
    }
    static Argument scalar(int32_t value)
    {
        return fromScalar(ScalarType::Int32, &value, sizeof(value));
    }
    static Argument scalar(uint32_t value)
    {
        return fromScalar(ScalarType::UInt32, &value, sizeof(value));
    }
    static Argument scalar(float value)
    {
        return fromScalar(ScalarType::Float32, &value, sizeof(value));
    }
    static Argument scalar(double value)
    {
        return fromScalar(ScalarType::Float64, &value, sizeof(value));
    }
    static Argument fromValue(const Value& value)
    {
        Argument result;
        result.kind = Kind::Value;
        result.valueStorage = value;
        return result;
    }

    /// Borrows a host object into a call on an interpreter using that object's host services.
    /// The argument retains the object; its host services must outlive the invocation.
    static Argument fromHostObject(HGVMHostObject* object)
    {
        Argument result;
        result.kind = Kind::HostObject;
        result.hostObject = object;
        return result;
    }
};

/// Borrows one host value for a synchronous invocation. Bytes use the canonical VM layout.
/// This compatibility API accepts scalar values only; aggregates use owned Value handles.
struct ValueView
{
    HGBCId typeId = 0;
    const void* data = nullptr;
    size_t size = 0;
};

/// Borrows host result storage, written only after successful execution. Void uses empty storage
/// with the function's void type ID. Storage need not be aligned and may alias an input buffer.
struct MutableValueView
{
    HGBCId typeId = 0;
    void* data = nullptr;
    size_t size = 0;
};

} // namespace Slang::HGVM
