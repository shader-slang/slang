// slang-ast-clone.h
#pragma once

#include "slang-ast-all.h"

#include <type_traits>

namespace Slang
{

struct SemanticsVisitor;

struct ASTCloneContext
{
    ASTBuilder* astBuilder = nullptr;
    SemanticsVisitor* semantics = nullptr;

    // Source of truth for hygienic cloning: references to these declarations are rewritten to the
    // cloned declarations.
    Dictionary<Decl*, Decl*> oldToNewDecls;

    // Performance cache for semantic values rebuilt after applying `oldToNewDecls`.
    Dictionary<Val*, Val*> oldToNewVals;

    // Witness tables are derived semantic data tied to the original declaration graph. Raw
    // ASTCloner users fail loudly if they encounter populated witness tables;
    // GenericSignatureCloner opts in because its callers rebuild generic constraint path tables
    // after cloning.
    bool allowDroppingWitnessTables = false;
};

// Returns `genericDeclToSpecialize` applied to the default arguments from
// `genericDeclProvidingSpecializationArgs`.
//
// For example, after cloning `generic<T>` to `generic<T2>`, this returns a decl-ref for the
// source generic specialized as `generic<T2>`. Callers use that substitution to rewrite references
// from the original generic environment into the standalone cloned environment.
DeclRef<Decl> getSpecializedDeclRefWithParamsFromGeneric(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    GenericDecl* genericDeclToSpecialize,
    GenericDecl* genericDeclProvidingSpecializationArgs);

// Clones AST syntax nodes while rewriting semantic references through `oldToNewDecls`.
//
// Callers pre-map declaration binders that are being cloned into a new scope. The cloner then
// rewrites DeclRef/Val/Type fields that mention those binders, preserves unmapped raw Decl*
// references as external references, and leaves container membership to the caller. Derived witness
// tables are out of contract for ordinary ASTCloner use because they are tied to the checked source
// declaration graph.
class ASTCloner
{
public:
    ASTCloner(ASTBuilder* astBuilder, SemanticsVisitor* semantics = nullptr);

    ASTCloneContext& getContext() { return m_context; }
    ASTCloneContext const& getContext() const { return m_context; }

    void mapDecl(Decl* oldDecl, Decl* newDecl);

    Decl* cloneDecl(Decl* decl);
    Expr* cloneExpr(Expr* expr);
    Stmt* cloneStmt(Stmt* stmt);
    Modifier* cloneModifier(Modifier* modifier);

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
    TypeExp cloneTypeExp(TypeExp const& typeExp);

    void cloneField(QualType& dst, QualType const& src);
    void cloneField(TypeExp& dst, TypeExp const& src);
    void cloneField(Modifiers& dst, Modifiers const& src);
    void cloneField(ValNodeOperand& dst, ValNodeOperand const& src);
    void cloneField(RefPtr<WitnessTable>& dst, RefPtr<WitnessTable> const& src);
    void cloneField(ContainerDeclDirectMemberDecls& dst, ContainerDeclDirectMemberDecls const& src);

    template<typename T>
    void cloneField(DeclRef<T>& dst, DeclRef<T> const& src)
    {
        dst = rewriteDeclRef(src);
    }

    template<typename T>
    void cloneField(List<T>& dst, List<T> const& src)
    {
        dst.clear();
        for (auto const& item : src)
        {
            T clonedItem{};
            cloneField(clonedItem, item);
            dst.add(clonedItem);
        }
    }

    template<typename T, int N>
    void cloneField(T (&dst)[N], T const (&src)[N])
    {
        for (int i = 0; i < N; ++i)
            cloneField(dst[i], src[i]);
    }

    template<typename T>
    void cloneField(T*& dst, T* src)
    {
        if constexpr (std::is_base_of_v<Expr, T>)
        {
            dst = as<T>(cloneExpr(src));
        }
        else if constexpr (std::is_base_of_v<Stmt, T>)
        {
            dst = as<T>(cloneStmt(src));
        }
        else if constexpr (std::is_base_of_v<Modifier, T>)
        {
            dst = as<T>(cloneModifier(src));
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
                // present in `oldToNewDecls` are part of the cloned generic environment; all other
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
    std::enable_if_t<!std::is_array_v<T>, void> cloneField(T& dst, T const& src)
    {
        dst = src;
    }

private:
    NodeBase* cloneSyntaxNode(NodeBase* node);
    void cloneNodeFields(NodeBase* dst, NodeBase* src);
    Decl* rewriteDecl(Decl* decl);
    Val* rewriteValImpl(Val* val);

    ASTCloneContext m_context;
};

// Clones the parameter and constraint members that define a generic declaration's signature.
//
// The destination generic owns the cloned members. `oldToNewDecls` maps source binders and
// proof-bearing constraint declarations to those clones so later constraints and default
// substitution arguments can reference the cloned environment. Witness/path-resolution tables are
// dropped during field cloning; callers rebuild generic constraint path tables after all cloned
// constraints are present.
class GenericSignatureCloner
{
public:
    GenericSignatureCloner(
        ASTBuilder* astBuilder,
        SemanticsVisitor* semantics,
        GenericDecl* sourceGenericDecl,
        GenericDecl* destGenericDecl,
        List<Expr*>* outGenericArgs = nullptr);

    ASTCloner& getASTCloner() { return m_astCloner; }

    void cloneParameterMembers();
    Decl* cloneConstraintMember(Decl* member, SubstitutionSet const& sourceToDestSubstitution);

private:
    Expr* createGenericArgExpr(Decl* decl);

    ASTCloner m_astCloner;
    GenericDecl* m_sourceGenericDecl = nullptr;
    GenericDecl* m_destGenericDecl = nullptr;
    List<Expr*>* m_outGenericArgs = nullptr;
};

} // namespace Slang
