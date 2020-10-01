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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ModuleSerialFilter  !!!!!!!!!!!!!!!!!!!!!!!!

SerialIndex ModuleSerialFilter::writePointer(SerialWriter* writer, const RefObject* inPtr)
{
    // We don't serialize Module or Scope
    if (as<Module>(inPtr) || as<Scope>(inPtr))
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
        SLANG_ASSERT(moduleDecl);
        if (moduleDecl && moduleDecl != m_moduleDecl)
        {
            ASTBuilder* astBuilder = m_moduleDecl->module->getASTBuilder();

            // It's a reference to a declaration in another module, so create an ImportExternalDecl.

            String mangledName = getMangledName(astBuilder, decl);

            ImportExternalDecl* importDecl = astBuilder->create<ImportExternalDecl>();
            importDecl->mangledName = mangledName;
            const SerialIndex index = writer->addPointer(importDecl);

            // Set as the index of this
            writer->setPointerIndex(ptr, index);

            return index;
        }
        else
        {
            // Okay... we can just write it out then
            return writer->writeObject(ptr);
        }
    }

    // TODO(JS): What we really want to do here is to ignore bodies functions.
    // It's not 100% clear if this is even right though - for example does type inference
    // imply the body is needed to say infer a return type?
    // Also not clear if statements in other scenarios (if there are others) might need to be kept.
    //
    // For now we just ignore all stmts

    if (Stmt* stmt = as<Stmt>(ptr))
    {
        //
        writer->setPointerIndex(stmt, SerialIndex(0));
        return SerialIndex(0);
    }

    // For now for everything else just write it
    return writer->writeObject(ptr);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialClassesUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */SlangResult SerialClassesUtil::add(SerialClasses* serialClasses)
{
    ASTSerialUtil::addSerialClasses(serialClasses);
    SerialRefObjects::addSerialClasses(serialClasses);

    return SLANG_OK;
}

/* static */SlangResult SerialClassesUtil::create(RefPtr<SerialClasses>& out)
{
    RefPtr<SerialClasses> classes(new SerialClasses);
    SLANG_RETURN_ON_FAIL(add(classes));

    out = classes;
    return SLANG_OK;
}

} // namespace Slang
