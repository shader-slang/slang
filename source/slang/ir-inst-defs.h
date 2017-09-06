// ir-inst-defs.h

#ifndef INST
#error Must #define `INST` before including `ir-inst-defs.h`
#endif

#define PARENT kIROpFlag_Parent

INST(TypeType,    type.type,      0, 0)
INST(VoidType,    type.void,      0, 0)
INST(BlockType,   type.block,     0, 0)
INST(VectorType,  type.vector,    2, 0)
INST(BoolType,    type.bool,      0, 0)
INST(Float32Type, type.f32,       0, 0)
INST(Int32Type,   type.i32,       0, 0)
INST(UInt32Type,  type.u32,       0, 0)
INST(StructType,  type.struct,    0, PARENT)
INST(FuncType,    func_type,      0, 0)
INST(PtrType,       ptr_type,       1, 0)
INST(TextureType,   texture_type,   2, 0)
INST(SamplerType,   sampler_type,   1, 0)
INST(ConstantBufferType,    constant_buffer_type,   1, 0)
INST(TextureBufferType,     texture_buffer_type,   1, 0)

INST(IntLit,      integer_constant,   0, 0)
INST(FloatLit,    float_constant,     0, 0)

INST(Construct,   construct,        0, 0)
INST(Call,          call,             1, 0)

INST(Module,      module, 0, PARENT)
INST(Func,        func,   0, PARENT)
INST(Block,       block,  0, PARENT)

INST(Param,         param,  0, 0)
INST(StructField,   field,  0, 0)
INST(Var,           var,    0, 0)

INST(Load,          load,   1, 0)
INST(Store,         store,  2, 0)

INST(FieldExtract,    get_field,        2, 0)
INST(FieldAddress,    get_field_addr,   2, 0)

INST(ReturnVal,       return_val,     1, 0)
INST(ReturnVoid, return_void, 1, 0)

#define INTRINSIC(NAME)                     \
    INST(Intrinsic_##NAME, intrinsic.NAME, 0, 0)
#include "intrinsic-defs.h"

#undef PARENT
#undef INST

