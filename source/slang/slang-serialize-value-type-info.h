// slang-serialize-value-type-info.h

#ifndef SLANG_SERIALIZE_VALUE_TYPE_INFO_H
#define SLANG_SERIALIZE_VALUE_TYPE_INFO_H

#include "slang-ast-support-types.h"

#include "slang-serialize.h"

#include "slang-serialize-misc-type-info.h"
#include "slang-serialize-type-info.h"

#include "slang-generated-value.h"
#include "slang-generated-value-macro.h"

// Create the functions to automatically convert between value types

namespace Slang {

// TODO(JS): We may want to strip const or other modifiers
// Just strips the brackets.
#define SLANG_VALUE_GET_TYPE(TYPE)  TYPE

#define SLANG_VALUE_FIELD_TO_SERIAL(FIELD_NAME, TYPE, param) SerialTypeInfo<decltype(src->FIELD_NAME)>::toSerial(writer, &src->FIELD_NAME, &dst->FIELD_NAME);
#define SLANG_VALUE_FIELD_TO_NATIVE(FIELD_NAME, TYPE, param) SerialTypeInfo<decltype(dst->FIELD_NAME)>::toNative(reader, &src->FIELD_NAME, &dst->FIELD_NAME);

#define SLANG_IF_HAS_SUPER_BASE(x)
#define SLANG_IF_HAS_SUPER_INNER(x) x
#define SLANG_IF_HAS_SUPER_LEAF(x) x

#define SLANG_VALUE_TO_SERIAL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
static void toSerial(SerialWriter* writer, const void* native, void* serial) \
{ \
    SLANG_IF_HAS_SUPER_##TYPE(SerialTypeInfo<SUPER>::toSerial(writer, native, serial);) \
    auto dst = (SerialType*)serial; \
    auto src = (const NativeType*)native; \
    SLANG_FIELDS_Value_##NAME(SLANG_VALUE_FIELD_TO_SERIAL, param) \
}

#define SLANG_VALUE_TO_NATIVE(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
static void toNative(SerialReader* reader, const void* serial, void* native) \
{ \
    SLANG_IF_HAS_SUPER_##TYPE(SerialTypeInfo<SUPER>::toNative(reader, serial, native);) \
    auto src = (const SerialType*)serial; \
    auto dst = (NativeType*)native; \
    SLANG_FIELDS_Value_##NAME(SLANG_VALUE_FIELD_TO_NATIVE, param) \
}

//#define SLANG_VALUE_SERIAL_FIELD(FIELD_NAME, TYPE, param) SerialTypeInfo<SLANG_VALUE_GET_TYPE TYPE>::SerialType FIELD_NAME;
#define SLANG_VALUE_SERIAL_FIELD(FIELD_NAME, TYPE, param) SerialTypeInfo<decltype(((param*)nullptr)->FIELD_NAME)>::SerialType FIELD_NAME;

#define SLANG_VALUE_SERIAL_STRUCT(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
struct SerialType SLANG_IF_HAS_SUPER_##TYPE( : SerialTypeInfo<SUPER>::SerialType) \
{ \
    SLANG_FIELDS_Value_##NAME(SLANG_VALUE_SERIAL_FIELD, NAME) \
}; 

#define SLANG_VALUE_TYPE_INFO_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
template <> \
struct SerialTypeInfo<NAME> \
{ \
    typedef NAME NativeType; \
    SLANG_VALUE_SERIAL_STRUCT(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    \
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) }; \
    \
    SLANG_VALUE_TO_NATIVE(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    SLANG_VALUE_TO_SERIAL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
};
    
#define SLANG_VALUE_TYPE_INFO(NAME) \
    SLANG_Value_##NAME(SLANG_VALUE_TYPE_INFO_IMPL, _)


} // namespace Slang

#endif // SLANG_SERIALIZE_VALUE_TYPE_INFO_H
