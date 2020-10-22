#include "../../slang.h"

#include "slang-ref-object-reflect.h"

#include "slang-generated-obj.h"
#include "slang-generated-obj-macro.h"

#include "slang-ast-support-types.h"

//#include "slang-serialize.h"

#include "slang-serialize-ast-type-info.h"

namespace Slang
{

static const SerialClass* _addClass(SerialClasses* serialClasses, RefObjectType type, RefObjectType super, const List<SerialField>& fields)
{
    const SerialClass* superClass = serialClasses->getSerialClass(SerialTypeKind::RefObject, SerialSubType(super));
    return serialClasses->add(SerialTypeKind::RefObject, SerialSubType(type), fields.getBuffer(), fields.getCount(), superClass);
}

#define SLANG_REF_OBJECT_ADD_SERIAL_FIELD(FIELD_NAME, TYPE, param) fields.add(SerialField::make(#FIELD_NAME, &obj->FIELD_NAME));

// Note that the obj point is not nullptr, because some compilers notice this is 'indexing from null'
// and warn/error. So we offset from 1.
#define SLANG_REF_OBJECT_ADD_SERIAL_CLASS(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
{ \
    NAME* obj = SerialField::getPtr<NAME>(); \
    SLANG_UNUSED(obj); \
    fields.clear(); \
    SLANG_FIELDS_RefObject_##NAME(SLANG_REF_OBJECT_ADD_SERIAL_FIELD, param) \
    _addClass(serialClasses, RefObjectType::NAME, RefObjectType::SUPER, fields); \
}

struct RefObjectAccess
{
    template <typename T>
    static void* create(void* context)
    {
        SLANG_UNUSED(context)
        return new T;
    }

    static void calcClasses(SerialClasses* serialClasses)
    {
        // Add SerialRefObject first, and specially handle so that we add a null super class
        serialClasses->add(SerialTypeKind::RefObject, SerialSubType(RefObjectType::SerialRefObject), nullptr, 0, nullptr);

        // Add the rest in order such that Super class is always added before its children
        List<SerialField> fields;
        SLANG_CHILDREN_RefObject_SerialRefObject(SLANG_REF_OBJECT_ADD_SERIAL_CLASS, _)
    }
};

#define SLANG_GET_SUPER_BASE(SUPER) nullptr
#define SLANG_GET_SUPER_INNER(SUPER) &SUPER::kReflectClassInfo
#define SLANG_GET_SUPER_LEAF(SUPER) &SUPER::kReflectClassInfo

#define SLANG_GET_CREATE_FUNC_NONE(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_OBJ_ABSTRACT(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_OBJ(NAME) &RefObjectAccess::create<NAME>

#define SLANG_REFLECT_CLASS_INFO(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    /* static */const ReflectClassInfo NAME::kReflectClassInfo = { uint32_t(RefObjectType::NAME), uint32_t(RefObjectType::LAST), SLANG_GET_SUPER_##TYPE(SUPER), #NAME, SLANG_GET_CREATE_FUNC_##MARKER(NAME), nullptr, uint32_t(sizeof(NAME)), uint8_t(SLANG_ALIGN_OF(NAME)) };

SLANG_ALL_RefObject_SerialRefObject(SLANG_REFLECT_CLASS_INFO, _)

/* static */const SerialRefObjects SerialRefObjects::g_singleton;

// Macro to set all of the entries in m_infos for SerialRefObjects
#define SLANG_GET_REFLECT_CLASS_INFO(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) m_infos[Index(RefObjectType::NAME)] = &NAME::kReflectClassInfo; 

SerialRefObjects::SerialRefObjects()
{
    SLANG_ALL_RefObject_SerialRefObject(SLANG_GET_REFLECT_CLASS_INFO, _)
}

/* static */SlangResult SerialRefObjects::addSerialClasses(SerialClasses* serialClasses)
{
    RefObjectAccess::calcClasses(serialClasses);
    return SLANG_OK;
}

} // namespace Slang
