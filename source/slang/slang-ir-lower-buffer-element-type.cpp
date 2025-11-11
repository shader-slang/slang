#include "slang-ir-lower-buffer-element-type.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

/// This file implements an important IR transformation pass in the Slang compiler
/// that rewrites buffer element types into valid storage types, a.k.a physical types
/// in SPIRV terminology.
///
/// Many of our targets have special restrictions on what is allowed to be used as a
/// buffer element. Examples are:
/// - In SPIRV, `bool` is considered a logical type, meaning it cannot appear inside
///   buffers. bool vectors and matrices needs to be lowered into arrays.
/// - In SPIRV, if `T` is used to declare a buffer, then every member in `T` must have
///   explicit offset. But if it is used to declare a local variable, then it cannot
///   have explicit member offset. This means that we cannot use the same `Foo` struct
///   inside a `StructuredBuffer<Foo>` and also use it to declare a local variable.
/// - In Metal, if we have a `struct Foo {Texture2D member; }` and
///   `ParameterBlock<Foo>`, then we should translate it to
///   `struct Foo_pb { Texture2D.Handle member; }` and `ParameterBlock<Foo_pb>`, so that
///    the resource legalization pass won't hoist the texture out of the parameter block.
///
/// We use the terms "physical", "storage", or "lowered" types to refer to types that
/// are legal to use as buffer elements. In contrast, the terms "original" or "logical"
/// refers to types that are declared by the user in its original form.
/// For example, `bool4` is a "logical" type, and its lowered type is `int4`.
///
///
/// # Algorithm Overview
/// ----------------------
///
/// This pass performs the transformation to create one "storage" type for each type that
/// are used in each kind of buffer. For example, if user defined `Foo`, and used it in
/// `ConstantBuffer<Foo>` and `StructuredBuffer<Foo>` and is targeting SPIRV, this pass will
/// create `Foo_std140` and `Foo_std430` types, and update the buffer to be
/// `ConstantBuffer<Foo_std140>` and `StructuredBuffer<Foo_std430>`.
///
/// The pass will rewrite all the code that uses this buffers, and insert translations between
/// Foo_std140/Foo_std430 and Foo to keep types consistent.
///
/// For example, given:
/// ```
/// struct Foo {
///     bool4x4 v;
/// }
/// ConstantBuffer<Foo> cb;
/// bool test(Foo f) {
///     return f.v[0][1];
/// }
/// void main() { test(cb); }
/// ```
///
/// This pass will rewrite it as:
/// ```
/// struct Foo {
///     bool4x4 v;
/// }
/// struct Foo_std140 {
///     Matrix_bool4x4_std140 v;
/// };
/// struct Matrix_bool4x4_std140 {
///     int4 values[4];
/// };
/// ConstantBuffer<Foo_std140> cb;
/// bool test_1(Foo_std140 f) {
///     return f.v.values[0][1];
/// }
/// void main() { test_1(cb); }
/// ```
///
/// Note that the one important optimization here is we will defer the translation from
/// storage type to logical type at latest possible time. In the example above, we could
/// have loaded `cb` and then immediately translate it into `Foo` and call `test` with
/// the translated value. However that can lead to code that create unnecessary copies
/// that can't always be removed by the downstream compiler, particulary if there are
/// arrays whose element type needs non-trivial translation.
///
/// To avoid the performance issue, we will defer this translation until a logical value
/// is actually needed. This is done by pushing the translation to the use sites, and
/// across function call boundaries, specializing any functions being called along the
/// chain. This case, since we are calling `test()` from `main()` with `Foo_std140`, instead
/// of converting the `Foo_std140` to `Foo` before the call, we create a specialization
/// of `test` that accepts `Foo_std140` instead.
///
/// To enable this interprecedural transformation, the pass is organized as two phases:
/// 1. Create lowered / storage types for all buffer element types, and update
///    global buffer declarations to use storage types. This is implemented in `processModule()`
/// 2. Insert a `CastStorageToLogical(loweredBuffer)` inst, and replace all uses of
///    `loweredBuffer` with the cast inst. This is implemented in `processModule()`
/// 3. Push the `CastStorageToLogical` insts to as late as possible, which means if we see
///    `FieldAddress(CastStorageToLogical(storageAddr), memberKey)`, we should translate
///    it into `CastStorageToLogical(FieldAddress(storageAddr, memberKey)`.
///    If we see a `CastStorageToLogical` inst being used as argument to call a function `f`,
///    specialize `f` to take a pointer to the storage type instead, and insert a
///    `CastStorageToLogical(param)` to convert the param type to logical type at the
///    beginning of the specialized function. (implemented in `deferStorageToLogicalCasts()`)
///
/// Repeat step 2 and 3 until no more changes can be made, then proceed to step 4.
///
/// 4. Materialize all remaining `CastStorageToLogical(addr)` by replacing all `load` of such
///    cast insts with `call unpackStorage(addr)`, where `unpackStorage` is a function we
///    synthesize that reads from an address of a storage type and returns a logical type;
///    and replacing all `store(CastStorageToLogical(addr), value)` with `packStorage(addr, value)`,
///    where `packStorage` is a function we synthesis that writes a logical value into a storage
///    addr. This is implemented in `materializeStorageToLogicalCasts()`.
///
/// That's the main idea of the pass.
///
/// # Propagating through SSA values
///
/// Note that `kIROp_CastStorageToLogical` is a pseudo instruction introduced in this pass that
/// has the semantics of "converting a pointer to a storage value into a pointer to a logical
/// value". A dual of this inst is `kIROp_CastStorageToLogicalDeref`, which has an additional
/// builtin "load" semantic. That is, given `Ptr<StorageType> addr`, `CastStorageToLogical(addr)`
/// will have type `Ptr<LogicalType>`, and `CastStorageToLogicalDeref(addr)` will have type
/// `LogicalType`. In other words, `CastStorageToLogicalDeref(addr)` is equivalent to
/// `load(CastStorageToLogical(addr))`.
///
/// The `CastStorageToLogicalDeref` pseudo inst is needed to push defer through `load`s.
/// Consider the following example:
/// ```
///    ptr : StorageType* = ...
///    lptr : LogicalType* = CastStorageToLogical(ptr);
///    l = load(lptr)
///    m = fieldExtract(l, member)
///    call f, m
/// ```
/// In this case, only l.member is used, so we should avoid translating other unrelated members
/// from storage type to logical type. To achieve this we must be able to push the
/// `CastStorageToLogical` operation beyond the `load`. The steps to achieve this are:
/// 1. we process `lptr` inst by inspecting its users. We find that a `load` (l) uses it.
/// 2. replace the `load` with `CastStorageToLogicalDeref(ptr)`, the IR become:
/// ```
///    ptr : StorageType* = ...
///    l_1 = CastStorageToLogicalDeref(ptr);
///    m = fieldExtract(l_1, member);
///    call f, m
/// ```
/// 3. push the new `l_1` inst to worklist, and when it gets processed, we continue to inspect
///    its users, and find that it is being used by `fieldExtract`. We will rewrite the
///    `fieldExtract` into `CastStorageToLogicalDeref(fieldAddr(ptr, member))`, and the IR become:
///    ```
///       ptr : StorageType* = ...
///       m_ptr = FieldAddr(ptr, member)
///       m = CastStorageToLogicalDeref(m_ptr);
///       call f, m
///    ```
/// 4. Since there are no more uses of `m` that can be translated, stop. Note that it is possible
///    to continue specializing `f` and replace its first parameter's type to storage type. However
///    this implementation currently does not specialize functions whose parameter type is not a
///    pointer/reference type. When we target SPIRV, we will already be running the
///    `transformParamsToConstRef` pass that would have converted `f` to take in `ConstRef<T>`.
///    In this case, the initial IR would be in the form of
///    ```
///       ptr : StorageType* = ...
///       lptr : LogicalType* = CastStorageToLogical(ptr);
///       l = load(lptr)
///       m = fieldExtract(l, member)
///       var tmpVar : MemberLogiocalType  [[ImmutableTempVar]]
///       store tmpVar, m
///       call f, tmpVar
///    ```
///    To allow us to remove the `tmpVar` store introduced during `transformParamsToConstRef`,
///    this pass also handles the propagation through temp var stores. After pushing the cast
///    through `m`, we will get IR to this form:
///    ```
///       ptr : StorageType* = ...
///       m_ptr = FieldAddr(ptr, member)
///       m = CastStorageToLogicalDeref(m_ptr);
///       var tmpVar : MemberLogiocalType  [[ImmutableTempVar]]
///       store tmpVar, m
///       call f, tmpVar
///    ```
///    This time, we will see that `m` is being used by a `store` into a `[[ImmutableTempVar]]` var,
///    and we can safely replace all uses of `tmpVar` to `m_ptr`, and therefore the IR will become:
///    ```
///       ptr : StorageType* = ...
///       m_ptr = FieldAddr(ptr, member)
///       m = CastStorageToLogical(m_ptr);
///       call f, m_ptr
///    ```
///    Now, we are in the case where a `CastStorageToLogical` is used as argument in a `call`.
///    This will trigger our function specialization rule to create `f_1` that accepets a
///    `StorageMember*`, and we will rewrite the IR again to:
///    ```
///       ptr : StorageType* = ...
///       m_ptr = FieldAddr(ptr, member)
///       call f_1, m_ptr
///    ```
///    Note that it is only correct to defer a load/CastStorageToLogicalDeref if the location
///    being loaded from is immutable. Otherwise, we might be changing the order of memory
///    operations and result in a change in application behavior. So this pass will also make sure
///    that we only create `CastStorageToLogicalDeref(x)` such that `x` is an immutable location,
///    such as an immutable temporary variable.
///
/// # Trailing Pointer Rewrite
///
/// Another transformation done in this pass is it also rewrites struct with unsized trailing
/// arrays. Since an unsized type isn't a physical type and cannot be used as a pointee type,
/// we will have problem translating the following code to SPIRV:
/// ```
/// struct Foo { int count; int[] values; }
/// uniform Foo* b;
/// ```
///
/// When we create a storage type for `Foo`, we will define it as:
/// ```
/// struct Foo_std430 { int count; }
/// ```
/// Where we removed the trailing array.
/// This makes `Foo_std430` an ordinary sized type that can be used freely as pointee type
/// in SPIRV.
///
/// However this does mean that we also need to translate things like `ptr->values[2]`
/// into `((int*)(ptr+1))[2]`. Which we also handle during step 2 of the algorithm.
/// (`maybeTranslateTrailingPointerGetElementAddress`)
///

namespace Slang
{

enum ConversionMethodKind
{
    Func,
    Opcode
};

struct ConversionMethod
{
    ConversionMethodKind kind = ConversionMethodKind::Func;
    union
    {
        IRFunc* func;
        IROp op;
    };
    ConversionMethod() { func = nullptr; }
    operator bool()
    {
        return kind == ConversionMethodKind::Func ? func != nullptr : op != kIROp_Nop;
    }
    ConversionMethod& operator=(IRFunc* f)
    {
        kind = ConversionMethodKind::Func;
        this->func = f;
        return *this;
    }
    ConversionMethod& operator=(IROp irop)
    {
        kind = ConversionMethodKind::Opcode;
        this->op = irop;
        return *this;
    }
    IRInst* apply(IRBuilder& builder, IRType* resultType, IRInst* operandAddr);
    void applyDestinationDriven(IRBuilder& builder, IRInst* dest, IRInst* operand);
};

struct TypeLoweringConfig
{
    AddressSpace addressSpace;
    IRTypeLayoutRuleName layoutRuleName;
    bool lowerToPhysicalType = true;

    static TypeLoweringConfig getLogicalTypeLoweringConfig(TypeLoweringConfig config)
    {
        TypeLoweringConfig result = config;
        result.lowerToPhysicalType = false;
        return result;
    }

    IRTypeLayoutRules* getLayoutRule() const { return IRTypeLayoutRules::get(layoutRuleName); }

    bool operator==(const TypeLoweringConfig& other) const
    {
        return addressSpace == other.addressSpace && layoutRuleName == other.layoutRuleName &&
               lowerToPhysicalType == other.lowerToPhysicalType;
    }
    HashCode getHashCode() const
    {
        return combineHash(
            Slang::getHashCode(addressSpace),
            Slang::getHashCode(layoutRuleName),
            Slang::getHashCode(lowerToPhysicalType));
    }
};

struct LoweredElementTypeInfo
{
    IRType* originalType;
    IRType* loweredType;
    IRType* loweredInnerArrayType =
        nullptr; // For matrix/array types that are lowered into a struct type, this is the
                 // inner array type of the data field.
    IRStructKey* loweredInnerStructKey =
        nullptr; // For matrix/array types that are lowered into a struct type, this is the
                 // struct key of the data field.
    ConversionMethod convertOriginalToLowered;
    ConversionMethod convertLoweredToOriginal;
};

/// Defines target-specific behavior of how to lower buffer element types.
struct BufferElementTypeLoweringPolicy : public RefObject
{
    /// Defines target-specific behavior of how to translate a non-composite logical type to a
    /// storage type.
    virtual LoweredElementTypeInfo lowerLeafLogicalType(
        IRType* type,
        TypeLoweringConfig config) = 0;

    /// Returns true if we should always create a fresh lowered storage type for a composite type,
    /// even if every member/element of the composite type is not changed by the lowering.
    virtual bool shouldAlwaysCreateLoweredStorageTypeForCompositeTypes(TypeLoweringConfig config)
    {
        SLANG_UNUSED(config);
        return false;
    }

    /// Returns true if the target requires all array of scalars or vectors inside a constant buffer
    /// to be translated into a 16-byte aligned vector type.
    virtual bool shouldTranslateArrayElementTo16ByteAlignedVectorForConstantBuffer()
    {
        return false;
    }

    /// Returns true if the target allows declaring a local var in Function address space with a
    /// StorageType that may have explicit layout. This is currently true for all targets except
    /// SPIRV.
    virtual bool canUseStorageTypeInLocalVar() { return true; }
};

BufferElementTypeLoweringPolicy* getBufferElementTypeLoweringPolicy(
    BufferElementTypeLoweringPolicyKind kind,
    TargetProgram* target,
    BufferElementTypeLoweringOptions options);

TypeLoweringConfig getTypeLoweringConfigForBuffer(TargetProgram* target, IRType* bufferType);

IRInst* ConversionMethod::apply(IRBuilder& builder, IRType* resultType, IRInst* operandAddr)
{
    if (!*this)
        return builder.emitLoad(operandAddr);
    if (kind == ConversionMethodKind::Func)
        return builder.emitCallInst(resultType, func, 1, &operandAddr);
    else
    {
        auto val = builder.emitLoad(operandAddr);
        return builder.emitIntrinsicInst(resultType, op, 1, &val);
    }
}

void ConversionMethod::applyDestinationDriven(IRBuilder& builder, IRInst* dest, IRInst* operand)
{
    if (!*this)
    {
        builder.emitStore(dest, operand);
        return;
    }
    if (kind == ConversionMethodKind::Func)
    {
        IRInst* operands[] = {dest, operand};
        builder.emitCallInst(builder.getVoidType(), func, 2, operands);
    }
    else
    {
        auto val = builder.emitIntrinsicInst(
            tryGetPointedToOrBufferElementType(&builder, dest->getDataType()),
            op,
            1,
            &operand);
        builder.emitStore(dest, val);
    }
}

// Returns the number of elements N that ensures the IRVectorType(elementType,N)
// has 16-byte aligned size and N is no less than `minCount`.
IRIntegerValue get16ByteAlignedVectorElementCount(
    TargetProgram* target,
    IRType* elementType,
    IRIntegerValue minCount)
{
    IRSizeAndAlignment sizeAlignment;
    getNaturalSizeAndAlignment(target->getOptionSet(), elementType, &sizeAlignment);
    if (sizeAlignment.size)
        return align(sizeAlignment.size * minCount, 16) / sizeAlignment.size;
    return 4;
}

const char* getLayoutName(IRTypeLayoutRuleName name)
{
    switch (name)
    {
    case IRTypeLayoutRuleName::Std140:
        return "std140";
    case IRTypeLayoutRuleName::Std430:
        return "std430";
    case IRTypeLayoutRuleName::Natural:
        return "natural";
    case IRTypeLayoutRuleName::C:
        return "c";
    default:
        return "default";
    }
}

void maybeAddPhysicalTypeDecoration(IRBuilder& builder, IRInst* type, TypeLoweringConfig config)
{
    if (config.lowerToPhysicalType)
        builder.addPhysicalTypeDecoration(type);
}

struct LoweredElementTypeContext
{
    static const IRIntegerValue kMaxArraySizeToUnroll = 32;

    struct LoweredTypeMap : RefObject
    {
        Dictionary<IRType*, LoweredElementTypeInfo> loweredTypeInfo;
        Dictionary<IRType*, LoweredElementTypeInfo> mapLoweredTypeToInfo;
    };

    Dictionary<TypeLoweringConfig, RefPtr<LoweredTypeMap>> loweredTypeInfoMaps;
    RefPtr<BufferElementTypeLoweringPolicy> leafTypeLoweringPolicy;

    struct ConversionMethodKey
    {
        IRType* toType;
        IRType* fromType;
        bool operator==(const ConversionMethodKey& other) const
        {
            return toType == other.toType && fromType == other.fromType;
        }
        HashCode64 getHashCode() const
        {
            return combineHash(Slang::getHashCode(toType), Slang::getHashCode(fromType));
        }
    };

    Dictionary<ConversionMethodKey, ConversionMethod> conversionMethodMap;
    ConversionMethod getConversionMethod(IRType* toType, IRType* fromType)
    {
        ConversionMethodKey key;
        key.toType = toType;
        key.fromType = fromType;
        ConversionMethod method;
        conversionMethodMap.tryGetValue(key, method);
        return method;
    }

    SlangMatrixLayoutMode defaultMatrixLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    TargetProgram* target;
    BufferElementTypeLoweringOptions options;

    struct SpecializationKey
    {
        IRFunc* callee;
        IRFuncType* specializedFuncType;
        bool operator==(const SpecializationKey& other) const
        {
            return (callee == other.callee && specializedFuncType == other.specializedFuncType);
        }
        HashCode64 getHashCode() const
        {
            return combineHash(Slang::getHashCode(callee), Slang::getHashCode(specializedFuncType));
        }
    };
    // Specialized functions that takes storage-typed pointers instead of logical-typed pointers.
    Dictionary<SpecializationKey, IRFunc*> specializedFuncs;

    LoweredElementTypeContext(TargetProgram* target, BufferElementTypeLoweringOptions inOptions)
        : target(target), options(inOptions)
    {
        leafTypeLoweringPolicy =
            getBufferElementTypeLoweringPolicy(options.loweringPolicyKind, target, options);
    }

    IRFunc* createArrayUnpackFunc(
        IRArrayType* arrayType,
        IRStructType* structType,
        IRStructKey* dataKey,
        LoweredElementTypeInfo innerTypeInfo)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto refStructType = builder.getRefParamType(structType, AddressSpace::Generic);
        auto funcType = builder.getFuncType(1, (IRType**)&refStructType, arrayType);
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("unpackStorage"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto packedParam = builder.emitParam(refStructType);
        auto packedArray = builder.emitFieldAddress(packedParam, dataKey);
        auto count = getArraySizeVal(arrayType->getElementCount());
        IRInst* result = nullptr;
        if (count <= kMaxArraySizeToUnroll)
        {
            // If the array is small enough, just process each element directly.
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto packedElementAddr = builder.emitElementAddress(packedArray, ii);
                auto originalElement = innerTypeInfo.convertLoweredToOriginal.apply(
                    builder,
                    innerTypeInfo.originalType,
                    packedElementAddr);
                args[(Index)ii] = originalElement;
            }
            result = builder.emitMakeArray(arrayType, (UInt)args.getCount(), args.getBuffer());
        }
        else
        {
            // The general case for large arrays is to emit a loop through the elements.
            IRVar* resultVar = builder.emitVar(arrayType);
            IRBlock* loopBodyBlock;
            IRBlock* loopBreakBlock;
            auto loopParam = emitLoopBlocks(
                &builder,
                builder.getIntValue(builder.getIntType(), 0),
                builder.getIntValue(builder.getIntType(), count),
                loopBodyBlock,
                loopBreakBlock);

            builder.setInsertBefore(loopBodyBlock->getFirstOrdinaryInst());
            auto packedElementAddr = builder.emitElementAddress(packedArray, loopParam);
            auto originalElement = innerTypeInfo.convertLoweredToOriginal.apply(
                builder,
                innerTypeInfo.originalType,
                packedElementAddr);
            auto varPtr = builder.emitElementAddress(resultVar, loopParam);
            builder.emitStore(varPtr, originalElement);
            builder.setInsertInto(loopBreakBlock);
            result = builder.emitLoad(resultVar);
        }
        builder.emitReturn(result);
        return func;
    }

    IRFunc* createArrayPackFunc(
        IRArrayType* arrayType,
        IRStructType* structType,
        IRStructKey* arrayStructKey,
        LoweredElementTypeInfo innerTypeInfo)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto outLoweredType = builder.getRefParamType(structType, AddressSpace::Generic);
        IRType* paramTypes[] = {outLoweredType, structType};
        auto funcType = builder.getFuncType(2, paramTypes, builder.getVoidType());
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("packStorage"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto outParam = builder.emitParam(outLoweredType);
        auto originalParam = builder.emitParam(arrayType);
        auto count = getArraySizeVal(arrayType->getElementCount());
        auto destArray = builder.emitFieldAddress(outParam, arrayStructKey);
        if (count <= kMaxArraySizeToUnroll)
        {
            // If the array is small enough, just process each element directly.
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto originalElement = builder.emitElementExtract(originalParam, ii);
                auto destArrayElement = builder.emitElementAddress(destArray, ii);
                innerTypeInfo.convertOriginalToLowered.applyDestinationDriven(
                    builder,
                    destArrayElement,
                    originalElement);
            }
        }
        else
        {
            // The general case for large arrays is to emit a loop through the elements.
            IRBlock* loopBodyBlock;
            IRBlock* loopBreakBlock;
            auto loopParam = emitLoopBlocks(
                &builder,
                builder.getIntValue(builder.getIntType(), 0),
                builder.getIntValue(builder.getIntType(), count),
                loopBodyBlock,
                loopBreakBlock);

            builder.setInsertBefore(loopBodyBlock->getFirstOrdinaryInst());
            auto originalElement = builder.emitElementExtract(originalParam, loopParam);
            auto varPtr = builder.emitElementAddress(destArray, loopParam);
            innerTypeInfo.convertOriginalToLowered.applyDestinationDriven(
                builder,
                varPtr,
                originalElement);
            builder.setInsertInto(loopBreakBlock);
        }
        builder.emitReturn();
        return func;
    }

    LoweredElementTypeInfo getLoweredTypeInfoImpl(IRType* type, TypeLoweringConfig config)
    {
        IRBuilder builder(type);
        builder.setInsertAfter(type);

        LoweredElementTypeInfo info;
        info.originalType = type;
        if (auto arrayTypeBase = as<IRArrayTypeBase>(type))
        {
            auto loweredInnerTypeInfo = getLoweredTypeInfo(arrayTypeBase->getElementType(), config);

            if (config.layoutRuleName == IRTypeLayoutRuleName::Std140 &&
                leafTypeLoweringPolicy
                    ->shouldTranslateArrayElementTo16ByteAlignedVectorForConstantBuffer())
            {
                // For constant buffer layout, we need to use 16-byte-aligned vector if
                // we are required to ensure array element types has 16-byte stride.
                // We only need to handle the case where the element type is a scalar or vector
                // type here, because if the element type is a matrix type or struct type,
                // the size promotion will be handled during lowering of the element type.
                IRType* packedVectorType = nullptr;
                if (auto vectorType = as<IRVectorType>(loweredInnerTypeInfo.loweredType))
                {
                    packedVectorType = builder.getVectorType(
                        vectorType->getElementType(),
                        builder.getIntValue(get16ByteAlignedVectorElementCount(
                            target,
                            vectorType->getElementType(),
                            getIntVal(vectorType->getElementCount()))));
                    if (packedVectorType != loweredInnerTypeInfo.originalType)
                    {
                        loweredInnerTypeInfo.convertLoweredToOriginal = kIROp_VectorReshape;
                        loweredInnerTypeInfo.convertOriginalToLowered = kIROp_VectorReshape;
                    }
                }
                else if (auto scalarType = as<IRBasicType>(loweredInnerTypeInfo.loweredType))
                {
                    packedVectorType = builder.getVectorType(
                        loweredInnerTypeInfo.loweredType,
                        get16ByteAlignedVectorElementCount(target, scalarType, 1));
                    loweredInnerTypeInfo.convertLoweredToOriginal = kIROp_VectorReshape;
                    loweredInnerTypeInfo.convertOriginalToLowered = kIROp_MakeVectorFromScalar;
                }
                if (packedVectorType)
                {
                    loweredInnerTypeInfo.loweredType = packedVectorType;
                    if (loweredInnerTypeInfo.convertLoweredToOriginal)
                        conversionMethodMap[ConversionMethodKey{
                            packedVectorType,
                            loweredInnerTypeInfo.originalType}] =
                            loweredInnerTypeInfo.convertOriginalToLowered;
                    if (loweredInnerTypeInfo.convertOriginalToLowered)
                        conversionMethodMap[ConversionMethodKey{
                            loweredInnerTypeInfo.originalType,
                            packedVectorType}] = loweredInnerTypeInfo.convertLoweredToOriginal;
                }
            }

            // We can skip lowering this type if all field types are unchanged, unless the target
            // specific policy tells us to always create a lowered type.
            if (!leafTypeLoweringPolicy->shouldAlwaysCreateLoweredStorageTypeForCompositeTypes(
                    config))
            {
                if (!loweredInnerTypeInfo.convertLoweredToOriginal)
                {
                    info.loweredType = type;
                    return info;
                }
            }

            bool needExplicitLayout = config.lowerToPhysicalType;
            auto arrayType = as<IRArrayType>(arrayTypeBase);
            if (arrayType)
            {
                auto loweredType = builder.createStructType();
                maybeAddPhysicalTypeDecoration(builder, loweredType, config);

                info.loweredType = loweredType;
                StringBuilder nameSB;
                nameSB << "_Array_" << getLayoutName(config.layoutRuleName) << "_";
                getTypeNameHint(nameSB, arrayType->getElementType());
                if (!config.lowerToPhysicalType)
                    nameSB << "_logical";
                nameSB << getArraySizeVal(arrayType->getElementCount());

                builder.addNameHintDecoration(
                    loweredType,
                    nameSB.produceString().getUnownedSlice());
                auto structKey = builder.createStructKey();
                builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
                IRSizeAndAlignment elementSizeAlignment;
                getSizeAndAlignment(
                    target->getOptionSet(),
                    config.getLayoutRule(),
                    loweredInnerTypeInfo.loweredType,
                    &elementSizeAlignment);
                elementSizeAlignment =
                    config.getLayoutRule()->alignCompositeElement(elementSizeAlignment);
                auto innerArrayType = builder.getArrayType(
                    loweredInnerTypeInfo.loweredType,
                    arrayType->getElementCount(),
                    needExplicitLayout ? builder.getIntValue(
                                             builder.getIntType(),
                                             elementSizeAlignment.getStride())
                                       : nullptr);
                builder.createStructField(loweredType, structKey, innerArrayType);
                info.loweredInnerArrayType = innerArrayType;
                info.loweredInnerStructKey = structKey;
                info.convertLoweredToOriginal =
                    createArrayUnpackFunc(arrayType, loweredType, structKey, loweredInnerTypeInfo);
                info.convertOriginalToLowered =
                    createArrayPackFunc(arrayType, loweredType, structKey, loweredInnerTypeInfo);
            }
            else
            {
                IRSizeAndAlignment elementSizeAlignment;
                getSizeAndAlignment(
                    target->getOptionSet(),
                    config.getLayoutRule(),
                    loweredInnerTypeInfo.loweredType,
                    &elementSizeAlignment);
                elementSizeAlignment =
                    config.getLayoutRule()->alignCompositeElement(elementSizeAlignment);
                auto innerArrayType = builder.getArrayTypeBase(
                    arrayTypeBase->getOp(),
                    loweredInnerTypeInfo.loweredType,
                    nullptr,
                    needExplicitLayout ? builder.getIntValue(
                                             builder.getIntType(),
                                             elementSizeAlignment.getStride())
                                       : nullptr);
                maybeAddPhysicalTypeDecoration(builder, innerArrayType, config);
                info.loweredType = innerArrayType;
            }
            return info;
        }
        else if (auto structType = as<IRStructType>(type))
        {
            List<LoweredElementTypeInfo> fieldLoweredTypeInfo;
            bool isTrivial = true;
            for (auto field : structType->getFields())
            {
                auto loweredFieldTypeInfo = getLoweredTypeInfo(field->getFieldType(), config);
                fieldLoweredTypeInfo.add(loweredFieldTypeInfo);
                if (loweredFieldTypeInfo.convertLoweredToOriginal ||
                    config.layoutRuleName != IRTypeLayoutRuleName::Natural)
                    isTrivial = false;
            }

            // We can skip lowering this type if all field types are unchanged, unless the target
            // specific policy tells us to always create a lowered type.
            if (!leafTypeLoweringPolicy->shouldAlwaysCreateLoweredStorageTypeForCompositeTypes(
                    config))
            {
                if (isTrivial)
                {
                    info.loweredType = type;
                    return info;
                }
            }
            auto loweredType = builder.createStructType();
            maybeAddPhysicalTypeDecoration(builder, loweredType, config);

            StringBuilder nameSB;
            getTypeNameHint(nameSB, type);
            nameSB << "_" << getLayoutName(config.layoutRuleName);
            if (!config.lowerToPhysicalType)
                nameSB << "_logical";
            builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
            info.loweredType = loweredType;
            // Create fields.
            {
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    auto& loweredFieldTypeInfo = fieldLoweredTypeInfo[fieldId];
                    // When lowering type for user pointer, skip fields that are unsized array.
                    if (config.addressSpace == AddressSpace::UserPointer &&
                        as<IRUnsizedArrayType>(loweredFieldTypeInfo.loweredType))
                    {
                        fieldId++;
                        loweredFieldTypeInfo.loweredType = builder.getVoidType();
                        continue;
                    }
                    builder.createStructField(
                        loweredType,
                        field->getKey(),
                        loweredFieldTypeInfo.loweredType);
                    fieldId++;
                }
            }

            // Create unpack func.
            {
                builder.setInsertAfter(loweredType);
                info.convertLoweredToOriginal = builder.createFunc();
                builder.setInsertInto(info.convertLoweredToOriginal.func);
                builder.addNameHintDecoration(
                    info.convertLoweredToOriginal.func,
                    UnownedStringSlice("unpackStorage"));
                builder.addForceInlineDecoration(info.convertLoweredToOriginal.func);
                auto refLoweredType = builder.getRefParamType(loweredType, AddressSpace::Generic);
                info.convertLoweredToOriginal.func->setFullType(
                    builder.getFuncType(1, (IRType**)&refLoweredType, type));
                builder.emitBlock();
                auto loweredParam = builder.emitParam(refLoweredType);
                List<IRInst*> args;
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    if (as<IRVoidType>(fieldLoweredTypeInfo[fieldId].loweredType))
                    {
                        fieldId++;
                        continue;
                    }
                    auto storageField = builder.emitFieldAddress(loweredParam, field->getKey());
                    auto unpackedField =
                        fieldLoweredTypeInfo[fieldId].convertLoweredToOriginal.apply(
                            builder,
                            field->getFieldType(),
                            storageField);
                    args.add(unpackedField);
                    fieldId++;
                }
                auto result = builder.emitMakeStruct(type, args);
                builder.emitReturn(result);
            }

            // Create pack func.
            {
                builder.setInsertAfter(info.convertLoweredToOriginal.func);
                info.convertOriginalToLowered = builder.createFunc();
                builder.setInsertInto(info.convertOriginalToLowered.func);
                builder.addNameHintDecoration(
                    info.convertOriginalToLowered.func,
                    UnownedStringSlice("packStorage"));
                builder.addForceInlineDecoration(info.convertOriginalToLowered.func);

                auto outLoweredType = builder.getRefParamType(loweredType, AddressSpace::Generic);
                IRType* paramTypes[] = {outLoweredType, type};
                info.convertOriginalToLowered.func->setFullType(
                    builder.getFuncType(2, paramTypes, builder.getVoidType()));
                builder.emitBlock();
                auto outParam = builder.emitParam(outLoweredType);
                auto param = builder.emitParam(type);
                List<IRInst*> args;
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    if (as<IRVoidType>(fieldLoweredTypeInfo[fieldId].loweredType))
                    {
                        fieldId++;
                        continue;
                    }
                    auto fieldVal =
                        builder.emitFieldExtract(field->getFieldType(), param, field->getKey());
                    auto destAddr = builder.emitFieldAddress(outParam, field->getKey());

                    fieldLoweredTypeInfo[fieldId].convertOriginalToLowered.applyDestinationDriven(
                        builder,
                        destAddr,
                        fieldVal);
                    fieldId++;
                }
                builder.emitReturn();
            }

            return info;
        }
        return leafTypeLoweringPolicy->lowerLeafLogicalType(type, config);
    }

    LoweredTypeMap& getTypeLoweringMap(TypeLoweringConfig config)
    {
        RefPtr<LoweredTypeMap> map;
        if (loweredTypeInfoMaps.tryGetValue(config, map))
            return *map;
        map = new LoweredTypeMap();
        loweredTypeInfoMaps.add(config, map);
        return *map;
    }

    LoweredElementTypeInfo getLoweredTypeInfo(IRType* type, TypeLoweringConfig config)
    {
        // If `type` is already a lowered type, no more lowering is required.
        LoweredElementTypeInfo info;
        auto& map = getTypeLoweringMap(config);
        auto& mapLoweredTypeToInfo = map.mapLoweredTypeToInfo;
        auto& loweredTypeInfo = map.loweredTypeInfo;
        if (mapLoweredTypeToInfo.tryGetValue(type))
        {
            info.originalType = type;
            info.loweredType = type;
            return info;
        }
        if (loweredTypeInfo.tryGetValue(type, info))
            return info;
        info = getLoweredTypeInfoImpl(type, config);
        IRSizeAndAlignment sizeAlignment;
        getSizeAndAlignment(
            target->getOptionSet(),
            config.getLayoutRule(),
            info.loweredType,
            &sizeAlignment);
        loweredTypeInfo.set(type, info);
        mapLoweredTypeToInfo.set(info.loweredType, info);
        conversionMethodMap[{info.originalType, info.loweredType}] = info.convertLoweredToOriginal;
        conversionMethodMap[{info.loweredType, info.originalType}] = info.convertOriginalToLowered;
        return info;
    }

    IRType* getLoweredPtrLikeType(IRType* originalPtrLikeType, IRType* newElementType)
    {
        IRBuilder builder(newElementType);
        builder.setInsertAfter(newElementType);
        if (auto ptrType = as<IRPtrTypeBase>(originalPtrLikeType))
        {
            return builder.getPtrType(newElementType, ptrType);
        }

        if (as<IRPointerLikeType>(originalPtrLikeType) ||
            as<IRHLSLStructuredBufferTypeBase>(originalPtrLikeType) ||
            as<IRGLSLShaderStorageBufferType>(originalPtrLikeType))
        {
            ShortList<IRInst*> operands;
            operands.add(newElementType);
            for (UInt i = 1; i < originalPtrLikeType->getOperandCount(); i++)
            {
                operands.add(originalPtrLikeType->getOperand(i));
            }
            return (IRType*)builder.emitIntrinsicInst(
                builder.getTypeKind(),
                originalPtrLikeType->getOp(),
                (UInt)operands.getCount(),
                operands.getArrayView().getBuffer());
        }
        SLANG_UNREACHABLE("unhandled ptr like or buffer type");
    }

    IRInst* getStoreVal(IRInst* storeInst)
    {
        if (auto store = as<IRStore>(storeInst))
            return store->getVal();
        else if (auto sbStore = as<IRRWStructuredBufferStore>(storeInst))
            return sbStore->getVal();
        return nullptr;
    }

    struct MatrixAddrWorkItem
    {
        IRInst* matrixAddrInst;
        TypeLoweringConfig config;
    };

    IRInst* getBufferAddr(IRBuilder& builder, IRInst* loadStoreInst, IRInst* baseAddr)
    {
        switch (loadStoreInst->getOp())
        {
        case kIROp_Load:
        case kIROp_Store:
            return baseAddr;
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_RWStructuredBufferLoadStatus:
        case kIROp_RWStructuredBufferStore:
            return builder.emitRWStructuredBufferGetElementPtr(
                baseAddr,
                loadStoreInst->getOperand(1));
        default:
            return nullptr;
        }
    }

    bool maybeTranslateTrailingPointerGetElementAddress(
        IRBuilder& builder,
        IRFieldAddress* fieldAddr,
        IRCastStorageToLogicalBase* castInst,
        TypeLoweringConfig& config,
        List<IRCastStorageToLogicalBase*>& castInstWorkList)
    {
        // If we are accessing an unsized array element from a pointer, we need to
        // compute
        // the trailing ptr that points to the first element of the array.
        // And then replace all getElementPtr(arrayPtr, index) with
        // getOffsetPtr(trailingPtr, index).

        auto ptrType = as<IRPtrTypeBase>(fieldAddr->getDataType());
        if (!ptrType)
            return false;
        if (ptrType->getAddressSpace() != AddressSpace::UserPointer)
            return false;
        if (auto unsizedArrayType = as<IRUnsizedArrayType>(ptrType->getValueType()))
        {
            builder.setInsertBefore(fieldAddr);
            auto newArrayPtrVal = fieldAddr->getBase();
            auto loweredInnerType = getLoweredTypeInfo(unsizedArrayType->getElementType(), config);

            IRSizeAndAlignment arrayElementSizeAlignment;
            getSizeAndAlignment(
                target->getOptionSet(),
                config.getLayoutRule(),
                loweredInnerType.loweredType,
                &arrayElementSizeAlignment);
            IRSizeAndAlignment baseSizeAlignment;
            getSizeAndAlignment(
                target->getOptionSet(),
                config.getLayoutRule(),
                tryGetPointedToOrBufferElementType(&builder, fieldAddr->getBase()->getDataType()),
                &baseSizeAlignment);

            // Convert pointer to uint64 and adjust offset.
            IRIntegerValue offset = baseSizeAlignment.size;
            offset = align(offset, arrayElementSizeAlignment.alignment);
            if (offset != 0)
            {
                auto rawPtr = builder.emitBitCast(builder.getUInt64Type(), newArrayPtrVal);
                newArrayPtrVal = builder.emitAdd(
                    rawPtr->getFullType(),
                    rawPtr,
                    builder.getIntValue(builder.getUInt64Type(), offset));
            }
            newArrayPtrVal = builder.emitBitCast(
                builder.getPtrType(loweredInnerType.loweredType, ptrType),
                newArrayPtrVal);
            traverseUses(
                fieldAddr,
                [&](IRUse* fieldAddrUse)
                {
                    auto fieldAddrUser = fieldAddrUse->getUser();
                    if (fieldAddrUser->getOp() == kIROp_GetElementPtr)
                    {
                        builder.setInsertBefore(fieldAddrUser);
                        auto newElementPtr =
                            builder.emitGetOffsetPtr(newArrayPtrVal, fieldAddrUser->getOperand(1));
                        auto castedGEP = builder.emitCastStorageToLogical(
                            fieldAddrUser->getFullType(),
                            newElementPtr,
                            castInst->getLayoutConfig());
                        fieldAddrUser->replaceUsesWith(castedGEP);
                        fieldAddrUser->removeAndDeallocate();
                        if (auto castStorage = as<IRCastStorageToLogicalBase>(castedGEP))
                            castInstWorkList.add(castStorage);
                    }
                    else if (fieldAddrUser->getOp() == kIROp_GetOffsetPtr)
                    {
                    }
                    else
                    {
                        SLANG_UNEXPECTED("unknown use of pointer to unsized array.");
                    }
                });
            SLANG_ASSERT(!fieldAddr->hasUses());
            fieldAddr->removeAndDeallocate();
            return true;
        }
        return false;
    }


    // Helper function to discover all `call`s in `func` that has at least one argument
    // that is `CastStorageToPhysical`.
    void discoverCallsToProcess(List<IRCall*>& callWorkList, IRFunc* func)
    {
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto call = as<IRCall>(inst);
                if (!call)
                    continue;
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    auto arg = call->getArg(i);
                    if (arg->getOp() == kIROp_CastStorageToLogical)
                    {
                        callWorkList.add(call);
                        break;
                    }
                }
            }
        }
    }

    // Given a load/structured-buffer-load instruction `loadInst`, and a new base pointer
    // `newBasePtr`, return the source address that can be used to represent the source address
    // of the load instruction, but with `newBasePtr` as the base structured buffer pointer.
    IRInst* getSourceAddrFromLoadInstWithNewBase(
        IRBuilder& builder,
        IRInst* loadInst,
        IRInst* newBasePtr)
    {
        switch (loadInst->getOp())
        {
        case kIROp_Load:
            // A simple load does not have any extra access chain info, so we
            // can simply return the new base pointer.
            return newBasePtr;
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_RWStructuredBufferLoadStatus:
            // For structured buffer loads, the new address can be obtained by
            // getting the element pointer from the new base pointer and the original
            // index operand.
            return builder.emitRWStructuredBufferGetElementPtr(newBasePtr, loadInst->getOperand(1));
        default:
            return nullptr;
        }
    }

    IRMakeStorageTypeLoweringConfig* emitTypeLoweringConfigToIR(
        IRBuilder& builder,
        TypeLoweringConfig config)
    {
        return builder.emitMakeStorageTypeLoweringConfig(
            config.addressSpace,
            config.layoutRuleName,
            config.lowerToPhysicalType);
    }

    TypeLoweringConfig getTypeLoweringConfigFromInst(IRMakeStorageTypeLoweringConfig* inst)
    {
        SLANG_ASSERT(inst);
        TypeLoweringConfig config;
        config.addressSpace = (AddressSpace)getIntVal(inst->getAddressSpace());
        config.layoutRuleName = (IRTypeLayoutRuleName)getIntVal(inst->getLayoutRule());
        config.lowerToPhysicalType = getIntVal(inst->getLowerToPhysicalType()) != 0;
        return config;
    }

    void deferStorageToLogicalCasts(
        IRModule* module,
        List<IRCastStorageToLogicalBase*> castInstWorkList)
    {
        IRBuilder builder(module);

        while (castInstWorkList.getCount())
        {
            // We process call instructions after other instructions, so we
            // can be sure that all castStorageToLogical insts have already
            // been pushed to the call argument lists before we process it.
            HashSet<IRCall*> callWorkListSet;
            // Defer the storage-to-logical cast operation to latest possible time to avoid
            // unnecessary packing/unpacking.
            for (Index i = 0; i < castInstWorkList.getCount(); i++)
            {
                auto castInst = castInstWorkList[i];
                auto ptrVal = castInst->getOperand(0);
                auto config = getTypeLoweringConfigFromInst(castInst->getLayoutConfig());
                traverseUses(
                    castInst,
                    [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        switch (user->getOp())
                        {
                        case kIROp_FieldAddress:
                            if (!isUseBaseAddrOperand(use, user))
                                break;
                            // If our logical struct type ends with an unsized array field, the
                            // storage struct type won't have this field defined.
                            // Therefore, all fieldAddress(obj, lastField) inst retrieving the last
                            // field of such struct should be translated into
                            // `(ArrayElementType*)((StorageStruct*)(obj)+1) + idx`.
                            // That is, we should first compute the tailing pointer of the
                            // struct, and replace all getElementPtr(fieldAddr, idx) with
                            // getOffsetPtr(tailingPtr, idx).
                            if (maybeTranslateTrailingPointerGetElementAddress(
                                    builder,
                                    (IRFieldAddress*)user,
                                    castInst,
                                    config,
                                    castInstWorkList))
                                return;
                            [[fallthrough]];
                        case kIROp_GetElementPtr:
                        case kIROp_GetOffsetPtr:
                        case kIROp_RWStructuredBufferGetElementPtr:
                            {
                                // gep(castStorageToLogical(x)) ==> castStorageToLogical(gep(x))
                                if (!isUseBaseAddrOperand(use, user))
                                    break;
                                auto logicalBaseType = castInst->getDataType();
                                auto logicalType = user->getDataType();
                                IRInst* storageBaseAddr = ptrVal;
                                auto originalBaseValueType =
                                    tryGetPointedToOrBufferElementType(&builder, logicalBaseType);
                                if (user->getOp() == kIROp_GetElementPtr)
                                {
                                    // If original type is an array, the lowered type will be a
                                    // struct. In that case, all existing address insts should be
                                    // appended with a field extract.
                                    if (as<IRArrayType>(originalBaseValueType))
                                    {
                                        auto arrayLowerInfo =
                                            getLoweredTypeInfo(originalBaseValueType, config);
                                        if (arrayLowerInfo.loweredInnerArrayType)
                                        {
                                            builder.setInsertBefore(user);
                                            List<IRInst*> args;
                                            for (UInt i = 0; i < user->getOperandCount(); i++)
                                                args.add(user->getOperand(i));
                                            storageBaseAddr = builder.emitFieldAddress(
                                                builder.getPtrType(
                                                    arrayLowerInfo.loweredInnerArrayType),
                                                ptrVal,
                                                arrayLowerInfo.loweredInnerStructKey);
                                        }
                                    }
                                    if (as<IRMatrixType>(originalBaseValueType))
                                    {
                                        // We are tring to get a pointer to a lowered matrix
                                        // element. We process this insts at a later phase.
                                        SLANG_ASSERT(user->getOp() == kIROp_GetElementPtr);
                                        lowerMatrixAddresses(
                                            module,
                                            MatrixAddrWorkItem{user, config});
                                        break;
                                    }
                                }


                                builder.setInsertBefore(user);
                                IRInst* storageGEP = nullptr;
                                switch (user->getOp())
                                {
                                case kIROp_GetElementPtr:
                                case kIROp_FieldAddress:
                                    {
                                        // For standard gep instructions, use the
                                        // IR builder to auto-deduce result type
                                        // of the new GEP inst.
                                        ShortList<IRInst*> newArgs;
                                        for (UInt i = 1; i < user->getOperandCount(); i++)
                                            newArgs.add(user->getOperand(i));
                                        storageGEP = builder.emitElementAddress(
                                            storageBaseAddr,
                                            newArgs.getArrayView().arrayView);
                                        break;
                                    }
                                default:
                                    {
                                        // For non-standard gep instructions, e.g.
                                        // RWStructuredBufferGetElementPtr,
                                        // manually create the inst here.
                                        ShortList<IRInst*> newArgs;
                                        newArgs.add(storageBaseAddr);
                                        for (UInt i = 1; i < user->getOperandCount(); i++)
                                            newArgs.add(user->getOperand(i));
                                        auto logicalValueType = tryGetPointedToOrBufferElementType(
                                            &builder,
                                            logicalType);
                                        auto storageTypeInfo =
                                            getLoweredTypeInfo(logicalValueType, config);
                                        storageGEP = builder.emitIntrinsicInst(
                                            builder.getPtrType(storageTypeInfo.loweredType),
                                            user->getOp(),
                                            newArgs.getCount(),
                                            newArgs.getArrayView().getBuffer());
                                        break;
                                    }
                                }
                                auto castOfGEP = builder.emitCastStorageToLogical(
                                    logicalType,
                                    storageGEP,
                                    castInst->getLayoutConfig());
                                user->replaceUsesWith(castOfGEP);
                                user->removeAndDeallocate();
                                if (auto castStorage = as<IRCastStorageToLogical>(castOfGEP))
                                    castInstWorkList.add(castStorage);
                                break;
                            }
                        case kIROp_Call:
                            {
                                // call(f, castStorageToLogical(x)) ==> call(f', x)
                                //
                                // If we see a call that takes a logical typed pointer, we will
                                // specialize the callee to take a storage typed pointer instead,
                                // and push the cast to inside the callee.
                                // We will process calls after other gep insts, so for now just add
                                // it into a separate worklist.
                                callWorkListSet.add((IRCall*)user);
                                break;
                            }
                        case kIROp_Load:
                        case kIROp_StructuredBufferLoad:
                        case kIROp_RWStructuredBufferLoad:
                        case kIROp_StructuredBufferLoadStatus:
                        case kIROp_RWStructuredBufferLoadStatus:
                        case kIROp_StructuredBufferConsume:
                            {
                                // If we see a load(CastStorageToLogical(storageAddr)),
                                // then based on what `storageAddr` is, we will push down
                                // the cast differently.
                                // - If `storageAddr` is already a tempVar that we introduced to
                                //   hold the value of a buffer resource load, we can simply
                                //   convert this into `CastStorageToLogicalDeref(storageAddr)`.
                                // - Otherwise, if `storageAddr` is a buffer location, we will
                                //   create a temp var to hold the result of the memory load,
                                //   Then we create a `CastStorageToLogicalDeref(tempVar)`
                                //   structure and use it to replace `user`.
                                // Note that it is important to introduce a temp var and preserve
                                // the buffer load operation, so we are not changing the memory
                                // semantics of the original program.
                                if (!isUseBaseAddrOperand(use, user))
                                    break;
                                // If loaded value is itself a pointer or buffer,
                                // stop pushing the cast along the resulting address.
                                // we will handle loads from the pointer separately.
                                if (as<IRPointerLikeType>(user->getDataType()) ||
                                    as<IRPtrTypeBase>(user->getDataType()) ||
                                    as<IRHLSLStructuredBufferTypeBase>(user->getDataType()))
                                    break;
                                // Don't push the cast beyond the load if we are already
                                // a simple type.
                                if (!isCompositeType(user->getDataType()))
                                    break;
                                IRInst* tempVar = nullptr;
                                if (as<IRLoad>(user))
                                {
                                    auto rootAddr = getRootAddr(ptrVal);
                                    if (rootAddr->findDecorationImpl(
                                            kIROp_TempCallArgImmutableVarDecoration))
                                        tempVar = ptrVal;
                                }
                                TypeLoweringConfig newLoweringConfig = config;
                                builder.setInsertBefore(user);
                                if (!tempVar)
                                {
                                    // If the load is not from an immutable location, we
                                    // must preserve the load and keep the result in a local
                                    // var.
                                    auto elementStorageType = tryGetPointedToOrBufferElementType(
                                        &builder,
                                        ptrVal->getDataType());

                                    if (!leafTypeLoweringPolicy->canUseStorageTypeInLocalVar())
                                    {
                                        // Unfortunately, SPIRV disallow a physical type (type with
                                        // explicit layout) to be used to declare a local variable.
                                        // Therefore for SPIRV target only, we want to create a
                                        // clone of the storage type such that it doesn't have SPIRV
                                        // "explicit layout" decorations, but is otherwise the same
                                        // as the lowered storage type. We will declare a temporary
                                        // variable of this "logical storage" type to hold the
                                        // loaded value in Function address space.
                                        newLoweringConfig =
                                            TypeLoweringConfig::getLogicalTypeLoweringConfig(
                                                config);
                                        auto logicalStorageTypeInfo = getLoweredTypeInfo(
                                            user->getDataType(),
                                            newLoweringConfig);

                                        // Try emit an inst that represent the address the load inst
                                        // is loading from.
                                        auto srcPtr = getSourceAddrFromLoadInstWithNewBase(
                                            builder,
                                            user,
                                            ptrVal);

                                        // If there isn't a way to get a pointer to the source data
                                        // from the load inst, there isn't anything we can do other
                                        // than translate the loaded value to logical type now.
                                        if (!srcPtr)
                                            break;
                                        tempVar =
                                            builder.emitVar(logicalStorageTypeInfo.loweredType);

                                        // `newLoad` currently has storage type, but `tempVar` has
                                        // the "logical storage" type, so we need to convert the
                                        // loaded value into the "logical storage" type using a
                                        // `CopyLogical` inst, before storing it into `tempVar`.
                                        if (logicalStorageTypeInfo.loweredType !=
                                            elementStorageType)
                                        {
                                            builder.emitCopyLogical(tempVar, srcPtr, user);
                                        }
                                        else
                                        {
                                            builder.emitStore(tempVar, builder.emitLoad(srcPtr));
                                        }
                                    }
                                    else
                                    {
                                        // The normal case is easy, we simply need to declare a
                                        // local var directly using the lowered storage type, and
                                        // write `newLoad` into it.
                                        tempVar = builder.emitVar(elementStorageType);
                                        builder.addDecoration(
                                            tempVar,
                                            kIROp_TempCallArgImmutableVarDecoration);

                                        // Emit `load_op(ptrVal, ...)`, where first operand is
                                        // replaced from `castInst` to `ptrVal`, while the opcode
                                        // and remaining operands remains the same as `user`.
                                        IRCloneEnv cloneEnv;
                                        cloneEnv.mapOldValToNew[castInst] = ptrVal;
                                        auto newLoad = cloneInst(&cloneEnv, &builder, user);
                                        newLoad->setFullType(elementStorageType);

                                        builder.emitStore(tempVar, newLoad);
                                    }
                                }
                                auto newCast = builder.emitCastStorageToLogicalDeref(
                                    user->getFullType(),
                                    tempVar,
                                    emitTypeLoweringConfigToIR(builder, newLoweringConfig));
                                user->replaceUsesWith(newCast);
                                user->removeAndDeallocate();
                                if (auto newCastStorage = as<IRCastStorageToLogicalBase>(newCast))
                                    castInstWorkList.add(newCastStorage);
                                break;
                            }
                        case kIROp_FieldExtract:
                        case kIROp_GetElement:
                            {
                                if (!isUseBaseAddrOperand(use, user))
                                    break;
                                // elementExtract(castStorageToLogicalDeref(addr), key)
                                // ==> load(gep(castStorageToLogical(addr), key)
                                builder.setInsertBefore(user);
                                auto castAddr = builder.emitCastStorageToLogical(
                                    builder.getPtrType(castInst->getDataType()),
                                    ptrVal,
                                    castInst->getLayoutConfig());
                                IRInst* gep = nullptr;
                                if (user->getOp() == kIROp_GetElement)
                                    gep = builder.emitElementAddress(castAddr, user->getOperand(1));
                                else
                                    gep = builder.emitFieldAddress(castAddr, user->getOperand(1));
                                auto load = builder.emitLoad(gep);
                                user->replaceUsesWith(load);
                                user->removeAndDeallocate();
                                if (auto castStorage = as<IRCastStorageToLogical>(castAddr))
                                    castInstWorkList.add(castStorage);
                                break;
                            }
                        case kIROp_Store:
                            {
                                // If we see `store(tempVar, castStorageToLogicalDeref(addr))`,
                                // replace `tempVar` with `castStorageToLogical(addr)`.
                                if (castInst->getOp() != kIROp_CastStorageToLogicalDeref)
                                    break;
                                auto store = as<IRStore>(user);
                                if (store->getVal() != castInst)
                                    break;
                                auto dest = store->getPtr();
                                if (!dest->findDecorationImpl(
                                        kIROp_TempCallArgImmutableVarDecoration))
                                    break;
                                builder.setInsertBefore(user);
                                auto castAddr = builder.emitCastStorageToLogical(
                                    builder.getPtrType(castInst->getDataType()),
                                    ptrVal,
                                    castInst->getLayoutConfig());
                                dest->replaceUsesWith(castAddr);
                                dest->removeAndDeallocate();
                                if (auto castStorage = as<IRCastStorageToLogical>(castAddr))
                                    castInstWorkList.add(castStorage);
                                break;
                            }
                        }
                    });
            }

            // Now that we have processed all GEP instructions, we can now proceed to
            // process all calls. This is done by making a clone of the callee, and change
            // the parameter type from logical type to storage type, and insert a
            // castStorageToLogical on the parameter. Then we go back to the beginning and make sure
            // we process those newly created castStorageToLogical insts.
            List<IRCastStorageToLogicalBase*> newCasts;
            List<IRCall*> callWorkList;
            for (auto call : callWorkListSet)
                callWorkList.add(call);
            for (Index c = 0; c < callWorkList.getCount(); c++)
            {
                auto call = callWorkList[c];
                auto calleeFunc = as<IRGlobalValueWithParams>(call->getCallee());
                // We compute the func type for the specialized func based on the arguments
                // provided, and check the specialization cache to reuse existing specialization
                // when possible.
                List<IRInst*> oldParams;
                for (auto param : calleeFunc->getParams())
                    oldParams.add(param);
                SLANG_ASSERT(oldParams.getCount() == (Index)call->getArgCount());

                ShortList<IRType*> paramTypes;
                ShortList<IRInst*> newArgs;
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    auto arg = call->getArg(i);
                    if (auto castArg = as<IRCastStorageToLogical>(arg))
                    {
                        auto oldParamPtrType = oldParams[i]->getDataType();
                        auto storageValueType = tryGetPointedToOrBufferElementType(
                            &builder,
                            castArg->getOperand(0)->getDataType());
                        auto storagePtrType =
                            getLoweredPtrLikeType(oldParamPtrType, storageValueType);
                        paramTypes.add(storagePtrType);
                        newArgs.add(castArg->getOperand(0));
                    }
                    else if (auto castArgDeref = as<IRCastStorageToLogicalDeref>(arg))
                    {
                        auto storageValueType = tryGetPointedToOrBufferElementType(
                            &builder,
                            castArgDeref->getOperand(0)->getDataType());
                        auto storagePtrType =
                            builder.getBorrowInParamType(storageValueType, AddressSpace::Generic);
                        paramTypes.add(storagePtrType);
                        newArgs.add(castArgDeref->getOperand(0));
                    }
                    else
                    {
                        paramTypes.add(arg->getDataType());
                        newArgs.add(arg);
                    }
                }
                auto specializedFuncType = builder.getFuncType(
                    (UInt)paramTypes.getCount(),
                    paramTypes.getArrayView().getBuffer(),
                    call->getDataType());
                auto key = SpecializationKey{(IRFunc*)calleeFunc, specializedFuncType};
                IRFunc* specializedFunc = nullptr;
                if (!specializedFuncs.tryGetValue(key, specializedFunc))
                {
                    specializedFunc = createSpecializedFuncThatUseStorageType(
                        call,
                        specializedFuncType,
                        newCasts);
                    specializedFuncs[key] = specializedFunc;
                }
                builder.setInsertBefore(call);
                auto newCall = builder.emitCallInst(
                    call->getFullType(),
                    specializedFunc,
                    newArgs.getArrayView().arrayView);
                call->replaceUsesWith(newCall);
                call->removeAndDeallocate();
            }

            // Remove any casts that have no more uses.
            for (auto cast : castInstWorkList)
            {
                if (!cast->hasUses())
                    cast->removeAndDeallocate();
            }

            // Continue to process new casts added during function specialization.
            castInstWorkList.swapWith(newCasts);
        }
    }

    IRFunc* createSpecializedFuncThatUseStorageType(
        IRCall* call,
        IRFuncType* specializedFuncType,
        List<IRCastStorageToLogicalBase*>& outNewCasts)
    {
        IRBuilder builder(call);
        builder.setInsertBefore(call->getCallee());

        // Create a clone of the callee.
        IRCloneEnv cloneEnv;
        auto clonedFunc = as<IRFunc>(cloneInst(&cloneEnv, &builder, call->getCallee()));
        List<IRUse*> uses;

        // If a parameter is being translated to storage type,
        // insert a cast to convert it to logical type.
        List<IRParam*> params;
        for (auto param : clonedFunc->getParams())
            params.add(param);
        for (UInt i = 0; i < (UInt)params.getCount(); i++)
        {
            auto param = params[i];
            SLANG_RELEASE_ASSERT(i < call->getArgCount());
            auto arg = call->getArg(i);
            auto cast = as<IRCastStorageToLogicalBase>(arg);
            if (!cast)
                continue;
            auto logicalParamType = param->getFullType();
            auto storageType = specializedFuncType->getParamType(i);
            param->setFullType((IRType*)storageType);
            setInsertAfterOrdinaryInst(&builder, param);

            // Store uses of param before creating a cast inst that uses it.
            uses.clear();
            for (auto use = param->firstUse; use; use = use->nextUse)
                uses.add(use);
            IRInst* castedParam = nullptr;
            if (arg->getOp() == kIROp_CastStorageToLogical)
            {
                castedParam = builder.emitCastStorageToLogical(
                    logicalParamType,
                    param,
                    cast->getLayoutConfig());
            }
            else
            {
                castedParam = builder.emitCastStorageToLogicalDeref(
                    logicalParamType,
                    param,
                    cast->getLayoutConfig());
            }

            // Replace all previous uses of param to use castedParam instead.
            for (auto use : uses)
                builder.replaceOperand(use, castedParam);
        }
        clonedFunc->setFullType(specializedFuncType);
        removeLinkageDecorations(clonedFunc);

        // Add all `CastStorageToLogical` insts in the cloned func to the worklist
        // for further processing.
        for (auto block : clonedFunc->getBlocks())
        {
            for (auto child : block->getChildren())
            {
                if (auto castStorage = as<IRCastStorageToLogicalBase>(child))
                    outNewCasts.add(castStorage);
            }
        }
        return clonedFunc;
    }

    void processModule(IRModule* module)
    {
        IRBuilder builder(module);
        struct BufferTypeInfo
        {
            IRType* bufferType;
            IRType* elementType;
            IRType* loweredBufferType = nullptr;
            bool shouldWrapArrayInStruct = false;
        };
        List<BufferTypeInfo> bufferTypeInsts;
        for (auto globalInst : module->getGlobalInsts())
        {
            IRType* elementType = nullptr;

            if (auto ptrType = as<IRPtrTypeBase>(globalInst))
            {
                switch (ptrType->getAddressSpace())
                {
                case AddressSpace::UserPointer:
                case AddressSpace::Input:
                case AddressSpace::Output:
                    elementType = ptrType->getValueType();
                    break;
                }
            }
            if (auto structBuffer = as<IRHLSLStructuredBufferTypeBase>(globalInst))
            {
                elementType = structBuffer->getElementType();
                auto config = getTypeLoweringConfigForBuffer(target, structBuffer);

                // Create size and alignment decoration for potential use
                // in`StructuredBufferGetDimensions`.
                IRSizeAndAlignment sizeAlignment;
                getSizeAndAlignment(
                    target->getOptionSet(),
                    config.getLayoutRule(),
                    elementType,
                    &sizeAlignment);
                SLANG_UNUSED(sizeAlignment);
            }
            else if (auto constBuffer = as<IRUniformParameterGroupType>(globalInst))
                elementType = constBuffer->getElementType();
            else if (auto storageBuffer = as<IRGLSLShaderStorageBufferType>(globalInst))
                elementType = storageBuffer->getElementType();

            if (as<IRTextureBufferType>(globalInst))
                continue;
            if (!as<IRStructType>(elementType) && !as<IRMatrixType>(elementType) &&
                !as<IRArrayType>(elementType) && !as<IRBoolType>(elementType))
                continue;
            bufferTypeInsts.add(BufferTypeInfo{(IRType*)globalInst, elementType});
        }


        List<IRCastStorageToLogicalBase*> castInstWorkList;

        for (auto& bufferTypeInfo : bufferTypeInsts)
        {
            auto bufferType = bufferTypeInfo.bufferType;
            auto elementType = bufferTypeInfo.elementType;

            if (elementType->findDecoration<IRPhysicalTypeDecoration>())
                continue;

            auto config = getTypeLoweringConfigForBuffer(target, bufferType);
            auto loweredBufferElementTypeInfo = getLoweredTypeInfo(elementType, config);

            // If the lowered type is the same as original type, no change is required.
            if (loweredBufferElementTypeInfo.loweredType ==
                loweredBufferElementTypeInfo.originalType)
                continue;

            builder.setInsertBefore(bufferType);

            ShortList<IRInst*> typeOperands;
            for (UInt i = 0; i < bufferType->getOperandCount(); i++)
                typeOperands.add(bufferType->getOperand(i));
            typeOperands[0] = loweredBufferElementTypeInfo.loweredType;
            auto loweredBufferType = builder.getType(
                bufferType->getOp(),
                (UInt)typeOperands.getCount(),
                typeOperands.getArrayView().getBuffer());

            // Replace all global buffer declarations to use the storage type instead,
            // and insert initial `castStorageToLogical` instructions to convert the
            // storage-typed pointer to logical-typed pointer.

            traverseUses(
                bufferType,
                [&](IRUse* use)
                {
                    auto user = use->getUser();
                    if (use != &user->typeUse)
                        return;
                    // We don't want to insert cast instructions for uses of
                    // intermediate address instruction that are themselves
                    // derived from some other base address. We will let
                    // the later part of the pass to systematically propagate
                    // the cast through them.
                    switch (user->getOp())
                    {
                    case kIROp_FieldAddress:
                    case kIROp_GetElementPtr:
                    case kIROp_GetOffsetPtr:
                    case kIROp_RWStructuredBufferGetElementPtr:
                        return;
                    }
                    auto ptrVal = use->getUser();
                    setInsertAfterOrdinaryInst(&builder, ptrVal);
                    builder.replaceOperand(use, loweredBufferType);
                    auto logicalBufferType = getLoweredPtrLikeType(bufferType, elementType);
                    auto loweringConfig = getTypeLoweringConfigForBuffer(target, bufferType);
                    auto castStorageToLogical = builder.emitCastStorageToLogical(
                        logicalBufferType,
                        ptrVal,
                        emitTypeLoweringConfigToIR(builder, loweringConfig));
                    traverseUses(
                        ptrVal,
                        [&](IRUse* ptrUse)
                        {
                            if (ptrUse->getUser() != castStorageToLogical)
                                builder.replaceOperand(ptrUse, castStorageToLogical);
                        });
                    if (auto castStorage = as<IRCastStorageToLogical>(castStorageToLogical))
                        castInstWorkList.add(castStorage);
                });
            bufferTypeInfo.loweredBufferType = loweredBufferType;
        }

        // Push down `CastStorageToLogical` insts we inserted above to latest possible locations,
        // specializing all function calls along the way, until we truly need the the logical value.
        // This means that `FieldAddr(CastStorageToLogical(buffer), field0))` is translated to
        // `CastStorageToLogical(FieldAddr(buffer, field0))`. This way we can be sure that we are
        // doing minimal packing/unpacking.
        deferStorageToLogicalCasts(module, _Move(castInstWorkList));

        // Now translate the `CastStorageToLogical` into actual packing/unpacking code.
        materializeStorageToLogicalCasts(module->getModuleInst());

        // Replace all remaining uses of bufferType to loweredBufferType, these uses are
        // non-operational and should be directly replaceable, such as uses in `IRFuncType`.
        for (auto bufferTypeInst : bufferTypeInsts)
        {
            if (!bufferTypeInst.loweredBufferType)
                continue;
            bufferTypeInst.bufferType->replaceUsesWith(bufferTypeInst.loweredBufferType);
            bufferTypeInst.bufferType->removeAndDeallocate();
        }
    }

    void copyLogical(IRBuilder& builder, IRInst* dest, IRInst* src)
    {
        auto destValType = tryGetPointedToType(&builder, dest->getDataType());
        auto srcValType = tryGetPointedToType(&builder, src->getDataType());
        if (isTypeEqual(destValType, srcValType))
        {
            builder.emitStore(dest, builder.emitLoad(src));
        }
        else
        {
            builder.emitCopyLogical(dest, src, nullptr);
        }
    }

    void materializeStorageToLogicalCastsImpl(IRCastStorageToLogicalBase* castInst)
    {
        IRBuilder builder(castInst);
        if (!castInst->hasUses())
        {
            castInst->removeAndDeallocate();
            return;
        }
        if (castInst->getOp() == kIROp_CastStorageToLogicalDeref)
        {
            // Convert CastStorageToLogicalDeref to load(CastStorageToLogical) to reuse
            // the same materialization logic for CastStorageToLogical.
            //
            builder.setInsertBefore(castInst);
            auto ptrType = builder.getPtrType(castInst->getDataType());
            auto castPtr = builder.emitCastStorageToLogical(
                (IRType*)ptrType,
                castInst->getVal(),
                castInst->getLayoutConfig());
            auto load = builder.emitLoad(castPtr);
            castInst->replaceUsesWith(load);
            castInst->removeAndDeallocate();
            if (auto castStorage = as<IRCastStorageToLogical>(castPtr))
                materializeStorageToLogicalCastsImpl(castStorage);
            return;
        }

        // Translate the values to use new lowered buffer type instead.

        auto ptrVal = castInst->getOperand(0);
        auto oldPtrType = castInst->getFullType();
        auto originalElementType = oldPtrType->getOperand(0);
        auto config = getTypeLoweringConfigFromInst(castInst->getLayoutConfig());

        LoweredElementTypeInfo loweredElementTypeInfo = {};
        if (auto getElementPtr = as<IRGetElementPtr>(ptrVal))
        {
            if (auto arrayType = as<IRArrayTypeBase>(tryGetPointedToOrBufferElementType(
                    &builder,
                    getElementPtr->getBase()->getDataType())))
            {
                // For WGSL, an array of scalar or vector type will always be converted to
                // an array of 16-byte aligned vector type. In this case, we will run into a
                // GetElementPtr where the result type is different from the element type of
                // the base array.
                // We should setup loweredElementTypeInfo so the remaining logic can handle
                // this case and insert proper packing/unpacking logic around it.
                if (arrayType->getElementType() != originalElementType &&
                    isScalarOrVectorType(originalElementType))
                {
                    loweredElementTypeInfo.loweredType = arrayType->getElementType();
                    loweredElementTypeInfo.originalType = (IRType*)originalElementType;
                    loweredElementTypeInfo.convertLoweredToOriginal = getConversionMethod(
                        loweredElementTypeInfo.originalType,
                        loweredElementTypeInfo.loweredType);
                    loweredElementTypeInfo.convertOriginalToLowered = getConversionMethod(
                        loweredElementTypeInfo.loweredType,
                        loweredElementTypeInfo.originalType);
                }
            }
        }

        // For general cases we simply check if the element type needs lowering.
        // If so we will insert packing/unpacking logic if necessary.
        //
        if (!loweredElementTypeInfo.loweredType)
        {
            loweredElementTypeInfo = getLoweredTypeInfo((IRType*)originalElementType, config);
        }

        if (loweredElementTypeInfo.loweredType == loweredElementTypeInfo.originalType)
        {
            castInst->replaceUsesWith(ptrVal);
            castInst->removeAndDeallocate();
            return;
        }

        traverseUses(
            castInst,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                if (as<IRDecoration>(user))
                    return;
                switch (user->getOp())
                {
                case kIROp_Load:
                case kIROp_StructuredBufferLoad:
                case kIROp_StructuredBufferLoadStatus:
                case kIROp_RWStructuredBufferLoad:
                case kIROp_RWStructuredBufferLoadStatus:
                case kIROp_StructuredBufferConsume:
                    {
                        if (castInst != user->getOperand(0))
                            break;
                        builder.setInsertBefore(user);
                        auto addr = getBufferAddr(builder, user, ptrVal);
                        if (!addr)
                        {
                            IRCloneEnv cloneEnv = {};
                            builder.setInsertBefore(user);
                            auto newLoad = cloneInst(&cloneEnv, &builder, user);
                            newLoad->setFullType(loweredElementTypeInfo.loweredType);
                            addr = builder.emitVar(loweredElementTypeInfo.loweredType);
                            builder.emitStore(addr, newLoad);
                        }
                        if (auto alignedAttr = user->findAttr<IRAlignedAttr>())
                        {
                            builder.addAlignedAddressDecoration(addr, alignedAttr->getAlignment());
                        }
                        auto unpackedVal = loweredElementTypeInfo.convertLoweredToOriginal.apply(
                            builder,
                            loweredElementTypeInfo.originalType,
                            addr);
                        user->replaceUsesWith(unpackedVal);
                        user->removeAndDeallocate();
                        return;
                    }
                case kIROp_Store:
                case kIROp_RWStructuredBufferStore:
                case kIROp_StructuredBufferAppend:
                    {
                        // Use must be the dest operand of the store inst.
                        if (use != user->getOperands() + 0)
                            break;
                        IRCloneEnv cloneEnv = {};
                        builder.setInsertBefore(user);
                        auto originalVal = getStoreVal(user);
                        if (auto sbAppend = as<IRStructuredBufferAppend>(user))
                        {
                            builder.setInsertBefore(sbAppend);
                            IRInst* addr = nullptr;
                            if (originalVal->getOp() == kIROp_CastStorageToLogicalDeref)
                            {
                                addr = originalVal->getOperand(0);

                                // `addr` should point to the same type as the lowered structure
                                // buffer element type. There is only one case when this is not
                                // true, that is when we are lowering for SPIRV, and `addr` may
                                // point to a "logical storage type" that is created to work around
                                // SPIRV restriction that physical types cannot be used to declare
                                // local variables. However when we generate SPIRV, we should have
                                // already lowered all Append/Consume structured buffer operations
                                // to standard Load/Store operations, so we should not hit this case
                                // here. Instead they will be handled by the "else" branch of the
                                // "if (sbAppend)" statement down below.
                                SLANG_ASSERT(isTypeEqual(
                                    tryGetPointedToType(&builder, addr->getDataType()),
                                    loweredElementTypeInfo.loweredType));
                            }
                            else
                            {
                                addr = builder.emitVar(loweredElementTypeInfo.loweredType);
                                loweredElementTypeInfo.convertOriginalToLowered
                                    .applyDestinationDriven(builder, addr, originalVal);
                            }
                            auto packedVal = builder.emitLoad(addr);
                            sbAppend->setOperand(1, packedVal);
                        }
                        else
                        {
                            IRInst* addr = getBufferAddr(builder, user, ptrVal);
                            if (auto alignedAttr = user->findAttr<IRAlignedAttr>())
                            {
                                builder.addAlignedAddressDecoration(
                                    addr,
                                    alignedAttr->getAlignment());
                            }
                            if (originalVal->getOp() == kIROp_CastStorageToLogicalDeref)
                            {
                                auto valAddr = originalVal->getOperand(0);

                                // In case `originalVal->getOperand(0)` is a tmp var of logical
                                // storage type (created for SPIRV conformance), we need to use a
                                // logical copy instead of a plain store to convert it to the actual
                                // storage type.
                                copyLogical(builder, addr, valAddr);
                            }
                            else
                            {
                                loweredElementTypeInfo.convertOriginalToLowered
                                    .applyDestinationDriven(builder, addr, originalVal);
                            }
                            user->removeAndDeallocate();
                        }
                        return;
                    }
                default:
                    break;
                }
                // If the pointer is used in any other way that we don't recognize,
                // preserve it as is without translation.
                builder.setInsertBefore(user);
                builder.replaceOperand(use, ptrVal);
            });

        if (!castInst->hasUses())
            castInst->removeAndDeallocate();
    }

    void collectInstsOfType(List<IRCastStorageToLogicalBase*>& insts, IRInst* root, IROp op)
    {
        if (root->getOp() == op)
        {
            insts.add((IRCastStorageToLogicalBase*)root);
            return;
        }
        for (auto child : root->getChildren())
        {
            collectInstsOfType(insts, child, op);
        }
    }

    void materializeStorageToLogicalCasts(IRInst* root)
    {
        // We will process all CastStorageToLogical insts first, before
        // processing all CastStorageToLogicalDeref.
        // This is because when we materialize a
        // `store(CastStorageToLogical(addr), CastStorageToLogicalDeref(src))`,
        // we can just fold out CastStorageToLogicalDeref and emit
        // `store(addr, load(src))` instead.
        // If we materialized `CastStorageToLogicalDeref` first we will
        // miss this opportunity and generate more bloated code.
        //
        List<IRCastStorageToLogicalBase*> castInsts;
        collectInstsOfType(castInsts, root, kIROp_CastStorageToLogical);
        for (auto inst : castInsts)
            materializeStorageToLogicalCastsImpl(inst);

        castInsts.clear();
        collectInstsOfType(castInsts, root, kIROp_CastStorageToLogicalDeref);
        for (auto inst : castInsts)
            materializeStorageToLogicalCastsImpl(inst);
    }

    // Lower all getElementPtr insts of a lowered matrix out of existance.
    void lowerMatrixAddresses(IRModule* module, MatrixAddrWorkItem workItem)
    {
        IRBuilder builder(module);
        auto majorAddr = workItem.matrixAddrInst;
        auto majorGEP = as<IRGetElementPtr>(majorAddr);
        SLANG_ASSERT(majorGEP);
        auto baseCast = as<IRCastStorageToLogical>(majorGEP->getBase());
        SLANG_ASSERT(baseCast);
        auto storageBase = baseCast->getOperand(0);
        auto loweredMatrixType = cast<IRPtrTypeBase>(storageBase->getFullType())->getValueType();
        auto matrixTypeInfo =
            getTypeLoweringMap(workItem.config).mapLoweredTypeToInfo.tryGetValue(loweredMatrixType);
        SLANG_ASSERT(matrixTypeInfo);
        if (matrixTypeInfo->loweredType == matrixTypeInfo->originalType)
            return;
        auto matrixType = as<IRMatrixType>(matrixTypeInfo->originalType);
        auto colCount = getIntVal(matrixType->getColumnCount());
        traverseUses(
            majorAddr,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                builder.setInsertBefore(user);
                switch (user->getOp())
                {
                case kIROp_Load:
                    {
                        IRInst* resultInst = nullptr;
                        auto dataPtr = builder.emitFieldAddress(
                            getLoweredPtrLikeType(
                                majorAddr->getDataType(),
                                matrixTypeInfo->loweredInnerArrayType),
                            storageBase,
                            matrixTypeInfo->loweredInnerStructKey);
                        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                        {
                            List<IRInst*> args;
                            for (IRIntegerValue i = 0; i < colCount; i++)
                            {
                                auto vector =
                                    builder.emitLoad(builder.emitElementAddress(dataPtr, i));
                                auto element =
                                    builder.emitElementExtract(vector, majorGEP->getIndex());
                                args.add(element);
                            }
                            resultInst = builder.emitMakeVector(
                                builder.getVectorType(
                                    matrixType->getElementType(),
                                    (IRIntegerValue)args.getCount()),
                                args);
                        }
                        else
                        {
                            auto element =
                                builder.emitElementAddress(dataPtr, majorGEP->getIndex());
                            resultInst = builder.emitLoad(element);
                        }
                        user->replaceUsesWith(resultInst);
                        user->removeAndDeallocate();
                    }
                    break;
                case kIROp_Store:
                    {
                        auto storeInst = cast<IRStore>(user);
                        if (storeInst->getOperand(0) != majorAddr)
                            break;
                        auto dataPtr = builder.emitFieldAddress(
                            getLoweredPtrLikeType(
                                majorAddr->getDataType(),
                                matrixTypeInfo->loweredInnerArrayType),
                            storageBase,
                            matrixTypeInfo->loweredInnerStructKey);
                        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                        {
                            for (IRIntegerValue i = 0; i < colCount; i++)
                            {
                                auto vectorAddr = builder.emitElementAddress(dataPtr, i);
                                auto elementAddr =
                                    builder.emitElementAddress(vectorAddr, majorGEP->getIndex());
                                builder.emitStore(
                                    elementAddr,
                                    builder.emitElementExtract(storeInst->getVal(), i));
                            }
                        }
                        else
                        {
                            auto rowAddr =
                                builder.emitElementAddress(dataPtr, majorGEP->getIndex());
                            builder.emitStore(rowAddr, storeInst->getVal());
                            user->removeAndDeallocate();
                        }
                        break;
                    }
                case kIROp_GetElementPtr:
                    {
                        auto gep2 = cast<IRGetElementPtr>(user);
                        auto rowIndex = majorGEP->getIndex();
                        auto colIndex = gep2->getIndex();
                        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                        {
                            Swap(rowIndex, colIndex);
                        }
                        auto dataPtr = builder.emitFieldAddress(
                            getLoweredPtrLikeType(
                                majorAddr->getDataType(),
                                matrixTypeInfo->loweredInnerArrayType),
                            storageBase,
                            matrixTypeInfo->loweredInnerStructKey);
                        auto vectorAddr = builder.emitElementAddress(dataPtr, rowIndex);
                        auto elementAddr = builder.emitElementAddress(vectorAddr, colIndex);
                        gep2->replaceUsesWith(elementAddr);
                        gep2->removeAndDeallocate();
                        break;
                    }
                default:
                    SLANG_UNREACHABLE("unhandled inst of a matrix address inst that needs "
                                      "storage lowering.");
                    break;
                }
            });
        if (!majorAddr->hasUses())
            majorAddr->removeAndDeallocate();
    }
};

void lowerBufferElementTypeToStorageType(
    TargetProgram* target,
    IRModule* module,
    BufferElementTypeLoweringOptions options)
{
    LoweredElementTypeContext context(target, options);
    context.processModule(module);
}

IRTypeLayoutRuleName getTypeLayoutRulesFromOp(IROp layoutTypeOp, IRTypeLayoutRuleName defaultLayout)
{
    switch (layoutTypeOp)
    {
    case kIROp_DefaultBufferLayoutType:
        return defaultLayout;
    case kIROp_Std140BufferLayoutType:
        return IRTypeLayoutRuleName::Std140;
    case kIROp_Std430BufferLayoutType:
        return IRTypeLayoutRuleName::Std430;
    case kIROp_ScalarBufferLayoutType:
        return IRTypeLayoutRuleName::Natural;
    case kIROp_CBufferLayoutType:
        return IRTypeLayoutRuleName::C;
    }
    return defaultLayout;
}

IRTypeLayoutRuleName getTypeLayoutRuleNameForBuffer(TargetProgram* target, IRType* bufferType)
{
    if (bufferType->getOp() == kIROp_ParameterBlockType && isMetalTarget(target->getTargetReq()))
    {
        return IRTypeLayoutRuleName::MetalParameterBlock;
    }
    if (target->getTargetReq()->getTarget() != CodeGenTarget::WGSL)
    {
        if (!isKhronosTarget(target->getTargetReq()))
            return IRTypeLayoutRuleName::Natural;

        // If we are just emitting GLSL, we can just use the general layout rule.
        if (!target->shouldEmitSPIRVDirectly())
            return IRTypeLayoutRuleName::Natural;

        // If the user specified a C-compatible buffer layout, then do that.
        if (target->getOptionSet().shouldUseCLayout())
            return IRTypeLayoutRuleName::C;

        // If the user specified a scalar buffer layout, then just use that.
        if (target->getOptionSet().shouldUseScalarLayout())
            return IRTypeLayoutRuleName::Natural;
    }

    if (target->getOptionSet().shouldUseDXLayout())
    {
        if (as<IRUniformParameterGroupType>(bufferType))
        {
            return IRTypeLayoutRuleName::D3DConstantBuffer;
        }
        else
            return IRTypeLayoutRuleName::Natural;
    }

    // The default behavior is to use std140 for constant buffers and std430 for other buffers.
    switch (bufferType->getOp())
    {
    case kIROp_HLSLStructuredBufferType:
    case kIROp_HLSLRWStructuredBufferType:
    case kIROp_HLSLAppendStructuredBufferType:
    case kIROp_HLSLConsumeStructuredBufferType:
    case kIROp_HLSLRasterizerOrderedStructuredBufferType:
        {
            auto structBufferType = as<IRHLSLStructuredBufferTypeBase>(bufferType);
            auto layoutTypeOp = structBufferType->getDataLayout()
                                    ? structBufferType->getDataLayout()->getOp()
                                    : kIROp_DefaultBufferLayoutType;
            return getTypeLayoutRulesFromOp(layoutTypeOp, IRTypeLayoutRuleName::Std430);
        }
    case kIROp_ParameterBlockType:
    case kIROp_ConstantBufferType:
        {
            auto parameterGroupType = as<IRUniformParameterGroupType>(bufferType);

            auto layoutTypeOp = parameterGroupType->getDataLayout()
                                    ? parameterGroupType->getDataLayout()->getOp()
                                    : kIROp_DefaultBufferLayoutType;
            return getTypeLayoutRulesFromOp(layoutTypeOp, IRTypeLayoutRuleName::Std140);
        }
    case kIROp_GLSLShaderStorageBufferType:
        {
            auto storageBufferType = as<IRGLSLShaderStorageBufferType>(bufferType);
            auto layoutTypeOp = storageBufferType->getDataLayout()
                                    ? storageBufferType->getDataLayout()->getOp()
                                    : kIROp_Std430BufferLayoutType;
            return getTypeLayoutRulesFromOp(layoutTypeOp, IRTypeLayoutRuleName::Std430);
        }
    case kIROp_PtrType:
        return IRTypeLayoutRuleName::Natural;
    }
    return IRTypeLayoutRuleName::Natural;
}

IRTypeLayoutRules* getTypeLayoutRuleForBuffer(TargetProgram* target, IRType* bufferType)
{
    auto ruleName = getTypeLayoutRuleNameForBuffer(target, bufferType);
    return IRTypeLayoutRules::get(ruleName);
}

TypeLoweringConfig getTypeLoweringConfigForBuffer(TargetProgram* target, IRType* bufferType)
{
    AddressSpace addrSpace = AddressSpace::Generic;
    if (auto ptrType = as<IRPtrTypeBase>(bufferType))
    {
        switch (ptrType->getAddressSpace())
        {
        case AddressSpace::Input:
        case AddressSpace::Output:
            addrSpace = AddressSpace::Input;
            break;
        case AddressSpace::UserPointer:
            addrSpace = AddressSpace::UserPointer;
            break;
        }
    }
    auto rules = getTypeLayoutRuleNameForBuffer(target, bufferType);
    return TypeLoweringConfig{addrSpace, rules};
}

struct DefaultBufferElementTypeLoweringPolicy : BufferElementTypeLoweringPolicy
{
    TargetProgram* target;
    BufferElementTypeLoweringOptions options;
    SlangMatrixLayoutMode defaultMatrixLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    DefaultBufferElementTypeLoweringPolicy(
        TargetProgram* inTarget,
        BufferElementTypeLoweringOptions inOptions)
        : target(inTarget), options(inOptions)
    {
        defaultMatrixLayout = (SlangMatrixLayoutMode)target->getOptionSet().getMatrixLayoutMode();
        if ((isCPUTarget(target->getTargetReq()) || isCUDATarget(target->getTargetReq()) ||
             isMetalTarget(target->getTargetReq())))
            defaultMatrixLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
        else if (defaultMatrixLayout == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
            defaultMatrixLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    }

    virtual bool shouldLowerMatrixType(IRMatrixType* matrixType, TypeLoweringConfig config)
    {
        if (getIntVal(matrixType->getLayout()) == defaultMatrixLayout &&
            config.getLayoutRule()->ruleName == IRTypeLayoutRuleName::Natural)
        {
            // We only lower the matrix types if they differ from the default
            // matrix layout.
            return false;
        }
        return true;
    }

    IRFunc* createMatrixUnpackFunc(
        IRMatrixType* matrixType,
        IRStructType* structType,
        IRStructKey* dataKey)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto refStructType = builder.getRefParamType(structType, AddressSpace::Generic);
        auto funcType = builder.getFuncType(1, (IRType**)&refStructType, matrixType);
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("unpackStorage"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto rowCount = (Index)getIntVal(matrixType->getRowCount());
        auto colCount = (Index)getIntVal(matrixType->getColumnCount());
        auto packedParamRef = builder.emitParam(refStructType);
        auto packedParam = builder.emitLoad(packedParamRef);
        auto vectorArray = builder.emitFieldExtract(packedParam, dataKey);
        List<IRInst*> args;
        args.setCount(rowCount * colCount);
        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
        {
            for (IRIntegerValue c = 0; c < colCount; c++)
            {
                auto vector = builder.emitElementExtract(vectorArray, c);
                for (IRIntegerValue r = 0; r < rowCount; r++)
                {
                    auto element = builder.emitElementExtract(vector, r);
                    args[(Index)(r * colCount + c)] = element;
                }
            }
        }
        else
        {
            for (IRIntegerValue r = 0; r < rowCount; r++)
            {
                auto vector = builder.emitElementExtract(vectorArray, r);
                for (IRIntegerValue c = 0; c < colCount; c++)
                {
                    auto element = builder.emitElementExtract(vector, c);
                    args[(Index)(r * colCount + c)] = element;
                }
            }
        }
        IRInst* result =
            builder.emitMakeMatrix(matrixType, (UInt)args.getCount(), args.getBuffer());
        builder.emitReturn(result);
        return func;
    }

    IRFunc* createMatrixPackFunc(
        IRMatrixType* matrixType,
        IRStructType* structType,
        IRVectorType* vectorType,
        IRArrayType* arrayType)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto outStructType = builder.getRefParamType(structType, AddressSpace::Generic);
        IRType* paramTypes[] = {outStructType, matrixType};
        auto funcType = builder.getFuncType(2, paramTypes, builder.getVoidType());
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("packMatrix"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto rowCount = getIntVal(matrixType->getRowCount());
        auto colCount = getIntVal(matrixType->getColumnCount());
        auto outParam = builder.emitParam(outStructType);
        auto originalParam = builder.emitParam(matrixType);
        List<IRInst*> elements;
        elements.setCount((Index)(rowCount * colCount));
        for (IRIntegerValue r = 0; r < rowCount; r++)
        {
            auto vector = builder.emitElementExtract(originalParam, r);
            for (IRIntegerValue c = 0; c < colCount; c++)
            {
                auto element = builder.emitElementExtract(vector, c);
                elements[(Index)(r * colCount + c)] = element;
            }
        }
        List<IRInst*> vectors;
        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
        {
            for (IRIntegerValue c = 0; c < colCount; c++)
            {
                List<IRInst*> vecArgs;
                for (IRIntegerValue r = 0; r < rowCount; r++)
                {
                    auto element = elements[(Index)(r * colCount + c)];
                    vecArgs.add(element);
                }
                // Fill in default values for remaining elements in the vector.
                for (IRIntegerValue r = rowCount; r < getIntVal(vectorType->getElementCount()); r++)
                {
                    vecArgs.add(builder.emitDefaultConstruct(vectorType->getElementType()));
                }
                auto colVector = builder.emitMakeVector(
                    vectorType,
                    (UInt)vecArgs.getCount(),
                    vecArgs.getBuffer());
                vectors.add(colVector);
            }
        }
        else
        {
            for (IRIntegerValue r = 0; r < rowCount; r++)
            {
                List<IRInst*> vecArgs;
                for (IRIntegerValue c = 0; c < colCount; c++)
                {
                    auto element = elements[(Index)(r * colCount + c)];
                    vecArgs.add(element);
                }
                // Fill in default values for remaining elements in the vector.
                for (IRIntegerValue c = colCount; c < getIntVal(vectorType->getElementCount()); c++)
                {
                    vecArgs.add(builder.emitDefaultConstruct(vectorType->getElementType()));
                }
                auto rowVector = builder.emitMakeVector(
                    vectorType,
                    (UInt)vecArgs.getCount(),
                    vecArgs.getBuffer());
                vectors.add(rowVector);
            }
        }

        auto vectorArray =
            builder.emitMakeArray(arrayType, (UInt)vectors.getCount(), vectors.getBuffer());
        auto result = builder.emitMakeStruct(structType, 1, &vectorArray);
        builder.emitStore(outParam, result);
        builder.emitReturn();
        return func;
    }

    LoweredElementTypeInfo lowerLeafLogicalType(IRType* type, TypeLoweringConfig config) override
    {
        IRBuilder builder(type);
        builder.setInsertAfter(type);

        LoweredElementTypeInfo info;
        info.originalType = type;

        bool needExplicitLayout = config.lowerToPhysicalType;

        if (auto matrixType = as<IRMatrixType>(type))
        {
            if (!shouldLowerMatrixType(matrixType, config))
            {
                info.loweredType = type;
                return info;
            }

            auto loweredType = builder.createStructType();
            maybeAddPhysicalTypeDecoration(builder, loweredType, config);

            StringBuilder nameSB;
            bool isColMajor =
                getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
            nameSB << "_MatrixStorage_";
            getTypeNameHint(nameSB, matrixType->getElementType());
            nameSB << getIntVal(matrixType->getRowCount()) << "x"
                   << getIntVal(matrixType->getColumnCount());
            if (isColMajor)
                nameSB << "_ColMajor";
            if (!needExplicitLayout)
                nameSB << "_logical";
            nameSB << getLayoutName(config.layoutRuleName);
            builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
            auto structKey = builder.createStructKey();
            builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
            auto vectorSize = isColMajor ? matrixType->getRowCount() : matrixType->getColumnCount();
            if (config.layoutRuleName == IRTypeLayoutRuleName::Std140 &&
                shouldTranslateArrayElementTo16ByteAlignedVectorForConstantBuffer())
            {
                // For constant buffer layout, we need to use 16-byte aligned vector if
                // we are required to ensure array element types has 16-byte stride.
                vectorSize = builder.getIntValue(get16ByteAlignedVectorElementCount(
                    target,
                    matrixType->getElementType(),
                    getIntVal(vectorSize)));
            }

            auto vectorType = builder.getVectorType(matrixType->getElementType(), vectorSize);
            IRSizeAndAlignment elementSizeAlignment;
            getSizeAndAlignment(
                target->getOptionSet(),
                config.getLayoutRule(),
                vectorType,
                &elementSizeAlignment);
            elementSizeAlignment =
                config.getLayoutRule()->alignCompositeElement(elementSizeAlignment);

            auto arrayType = builder.getArrayType(
                vectorType,
                isColMajor ? matrixType->getColumnCount() : matrixType->getRowCount(),
                needExplicitLayout
                    ? builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride())
                    : nullptr);
            builder.createStructField(loweredType, structKey, arrayType);

            info.loweredType = loweredType;
            info.loweredInnerArrayType = arrayType;
            info.loweredInnerStructKey = structKey;
            info.convertLoweredToOriginal =
                createMatrixUnpackFunc(matrixType, loweredType, structKey);
            info.convertOriginalToLowered =
                createMatrixPackFunc(matrixType, loweredType, vectorType, arrayType);
            return info;
        }

        info.loweredType = type;
        return info;
    }
};

struct KhronosTargetBufferElementTypeLoweringPolicy : DefaultBufferElementTypeLoweringPolicy
{
    KhronosTargetBufferElementTypeLoweringPolicy(
        TargetProgram* inTarget,
        BufferElementTypeLoweringOptions inOptions)
        : DefaultBufferElementTypeLoweringPolicy(inTarget, inOptions)
    {
    }

    virtual bool canUseStorageTypeInLocalVar() override
    {
        // SPIRV (Vulkan) does not allow using an explicitly laid out type to declare a local
        // variable.
        return false;
    }

    virtual bool shouldLowerMatrixType(IRMatrixType* matrixType, TypeLoweringConfig config) override
    {
        // For spirv, we always want to lower all matrix types, because SPIRV does not support
        // specifying matrix layout/stride if the matrix type is used in places other than
        // defining a struct field. This means that if a matrix is used to define a varying
        // parameter, we always want to wrap it in a struct.
        //
        if (target->shouldEmitSPIRVDirectly())
            return true;
        return DefaultBufferElementTypeLoweringPolicy::shouldLowerMatrixType(matrixType, config);
    }

    virtual bool shouldAlwaysCreateLoweredStorageTypeForCompositeTypes(
        TypeLoweringConfig config) override
    {
        // For spirv backend, we always want to lower all array types, even if the element type
        // comes out the same. This is because different layout rules may have different array
        // stride requirements.
        //
        // Additionally, `buffer` blocks do not work correctly unless lowered when targeting
        // GLSL.
        return target->shouldEmitSPIRVDirectly() && config.addressSpace != AddressSpace::Input;
    }

    LoweredElementTypeInfo lowerLeafLogicalType(IRType* type, TypeLoweringConfig config) override
    {
        if (target->shouldEmitSPIRVDirectly())
        {
            LoweredElementTypeInfo info = {};
            info.originalType = type;

            switch (target->getTargetReq()->getTarget())
            {
            case CodeGenTarget::SPIRV:
            case CodeGenTarget::SPIRVAssembly:
                {
                    auto scalarType = type;
                    auto vectorType = as<IRVectorType>(scalarType);
                    if (vectorType)
                        scalarType = vectorType->getElementType();
                    IRBuilder builder(type);
                    builder.setInsertBefore(type);

                    if (as<IRBoolType>(scalarType))
                    {
                        // Bool is an abstract type in SPIRV, so we need to lower them into an int.

                        // Find an integer type of the correct size for the current layout rule.
                        IRSizeAndAlignment boolSizeAndAlignment;
                        if (getSizeAndAlignment(
                                target->getOptionSet(),
                                config.getLayoutRule(),
                                scalarType,
                                &boolSizeAndAlignment) == SLANG_OK)
                        {
                            IntInfo ii;
                            ii.width = boolSizeAndAlignment.size * 8;
                            ii.isSigned = true;
                            info.loweredType = builder.getType(getIntTypeOpFromInfo(ii));
                        }
                        else
                        {
                            // Just in case that fails for some reason, just use an int.
                            info.loweredType = builder.getIntType();
                        }

                        if (vectorType)
                            info.loweredType = builder.getVectorType(
                                info.loweredType,
                                vectorType->getElementCount());
                        info.convertLoweredToOriginal = kIROp_BuiltinCast;
                        info.convertOriginalToLowered = kIROp_BuiltinCast;
                        return info;
                    }
                }
                break;
            default:
                break;
            }
        }
        return DefaultBufferElementTypeLoweringPolicy::lowerLeafLogicalType(type, config);
    }
};

struct MetalParameterBlockElementTypeLoweringPolicy : DefaultBufferElementTypeLoweringPolicy
{
    MetalParameterBlockElementTypeLoweringPolicy(
        TargetProgram* inTarget,
        BufferElementTypeLoweringOptions inOptions)
        : DefaultBufferElementTypeLoweringPolicy(inTarget, inOptions)
    {
    }

    virtual bool shouldLowerMatrixType(IRMatrixType* matrixType, TypeLoweringConfig config) override
    {
        SLANG_UNUSED(matrixType);
        SLANG_UNUSED(config);
        return false;
    }

    LoweredElementTypeInfo lowerLeafLogicalType(IRType* type, TypeLoweringConfig config) override
    {
        if (config.layoutRuleName == IRTypeLayoutRuleName::MetalParameterBlock &&
            isResourceType(type))
        {
            IRBuilder builder(type);
            builder.setInsertBefore(type);
            LoweredElementTypeInfo info = {};
            info.originalType = type;
            info.loweredType = builder.getType(kIROp_DescriptorHandleType, type);
            info.convertLoweredToOriginal = kIROp_CastDescriptorHandleToResource;
            info.convertOriginalToLowered = kIROp_CastResourceToDescriptorHandle;
            return info;
        }
        return DefaultBufferElementTypeLoweringPolicy::lowerLeafLogicalType(type, config);
    }
};

struct WGSLBufferElementTypeLoweringPolicy : DefaultBufferElementTypeLoweringPolicy
{
    WGSLBufferElementTypeLoweringPolicy(
        TargetProgram* inTarget,
        BufferElementTypeLoweringOptions inOptions)
        : DefaultBufferElementTypeLoweringPolicy(inTarget, inOptions)
    {
    }

    virtual bool shouldTranslateArrayElementTo16ByteAlignedVectorForConstantBuffer() override
    {
        return true;
    }
};

BufferElementTypeLoweringPolicy* getBufferElementTypeLoweringPolicy(
    BufferElementTypeLoweringPolicyKind kind,
    TargetProgram* target,
    BufferElementTypeLoweringOptions options)
{
    switch (kind)
    {
    case BufferElementTypeLoweringPolicyKind::Default:
        return new DefaultBufferElementTypeLoweringPolicy(target, options);
    case BufferElementTypeLoweringPolicyKind::KhronosTarget:
        return new KhronosTargetBufferElementTypeLoweringPolicy(target, options);
    case BufferElementTypeLoweringPolicyKind::MetalParameterBlock:
        return new MetalParameterBlockElementTypeLoweringPolicy(target, options);
    case BufferElementTypeLoweringPolicyKind::WGSL:
        return new WGSLBufferElementTypeLoweringPolicy(target, options);
    }
    SLANG_UNREACHABLE("unknown buffer element type lowering policy");
}

} // namespace Slang
