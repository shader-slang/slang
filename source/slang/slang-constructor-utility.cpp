// slang-constructor-utility.cpp
#include "slang-check-impl.h"

namespace Slang
{
    ConstructorDecl* _getDefaultCtor(StructDecl* structDecl)
    {
        for (auto ctor : structDecl->getMembersOfType<ConstructorDecl>())
        {
            if (!ctor->body || ctor->members.getCount() != 0)
                continue;
            return ctor;
        }
        return nullptr;
    }

    List<ConstructorDecl*> _getCtorList(ASTBuilder* m_astBuilder, SemanticsVisitor* visitor, StructDecl* structDecl, ConstructorDecl** defaultCtorOut)
    {
        List<ConstructorDecl*> ctorList;

        auto ctorLookupResult = lookUpMember(
            m_astBuilder,
            visitor,
            visitor->getName("$init"),
            DeclRefType::create(m_astBuilder, structDecl),
            structDecl->ownedScope,
            LookupMask::Function,
            (LookupOptions)((Index)LookupOptions::IgnoreInheritance | (Index)LookupOptions::IgnoreBaseInterfaces | (Index)LookupOptions::NoDeref));

        if (!ctorLookupResult.isValid())
            return ctorList;

        auto lookupResultHandle = [&](LookupResultItem& item)
            {
                auto ctor = as<ConstructorDecl>(item.declRef.getDecl());
                if (!ctor)
                    return;
                ctorList.add(ctor);
                if (ctor->members.getCount() != 0 || !defaultCtorOut)
                    return;
                *defaultCtorOut = ctor;
            };
        if (ctorLookupResult.items.getCount() == 0)
        {
            lookupResultHandle(ctorLookupResult.item);
            return ctorList;
        }

        for (auto m : ctorLookupResult.items)
        {
            lookupResultHandle(m);
        }

        return ctorList;
    }

    Expr* constructDefaultInitExprForVar(SemanticsVisitor* visitor, TypeExp varDeclType)
    {
        if (!varDeclType || !varDeclType.type)
            return nullptr;

        ConstructorDecl* defaultCtor = nullptr;
        auto declRefType = as<DeclRefType>(varDeclType.type);
        if (declRefType)
        {
            if (auto structDecl = as<StructDecl>(declRefType->getDeclRef().getDecl()))
            {
                defaultCtor = _getDefaultCtor(structDecl);
            }
        }

        if (defaultCtor)
        {
            auto* invoke = visitor->getASTBuilder()->create<InvokeExpr>();
            auto member = visitor->getASTBuilder()->getMemberDeclRef(declRefType->getDeclRef(), defaultCtor);
            invoke->functionExpr = visitor->ConstructDeclRefExpr(member, nullptr, defaultCtor->loc, nullptr);
            invoke->type = varDeclType.type;
            return invoke;
        }
        else
        {
            auto* defaultCall = visitor->getASTBuilder()->create<DefaultConstructExpr>();
            defaultCall->type = QualType(varDeclType.type);
            return defaultCall;
        }
    }
}

