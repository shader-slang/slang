// slang-ir-inst-defs.h

#ifndef INST
#error Must #define `INST` before including `ir-inst-defs.h`
#endif

#ifndef INST_RANGE
#define INST_RANGE(BASE, FIRST, LAST) /* empty */
#endif

#define PARENT kIROpFlag_Parent
#define USE_OTHER kIROpFlag_UseOther
#define HOISTABLE kIROpFlag_Hoistable
#define GLOBAL kIROpFlag_Global

INST(Nop, nop, 0, 0)

/* Types */

    /* Basic Types */

    #define DEFINE_BASE_TYPE_INST(NAME) INST(NAME ## Type, NAME, 0, HOISTABLE)
    FOREACH_BASE_TYPE(DEFINE_BASE_TYPE_INST)
    #undef DEFINE_BASE_TYPE_INST
    INST(AfterBaseType, afterBaseType, 0, 0)

    INST_RANGE(BasicType, VoidType, AfterBaseType)

    /* StringTypeBase */
        INST(StringType, String, 0, HOISTABLE)
        INST(NativeStringType, NativeString, 0, HOISTABLE)
    INST_RANGE(StringTypeBase, StringType, NativeStringType)

    INST(CapabilitySetType, CapabilitySet, 0, HOISTABLE)

    INST(DynamicType, DynamicType, 0, HOISTABLE)

    INST(AnyValueType, AnyValueType, 1, HOISTABLE)

    INST(RawPointerType, RawPointerType, 0, HOISTABLE)
    INST(RTTIPointerType, RTTIPointerType, 1, HOISTABLE)
    INST(AfterRawPointerTypeBase, AfterRawPointerTypeBase, 0, 0)
    INST_RANGE(RawPointerTypeBase, RawPointerType, AfterRawPointerTypeBase)


    /* ArrayTypeBase */
        INST(ArrayType, Array, 2, HOISTABLE)
        INST(UnsizedArrayType, UnsizedArray, 1, HOISTABLE)
    INST_RANGE(ArrayTypeBase, ArrayType, UnsizedArrayType)

    INST(FuncType, Func, 0, HOISTABLE)
    INST(BasicBlockType, BasicBlock, 0, HOISTABLE)

    INST(VectorType, Vec, 2, HOISTABLE)
    INST(MatrixType, Mat, 3, HOISTABLE)

    INST(TaggedUnionType, TaggedUnion, 0, HOISTABLE)

    INST(ConjunctionType, Conjunction, 0, HOISTABLE)
    INST(AttributedType, Attributed, 0, HOISTABLE)
    INST(ResultType, Result, 2, HOISTABLE)
    INST(OptionalType, Optional, 1, HOISTABLE)

    INST(DifferentialPairType, DiffPair, 1, HOISTABLE)
    INST(DifferentialPairUserCodeType, DiffPairUserCode, 1, HOISTABLE)
    INST_RANGE(DifferentialPairTypeBase, DifferentialPairType, DifferentialPairUserCodeType)

    INST(BackwardDiffIntermediateContextType, BwdDiffIntermediateCtxType, 1, HOISTABLE)

    /* BindExistentialsTypeBase */

        // A `BindExistentials<B, T0,w0, T1,w1, ...>` represents
        // taking type `B` and binding each of its existential type
        // parameters, recursively, with the specified arguments,
        // where each `Ti, wi` pair represents the concrete type
        // and witness table to plug in for parameter `i`.
        //
        INST(BindExistentialsType, BindExistentials, 1, HOISTABLE)

        // An `BindInterface<B, T0, w0>` represents the special case
        // of a `BindExistentials` where the type `B` is known to be
        // an interface type.
        //
        INST(BoundInterfaceType, BoundInterface, 3, HOISTABLE)

    INST_RANGE(BindExistentialsTypeBase, BindExistentialsType, BoundInterfaceType)

    /* Rate */
        INST(ConstExprRate, ConstExpr, 0, HOISTABLE)
        INST(GroupSharedRate, GroupShared, 0, HOISTABLE)
        INST(ActualGlobalRate, ActualGlobalRate, 0, HOISTABLE)
    INST_RANGE(Rate, ConstExprRate, GroupSharedRate)

    INST(RateQualifiedType, RateQualified, 2, HOISTABLE)

    // Kinds represent the "types of types."
    // They should not really be nested under `IRType`
    // in the overall hierarchy, but we can fix that later.
    //
    /* Kind */
        INST(TypeKind, Type, 0, HOISTABLE)
        INST(RateKind, Rate, 0, HOISTABLE)
        INST(GenericKind, Generic, 0, HOISTABLE)
    INST_RANGE(Kind, TypeKind, GenericKind)

    /* PtrTypeBase */
        INST(PtrType, Ptr, 1, HOISTABLE)
        INST(RefType, Ref, 1, HOISTABLE)
        // A `PsuedoPtr<T>` logically represents a pointer to a value of type
        // `T` on a platform that cannot support pointers. The expectation
        // is that the "pointer" will be legalized away by storing a value
        // of type `T` somewhere out-of-line.

        INST(PseudoPtrType, PseudoPtr, 1, HOISTABLE)

        /* OutTypeBase */
            INST(OutType, Out, 1, HOISTABLE)
            INST(InOutType, InOut, 1, HOISTABLE)
        INST_RANGE(OutTypeBase, OutType, InOutType)
    INST_RANGE(PtrTypeBase, PtrType, InOutType)

    // A ComPtr<T> type is treated as a opaque type that represents a reference-counted handle to a COM object.
    INST(ComPtrType, ComPtr, 1, HOISTABLE)
    // A NativePtr<T> type represents a native pointer to a managed resource.
    INST(NativePtrType, NativePtr, 1, HOISTABLE)

    /* SamplerStateTypeBase */
        INST(SamplerStateType, SamplerState, 0, HOISTABLE)
        INST(SamplerComparisonStateType, SamplerComparisonState, 0, HOISTABLE)
    INST_RANGE(SamplerStateTypeBase, SamplerStateType, SamplerComparisonStateType)

    // TODO: Why do we have all this hierarchy here, when everything
    // that actually matters is currently nested under `TextureTypeBase`?
    /* ResourceTypeBase */
        /* ResourceType */
            /* TextureTypeBase */
                // NOTE! TextureFlavor::Flavor is stored in 'other' bits for these types.
                /* TextureType */
                INST(TextureType, TextureType, 0, USE_OTHER | HOISTABLE)
                /* TextureSamplerType */
                INST(TextureSamplerType, TextureSamplerType, 0, USE_OTHER | HOISTABLE)
                /* GLSLImageType */
                INST(GLSLImageType, GLSLImageType, 0, USE_OTHER | HOISTABLE)
            INST_RANGE(TextureTypeBase, TextureType, GLSLImageType)
        INST_RANGE(ResourceType, TextureType, GLSLImageType)
    INST_RANGE(ResourceTypeBase, TextureType, GLSLImageType)


    /* UntypedBufferResourceType */
        /* ByteAddressBufferTypeBase */
            INST(HLSLByteAddressBufferType,                     ByteAddressBuffer,   0, HOISTABLE)
            INST(HLSLRWByteAddressBufferType,                   RWByteAddressBuffer, 0, HOISTABLE)
            INST(HLSLRasterizerOrderedByteAddressBufferType,    RasterizerOrderedByteAddressBuffer, 0, HOISTABLE)
        INST_RANGE(ByteAddressBufferTypeBase, HLSLByteAddressBufferType, HLSLRasterizerOrderedByteAddressBufferType)
        INST(RaytracingAccelerationStructureType, RaytracingAccelerationStructure, 0, HOISTABLE)
    INST_RANGE(UntypedBufferResourceType, HLSLByteAddressBufferType, RaytracingAccelerationStructureType)

    /* HLSLPatchType */
        INST(HLSLInputPatchType,    InputPatch,     2, HOISTABLE)
        INST(HLSLOutputPatchType,   OutputPatch,    2, HOISTABLE)
    INST_RANGE(HLSLPatchType, HLSLInputPatchType, HLSLOutputPatchType)

    INST(GLSLInputAttachmentType, GLSLInputAttachment, 0, HOISTABLE)

    /* BuiltinGenericType */
        /* HLSLStreamOutputType */
            INST(HLSLPointStreamType,       PointStream,    1, HOISTABLE)
            INST(HLSLLineStreamType,        LineStream,     1, HOISTABLE)
            INST(HLSLTriangleStreamType,    TriangleStream, 1, HOISTABLE)
        INST_RANGE(HLSLStreamOutputType, HLSLPointStreamType, HLSLTriangleStreamType)

        /* MeshOutputType */
            INST(VerticesType,   Vertices, 2, HOISTABLE)
            INST(IndicesType,    Indices,  2, HOISTABLE)
            INST(PrimitivesType, Primitives, 2, HOISTABLE)
        INST_RANGE(MeshOutputType, VerticesType, PrimitivesType)

        /* HLSLStructuredBufferTypeBase */
            INST(HLSLStructuredBufferType,                  StructuredBuffer,                   0, HOISTABLE)
            INST(HLSLRWStructuredBufferType,                RWStructuredBuffer,                 0, HOISTABLE)
            INST(HLSLRasterizerOrderedStructuredBufferType, RasterizerOrderedStructuredBuffer,  0, HOISTABLE)
            INST(HLSLAppendStructuredBufferType,            AppendStructuredBuffer,             0, HOISTABLE)
            INST(HLSLConsumeStructuredBufferType,           ConsumeStructuredBuffer,            0, HOISTABLE)
        INST_RANGE(HLSLStructuredBufferTypeBase, HLSLStructuredBufferType, HLSLConsumeStructuredBufferType)

        /* PointerLikeType */
            /* ParameterGroupType */
                /* UniformParameterGroupType */
                    INST(ConstantBufferType, ConstantBuffer, 1, HOISTABLE)
                    INST(TextureBufferType, TextureBuffer, 1, HOISTABLE)
                    INST(ParameterBlockType, ParameterBlock, 1, HOISTABLE)
                    INST(GLSLShaderStorageBufferType, GLSLShaderStorageBuffer, 0, HOISTABLE)
                INST_RANGE(UniformParameterGroupType, ConstantBufferType, GLSLShaderStorageBufferType)
            
                /* VaryingParameterGroupType */
                    INST(GLSLInputParameterGroupType, GLSLInputParameterGroup, 0, HOISTABLE)
                    INST(GLSLOutputParameterGroupType, GLSLOutputParameterGroup, 0, HOISTABLE)
                INST_RANGE(VaryingParameterGroupType, GLSLInputParameterGroupType, GLSLOutputParameterGroupType)
            INST_RANGE(ParameterGroupType, ConstantBufferType, GLSLOutputParameterGroupType)
        INST_RANGE(PointerLikeType, ConstantBufferType, GLSLOutputParameterGroupType)
    INST_RANGE(BuiltinGenericType, HLSLPointStreamType, GLSLOutputParameterGroupType)




// A user-defined structure declaration at the IR level.
// Unlike in the AST where there is a distinction between
// a `StructDecl` and a `DeclRefType` that refers to it,
// at the IR level the struct declaration and the type
// are the same IR instruction.
//
// This is a parent instruction that holds zero or more
// `field` instructions.
//
INST(StructType, struct, 0, PARENT)
INST(ClassType, class, 0, PARENT)
INST(InterfaceType, interface, 0, GLOBAL)
INST(AssociatedType, associated_type, 0, HOISTABLE)
INST(ThisType, this_type, 0, HOISTABLE)
INST(RTTIType, rtti_type, 0, HOISTABLE)
INST(RTTIHandleType, rtti_handle_type, 0, HOISTABLE)
INST(TupleType, tuple_type, 0, HOISTABLE)

// A type that identifies it's contained type as being emittable as `spirv_literal.
INST(SPIRVLiteralType, spirvLiteralType, 1, HOISTABLE)

// A TypeType-typed IRValue represents a IRType.
// It is used to represent a type parameter/argument in a generics.
INST(TypeType, type_t, 0, HOISTABLE)

/*IRWitnessTableTypeBase*/
    // An `IRWitnessTable` has type `WitnessTableType`.
    INST(WitnessTableType, witness_table_t, 1, HOISTABLE)
    // An integer type representing a witness table for targets where
    // witness tables are represented as integer IDs. This type is used
    // during the lower-generics pass while generating dynamic dispatch
    // code and will eventually lower into an uint type.
    INST(WitnessTableIDType, witness_table_id_t, 1, HOISTABLE)
INST_RANGE(WitnessTableTypeBase, WitnessTableType, WitnessTableIDType)
INST_RANGE(Type, VoidType, WitnessTableIDType)

/*IRGlobalValueWithCode*/
    /* IRGlobalValueWithParams*/
        INST(Func, func, 0, PARENT)
        INST(Generic, generic, 0, PARENT)
    INST_RANGE(GlobalValueWithParams, Func, Generic)

    INST(GlobalVar, global_var, 0, GLOBAL)
INST_RANGE(GlobalValueWithCode, Func, GlobalVar)

INST(GlobalParam, global_param, 0, GLOBAL)
INST(GlobalConstant, globalConstant, 0, GLOBAL)

INST(StructKey, key, 0, GLOBAL)
INST(GlobalGenericParam, global_generic_param, 0, GLOBAL)
INST(WitnessTable, witness_table, 0, 0)

INST(GlobalHashedStringLiterals, global_hashed_string_literals, 0, 0)

INST(Module, module, 0, PARENT)

INST(Block, block, 0, PARENT)

/* IRConstant */
    INST(BoolLit, boolConst, 0, 0)
    INST(IntLit, integer_constant, 0, 0)
    INST(FloatLit, float_constant, 0, 0)
    INST(PtrLit, ptr_constant, 0, 0)
    INST(StringLit, string_constant, 0, 0)
    INST(VoidLit, void_constant, 0, 0)
INST_RANGE(Constant, BoolLit, VoidLit)

INST(CapabilitySet, capabilitySet, 0, HOISTABLE)

INST(undefined, undefined, 0, 0)

// A `defaultConstruct` operation creates an initialized
// value of the result type, and can only be used for types
// where default construction is a meaningful thing to do.
//
INST(DefaultConstruct, defaultConstruct, 0, 0)

INST(MakeDifferentialPair, MakeDiffPair, 2, 0)
INST(MakeDifferentialPairUserCode, MakeDiffPairUserCode, 2, 0)
INST_RANGE(MakeDifferentialPairBase, MakeDifferentialPair, MakeDifferentialPairUserCode)

INST(DifferentialPairGetDifferential, GetDifferential, 1, 0)
INST(DifferentialPairGetDifferentialUserCode, GetDifferentialUserCode, 1, 0)
INST_RANGE(DifferentialPairGetDifferentialBase, DifferentialPairGetDifferential, DifferentialPairGetDifferentialUserCode)

INST(DifferentialPairGetPrimal, GetPrimal, 1, 0)
INST(DifferentialPairGetPrimalUserCode, GetPrimalUserCode, 1, 0)
INST_RANGE(DifferentialPairGetPrimalBase, DifferentialPairGetPrimal, DifferentialPairGetPrimalUserCode)

INST(Specialize, specialize, 2, HOISTABLE)
INST(LookupWitness, lookupWitness, 2, HOISTABLE)
INST(GetSequentialID, GetSequentialID, 1, 0)
INST(BindGlobalGenericParam, bind_global_generic_param, 2, 0)
INST(AllocObj, allocObj, 0, 0)

INST(MakeUInt64, makeUInt64, 2, 0)
INST(MakeVector, makeVector, 0, 0)
INST(MakeMatrix, makeMatrix, 0, 0)
INST(MakeMatrixFromScalar, makeMatrixFromScalar, 1, 0)
INST(MatrixReshape, matrixReshape, 1, 0)
INST(VectorReshape, vectorReshape, 1, 0)
INST(MakeArray, makeArray, 0, 0)
INST(MakeArrayFromElement, makeArrayFromElement, 1, 0)
INST(MakeStruct, makeStruct, 0, 0)
INST(MakeTuple, makeTuple, 0, 0)
INST(GetTupleElement, getTupleElement, 2, 0)
INST(MakeResultValue, makeResultValue, 1, 0)
INST(MakeResultError, makeResultError, 1, 0)
INST(IsResultError, isResultError, 1, 0)
INST(GetResultError, getResultError, 1, 0)
INST(GetResultValue, getResultValue, 1, 0)
INST(GetOptionalValue, getOptionalValue, 1, 0)
INST(OptionalHasValue, optionalHasValue, 1, 0)
INST(MakeOptionalValue, makeOptionalValue, 1, 0)
INST(MakeOptionalNone, makeOptionalNone, 1, 0)
INST(Call, call, 1, 0)

INST(RTTIObject, rtti_object, 0, 0)
INST(Alloca, alloca, 1, 0)

INST(UpdateElement, updateElement, 2, 0)
INST(DetachDerivative, detachDerivative, 1, 0)

INST(PackAnyValue, packAnyValue, 1, 0)
INST(UnpackAnyValue, unpackAnyValue, 1, 0)

INST(WitnessTableEntry, witness_table_entry, 2, 0)
INST(InterfaceRequirementEntry, interface_req_entry, 2, GLOBAL)

INST(Param, param, 0, 0)
INST(StructField, field, 2, 0)
INST(Var, var, 0, 0)

INST(Load, load, 1, 0)
INST(Store, store, 2, 0)

// Produced and removed during backward auto-diff pass as a temporary placeholder representing the
// currently accumulated derivative to pass to some dOut argument in a nested call.
INST(LoadReverseGradient, LoadReverseGradient, 1, 0)

// Produced and removed during backward auto-diff pass as a temporary placeholder containing the
// primal and accumulated derivative values to pass to an inout argument in a nested call.
INST(ReverseGradientDiffPairRef, ReverseGradientDiffPairRef, 2, 0)

// Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
// to represent a reference to an inout parameter for use in the primal part of the computation.
INST(PrimalParamRef, PrimalParamRef, 1, 0)

// Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
// to represent a reference to an inout parameter for use in the back-prop part of the computation.
INST(DiffParamRef, DiffParamRef, 1, 0)

INST(FieldExtract, get_field, 2, 0)
INST(FieldAddress, get_field_addr, 2, 0)

INST(GetElement, getElement, 2, 0)
INST(GetElementPtr, getElementPtr, 2, 0)
INST(GetAddr, getAddr, 1, 0)

// Get an unowned NativeString from a String.
INST(getNativeStr, getNativeStr, 1, 0)

// Make String from a NativeString.
INST(MakeString, makeString, 1, 0)

// Get a native ptr from a ComPtr or RefPtr
INST(GetNativePtr, getNativePtr, 1, 0)

// Get a write reference to a managed ptr var (operand must be Ptr<ComPtr<T>> or Ptr<RefPtr<T>>).
INST(GetManagedPtrWriteRef, getManagedPtrWriteRef, 1, 0)

// Attach a managedPtr var to a NativePtr without changing its ref count.
INST(ManagedPtrAttach, ManagedPtrAttach, 1, 0)

// Attach a managedPtr var to a NativePtr without changing its ref count.
INST(ManagedPtrDetach, ManagedPtrDetach, 1, 0)

// "Subscript" an image at a pixel coordinate to get pointer
INST(ImageSubscript, imageSubscript, 2, 0)

// Load from an Image.
INST(ImageLoad, imageLoad, 2, 0)
// Store into an Image.
INST(ImageStore, imageStore, 3, 0)

// Load (almost) arbitrary-type data from a byte-address buffer
//
// %dst = byteAddressBufferLoad(%buffer, %offset)
//
// where
// - `buffer` is a value of some `ByteAddressBufferTypeBase` type
// - `offset` is an `int`
// - `dst` is a value of some type containing only ordinary data
//
INST(ByteAddressBufferLoad, byteAddressBufferLoad, 2, 0)

// Store (almost) arbitrary-type data to a byte-address buffer
//
// byteAddressBufferLoad(%buffer, %offset, %src)
//
// where
// - `buffer` is a value of some `ByteAddressBufferTypeBase` type
// - `offset` is an `int`
// - `src` is a value of some type containing only ordinary data
//
INST(ByteAddressBufferStore, byteAddressBufferStore, 3, 0)

// Load data from a structured buffer
//
// %dst = structuredBufferLoad(%buffer, %index)
//
// where
// - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
// - `offset` is an `int`
// - `dst` is a value of type T
//
INST(StructuredBufferLoad, structuredBufferLoad, 2, 0)

// Store data to a structured buffer
//
// structuredBufferLoad(%buffer, %offset, %src)
//
// where
// - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
// - `offset` is an `int`
// - `src` is a value of type T
//
INST(StructuredBufferStore, structuredBufferStore, 3, 0)

INST(MeshOutputRef, meshOutputRef, 2, 0)

// Construct a vector from a scalar
//
// %dst = MakeVectorFromScalar %T %N %val
//
// where
// - `T` is a `Type`
// - `N` is a (compile-time) `Int`
// - `val` is a `T`
// - dst is a `Vec<T,N>`
//
INST(MakeVectorFromScalar, MakeVectorFromScalar, 3, 0)

// A swizzle of a vector:
//
// %dst = swizzle %src %idx0 %idx1 ...
//
// where:
// - `src` is a vector<T,N>
// - `dst` is a vector<T,M>
// - `idx0` through `idx[M-1]` are literal integers
//
INST(swizzle, swizzle, 1, 0)

// Setting a vector via swizzle
//
// %dst = swizzle %base %src %idx0 %idx1 ...
//
// where:
// - `base` is a vector<T,N>
// - `dst` is a vector<T,N>
// - `src` is a vector<T,M>
// - `idx0` through `idx[M-1]` are literal integers
//
// The semantics of the op is:
//
//     dst = base;
//     for(ii : 0 ... M-1 )
//         dst[ii] = src[idx[ii]];
//
INST(swizzleSet, swizzleSet, 2, 0)

// Store to memory with a swizzle
//
// TODO: eventually this should be reduced to just
// a write mask by moving the actual swizzle to the RHS.
//
// swizzleStore %dst %src %idx0 %idx1 ...
//
// where:
// - `dst` is a vector<T,N>
// - `src` is a vector<T,M>
// - `idx0` through `idx[M-1]` are literal integers
//
// The semantics of the op is:
//
//     for(ii : 0 ... M-1 )
//         dst[ii] = src[idx[ii]];
//
INST(SwizzledStore, swizzledStore, 2, 0)


/* IRTerminatorInst */

    INST(Return, return_val, 1, 0)
    /* IRUnconditionalBranch */
        // unconditionalBranch <target>
        INST(unconditionalBranch, unconditionalBranch, 1, 0)

        // loop <target> <breakLabel> <continueLabel>
        INST(loop, loop, 3, 0)
    INST_RANGE(UnconditionalBranch, unconditionalBranch, loop)

    /* IRConditionalbranch */

        // conditionalBranch <condition> <trueBlock> <falseBlock>
        INST(conditionalBranch, conditionalBranch, 3, 0)

        // ifElse <condition> <trueBlock> <falseBlock> <mergeBlock>
        INST(ifElse, ifElse, 4, 0)
    INST_RANGE(ConditionalBranch, conditionalBranch, ifElse)

    INST(Throw, throw, 1, 0)
    // tryCall <successBlock> <failBlock> <callee> <args>...
    INST(TryCall, tryCall, 3, 0)
    // switch <val> <break> <default> <caseVal1> <caseBlock1> ...
    INST(Switch, switch, 3, 0)

    INST(discard, discard, 0, 0)

    /* IRUnreachable */
        INST(MissingReturn, missingReturn, 0, 0)
        INST(Unreachable, unreachable, 0, 0)
    INST_RANGE(Unreachable, MissingReturn, Unreachable)

INST_RANGE(TerminatorInst, Return, Unreachable)

// TODO: We should consider splitting the basic arithmetic/comparison
// ops into cases for signed integers, unsigned integers, and floating-point
// values, to better match downstream targets that want to treat them
// all differently (e.g., SPIR-V).

INST(Add, add, 2, 0)
INST(Sub, sub, 2, 0)
INST(Mul, mul, 2, 0)
INST(Div, div, 2, 0)

// Remainder of division.
//
// Note: this is distinct from modulus, and we should have a separate
// opcode for `mod` if we ever need to support it.
//
INST(IRem, irem, 2, 0) // integer (signed or unsigned)
INST(FRem, frem, 2, 0) // floating-point

INST(Lsh, shl, 2, 0)
INST(Rsh, shr, 2, 0)

INST(Eql, cmpEQ, 2, 0)
INST(Neq, cmpNE, 2, 0)
INST(Greater, cmpGT, 2, 0)
INST(Less, cmpLT, 2, 0)
INST(Geq, cmpGE, 2, 0)
INST(Leq, cmpLE, 2, 0)

INST(BitAnd, and, 2, 0)
INST(BitXor, xor, 2, 0)
INST(BitOr, or , 2, 0)

INST(And, logicalAnd, 2, 0)
INST(Or, logicalOr, 2, 0)

INST(Neg, neg, 1, 0)
INST(Not, not, 1, 0)
INST(BitNot, bitnot, 1, 0)

INST(Select, select, 3, 0)

INST(GetStringHash, getStringHash, 1, 0)

INST(WaveGetActiveMask, waveGetActiveMask, 0, 0)

    /// trueMask = waveMaskBallot(mask, condition)
INST(WaveMaskBallot, waveMaskBallot, 2, 0)

    /// matchMask = waveMaskBallot(mask, value)
INST(WaveMaskMatch, waveMaskMatch, 2, 0)

// Texture sampling operation of the form `t.Sample(s,u)`
INST(Sample, sample, 3, 0)

INST(SampleGrad, sampleGrad, 4, 0)

INST(GroupMemoryBarrierWithGroupSync, GroupMemoryBarrierWithGroupSync, 0, 0)

// GPU_FOREACH loop of the form 
INST(GpuForeach, gpuForeach, 3, 0)

// Wrapper for OptiX intrinsics used to load and store ray payload data using
// a pointer represented by two payload registers.
INST(GetOptiXRayPayloadPtr, getOptiXRayPayloadPtr, 0, 0)

// Wrapper for OptiX intrinsics used to load a single hit attribute
// Takes two arguments: the type (either float or int), and the hit 
// attribute index
INST(GetOptiXHitAttribute, getOptiXHitAttribute, 2, 0)

// Wrapper for OptiX intrinsics used to load shader binding table record data
// using a pointer. 
INST(GetOptiXSbtDataPtr, getOptiXSbtDataPointer, 0, 0)

/* Decoration */

INST(HighLevelDeclDecoration,               highLevelDecl,          1, 0)
    INST(LayoutDecoration,                  layout,                 1, 0)
    INST(LoopControlDecoration,             loopControl,            1, 0)
    INST(LoopMaxItersDecoration,            loopMaxIters,           1, 0)
    INST(LoopInferredMaxItersDecoration,    loopInferredMaxIters,   2, 0)
    INST(LoopExitPrimalValueDecoration,     loopExitPrimalValue,    2, 0)
    INST(IntrinsicOpDecoration, intrinsicOp, 1, 0)
    /* TargetSpecificDecoration */
        INST(TargetDecoration,              target,                 1, 0)
        INST(TargetIntrinsicDecoration,     targetIntrinsic,        2, 0)
    INST_RANGE(TargetSpecificDecoration, TargetDecoration, TargetIntrinsicDecoration)
    INST(GLSLOuterArrayDecoration,          glslOuterArray,         1, 0)
    
    INST(InterpolationModeDecoration,       interpolationMode,      1, 0)
    INST(NameHintDecoration,                nameHint,               1, 0)

    /**  The decorated _instruction_ is transitory. Such a decoration should NEVER be found on an output instruction a module. 
        Typically used mark an instruction so can be specially handled - say when creating a IRConstant literal, and the payload of 
        needs to be special cased for lookup. */
    INST(TransitoryDecoration,              transitory,             0, 0)

    INST(VulkanRayPayloadDecoration,        vulkanRayPayload,       0, 0)
    INST(VulkanHitAttributesDecoration,     vulkanHitAttributes,    0, 0)
    INST(VulkanHitObjectAttributesDecoration, vulkanHitObjectAttributes, 0, 0)

    INST(RequireSPIRVVersionDecoration,     requireSPIRVVersion,    1, 0)
    INST(RequireGLSLVersionDecoration,      requireGLSLVersion,     1, 0)
    INST(RequireGLSLExtensionDecoration,    requireGLSLExtension,   1, 0)
    INST(RequireCUDASMVersionDecoration,    requireCUDASMVersion,   1, 0)

    INST(ReadNoneDecoration,                readNone,               0, 0)
    INST(VulkanCallablePayloadDecoration,   vulkanCallablePayload,  0, 0)
    INST(EarlyDepthStencilDecoration,       earlyDepthStencil,      0, 0)
    INST(GloballyCoherentDecoration,        globallyCoherent,       0, 0)
    INST(PreciseDecoration,                 precise,                0, 0)
    INST(PublicDecoration,                  public,                 0, 0)
    INST(HLSLExportDecoration,              hlslExport,             0, 0)
    INST(PatchConstantFuncDecoration,       patchConstantFunc,      1, 0)

    INST(OutputControlPointsDecoration,     outputControlPoints,    1, 0)
    INST(OutputTopologyDecoration,          outputTopology,         1, 0)
    INST(PartitioningDecoration,            partioning,             1, 0)
    INST(DomainDecoration,                  domain,                 1, 0)
    INST(MaxVertexCountDecoration,          maxVertexCount,         1, 0)
    INST(InstanceDecoration,                instance,               1, 0)
    INST(NumThreadsDecoration,              numThreads,             3, 0)

        // Added to IRParam parameters to an entry point
    /* GeometryInputPrimitiveTypeDecoration */
        INST(PointInputPrimitiveTypeDecoration,  pointPrimitiveType,     0, 0)
        INST(LineInputPrimitiveTypeDecoration,   linePrimitiveType,      0, 0)
        INST(TriangleInputPrimitiveTypeDecoration, trianglePrimitiveType, 0, 0)
        INST(LineAdjInputPrimitiveTypeDecoration,  lineAdjPrimitiveType,  0, 0)
        INST(TriangleAdjInputPrimitiveTypeDecoration, triangleAdjPrimitiveType, 0, 0)
    INST_RANGE(GeometryInputPrimitiveTypeDecoration, PointInputPrimitiveTypeDecoration, TriangleAdjInputPrimitiveTypeDecoration)

    INST(StreamOutputTypeDecoration,       streamOutputTypeDecoration,    1, 0)

        /// An `[entryPoint]` decoration marks a function that represents a shader entry point
    INST(EntryPointDecoration,              entryPoint,             2, 0)

        /// Used to mark parameters that are moved from entry point parameters to global params as coming from the entry point.
    INST(EntryPointParamDecoration,         entryPointParam,        0, 0)

        /// A `[dependsOn(x)]` decoration indicates that the parent instruction depends on `x`
        /// even if it does not otherwise reference it.
    INST(DependsOnDecoration,               dependsOn,              1, 0)

        /// A `[keepAlive]` decoration marks an instruction that should not be eliminated.
    INST(KeepAliveDecoration,              keepAlive,             0, 0)

        /// A `[NoSideEffect]` decoration marks a callee to be side-effect free.
    INST(NoSideEffectDecoration,           noSideEffect, 0, 0)

    INST(BindExistentialSlotsDecoration, bindExistentialSlots, 0, 0)

        /// A `[format(f)]` decoration specifies that the format of an image should be `f`
    INST(FormatDecoration, format, 1, 0)

        /// An `[unsafeForceInlineEarly]` decoration specifies that calls to this function should be inline after initial codegen
    INST(UnsafeForceInlineEarlyDecoration, unsafeForceInlineEarly, 0, 0)

        /// A `[ForceInline]` decoration indicates the callee should be inlined by the Slang compiler.
    INST(ForceInlineDecoration, ForceInline, 0, 0)

        /// A `[ForceUnroll]` decoration indicates the loop should be unrolled by the Slang compiler.
    INST(ForceUnrollDecoration, ForceUnroll, 0, 0)

        /// A `[naturalSizeAndAlignment(s,a)]` decoration is attached to a type to indicate that is has natural size `s` and alignment `a`
    INST(NaturalSizeAndAlignmentDecoration, naturalSizeAndAlignment, 2, 0)

        /// A `[naturalOffset(o)]` decoration is attached to a field to indicate that it has natural offset `o` in the parent type
    INST(NaturalOffsetDecoration, naturalOffset, 1, 0)

    /* LinkageDecoration */
        INST(ImportDecoration, import, 1, 0)
        INST(ExportDecoration, export, 1, 0)
    INST_RANGE(LinkageDecoration, ImportDecoration, ExportDecoration)

        /// An extern_cpp decoration marks the inst to emit its name without mangling for C++ interop.
    INST(ExternCppDecoration, externCpp, 1, 0)

        /// An dllImport decoration marks a function as imported from a DLL. Slang will generate dynamic function loading logic to use this function at runtime.
    INST(DllImportDecoration, dllImport, 2, 0)
        /// An dllExport decoration marks a function as an export symbol. Slang will generate a native wrapper function that is exported to DLL.
    INST(DllExportDecoration, dllExport, 1, 0)

        /// Marks an interface as a COM interface declaration.
    INST(ComInterfaceDecoration, COMInterface, 0, 0)

    /* Decorations for RTTI objects */
    INST(RTTITypeSizeDecoration, RTTI_typeSize, 1, 0)
    INST(AnyValueSizeDecoration, AnyValueSize, 1, 0)
    INST(SpecializeDecoration, SpecializeDecoration, 0, 0)
    INST(SequentialIDDecoration, SequentialIDDecoration, 1, 0)

    INST(TypeConstraintDecoration, TypeConstraintDecoration, 1, 0)

    
    INST(BuiltinDecoration, BuiltinDecoration, 0, 0)

        /// The decorated instruction requires NVAPI to be included via prelude when compiling for D3D.
    INST(RequiresNVAPIDecoration, requiresNVAPI, 0, 0)

        /// The decorated instruction is part of the NVAPI "magic" and should always use its original name
    INST(NVAPIMagicDecoration, nvapiMagic, 1, 0)

        /// A decoration that applies to an entire IR module, and indicates the register/space binding
        /// that the NVAPI shader parameter intends to use.
    INST(NVAPISlotDecoration, nvapiSlot, 2, 0)

        /// Applie to an IR function and signals that inlining should not be performed unless unavoidable.
    INST(NoInlineDecoration, noInline, 0, 0)

        /// A call to the decorated function should always be folded into its use site.
    INST(AlwaysFoldIntoUseSiteDecoration, alwaysFold, 0, 0)

    INST(PayloadDecoration, payload, 0, 0)

    /* Mesh Shader outputs */
        INST(VerticesDecoration, vertices, 1, 0)
        INST(IndicesDecoration, indices, 1, 0)
        INST(PrimitivesDecoration, primitives, 1, 0)
    INST_RANGE(MeshOutputDecoration, VerticesDecoration, PrimitivesDecoration)
    INST(GLSLPrimitivesRateDecoration, perprimitive, 0, 0)

    /* StageAccessDecoration */
        INST(StageReadAccessDecoration, stageReadAccess, 0, 0)
        INST(StageWriteAccessDecoration, stageWriteAccess, 0, 0)
    INST_RANGE(StageAccessDecoration, StageReadAccessDecoration, StageWriteAccessDecoration)

    INST(SemanticDecoration, semantic, 2, 0)

    INST(SPIRVOpDecoration, spirvOpDecoration, 1, 0)

        /// Decorated function is marked for the forward-mode differentiation pass.
    INST(ForwardDifferentiableDecoration, forwardDifferentiable, 0, 0)

        /// Decorates a auto-diff transcribed value with the original value that the inst is transcribed from.
    INST(AutoDiffOriginalValueDecoration, AutoDiffOriginalValueDecoration, 1, 0)

        /// Used by the auto-diff pass to hold a reference to the
        /// generated derivative function.
    INST(ForwardDerivativeDecoration, fwdDerivative, 1, 0)

        /// Used by the auto-diff pass to hold a reference to the
        /// generated derivative function.
    INST(BackwardDifferentiableDecoration, backwardDifferentiable, 1, 0)

        /// Used by the auto-diff pass to hold a reference to the
        /// primal substitute function.
    INST(PrimalSubstituteDecoration, primalSubstFunc, 1, 0)

        /// Decorations to associate an original function with compiler generated backward derivative functions.
    INST(BackwardDerivativePrimalDecoration, backwardDiffPrimalReference, 1, 0)
    INST(BackwardDerivativePropagateDecoration, backwardDiffPropagateReference, 1, 0)
    INST(BackwardDerivativeIntermediateTypeDecoration, backwardDiffIntermediateTypeReference, 1, 0)
    INST(BackwardDerivativeDecoration, backwardDiffReference, 1, 0)

    INST(UserDefinedBackwardDerivativeDecoration, userDefinedBackwardDiffReference, 1, 0)
    INST(BackwardDerivativePrimalContextDecoration, BackwardDerivativePrimalContextDecoration, 1, 0)
    INST(BackwardDerivativePrimalReturnDecoration, BackwardDerivativePrimalReturnDecoration, 1, 0)

    INST(LoopCounterDecoration, loopCounterDecoration, 0, 0)
    INST(PrimalValueAccessDecoration, primalValueAccessDecoration, 0, 0)

    /* Auto-diff inst decorations */
        /// Used by the auto-diff pass to mark insts that compute
        /// a primal value.
        INST(PrimalInstDecoration, primalInstDecoration, 0, 0)

        /// Used by the auto-diff pass to mark insts that compute
        /// a differential value.
        INST(DifferentialInstDecoration, diffInstDecoration, 1, 0)

        /// Used by the auto-diff pass to mark insts that compute
        /// BOTH a differential and a primal value.
        INST(MixedDifferentialInstDecoration, mixedDiffInstDecoration, 1, 0)
    INST_RANGE(AutodiffInstDecoration, PrimalInstDecoration, MixedDifferentialInstDecoration)

        /// Used by the auto-diff pass to mark insts whose result is stored
        /// in an intermediary struct for reuse in backward propagation phase.
    INST(PrimalValueStructKeyDecoration, primalValueKey, 1, 0)

        /// Used by the auto-diff pass to mark the primal element type of an
        /// forward-differentiated updateElement inst.
    INST(PrimalElementTypeDecoration, primalElementType, 1, 0)

        /// Used by the auto-diff pass to mark the differential type of an intermediate context field.
    INST(IntermediateContextFieldDifferentialTypeDecoration, IntermediateContextFieldDifferentialTypeDecoration, 1, 0)

        /// Used by the auto-diff pass to hold a reference to a
        /// differential member of a type in its associated differential type.
    INST(DerivativeMemberDecoration, derivativeMemberDecoration, 1, 0)

        /// Treat a function as differentiable function, or an IRCall as a call to a differentiable function.
    INST(TreatAsDifferentiableDecoration, treatAsDifferentiableDecoration, 0, 0)

        /// Marks a class type as a COM interface implementation, which enables
        /// the witness table to be easily picked up by emit.
    INST(COMWitnessDecoration, COMWitnessDecoration, 1, 0)

    /* Differentiable Type Dictionary */
    INST(DifferentiableTypeDictionaryDecoration, DifferentiableTypeDictionaryDecoration, 0, PARENT)

        /// Overrides the floating mode for the target function
    INST(FloatingPointModeOverrideDecoration, FloatingPointModeOverride, 1, 0)

        /// Marks a struct type as being used as a structured buffer block.
        /// Recognized by SPIRV-emit pass so we can emit a SPIRV `BufferBlock` decoration.
    INST(SPIRVBufferBlockDecoration, spvBufferBlock, 0, 0)

    INST_RANGE(Decoration, HighLevelDeclDecoration, SPIRVBufferBlockDecoration)

    //

// A `makeExistential(v : C, w) : I` instruction takes a value `v` of type `C`
// and produces a value of interface type `I` by using the witness `w` which
// shows that `C` conforms to `I`.
//
INST(MakeExistential,                   makeExistential,                2, 0)
// A `MakeExistentialWithRTTI(v, w, t)` is the same with `MakeExistential`,
// but with the type of `v` being an explict operand.
INST(MakeExistentialWithRTTI,           makeExistentialWithRTTI,        3, 0)

// A 'CreateExistentialObject<I>(typeID, T)` packs user-provided `typeID` and a
// value of any type, and constructs an existential value of type `I`.
INST(CreateExistentialObject,           createExistentialObject,        2, 0)

// A `wrapExistential(v, T0,w0, T1,w0) : T` instruction is similar to `makeExistential`.
// but applies to a value `v` that is of type `BindExistentials(T, T0,w0, ...)`. The
// result of the `wrapExistentials` operation is a value of type `T`, allowing us to
// "smuggle" a value of specialized type into computations that expect an unspecialized type.
//
INST(WrapExistential,                   wrapExistential,                1, 0)

// A `GetValueFromBoundInterface` takes a `BindInterface<I, T, w0>` value and returns the
// value of concrete type `T` value that is being stored.
//
INST(GetValueFromBoundInterface,        getValueFromBoundInterface,     1, 0)

INST(ExtractExistentialValue,           extractExistentialValue,        1, 0)
INST(ExtractExistentialType,            extractExistentialType,         1, 0)
INST(ExtractExistentialWitnessTable,    extractExistentialWitnessTable, 1, 0)

INST(ExtractTaggedUnionTag,             extractTaggedUnionTag,      1, 0)
INST(ExtractTaggedUnionPayload,         extractTaggedUnionPayload,  1, 0)

INST(BitCast,                           bitCast,                    1, 0)
INST(Reinterpret,                       reinterpret,                1, 0)
INST(IntCast, intCast, 1, 0)
INST(FloatCast, floatCast, 1, 0)
INST(CastIntToFloat, castIntToFloat, 1, 0)
INST(CastFloatToInt, castFloatToInt, 1, 0)
INST(CastPtrToBool, CastPtrToBool, 1, 0)
INST(CastPtrToInt, CastPtrToInt, 1, 0)
INST(CastIntToPtr, CastIntToPtr, 1, 0)
INST(CastToVoid, castToVoid, 1, 0)

INST(IsType, IsType, 3, 0)
INST(ForwardDifferentiate,                   ForwardDifferentiate,            1, 0)

// Produces the primal computation of backward derivatives, will return an intermediate context for
// backward derivative func.
INST(BackwardDifferentiatePrimal,            BackwardDifferentiatePrimal,     1, 0)

// Produces the actual backward derivative propagate function, using the intermediate context returned by the
// primal func produced from `BackwardDifferentiatePrimal`.
INST(BackwardDifferentiatePropagate,         BackwardDifferentiatePropagate,  1, 0)

// Represents the conceptual backward derivative function. Only produced by lower-to-ir and will be
// replaced with `BackwardDifferentiatePrimal` and `BackwardDifferentiatePropagate`.
INST(BackwardDifferentiate, BackwardDifferentiate, 1, 0)

INST(PrimalSubstitute, PrimalSubstitute, 1, 0)

// Converts other resources (such as ByteAddressBuffer) to the equivalent StructuredBuffer
INST(GetEquivalentStructuredBuffer,     getEquivalentStructuredBuffer, 1, 0)

/* Layout */
    INST(VarLayout, varLayout, 1, HOISTABLE)

    /* TypeLayout */
        INST(TypeLayoutBase, typeLayout, 0, HOISTABLE)
        INST(ParameterGroupTypeLayout, parameterGroupTypeLayout, 2, HOISTABLE)
        INST(ArrayTypeLayout, arrayTypeLayout, 1, HOISTABLE)
        INST(StreamOutputTypeLayout, streamOutputTypeLayout, 1, HOISTABLE)
        INST(MatrixTypeLayout, matrixTypeLayout, 1, HOISTABLE)
        INST(TaggedUnionTypeLayout, taggedUnionTypeLayout, 0, HOISTABLE)
        INST(ExistentialTypeLayout, existentialTypeLayout, 0, HOISTABLE)
        INST(StructTypeLayout, structTypeLayout, 0, HOISTABLE)
    INST_RANGE(TypeLayout, TypeLayoutBase, StructTypeLayout)

    INST(EntryPointLayout, EntryPointLayout, 1, HOISTABLE)
INST_RANGE(Layout, VarLayout, EntryPointLayout)

/* Attr */
    INST(PendingLayoutAttr, pendingLayout, 1, HOISTABLE)
    INST(StageAttr, stage, 1, HOISTABLE)
    INST(StructFieldLayoutAttr, fieldLayout, 2, HOISTABLE)
    INST(CaseTypeLayoutAttr, caseLayout, 1, HOISTABLE)
    INST(UNormAttr, unorm, 0, HOISTABLE)
    INST(SNormAttr, snorm, 0, HOISTABLE)
    INST(NoDiffAttr, no_diff, 0, HOISTABLE)

    /* SemanticAttr */
        INST(UserSemanticAttr, userSemantic, 2, HOISTABLE)
        INST(SystemValueSemanticAttr, systemValueSemantic, 2, HOISTABLE)
    INST_RANGE(SemanticAttr, UserSemanticAttr, SystemValueSemanticAttr)
    /* LayoutResourceInfoAttr */
        INST(TypeSizeAttr, size, 2, HOISTABLE)
        INST(VarOffsetAttr, offset, 2, HOISTABLE)
    INST_RANGE(LayoutResourceInfoAttr, TypeSizeAttr, VarOffsetAttr)
    INST(FuncThrowTypeAttr, FuncThrowType, 1, HOISTABLE)
INST_RANGE(Attr, PendingLayoutAttr, FuncThrowTypeAttr)

/* Liveness */
    INST(LiveRangeStart, liveRangeStart, 2, 0)
    INST(LiveRangeEnd, liveRangeEnd, 0, 0)
INST_RANGE(LiveRangeMarker, LiveRangeStart, LiveRangeEnd)

/* IRSpecialization */
INST(SpecializationDictionaryItem, SpecializationDictionaryItem, 0, 0)
INST(GenericSpecializationDictionary, GenericSpecializationDictionary, 0, PARENT)
INST(ExistentialFuncSpecializationDictionary, ExistentialFuncSpecializationDictionary, 0, PARENT)
INST(ExistentialTypeSpecializationDictionary, ExistentialTypeSpecializationDictionary, 0, PARENT)

/* Differentiable Type Dictionary */
INST(DifferentiableTypeDictionaryItem, DifferentiableTypeDictionaryItem, 0, 0)

#undef PARENT
#undef USE_OTHER
#undef INST_RANGE
#undef INST
