#ifndef SLANG_SYNTAX_H
#define SLANG_SYNTAX_H

#include "slang-ast-support-types.h"

#include "slang-ast-all.h"

namespace Slang
{

    inline RefPtr<Type> GetSub(DeclRef<GenericTypeConstraintDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->sub.Ptr());
    }

    inline RefPtr<Type> GetSup(DeclRef<TypeConstraintDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->getSup().type);
    }

    // Note(tfoley): These logically belong to `Type`,
    // but order-of-declaration stuff makes that tricky
    //
    // TODO(tfoley): These should really belong to the compilation context!
    //
    void registerBuiltinDecl(
        Session*                    session,
        RefPtr<Decl>                decl,
        RefPtr<BuiltinTypeModifier> modifier);
    void registerMagicDecl(
        Session*                    session,
        RefPtr<Decl>                decl,
        RefPtr<MagicTypeModifier>   modifier);

    // Look up a magic declaration by its name
    RefPtr<Decl> findMagicDecl(
        Session*        session,
        String const&   name);

    // Create an instance of a syntax class by name
    SyntaxNodeBase* createInstanceOfSyntaxClassByName(
        String const&   name);

    // `Val`

    inline bool areValsEqual(Val* left, Val* right)
    {
        if(!left || !right) return left == right;
        return left->equalsVal(right);
    }

    //

    inline BaseType GetVectorBaseType(VectorExpressionType* vecType)
    {
        auto basicExprType = as<BasicExpressionType>(vecType->elementType);
        return basicExprType->baseType;
    }

    inline int GetVectorSize(VectorExpressionType* vecType)
    {
        auto constantVal = as<ConstantIntVal>(vecType->elementCount);
        if (constantVal)
            return (int) constantVal->value;
        // TODO: what to do in this case?
        return 0;
    }

    //
    // Declarations
    //

    inline ExtensionDecl* GetCandidateExtensions(DeclRef<AggTypeDecl> const& declRef)
    {
        return declRef.getDecl()->candidateExtensions;
    }

    inline FilteredMemberRefList<Decl> getMembers(DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle = MemberFilterStyle::All)
    {
        return FilteredMemberRefList<Decl>(declRef.getDecl()->members, declRef.substitutions, filterStyle);
    }

    template<typename T>
    inline FilteredMemberRefList<T> getMembersOfType( DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle = MemberFilterStyle::All)
    {
        return FilteredMemberRefList<T>(declRef.getDecl()->members, declRef.substitutions, filterStyle);
    }

    template<typename T>
    inline List<DeclRef<T>> getMembersOfTypeWithExt(DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle = MemberFilterStyle::All)
    {
        List<DeclRef<T>> rs;
        for (auto d : getMembersOfType<T>(declRef, filterStyle))
            rs.add(d);
        if (auto aggDeclRef = declRef.as<AggTypeDecl>())
        {
            for (auto ext = GetCandidateExtensions(aggDeclRef); ext; ext = ext->nextCandidateExtension)
            {
                auto extMembers = getMembersOfType<T>(DeclRef<ContainerDecl>(ext, declRef.substitutions), filterStyle);
                for (auto mbr : extMembers)
                    rs.add(mbr);
            }
        }
        return rs;
    }

        /// The the user-level name for a variable that might be a shader parameter.
        ///
        /// In most cases this is just the name of the variable declaration itself,
        /// but in the specific case of a `cbuffer`, the name that the user thinks
        /// of is really metadata. For example:
        ///
        ///     cbuffer C { int x; }
        ///
        /// In this example, error messages relating to the constant buffer should
        /// really use the name `C`, but that isn't the name of the declaration
        /// (it is in practice anonymous, and `C` can be used for a different
        /// declaration in the same file).
        ///
    Name* getReflectionName(VarDeclBase* varDecl);

    inline RefPtr<Type> GetType(DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Expr> getInitExpr(DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->initExpr);
    }

    inline RefPtr<Type> getType(DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Expr> getTagExpr(DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->tagExpr);
    }

    inline RefPtr<Type> GetTargetType(DeclRef<ExtensionDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->targetType.Ptr());
    }
    
    inline FilteredMemberRefList<VarDecl> GetFields(DeclRef<StructDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        return getMembersOfType<VarDecl>(declRef, filterStyle);
    }

    

    inline RefPtr<Type> getBaseType(DeclRef<InheritanceDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->base.type);
    }
    
    inline RefPtr<Type> GetType(DeclRef<TypeDefDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Type> GetResultType(DeclRef<CallableDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->returnType.type.Ptr());
    }

    inline FilteredMemberRefList<ParamDecl> GetParameters(DeclRef<CallableDecl> const& declRef)
    {
        return getMembersOfType<ParamDecl>(declRef);
    }

    inline Decl* GetInner(DeclRef<GenericDecl> const& declRef)
    {
        // TODO: Should really return a `DeclRef<Decl>` for the inner
        // declaration, and not just a raw pointer
        return declRef.getDecl()->inner.Ptr();
    }


    //

    RefPtr<ArrayExpressionType> getArrayType(
        Type* elementType,
        IntVal*         elementCount);

    RefPtr<ArrayExpressionType> getArrayType(
        Type* elementType);

    RefPtr<NamedExpressionType> getNamedType(
        Session*                    session,
        DeclRef<TypeDefDecl> const& declRef);

    RefPtr<TypeType> getTypeType(
        Type* type);

    RefPtr<FuncType> getFuncType(
        Session*                        session,
        DeclRef<CallableDecl> const&    declRef);

    RefPtr<GenericDeclRefType> getGenericDeclRefType(
        Session*                    session,
        DeclRef<GenericDecl> const& declRef);

    RefPtr<NamespaceType> getNamespaceType(
        Session*                            session,
        DeclRef<NamespaceDeclBase> const&   declRef);

    RefPtr<SamplerStateType> getSamplerStateType(
        Session*        session);


    // Definitions that can't come earlier despite
    // being in templates, because gcc/clang get angry.
    //
    template<typename T>
    void FilteredModifierList<T>::Iterator::operator++()
    {
        current = Adjust(current->next.Ptr());
    }
    //
    template<typename T>
    Modifier* FilteredModifierList<T>::Adjust(Modifier* modifier)
    {
        Modifier* m = modifier;
        for (;;)
        {
            if (!m) return m;
            if (as<T>(m))
            {
                return m;
            }
            m = m->next.Ptr();
        }        
    }

    // TODO: where should this live?
    SubstitutionSet createDefaultSubstitutions(
        Session*        session,
        Decl*           decl,
        SubstitutionSet  parentSubst);

    SubstitutionSet createDefaultSubstitutions(
        Session* session,
        Decl*   decl);

    DeclRef<Decl> createDefaultSubstitutionsIfNeeded(
        Session*        session,
        DeclRef<Decl>   declRef);

    RefPtr<GenericSubstitution> createDefaultSubsitutionsForGeneric(
        Session*                session,
        GenericDecl*            genericDecl,
        RefPtr<Substitutions>   outerSubst);

    RefPtr<GenericSubstitution> findInnerMostGenericSubstitution(Substitutions* subst);

    enum class UserDefinedAttributeTargets
    {
        None = 0,
        Struct = 1,
        Var = 2,
        Function = 4,
        All = 7
    };

        /// Get the module that a declaration is associated with, if any.
    Module* getModule(Decl* decl);

   

} // namespace Slang

#endif
