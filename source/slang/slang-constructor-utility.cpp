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

    bool DiagnoseIsAllowedInitExpr(VarDeclBase* varDecl, DiagnosticSink* sink)
    {
        if (!varDecl)
            return true;

        // find groupshared modifier
        if (varDecl->findModifier<HLSLGroupSharedModifier>())
        {
            if (sink && varDecl->initExpr)
                sink->diagnose(varDecl, Diagnostics::cannotHaveInitializer, varDecl, "groupshared");
            return false;
        }

        return true;
    }

    bool isDefaultInitializable(Type* varDeclType, VarDeclBase* associatedDecl)
    {
        if (!DiagnoseIsAllowedInitExpr(associatedDecl, nullptr))
            return false;

        // Find struct and modifiers associated with varDecl
        StructDecl* structDecl = nullptr;
        if (auto declRefType = as<DeclRefType>(varDeclType))
        {
            if (auto genericAppRefDecl = as<GenericAppDeclRef>(declRefType->getDeclRefBase()))
            {
                auto baseGenericRefType = genericAppRefDecl->getBase()->getDecl();
                if (auto baseTypeStruct = as<StructDecl>(baseGenericRefType))
                {
                    structDecl = baseTypeStruct;
                }
                else if (auto genericDecl = as<GenericDecl>(baseGenericRefType))
                {
                    if (auto innerTypeStruct = as<StructDecl>(genericDecl->inner))
                        structDecl = innerTypeStruct;
                }
            }
            else
            {
                structDecl = as<StructDecl>(declRefType->getDeclRef().getDecl());
            }
        }
        if (structDecl)
        {
            // find if a type is non-copyable
            if (structDecl->findModifier<NonCopyableTypeAttribute>())
                return false;
        }

        return true;
    }

    Expr* constructDefaultInitExprForVar(SemanticsVisitor* visitor, TypeExp varDeclType, VarDeclBase* decl)
    {
        if (!varDeclType || !varDeclType.type)
            return nullptr;

        if (!isDefaultInitializable(varDeclType.type, decl))
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

