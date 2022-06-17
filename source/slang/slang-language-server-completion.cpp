// slang-language-server-completion.cpp

#include "slang-language-server-completion.h"
#include "slang-language-server-ast-lookup.h"
#include "slang-language-server.h"

#include "slang-ast-all.h"
#include "slang-syntax.h"
#include "slang-check-impl.h"

namespace Slang
{

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
    if (version->linkage->contentAssistInfo.completionSuggestions.scopeKind != CompletionSuggestions::ScopeKind::HLSLSemantics)
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

SlangResult CompletionContext::tryCompleteMember()
{
    List<LanguageServerProtocol::CompletionItem> items = collectMembers();
    server->m_connection->sendResult(&items, responseId);
    return SLANG_OK;
}

List<LanguageServerProtocol::CompletionItem> CompletionContext::collectMembers()
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
    if (linkage->contentAssistInfo.completionSuggestions.scopeKind !=
        CompletionSuggestions::ScopeKind::Member)
    {
        return result;
    }
    HashSet<String> deduplicateSet;
    for (Index i = 0;
         i < linkage->contentAssistInfo.completionSuggestions.candidateItems.getCount();
         i++)
    {
        auto& suggestedItem = linkage->contentAssistInfo.completionSuggestions.candidateItems[i];
        auto member = suggestedItem.declRef.getDecl();
        if (!member->getName())
            continue;
        LanguageServerProtocol::CompletionItem item;
        item.label = member->getName()->text;
        item.kind = 0;
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

    for (auto& item : result)
    {
        for (auto ch : getCommitChars())
            item.commitCharacters.add(ch);
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
                nameSB << "_" << i + 1<< j + 1;
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
