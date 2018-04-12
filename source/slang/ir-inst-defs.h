// ir-inst-defs.h

#ifndef INST
#error Must #define `INST` before including `ir-inst-defs.h`
#endif

#ifndef INST_RANGE
#define INST_RANGE(BASE, FIRST, LAST) /* empty */
#endif

#ifndef MANUAL_INST_RANGE
#define MANUAL_INST_RANGE(NAME, START, COUNT) /* empty */
#endif

#ifndef PSEUDO_INST
#define PSEUDO_INST(ID) /* empty */
#endif

#define PARENT kIROpFlag_Parent

INST(Nop, nop, 0, 0)

/* Types */

    /* Basic Types */

    #define DEFINE_BASE_TYPE_INST(NAME) INST(NAME ## Type, NAME, 0, 0)
    FOREACH_BASE_TYPE(DEFINE_BASE_TYPE_INST)
    #undef DEFINE_BASE_TYPE_INST
    INST(AfterBaseType, afterBaseType, 0, 0)

    INST_RANGE(BasicType, VoidType, AfterBaseType)

    INST(StringType, String, 0, 0)
    INST(RayDescType, RayDesc, 0, 0)
    INST(BuiltInTriangleIntersectionAttributesType, BuiltInTriangleIntersectionAttributes, 0, 0)

    /* ArrayTypeBase */
        INST(ArrayType, Array, 2, 0)
        INST(UnsizedArrayType, UnsizedArray, 1, 0)
    INST_RANGE(ArrayTypeBase, ArrayType, UnsizedArrayType)

    INST(FuncType, Func, 0, 0)
    INST(BasicBlockType, BasicBlock, 0, 0)

    INST(VectorType, Vec, 2, 0)
    INST(MatrixType, Mat, 3, 0)

    /* Rate */
        INST(ConstExprRate, ConstExpr, 0, 0)
        INST(GroupSharedRate, GroupShared, 0, 0)
    INST_RANGE(Rate, ConstExprRate, GroupSharedRate)

    INST(RateQualifiedType, RateQualified, 2, 0)

    // Kinds represent the "types of types."
    // They should not really be nested under `IRType`
    // in the overall hierarchy, but we can fix that later.
    //
    /* Kind */
        INST(TypeKind, Type, 0, 0)
        INST(RateKind, Rate, 0, 0)
        INST(GenericKind, Generic, 0, 0)
    INST_RANGE(Kind, TypeKind, GenericKind)

    /* PtrTypeBase */
        INST(PtrType, Ptr, 1, 0)
        /* OutTypeBase */
            INST(OutType, Out, 1, 0)
            INST(InOutType, InOut, 1, 0)
        INST_RANGE(OutTypeBase, OutType, InOutType)
    INST_RANGE(PtrTypeBase, PtrType, InOutType)

    /* SamplerStateTypeBase */
        INST(SamplerStateType, SamplerState, 0, 0)
        INST(SamplerComparisonStateType, SamplerComparisonState, 0, 0)
    INST_RANGE(SamplerStateTypeBase, SamplerStateType, SamplerComparisonStateType)

    // TODO: Why do we have all this hierarchy here, when everything
    // that actually matters is currently nested under `TextureTypeBase`?
    /* ResourceTypeBase */
        /* ResourceType */
            /* TextureTypeBase */
                /* TextureType */
                MANUAL_INST_RANGE(TextureType, 0x10000, TextureFlavor::Count)
                /* TextureSamplerType */
                MANUAL_INST_RANGE(TextureSamplerType, 0x20000, TextureFlavor::Count)
                /* GLSLImageType */
                MANUAL_INST_RANGE(GLSLImageType, 0x30000, TextureFlavor::Count)
            INST_RANGE(TextureTypeBase, FirstTextureType, LastGLSLImageType)
        INST_RANGE(ResourceType, FirstTextureType, LastGLSLImageType)
    INST_RANGE(ResourceTypeBase, FirstTextureType, LastGLSLImageType)

    /* UntypedBufferResourceType */
        INST(HLSLByteAddressBufferType,      ByteAddressBuffer,   0, 0)
        INST(HLSLRWByteAddressBufferType,    RWByteAddressBuffer, 0, 0)
        INST(RaytracingAccelerationStructureType, RaytracingAccelerationStructure, 0, 0)
    INST_RANGE(UntypedBufferResourceType, HLSLByteAddressBufferType, RaytracingAccelerationStructureType)

    /* HLSLPatchType */
        INST(HLSLInputPatchType,    InputPatch,     2, 0)
        INST(HLSLOutputPatchType,   OutputPatch,    2, 0)
    INST_RANGE(HLSLPatchType, HLSLInputPatchType, HLSLOutputPatchType)

    INST(GLSLInputAttachmentType, GLSLInputAttachment, 0, 0)

    /* BuiltinGenericType */
        /* HLSLStreamOutputType */
            INST(HLSLPointStreamType,       PointStream,    1, 0)
            INST(HLSLLineStreamType,        LineStream,     1, 0)
            INST(HLSLTriangleStreamType,    TriangleStream, 1, 0)
        INST_RANGE(HLSLStreamOutputType, HLSLPointStreamType, HLSLTriangleStreamType)

        /* HLSLStructuredBufferTypeBase */
            INST(HLSLStructuredBufferType,          StructuredBuffer,           0, 0)
            INST(HLSLRWStructuredBufferType,        RWStructuredBuffer,         0, 0)
            INST(HLSLAppendStructuredBufferType,    AppendStructuredBuffer,     0, 0)
            INST(HLSLConsumeStructuredBufferType,   ConsumeStructuredBuffer,    0, 0)
        INST_RANGE(HLSLStructuredBufferTypeBase, HLSLStructuredBufferType, HLSLConsumeStructuredBufferType)

        /* PointerLikeType */
            /* ParameterGroupType */
                /* UniformParameterGroupType */
                    INST(ConstantBufferType, ConstantBuffer, 1, 0)
                    INST(TextureBufferType, TextureBuffer, 1, 0)
                    INST(ParameterBlockType, ParameterBlock, 1, 0)
                    INST(GLSLShaderStorageBufferType, GLSLShaderStorageBuffer, 0, 0)
                INST_RANGE(UniformParameterGroupType, ConstantBufferType, GLSLShaderStorageBufferType)
            
                /* VaryingParameterGroupType */
                    INST(GLSLInputParameterGroupType, GLSLInputParameterGroup, 0, 0)
                    INST(GLSLOutputParameterGroupType, GLSLOutputParameterGroup, 0, 0)
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
// Note: we are being a bit slippery here, because a `struct`
// instruction is really an `IRParentInst`, but we want it
// to also be caught in any dynamic cast to `IRType`, so we
// ensure that it comes at the *end* of the range for `IRType`,
// and the start of the range for `IRParentInst` (and `IRGlobalValue`)
INST(StructType, struct, 0, PARENT)

INST_RANGE(Type, VoidType, StructType)

/*IRParentInst*/

    /*IRGlobalValue*/

        /*IRGlobalValueWithCode*/
            /* IRGlobalValueWIthParams*/
                INST(Func, func, 0, PARENT)
                INST(Generic, generic, 0, PARENT)
            INST_RANGE(GlobalValueWithParams, Func, Generic)

            INST(GlobalVar, global_var, 0, 0)
            INST(GlobalConstant, global_constant, 0, 0)
        INST_RANGE(GlobalValueWithCode, Func, GlobalConstant)

        INST(StructKey, key, 0, 0)
        INST(GlobalGenericParam, global_generic_param, 0, 0)
        INST(WitnessTable, witness_table, 0, 0)

    INST_RANGE(GlobalValue, StructType, WitnessTable)

    INST(Module, module, 0, PARENT)

    INST(Block, block, 0, PARENT)

INST_RANGE(ParentInst, StructType, Block)

/* IRConstant */
    INST(boolConst, boolConst, 0, 0)
    INST(IntLit, integer_constant, 0, 0)
    INST(FloatLit, float_constant, 0, 0)
INST_RANGE(Constant, boolConst, FloatLit)

INST(undefined, undefined, 0, 0)

INST(Specialize, specialize, 2, 0)
INST(lookup_interface_method, lookup_interface_method, 2, 0)
INST(lookup_witness_table, lookup_witness_table, 2, 0)
INST(BindGlobalGenericParam, bind_global_generic_param, 2, 0)

INST(Construct, construct, 0, 0)

INST(makeVector, makeVector, 0, 0)
INST(makeMatrix, makeMatrix, 0, 0)
INST(makeArray, makeArray, 0, 0)
INST(makeStruct, makeStruct, 0, 0)

INST(Call, call, 1, 0)


INST(WitnessTableEntry, witness_table_entry, 2, 0)

INST(Param, param, 0, 0)
INST(StructField, field, 2, 0)
INST(Var, var, 0, 0)

INST(Load, load, 1, 0)
INST(Store, store, 2, 0)

INST(BufferLoad, bufferLoad, 2, 0)
INST(BufferStore, bufferStore, 3, 0)
INST(BufferElementRef, bufferElementRef, 2, 0)

INST(FieldExtract, get_field, 2, 0)
INST(FieldAddress, get_field_addr, 2, 0)

INST(getElement, getElement, 2, 0)
INST(getElementPtr, getElementPtr, 2, 0)

// Construct a vector from a scalar
//
// %dst = constructVectorFromScalar %T %N %val
//
// where
// - `T` is a `Type`
// - `N` is a (compile-time) `Int`
// - `val` is a `T`
// - dst is a `Vec<T,N>`
//
INST(constructVectorFromScalar, constructVectorFromScalar, 3, 0)

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


/* IRTerminatorInst */

    INST(ReturnVal, return_val, 1, 0)
    INST(ReturnVoid, return_void, 1, 0)

    // unconditionalBranch <target>
    INST(unconditionalBranch, unconditionalBranch, 1, 0)

    // loop <target> <breakLabel> <continueLabel>
    INST(loop, loop, 3, 0)

    // conditionalBranch <condition> <trueBlock> <falseBlock>
    INST(conditionalBranch, conditionalBranch, 3, 0)

    // ifElse <condition> <trueBlock> <falseBlock> <mergeBlock>
    INST(ifElse, ifElse, 4, 0)

    // switch <val> <break> <default> <caseVal1> <caseBlock1> ...
    INST(switch, switch, 3, 0)

    INST(discard, discard, 0, 0)
    INST(unreachable, unreachable, 0, 0)

INST_RANGE(TerminatorInst, ReturnVal, unreachable)

INST(Add, add, 2, 0)
INST(Sub, sub, 2, 0)
INST(Mul, mul, 2, 0)
INST(Div, div, 2, 0)
INST(Mod, mod, 2, 0)

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

INST(Dot, dot, 2, 0)

INST(Mul_Vector_Matrix, mulVectorMatrix, 2, 0)
INST(Mul_Matrix_Vector, mulMatrixVector, 2, 0)
INST(Mul_Matrix_Matrix, mulMatrixMatrix, 2, 0)

// Texture sampling operation of the form `t.Sample(s,u)`
INST(Sample, sample, 3, 0)

INST(SampleGrad, sampleGrad, 4, 0)

INST(GroupMemoryBarrierWithGroupSync, GroupMemoryBarrierWithGroupSync, 0, 0)

PSEUDO_INST(Pos)
PSEUDO_INST(PreInc)

PSEUDO_INST(PreDec)
PSEUDO_INST(PostInc)
PSEUDO_INST(PostDec)
PSEUDO_INST(Sequence)
PSEUDO_INST(AddAssign)
PSEUDO_INST(SubAssign)
PSEUDO_INST(MulAssign)
PSEUDO_INST(DivAssign)
PSEUDO_INST(ModAssign)
PSEUDO_INST(AndAssign)
PSEUDO_INST(OrAssign)
PSEUDO_INST(XorAssign )
PSEUDO_INST(LshAssign)
PSEUDO_INST(RshAssign)
PSEUDO_INST(Assign)
PSEUDO_INST(And)
PSEUDO_INST(Or)


#undef PSEUDO_INST
#undef PARENT
#undef MANUAL_INST_RANGE
#undef INST_RANGE
#undef INST

