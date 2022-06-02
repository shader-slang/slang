#ifndef SLANG_SYNTAX_H
#define SLANG_SYNTAX_H

#include "slang-ast-builder.h"

namespace Slang
{

    inline Type* getSub(ASTBuilder* astBuilder, DeclRef<GenericTypeConstraintDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->sub.Ptr());
    }

    inline Type* getSup(ASTBuilder* astBuilder, DeclRef<TypeConstraintDecl> const& declRef)
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

    struct SemanticsVisitor;

    List<ExtensionDecl*> const& getCandidateExtensions(
        DeclRef<AggTypeDecl> const& declRef,
        SemanticsVisitor*           semantics);

    inline FilteredMemberRefList<Decl> getMembers(DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle = MemberFilterStyle::All)
    {
        return FilteredMemberRefList<Decl>(declRef.getDecl()->members, declRef.substitutions, filterStyle);
    }

    template<typename T>
    inline FilteredMemberRefList<T> getMembersOfType( DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle = MemberFilterStyle::All)
    {
        return FilteredMemberRefList<T>(declRef.getDecl()->members, declRef.substitutions, filterStyle);
    }

    void _foreachDirectOrExtensionMemberOfType(
        SemanticsVisitor*               semantics,
        DeclRef<ContainerDecl> const&   declRef,
        SyntaxClassBase const&          syntaxClass,
        void                            (*callback)(DeclRefBase, void*),
        void const*                     userData);

    template<typename T, typename F>
    inline void foreachDirectOrExtensionMemberOfType(
        SemanticsVisitor*               semantics,
        DeclRef<ContainerDecl> const&   declRef,
        F const&                        func)
    {
        struct Helper
        {
            static void callback(DeclRefBase declRef, void* userData)
            {
                (*(F*)userData)(DeclRef<T>((T*) declRef.decl, declRef.substitutions));
            }
        };
        _foreachDirectOrExtensionMemberOfType(semantics, declRef, getClass<T>(), &Helper::callback, &func);
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

    inline Type* getType(ASTBuilder* astBuilder, DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline SubstExpr<Expr> getInitExpr(ASTBuilder* astBuilder, DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->initExpr);
    }

    inline Type* getType(ASTBuilder* astBuilder, DeclRef<PropertyDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline Type* getType(ASTBuilder* astBuilder, DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline SubstExpr<Expr> getTagExpr(ASTBuilder* astBuilder, DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->tagExpr);
    }

    inline Type* getTargetType(ASTBuilder* astBuilder, DeclRef<ExtensionDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->targetType.Ptr());
    }
    
    inline FilteredMemberRefList<VarDecl> getFields(DeclRef<StructDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        return getMembersOfType<VarDecl>(declRef, filterStyle);
    }

            /// If the given `structTypeDeclRef` inherits from another struct type, return that base type
    DeclRefType* findBaseStructType(ASTBuilder* astBuilder, DeclRef<StructDecl> const& structTypeDeclRef);

        /// If the given `structTypeDeclRef` inherits from another struct type, return that base struct decl
    DeclRef<StructDecl> findBaseStructDeclRef(ASTBuilder* astBuilder, DeclRef<StructDecl> const& structTypeDeclRef);

    inline Type* getTagType(ASTBuilder* astBuilder, DeclRef<EnumDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->tagType);
    }

    inline Type* getBaseType(ASTBuilder* astBuilder, DeclRef<InheritanceDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->base.type);
    }
    
    inline Type* getType(ASTBuilder* astBuilder, DeclRef<TypeDefDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->type.Ptr());
    }

    inline Type* getResultType(ASTBuilder* astBuilder, DeclRef<CallableDecl> const& declRef)
    {
        return declRef.substitute(astBuilder, declRef.getDecl()->returnType.type);
    }

    inline Type* getErrorCodeType(ASTBuilder* astBuilder, DeclRef<CallableDecl> const& declRef)
    {
        if (declRef.getDecl()->errorType.type)
        {
            return declRef.substitute(astBuilder, declRef.getDecl()->errorType.type);
        }
        else
        {
            return astBuilder->getBottomType();
        }
    }

    inline FilteredMemberRefList<ParamDecl> getParameters(DeclRef<CallableDecl> const& declRef)
    {
        return getMembersOfType<ParamDecl>(declRef);
    }

    inline Decl* getInner(DeclRef<GenericDecl> const& declRef)
    {
        // TODO: Should really return a `DeclRef<Decl>` for the inner
        // declaration, and not just a raw pointer
        return declRef.getDecl()->inner;
    }

    //

    inline Type* getType(ASTBuilder* astBuilder, SubstExpr<Expr> expr)
    {
        return substituteType(expr.getSubsts(), astBuilder, expr.getExpr()->type);
    }

    inline SubstExpr<Expr> getBaseExpr(SubstExpr<ParenExpr> expr)
    {
        return substituteExpr(expr.getSubsts(), expr.getExpr()->base);
    }

    inline SubstExpr<Expr> getBaseExpr(SubstExpr<InvokeExpr> expr)
    {
        return substituteExpr(expr.getSubsts(), expr.getExpr()->functionExpr);
    }

    inline Index getArgCount(SubstExpr<InvokeExpr> expr)
    {
        return expr.getExpr()->arguments.getCount();
    }

    inline SubstExpr<Expr> getArg(SubstExpr<InvokeExpr> expr, Index index)
    {
        return substituteExpr(expr.getSubsts(), expr.getExpr()->arguments[index]);
    }

    inline DeclRef<Decl> getDeclRef(ASTBuilder* astBuilder, SubstExpr<DeclRefExpr> expr)
    {
        return substituteDeclRef(expr.getSubsts(), astBuilder, expr.getExpr()->declRef);
    }

    //

    ArrayExpressionType* getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType,
        IntVal*         elementCount);

    ArrayExpressionType* getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType);

    NamedExpressionType* getNamedType(
        ASTBuilder*                 astBuilder,
        DeclRef<TypeDefDecl> const& declRef);

    FuncType* getFuncType(
        ASTBuilder*                     astBuilder,
        DeclRef<CallableDecl> const&    declRef);

    GenericDeclRefType* getGenericDeclRefType(
        ASTBuilder*                 astBuilder,
        DeclRef<GenericDecl> const& declRef);

    NamespaceType* getNamespaceType(
        ASTBuilder*                     astBuilder,
        DeclRef<NamespaceDeclBase> const&   declRef);

    SamplerStateType* getSamplerStateType(
        ASTBuilder*     astBuilder);


    // Definitions that can't come earlier despite
    // being in templates, because gcc/clang get angry.
    //
    template<typename T>
    void FilteredModifierList<T>::Iterator::operator++()
    {
        current = adjust(current->next);
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
            m = m->next;
        }        
    }

    //

    ThisTypeSubstitution* findThisTypeSubstitution(
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

    GenericSubstitution* createDefaultSubstitutionsForGeneric(
        ASTBuilder*             astBuilder, 
        GenericDecl*            genericDecl,
        Substitutions*   outerSubst);

    GenericSubstitution* findInnerMostGenericSubstitution(Substitutions* subst);

    enum class UserDefinedAttributeTargets
    {
        None = 0,
        Struct = 1,
        Var = 2,
        Function = 4,
        All = 7
    };

        /// Get the module dclaration that a declaration is associated with, if any.
    ModuleDecl* getModuleDecl(Decl* decl);

    /// Get the module that a declaration is associated with, if any.
    Module* getModule(Decl* decl);

   

} // namespace Slang

#endif
