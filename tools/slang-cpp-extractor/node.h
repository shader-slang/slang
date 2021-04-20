#ifndef CPP_EXTRACT_NODE_H
#define CPP_EXTRACT_NODE_H

#include "diagnostics.h"

namespace CppExtract {
using namespace Slang;

enum class ReflectionType : uint8_t
{
    NotReflected,
    Reflected,
};

// Pre-declare
class TypeSet;
class SourceOrigin;

struct ScopeNode;

class Node : public RefObject
{
public:
    enum class Type : uint8_t
    {
        Invalid,

        StructType,
        ClassType,

        Enum,               
        EnumClass,

        Namespace,
        AnonymousNamespace,

        Field,
        EnumCase,

        TypeDef,

        CountOf,
    };

    enum class TypeRange
    {
        ScopeStart = int(Type::StructType),
        ScopeEnd = int(Type::AnonymousNamespace),

        ClassLikeStart = int(Type::StructType),
        ClassLikeEnd = int(Type::ClassType),

        EnumStart = int(Type::Enum),
        EnumEnd = int(Type::EnumClass),
    };

    static bool isScopeType(Type type) { return int(type) >= int(TypeRange::ScopeStart) && int(type) <= int(TypeRange::ScopeEnd); }
    static bool isClassLikeType(Type type) { return int(type) >= int(TypeRange::ClassLikeStart) &&  int(type) <= int(TypeRange::ClassLikeEnd); }
    static bool isEnumLikeType(Type type) { return int(type) >= int(TypeRange::EnumStart) &&  int(type) <= int(TypeRange::EnumEnd); }
    static bool canAcceptTypes(Type type)
    {
        switch (type)
        {
            case Type::StructType:
            case Type::ClassType:
            case Type::Namespace:
            case Type::AnonymousNamespace:
            {
                return true;
            }
            default: break;
        }
        return false;
    }

    static bool isType(Type type) { return true; }

    bool isClassLike() const { return isClassLikeType(m_type); }

    virtual void dump(int indent, StringBuilder& out) = 0;

        /// Do depth first traversal of nodes in scopes
    virtual void calcScopeDepthFirst(List<Node*>& outNodes);

        /// Calculate the absolute name for this namespace/type
    void calcAbsoluteName(StringBuilder& outName) const;

        /// Get the absolute name
    String getAbsoluteName() const { StringBuilder buf; calcAbsoluteName(buf); return buf.ProduceString(); }

        /// Calculate the scope path to this node, from the root 
    void calcScopePath(List<Node*>& outPath) { calcScopePath(this, outPath); }

        /// True if reflected
    bool isReflected() const { return m_reflectionType == ReflectionType::Reflected; }

    ScopeNode* getRootScope();

    typedef bool(*Filter)(Node* node);

    static bool isClassLikeAndReflected(Node* node) { return node->isClassLike() && node->isReflected(); }
    static bool isClassLike(Node* node) { return isClassLikeType(node->m_type); }

    template <typename T>
    static void filter(Filter filter, List<T*>& io) { const Node* _isNodeDerived = (T*)nullptr; SLANG_UNUSED(_isNodeDerived); filterImpl(filter, reinterpret_cast<List<Node*>&>(io)); }

    static void filterImpl(Filter filter, List<Node*>& io);

    static void calcScopePath(Node* node, List<Node*>& outPath);

        /// Lookup a name in just the specified scope
        /// Handles anonymous namespaces, or name lookups that are in the parents space
    static Node* lookupNameInScope(ScopeNode* scope, const UnownedStringSlice& name);

        /// Lookup from a path
    static Node* lookupFromScope(ScopeNode* scope, const UnownedStringSlice* path, Index pathCount);
        /// Looks up *just* from the specified scope. 
    static Node* lookupFromScope(ScopeNode* scope, const UnownedStringSlice& slice);

        /// Look up name (which can contain ::) 
    static Node* lookup(ScopeNode* scope, const UnownedStringSlice& name);

    static void splitPath(const UnownedStringSlice& slice, List<UnownedStringSlice>& outSplitPath);

    Node(Type type) :
        m_type(type),
        m_parentScope(nullptr),
        m_reflectionType(ReflectionType::NotReflected)
    {
    }

    Type m_type;                        ///< The type of node this is
    ReflectionType m_reflectionType;    /// Classes can be traversed, but not reflected. To be reflected they have to contain the marker

    Token m_name;                       ///< The name of this scope/type    

    ScopeNode* m_parentScope;           ///< The scope this type/scope is defined in
};

struct ScopeNode : public Node
{
    typedef Node Super;

    static bool isType(Type type) { return isScopeType(type); }

    virtual void dump(int indent, StringBuilder& out) SLANG_OVERRIDE;
    virtual void calcScopeDepthFirst(List<Node*>& outNodes) SLANG_OVERRIDE;

        /// True if can accept fields (class like types can)
    bool acceptsFields() const { return isClassLike(); }
        /// True if the scope can accept types
    bool acceptsTypes() const { return canAcceptTypes(m_type); }

        /// Gets the reflection for any contained types
    ReflectionType getContainedReflectionType() const { return m_reflectionType == ReflectionType::NotReflected ? ReflectionType::NotReflected : m_reflectionOverride; }

        /// Add a child node to this nodes scope
    void addChild(Node* child);

        /// Find a child node in this scope with the specified name. Return nullptr if not found
    Node* findChild(const UnownedStringSlice& name) const;

        /// Gets the anonymous namespace associated with this scope
    ScopeNode* getAnonymousNamespace();

    ScopeNode(Type type) :
        Super(type),
        m_reflectionOverride(ReflectionType::Reflected),
        m_anonymousNamespace(nullptr)
    {
    }

    /// For child types, fields, how reflection is handled. If this type is not reflected
    ReflectionType m_reflectionOverride;

    /// All of the types and namespaces in this *scope*
    List<RefPtr<Node>> m_children;

    /// Map from a name (in this scope) to the Node
    Dictionary<UnownedStringSlice, Node*> m_childMap;

    /// There can only be one anonymousNamespace for a scope. If there is one it's held here
    ScopeNode* m_anonymousNamespace;
};

struct FieldNode : public Node
{
    typedef Node Super;

    static bool isType(Type type) { return type == Type::Field; }

    virtual void dump(int indent, StringBuilder& out) SLANG_OVERRIDE;

    FieldNode() :
        Super(Type::Field)
    {
    }

    UnownedStringSlice m_fieldType;

    // We may want to add initializer tokens
};

struct ClassLikeNode : public ScopeNode
{
    typedef ScopeNode Super;

    static bool isType(Type type) { return isClassLikeType(type); }

        /// Add a node that is derived from this
    void addDerived(ClassLikeNode* derived);

        /// Dump all of the derived types
    void dumpDerived(int indentCount, StringBuilder& out);

        /// Calculates the derived depth 
    Index calcDerivedDepth() const;

        /// Find the last (reflected) derived type
    ClassLikeNode* findLastDerived();

        /// Traverse the hierarchy of derived nodes, in depth first order
    void calcDerivedDepthFirst(List<ClassLikeNode*>& outNodes);

        /// True if has a derived type that is reflected
    bool hasReflectedDerivedType() const;

        /// Stores in out any reflected derived types
    void getReflectedDerivedTypes(List<ClassLikeNode*>& out) const;

    // Node Impl
    virtual void dump(int indent, StringBuilder& out) SLANG_OVERRIDE;

    ClassLikeNode(Type type) :
        Super(type),
        m_origin(nullptr),
        m_typeSet(nullptr),
        m_superNode(nullptr)
    {
        SLANG_ASSERT(type == Type::ClassType || type == Type::StructType);
    }

    SourceOrigin* m_origin;                             ///< Defines where this was uniquely defined. 

    Token m_marker;                                     ///< The marker associated with this scope (typically the marker is SLANG_CLASS etc, that is used to identify reflectedType)

    List<RefPtr<ClassLikeNode>> m_derivedTypes;         ///< All of the types derived from this type

    TypeSet* m_typeSet;                                 ///< The typeset this type belongs to. 

    Token m_super;                   ///< Super class name
    ClassLikeNode* m_superNode;      ///< If this is a class/struct, the type it is derived from (or nullptr if base)
};

struct EnumCaseNode : public Node
{
    typedef Node Super;

    static bool isType(Type type) { return type == Type::EnumCase; }

    virtual void dump(int indent, StringBuilder& out) SLANG_OVERRIDE;

    EnumCaseNode():
        Super(Type::EnumCase)
    {
    }

    Token m_value;              ///< If not defined will be invalid
};

struct EnumNode : public ScopeNode
{
    typedef ScopeNode Super;
    static bool isType(Type type) { return isEnumLikeType(type); }

    virtual void dump(int indent, StringBuilder& out) SLANG_OVERRIDE;

    EnumNode(Type type):
        Super(type)
    {
        SLANG_ASSERT(isEnumLikeType(type));
    }

    Token m_backingToken;
};

struct TypeDefNode : public Node
{
    typedef Node Super;
    static bool isType(Type type) { return type == Type::TypeDef; }

    virtual void dump(int indent, StringBuilder& out) SLANG_OVERRIDE;

    TypeDefNode():
        Super(Type::TypeDef)
    {
    }

    List<Token> m_targetTypeTokens;
};

template <typename T>
T* as(Node* node) { return (node && T::isType(node->m_type)) ? static_cast<T*>(node) : nullptr; }

} // CppExtract

#endif
