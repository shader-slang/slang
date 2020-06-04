#ifndef SLANG_SYNTAX_H
#define SLANG_SYNTAX_H

#include "slang-ast-builder.h"

namespace Slang
{

    inline RefPtr<Type> getSub(ASTBuilder* astBuilder, DeclRef<GenericTypeConstraintDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->sub.Ptr());
    }

    inline RefPtr<Type> getSup(ASTBuilder* astBuilder, DeclRef<TypeConstraintDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->getSup().type);
    }


    // `Val`

    inline bool areValsEqual(Val* left, Val* right)
    {
        if(!left || !right) return left == right;
        return left->equalsVal(right);
    }

    //

    inline BaseType getVectorBaseType(VectorExpressionType* vecType)
    {
        auto basicExprType = as<BasicExpressionType>(vecType->elementType);
        return basicExprType->baseType;
    }

    inline int getVectorSize(VectorExpressionType* vecType)
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

    inline ExtensionDecl* getCandidateExtensions(DeclRef<AggTypeDecl> const& declRef)
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
            for (auto ext = getCandidateExtensions(aggDeclRef); ext; ext = ext->nextCandidateExtension)
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

    inline RefPtr<Type> getType(ASTBuilder* astBuilder, DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Expr> getInitExpr(ASTBuilder* astBuilder, DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->initExpr);
    }

    inline RefPtr<Type> getType(ASTBuilder* astBuilder, DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Expr> getTagExpr(ASTBuilder* astBuilder, DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->tagExpr);
    }

    inline RefPtr<Type> getTargetType(ASTBuilder* astBuilder, DeclRef<ExtensionDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->targetType.Ptr());
    }
    
    inline FilteredMemberRefList<VarDecl> getFields(DeclRef<StructDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        return getMembersOfType<VarDecl>(declRef, filterStyle);
    }

    

    inline RefPtr<Type> getBaseType(ASTBuilder* astBuilder, DeclRef<InheritanceDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->base.type);
    }
    
    inline RefPtr<Type> getType(ASTBuilder* astBuilder, DeclRef<TypeDefDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Type> getResultType(ASTBuilder* astBuilder, DeclRef<CallableDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->returnType.type.Ptr());
    }

    inline FilteredMemberRefList<ParamDecl> getParameters(DeclRef<CallableDecl> const& declRef)
    {
        return getMembersOfType<ParamDecl>(declRef);
    }

    inline Decl* getInner(DeclRef<GenericDecl> const& declRef)
    {
        // TODO: Should really return a `DeclRef<Decl>` for the inner
        // declaration, and not just a raw pointer
        return declRef.getDecl()->inner.Ptr();
    }


    //

    RefPtr<ArrayExpressionType> getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType,
        IntVal*         elementCount);

    RefPtr<ArrayExpressionType> getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType);

    RefPtr<NamedExpressionType> getNamedType(
        ASTBuilder*                 astBuilder,
        DeclRef<TypeDefDecl> const& declRef);

    RefPtr<FuncType> getFuncType(
        ASTBuilder*                     astBuilder,
        DeclRef<CallableDecl> const&    declRef);

    RefPtr<GenericDeclRefType> getGenericDeclRefType(
        ASTBuilder*                 astBuilder,
        DeclRef<GenericDecl> const& declRef);

    RefPtr<NamespaceType> getNamespaceType(
        ASTBuilder*                     astBuilder,
        DeclRef<NamespaceDeclBase> const&   declRef);

    RefPtr<SamplerStateType> getSamplerStateType(
        ASTBuilder*     astBuilder);


    // Definitions that can't come earlier despite
    // being in templates, because gcc/clang get angry.
    //
    template<typename T>
    void FilteredModifierList<T>::Iterator::operator++()
    {
        current = adjust(current->next.Ptr());
    }
    //
    template<typename T>
    Modifier* FilteredModifierList<T>::adjust(Modifier* modifier)
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

    //

    RefPtr<ThisTypeSubstitution> findThisTypeSubstitution(
        Substitutions*  substs,
        InterfaceDecl*  interfaceDecl);

    RequirementWitness tryLookUpRequirementWitness(
        ASTBuilder*     astBuilder,
        SubtypeWitness* subtypeWitness,
        Decl*           requirementKey);

    // TODO: where should this live?
    SubstitutionSet createDefaultSubstitutions(
        ASTBuilder*     astBuilder, 
        Decl*           decl,
        SubstitutionSet  parentSubst);

    SubstitutionSet createDefaultSubstitutions(
        ASTBuilder*     astBuilder, 
        Decl*   decl);

    DeclRef<Decl> createDefaultSubstitutionsIfNeeded(
        ASTBuilder*     astBuilder, 
        DeclRef<Decl>   declRef);

    RefPtr<GenericSubstitution> createDefaultSubstitutionsForGeneric(
        ASTBuilder*             astBuilder, 
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
