#include "../../slang.h"

#include "slang-ref-object-reflect.h"

#include "slang-ref-object-generated.h"
#include "slang-ref-object-generated-macro.h"

#include "slang-ast-support-types.h"

#include "slang-serialize.h"

#include "slang-serialize-ast-type-info.h"

namespace Slang
{


struct RefObjectAccess
{
    template <typename T>
    static void* create(void* context)
    {
        SLANG_UNUSED(context)
        return new T;
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
    {
        // Let's hack Breadcrumbs...

        typedef LookupResultItem::Breadcrumb Type;
        SerialField field = SerialField::make("_", SerialField::getPtr<Type>());
        serialClasses->add(SerialTypeKind::RefObject, SerialSubType(RefObjectType::LookupResultItem_Breadcrumb), &field, 1, nullptr);
    }

    return SLANG_OK;
}

} // namespace Slang
