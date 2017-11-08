// ir-inst-defs.h

#ifndef INST
#error Must #define `INST` before including `ir-inst-defs.h`
#endif

#ifndef PSEUDO_INST
#define PSEUDO_INST(ID) /* empty */
#endif

#define PARENT kIROpFlag_Parent

// Invalid operation: should not appear in valid code
INST(Nop, nop, 0, 0)

INST(TypeType, Type, 0, 0)
INST(VoidType, Void, 0, 0)
INST(BlockType, Block, 0, 0)
INST(VectorType, Vec, 2, 0)
INST(MatrixType, Mat, 3, 0)
INST(arrayType, Array, 2, 0)

INST(BoolType, Bool, 0, 0)

INST(Float16Type, Float16, 0, 0)
INST(Float32Type, Float32, 0, 0)
INST(Float64Type, Float64, 0, 0)

// Signed integer types.
// Note that `IntPtr` represents a pointer-sized integer type,
// and will end up being equivalent to either `Int32` or `Int64`
// when it comes time to actually generate code.
//
INST(Int8Type, Int8, 0, 0)
INST(Int16Type, Int16, 0, 0)
INST(Int32Type, Int32, 0, 0)
INST(IntPtrType, IntPtr, 0, 0)
INST(Int64Type, Int64, 0, 0)

// Unlike a lot of other IRs, we retain a distinction between
// signed and unsigned integer types, simply because many of
// the target languages we need to generate code for also
// keep this distinction, and it will help us generate variable
// declarations that will be friendly to debuggers.
//
// TODO: We may want to reconsider this choice simply because
// some targets (e.g., those based on C++) may have undefined
// behavior around operations on signed integers that are
// well-defined (two's complement) on unsigned integers. In
// those cases we either want to default to unsigned integers,
// and then cast around the few ops that care about the difference,
// or else we want to keep using the orignal types, but need
// to cast around any ordinary math operations on signed types.
//
INST(UInt8Type, Int8, 0, 0)
INST(UInt16Type, Int16, 0, 0)
INST(UInt32Type, Int32, 0, 0)
INST(UIntPtrType, IntPtr, 0, 0)
INST(UInt64Type, Int64, 0, 0)

// A user-defined structure declaration at the IR level.
// Unlike in the AST where there is a distinction between
// a `StructDecl` and a `DeclRefType` that refers to it,
// at the IR level the struct declaration and the type
// are the same IR instruction.
//
// This is a parent instruction that holds zero or more
// `field` instructions.
//
INST(StructType, Struct, 0, PARENT)

INST(FuncType, Func, 0, 0)
INST(PtrType, Ptr, 1, 0)
INST(TextureType, Texture, 2, 0)
INST(SamplerType, SamplerState, 1, 0)
INST(ConstantBufferType, ConstantBuffer, 1, 0)
INST(TextureBufferType, TextureBuffer, 1, 0)

INST(structuredBufferType, StructuredBuffer, 1, 0)
INST(readWriteStructuredBufferType, RWStructuredBuffer, 1, 0)

// A type use to represent an earlier generic parameter in
// a signature. For example, given an AST declaration like:
//
//     func Foo<T, U>(int a, T b) -> U;
//
// The lowered function type would be something like:
//
//      T     U     a      b
//     (Type, Type, Int32, GenericParameterType<0>) -> GenericParameterType<1>
//
INST(GenericParameterType, GenericParameterType, 1, 0)

INST(boolConst, boolConst, 0, 0)
INST(IntLit, integer_constant, 0, 0)
INST(FloatLit, float_constant, 0, 0)
INST(decl_ref, decl_ref, 0, 0)

INST(specialize, specialize, 2, 0)
INST(lookup_interface_method, lookup_interface_method, 2, 0)

INST(Construct, construct, 0, 0)
INST(Call, call, 1, 0)

INST(Module, module, 0, PARENT)
INST(Func, func, 0, PARENT)
INST(Block, block, 0, PARENT)

INST(global_var, global_var, 0, 0)
INST(witness_table, witness_table, 0, 0)
INST(witness_table_entry, witness_table_entry, 2, 0)

INST(Param, param, 0, 0)
INST(StructField, field, 0, 0)
INST(Var, var, 0, 0)

INST(Load, load, 1, 0)
INST(Store, store, 2, 0)

INST(BufferLoad, bufferLoad, 2, 0)
INST(BufferStore, bufferStore, 3, 0)

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


INST(ReturnVal, return_val, 1, 0)
INST(ReturnVoid, return_void, 1, 0)

INST(unconditionalBranch, unconditionalBranch, 1, 0)
INST(break, break, 1, 0)
INST(continue, continue, 1, 0)
INST(loop, loop, 3, 0)

INST(conditionalBranch, conditionalBranch, 1, 0)
INST(if, if, 3, 0)
INST(ifElse, ifElse, 4, 0)
INST(loopTest, loopTest, 3, 0)

INST(discard, discard, 0, 0)

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

#if 0
INST(Assign, assign, 2, 0)
INST(AddAssign, addAssign, 2, 0)
INST(SubAssign, subAssign, 2, 0)
INST(SubAssign, subAssign, 2, 0)

INTRINSIC(SubAssign)
INTRINSIC(MulAssign)
INTRINSIC(DivAssign)
INTRINSIC(ModAssign)
INTRINSIC(LshAssign)
INTRINSIC(RshAssign)
INTRINSIC(OrAssign)
INTRINSIC(AndAssign)
INTRINSIC(XorAssign)
INTRINSIC(Pos)
#endif

INST(Neg, neg, 1, 0)
INST(Not, not, 1, 0)

#if 0
INTRINSIC(PreInc)
INTRINSIC(PreDec)
INTRINSIC(PostInc)
INTRINSIC(PostDec)

INTRINSIC(Sequence)
#endif

INST(Select, select, 3, 0)

INST(Dot, dot, 2, 0)

INST(Mul_Vector_Matrix, mulVectorMatrix, 2, 0)
INST(Mul_Matrix_Vector, mulMatrixVector, 2, 0)
INST(Mul_Matrix_Matrix, mulMatrixMatrix, 2, 0)

#if 0
INTRINSIC(Mul_Scalar_Scalar)
INTRINSIC(Mul_Vector_Scalar)
INTRINSIC(Mul_Scalar_Vector)
INTRINSIC(Mul_Matrix_Scalar)
INTRINSIC(Mul_Scalar_Matrix)
INTRINSIC(InnerProduct_Vector_Vector)
INTRINSIC(InnerProduct_Vector_Matrix)
INTRINSIC(InnerProduct_Matrix_Vector)
INTRINSIC(InnerProduct_Matrix_Matrix)
#endif

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
PSEUDO_INST(BitNot)
PSEUDO_INST(And)
PSEUDO_INST(Or)


#undef PSEUDO_INST
#undef PARENT
#undef INST

