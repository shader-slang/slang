#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace spvdb
{

struct SpvType
{
    enum class Kind
    {
        Void,
        Bool,
        Int,
        Float,
        Vector,
        Matrix,
        Array,
        RuntimeArray,
        Struct,
        Pointer,
        Function,
        Image,
        Sampler,
        SampledImage,
    };
    Kind kind;
    explicit SpvType(Kind k)
        : kind(k)
    {
    }
    virtual ~SpvType() = default;
};

struct VoidType : SpvType
{
    VoidType()
        : SpvType(Kind::Void)
    {
    }
};

struct BoolType : SpvType
{
    BoolType()
        : SpvType(Kind::Bool)
    {
    }
};

struct IntType : SpvType
{
    uint32_t width; // 8, 16, 32, or 64
    bool is_signed;
    IntType(uint32_t w, bool s)
        : SpvType(Kind::Int)
        , width(w)
        , is_signed(s)
    {
    }
};

struct FloatType : SpvType
{
    uint32_t width; // 16, 32, or 64
    explicit FloatType(uint32_t w)
        : SpvType(Kind::Float)
        , width(w)
    {
    }
};

struct VectorType : SpvType
{
    std::shared_ptr<SpvType> element_type;
    uint32_t count;
    VectorType(std::shared_ptr<SpvType> et, uint32_t c)
        : SpvType(Kind::Vector)
        , element_type(std::move(et))
        , count(c)
    {
    }
};

struct MatrixType : SpvType
{
    std::shared_ptr<SpvType> column_type; // must be VectorType
    uint32_t columns;
    MatrixType(std::shared_ptr<SpvType> ct, uint32_t c)
        : SpvType(Kind::Matrix)
        , column_type(std::move(ct))
        , columns(c)
    {
    }
};

struct ArrayType : SpvType
{
    std::shared_ptr<SpvType> element_type;
    uint32_t length; // 0 means runtime array
    uint32_t array_stride; // 0 if no ArrayStride decoration
    ArrayType(std::shared_ptr<SpvType> et, uint32_t len, uint32_t stride = 0)
        : SpvType(Kind::Array)
        , element_type(std::move(et))
        , length(len)
        , array_stride(stride)
    {
    }
};

struct RuntimeArrayType : SpvType
{
    std::shared_ptr<SpvType> element_type;
    uint32_t array_stride;
    RuntimeArrayType(std::shared_ptr<SpvType> et, uint32_t stride = 0)
        : SpvType(Kind::RuntimeArray)
        , element_type(std::move(et))
        , array_stride(stride)
    {
    }
};

struct StructMember
{
    std::string name;
    std::shared_ptr<SpvType> type;
    uint32_t offset = 0;
    uint32_t matrix_stride = 0;
    bool row_major = false;
};

struct StructType : SpvType
{
    std::string name;
    std::vector<StructMember> members;
    StructType()
        : SpvType(Kind::Struct)
    {
    }
};

struct PointerType : SpvType
{
    uint32_t storage_class;
    std::shared_ptr<SpvType> pointee;
    PointerType(uint32_t sc, std::shared_ptr<SpvType> p)
        : SpvType(Kind::Pointer)
        , storage_class(sc)
        , pointee(std::move(p))
    {
    }
};

struct FunctionType : SpvType
{
    std::shared_ptr<SpvType> return_type;
    std::vector<std::shared_ptr<SpvType>> param_types;
    FunctionType()
        : SpvType(Kind::Function)
    {
    }
};

struct ImageType : SpvType
{
    std::shared_ptr<SpvType> sampled_type;
    uint32_t dim;
    uint32_t depth;
    uint32_t arrayed;
    uint32_t ms;
    uint32_t sampled;
    uint32_t image_format;
    ImageType()
        : SpvType(Kind::Image)
        , dim(0)
        , depth(0)
        , arrayed(0)
        , ms(0)
        , sampled(0)
        , image_format(0)
    {
    }
};

struct SamplerType : SpvType
{
    SamplerType()
        : SpvType(Kind::Sampler)
    {
    }
};

struct SampledImageType : SpvType
{
    std::shared_ptr<SpvType> image_type;
    SampledImageType()
        : SpvType(Kind::SampledImage)
    {
    }
};

} // namespace spvdb
