// slang-ast-copy.h
#pragma once

#include "slang-ast-all.h"

#include <type_traits>

namespace Slang
{

struct SemanticsVisitor;

struct ASTCopyContext
{
    ASTBuilder* astBuilder = nullptr;
    SemanticsVisitor* semantics = nullptr;

    // Source of truth for hygienic copying: references to these declarations are rewritten to the
    // copied declarations.
    Dictionary<Decl*, Decl*> oldToNewDecls;

    // Performance cache for semantic values rebuilt after applying `oldToNewDecls`.
    Dictionary<Val*, Val*> oldToNewVals;

    // Witness tables are derived semantic data tied to the original declaration graph. Raw
    // ASTCopier users fail loudly if they encounter populated witness tables;
    // GenericSignatureCopier opts in because its callers rebuild generic constraint path tables
    // after copying.
    bool allowDroppingWitnessTables = false;
};

// Returns `sourceGenericDecl` applied to the copied generic's default arguments.
//
// For example, after copying `generic<T>` to `generic<T2>`, this returns a decl-ref for the
// source generic specialized as `generic<T2>`. Callers use that substitution to rewrite references
// from the original generic environment into the standalone copied environment.
DeclRef<Decl> getSourceDeclRefWithCopiedGenericArgs(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    GenericDecl* sourceGenericDecl,
    GenericDecl* copiedGenericDecl);

// Copies AST syntax nodes while rewriting semantic references through `oldToNewDecls`.
//
// Callers pre-map declaration binders that are being copied into a new scope. The copier then
// rewrites DeclRef/Val/Type fields that mention those binders, preserves unmapped raw Decl*
// references as external references, and leaves container membership to the caller. Derived witness
// tables are out of contract for ordinary ASTCopier use because they are tied to the checked source
// declaration graph.
class ASTCopier
{
public:
    ASTCopier(ASTBuilder* astBuilder, SemanticsVisitor* semantics = nullptr);

    ASTCopyContext& getContext() { return m_context; }
    ASTCopyContext const& getContext() const { return m_context; }

    void mapDecl(Decl* oldDecl, Decl* newDecl);

    Decl* copyDecl(Decl* decl);
    Expr* copyExpr(Expr* expr);
    Stmt* copyStmt(Stmt* stmt);
    Modifier* copyModifier(Modifier* modifier);

    Val* rewriteVal(Val* val);
    Type* rewriteType(Type* type) { return as<Type>(rewriteVal(type)); }
    IntVal* rewriteIntVal(IntVal* val) { return as<IntVal>(rewriteVal(val)); }
    DeclRefBase* rewriteDeclRef(DeclRefBase* declRef)
    {
        return as<DeclRefBase>(rewriteVal(declRef));
    }

    template<typename T>
    DeclRef<T> rewriteDeclRef(DeclRef<T> declRef)
    {
        return DeclRef<T>(rewriteDeclRef(declRef.declRefBase));
    }

    QualType rewriteQualType(QualType const& type);
    TypeExp copyTypeExp(TypeExp const& typeExp);

    void copyField(QualType& dst, QualType const& src);
    void copyField(TypeExp& dst, TypeExp const& src);
    void copyField(Modifiers& dst, Modifiers const& src);
    void copyField(ValNodeOperand& dst, ValNodeOperand const& src);
    void copyField(RefPtr<WitnessTable>& dst, RefPtr<WitnessTable> const& src);
    void copyField(ContainerDeclDirectMemberDecls& dst, ContainerDeclDirectMemberDecls const& src);

    template<typename T>
    void copyField(DeclRef<T>& dst, DeclRef<T> const& src)
    {
        dst = rewriteDeclRef(src);
    }

    template<typename T>
    void copyField(List<T>& dst, List<T> const& src)
    {
        dst.clear();
        for (auto const& item : src)
        {
            T copiedItem{};
            copyField(copiedItem, item);
            dst.add(copiedItem);
        }
    }

    template<typename T, int N>
    void copyField(T (&dst)[N], T const (&src)[N])
    {
        for (int i = 0; i < N; ++i)
            copyField(dst[i], src[i]);
    }

    template<typename T>
    void copyField(T*& dst, T* src)
    {
        if constexpr (std::is_base_of_v<Expr, T>)
        {
            dst = as<T>(copyExpr(src));
        }
        else if constexpr (std::is_base_of_v<Stmt, T>)
        {
            dst = as<T>(copyStmt(src));
        }
        else if constexpr (std::is_base_of_v<Modifier, T>)
        {
            dst = as<T>(copyModifier(src));
        }
        else if constexpr (std::is_base_of_v<Val, T>)
        {
            dst = as<T>(rewriteVal(src));
        }
        else if constexpr (std::is_base_of_v<Decl, T>)
        {
            if (!src)
            {
                dst = nullptr;
            }
            else if (auto newDecl = m_context.oldToNewDecls.tryGetValue(src))
            {
                dst = as<T>(*newDecl);
            }
            else
            {
                // Fiddle-reflected raw Decl* fields include external semantic references and lookup
                // accelerator links such as `_prevInContainerWithSameName`. Only declarations
                // present in `oldToNewDecls` are part of the copied generic environment; all other
                // raw declaration pointers remain external references.
                dst = src;
            }
        }
        else
        {
            dst = src;
        }
    }

    template<typename T>
    std::enable_if_t<!std::is_array_v<T>, void> copyField(T& dst, T const& src)
    {
        dst = src;
    }

private:
    NodeBase* copySyntaxNode(NodeBase* node);
    void copyNodeFields(NodeBase* dst, NodeBase* src);
    Decl* rewriteDecl(Decl* decl);
    Val* rewriteValImpl(Val* val);

    ASTCopyContext m_context;
};

// Copies the parameter and constraint members that define a generic declaration's signature.
//
// The destination generic owns the copied members. `oldToNewDecls` maps source binders and
// proof-bearing constraint declarations to those copies so later constraints and default
// substitution arguments can reference the copied environment. Witness/path-resolution tables are
// dropped during the field copy; callers rebuild generic constraint path tables after all copied
// constraints are present.
class GenericSignatureCopier
{
public:
    GenericSignatureCopier(
        ASTBuilder* astBuilder,
        SemanticsVisitor* semantics,
        GenericDecl* sourceGenericDecl,
        GenericDecl* destGenericDecl,
        List<Expr*>* outGenericArgs = nullptr);

    ASTCopier& getASTCopier() { return m_astCopier; }

    void copyParameterMembers();
    Decl* copyConstraintMember(Decl* member, SubstitutionSet const& sourceToDestSubstitution);

private:
    Expr* createGenericArgExpr(Decl* decl);

    ASTCopier m_astCopier;
    GenericDecl* m_sourceGenericDecl = nullptr;
    GenericDecl* m_destGenericDecl = nullptr;
    List<Expr*>* m_outGenericArgs = nullptr;
};

} // namespace Slang
