// slang-constructor-utility.cpp
#include "slang-check-impl.h"

namespace Slang
{

    DefaultConstructExpr* createDefaultConstructExprForType(ASTBuilder* m_astBuilder, QualType type, SourceLoc loc)
    {
        auto defaultConstructExpr = m_astBuilder->create<DefaultConstructExpr>();
        defaultConstructExpr->type = type;
        defaultConstructExpr->loc = loc;
        return defaultConstructExpr;
    }

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
    bool allParamHaveInitExpr(ConstructorDecl* ctor)
    {
        for (auto i : ctor->getParameters())
            if (!i->initExpr)
                return false;
        return true;
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
                if (ctor->members.getCount() != 0
                    && !allParamHaveInitExpr(ctor)
                    || !defaultCtorOut)
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
            invoke->functionExpr = visitor->ConstructDeclRefExpr(member, nullptr, defaultCtor->getName(), defaultCtor->loc, nullptr);
            invoke->type = varDeclType.type;
            return invoke;
        }
        else
        {
            return createDefaultConstructExprForType(visitor->getASTBuilder(), QualType(varDeclType.type), {});
        }
    }

    FuncDecl* findZeroInitListFunc(StructDecl* structDecl)
    {
        for (auto funcDecl : structDecl->getMembersOfType<FuncDecl>())
        {
            if (!funcDecl->findModifier<ZeroInitModifier>())
                continue;
            return funcDecl;
        }
        return nullptr;
    }

    Expr* _constructZeroInitListFuncMakeDefaultCtor(SemanticsVisitor* visitor, StructDecl* structDecl, Type* structDeclType, ConstructorDecl* defaultCtor)
    {
            auto* invoke = visitor->getASTBuilder()->create<InvokeExpr>();
            auto member = visitor->getASTBuilder()->getMemberDeclRef(structDecl->getDefaultDeclRef(), defaultCtor);
            invoke->functionExpr = visitor->ConstructDeclRefExpr(member, nullptr, defaultCtor->getName(), defaultCtor->loc, nullptr);
            invoke->type = structDeclType;
            return invoke;
    }
    Expr* constructZeroInitListFunc(SemanticsVisitor* visitor, StructDecl* structDecl, Type* structDeclType, ConstructZeroInitListOptions options)
    {
        SLANG_ASSERT(structDecl);

        // 1. Prefer non-synth default-ctor
        //  * Skip this option if `ConstructZeroInitListOptions::PreferZeroInitFunc` is true
        //  * Skip this option if `ConstructZeroInitListOptions::CheckToAvoidRecursion` detects recursion
        //      * Only user-defined ctor will try and have recursion of `{}`
        // 2. Prefer $ZeroInit
        // 3. Prefer any default-ctor
        // 4. Use `DefaultConstructExpr`

        // 1.
        auto defaultCtor = _getDefaultCtor(structDecl);
        if(defaultCtor
            && !defaultCtor->containsOption(ConstructorTags::Synthesized)
            && !((UInt)options & (UInt)ConstructZeroInitListOptions::PreferZeroInitFunc))
        {
            bool canCreateCtor = true;
            if(((UInt)options & (UInt)ConstructZeroInitListOptions::CheckToAvoidRecursion))
            {
                auto callingScope = visitor->getOuterScope();
                while (callingScope)
                {
                    if (callingScope->containerDecl == defaultCtor)
                    {
                        canCreateCtor = false;
                        break;
                    }
                    callingScope = callingScope->parent;
                }
            }
            if(canCreateCtor)
                return _constructZeroInitListFuncMakeDefaultCtor(visitor, structDecl, structDeclType, defaultCtor);
        }

        // 2.
        if (auto zeroInitListFunc = findZeroInitListFunc(structDecl))
        {
            auto* invoke = visitor->getASTBuilder()->create<InvokeExpr>();
            DeclRef<Decl> member;
            auto declRefType = as<DeclRefType>(structDeclType);
            if(declRefType && as<GenericAppDeclRef>(declRefType->getDeclRefBase()))
                member = visitor->getASTBuilder()->getMemberDeclRef(as<GenericAppDeclRef>(declRefType->getDeclRefBase()), zeroInitListFunc);
            else
                member = visitor->getASTBuilder()->getMemberDeclRef(structDecl, zeroInitListFunc);

            invoke->functionExpr = visitor->ConstructDeclRefExpr(member, nullptr, zeroInitListFunc->getName(), zeroInitListFunc->loc, nullptr);
            invoke->type = structDeclType;
            return invoke;
        }

        // 3.
        if (defaultCtor)
            return _constructZeroInitListFuncMakeDefaultCtor(visitor, structDecl, structDeclType, defaultCtor);

        // 4.
        return createDefaultConstructExprForType(visitor->getASTBuilder(), QualType(structDeclType), {});
    }

    bool checkIfCStyleStruct(SemanticsVisitor* visitor, StructDecl* structDecl)
    {
        // CStyleStruct follows the following rules:
        // 1. Does not contain a non 'Synthesized' Ctor (excluding 'DefaultCtor')
        //
        // 2. Only contains 1 'non-default' ctor regardless of synthisis or not, else 
        //    `__init(int, int)` and `__init(int)` would have ambiguity for 
        //    c-style-initialization of `MyStruct[3] tmp = {1,2, 1,2, 1,2};`
        // 
        // 3. Every `VarDeclBase*` member has the same visibility

        auto isCStyleStruct = visitor->getShared()->tryGetIsCStyleStructFromCache(structDecl);

        if (isCStyleStruct)
            return *isCStyleStruct;

        // Add to IsCStyleStruct cache
        auto ctorList = _getCtorList(visitor->getASTBuilder(), visitor, structDecl, nullptr);
        int nonDefaultInitCount = 0;
        for (auto i : ctorList)
        {
            // Default ctor is always fine
            if (i->getParameters().getCount() == 0)
                continue;

            // Cannot contain user defined ctor which is a non default ctor
            if (!i->containsOption(ConstructorTags::Synthesized))
            {
                visitor->getShared()->cacheIsCStyleStruct(structDecl, false);
                return false;
            }

            // Cannot contain 2+ non-default init's
            nonDefaultInitCount++;
            if (nonDefaultInitCount > 1)
            {
                visitor->getShared()->cacheIsCStyleStruct(structDecl, false);
                return false;
            }
        }
        return true;
    }
}
