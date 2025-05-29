// slang-serialize-ast.cpp
#include "slang-serialize-ast.h"

#include "slang-ast-dispatch.h"
#include "slang-compiler.h"
#include "slang-diagnostics.h"
#include "slang-mangle.h"
#include "slang-serialize-riff.h"

namespace Slang
{
// TODO(tfoley): have the parser export this, or a utility function
// for initializing a `SyntaxDecl` in the common case.
//
NodeBase* parseSimpleSyntax(Parser* parser, void* userData);

//
// Many of the types used in the AST can be serialized using
// just the `Serializer` type, so we will handle all of those first.
//

void serialize(Serializer const& serializer, ASTNodeType& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, TypeTag& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, BaseType& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, TryClauseType& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, DeclVisibility& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, BuiltinRequirementKind& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, ImageFormat& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, PreferRecomputeAttribute::SideEffectBehavior& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, TreatAsDifferentiableExpr::Flavor& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, LogicOperatorShortCircuitExpr::Flavor& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, RequirementWitness::Flavor& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, CapabilityAtom& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, DeclAssociationKind& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, TokenType& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, ValNodeOperandKind& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, SPIRVAsmOperand::Flavor& value)
{
    serializeEnum(serializer, value);
}

void serialize(Serializer const& serializer, SlangLanguageVersion version)
{
    serializeEnum(serializer, version);
}

void serialize(Serializer const& serializer, MatrixCoord& value)
{
    SLANG_SCOPED_SERIALIZER_TUPLE(serializer);
    serialize(serializer, value.row);
    serialize(serializer, value.col);
}

void serializePtr(Serializer const& serializer, DiagnosticInfo const*& value, DiagnosticInfo const*)
{
    Int32 id = 0;
    if (isWriting(serializer))
    {
        id = value->id;
        serialize(serializer, id);
    }
    else
    {
        serialize(serializer, id);
        value = getDiagnosticsLookup()->getDiagnosticById(id);
    }
}

void serialize(Serializer const& serializer, SemanticVersion& value)
{
    auto raw = value.getRawValue();
    serialize(serializer, raw);
    value = SemanticVersion::fromRaw(raw);
}

void serialize(Serializer const& serializer, SyntaxClass<NodeBase>& value)
{
    ASTNodeType raw;
    if (isWriting(serializer))
    {
        raw = value.getTag();
    }
    serialize(serializer, raw);
    if (isReading(serializer))
    {
        value = SyntaxClass<NodeBase>(raw);
    }
}

//
// Many types in the AST need additional context (beyond
// what the `Serializer` has) in order to serialize
// themselves or their members.
//
// We define a custom serializer interface to capture
// the cases that can't be handled by a `Serializer`
// alone.
//

/// Interface for AST serialization
struct ASTSerializerImpl
{
public:
    virtual void handleASTNode(NodeBase*& value) = 0;
    virtual void handleASTNodeContents(NodeBase* value) = 0;
    virtual void handleName(Name*& value) = 0;
    virtual void handleSourceLoc(SourceLoc& value) = 0;
    virtual void handleToken(Token& value) = 0;

    // Note that this type does *not* inherit from `ISerializerImpl`.
    //
    // We want to decouple the AST-specific context information
    // from the lower-level details of the serialization format.
    //
    // Instead of using inheritance, we expect that any
    // `ASTSerializerImpl` will aggregate a lower-level
    // serializer, and the interface exposes access to
    // that base serializer implementation.

    virtual ISerializerImpl* getBaseSerializer() = 0;
};

/// Specialization of `Serializer_` for AST serialization.
template<>
struct Serializer_<ASTSerializerImpl> : SerializerBase<ASTSerializerImpl>
{
public:
    using SerializerBase::SerializerBase;

    //
    // In order to allow an `ASTSerializer` to be used with
    // functions that expect an ordinary `Serializer`, we
    // implement an implicit conversion operator.
    //

    operator Serializer() const { return Serializer(get()->getBaseSerializer()); }
};

/// Context type for AST serialization.
using ASTSerializer = Serializer_<ASTSerializerImpl>;

template<typename T>
void serializeObject(ASTSerializer const& serializer, T*& value, NodeBase*)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serializer->handleASTNode(*(NodeBase**)&value);
}

void serializeObjectContents(ASTSerializer const& serializer, NodeBase* value, NodeBase*)
{
    serializer->handleASTNodeContents(value);
}

template<typename T>
void serialize(ASTSerializer const& serializer, DeclRef<T>& value)
{
    serialize(serializer, value.declRefBase);
}

void serialize(ASTSerializer const& serializer, SourceLoc& value)
{
    serializer->handleSourceLoc(value);
}

void serialize(ASTSerializer const& serializer, RequirementWitness& value)
{
    SLANG_SCOPED_SERIALIZER_TAGGED_UNION(serializer);
    serialize(serializer, value.m_flavor);
    switch (value.m_flavor)
    {
    case RequirementWitness::Flavor::none:
        break;

    case RequirementWitness::Flavor::declRef:
        serialize(serializer, value.m_declRef);
        break;

    case RequirementWitness::Flavor::val:
        serialize(serializer, value.m_val);
        break;

    case RequirementWitness::Flavor::witnessTable:
        serialize(serializer, value.m_obj);
        break;
    }
}

void serialize(ASTSerializer const& serializer, WitnessTable& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.baseType);
    serialize(serializer, value.witnessedType);
    serialize(serializer, value.isExtern);

    // TODO(tfoley): In theory we should be able to streamline
    // this so that we only encode the requirements that we
    // absolutely need to (which basically amounts to `associatedtype`
    // requirements where the satisfying type is part of the public
    // API of the type).
    //
    serialize(serializer, value.m_requirementDictionary);
}

void serialize(Serializer const& serializer, CapabilityAtomSet& value)
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);
    if (isWriting(serializer))
    {
        for (auto rawAtom : value)
        {
            auto atom = CapabilityAtom(rawAtom);
            serialize(serializer, atom);
        }
    }
    else
    {
        while (hasElements(serializer))
        {
            CapabilityAtom atom;
            serialize(serializer, atom);
            value.add(UInt(atom));
        }
    }
}

void serialize(Serializer const& serializer, CapabilityStageSet& value)
{
    serialize(serializer, value.atomSet);
}

void serialize(Serializer const& serializer, CapabilityTargetSet& value)
{
    serialize(serializer, value.shaderStageSets);

    // The value for each entry in `shaderStageSets` have
    // a `stage` field that is redundant with the key for
    // that entry. Rather than serialize the key as part
    // of the `CapabilityStageSet` type, we instead copy
    // it over from the key to the value in the case where
    // we are reading.
    //
    if (isReading(serializer))
    {
        for (auto& p : value.shaderStageSets)
            p.second.stage = p.first;
    }
}

void serialize(Serializer const& serializer, CapabilitySet& value)
{
    serialize(serializer, value.getCapabilityTargetSets());

    // The value for each entry in `getCapabilityTargetSets()` have
    // a `target` field that is redundant with the key for
    // that entry. Rather than serialize the key as part
    // of the `CapabilityTargetSet` type, we instead copy
    // it over from the key to the value in the case where
    // we are reading.
    //
    if (isReading(serializer))
    {
        for (auto& p : value.getCapabilityTargetSets())
            p.second.target = p.first;
    }
}

void serialize(ASTSerializer const& serializer, CandidateExtensionList& value)
{
    serialize(serializer, value.candidateExtensions);
}

void serialize(ASTSerializer const& serializer, DeclAssociation& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.kind);
    serialize(serializer, value.decl);
}

void serialize(ASTSerializer const& serializer, DeclAssociationList& value)
{
    serialize(serializer, value.associations);
}

void serialize(ASTSerializer const& serializer, Modifiers& value)
{
    SLANG_SCOPED_SERIALIZER_ARRAY(serializer);
    if (isWriting(serializer))
    {
        for (auto modifier : value)
        {
            serialize(serializer, modifier);
        }
    }
    else
    {
        Modifier** link = &value.first;

        while (hasElements(serializer))
        {
            Modifier* modifier = nullptr;
            serialize(serializer, modifier);

            *link = modifier;
            link = &modifier->next;
        }
    }
}

void serialize(ASTSerializer const& serializer, TypeExp& value)
{
    serialize(serializer, value.type);
}

void serialize(ASTSerializer const& serializer, QualType& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.type);
    serialize(serializer, value.isLeftValue);
    serialize(serializer, value.hasReadOnlyOnTarget);
    serialize(serializer, value.isWriteOnly);
}

void serialize(ASTSerializer const& serializer, Token& value)
{
    serializer->handleToken(value);
}

void serialize(ASTSerializer const& serializer, SPIRVAsmOperand& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.flavor);
    serialize(serializer, value.token);
    serialize(serializer, value.expr);
    serialize(serializer, value.bitwiseOrWith);
    serialize(serializer, value.knownValue);
    serialize(serializer, value.wrapInId);
    serialize(serializer, value.type);
}

void serialize(ASTSerializer const& serializer, SPIRVAsmInst& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.opcode);
    serialize(serializer, value.operands);
}

void serialize(ASTSerializer const& serializer, ValNodeOperand& value)
{
    SLANG_SCOPED_SERIALIZER_TAGGED_UNION(serializer);
    serialize(serializer, value.kind);
    switch (value.kind)
    {
    case ValNodeOperandKind::ConstantValue:
        serialize(serializer, value.values.intOperand);
        break;

    case ValNodeOperandKind::ValNode:
    case ValNodeOperandKind::ASTNode:
        serialize(serializer, value.values.nodeOperand);
        break;
    }
}

void serializeObject(ASTSerializer const& serializer, Name*& value, Name*)
{
    serializer->handleName(value);
}

void serialize(ASTSerializer const& serializer, NameLoc& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.name);
    serialize(serializer, value.loc);
}

#if 0 // FIDDLE TEMPLATE:
%for _,T in ipairs(Slang.NodeBase.subclasses) do
void _serializeASTNodeContents(ASTSerializer const& serializer, $T* value)
{
    SLANG_UNUSED(serializer);
    SLANG_UNUSED(value);
%   if T.directSuperClass then
    _serializeASTNodeContents(serializer, static_cast<$(T.directSuperClass)*>(value));
%   end
%   for _,f in ipairs(T.directFields) do
    serialize(serializer, value->$f);
%   end
        }
%end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-serialize-ast.cpp.fiddle"
#endif // FIDDLE END

void serializeASTNodeContents(ASTSerializer const& serializer, NodeBase* node)
{
    ASTNodeDispatcher<NodeBase, void>::dispatch(
        node,
        [&](auto n) { _serializeASTNodeContents(serializer, n); });
}

enum class PseudoASTNodeType
{
    None,
    ImportedModule,
    ImportedDecl,
};

static PseudoASTNodeType _getPseudoASTNodeType(ASTNodeType type)
{
    return int(type) < 0 ? PseudoASTNodeType(~int(type)) : PseudoASTNodeType::None;
}

static ASTNodeType _getAsASTNodeType(PseudoASTNodeType type)
{
    return ASTNodeType(~int(type));
}

struct ASTEncodingContext : ASTSerializerImpl
{
public:
    ASTEncodingContext(
        RIFF::BuildCursor& cursor,
        ModuleDecl* module,
        SerialSourceLocWriter* sourceLocWriter)
        : _writer(cursor.getCurrentChunk()), _module(module), _sourceLocWriter(sourceLocWriter)
    {
    }

private:
    RIFFSerialWriter _writer;
    ModuleDecl* _module = nullptr;
    SerialSourceLocWriter* _sourceLocWriter = nullptr;

    virtual ISerializerImpl* getBaseSerializer() override { return &_writer; }

    virtual void handleName(Name*& value) override;
    virtual void handleSourceLoc(SourceLoc& value) override;
    virtual void handleToken(Token& value) override;
    virtual void handleASTNode(NodeBase*& node) override;
    virtual void handleASTNodeContents(NodeBase* node) override;

    void _writeImportedModule(ModuleDecl* moduleDecl);
    void _writeImportedDecl(Decl* decl, ModuleDecl* importedFromModuleDecl);

    ModuleDecl* _findModuleForDecl(Decl* decl)
    {
        for (auto d = decl; d; d = d->parentDecl)
        {
            if (auto m = as<ModuleDecl>(d))
                return m;
        }
        return nullptr;
    }

    ModuleDecl* _findModuleDeclWasImportedFrom(Decl* decl)
    {
        auto declModule = _findModuleForDecl(decl);
        if (declModule == nullptr)
            return nullptr;
        if (declModule == _module)
            return nullptr;
        return declModule;
    }
};

struct ASTDecodingContext : ASTSerializerImpl
{
public:
    ASTDecodingContext(
        Linkage* linkage,
        ASTBuilder* astBuilder,
        DiagnosticSink* sink,
        RIFF::Chunk const* baseChunk,
        SerialSourceLocReader* sourceLocReader,
        SourceLoc requestingSourceLoc)
        : _linkage(linkage)
        , _astBuilder(astBuilder)
        , _sink(sink)
        , _sourceLocReader(sourceLocReader)
        , _requestingSourceLoc(requestingSourceLoc)
        , _riffReader(baseChunk)
    {
    }

private:
    Linkage* _linkage = nullptr;
    ASTBuilder* _astBuilder = nullptr;
    DiagnosticSink* _sink = nullptr;
    SerialSourceLocReader* _sourceLocReader = nullptr;
    SourceLoc _requestingSourceLoc;
    RIFFSerialReader _riffReader;

    virtual ISerializerImpl* getBaseSerializer() override { return &_riffReader; }

    virtual void handleName(Name*& value) override;
    virtual void handleSourceLoc(SourceLoc& value) override;
    virtual void handleToken(Token& value) override;
    virtual void handleASTNode(NodeBase*& outNode) override;
    virtual void handleASTNodeContents(NodeBase* node) override;

    ModuleDecl* _readImportedModule();
    NodeBase* _readImportedDecl();

    void _cleanUpASTNode(NodeBase* node)
    {
        if (auto expr = as<Expr>(node))
        {
            expr->checked = true;
        }
        else if (auto decl = as<Decl>(node))
        {
            decl->checkState = DeclCheckState::CapabilityChecked;

            if (auto genericDecl = as<GenericDecl>(node))
            {
                _assignGenericParameterIndices(genericDecl);
            }
            else if (auto syntaxDecl = as<SyntaxDecl>(node))
            {
                syntaxDecl->parseCallback = &parseSimpleSyntax;
                syntaxDecl->parseUserData = (void*)syntaxDecl->syntaxClass.getInfo();
            }
            else if (auto namespaceLikeDecl = as<NamespaceDeclBase>(node))
            {
                auto declScope = _astBuilder->create<Scope>();
                declScope->containerDecl = namespaceLikeDecl;
                namespaceLikeDecl->ownedScope = declScope;
            }
        }
    }

    void _assignGenericParameterIndices(GenericDecl* genericDecl)
    {
        int parameterCounter = 0;
        for (auto m : genericDecl->members)
        {
            if (auto typeParam = as<GenericTypeParamDeclBase>(m))
            {
                typeParam->parameterIndex = parameterCounter++;
            }
            else if (auto valParam = as<GenericValueParamDecl>(m))
            {
                valParam->parameterIndex = parameterCounter++;
            }
        }
    }
};

//
// We are matching up the corresponding `handle*()` operations from the
// `AST{Encoding|Decoding}Context` types here, so that it is easier
// to visually verify that they are serializing the same data with the
// same ordering.
//

//
// AST{Encoding|Decoding}Context::handleName()
//

void ASTEncodingContext::handleName(Name*& value)
{
    serialize(ASTSerializer(this), value->text);
}

void ASTDecodingContext::handleName(Name*& value)
{
    String text;
    serialize(ASTSerializer(this), text);
    value = _astBuilder->getNamePool()->getName(text);
}

//
// AST{Encoding|Decoding}Context::handleSourceLoc()
//

void ASTEncodingContext::handleSourceLoc(SourceLoc& value)
{
    ASTSerializer serializer(this);
    SLANG_SCOPED_SERIALIZER_OPTIONAL(serializer);
    if (_sourceLocWriter != nullptr)
    {
        auto rawValue = _sourceLocWriter->addSourceLoc(value);
        serialize(serializer, rawValue);
    }
}

void ASTDecodingContext::handleSourceLoc(SourceLoc& value)
{
    ASTSerializer serializer(this);
    SLANG_SCOPED_SERIALIZER_OPTIONAL(serializer);
    if (hasElements(serializer))
    {
        SerialSourceLocData::SourceLoc rawValue;
        serialize(serializer, rawValue);

        if (_sourceLocReader)
        {
            value = _sourceLocReader->getSourceLoc(rawValue);
        }
    }
}

//
// AST{Encoding|Decoding}Context::handleToken()
//

void ASTDecodingContext::handleToken(Token& value)
{
    ASTSerializer serializer(this);

    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.type);
    serialize(serializer, value.loc);

    serialize(serializer, value.flags);

    {
        SLANG_SCOPED_SERIALIZER_OPTIONAL(serializer);
        if (hasElements(serializer))
        {
            String content;
            serialize(serializer, content);

            // An important note here is that we cannot just
            // call `value.setContent(...)` and pass in an
            // `UnownedStringSlice` of `content`, because the
            // `Token` will not take ownership of its own
            // textual content.
            //
            // Instead, we need to get the text we just loaded
            // into something that the `Token` can refer info,
            // and the easiest way to accomplish that is to
            // represent the text using a `Name`.
            //
            Name* name = _astBuilder->getNamePool()->getName(content);
            value.setName(name);
        }
    }
}

void ASTEncodingContext::handleToken(Token& value)
{
    ASTSerializer serializer(this);

    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.type);
    serialize(serializer, value.loc);

    TokenFlags flags = TokenFlags(value.flags & ~TokenFlag::Name);
    serialize(serializer, flags);

    {
        SLANG_SCOPED_SERIALIZER_OPTIONAL(serializer);
        if (value.hasContent())
        {
            String content = value.getContent();
            serialize(serializer, content);
        }
    }
}

//
// AST{Encoding|Decoding}Context::handleASTNode()
//

void ASTEncodingContext::handleASTNode(NodeBase*& node)
{
    if (auto decl = as<Decl>(node))
    {
        if (auto importedFromModule = _findModuleDeclWasImportedFrom(decl))
        {
            if (decl == importedFromModule)
            {
                _writeImportedModule(importedFromModule);
                return;
            }
            else
            {
                _writeImportedDecl(decl, importedFromModule);
                return;
            }
        }
    }

    ASTSerializer serializer(this);

    if (auto val = as<Val>(node))
    {
        val = val->resolve();

        // On the reading side of things, sublcasses of `Val`
        // are deduplicated as part of creation, and will read the
        // operands out immediately, so we mirror that approach
        // on the writing side to make sure the code is consistent.
        //
        serialize(serializer, val->astNodeType);
        serialize(serializer, val->m_operands);
    }
    else
    {
        serialize(serializer, node->astNodeType);
        deferSerializeObjectContents(serializer, node);
    }
}

void ASTDecodingContext::handleASTNode(NodeBase*& outNode)
{
    ASTSerializer serializer(this);

    ASTNodeType typeTag;
    serialize(serializer, typeTag);
    switch (_getPseudoASTNodeType(typeTag))
    {
    default:
        break;

    case PseudoASTNodeType::ImportedModule:
        outNode = _readImportedModule();
        return;

    case PseudoASTNodeType::ImportedDecl:
        outNode = _readImportedDecl();
        return;
    }

    auto syntaxClass = SyntaxClass<NodeBase>(typeTag);
    if (syntaxClass.isSubClassOf<Val>())
    {
        // Subclasses of `Val` are deduplicated as part
        // of creation, so we need to read in their
        // operands before we can create them, rather
        // than allocating the object up front and
        // then deserializing its content into it later.

        ValNodeDesc desc;
        desc.type = syntaxClass;
        serialize(serializer, desc.operands);

        desc.init();

        auto node = _astBuilder->_getOrCreateImpl(std::move(desc));
        outNode = node;
    }
    else
    {
        auto node = syntaxClass.createInstance(_astBuilder);
        outNode = node;

        deferSerializeObjectContents(serializer, node);
    }
}

//
// AST{Encoding|Decoding}Context::handleASTNodeContents()
//

void ASTEncodingContext::handleASTNodeContents(NodeBase* node)
{
    ASTSerializer serializer(this);
    serializeASTNodeContents(serializer, node);
}

void ASTDecodingContext::handleASTNodeContents(NodeBase* node)
{
    ASTSerializer serializer(this);
    serializeASTNodeContents(serializer, node);

    _cleanUpASTNode(node);
}

//
// AST{Encoding|Decoding}Context::_{write|read}ImportedModule()
//

void ASTEncodingContext::_writeImportedModule(ModuleDecl* moduleDecl)
{
    ASTNodeType type = _getAsASTNodeType(PseudoASTNodeType::ImportedModule);
    auto moduleName = moduleDecl->getName();

    ASTSerializer serializer(this);
    serialize(serializer, type);
    serialize(serializer, moduleName);
}

ModuleDecl* ASTDecodingContext::_readImportedModule()
{
    ASTSerializer serializer(this);

    Name* moduleName = nullptr;
    serialize(serializer, moduleName);
    auto module = _linkage->findOrImportModule(moduleName, _requestingSourceLoc, _sink);
    if (!module)
    {
        SLANG_ABORT_COMPILATION("failed to load an imported module during AST deserialization");
    }
    return module->getModuleDecl();
}

//
// AST{Encoding|Decoding}Context::_{write|read}ImportedModule()
//

void ASTEncodingContext::_writeImportedDecl(Decl* decl, ModuleDecl* importedFromModuleDecl)
{
    ASTNodeType type = _getAsASTNodeType(PseudoASTNodeType::ImportedDecl);
    auto mangledName = getMangledName(getCurrentASTBuilder(), decl);

    ASTSerializer serializer(this);
    serialize(serializer, type);
    serialize(serializer, importedFromModuleDecl);
    serialize(serializer, mangledName);
}

NodeBase* ASTDecodingContext::_readImportedDecl()
{
    ASTSerializer serializer(this);

    ModuleDecl* importedFromModuleDecl = nullptr;
    String mangledName;

    serialize(serializer, importedFromModuleDecl);
    serialize(serializer, mangledName);

    auto importedFromModule = importedFromModuleDecl->module;
    if (!importedFromModule)
    {
        return nullptr;
    }

    auto importedDecl =
        importedFromModule->findExportFromMangledName(mangledName.getUnownedSlice());
    if (!importedDecl)
    {
        SLANG_ABORT_COMPILATION(
            "failed to load an imported declaration during AST deserialization");
    }
    return importedDecl;
}

//
// {write|read}SerializedModuleAST()
//

void writeSerializedModuleAST(
    RIFF::BuildCursor& cursor,
    ModuleDecl* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter)
{
    // TODO: we might want to have a more careful pass here,
    // where we only encode the public declarations.

    ASTEncodingContext context(cursor, moduleDecl, sourceLocWriter);
    serialize(ASTSerializer(&context), moduleDecl);
}

ModuleDecl* readSerializedModuleAST(
    Linkage* linkage,
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    RIFF::Chunk const* chunk,
    SerialSourceLocReader* sourceLocReader,
    SourceLoc requestingSourceLoc)
{
    ASTDecodingContext
        context(linkage, astBuilder, sink, chunk, sourceLocReader, requestingSourceLoc);

    ModuleDecl* moduleDecl = nullptr;
    serialize(ASTSerializer(&context), moduleDecl);
    return moduleDecl;
}

} // namespace Slang
