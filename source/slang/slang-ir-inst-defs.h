// slang-ir-inst-defs.h

#ifndef INST
#error Must #define `INST` before including `ir-inst-defs.h`
#endif

#ifndef INST_RANGE
#define INST_RANGE(BASE, FIRST, LAST) /* empty */
#endif

#ifndef PSEUDO_INST
#define PSEUDO_INST(ID) /* empty */
#endif

#define PARENT kIROpFlag_Parent
#define USE_OTHER kIROpFlag_UseOther

INST(Nop, nop, 0, 0)

/* Types */

    /* Basic Types */

    #define DEFINE_BASE_TYPE_INST(NAME) INST(NAME ## Type, NAME, 0, 0)
    FOREACH_BASE_TYPE(DEFINE_BASE_TYPE_INST)
    #undef DEFINE_BASE_TYPE_INST
    INST(AfterBaseType, afterBaseType, 0, 0)

    INST_RANGE(BasicType, VoidType, AfterBaseType)

    INST(StringType, String, 0, 0)

    /* ArrayTypeBase */
        INST(ArrayType, Array, 2, 0)
        INST(UnsizedArrayType, UnsizedArray, 1, 0)
    INST_RANGE(ArrayTypeBase, ArrayType, UnsizedArrayType)

    INST(FuncType, Func, 0, 0)
    INST(BasicBlockType, BasicBlock, 0, 0)

    INST(VectorType, Vec, 2, 0)
    INST(MatrixType, Mat, 3, 0)

    INST(TaggedUnionType, TaggedUnion, 0, 0)

    // A `BindExistentials<B, T0,w0, T1,w1, ...>` represents
    // taking type `B` and binding each of its existential type
    // parameters, recursively, with the specified arguments,
    // where each `Ti, wi` pair represents the concrete type
    // and witness table to plug in for parameter `i`.
    //
    INST(BindExistentialsType, BindExistentials, 1, 0)

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
        INST(RefType, Ref, 1, 0)

        // An `ExistentialBox<T>` represents a logical pointer to a value of type `T`.
        // On targets that support pointers this might lower to a pointer, but on
        // current targets it will lower to zero bytes, with a value of type `T`
        // being stored "out of line" somewhere.
        //
        INST(ExistentialBoxType, ExistentialBox, 1, 0)

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
                // NOTE! TextureFlavor::Flavor is stored in 'other' bits for these types.
                /* TextureType */
                INST(TextureType, TextureType, 0, USE_OTHER)
                /* TextureSamplerType */
                INST(TextureSamplerType, TextureSamplerType, 0, USE_OTHER)
                /* GLSLImageType */
                INST(GLSLImageType, GLSLImageType, 0, USE_OTHER)
            INST_RANGE(TextureTypeBase, TextureType, GLSLImageType)
        INST_RANGE(ResourceType, TextureType, GLSLImageType)
    INST_RANGE(ResourceTypeBase, TextureType, GLSLImageType)


    /* UntypedBufferResourceType */
        /* ByteAddressBufferTypeBase */
            INST(HLSLByteAddressBufferType,                     ByteAddressBuffer,   0, 0)
            INST(HLSLRWByteAddressBufferType,                   RWByteAddressBuffer, 0, 0)
            INST(HLSLRasterizerOrderedByteAddressBufferType,    RasterizerOrderedByteAddressBuffer, 0, 0)
        INST_RANGE(ByteAddressBufferTypeBase, HLSLByteAddressBufferType, HLSLRasterizerOrderedByteAddressBufferType)
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
            INST(HLSLStructuredBufferType,                  StructuredBuffer,                   0, 0)
            INST(HLSLRWStructuredBufferType,                RWStructuredBuffer,                 0, 0)
            INST(HLSLRasterizerOrderedStructuredBufferType, RasterizerOrderedStructuredBuffer,  0, 0)
            INST(HLSLAppendStructuredBufferType,            AppendStructuredBuffer,             0, 0)
            INST(HLSLConsumeStructuredBufferType,           ConsumeStructuredBuffer,            0, 0)
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
INST(StructType, struct, 0, PARENT)
INST(InterfaceType, interface, 0, PARENT)

INST_RANGE(Type, VoidType, InterfaceType)

/*IRGlobalValueWithCode*/
    /* IRGlobalValueWIthParams*/
        INST(Func, func, 0, PARENT)
        INST(Generic, generic, 0, PARENT)
    INST_RANGE(GlobalValueWithParams, Func, Generic)

    INST(GlobalVar, global_var, 0, 0)
INST_RANGE(GlobalValueWithCode, Func, GlobalVar)

INST(GlobalParam, global_param, 0, 0)
INST(GlobalConstant, globalConstant, 0, 0)

INST(StructKey, key, 0, 0)
INST(GlobalGenericParam, global_generic_param, 0, 0)
INST(WitnessTable, witness_table, 0, 0)

INST(Module, module, 0, PARENT)

INST(Block, block, 0, PARENT)

/* IRConstant */
    INST(BoolLit, boolConst, 0, 0)
    INST(IntLit, integer_constant, 0, 0)
    INST(FloatLit, float_constant, 0, 0)
    INST(PtrLit, ptr_constant, 0, 0)
    INST(StringLit, string_constant, 0, 0)
INST_RANGE(Constant, BoolLit, StringLit)

INST(undefined, undefined, 0, 0)

INST(Specialize, specialize, 2, 0)
INST(lookup_interface_method, lookup_interface_method, 2, 0)
INST(lookup_witness_table, lookup_witness_table, 2, 0)
INST(BindGlobalGenericParam, bind_global_generic_param, 2, 0)

INST(Construct, construct, 0, 0)

INST(makeVector, makeVector, 0, 0)
INST(MakeMatrix, makeMatrix, 0, 0)
INST(makeArray, makeArray, 0, 0)
INST(makeStruct, makeStruct, 0, 0)

INST(Call, call, 1, 0)


INST(WitnessTableEntry, witness_table_entry, 2, 0)

INST(Param, param, 0, 0)
INST(StructField, field, 2, 0)
INST(Var, var, 0, 0)

INST(Load, load, 1, 0)
INST(Store, store, 2, 0)

INST(FieldExtract, get_field, 2, 0)
INST(FieldAddress, get_field_addr, 2, 0)

INST(getElement, getElement, 2, 0)
INST(getElementPtr, getElementPtr, 2, 0)

// "Subscript" an image at a pixel coordinate to get pointer
INST(ImageSubscript, imageSubscript, 2, 0)

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

    INST(ReturnVal, return_val, 1, 0)
    INST(ReturnVoid, return_void, 1, 0)

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

    // switch <val> <break> <default> <caseVal1> <caseBlock1> ...
    INST(Switch, switch, 3, 0)

    INST(discard, discard, 0, 0)

    /* IRUnreachable */
        INST(MissingReturn, missingReturn, 0, 0)
        INST(Unreachable, unreachable, 0, 0)
    INST_RANGE(Unreachable, MissingReturn, Unreachable)

INST_RANGE(TerminatorInst, ReturnVal, Unreachable)

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

INST(Dot, dot, 2, 0)

INST(Mul_Vector_Matrix, mulVectorMatrix, 2, 0)
INST(Mul_Matrix_Vector, mulMatrixVector, 2, 0)
INST(Mul_Matrix_Matrix, mulMatrixMatrix, 2, 0)

// Texture sampling operation of the form `t.Sample(s,u)`
INST(Sample, sample, 3, 0)

INST(SampleGrad, sampleGrad, 4, 0)

INST(GroupMemoryBarrierWithGroupSync, GroupMemoryBarrierWithGroupSync, 0, 0)

/* Decoration */

INST(HighLevelDeclDecoration,               highLevelDecl,          1, 0)
    INST(LayoutDecoration,                  layout,                 1, 0)
    INST(LoopControlDecoration,             loopControl,            1, 0)
    /* TargetSpecificDecoration */
        INST(TargetDecoration,              target,                 1, 0)
        INST(TargetIntrinsicDecoration,     targetIntrinsic,        2, 0)
    INST_RANGE(TargetSpecificDecoration, TargetDecoration, TargetIntrinsicDecoration)
    INST(GLSLOuterArrayDecoration,          glslOuterArray,         1, 0)
    INST(SemanticDecoration,                semantic,               1, 0)
    INST(InterpolationModeDecoration,       interpolationMode,      1, 0)
    INST(NameHintDecoration,                nameHint,               1, 0)

    /**  The decorated _instruction_ is transitory. Such a decoration should NEVER be found on an output instruction a module. 
        Typically used mark an instruction so can be specially handled - say when creating a IRConstant literal, and the payload of 
        needs to be special cased for lookup. */
    INST(TransitoryDecoration,              transitory,             0, 0)

    INST(VulkanRayPayloadDecoration,        vulkanRayPayload,       0, 0)
    INST(VulkanHitAttributesDecoration,     vulkanHitAttributes,    0, 0)
    INST(RequireGLSLVersionDecoration,      requireGLSLVersion,     1, 0)
    INST(RequireGLSLExtensionDecoration,    requireGLSLExtension,   1, 0)
    INST(ReadNoneDecoration,                readNone,               0, 0)
    INST(VulkanCallablePayloadDecoration,   vulkanCallablePayload,  0, 0)
    INST(EarlyDepthStencilDecoration,       earlyDepthStencil,      0, 0)
    INST(GloballyCoherentDecoration,        globallyCoherent,       0, 0)
    INST(PreciseDecoration,                 precise,       0, 0)
    INST(PatchConstantFuncDecoration,       patchConstantFunc,      1, 0)

        /// An `[entryPoint]` decoration marks a function that represents a shader entry point.
        /// Also used in some scenarios mark parameters that are moved from entry point parameters to global params as coming from the entry point.
    INST(EntryPointDecoration,              entryPoint,             0, 0)

        /// A `[dependsOn(x)]` decoration indicates that the parent instruction depends on `x`
        /// even if it does not otherwise reference it.
    INST(DependsOnDecoration,               dependsOn,              1, 0)

        /// A `[keepAlive]` decoration marks an instruction that should not be eliminated.
    INST(KeepAliveDecoration,              keepAlive,             0, 0)

    INST(BindExistentialSlotsDecoration, bindExistentialSlots, 0, 0)

        /// A `[format(f)]` decoration specifies that the format of an image should be `f`
    INST(FormatDecoration, format, 1, 0)

    /* LinkageDecoration */
        INST(ImportDecoration, import, 1, 0)
        INST(ExportDecoration, export, 1, 0)
    INST_RANGE(LinkageDecoration, ImportDecoration, ExportDecoration)

INST_RANGE(Decoration, HighLevelDeclDecoration, ExportDecoration)


//

// A `makeExistential(v : C, w) : I` instruction takes a value `v` of type `C`
// and produces a value of interface type `I` by using the witness `w` which
// shows that `C` conforms to `I`.
//
INST(MakeExistential,                   makeExistential,                2, 0)

// A `wrapExistential(v, T0,w0, T1,w0) : T` instruction is similar to `makeExistential`.
// but applies to a value `v` that is of type `BindExistentials(T, T0,w0, ...)`. The
// result of the `wrapExistentials` operation is a value of type `T`, allowing us to
// "smuggle" a value of specialized type into computations that expect an unspecialized type.
//
INST(WrapExistential,                   wrapExistential,                2, 0)

INST(ExtractExistentialValue,           extractExistentialValue,        1, 0)
INST(ExtractExistentialType,            extractExistentialType,         1, 0)
INST(ExtractExistentialWitnessTable,    extractExistentialWitnessTable, 1, 0)

INST(ExtractTaggedUnionTag,             extractTaggedUnionTag,      1, 0)
INST(ExtractTaggedUnionPayload,         extractTaggedUnionPayload,  1, 0)

INST(BitCast,                           bitCast,                    1, 0)

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
PSEUDO_INST(IRemAssign)
PSEUDO_INST(FRemAssign)
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
#undef USE_OTHER
#undef INST_RANGE
#undef INST

