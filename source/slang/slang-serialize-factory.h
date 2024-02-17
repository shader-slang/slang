// slang-serialize-factory.h
#ifndef SLANG_SERIALIZE_FACTORY_H
#define SLANG_SERIALIZE_FACTORY_H

#include "slang-serialize.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!! DefaultSerialObjectFactory !!!!!!!!!!!!!!!!!!!!!!!!!!!

class ASTBuilder;

class DefaultSerialObjectFactory : public SerialObjectFactory
{
public:

    virtual void* create(SerialTypeKind typeKind, SerialSubType subType) SLANG_OVERRIDE;
    virtual void* getOrCreateVal(ValNodeDesc&& desc) SLANG_OVERRIDE;

    DefaultSerialObjectFactory(ASTBuilder* astBuilder) :
        m_astBuilder(astBuilder)
    {
    }

protected:
    RefObject* _add(RefObject* obj)
    {
        m_scope.add(obj);
        return obj;
    }

    // We keep RefObjects in scope 
    List<RefPtr<RefObject>> m_scope;
    ASTBuilder* m_astBuilder;
};


struct SerialClassesUtil
{
        /// Add all types to serialClasses
    static SlangResult addSerialClasses(SerialClasses* serialClasses);
        /// Create SerialClasses with all the types added
    static SlangResult create(RefPtr<SerialClasses>& out);
};


} // namespace Slang

#endif
