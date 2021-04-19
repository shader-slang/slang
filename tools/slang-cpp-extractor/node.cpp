#include "node.h"

namespace CppExtract {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Node Impl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static void _indent(Index indentCount, StringBuilder& out)
{
    for (Index i = 0; i < indentCount; ++i)
    {
        out << CPP_EXTRACT_INDENT_STRING;
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

/* static */Node* Node::findNode(ScopeNode* scope, const UnownedStringSlice& name)
{
    // TODO(JS): We may want to lookup based on the path. 
    // If the name is qualified, we give up for not
    if (String(name).indexOf("::") >= 0)
    {
        return nullptr;
    }

    // Okay try in all scopes up to the root
    while (scope)
    {
        if (Node* node = scope->findChild(name))
        {
            return node;
        }

        scope = scope->m_parentScope;
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
    return (nodePtr) ? *nodePtr : nullptr;
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
