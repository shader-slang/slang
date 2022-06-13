// slang-language-server-completion.cpp

#include "slang-language-server-completion.h"
#include "slang-language-server-ast-lookup.h"
#include "slang-language-server.h"

#include "slang-ast-all.h"
#include "slang-syntax.h"
#include "slang-check-impl.h"

namespace Slang
{

static bool _isIdentifierChar(char ch)
{
    return ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_';
}

static bool _isWhitespaceChar(char ch) { return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t'; }

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
    auto findResult = findASTNodesAt(
        doc,
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath,
        line,
        col);
    if (findResult.getCount() == 1 && findResult[0].path.getCount() != 0)
    {
        if (auto semantic = as<HLSLSemantic>(findResult[0].path.getLast()))
        {
            List<LanguageServerProtocol::CompletionItem> items;
            for (auto name : hlslSemanticNames)
            {
                LanguageServerProtocol::CompletionItem item;
                item.label = name;
                item.kind = LanguageServerProtocol::kCompletionItemKindKeyword;
                for (auto ch : getCommitChars())
                    item.commitCharacters.add(ch);
                items.add(item);
            }
            server->m_connection->sendResult(&items, responseId);
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

SlangResult CompletionContext::tryCompleteMember()
{
    // Scan backward until we locate a '.' or ':'.
    if (cursorOffset > 0)
        cursorOffset--;
    while (cursorOffset > 0 && _isWhitespaceChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }
    while (cursorOffset > 0 && _isIdentifierChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }
    while (cursorOffset > 0 && _isWhitespaceChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }
    if (cursorOffset > 0 && doc->getText()[cursorOffset] == ':')
        cursorOffset--;
    if (cursorOffset <= 0 ||
        (doc->getText()[cursorOffset] != '.' && doc->getText()[cursorOffset] != ':'))
    {
        return SLANG_FAIL;
    }
    doc->offsetToLineCol(cursorOffset, line, col);
    auto findResult = findASTNodesAt(
        doc,
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath,
        line,
        col);
    if (findResult.getCount() != 1)
    {
        return SLANG_FAIL;
    }
    if (findResult[0].path.getCount() == 0)
    {
        return SLANG_FAIL;
    }
    Expr* baseExpr = nullptr;
    if (auto memberExpr = as<MemberExpr>(findResult[0].path.getLast()))
    {
        baseExpr = memberExpr->baseExpression;
    }
    else if (auto staticMemberExpr = as<StaticMemberExpr>(findResult[0].path.getLast()))
    {
        baseExpr = staticMemberExpr->baseExpression;
    }
    else if (auto swizzleExpr = as<SwizzleExpr>(findResult[0].path.getLast()))
    {
        baseExpr = swizzleExpr->base;
    }
    else if (auto matSwizzleExpr = as<MatrixSwizzleExpr>(findResult[0].path.getLast()))
    {
        baseExpr = matSwizzleExpr->base;
    }
    if (!baseExpr || !baseExpr->type.type ||
        baseExpr->type.type->equals(version->linkage->getASTBuilder()->getErrorType()))
    {
        return SLANG_FAIL;
    }

    List<LanguageServerProtocol::CompletionItem> items = collectMembers(baseExpr);
    server->m_connection->sendResult(&items, responseId);
    return SLANG_OK;
}

// The following collectMember* functions implement the logic to collect all members from a parsed type.]
// The flow is mostly the same as `lookupMemberInType`, but instead of looking for a specific name,
// we collect all members we see.

struct MemberCollectingContext
{
    ASTBuilder* astBuilder;
    List<Decl*> members;
    bool includeInstanceMembers = true;
    SharedSemanticsContext semanticsContext;
    MemberCollectingContext(Linkage* linkage, Module* module, DiagnosticSink* sink)
        : semanticsContext(linkage, module, sink)
    {}
};

void collectMembersInTypeDeclImpl(MemberCollectingContext* context, DeclRef<Decl> declRef);

void collectMembersInType(MemberCollectingContext* context, Type* type);

void collectMembersInType(MemberCollectingContext* context, Type* type)
{
    if (auto pointerLikeType = as<PointerLikeType>(type))
    {
        collectMembersInType(context, pointerLikeType->elementType);
        return;
    }

    if (auto declRefType = as<DeclRefType>(type))
    {
        auto declRef = declRefType->declRef;

        collectMembersInTypeDeclImpl(
            context,
            declRef);
    }
    else if (auto nsType = as<NamespaceType>(type))
    {
        auto declRef = nsType->declRef;

        collectMembersInTypeDeclImpl(context, declRef);
    }
    else if (auto extractExistentialType = as<ExtractExistentialType>(type))
    {
        // We want lookup to be performed on the underlying interface type of the existential,
        // but we need to have a this-type substitution applied to ensure that the result of
        // lookup will have a comparable substitution applied (allowing things like associated
        // types, etc. used in the signature of a method to resolve correctly).
        //
        auto interfaceDeclRef = extractExistentialType->getSpecializedInterfaceDeclRef();
        collectMembersInTypeDeclImpl(context, interfaceDeclRef);
    }
    else if (auto thisType = as<ThisType>(type))
    {
        auto interfaceType = DeclRefType::create(context->astBuilder, thisType->interfaceDeclRef);
        collectMembersInType(context, interfaceType);
    }
    else if (auto andType = as<AndType>(type))
    {
        auto leftType = andType->left;
        auto rightType = andType->right;
        collectMembersInType(context, leftType);
        collectMembersInType(context, rightType);
    }
}

void collectMembersInTypeDeclImpl(
    MemberCollectingContext* context,
    DeclRef<Decl> declRef)
{
    if (declRef.getDecl()->checkState.getState() < DeclCheckState::ReadyForLookup)
        return;

    if (auto genericTypeParamDeclRef = declRef.as<GenericTypeParamDecl>())
    {
        // If the type we are doing lookup in is a generic type parameter,
        // then the members it provides can only be discovered by looking
        // at the constraints that are placed on that type.
        auto genericDeclRef = genericTypeParamDeclRef.getParent().as<GenericDecl>();
        assert(genericDeclRef);

        for (auto constraintDeclRef : getMembersOfType<GenericTypeConstraintDecl>(genericDeclRef))
        {
            if (constraintDeclRef.decl->checkState.getState() < DeclCheckState::ReadyForLookup)
            {
                continue;
            }

            collectMembersInType(
                context,
                getSup(context->astBuilder, constraintDeclRef));
        }
    }
    else if (declRef.as<AssocTypeDecl>() || declRef.as<GlobalGenericParamDecl>())
    {
        for (auto constraintDeclRef :
             getMembersOfType<TypeConstraintDecl>(declRef.as<ContainerDecl>()))
        {
            if (constraintDeclRef.decl->checkState.getState() < DeclCheckState::ReadyForLookup)
            {
                continue;
            }
            collectMembersInType(context, getSup(context->astBuilder, constraintDeclRef));
        }
    }
    else if (auto namespaceDecl = declRef.as<NamespaceDecl>())
    {
        for (auto member : namespaceDecl.getDecl()->members)
        {
            if (member->getName())
            {
                context->members.add(member);
            }
        }
    }
    else if (auto aggTypeDeclBaseRef = declRef.as<AggTypeDeclBase>())
    {
        // In this case we are peforming lookup in the context of an aggregate
        // type or an `extension`, so the first thing to do is to look for
        // matching members declared directly in the body of the type/`extension`.
        //
        for (auto member : aggTypeDeclBaseRef.getDecl()->members)
        {
            if (member->getName())
            {
                if (!context->includeInstanceMembers)
                {
                    // Skip non-static members.
                    if (as<PropertyDecl>(member))
                        continue;
                    if (as<SubscriptDecl>(member))
                        continue;
                    if (as<VarDeclBase>(member) || as<FuncDecl>(member))
                    {
                        if (!member->findModifier<HLSLStaticModifier>())
                        {
                            continue;
                        }
                    }
                }
                context->members.add(member);
            }
        }
        
        if (auto aggTypeDeclRef = aggTypeDeclBaseRef.as<AggTypeDecl>())
        {
            auto extensions =
                context->semanticsContext.getCandidateExtensionsForTypeDecl(aggTypeDeclRef);
            for (auto extDecl : extensions)
            {
                // TODO: check if the extension can be applied before including its members.
                // TODO: eventually we need to insert a breadcrumb here so that
                // the constructed result can somehow indicate that a member
                // was found through an extension.
                //
                collectMembersInTypeDeclImpl(
                    context,
                    DeclRef<Decl>(extDecl, nullptr));
            }
        }

        // For both aggregate types and their `extension`s, we want lookup to follow
        // through the declared inheritance relationships on each declaration.
        //
        for (auto inheritanceDeclRef : getMembersOfType<InheritanceDecl>(aggTypeDeclBaseRef))
        {
            // Some things that are syntactically `InheritanceDecl`s don't actually
            // represent a subtype/supertype relationship, and thus we shouldn't
            // include members from the base type when doing lookup in the
            // derived type.
            //
            if (inheritanceDeclRef.getDecl()->hasModifier<IgnoreForLookupModifier>())
                continue;

            collectMembersInType(
                context, getSup(context->astBuilder, inheritanceDeclRef));
        }
    }
}

List<LanguageServerProtocol::CompletionItem> CompletionContext::collectMembers(Expr* baseExpr)
{
    List<LanguageServerProtocol::CompletionItem> result;
    auto linkage = version->linkage;
    Type* type = baseExpr->type.type;
    bool isInstance = true;
    if (auto typeType = as<TypeType>(type))
    {
        type = typeType->type;
        isInstance = false;
    }
    version->currentCompletionItems.clear();
    if (type)
    {
        if (isInstance && as<ArithmeticExpressionType>(type))
        {
            // Hard code members for vector and matrix types.
            result.clear();
            version->currentCompletionItems.clear();
            int elementCount = 0;
            Type* elementType = nullptr;
            const char* memberNames[4] = {"x", "y", "z", "w"};
            if (auto vectorType = as<VectorExpressionType>(type))
            {
                if (auto elementCountVal = as<ConstantIntVal>(vectorType->elementCount))
                {
                    elementCount = (int)elementCountVal->value;
                    elementType = vectorType->elementType;
                }
            }
            else if (auto matrixType = as<MatrixExpressionType>(type))
            {
                if (auto elementCountVal = as<ConstantIntVal>(matrixType->getRowCount()))
                {
                    elementCount = (int)elementCountVal->value;
                    elementType = matrixType->getRowType();
                }
            }
            String typeStr;
            if (elementType)
                typeStr = elementType->toString();
            elementCount = Math::Min(elementCount, 4);
            for (int i = 0; i < elementCount; i++)
            {
                LanguageServerProtocol::CompletionItem item;
                item.data = 0;
                item.detail = typeStr;
                item.kind = LanguageServerProtocol::kCompletionItemKindVariable;
                item.label = memberNames[i];
                result.add(item);
            }
        }
        else
        {
            DiagnosticSink sink;
            MemberCollectingContext context(linkage, parsedModule, &sink);
            context.astBuilder = linkage->getASTBuilder();
            context.includeInstanceMembers = isInstance;
            collectMembersInType(&context, type);
            HashSet<String> deduplicateSet;
            for (auto member : context.members)
            {
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
                item.data = String(version->currentCompletionItems.getCount());
                result.add(item);
                version->currentCompletionItems.add(member);
            }
        }

        for (auto& item : result)
        {
            for (auto ch : getCommitChars())
                item.commitCharacters.add(ch);
        }
    }
    return result;
}

} // namespace Slang
