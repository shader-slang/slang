// intrinsic-defs.h

// The file is meant to be included multiple times, to produce different
// pieces of code related to intrinsic operations
//
// Each intrinsic op is declared here with:
//
//      INTRINSIC(name)
//

#ifndef INTRINSIC
#error Need to define INTRINSIC(NAME) before including "intrinsic-defs.h"
#endif

INTRINSIC(Add)
INTRINSIC(Sub)
INTRINSIC(Mul)
INTRINSIC(Div)
INTRINSIC(Mod)

INTRINSIC(Lsh)
INTRINSIC(Rsh)

INTRINSIC(Eql)
INTRINSIC(Neq)
INTRINSIC(Greater)
INTRINSIC(Less)
INTRINSIC(Geq)
INTRINSIC(Leq)
INTRINSIC(BitAnd)
INTRINSIC(BitXor)
INTRINSIC(BitOr)

// TODO(tfoley): need to distinguish short-circuiting and not...
INTRINSIC(And)
INTRINSIC(Or)

INTRINSIC(Assign)
INTRINSIC(AddAssign)
INTRINSIC(SubAssign)
INTRINSIC(MulAssign)
INTRINSIC(DivAssign)
INTRINSIC(ModAssign)
INTRINSIC(LshAssign)
INTRINSIC(RshAssign)
INTRINSIC(OrAssign)
INTRINSIC(AndAssign)
INTRINSIC(XorAssign)
INTRINSIC(Neg)
INTRINSIC(Not)
INTRINSIC(BitNot)
INTRINSIC(PreInc)
INTRINSIC(PreDec)
INTRINSIC(PostInc)
INTRINSIC(PostDec)

INTRINSIC(Sequence)
INTRINSIC(Select)

INTRINSIC(Mul_Scalar_Scalar)
INTRINSIC(Mul_Vector_Scalar)
INTRINSIC(Mul_Scalar_Vector)
INTRINSIC(Mul_Matrix_Scalar)
INTRINSIC(Mul_Scalar_Matrix)
INTRINSIC(InnerProduct_Vector_Vector)
INTRINSIC(InnerProduct_Vector_Matrix)
INTRINSIC(InnerProduct_Matrix_Vector)
INTRINSIC(InnerProduct_Matrix_Matrix)
























// Un-deefine the macor here, so that the client does not have to.
#undef INTRINSIC
