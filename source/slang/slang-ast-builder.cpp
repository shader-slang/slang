// slang-ast-builder.cpp
#include "slang-ast-builder.h"

#include "slang-check.h"
#include "slang-compiler.h"
#include "slang-syntax.h"

#include <assert.h>

namespace Slang
{

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SharedASTBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SharedASTBuilder::SharedASTBuilder(Session* session, RootASTBuilder* rootASTBuilder)
{
    m_namePool = session->getNamePool();

    // Save the associated session
    m_session = session;

    // The root AST builder is the one that owns this `SharedASTBuilder`.
    //
    m_astBuilder = rootASTBuilder;

    // Clear the built in types
    memset(m_builtinTypes, 0, sizeof(m_builtinTypes));

    // We can just iterate over the class pointers.
    // NOTE! That this adds the names of the abstract classes too(!)
    for (Index i = 0; i < Index(ASTNodeType::CountOf); ++i)
    {
        auto syntaxClass = SyntaxClass(ASTNodeType(i));
        if (!syntaxClass)
            continue;
        auto nameText = syntaxClass.getName();
        m_sliceToTypeMap.add(nameText, syntaxClass);
        Name* nameObj = m_namePool->getName(nameText);
        m_nameToTypeMap.add(nameObj, syntaxClass);
    }
}

SyntaxClass<> SharedASTBuilder::findSyntaxClass(const UnownedStringSlice& slice)
{
    SyntaxClass typeInfo;
    if (m_sliceToTypeMap.tryGetValue(slice, typeInfo))
    {
        return typeInfo;
    }
    return getSyntaxClass<NodeBase>();
}

SyntaxClass<NodeBase> SharedASTBuilder::findSyntaxClass(Name* name)
{
    SyntaxClass<NodeBase> typeInfo;
    if (m_nameToTypeMap.tryGetValue(name, typeInfo))
    {
        return typeInfo;
    }
    return getSyntaxClass<NodeBase>();
}

Type* SharedASTBuilder::getStringType()
{
    if (!m_stringType)
    {
        auto stringTypeDecl = findMagicDecl("StringType");
        m_stringType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(stringTypeDecl));
    }
    return m_stringType;
}

Type* SharedASTBuilder::getNativeStringType()
{
    if (!m_nativeStringType)
    {
        auto nativeStringTypeDecl = findMagicDecl("NativeStringType");
        m_nativeStringType =
            DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(nativeStringTypeDecl));
    }
    return m_nativeStringType;
}

Type* SharedASTBuilder::getEnumTypeType()
{
    if (!m_enumTypeType)
    {
        auto enumTypeTypeDecl = findMagicDecl("EnumTypeType");
        m_enumTypeType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(enumTypeTypeDecl));
    }
    return m_enumTypeType;
}

Type* SharedASTBuilder::getDynamicType()
{
    if (!m_dynamicType)
    {
        auto dynamicTypeDecl = findMagicDecl("DynamicType");
        m_dynamicType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(dynamicTypeDecl));
    }
    return m_dynamicType;
}

Type* SharedASTBuilder::getNullPtrType()
{
    if (!m_nullPtrType)
    {
        auto nullPtrTypeDecl = findMagicDecl("NullPtrType");
        m_nullPtrType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(nullPtrTypeDecl));
    }
    return m_nullPtrType;
}

Type* SharedASTBuilder::getNoneType()
{
    if (!m_noneType)
    {
        auto noneTypeDecl = findMagicDecl("NoneType");
        m_noneType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(noneTypeDecl));
    }
    return m_noneType;
}

Type* SharedASTBuilder::getDiffInterfaceType()
{
    if (!m_diffInterfaceType)
    {
        auto decl = findMagicDecl("DifferentiableType");
        m_diffInterfaceType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(decl));
    }
    return m_diffInterfaceType;
}

Type* SharedASTBuilder::getIBufferDataLayoutType()
{
    if (!m_IBufferDataLayoutType)
    {
        auto decl = findMagicDecl("IBufferDataLayoutType");
        m_IBufferDataLayoutType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(decl));
    }
    return m_IBufferDataLayoutType;
}

Type* SharedASTBuilder::getErrorType()
{
    if (!m_errorType)
        m_errorType = m_astBuilder->getOrCreate<ErrorType>();
    return m_errorType;
}
Type* SharedASTBuilder::getBottomType()
{
    if (!m_bottomType)
        m_bottomType = m_astBuilder->getOrCreate<BottomType>();
    return m_bottomType;
}
Type* SharedASTBuilder::getInitializerListType()
{
    if (!m_initializerListType)
        m_initializerListType = m_astBuilder->getOrCreate<InitializerListType>();
    return m_initializerListType;
}
Type* SharedASTBuilder::getOverloadedType()
{
    if (!m_overloadedType)
        m_overloadedType = m_astBuilder->getOrCreate<OverloadGroupType>();
    return m_overloadedType;
}

void SharedASTBuilder::registerBuiltinDecl(Decl* decl, BuiltinTypeModifier* modifier)
{
    auto type = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(decl));
    m_builtinTypes[Index(modifier->tag)] = type;
}

void SharedASTBuilder::registerBuiltinRequirementDecl(
    Decl* decl,
    BuiltinRequirementModifier* modifier)
{
    m_builtinRequirementDecls[modifier->kind] = decl;
}

void SharedASTBuilder::registerMagicDecl(Decl* decl, MagicTypeModifier* modifier)
{
    // In some cases the modifier will have been applied to the
    // "inner" declaration of a `GenericDecl`, but what we
    // actually want to register is the generic itself.
    //
    auto declToRegister = decl;
    if (auto genericDecl = as<GenericDecl>(decl->parentDecl))
        declToRegister = genericDecl;

    m_magicDecls[modifier->magicName] = declToRegister;
}

Decl* SharedASTBuilder::findMagicDecl(const String& name)
{
    return m_magicDecls.getValue(name);
}

Decl* SharedASTBuilder::tryFindMagicDecl(const String& name)
{
    auto d = m_magicDecls.tryGetValue(name);
    return d ? *d : nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/// Default block size of 2MB.
static const size_t kASTBuilderMemoryArenaBlockSize = 2 * 1024 * 1024;

ASTBuilder::ASTBuilder(ASTBuilder* parent, String const& debugName)
    : m_parent(parent), m_name(debugName), m_arena(kASTBuilderMemoryArenaBlockSize)
{
    SLANG_ASSERT(parent);
    auto sharedASTBuilder = parent->getSharedASTBuilder();
    SLANG_ASSERT(sharedASTBuilder);

    m_depth = parent->m_depth + 1;

    m_sharedASTBuilder = sharedASTBuilder;
    m_id = sharedASTBuilder->m_id++;
}

ASTBuilder::ASTBuilder()
    : m_arena(kASTBuilderMemoryArenaBlockSize)
{
}

RootASTBuilder::RootASTBuilder(Session* globalSession)
    : m_sharedASTBuilderStorage(globalSession, this)
{
    m_sharedASTBuilder = &m_sharedASTBuilderStorage;
    m_name = "RootASTBuilder";
}

ASTBuilder::~ASTBuilder()
{
    for (NodeBase* node : m_dtorNodes)
    {
        auto nodeClass = node->getClass();
        nodeClass.destructInstance(node);
    }
    incrementEpoch();
}

Val* ASTBuilder::_getOrCreateImplSlowPath(ValNodeDesc&& desc)
{
    // The most important thing we need to determine here
    // is whether the node described by `desc` would be
    // created using the arena of this `ASTBuilder`, or
    // one of its ancestors (and if so, which one...).
    //
    ASTBuilder* astBuilderToUse = _findAppropriateASTBuilderForVal(desc);

    // Once we've identified the right level of the hierarchy,
    // we can check the cache at that level and create
    // the node if it doesn't already exist.
    //
    Val* valNode = astBuilderToUse->_getOrCreateValDirectly(std::move(desc));

    // If the chosen `astBuilderToUse` was `this`, then the
    // call to `_getOrCreateValDirectly` will have updated
    // `m_cachedNodes` already.
    //
    // If the node was created using a different builder,
    // which is an ancestor than this one (which would mean
    // its depth is lower), then we can also update our
    // own cache to match.
    //
    if (astBuilderToUse->m_depth < this->m_depth)
    {
        // Our approach to caching assumes that we cannot
        // mix-and-match AST nodes from builders that aren't
        // in some kind of ancestor/descendent relationship.
        // Thus, if the builder that was chosen is less deep
        // than `this`, we expect that to be because it is
        // an ancestor.
        //
        SLANG_ASSERT(this->isDescendentOf(astBuilderToUse));

        m_cachedNodes.add(ValKey(valNode), valNode);
    }
    //
    // Note that we do *not* want to update our cache in
    // the case where the chosen builder has higher depth
    // then `this`, because `this` could outlive the chosen
    // builder, and we don't want to be left with
    // garbage pointers sitting in the cache.
    //
    // We also don't consider that case to be an error,
    // because it is reasonable for code to do things like
    // construct a specialized decl-ref for `Foo<Bar>` using
    // the builder associated with declaration `Foo`, even
    // when specializing to a type `Bar` that comes from a
    // deeper/child builder.

    return valNode;
}

ASTBuilder* ASTBuilder::_findAppropriateASTBuilderForVal(ValNodeDesc const& desc)
{
    // AST builders are arranged in a hierarchy, where a child builder
    // can see nodes cached in its ancestors, but not vice versa.
    //
    // We basically want to allocate a given `Val` as far down
    // the hierarchy as we can (away from the root), so that the
    // lifetime of those allocations can be narrowly scoped. However,
    // we also need to ensure that `Val`s are cached far enough
    // *up* the hierarchy that deduplication is possible, and that
    // we can be sure a `Val` lives at least as long as each of
    // its operands.
    //
    // Our approach to the caching problem relies on a key
    // constraint, that the `ASTBuilder` used for a `_getOrCreateImpl()`
    // operation and all of the `ASTBuilder`s used to create the
    // nodes referenced as operands in the `desc` must be part
    // of a single path of parent links in the hierarchy.
    // Put another way: for any two `ASTBuilder`s involved in the
    // creation of the node or its operands, they must be in some
    // kind of ancestor/descendent relationship.
    //
    // Given this constraint, we can determine that the `Val` should
    // be allocated and cached on the *deepest* AST builder
    // from among the operands (or on the root AST builder in the
    // case where there are no operands).
    //
    // We thus initialize our variable to the *shallowest* builder,
    // which is the one we'll use if there are no operands.
    //
    ASTBuilder* deepestBuilder = getSharedASTBuilder()->getInnerASTBuilder();
    for (auto const& operand : desc.operands)
    {
        // We are only interested in operands that reference
        // an AST node, so we will skip over all others.
        //
        switch (operand.kind)
        {
        default:
            continue;

        case ValNodeOperandKind::ASTNode:
        case ValNodeOperandKind::ValNode:
            break;
        }

        // We now know that the operand is represented
        // as an AST node, but we need to skip over
        // null operands because they aren't relevant
        // to picking the right AST builder to use.
        //
        NodeBase* node = operand.values.nodeOperand;
        if (!node)
            continue;

        // Once we have an AST node worth looking at,
        // we find the AST builder responsible for
        // allocating that node.
        //
        ASTBuilder* nodeBuilder = node->getASTBuilder();
        SLANG_ASSERT(nodeBuilder);

        // The approach we are taking here relies on all
        // the AST builders involved being part of a single
        // path in the hierarchy, so we will do a minimal
        // amount of validation in debug builds to ensure
        // that each of the node builders for the operands
        // is in some kind of ancestor/descendent relationship
        // with the builder being used to make the request.
        //
        SLANG_ASSERT(nodeBuilder->isDescendentOf(this) || this->isDescendentOf(nodeBuilder));

        // If the builder we are looking at is deeper than the
        // deepest builder we've seen previously, then we update
        // our candiate for the deepest builder.
        //
        if (nodeBuilder->m_depth > deepestBuilder->m_depth)
            deepestBuilder = nodeBuilder;
    }

    //
    // At the end of that loop, we have a maximally-deep builder,
    // and because we require all the builders to come from
    // a single path in the hierarchy, that builder is also
    // uniquely determined (a maximum rather than just maximal).
    //

    return deepestBuilder;
}

bool ASTBuilder::isDescendentOf(ASTBuilder* ancestor)
{
    SLANG_ASSERT(ancestor);

    auto builder = this;
    while (builder)
    {
        if (builder == ancestor)
            return true;
        builder = builder->m_parent;
    }
    return false;
}


Val* ASTBuilder::_getOrCreateValDirectly(ValNodeDesc&& desc)
{
    // This operation should only be called if `this`
    // was determined to be the appropriate AST builder
    // to use when allocating/caching a `Val` based on `desc`.
    //
    SLANG_ASSERT(this == _findAppropriateASTBuilderForVal(desc));

    // We start by checking the cache. This might have
    // already been done as part of `_getOrCreateImpl()`,
    // but it is also possible that the `_getOrCreateImpl()`
    // call was made on a descendent `ASTBuilder` and its
    // cache might not (yet) contain the given node.
    //
    if (auto found = m_cachedNodes.tryGetValue(desc))
        return *found;

    // If we don't have a cache hit at this level,
    // then we just need to create the node and
    // update our cache.
    //
    auto node = as<Val>(desc.type.createInstance(this));
    SLANG_ASSERT(node);
    for (auto& operand : desc.operands)
        node->m_operands.add(operand);

    m_cachedNodes.add(ValKey(node), _Move(node));

    return node;
}

Index ASTBuilder::getEpoch()
{
    return getSharedASTBuilder()->m_session->m_epochId;
}

void ASTBuilder::incrementEpoch()
{
    getSharedASTBuilder()->m_session->m_epochId++;
}

NodeBase* ASTBuilder::createByNodeType(ASTNodeType nodeType)
{
    auto syntaxClass = SyntaxClass<NodeBase>(nodeType);
    return syntaxClass.createInstance(this);
}

Type* ASTBuilder::getSpecializedBuiltinType(Type* typeParam, char const* magicTypeName)
{
    auto declRef = getBuiltinDeclRef(magicTypeName, typeParam);
    auto rsType = DeclRefType::create(this, declRef);
    return rsType;
}

Type* ASTBuilder::getSpecializedBuiltinType(ArrayView<Val*> genericArgs, const char* magicTypeName)
{
    auto declRef = getBuiltinDeclRef(magicTypeName, genericArgs);
    auto rsType = DeclRefType::create(this, declRef);
    return rsType;
}

Type* ASTBuilder::getForwardDiffFuncInterfaceType(Type* baseType, Witness* typeInfoWitness)
{
    auto decl = getSharedASTBuilder()->findMagicDecl("ForwardDiffFuncInterfaceType");
    return DeclRefType::create(
        this,
        this->getGenericAppDeclRef(
            DeclRef<GenericDecl>(decl->getDefaultDeclRef()),
            makeConstArrayView({as<Val>(baseType), as<Val>(typeInfoWitness)})));
}

Type* ASTBuilder::getBackwardDiffFuncInterfaceType(Type* baseType, Witness* typeInfoWitness)
{
    auto decl = getSharedASTBuilder()->findMagicDecl("BwdDiffFuncInterfaceType");
    return DeclRefType::create(
        this,
        this->getGenericAppDeclRef(
            DeclRef<GenericDecl>(decl->getDefaultDeclRef()),
            makeConstArrayView({as<Val>(baseType), as<Val>(typeInfoWitness)})));
}

Type* ASTBuilder::getBwdCallableBaseType(Type* baseType, Witness* typeInfoWitness)
{
    auto decl = getSharedASTBuilder()->findMagicDecl("BwdCallableBaseType");
    return DeclRefType::create(
        this,
        this->getGenericAppDeclRef(
            DeclRef<GenericDecl>(decl->getDefaultDeclRef()),
            makeConstArrayView({as<Val>(baseType), as<Val>(typeInfoWitness)})));
}

Type* ASTBuilder::getMagicEnumType(const char* magicEnumName)
{
    auto& cache = getSharedASTBuilder()->m_magicEnumTypes;
    Type* res = nullptr;
    if (!cache.tryGetValue(magicEnumName, res))
    {
        res = getSpecializedBuiltinType({}, magicEnumName);
        cache.add(magicEnumName, res);
    }
    return res;
}

PtrType* ASTBuilder::getPtrType(
    Type* valueType,
    Val* accessQualifier,
    Val* addrSpace,
    Type* dataLayout)
{
    return dynamicCast<PtrType>(
        getPtrType(valueType, accessQualifier, addrSpace, dataLayout, "PtrType"));
}

PtrType* ASTBuilder::getPtrType(
    Type* valueType,
    AccessQualifier accessQualifier,
    AddressSpace addrSpace,
    Type* dataLayout)
{
    return dynamicCast<PtrType>(
        getPtrType(valueType, accessQualifier, addrSpace, dataLayout, "PtrType"));
}

Type* ASTBuilder::getDefaultLayoutType()
{
    return getSpecializedBuiltinType({}, "DefaultDataLayoutType");
}

Type* ASTBuilder::getDefaultPushConstantLayoutType()
{
    return getSpecializedBuiltinType({}, "DefaultPushConstantDataLayoutType");
}

Type* ASTBuilder::getStd140LayoutType()
{
    return getSpecializedBuiltinType({}, "Std140DataLayoutType");
}
Type* ASTBuilder::getStd430LayoutType()
{
    return getSpecializedBuiltinType({}, "Std430DataLayoutType");
}
Type* ASTBuilder::getScalarLayoutType()
{
    return getSpecializedBuiltinType({}, "ScalarDataLayoutType");
}

// Construct the type `Out<valueType>`
OutType* ASTBuilder::getOutParamType(Type* valueType)
{
    return dynamicCast<OutType>(getPtrType(valueType, "OutParamType"));
}

BorrowInOutParamType* ASTBuilder::getBorrowInOutParamType(Type* valueType)
{
    return dynamicCast<BorrowInOutParamType>(getPtrType(valueType, "BorrowInOutParamType"));
}

RefParamType* ASTBuilder::getRefParamType(Type* valueType)
{
    return dynamicCast<RefParamType>(getPtrType(valueType, "RefParamType"));
}

BorrowInParamType* ASTBuilder::getConstRefParamType(Type* valueType)
{
    return dynamicCast<BorrowInParamType>(getPtrType(valueType, "BorrowInParamType"));
}

ExplicitRefType* ASTBuilder::getExplicitRefType(Type* valueType)
{
    return dynamicCast<ExplicitRefType>(getPtrType(valueType, "ExplicitRefType"));
}

OptionalType* ASTBuilder::getOptionalType(Type* valueType)
{
    auto rsType = getSpecializedBuiltinType(valueType, "OptionalType");
    return as<OptionalType>(rsType);
}

PtrTypeBase* ASTBuilder::getPtrType(Type* valueType, char const* ptrTypeName)
{
    return as<PtrTypeBase>(getSpecializedBuiltinType(valueType, ptrTypeName));
}

PtrTypeBase* ASTBuilder::getPtrType(
    Type* valueType,
    Val* accessQualifier,
    Val* addrSpace,
    Type* dataLayoutType,
    char const* ptrTypeName)
{
    auto declRef = as<DeclRefType>(dataLayoutType)->getDeclRef();
    auto containerDeclRef = declRef.as<ContainerDecl>();
    auto dataLayoutInterface = m_sharedASTBuilder->getIBufferDataLayoutType();
    DeclaredSubtypeWitness* subtypeWitness = nullptr;

    for (auto typeConstraintDeclRef :
         containerDeclRef.getDecl()->getDirectMemberDeclsOfType<TypeConstraintDecl>())
    {
        if (typeConstraintDeclRef->getSup().type == dataLayoutInterface)
        {
            subtypeWitness = getDeclaredSubtypeWitness(
                dataLayoutType,
                m_sharedASTBuilder->getIBufferDataLayoutType(),
                typeConstraintDeclRef);
        }
    }

    SLANG_RELEASE_ASSERT(subtypeWitness);

    Val* args[] = {valueType, accessQualifier, addrSpace, dataLayoutType, subtypeWitness};
    return as<PtrTypeBase>(getSpecializedBuiltinType(makeArrayView(args), ptrTypeName));
}

PtrTypeBase* ASTBuilder::getPtrType(
    Type* valueType,
    AccessQualifier accessQualifier,
    AddressSpace addrSpace,
    Type* dataLayoutType,
    char const* ptrTypeName)
{
    Type* typeOfAccessQualifier = getMagicEnumType("AccessQualifier");
    Type* typeOfAddressSpace = getMagicEnumType("AddressSpace");
    return as<PtrTypeBase>(getPtrType(
        valueType,
        getIntVal(typeOfAccessQualifier, (IntegerLiteralValue)accessQualifier),
        getIntVal(typeOfAddressSpace, (IntegerLiteralValue)addrSpace),
        dataLayoutType,
        ptrTypeName));
}

ArrayExpressionType* ASTBuilder::getArrayType(Type* elementType, IntVal* elementCount)
{
    if (!elementCount)
        elementCount = getIntVal(getIntType(), kUnsizedArrayMagicLength);

    if (elementCount->getType() != getIntType())
    {
        // Canonicalize constant elementCount to int.
        if (auto elementCountConstantInt = as<ConstantIntVal>(elementCount))
        {
            elementCount = getIntVal(getIntType(), elementCountConstantInt->getValue());
        }
        else
        {
            elementCount = getTypeCastIntVal(getIntType(), elementCount);
        }
    }
    Val* args[] = {elementType, elementCount};
    return as<ArrayExpressionType>(
        getSpecializedBuiltinType(makeArrayView(args), "ArrayExpressionType"));
}

ConstantBufferType* ASTBuilder::getConstantBufferType(
    Type* elementType,
    Type* layoutType,
    Val* layoutWitness)
{
    Val* args[] = {elementType, layoutType, layoutWitness};

    return as<ConstantBufferType>(
        getSpecializedBuiltinType(makeArrayView(args), "ConstantBufferType"));
}

ParameterBlockType* ASTBuilder::getParameterBlockType(Type* elementType)
{
    return as<ParameterBlockType>(getSpecializedBuiltinType(elementType, "ParameterBlockType"));
}

HLSLStructuredBufferType* ASTBuilder::getStructuredBufferType(Type* elementType)
{
    return as<HLSLStructuredBufferType>(
        getSpecializedBuiltinType(elementType, "HLSLStructuredBufferType"));
}

HLSLRWStructuredBufferType* ASTBuilder::getRWStructuredBufferType(Type* elementType)
{
    return as<HLSLRWStructuredBufferType>(
        getSpecializedBuiltinType(elementType, "HLSLRWStructuredBufferType"));
}

SamplerStateType* ASTBuilder::getSamplerStateType()
{
    return as<SamplerStateType>(getSpecializedBuiltinType(nullptr, "HLSLStructuredBufferType"));
}

VectorExpressionType* ASTBuilder::getVectorType(Type* elementType, IntVal* elementCount)
{
    // Canonicalize constant elementCount to int.
    if (auto elementCountConstantInt = as<ConstantIntVal>(elementCount))
    {
        elementCount = getIntVal(getIntType(), elementCountConstantInt->getValue());
    }
    Val* args[] = {elementType, elementCount};
    return as<VectorExpressionType>(
        getSpecializedBuiltinType(makeArrayView(args), "VectorExpressionType"));
}

MatrixExpressionType* ASTBuilder::getMatrixType(
    Type* elementType,
    IntVal* rowCount,
    IntVal* colCount,
    IntVal* layout)
{
    // Canonicalize constant size arguments to int.
    if (auto rowCountConstantInt = as<ConstantIntVal>(rowCount))
    {
        rowCount = getIntVal(getIntType(), rowCountConstantInt->getValue());
    }
    if (auto colCountConstantInt = as<ConstantIntVal>(colCount))
    {
        colCount = getIntVal(getIntType(), colCountConstantInt->getValue());
    }
    Val* args[] = {elementType, rowCount, colCount, layout};
    return as<MatrixExpressionType>(
        getSpecializedBuiltinType(makeArrayView(args), "MatrixExpressionType"));
}

DifferentialPairType* ASTBuilder::getDifferentialPairType(Type* valueType, Witness* diffTypeWitness)
{
    Val* args[] = {valueType, diffTypeWitness};
    return as<DifferentialPairType>(
        getSpecializedBuiltinType(makeArrayView(args), "DifferentialPairType"));
}

DeclRef<InterfaceDecl> ASTBuilder::getDiffTypeInfoInterfaceDecl()
{
    DeclRef<InterfaceDecl> declRef =
        DeclRef<InterfaceDecl>(getBuiltinDeclRef("DiffTypeInfoInterfaceType", nullptr));
    return declRef;
}

Type* ASTBuilder::getDiffTypeInfoInterfaceType()
{
    return DeclRefType::create(this, getDiffTypeInfoInterfaceDecl());
}

DifferentialPtrPairType* ASTBuilder::getDifferentialPtrPairType(
    Type* valueType,
    Witness* diffRefTypeWitness)
{
    Val* args[] = {valueType, diffRefTypeWitness};
    return as<DifferentialPtrPairType>(
        getSpecializedBuiltinType(makeArrayView(args), "DifferentialPtrPairType"));
}

DeclRef<InterfaceDecl> ASTBuilder::getDifferentiableInterfaceDecl()
{
    DeclRef<InterfaceDecl> declRef =
        DeclRef<InterfaceDecl>(getBuiltinDeclRef("DifferentiableType", nullptr));
    return declRef;
}

DeclRef<InterfaceDecl> ASTBuilder::getFunctionBaseInterfaceDecl()
{
    DeclRef<InterfaceDecl> declRef =
        DeclRef<InterfaceDecl>(getBuiltinDeclRef("FunctionBaseType", nullptr));
    return declRef;
}

DeclRef<InterfaceDecl> ASTBuilder::getDifferentiableRefInterfaceDecl()
{
    DeclRef<InterfaceDecl> declRef =
        DeclRef<InterfaceDecl>(getBuiltinDeclRef("DifferentiablePtrType", nullptr));
    return declRef;
}

bool ASTBuilder::isDifferentiableInterfaceAvailable()
{
    return (m_sharedASTBuilder->tryFindMagicDecl("DifferentiableType") != nullptr);
}

DeclRef<InterfaceDecl> ASTBuilder::getDefaultInitializableTypeInterfaceDecl()
{
    DeclRef<InterfaceDecl> declRef =
        DeclRef<InterfaceDecl>(getBuiltinDeclRef("DefaultInitializableType", nullptr));
    return declRef;
}
Type* ASTBuilder::getDefaultInitializableType()
{
    return DeclRefType::create(
        m_sharedASTBuilder->m_astBuilder,
        getDefaultInitializableTypeInterfaceDecl());
}

MeshOutputType* ASTBuilder::getMeshOutputTypeFromModifier(
    HLSLMeshShaderOutputModifier* modifier,
    Type* elementType,
    IntVal* maxElementCount)
{
    SLANG_ASSERT(modifier);
    SLANG_ASSERT(elementType);
    SLANG_ASSERT(maxElementCount);

    const char* declName = as<HLSLVerticesModifier>(modifier)  ? "VerticesType"
                           : as<HLSLIndicesModifier>(modifier) ? "IndicesType"
                           : as<HLSLPrimitivesModifier>(modifier)
                               ? "PrimitivesType"
                               : (SLANG_UNEXPECTED("Unhandled mesh output modifier"), nullptr);

    Val* args[] = {elementType, maxElementCount};
    return as<MeshOutputType>(getSpecializedBuiltinType(makeArrayView(args), declName));
}

Type* ASTBuilder::getDifferentiableInterfaceType()
{
    return DeclRefType::create(this, getDifferentiableInterfaceDecl());
}

Type* ASTBuilder::getFunctionBaseType()
{
    return DeclRefType::create(this, getFunctionBaseInterfaceDecl());
}

Type* ASTBuilder::getDifferentiableRefInterfaceType()
{
    return DeclRefType::create(this, getDifferentiableRefInterfaceDecl());
}

DeclRef<Decl> ASTBuilder::getBuiltinDeclRef(const char* builtinMagicTypeName, Val* genericArg)
{
    auto decl = m_sharedASTBuilder->findMagicDecl(builtinMagicTypeName);
    if (auto genericDecl = as<GenericDecl>(decl))
    {
        auto declRef =
            getGenericAppDeclRef(makeDeclRef(genericDecl), makeConstArrayViewSingle(genericArg));
        return declRef;
    }
    else
    {
        SLANG_ASSERT(!genericArg);
    }
    return makeDeclRef(decl);
}

DeclRef<Decl> ASTBuilder::getBuiltinDeclRef(
    const char* builtinMagicTypeName,
    ArrayView<Val*> genericArgs)
{
    auto decl = m_sharedASTBuilder->findMagicDecl(builtinMagicTypeName);
    if (auto genericDecl = as<GenericDecl>(decl))
    {
        auto declRef = getGenericAppDeclRef(makeDeclRef(genericDecl), genericArgs);
        return declRef;
    }
    else
    {
        SLANG_ASSERT(!decl && !genericArgs.getCount());
    }
    return makeDeclRef(decl);
}

Type* ASTBuilder::getAndType(Type* left, Type* right)
{
    auto type = getOrCreate<AndType>(left, right);
    return type;
}

Type* ASTBuilder::getModifiedType(Type* base, Count modifierCount, Val* const* modifiers)
{
    auto type = getOrCreate<ModifiedType>(base, makeArrayView((Val**)modifiers, modifierCount));
    return type;
}

Val* ASTBuilder::getUNormModifierVal()
{
    return getOrCreate<UNormModifierVal>();
}

Val* ASTBuilder::getSNormModifierVal()
{
    return getOrCreate<SNormModifierVal>();
}

Val* ASTBuilder::getNoDiffModifierVal()
{
    return getOrCreate<NoDiffModifierVal>();
}

UIntSetVal* ASTBuilder::getUIntSetVal(const UIntSet& uintSet)
{
    // Convert UIntSet buffer to list of ConstantIntVals
    List<ConstantIntVal*> bitmasks;
    const auto& buffer = uintSet.getBuffer();
    bitmasks.reserve(buffer.getCount());

    for (Index i = 0; i < buffer.getCount(); i++)
    {
        auto element = buffer[i];
        auto bitmask = getIntVal(getUInt64Type(), element);
        bitmasks.add(bitmask);
    }

    return getOrCreate<UIntSetVal>(bitmasks);
}

CapabilitySetVal* ASTBuilder::getCapabilitySetVal(CapabilityName capability)
{
    return CapabilitySet{capability}.freeze(this);
}

FuncType* ASTBuilder::getFuncType(ArrayView<Type*> parameters, Type* result, Type* errorType)
{
    if (!errorType)
        errorType = getOrCreate<BottomType>();
    return getOrCreate<FuncType>(parameters, result, errorType);
}

TupleType* ASTBuilder::getTupleType(ArrayView<Type*> types)
{
    // The canonical form of a tuple type is always a DeclRefType(GenAppDeclRef(TupleDecl,
    // ConcreteTypePack(types...))). If `types` is already a single ConcreteTypePack, then we
    // can use that directly.
    if (types.getCount() == 1)
    {
        if (isTypePack(types[0]))
        {
            return as<TupleType>(getSpecializedBuiltinType(types[0], "TupleType"));
        }
    }

    // Otherwise, we need to create a ConcreteTypePack to hold the types.
    auto typePack = getTypePack(types);
    return as<TupleType>(getSpecializedBuiltinType(typePack, "TupleType"));
}

TypeType* ASTBuilder::getTypeType(Type* type)
{
    return getOrCreate<TypeType>(type);
}

Type* ASTBuilder::getEachType(Type* baseType)
{
    // each expand T ==> T
    if (auto expandType = as<ExpandType>(baseType))
    {
        return expandType->getPatternType();
    }

    // each Tuple<X> ==> each X, because we know that Tuple type must be in the form of
    // Tuple<ConcreteTypePack<...>>.
    if (auto tupleType = as<TupleType>(baseType))
    {
        return getEachType(tupleType->getTypePack());
    }
    SLANG_ASSERT(!as<EachType>(baseType));
    return getOrCreate<EachType>(baseType);
}

Type* ASTBuilder::getExpandType(Type* pattern, ArrayView<Val*> capturedPacks)
{
    // expand each T ==> T
    if (auto eachType = as<EachType>(pattern))
    {
        return eachType->getElementType();
    }
    return getOrCreate<ExpandType>(pattern, capturedPacks);
}

Type* ASTBuilder::getPackBranchType(Val* packOperand, Type* emptyType, Type* nonEmptyType)
{
    if (auto tupleType = as<TupleType>(packOperand))
        packOperand = tupleType->getTypePack();

    if (emptyType == nonEmptyType)
        return emptyType;

    switch (getKnownPackCardinality(packOperand))
    {
    case VariadicPackCardinality::Empty:
        return emptyType;
    case VariadicPackCardinality::NonEmpty:
        return nonEmptyType;
    default:
        break;
    }

    return getOrCreate<PackBranchType>(packOperand, emptyType, nonEmptyType);
}

static bool _containsEachType(Type* type)
{
    if (!type)
        return false;
    if (as<EachType>(type))
        return true;
    for (Index i = 0; i < type->getOperandCount(); ++i)
    {
        if (auto operandType = as<Type>(type->getOperand(i)))
        {
            if (_containsEachType(operandType))
                return true;
        }
    }
    return false;
}

Type* ASTBuilder::getFirstElement(Type* basePack)
{
    if (auto tupleType = as<TupleType>(basePack))
        return getFirstElement(tupleType->getTypePack());

    if (auto expandType = as<ExpandType>(basePack))
    {
        auto patternType = expandType->getPatternType();
        if (!_containsEachType(patternType))
            return patternType;
    }

    if (auto typePack = as<ConcreteTypePack>(basePack))
    {
        if (typePack->getTypeCount() > 0)
            return typePack->getElementType(0);
    }

    return getOrCreate<FirstPackElementType>(basePack);
}

Type* ASTBuilder::getLastElement(Type* basePack)
{
    if (auto tupleType = as<TupleType>(basePack))
        return getLastElement(tupleType->getTypePack());

    if (auto expandType = as<ExpandType>(basePack))
    {
        auto patternType = expandType->getPatternType();
        if (!_containsEachType(patternType))
            return patternType;
    }

    if (auto typePack = as<ConcreteTypePack>(basePack))
    {
        if (typePack->getTypeCount() > 0)
            return typePack->getElementType(typePack->getTypeCount() - 1);
    }

    return getOrCreate<LastPackElementType>(basePack);
}

Type* ASTBuilder::getTrimFirstPack(Type* basePack)
{
    if (auto tupleType = as<TupleType>(basePack))
    {
        Type* trimmedPack = getTrimFirstPack(tupleType->getTypePack());
        return getTupleType(makeArrayView(&trimmedPack, 1));
    }

    if (auto typePack = as<ConcreteTypePack>(basePack))
    {
        ShortList<Type*> trimmedTypes;
        for (Index i = 1; i < typePack->getTypeCount(); i++)
            trimmedTypes.add(typePack->getElementType(i));
        return getTypePack(trimmedTypes.getArrayView().arrayView);
    }

    return getOrCreate<TrimFirstTypePack>(basePack);
}

Type* ASTBuilder::getTrimLastPack(Type* basePack)
{
    if (auto tupleType = as<TupleType>(basePack))
    {
        Type* trimmedPack = getTrimLastPack(tupleType->getTypePack());
        return getTupleType(makeArrayView(&trimmedPack, 1));
    }

    if (auto typePack = as<ConcreteTypePack>(basePack))
    {
        ShortList<Type*> trimmedTypes;
        for (Index i = 0; i + 1 < typePack->getTypeCount(); i++)
            trimmedTypes.add(typePack->getElementType(i));
        return getTypePack(trimmedTypes.getArrayView().arrayView);
    }

    return getOrCreate<TrimLastTypePack>(basePack);
}

void flattenTypeList(ShortList<Type*>& flattenedList, Type* type)
{
    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); i++)
            flattenTypeList(flattenedList, typePack->getElementType(i));
    }
    else
    {
        flattenedList.add(type);
    }
}

ConcreteTypePack* ASTBuilder::getTypePack(ArrayView<Type*> types)
{
    // Flatten all type packs in the type list.
    ShortList<Type*> flattenedTypes;
    for (auto type : types)
        flattenTypeList(flattenedTypes, type);
    return getOrCreate<ConcreteTypePack>(flattenedTypes.getArrayView().arrayView);
}

static void flattenIntValList(ShortList<IntVal*>& flattenedList, IntVal* val)
{
    if (auto valPack = as<ConcreteIntValPack>(val))
    {
        for (Index i = 0; i < valPack->getCount(); i++)
            flattenIntValList(flattenedList, valPack->getElement(i));
    }
    else
    {
        flattenedList.add(val);
    }
}

static Type* _getIntPackTypeForVal(ASTBuilder* astBuilder, Val* pack)
{
    if (auto intVal = as<IntVal>(pack))
    {
        if (auto type = as<Type>(intVal->getType()))
            return type;
    }
    return astBuilder->getOrCreate<ValuePackType>(astBuilder->getIntType());
}

static Type* _getIntPackTypeForVals(ASTBuilder* astBuilder, Val* pack0, Val* pack1 = nullptr)
{
    for (auto pack : {pack0, pack1})
    {
        if (auto intVal = as<IntVal>(pack))
        {
            if (auto valuePackType = as<ValuePackType>(intVal->getType()))
                return valuePackType;
        }
    }

    if (auto intVal = as<IntVal>(pack0))
    {
        if (auto type = as<Type>(intVal->getType()))
            return type;
    }
    if (pack1)
        return _getIntPackTypeForVal(astBuilder, pack1);
    return astBuilder->getOrCreate<ValuePackType>(astBuilder->getIntType());
}

ConcreteIntValPack* ASTBuilder::getIntValPack(ArrayView<IntVal*> vals)
{
    ShortList<IntVal*> flattenedVals;
    for (auto val : vals)
        flattenIntValList(flattenedVals, val);
    Type* elementType = getIntType();
    if (flattenedVals.getCount() > 0 && flattenedVals[0]->getType())
        elementType = flattenedVals[0]->getType();
    auto packType = getOrCreate<ValuePackType>(elementType);
    return getOrCreate<ConcreteIntValPack>(packType, flattenedVals.getArrayView().arrayView);
}

IntVal* ASTBuilder::getEachIntVal(Type* elementType, Val* basePack)
{
    // each expand P ==> P (the pattern val)
    if (auto expandPack = as<ExpandIntValPack>(basePack))
    {
        auto result = as<IntVal>(expandPack->getPatternVal());
        SLANG_ASSERT(result);
        return result;
    }
    return getOrCreate<EachIntVal>(elementType, basePack);
}

Val* ASTBuilder::getExpandIntValPack(Val* patternVal, ArrayView<Val*> capturedPacks)
{
    // expand each B ==> B (fold identity expansion)
    if (auto eachVal = as<EachIntVal>(patternVal))
    {
        if (capturedPacks.getCount() == 1 && eachVal->getBasePack() == capturedPacks[0])
        {
            return capturedPacks[0];
        }
    }
    auto patternIntVal = as<IntVal>(patternVal);
    Type* packType = patternIntVal && patternIntVal->getType()
                         ? getOrCreate<ValuePackType>(patternIntVal->getType())
                         : (Type*)getIntType();
    return getOrCreate<ExpandIntValPack>(packType, patternVal, capturedPacks);
}

Val* ASTBuilder::getFirstElement(Val* basePack)
{
    if (auto valPack = as<ConcreteIntValPack>(basePack))
    {
        if (valPack->getCount() > 0)
            return valPack->getElement(0);
    }

    auto baseIntVal = as<IntVal>(basePack);
    auto packType = baseIntVal ? as<ValuePackType>(baseIntVal->getType()) : nullptr;
    auto elementType = packType ? packType->getElementType() : getIntType();
    return getOrCreate<FirstIntVal>(elementType, basePack);
}

Val* ASTBuilder::getLastElement(Val* basePack)
{
    if (auto valPack = as<ConcreteIntValPack>(basePack))
    {
        if (valPack->getCount() > 0)
            return valPack->getElement(valPack->getCount() - 1);
    }

    auto baseIntVal = as<IntVal>(basePack);
    auto packType = baseIntVal ? as<ValuePackType>(baseIntVal->getType()) : nullptr;
    auto elementType = packType ? packType->getElementType() : getIntType();
    return getOrCreate<LastIntVal>(elementType, basePack);
}

Val* ASTBuilder::getTrimFirstPack(Val* basePack)
{
    if (auto valPack = as<ConcreteIntValPack>(basePack))
    {
        ShortList<IntVal*> trimmedVals;
        for (Index i = 1; i < valPack->getCount(); i++)
            trimmedVals.add(valPack->getElement(i));
        return getOrCreate<ConcreteIntValPack>(
            as<Type>(valPack->getType()),
            trimmedVals.getArrayView().arrayView);
    }

    auto baseIntVal = as<IntVal>(basePack);
    auto packType = baseIntVal ? baseIntVal->getType() : getOrCreate<ValuePackType>(getIntType());
    return getOrCreate<TrimFirstIntValPack>(packType, basePack);
}

Val* ASTBuilder::getTrimLastPack(Val* basePack)
{
    if (auto valPack = as<ConcreteIntValPack>(basePack))
    {
        ShortList<IntVal*> trimmedVals;
        for (Index i = 0; i + 1 < valPack->getCount(); i++)
            trimmedVals.add(valPack->getElement(i));
        return getOrCreate<ConcreteIntValPack>(
            as<Type>(valPack->getType()),
            trimmedVals.getArrayView().arrayView);
    }

    auto baseIntVal = as<IntVal>(basePack);
    auto packType = baseIntVal ? baseIntVal->getType() : getOrCreate<ValuePackType>(getIntType());
    return getOrCreate<TrimLastIntValPack>(packType, basePack);
}

Val* ASTBuilder::getShapeConcatIntValPack(Val* leftPack, Val* rightPack, IntVal* axis)
{
    IntegerLiteralValue axisValue = 0;
    if (auto leftValPack = as<ConcreteIntValPack>(leftPack))
    {
        if (auto rightValPack = as<ConcreteIntValPack>(rightPack))
        {
            if (tryGetConstantIntVal(axis, axisValue) &&
                leftValPack->getCount() == rightValPack->getCount() && axisValue >= 0 &&
                axisValue < leftValPack->getCount())
            {
                ShortList<IntVal*> resultVals;
                bool canFold = true;
                for (Index i = 0; i < leftValPack->getCount(); ++i)
                {
                    if (i == axisValue)
                    {
                        resultVals.add(PolynomialIntVal::add(
                            this,
                            leftValPack->getElement(i),
                            rightValPack->getElement(i)));
                    }
                    else if (leftValPack->getElement(i)->equals(rightValPack->getElement(i)))
                    {
                        resultVals.add(leftValPack->getElement(i));
                    }
                    else
                    {
                        canFold = false;
                        break;
                    }
                }
                if (canFold)
                    return getIntValPack(resultVals.getArrayView().arrayView);
            }
        }
    }

    auto packType = _getIntPackTypeForVals(this, leftPack, rightPack);
    return getOrCreate<ShapeConcatIntValPack>(packType, leftPack, rightPack, axis);
}

Val* ASTBuilder::getShapePermuteIntValPack(Val* valuePack, Val* orderPack)
{
    if (auto concreteValuePack = as<ConcreteIntValPack>(valuePack))
    {
        if (auto concreteOrderPack = as<ConcreteIntValPack>(orderPack))
        {
            if (concreteValuePack->getCount() == concreteOrderPack->getCount())
            {
                List<bool> seen;
                seen.setCount(concreteValuePack->getCount());
                for (Index i = 0; i < seen.getCount(); ++i)
                    seen[i] = false;

                ShortList<IntVal*> resultVals;
                bool canFold = true;
                for (Index i = 0; i < concreteOrderPack->getCount(); ++i)
                {
                    IntegerLiteralValue orderIndex = 0;
                    if (!tryGetConstantIntVal(concreteOrderPack->getElement(i), orderIndex) ||
                        orderIndex < 0 || orderIndex >= concreteValuePack->getCount() ||
                        seen[orderIndex])
                    {
                        canFold = false;
                        break;
                    }

                    seen[orderIndex] = true;
                    resultVals.add(concreteValuePack->getElement(orderIndex));
                }

                if (canFold)
                    return getIntValPack(resultVals.getArrayView().arrayView);
            }
        }
    }

    auto packType = _getIntPackTypeForVal(this, valuePack);
    return getOrCreate<ShapePermuteIntValPack>(packType, valuePack, orderPack);
}

Val* ASTBuilder::getShapeSwapIntValPack(Val* valuePack, IntVal* dim0, IntVal* dim1)
{
    IntegerLiteralValue dim0Value = 0;
    IntegerLiteralValue dim1Value = 0;
    if (auto concreteValuePack = as<ConcreteIntValPack>(valuePack))
    {
        if (tryGetConstantIntVal(dim0, dim0Value) && tryGetConstantIntVal(dim1, dim1Value) &&
            dim0Value >= 0 && dim1Value >= 0 && dim0Value < concreteValuePack->getCount() &&
            dim1Value < concreteValuePack->getCount())
        {
            if (dim0Value == dim1Value)
                return valuePack;

            ShortList<IntVal*> resultVals;
            for (Index i = 0; i < concreteValuePack->getCount(); ++i)
            {
                if (i == dim0Value)
                    resultVals.add(concreteValuePack->getElement(dim1Value));
                else if (i == dim1Value)
                    resultVals.add(concreteValuePack->getElement(dim0Value));
                else
                    resultVals.add(concreteValuePack->getElement(i));
            }
            return getIntValPack(resultVals.getArrayView().arrayView);
        }
    }

    auto packType = _getIntPackTypeForVal(this, valuePack);
    return getOrCreate<ShapeSwapIntValPack>(packType, valuePack, dim0, dim1);
}

Val* ASTBuilder::getShapeReduceIntValPack(Val* valuePack, IntVal* axis)
{
    IntegerLiteralValue axisValue = 0;
    if (auto concreteValuePack = as<ConcreteIntValPack>(valuePack))
    {
        if (tryGetConstantIntVal(axis, axisValue) && axisValue >= 0 &&
            axisValue < concreteValuePack->getCount())
        {
            ShortList<IntVal*> resultVals;
            for (Index i = 0; i < concreteValuePack->getCount(); ++i)
            {
                if (i == axisValue)
                    resultVals.add(
                        getIntVal(as<Type>(concreteValuePack->getElement(i)->getType()), 1));
                else
                    resultVals.add(concreteValuePack->getElement(i));
            }
            return getIntValPack(resultVals.getArrayView().arrayView);
        }
    }

    auto packType = _getIntPackTypeForVal(this, valuePack);
    return getOrCreate<ShapeReduceIntValPack>(packType, valuePack, axis);
}

NonEmptyPackWitness* ASTBuilder::getNonEmptyPackWitness(Val* pack)
{
    return getOrCreate<NonEmptyPackWitness>(pack);
}

TypeEqualityWitness* ASTBuilder::getTypeEqualityWitness(Type* type)
{
    return getOrCreate<TypeEqualityWitness>(type, type);
}

TypePackSubtypeWitness* ASTBuilder::getSubtypeWitnessPack(
    Type* subType,
    Type* superType,
    ArrayView<SubtypeWitness*> witnesses)
{
    return getOrCreate<TypePackSubtypeWitness>(subType, superType, witnesses);
}

SubtypeWitness* ASTBuilder::getExpandSubtypeWitness(
    Type* subType,
    Type* superType,
    SubtypeWitness* patternWitness)
{
    if (auto eachWitness = as<EachSubtypeWitness>(patternWitness))
        return eachWitness->getPatternTypeWitness();
    return getOrCreate<ExpandSubtypeWitness>(subType, superType, patternWitness);
}

SubtypeWitness* ASTBuilder::getEachSubtypeWitness(
    Type* subType,
    Type* superType,
    SubtypeWitness* patternWitness)
{
    if (auto expandWitness = as<ExpandSubtypeWitness>(patternWitness))
        return expandWitness->getPatternTypeWitness();
    return getOrCreate<EachSubtypeWitness>(subType, superType, patternWitness);
}

SubtypeWitness* ASTBuilder::getFirstSubtypeWitness(
    Type* subType,
    Type* superType,
    SubtypeWitness* patternWitness)
{
    if (auto witnessPack = as<TypePackSubtypeWitness>(patternWitness))
    {
        if (witnessPack->getCount() > 0)
            return witnessPack->getWitness(0);
    }
    return getOrCreate<FirstSubtypeWitness>(subType, superType, patternWitness);
}

SubtypeWitness* ASTBuilder::getLastSubtypeWitness(
    Type* subType,
    Type* superType,
    SubtypeWitness* patternWitness)
{
    if (auto witnessPack = as<TypePackSubtypeWitness>(patternWitness))
    {
        if (witnessPack->getCount() > 0)
            return witnessPack->getWitness(witnessPack->getCount() - 1);
    }
    return getOrCreate<LastSubtypeWitness>(subType, superType, patternWitness);
}

SubtypeWitness* ASTBuilder::getTrimFirstSubtypeWitness(
    Type* subType,
    Type* superType,
    SubtypeWitness* patternWitness)
{
    if (auto witnessPack = as<TypePackSubtypeWitness>(patternWitness))
    {
        List<SubtypeWitness*> newWitnesses;
        for (Index i = 1; i < witnessPack->getCount(); i++)
            newWitnesses.add(witnessPack->getWitness(i));
        return getSubtypeWitnessPack(subType, superType, newWitnesses.getArrayView());
    }
    return getOrCreate<TrimFirstSubtypeWitness>(subType, superType, patternWitness);
}

SubtypeWitness* ASTBuilder::getTrimLastSubtypeWitness(
    Type* subType,
    Type* superType,
    SubtypeWitness* patternWitness)
{
    if (auto witnessPack = as<TypePackSubtypeWitness>(patternWitness))
    {
        List<SubtypeWitness*> newWitnesses;
        for (Index i = 0; i + 1 < witnessPack->getCount(); i++)
            newWitnesses.add(witnessPack->getWitness(i));
        return getSubtypeWitnessPack(subType, superType, newWitnesses.getArrayView());
    }
    return getOrCreate<TrimLastSubtypeWitness>(subType, superType, patternWitness);
}

SubtypeWitness* ASTBuilder::getPackBranchSubtypeWitness(
    Type* subType,
    Type* superType,
    Val* packOperand,
    SubtypeWitness* emptyWitness,
    SubtypeWitness* nonEmptyWitness)
{
    switch (getKnownPackCardinality(packOperand))
    {
    case VariadicPackCardinality::Empty:
        return emptyWitness;
    case VariadicPackCardinality::NonEmpty:
        return nonEmptyWitness;
    default:
        break;
    }

    if (emptyWitness == nonEmptyWitness)
        return emptyWitness;

    return getOrCreate<PackBranchSubtypeWitness>(
        subType,
        superType,
        packOperand,
        emptyWitness,
        nonEmptyWitness);
}

DeclaredSubtypeWitness* ASTBuilder::getDeclaredSubtypeWitness(
    Type* subType,
    Type* superType,
    DeclRef<Decl> const& declRef)
{
    auto witness = getOrCreate<DeclaredSubtypeWitness>(subType, superType, declRef.declRefBase);
    return witness;
}

SubtypeWitness* ASTBuilder::getTransitiveSubtypeWitness(
    SubtypeWitness* aIsSubtypeOfBWitness,
    SubtypeWitness* bIsSubtypeOfCWitness)
{
top:
    // Our job is to take the witnesses that `a <: b` and `b <: c`
    // and produce a valid witness that `a <: c`
    //
    // There are some special cases we want to handle, in order
    // to simplify logic elesewhere in the compiler. For example,
    // if either of the input witnesses is a type *equality* witness,
    // then the other witness can be returned as-is.
    //
    // If `a == b`, then the `b <: c` witness is also a witness of `a <: c`.
    //
    if (as<TypeEqualityWitness>(aIsSubtypeOfBWitness))
    {
        return bIsSubtypeOfCWitness;
    }

    if (as<TypeEqualityWitness>(bIsSubtypeOfCWitness))
    {
        return aIsSubtypeOfBWitness;
    }

    if (auto declAIsSubtypeOfBWitness = as<DeclaredSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        if (declAIsSubtypeOfBWitness->isEquality())
            return bIsSubtypeOfCWitness;
    }

    else if (auto declBIsSubtypeOfCWitness = as<DeclaredSubtypeWitness>(bIsSubtypeOfCWitness))
    {
        if (declBIsSubtypeOfCWitness->isEquality())
            return declBIsSubtypeOfCWitness;
    }

    // HACK: There is downstream code generation logic that assumes that
    // a `TransitiveSubtypeWitness` will never have a transitive witness
    // as its `b <: c` witness. If we are at risk of creating such a witness here,
    // we will shift things around to make that not be the case:
    //
    if (auto bIsTransitiveSubtypeOfCWitness = as<TransitiveSubtypeWitness>(bIsSubtypeOfCWitness))
    {
        // Let's call the intermediate type here `x`, we know that the `b <: c`
        // witness is based on witnesses that `b <: x` and `x <: c`:
        //
        auto bIsSubtypeOfXWitness = bIsTransitiveSubtypeOfCWitness->getSubToMid();
        auto xIsSubtypeOfCWitness = bIsTransitiveSubtypeOfCWitness->getMidToSup();

        // We can recursively call this operation to produce a witness that
        // `a <: x`, based on the witnesses we already have for `a <: b` and `b <: x`:
        //
        auto aIsSubtypeOfXWitness =
            getTransitiveSubtypeWitness(aIsSubtypeOfBWitness, bIsSubtypeOfXWitness);

        // Now we can perform a "tail recursive" call to this function (via `goto`
        // to combine the `a <: x` witness with our `x <: c` witness:
        //
        aIsSubtypeOfBWitness = aIsSubtypeOfXWitness;
        bIsSubtypeOfCWitness = xIsSubtypeOfCWitness;
        goto top;
    }

    auto aType = aIsSubtypeOfBWitness->getSub();
    auto cType = bIsSubtypeOfCWitness->getSup();

    // If left hand is a TypePackSubtypeWitness, then we should also return a TypePackSubtypeWitness
    // where each witness in the pack is the transitive subtype witness of the corresponding
    // witness in the original pack.
    //
    if (auto witnessPack = as<TypePackSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        List<SubtypeWitness*> newWitnesses;
        for (Index i = 0; i < witnessPack->getCount(); i++)
        {
            newWitnesses.add(
                getTransitiveSubtypeWitness(witnessPack->getWitness(i), bIsSubtypeOfCWitness));
        }
        return getSubtypeWitnessPack(aType, cType, newWitnesses.getArrayView());
    }

    // If left hand is a ExpandSubtypeWitness, then we want to perform the transitive lookup
    // on the pattern witness, and then form a new ExpandSubtypeWitness with the result.
    //
    if (auto expandWitness = as<ExpandSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        auto innerTransitiveWitness = getTransitiveSubtypeWitness(
            expandWitness->getPatternTypeWitness(),
            bIsSubtypeOfCWitness);
        return getExpandSubtypeWitness(expandWitness->getSub(), cType, innerTransitiveWitness);
    }

    if (auto trimFirstWitness = as<TrimFirstSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        auto innerTransitiveWitness = getTransitiveSubtypeWitness(
            trimFirstWitness->getPatternTypeWitness(),
            bIsSubtypeOfCWitness);
        return getTrimFirstSubtypeWitness(
            trimFirstWitness->getSub(),
            cType,
            innerTransitiveWitness);
    }

    if (auto trimLastWitness = as<TrimLastSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        auto innerTransitiveWitness = getTransitiveSubtypeWitness(
            trimLastWitness->getPatternTypeWitness(),
            bIsSubtypeOfCWitness);
        return getTrimLastSubtypeWitness(trimLastWitness->getSub(), cType, innerTransitiveWitness);
    }

    if (auto packBranchWitness = as<PackBranchSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        auto emptyTransitiveWitness =
            getTransitiveSubtypeWitness(packBranchWitness->getEmptyWitness(), bIsSubtypeOfCWitness);
        auto nonEmptyTransitiveWitness = getTransitiveSubtypeWitness(
            packBranchWitness->getNonEmptyWitness(),
            bIsSubtypeOfCWitness);
        return getPackBranchSubtypeWitness(
            packBranchWitness->getSub(),
            cType,
            packBranchWitness->getPackOperand(),
            emptyTransitiveWitness,
            nonEmptyTransitiveWitness);
    }

    // If left hand is a DeclaredWitness for a type pack parameter T, then we want to perform the
    // transitive lookup on `each T`, and then form a new ExpandSubtypeWitness with the result.
    //
    if (auto declaredWitness = as<DeclaredSubtypeWitness>(aIsSubtypeOfBWitness))
    {
        if (auto declRefType = as<DeclRefType>(declaredWitness->getSub()))
        {
            if (declRefType->getDeclRef().as<GenericTypePackParamDecl>())
            {
                auto newLeftHandWitness = getEachSubtypeWitness(
                    getEachType(declaredWitness->getSub()),
                    declaredWitness->getSup(),
                    declaredWitness);
                auto transitiveWitness =
                    getTransitiveSubtypeWitness(newLeftHandWitness, bIsSubtypeOfCWitness);
                return getExpandSubtypeWitness(aType, cType, transitiveWitness);
            }
        }
    }

    // If none of the above special cases applied, then we are just going to create
    // a `TransitiveSubtypeWitness` directly.
    //
    // TODO: Identify other cases that we can potentially simplify.
    // It is particularly notable that we do not have simplification rules that
    // detect when the left-hand side of a composition has some particular
    // structure. This may be fine, or it may not; we should write down a more
    // formal set of rules for the allowed structure of our witnesses to
    // guarantee that our simplifications are sufficient.

    TransitiveSubtypeWitness* transitiveWitness = getOrCreate<TransitiveSubtypeWitness>(
        aType,
        cType,
        aIsSubtypeOfBWitness,
        bIsSubtypeOfCWitness);
    return transitiveWitness;
}

BuiltinTypeCoercionWitness* ASTBuilder::getBuiltinTypeCoercionWitness(Type* fromType, Type* toType)
{
    return getOrCreate<BuiltinTypeCoercionWitness>(fromType, toType);
}

DeclRefTypeCoercionWitness* ASTBuilder::getDeclRefTypeCoercionWitness(
    Type* fromType,
    Type* toType,
    DeclRef<Decl> declRef)
{
    return getOrCreate<DeclRefTypeCoercionWitness>(fromType, toType, declRef);
}

DeclRef<Decl> _getMemberDeclRef(ASTBuilder* builder, DeclRef<Decl> parent, Decl* decl)
{
    return builder->getMemberDeclRef(parent, decl);
}


// Session installs its root AST builder here during init() and nulls it on destruction.
// Compilation functions use SLANG_AST_BUILDER_RAII to scope the active builder.
thread_local ASTBuilder* gCurrentASTBuilder = nullptr;

ASTBuilder* getCurrentASTBuilder()
{
    return gCurrentASTBuilder;
}

void setCurrentASTBuilder(ASTBuilder* astBuilder)
{
    gCurrentASTBuilder = astBuilder;
}

} // namespace Slang
