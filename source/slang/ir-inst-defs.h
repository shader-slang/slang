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

INST(TypeType,    type.type,      0, 0)
INST(VoidType,    type.void,      0, 0)
INST(BlockType,   type.block,     0, 0)
INST(VectorType,  type.vector,    2, 0)
INST(BoolType,    type.bool,      0, 0)
INST(Float32Type, type.f32,       0, 0)
INST(Int32Type,   type.i32,       0, 0)
INST(UInt32Type,  type.u32,       0, 0)
INST(StructType,  type.struct,    0, PARENT)
INST(FuncType, func_type, 0, 0)
INST(PtrType, ptr_type, 1, 0)
INST(TextureType, texture_type, 2, 0)
INST(SamplerType, sampler_type, 1, 0)
INST(ConstantBufferType, constant_buffer_type, 1, 0)
INST(TextureBufferType, texture_buffer_type, 1, 0)
INST(readWriteStructuredBufferType, readWriteStructuredBufferType, 1, 0)

INST(IntLit, integer_constant, 0, 0)
INST(FloatLit, float_constant, 0, 0)

INST(Construct, construct, 0, 0)
INST(Call, call, 1, 0)

INST(Module, module, 0, PARENT)
INST(Func, func, 0, PARENT)
INST(Block, block, 0, PARENT)

INST(Param, param, 0, 0)
INST(StructField, field, 0, 0)
INST(Var, var, 0, 0)

INST(Load, load, 1, 0)
INST(Store, store, 2, 0)

INST(BufferLoad, bufferLoad, 2, 0)
INST(BufferStore, bufferStore, 3, 0)

INST(FieldExtract, get_field, 2, 0)
INST(FieldAddress, get_field_addr, 2, 0)

INST(ReturnVal, return_val, 1, 0)
INST(ReturnVoid, return_void, 1, 0)


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

