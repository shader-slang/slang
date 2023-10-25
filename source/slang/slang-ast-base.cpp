#include "slang-ast-base.h"
#include "slang-ast-builder.h"

namespace Slang
{
    void NodeBase::_initDebug(ASTNodeType inAstNodeType, ASTBuilder* inAstBuilder)
    {
#ifdef _DEBUG
        SLANG_UNUSED(inAstNodeType);
        static int32_t uidCounter = 0;
        static int32_t breakValue = 0;
        uidCounter++;
        _debugUID = uidCounter;
        if (inAstBuilder->getId() == -1)
            _debugUID = -_debugUID;
        if (breakValue != 0 && _debugUID == breakValue)
            SLANG_BREAKPOINT(0)
#else
        SLANG_UNUSED(inAstNodeType);
        SLANG_UNUSED(inAstBuilder);
#endif
    }
    DeclRefBase* Decl::getDefaultDeclRef()
    {
        if (auto astBuilder = getCurrentASTBuilder())
        {
            const Index currentEpoch = astBuilder->getEpoch();
            if (currentEpoch != m_defaultDeclRefEpoch || !m_defaultDeclRef)
            {
                m_defaultDeclRef = astBuilder->getOrCreate<DirectDeclRef>(this);
                m_defaultDeclRefEpoch = currentEpoch;
            }
        }
        return m_defaultDeclRef;
    }

    bool Decl::isChildOf(Decl* other) const
    {
        for (auto parent = parentDecl; parent; parent = parent->parentDecl)
            if (parent == other)
                return true;
        return false;
    }

}
