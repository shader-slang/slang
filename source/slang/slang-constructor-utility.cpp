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

    bool allParamHaveInitExpr(ConstructorDecl* ctor)
    {
        for (auto i : ctor->getParameters())
            if (!i->initExpr)
                return false;
        return true;
    }

    static inline bool _isDefaultCtor(ConstructorDecl* ctor)
    {
        // 1. default ctor must have definition
        // 2. either 2.1 or 2.2 is safisfied
        // 2.1. default ctor must have no parameters
        // 2.2. default ctor can have parameters, but all parameters have init expr (Because we won't differentiate this case from 2.)
        if (!ctor->body || (ctor->members.getCount() != 0 && !allParamHaveInitExpr(ctor)))
            return false;

        return true;
    }

    ConstructorDecl* _getDefaultCtor(StructDecl* structDecl)
    {
        for (auto ctor : structDecl->getMembersOfType<ConstructorDecl>())
        {
            if (_isDefaultCtor(ctor))
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
            m_astBuilder->getSharedASTBuilder()->getCtorName(),
            DeclRefType::create(m_astBuilder, structDecl),
            structDecl->ownedScope,
            LookupMask::Function,
            (LookupOptions)((uint8_t)LookupOptions::IgnoreInheritance | (uint8_t)LookupOptions::IgnoreBaseInterfaces | (uint8_t)LookupOptions::NoDeref));

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

        for (auto m : ctorLookupResult)
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

    FuncDecl* findDefaultInitListFunc(StructDecl* structDecl)
    {
        for (auto funcDecl : structDecl->getMembersOfType<FuncDecl>())
        {
            if (!funcDecl->findModifier<DefaultInitModifier>())
                continue;
            return funcDecl;
        }
        return nullptr;
    }

    Expr* constructDefaultCtorInvocationExpr(SemanticsVisitor* visitor, StructDecl* structDecl, Type* structDeclType, ConstructorDecl* defaultCtor)
    {
            auto* invoke = visitor->getASTBuilder()->create<InvokeExpr>();
            auto member = visitor->getASTBuilder()->getMemberDeclRef(structDecl->getDefaultDeclRef(), defaultCtor);
            invoke->functionExpr = visitor->ConstructDeclRefExpr(member, nullptr, defaultCtor->getName(), defaultCtor->loc, nullptr);
            invoke->type = structDeclType;
            return invoke;
    }
    Expr* constructDefaultInitListFunc(SemanticsVisitor* visitor, StructDecl* structDecl, Type* structDeclType, ConstructDefaultInitListOptions options)
    {
        SLANG_ASSERT(structDecl);

        // 1. Prefer non-synth default-ctor
        //  * Skip this option if `ConstructDefaultInitListOptions::PreferDefaultInitFunc` is true
        //  * Skip this option if `ConstructDefaultInitListOptions::CheckToAvoidRecursion` detects recursion
        //      * Only user-defined ctor will try and have recursion of `{}`
        // 2. Prefer $DefaultInit
        // 3. Prefer any default-ctor
        // 4. Use `DefaultConstructExpr`

        // 1.
        auto defaultCtor = _getDefaultCtor(structDecl);
        if(defaultCtor
            && !defaultCtor->containsOption(ConstructorTags::Synthesized)
            && !((UInt)options & (UInt)ConstructDefaultInitListOptions::PreferDefaultInitFunc))
        {
            bool canCreateCtor = true;
            if(((UInt)options & (UInt)ConstructDefaultInitListOptions::CheckToAvoidRecursion))
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
                return constructDefaultCtorInvocationExpr(visitor, structDecl, structDeclType, defaultCtor);
        }

        // 2.
        if (auto defaultInitFunc = findDefaultInitListFunc(structDecl))
        {
            auto* invoke = visitor->getASTBuilder()->create<InvokeExpr>();
            DeclRef<Decl> member;
            auto declRefType = as<DeclRefType>(structDeclType);
            if(declRefType && as<GenericAppDeclRef>(declRefType->getDeclRefBase()))
                member = visitor->getASTBuilder()->getMemberDeclRef(as<GenericAppDeclRef>(declRefType->getDeclRefBase()), defaultInitFunc);
            else
                member = visitor->getASTBuilder()->getMemberDeclRef(structDecl, defaultInitFunc);

            invoke->functionExpr = visitor->ConstructDeclRefExpr(member, nullptr, defaultInitFunc->getName(), defaultInitFunc->loc, nullptr);
            invoke->type = structDeclType;
            return invoke;
        }

        // 3.
        if (defaultCtor)
            return constructDefaultCtorInvocationExpr(visitor, structDecl, structDeclType, defaultCtor);

        // 4.
        return createDefaultConstructExprForType(visitor->getASTBuilder(), QualType(structDeclType), {});
    }

    bool isCStyleStructDecl(SemanticsVisitor* visitor, StructDecl* structDecl, List<ConstructorDecl*> const& ctorList)
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
