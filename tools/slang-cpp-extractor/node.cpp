#include "node.h"

#include "file-util.h"

#include "../../source/core/slang-string-util.h"

namespace CppExtract {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Node Impl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SLANG_FORCE_INLINE static void _indent(Index indentCount, StringBuilder& out) { FileUtil::indent(indentCount, out); }

ScopeNode* Node::getRootScope()
{
    if (m_parentScope)
    {
        ScopeNode* scope = m_parentScope;
        while (scope->m_parentScope)
        {
            scope = scope->m_parentScope;
        }
        return scope;
    }
    else
    {
        return as<ScopeNode>(this);
    }
}

void Node::calcScopeDepthFirst(List<Node*>& outNodes)
{
    outNodes.add(this);
}

void Node::calcAbsoluteName(StringBuilder& outName) const
{
    List<Node*> path;
    calcScopePath(const_cast<Node*>(this), path);

    // 1 so we skip the global scope
    for (Index i = 1; i < path.getCount(); ++i)
    {
        Node* node = path[i];

        if (i > 1)
        {
            outName << "::";
        }

        if (node->m_type == Type::AnonymousNamespace)
        {
            outName << "{Anonymous}";
        }
        else
        {
            outName << node->m_name.getContent();
        }
    }
}

/* static */void Node::calcScopePath(Node* node, List<Node*>& outPath)
{
    outPath.clear();

    while (node)
    {
        outPath.add(node);
        node = node->m_parentScope;
    }

    // reverse the order, so we go from root to the node
    outPath.reverse();
}

/* static */void Node::filterImpl(Filter inFilter, List<Node*>& ioNodes)
{
    // Filter out all the unreflected nodes
    Index count = ioNodes.getCount();
    for (Index j = 0; j < count; )
    {
        Node* node = ioNodes[j];

        if (!inFilter(node))
        {
            ioNodes.removeAt(j);
            count--;
        }
        else
        {
            j++;
        }
    }
}

/* static */Node* Node::lookupNameInScope(ScopeNode* scope, const UnownedStringSlice& name)
{
    // TODO(JS): Doesn't handle 'using namespace'.

    // Must be unqualified name
    SLANG_ASSERT(name.indexOf(UnownedStringSlice::fromLiteral("::")) < 0);

    Node* childNode = scope->findChild(name);
    if (childNode)
    {
        return childNode;
    }

    // If we have an anonymous namespace in this scope, try looking up in there..
    if (scope->m_anonymousNamespace)
    {
        Node* childNode = scope->m_anonymousNamespace->findChild(name);
        if (childNode)
        {
            return childNode;
        }
    }

    // I could have an enum (that's not an enum class)
    for (Node* node : scope->m_children)
    {
        EnumNode* enumNode = as<EnumNode>(node);
        if (enumNode && enumNode->m_type == Node::Type::Enum)
        {
            Node** nodePtr = enumNode->m_childMap.TryGetValue(name);
            if (nodePtr)
            {
                return *nodePtr;
            }
        }
    }

    return nullptr;
}

/* static */Node* Node::lookupFromScope(ScopeNode* scope, const UnownedStringSlice* parts, Index partsCount)
{
    SLANG_ASSERT(partsCount > 0);
    if (partsCount == 1)
    {
        return lookupNameInScope(scope, parts[0]);
    }

    for (Index i = 0; i < partsCount; ++i)
    {
        const UnownedStringSlice& part = parts[i];

        Node* node = lookupNameInScope(scope, part);
        if (node == nullptr)
        {
            return node;
        }
        // If at end, then we are done
        if (i == partsCount - 1)
        {
            return node;
        }

        // If there are more elements, then node must be some kind of scope,
        // if we are going to find it
        scope = as<ScopeNode>(node);
        if (scope == nullptr)
        {
            break;
        }
    }

    return nullptr;
}

/* static */void Node::splitPath(const UnownedStringSlice& inPath, List<UnownedStringSlice>& outParts)
{
    if (inPath.indexOf(UnownedStringSlice::fromLiteral("::")) >= 0)
    {
        StringUtil::split(inPath, UnownedStringSlice::fromLiteral("::"), outParts);
        // Remove any whitespace
        for (auto& part : outParts)
        {
            part = part.trim();
        }
    }
    else
    {
        outParts.clear();
        outParts.add(inPath.trim());
    }
}

/* static */Node* Node::lookupFromScope(ScopeNode* scope, const UnownedStringSlice& inPath)
{
    if (inPath.indexOf(UnownedStringSlice::fromLiteral("::")) >= 0)
    {
        List<UnownedStringSlice> parts;
        splitPath(inPath, parts);

        return lookupFromScope(scope, parts.getBuffer(), parts.getCount());
    }
    else
    {
        return lookupNameInScope(scope, inPath);
    }
}

/* static */Node* Node::lookup(ScopeNode* scope, const UnownedStringSlice& inPath)
{
    if (inPath.indexOf(UnownedStringSlice::fromLiteral("::")) >= 0)
    {
        List<UnownedStringSlice> parts;
        splitPath(inPath, parts);

        if (parts[0].getLength() == 0)
        {
            // It's a lookup from global scope
            ScopeNode* rootScope = scope->getRootScope();
            return lookupFromScope(rootScope, parts.getBuffer() + 1, parts.getCount() + 1);
        }

        // Okay lets try a lookup from each scope up to the global scope
        while (scope)
        {
            Node* node = lookupFromScope(scope, parts.getBuffer(), parts.getCount());
            if (node)
            {
                return node;
            }

            scope = scope->m_parentScope;
        }
    }
    else
    {
        while (scope)
        {
            // Lookup in this scope
            Node* node = lookupNameInScope(scope, inPath);
            if (node)
            {
                return node;
            }

            // Try parent scope
            scope = scope->m_parentScope;
        }
    }

    return nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ScopeNode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ScopeNode* ScopeNode::getAnonymousNamespace()
{
    if (!m_anonymousNamespace)
    {
        m_anonymousNamespace = new ScopeNode(Type::AnonymousNamespace);
        m_anonymousNamespace->m_parentScope = this;
        m_children.add(m_anonymousNamespace);
    }

    return m_anonymousNamespace;
}

void ScopeNode::addChild(Node* child)
{
    SLANG_ASSERT(child->m_parentScope == nullptr);
    // Can't add anonymous namespace this way - should be added via getAnonymousNamespace
    SLANG_ASSERT(child->m_type != Type::AnonymousNamespace);

    child->m_parentScope = this;
    m_children.add(child);

    if (child->m_name.hasContent())
    {
        m_childMap.Add(child->m_name.getContent(), child);
    }
}

Node* ScopeNode::findChild(const UnownedStringSlice& name) const
{
    Node** nodePtr = m_childMap.TryGetValue(name);
    if (nodePtr)
    {
        return *nodePtr;
    }



    return nullptr;
}

void ScopeNode::calcScopeDepthFirst(List<Node*>& outNodes)
{
    outNodes.add(this);
    for (Node* child : m_children)
    {
        child->calcScopeDepthFirst(outNodes);
    }
}

void ScopeNode::dump(int indentCount, StringBuilder& out)
{
    _indent(indentCount, out);

    switch (m_type)
    {
        case Type::AnonymousNamespace:
        {
            out << "namespace {\n";
        }
        case Type::Namespace:
        {
            if (m_name.hasContent())
            {
                out << "namespace " << m_name.getContent() << " {\n";
            }
            else
            {
                out << "{\n";
            }
            break;
        }
    }

    for (Node* child : m_children)
    {
        child->dump(indentCount + 1, out);
    }

    _indent(indentCount, out);
    out << "}\n";
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! EnumCaseNode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void EnumCaseNode::dump(int indent, StringBuilder& out)
{
    if (isReflected())
    {
        _indent(indent, out);
        out << m_name.getContent();

        if (m_value.type != TokenType::Invalid)
        {
            out << " = ";
            out << m_value.getContent();
        }

        out << ",\n";
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! EnumNode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void TypeDefNode::dump(int indent, StringBuilder& out)
{
    if (isReflected())
    {
        _indent(indent, out);

        out << "typedef ";

        for (auto& tok : m_targetTypeTokens)
        {
            out << tok.getContent() << " ";
        }

        out << m_name.getContent() << ";\n";
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! EnumNode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void EnumNode::dump(int indent, StringBuilder& out)
{
    if (!isReflected())
    {
        return;
    }

    _indent(indent, out);

    out << "enum ";

    if (m_type == Type::EnumClass)
    {
        out << "class ";
    }

    if (m_name.type != TokenType::Invalid)
    {
        out << m_name.getContent();
    }

    if (m_backingToken.type != TokenType::Invalid)
    {
        out << " : " << m_backingToken.getContent();
    }

    out << "\n";
    _indent(indent, out);
    out << "{\n";

    for (Node* child : m_children)
    {
        child->dump(indent + 1, out);
    }

    _indent(indent, out);
    out << "}\n";
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FieldNode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void FieldNode::dump(int indent, StringBuilder& out)
{
    if (isReflected())
    {
        _indent(indent, out);
        out << m_fieldType << " " << m_name.getContent() << "\n";
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ClassLikeNode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/// Add a node that is derived from this
void ClassLikeNode::addDerived(ClassLikeNode* derived)
{
    SLANG_ASSERT(derived->m_superNode == nullptr);
    derived->m_superNode = this;
    m_derivedTypes.add(derived);
}

void ClassLikeNode::calcDerivedDepthFirst(List<ClassLikeNode*>& outNodes)
{
    outNodes.add(this);
    for (ClassLikeNode* derivedType : m_derivedTypes)
    {
        derivedType->calcDerivedDepthFirst(outNodes);
    }
}

void ClassLikeNode::dumpDerived(int indentCount, StringBuilder& out)
{
    if (isClassLike() && isReflected() && m_name.hasContent())
    {
        _indent(indentCount, out);
        out << m_name.getContent() << "\n";
    }

    for (ClassLikeNode* derivedType : m_derivedTypes)
    {
        derivedType->dumpDerived(indentCount + 1, out);
    }
}

Index ClassLikeNode::calcDerivedDepth() const
{
    const ClassLikeNode* node = this;
    Index count = 0;

    while (node)
    {
        count++;
        node = node->m_superNode;
    }

    return count;
}

ClassLikeNode* ClassLikeNode::findLastDerived()
{
    for (Index i = m_derivedTypes.getCount() - 1; i >= 0; --i)
    {
        ClassLikeNode* derivedType = m_derivedTypes[i];
        ClassLikeNode* found = derivedType->findLastDerived();
        if (found)
        {
            return found;
        }
    }
    return this;
}

bool ClassLikeNode::hasReflectedDerivedType() const
{
    for (ClassLikeNode* type : m_derivedTypes)
    {
        if (type->isReflected())
        {
            return true;
        }
    }
    return false;
}

void ClassLikeNode::getReflectedDerivedTypes(List<ClassLikeNode*>& out) const
{
    out.clear();
    for (ClassLikeNode* type : m_derivedTypes)
    {
        if (type->isReflected())
        {
            out.add(type);
        }
    }
}

void ClassLikeNode::dump(int indentCount, StringBuilder& out)
{
    _indent(indentCount, out);

    const char* typeName = (m_type == Type::StructType) ? "struct" : "class";

    out << typeName << " ";

    if (!isReflected())
    {
        out << " (";
    }
    out << m_name.getContent();
    if (!isReflected())
    {
        out << ") ";
    }

    if (m_super.hasContent())
    {
        out << " : " << m_super.getContent();
    }

    out << " {\n";

    for (Node* child : m_children)
    {
        child->dump(indentCount + 1, out);
    }

    _indent(indentCount, out);
    out << "}\n";
}

} // namespace CppExtract
