// slang-language-server-completion.cpp

#include "slang-language-server-completion.h"
#include "slang-language-server-ast-lookup.h"
#include "slang-language-server.h"

#include "slang-ast-all.h"
#include "slang-check-impl.h"
#include "slang-syntax.h"

namespace Slang
{

static const char* kDeclKeywords[] = {
    "throws",    "static",         "const",     "in",        "out",     "inout",
    "ref",       "__subscript",    "__init",    "property",  "get",     "set",
    "class",     "struct",         "interface", "public",    "private", "internal",
    "protected", "typedef",        "typealias", "uniform",   "export",  "groupshared",
    "extension", "associatedtype", "namespace", "This",    "using",
    "__generic", "__exported",     "import",    "enum",      "cbuffer",   "tbuffer",   "func"};
static const char* kStmtKeywords[] = {
    "if",        "else",           "switch",    "case",      "default", "return",
    "try",       "throw",          "throws",    "catch",     "while",   "for",
    "do",        "static",         "const",     "in",        "out",     "inout",
    "ref",       "__subscript",    "__init",    "property",  "get",     "set",
    "class",     "struct",         "interface", "public",    "private", "internal",
    "protected", "typedef",        "typealias", "uniform",   "export",  "groupshared",
    "extension", "associatedtype", "this",      "namespace", "This",    "using",
    "__generic", "__exported",     "import",    "enum",      "break",   "continue",
    "discard",   "defer",          "cbuffer",   "tbuffer",   "func"};

static const char* hlslSemanticNames[] = {
    "register",
    "packoffset",
    "read",
    "write",
    "SV_ClipDistance",
    "SV_CullDistance",
    "SV_Coverage",
    "SV_Depth",
    "SV_DepthGreaterEqual",
    "SV_DepthLessEqual",
    "SV_DispatchThreadID",
    "SV_DomainLocation",
    "SV_GroupID",
    "SV_GroupIndex",
    "SV_GroupThreadID",
    "SV_GSInstanceID",
    "SV_InnerCoverage",
    "SV_InsideTessFactor",
    "SV_InstanceID",
    "SV_IsFrontFace",
    "SV_OutputControlPointID",
    "SV_Position",
    "SV_PrimitiveID",
    "SV_RenderTargetArrayIndex",
    "SV_SampleIndex",
    "SV_StencilRef",
    "SV_Target",
    "SV_TessFactor",
    "SV_VertexID",
    "SV_ViewportArrayIndex",
    "SV_ShadingRate",
};

SlangResult CompletionContext::tryCompleteHLSLSemantic()
{
    if (version->linkage->contentAssistInfo.completionSuggestions.scopeKind !=
        CompletionSuggestions::ScopeKind::HLSLSemantics)
    {
        return SLANG_FAIL;
    }
    List<LanguageServerProtocol::CompletionItem> items;
    for (auto name : hlslSemanticNames)
    {
        LanguageServerProtocol::CompletionItem item;
        item.label = name;
        item.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
        items.add(item);
    }
    server->m_connection->sendResult(&items, responseId);
    return SLANG_OK;
}

SlangResult CompletionContext::tryCompleteAttributes()
{
    if (version->linkage->contentAssistInfo.completionSuggestions.scopeKind !=
        CompletionSuggestions::ScopeKind::Attribute)
    {
        return SLANG_FAIL;
    }
    List<LanguageServerProtocol::CompletionItem> items = collectAttributes();
    server->m_connection->sendResult(&items, responseId);
    return SLANG_OK;
}

SlangResult CompletionContext::tryCompleteMemberAndSymbol()
{
    List<LanguageServerProtocol::CompletionItem> items = collectMembersAndSymbols();
    server->m_connection->sendResult(&items, responseId);
    return SLANG_OK;
}

List<LanguageServerProtocol::CompletionItem> CompletionContext::collectMembersAndSymbols()
{
    auto linkage = version->linkage;
    if (linkage->contentAssistInfo.completionSuggestions.scopeKind ==
        CompletionSuggestions::ScopeKind::Swizzle)
    {
        return createSwizzleCandidates(
            linkage->contentAssistInfo.completionSuggestions.swizzleBaseType,
            linkage->contentAssistInfo.completionSuggestions.elementCount);
    }
    List<LanguageServerProtocol::CompletionItem> result;
    bool useCommitChars = true;
    bool addKeywords = false;
    switch (linkage->contentAssistInfo.completionSuggestions.scopeKind)
    {
    case CompletionSuggestions::ScopeKind::Member:
        useCommitChars = (commitCharacterBehavior == CommitCharacterBehavior::MembersOnly || commitCharacterBehavior == CommitCharacterBehavior::All);
        break;
    case CompletionSuggestions::ScopeKind::Expr:
    case CompletionSuggestions::ScopeKind::Decl:
    case CompletionSuggestions::ScopeKind::Stmt:
        useCommitChars = (commitCharacterBehavior == CommitCharacterBehavior::All);
        addKeywords = true;
        break;
    default:
        return result;
    }
    HashSet<String> deduplicateSet;
    for (Index i = 0;
         i < linkage->contentAssistInfo.completionSuggestions.candidateItems.getCount();
         i++)
    {
        auto& suggestedItem = linkage->contentAssistInfo.completionSuggestions.candidateItems[i];
        auto member = suggestedItem.declRef.getDecl();
        if (auto genericDecl = as<GenericDecl>(member))
            member = genericDecl->inner;
        if (!member)
            continue;
        if (!member->getName())
            continue;
        LanguageServerProtocol::CompletionItem item;
        item.label = member->getName()->text;
        item.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
        if (as<TypeConstraintDecl>(member))
        {
            continue;
        }
        if (as<ConstructorDecl>(member))
        {
            continue;
        }
        if (as<SubscriptDecl>(member))
        {
            continue;
        }
        if (item.label.getLength() == 0)
            continue;
        if (!_isIdentifierChar(item.label[0]))
            continue;
        if (item.label.startsWith("$"))
            continue;
        if (!deduplicateSet.Add(item.label))
            continue;

        if (as<StructDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindStruct;
        }
        else if (as<ClassDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindClass;
        }
        else if (as<InterfaceDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindInterface;
        }
        else if (as<SimpleTypeDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindClass;
        }
        else if (as<PropertyDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindProperty;
        }
        else if (as<EnumDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindEnum;
        }
        else if (as<VarDeclBase>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindVariable;
        }
        else if (as<EnumCaseDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindEnumMember;
        }
        else if (as<CallableDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindMethod;
        }
        else if (as<AssocTypeDecl>(member))
        {
            item.kind = LanguageServerProtocol::kCompletionItemKindClass;
        }
        item.data = String(i);
        result.add(item);
    }
    if (addKeywords)
    {
        if (linkage->contentAssistInfo.completionSuggestions.scopeKind ==
            CompletionSuggestions::ScopeKind::Decl)
        {
            for (auto keyword : kDeclKeywords)
            {
                if (!deduplicateSet.Add(keyword))
                    continue;
                LanguageServerProtocol::CompletionItem item;
                item.label = keyword;
                item.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
                item.data = "-1";
                result.add(item);
            }
        }
        else
        {
            for (auto keyword : kStmtKeywords)
            {
                if (!deduplicateSet.Add(keyword))
                    continue;
                LanguageServerProtocol::CompletionItem item;
                item.label = keyword;
                item.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
                item.data = "-1";
                result.add(item);
            }
        }
        
        for (auto& def : linkage->contentAssistInfo.preprocessorInfo.macroDefinitions)
        {
            if (!def.name)
                continue;
            auto& text = def.name->text;
            if (!deduplicateSet.Add(text))
                continue;
            LanguageServerProtocol::CompletionItem item;
            item.label = text;
            item.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
            item.data = "-1";
            result.add(item);
        }
    }
    if (useCommitChars)
    {
        for (auto& item : result)
        {
            for (auto ch : getCommitChars())
                item.commitCharacters.add(ch);
        }
    }
    return result;
}

List<LanguageServerProtocol::CompletionItem> CompletionContext::createSwizzleCandidates(
    Type* type, IntegerLiteralValue elementCount[2])
{
    List<LanguageServerProtocol::CompletionItem> result;
    // Hard code members for vector and matrix types.
    result.clear();
    if (auto vectorType = as<VectorExpressionType>(type))
    {
        const char* memberNames[4] = {"x", "y", "z", "w"};
        Type* elementType = nullptr;
        elementType = vectorType->elementType;
        String typeStr;
        if (elementType)
            typeStr = elementType->toString();
        auto count = Math::Min((int)elementCount[0], 4);
        for (int i = 0; i < count; i++)
        {
            LanguageServerProtocol::CompletionItem item;
            item.data = 0;
            item.detail = typeStr;
            item.kind = LanguageServerProtocol::kCompletionItemKindVariable;
            item.label = memberNames[i];
            result.add(item);
        }
    }
    else if (auto matrixType = as<MatrixExpressionType>(type))
    {
        Type* elementType = nullptr;
        elementType = matrixType->getElementType();
        String typeStr;
        if (elementType)
        {
            typeStr = elementType->toString();
        }
        int rowCount = Math::Min((int)elementCount[0], 4);
        int colCount = Math::Min((int)elementCount[1], 4);
        StringBuilder nameSB;
        for (int i = 0; i < rowCount; i++)
        {
            for (int j = 0; j < colCount; j++)
            {
                LanguageServerProtocol::CompletionItem item;
                item.data = 0;
                item.detail = typeStr;
                item.kind = LanguageServerProtocol::kCompletionItemKindVariable;
                nameSB.Clear();
                nameSB << "_m" << i << j;
                item.label = nameSB.ToString();
                result.add(item);
                nameSB.Clear();
                nameSB << "_" << i + 1 << j + 1;
                item.label = nameSB.ToString();
                result.add(item);
            }
        }
    }
    for (auto& item : result)
    {
        for (auto ch : getCommitChars())
            item.commitCharacters.add(ch);
    }
    return result;
}

List<LanguageServerProtocol::CompletionItem> CompletionContext::collectAttributes()
{
    List<LanguageServerProtocol::CompletionItem> result;
    for (auto& item : version->linkage->contentAssistInfo.completionSuggestions.candidateItems)
    {
        if (auto attrDecl = as<AttributeDecl>(item.declRef.getDecl()))
        {
            if (attrDecl->getName())
            {
                LanguageServerProtocol::CompletionItem resultItem;
                resultItem.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
                resultItem.label = attrDecl->getName()->text;
                result.add(resultItem);
            }
        }
        else if (auto decl = as<AggTypeDecl>(item.declRef.getDecl()))
        {
            if (decl->getName())
            {
                LanguageServerProtocol::CompletionItem resultItem;
                resultItem.kind = LanguageServerProtocol::kCompletionItemKindStruct;
                resultItem.label = decl->getName()->text;
                if (resultItem.label.endsWith("Attribute"))
                    resultItem.label.reduceLength(resultItem.label.getLength() - 9);
                result.add(resultItem);
            }
        }
    }
    return result;
}

} // namespace Slang
