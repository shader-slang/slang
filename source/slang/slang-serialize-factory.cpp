// slang-serialize-factory.cpp
#include "slang-serialize-factory.h"

#include "../core/slang-math.h"

#include "slang-ast-builder.h"

#include "slang-ref-object-reflect.h"
#include "slang-ast-reflect.h"

#include "slang-serialize-ast.h"
#include "slang-ref-object-reflect.h"

// Needed for ModuleSerialFilter
// Needed for 'findModuleForDecl'
#include "slang-legalize-types.h"
#include "slang-mangle.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!! DefaultSerialObjectFactory !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* DefaultSerialObjectFactory::create(SerialTypeKind typeKind, SerialSubType subType)
{
    switch (typeKind)
    {
        case SerialTypeKind::NodeBase:
        {
            return m_astBuilder->createByNodeType(ASTNodeType(subType));
        }
        case SerialTypeKind::RefObject:
        {
            const ReflectClassInfo* info = SerialRefObjects::getClassInfo(RefObjectType(subType));

            if (info && info->m_createFunc)
            {
                RefObject* obj = reinterpret_cast<RefObject*>(info->m_createFunc(nullptr));
                return _add(obj);
            }
            return nullptr;
        }
        default: break;
    }

    return nullptr;
}

void* DefaultSerialObjectFactory::getOrCreateVal(ValNodeDesc&& desc)
{
    return m_astBuilder->_getOrCreateImpl(_Move(desc));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ModuleSerialFilter  !!!!!!!!!!!!!!!!!!!!!!!!

SerialIndex ModuleSerialFilter::writePointer(SerialWriter* writer, const RefObject* inPtr)
{
    // We don't serialize Module
    if (as<Module>(inPtr))
    {
        writer->setPointerIndex(inPtr, SerialIndex(0));
        return SerialIndex(0);
    }

    // For now for everything else just write it
    return writer->writeObject(inPtr);
}

SerialIndex ModuleSerialFilter::writePointer(SerialWriter* writer, const NodeBase* inPtr)
{
    NodeBase* ptr = const_cast<NodeBase*>(inPtr);
    SLANG_ASSERT(ptr);


    if (Decl* decl = as<Decl>(ptr))
    {
        ModuleDecl* moduleDecl = findModuleForDecl(decl);
        if (moduleDecl && moduleDecl != m_moduleDecl)
        {
            ASTBuilder* astBuilder = m_moduleDecl->module->getASTBuilder();

            // It's a reference to a declaration in another module, so first get the symbol name.
            // Note that we will always name an import symbol in the form of
            // <module_name>!<symbol_mangled_name> for serialization.
            // This is because <symbol_mangled_name> does not necessarily include the name of its
            // parent module when it is qualified as `extern` or `export`.
            //
            String mangledName = getText(moduleDecl->getName()) +"!"+ getMangledName(astBuilder, decl);

            // Add as an import symbol
            return writer->addImportSymbol(mangledName);
        }
        else
        {
            // Okay... we can just write it out then
            return writer->writeObject(ptr);
        }
    }
    // For now for everything else just write it
    return writer->writeObject(ptr);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialClassesUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */SlangResult SerialClassesUtil::addSerialClasses(SerialClasses* serialClasses)
{
    ASTSerialUtil::addSerialClasses(serialClasses);
    SerialRefObjects::addSerialClasses(serialClasses);

    // Check if it seems ok
    SLANG_ASSERT(serialClasses->isOk());

    return SLANG_OK;
}

/* static */SlangResult SerialClassesUtil::create(RefPtr<SerialClasses>& out)
{
    RefPtr<SerialClasses> classes(new SerialClasses);
    SLANG_RETURN_ON_FAIL(addSerialClasses(classes));

    out = classes;
    return SLANG_OK;
}

} // namespace Slang
