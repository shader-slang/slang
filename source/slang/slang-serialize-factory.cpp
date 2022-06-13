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

    // We don't serialize Scope
    if (as<Scope>(ptr))
    {
        writer->setPointerIndex(inPtr, SerialIndex(0));
        return SerialIndex(0);
    }

    if (Decl* decl = as<Decl>(ptr))
    {
        ModuleDecl* moduleDecl = findModuleForDecl(decl);
        if (moduleDecl && moduleDecl != m_moduleDecl)
        {
            ASTBuilder* astBuilder = m_moduleDecl->module->getASTBuilder();

            // It's a reference to a declaration in another module, so first get the symbol name. 
            String mangledName = getMangledName(astBuilder, decl);
            // Add as an import symbol
            return writer->addImportSymbol(mangledName);
        }
        else
        {
            // Okay... we can just write it out then
            return writer->writeObject(ptr);
        }
    }

    // TODO(JS): If I enable this section then the stdlib doesn't work correctly, it appears to be because of
    // `addCatchAllIntrinsicDecorationIfNeeded`. If this is enabled when AST is serialized, the 'body' (ie Stmt)
    // will not be serialized. When serialized back in, it will appear to be a function without a body.
    // In that case `addCatchAllIntrinsicDecorationIfNeeded` will add an intrinsic which in some cases is incorrect.
    // This happens during lowering. 
    //
    // So it seems the fix is for some other mechanism. Another solution is perhaps to run something like `addCatchAllIntrinsicDecorationIfNeeded`
    // on the stdlib after compilation, and before serialization. Then removing it from lowering.

#if 0
    // TODO(JS): What we really want to do here is to ignore bodies functions.
    // It's not 100% clear if this is even right though - for example does type inference
    // imply the body is needed to say infer a return type?
    // Also not clear if statements in other scenarios (if there are others) might need to be kept.
    //
    // For now we just ignore all stmts

    // TODO(yong): We should by default serialize everything. The logic to skip bodies need to be
    // behind a option flag.
    if (Stmt* stmt = as<Stmt>(ptr))
    {
        //
        writer->setPointerIndex(stmt, SerialIndex(0));
        return SerialIndex(0);
    }
#endif

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
