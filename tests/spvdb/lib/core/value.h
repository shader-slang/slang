#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace spvdb
{

// Tagged union representing any SPIR-V runtime value.
// Composite covers vectors, matrices, arrays, and structs (elements stored
// recursively). Pointer is an abstract logical pointer — not a raw address.
struct Value
{
    enum class Kind
    {
        Bool,
        Int32,
        UInt32,
        Float32,
        Int64,
        UInt64,
        Float64,
        Composite, // vector, matrix, array, struct
        Pointer, // logical SPIR-V pointer
    };

    Kind kind = Kind::UInt32;

    union ScalarData
    {
        bool b;
        int32_t i32;
        uint32_t u32;
        float f32;
        int64_t i64;
        uint64_t u64;
        double f64;
    } scalar = {};

    // Non-empty when kind == Composite.
    std::vector<Value> elements;
    // Parallel to elements; non-empty only for struct composites where member
    // names are known (e.g. values built from DebugTypeComposite/SpvOpTypeStruct).
    // Empty for vectors, matrices, and arrays.
    std::vector<std::string> member_names;

    // Non-empty when kind == Pointer.
    // Logical pointer: (storage_class, base_variable_result_id, access_chain).
    uint32_t ptr_storage_class = 0;
    uint32_t ptr_base_var = 0;
    std::vector<uint32_t> ptr_chain;

    // Human-readable type name; populated by inspection functions.
    std::string type_name;

    // --- Factories ---
    static Value make_bool(bool v)
    {
        Value r;
        r.kind = Kind::Bool;
        r.scalar.b = v;
        return r;
    }
    static Value make_i32(int32_t v)
    {
        Value r;
        r.kind = Kind::Int32;
        r.scalar.i32 = v;
        return r;
    }
    static Value make_u32(uint32_t v)
    {
        Value r;
        r.kind = Kind::UInt32;
        r.scalar.u32 = v;
        return r;
    }
    static Value make_f32(float v)
    {
        Value r;
        r.kind = Kind::Float32;
        r.scalar.f32 = v;
        return r;
    }
    static Value make_i64(int64_t v)
    {
        Value r;
        r.kind = Kind::Int64;
        r.scalar.i64 = v;
        return r;
    }
    static Value make_u64(uint64_t v)
    {
        Value r;
        r.kind = Kind::UInt64;
        r.scalar.u64 = v;
        return r;
    }
    static Value make_f64(double v)
    {
        Value r;
        r.kind = Kind::Float64;
        r.scalar.f64 = v;
        return r;
    }

    static Value make_composite(std::vector<Value> elems)
    {
        Value r;
        r.kind = Kind::Composite;
        r.elements = std::move(elems);
        return r;
    }

    static Value make_pointer(uint32_t sc, uint32_t base, std::vector<uint32_t> chain = {})
    {
        Value r;
        r.kind = Kind::Pointer;
        r.ptr_storage_class = sc;
        r.ptr_base_var = base;
        r.ptr_chain = std::move(chain);
        return r;
    }

    // Returns a zero value for the given bit-width and sign.
    static Value zero_int(uint32_t width, bool is_signed)
    {
        if (width <= 32)
            return is_signed ? make_i32(0) : make_u32(0);
        return is_signed ? make_i64(0) : make_u64(0);
    }
    static Value zero_float(uint32_t width)
    {
        return width <= 32 ? make_f32(0.0f) : make_f64(0.0);
    }
};

} // namespace spvdb
