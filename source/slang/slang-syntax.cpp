#include "slang-syntax.h"

#include "slang-compiler.h"
#include "slang-visitor.h"

#include <typeinfo>
#include <assert.h>

namespace Slang
{

/* static */const TypeExp TypeExp::empty;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! DiagnosticSink impls !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void printDiagnosticArg(StringBuilder& sb, Decl* decl)
{
    if (!decl)
        return;
    sb << getText(decl->getName());
}

void printDiagnosticArg(StringBuilder& sb, DeclRefBase* declRefBase)
{
    printDiagnosticArg(sb, declRefBase->getDecl());
}

void printDiagnosticArg(StringBuilder& sb, Type* type)
{
    if (!type)
        return;
    type->toText(sb);
}

void printDiagnosticArg(StringBuilder& sb, Val* val)
{
    if (!val)
        return;
    val->toText(sb);
}

void printDiagnosticArg(StringBuilder& sb, TypeExp const& type)
{
    if (type.type)
        type.type->toText(sb);
    else
        sb << "<null>";
}

void printDiagnosticArg(StringBuilder& sb, QualType const& type)
{
    if (type.type)
        type.type->toText(sb);
    else
        sb << "<null>";
}

SourceLoc getDiagnosticPos(SyntaxNode const* syntax)
{
    if (!syntax)
        return SourceLoc();
    return syntax->loc;
}

SourceLoc getDiagnosticPos(TypeExp const& typeExp)
{
    if (!typeExp.exp)
        return SourceLoc();
    return typeExp.exp->loc;
}

SourceLoc getDiagnosticPos(DeclRefBase* declRef)
{
    if (!declRef)
        return SourceLoc();
    return declRef->getDecl()->loc;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!  Free functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Decl*const* adjustFilterCursorImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
{
    switch (filterStyle)
    {
        default:
        case MemberFilterStyle::All:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo))
                {
                    return ptr;
                }
            }
            break;
        }
        case MemberFilterStyle::Instance:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && !decl->hasModifier<HLSLStaticModifier>())
                {
                    return ptr;
                }
            }
            break;
        }
        case MemberFilterStyle::Static:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && decl->hasModifier<HLSLStaticModifier>())
                {
                    return ptr;
                }
            }
            break;
        }
    }
    return end;
}

Decl*const* getFilterCursorByIndexImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end, Index index)
{
    switch (filterStyle)
    {
        default:
        case MemberFilterStyle::All:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo))
                {
                    if (index <= 0)
                    {
                        return ptr;
                    }
                    index--;
                }
            }
            break;
        }
        case MemberFilterStyle::Instance:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && !decl->hasModifier<HLSLStaticModifier>())
                {
                    if (index <= 0)
                    {
                        return ptr;
                    }
                    index--;
                }
            }
            break;
        }
        case MemberFilterStyle::Static:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && decl->hasModifier<HLSLStaticModifier>())
                {
                    if (index <= 0)
                    {
                        return ptr;
                    }
                    index--;
                }
            }
            break;
        }
    }
    return nullptr;
}

Index getFilterCountImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
{
    Index count = 0;
    switch (filterStyle)
    {
        default:
        case MemberFilterStyle::All:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                count += Index(decl->getClassInfo().isSubClassOf(clsInfo));
            }
            break;
        }
        case MemberFilterStyle::Instance:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                count += Index(decl->getClassInfo().isSubClassOf(clsInfo)&& !decl->hasModifier<HLSLStaticModifier>());
            }
            break;
        }
        case MemberFilterStyle::Static:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                count += Index(decl->getClassInfo().isSubClassOf(clsInfo) && decl->hasModifier<HLSLStaticModifier>());
            }
            break;
        }
    }
    return count;
}

    // TypeExp

    bool TypeExp::equals(Type* other)
    {
        return type->equals(other);
    }

    //
    // RequirementWitness
    //

    RequirementWitness::RequirementWitness(Val* val)
        : m_flavor(Flavor::val)
        , m_val(val)
    {}


    RequirementWitness::RequirementWitness(RefPtr<WitnessTable> witnessTable)
        : m_flavor(Flavor::witnessTable)
        , m_obj(witnessTable)
    {}

    RefPtr<WitnessTable> RequirementWitness::getWitnessTable()
    {
        SLANG_ASSERT(getFlavor() == Flavor::witnessTable);
        return m_obj.as<WitnessTable>();
    }

    RefPtr<WitnessTable> WitnessTable::specialize(ASTBuilder* astBuilder, SubstitutionSet const& subst)
    {
        auto newBaseType = baseType->substitute(astBuilder, subst);
        auto newWitnessedType = witnessedType->substitute(astBuilder, subst);
        if (newBaseType == baseType && newWitnessedType == witnessedType)
            return this;
        RefPtr<WitnessTable> result = new WitnessTable();
        result->baseType = as<Type>(newBaseType);
        result->witnessedType = as<Type>(newWitnessedType);
        for (auto requirement : m_requirements)
        {
            auto newRequirement = requirement.value.specialize(astBuilder, subst);
            result->add(requirement.key, newRequirement);
        }
        return result;
    }

    RequirementWitness RequirementWitness::specialize(ASTBuilder* astBuilder, SubstitutionSet const& subst)
    {
        switch(getFlavor())
        {
        default:
            SLANG_UNEXPECTED("unknown requirement witness flavor");
        case RequirementWitness::Flavor::none:
            return RequirementWitness();

        case RequirementWitness::Flavor::witnessTable:
            return RequirementWitness(this->getWitnessTable()->specialize(astBuilder, subst));

        case RequirementWitness::Flavor::declRef:
            {
                int diff = 0;
                return RequirementWitness(
                    getDeclRef().substituteImpl(astBuilder, subst, &diff));
            }

        case RequirementWitness::Flavor::val:
            {
                auto val = getVal();
                SLANG_ASSERT(val);

                return RequirementWitness(
                    val->substitute(astBuilder, subst));
            }
        }
    }

    RequirementWitness tryLookUpRequirementWitness(
        ASTBuilder*     astBuilder,
        SubtypeWitness* subtypeWitness,
        Decl*           requirementKey)
    {
        if(auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subtypeWitness))
        {
            if(auto inheritanceDeclRef = declaredSubtypeWitness->getDeclRef().as<InheritanceDecl>())
            {
                // A conformance that was declared as part of an inheritance clause
                // will have built up a dictionary of the satisfying declarations
                // for each of its requirements.
                RequirementWitness requirementWitness;
                auto witnessTable = inheritanceDeclRef.getDecl()->witnessTable;
                if(witnessTable && witnessTable->getRequirementDictionary().tryGetValue(requirementKey, requirementWitness))
                {
                    // The `inheritanceDeclRef` has substitutions applied to it that
                    // *aren't* present in the `requirementWitness`, because it was
                    // derived by the front-end when looking at the `InheritanceDecl` alone.
                    //
                    // We need to apply these substitutions here for the result to make sense.
                    //
                    // E.g., if we have a case like:
                    //
                    //      interface ISidekick { associatedtype Hero; void follow(Hero hero); }
                    //      struct Sidekick<H> : ISidekick { typedef H Hero; void follow(H hero) {} };
                    //
                    //      void followHero<S : ISidekick>(S s, S.Hero h)
                    //      {
                    //          s.follow(h);
                    //      }
                    //
                    //      Batman batman;
                    //      Sidekick<Batman> robin;
                    //      followHero<Sidekick<Batman>>(robin, batman);
                    //
                    // The second argument to `followHero` is `batman`, which has type `Batman`.
                    // The parameter declaration lists the type `S.Hero`, which is a reference
                    // to an associated type. The front  end will expand this into something
                    // like `S.{S:ISidekick}.Hero` - that is, we'll end up with a declaration
                    // reference to `ISidekick.Hero` with a this-type substitution that references
                    // the `{S:ISidekick}` declaration as a witness.
                    //
                    // The front-end will expand the generic application `followHero<Sidekick<Batman>>`
                    // to `followHero<Sidekick<Batman>, {Sidekick<H>:ISidekick}[H->Batman]>`
                    // (that is, the hidden second parameter will reference the inheritance
                    // clause on `Sidekick<H>`, with a substitution to map `H` to `Batman`.
                    //
                    // This step should map the `{S:ISidekick}` declaration over to the
                    // concrete `{Sidekick<H>:ISidekick}[H->Batman]` inheritance declaration.
                    // At that point `tryLookupRequirementWitness` might be called, because
                    // we want to look up the witness for the key `ISidekick.Hero` in the
                    // inheritance decl-ref that is `{Sidekick<H>:ISidekick}[H->Batman]`.
                    //
                    // That lookup will yield us a reference to the typedef `Sidekick<H>.Hero`,
                    // *without* any substitution for `H` (or rather, with a default one that
                    // maps `H` to `H`.
                    //
                    // So, in order to get the *right* end result, we need to apply
                    // the substitutions from the inheritance decl-ref to the witness.
                    //
                    requirementWitness = requirementWitness.specialize(astBuilder, SubstitutionSet(inheritanceDeclRef));

                    return requirementWitness;
                }
            }
        }
        else if (auto transitiveTypeWitness = as<TransitiveSubtypeWitness>(subtypeWitness))
        {
            if (auto declaredSubtypeWitnessMidToSup = as<DeclaredSubtypeWitness>(transitiveTypeWitness->getMidToSup()))
            {
                auto midKey = declaredSubtypeWitnessMidToSup->getDeclRef();
                auto midWitness = tryLookUpRequirementWitness(astBuilder, as<SubtypeWitness>(transitiveTypeWitness->getSubToMid()), midKey.getDecl());
                if (midWitness.getFlavor() == RequirementWitness::Flavor::witnessTable)
                {
                    auto table = midWitness.getWitnessTable();
                    RequirementWitness result;
                    if (table->getRequirementDictionary().tryGetValue(requirementKey, result))
                    {
                        result = result.specialize(astBuilder, SubstitutionSet(midKey));
                    }
                    return result;
                }
            }
        }
        else if (auto extractFromConjunctionTypeWitness = as<ExtractFromConjunctionSubtypeWitness>(subtypeWitness))
        {
            if (auto conjunctionTypeWitness = as<ConjunctionSubtypeWitness>(extractFromConjunctionTypeWitness->getConjunctionWitness()))
            {
                auto componentWitness = as<SubtypeWitness>(
                    conjunctionTypeWitness->getComponentWitness(
                        extractFromConjunctionTypeWitness->getIndexInConjunction()));

                return tryLookUpRequirementWitness(astBuilder, componentWitness, requirementKey);
            }
        }
        
        // If we are looking for `ThisType`, just return subtype.
        if (as<ThisTypeDecl>(requirementKey))
        {
            RequirementWitness result;
            result.m_flavor = RequirementWitness::Flavor::val;
            result.m_val = subtypeWitness->getSub();
            return result;
        }
        // If we are looking for `ThisTypeConstraint`, just return the witness itself.
        if (as<ThisTypeConstraintDecl>(requirementKey))
        {
            RequirementWitness result;
            result.m_flavor = RequirementWitness::Flavor::val;
            result.m_val = subtypeWitness;
            return result;
        }
        // TODO: should handle the transitive case here too

        return RequirementWitness();
    }

    //
    // WitnessTable
    //

    void WitnessTable::add(Decl* decl, RequirementWitness const& witness)
    {
        m_requirements.add(KeyValuePair<Decl*, RequirementWitness>(decl, witness));
        m_requirementDictionary.add(decl, witness);
    }

    // TODO: need to figure out how to unify this with the logic
    // in the generic case...
    Type* DeclRefType::create(
        ASTBuilder*     astBuilder,
        DeclRef<Decl>   declRef)
    {
        if (declRef.getDecl()->findModifier<BuiltinTypeModifier>())
        {
            // Always create builtin types in global AST builder.
            if (astBuilder->getSharedASTBuilder()->getInnerASTBuilder() != astBuilder)
                return DeclRefType::create(astBuilder->getSharedASTBuilder()->getInnerASTBuilder(), declRef);

            declRef = createDefaultSubstitutionsIfNeeded(astBuilder, nullptr, declRef);
            auto type = astBuilder->getOrCreate<BasicExpressionType>(declRef.declRefBase);
            return type;
        }
        else if (auto magicMod = declRef.getDecl()->findModifier<MagicTypeModifier>())
        {
            if (magicMod->magicNodeType == ASTNodeType(-1))
            {
                SLANG_UNEXPECTED("unhandled type");
            }
            // Always create builtin types in global AST builder.
            if (astBuilder->getSharedASTBuilder()->getInnerASTBuilder() != astBuilder)
                return DeclRefType::create(astBuilder->getSharedASTBuilder()->getInnerASTBuilder(), declRef);

            declRef = createDefaultSubstitutionsIfNeeded(astBuilder, nullptr, declRef);
            ValNodeDesc nodeDesc = {};
            nodeDesc.type = magicMod->magicNodeType;
            nodeDesc.operands.add(ValNodeOperand(declRef));
            nodeDesc.init();
            NodeBase* type = astBuilder->_getOrCreateImpl(_Move(nodeDesc));
            if (!type)
            {
                SLANG_UNEXPECTED("constructor failure");
            }

            auto declRefType = dynamicCast<DeclRefType>(type);
            if (!declRefType)
            {
                SLANG_UNEXPECTED("expected a declaration reference type");
            }
            return declRefType;
        }
        else if (as<ThisTypeDecl>(declRef.getDecl()) && as<DirectDeclRef>(declRef.declRefBase))
        {
            declRef = createDefaultSubstitutionsIfNeeded(astBuilder, nullptr, declRef);

            return astBuilder->getOrCreate<ThisType>(declRef.declRefBase);
        }
        else if (auto typedefDecl = as<TypeDefDecl>(declRef.getDecl()))
        {
            if (typedefDecl->type.type)
                return as<Type>(typedefDecl->type.type->substitute(astBuilder, SubstitutionSet(declRef)));
            return astBuilder->getErrorType();
        }
        else
        {
            declRef = createDefaultSubstitutionsIfNeeded(astBuilder, nullptr, declRef);

            return astBuilder->getOrCreate<DeclRefType>(declRef.declRefBase);
        }
    }

    //

    Val::OperandView<Val> findInnerMostGenericArgs(SubstitutionSet subst)
    {
        if (!subst.declRef)
            return Val::OperandView<Val>();
        if (auto genApp = subst.findGenericAppDeclRef())
            return genApp->getArgs();
        return Val::OperandView<Val>();
    }

    SubstExpr<Expr> substituteExpr(SubstitutionSet const& substs, Expr* expr)
    {
        return SubstExpr<Expr>(expr, substs);
    }

    DeclRef<Decl> substituteDeclRef(SubstitutionSet const& substs, ASTBuilder* astBuilder, DeclRef<Decl> const& declRef)
    {
        if(!substs)
            return declRef;

        int diff = 0;
        auto declRefBase = declRef.substituteImpl(astBuilder, substs, &diff);
        return declRefBase;
    }

    Type* substituteType(SubstitutionSet const& substs, ASTBuilder* astBuilder, Type* type)
    {
        if(!type) return nullptr;
        if(!substs) return type;

        SLANG_ASSERT(type);

        return Slang::as<Type>(type->substitute(astBuilder, substs));
    }

    InterfaceDecl* findOuterInterfaceDecl(Decl* decl)
    {
        Decl* dd = decl;
        while(dd)
        {
            if(auto interfaceDecl = as<InterfaceDecl>(dd))
                return interfaceDecl;

            dd = dd->parentDecl;
        }
        return nullptr;
    }

    // IntVal

    IntegerLiteralValue getIntVal(IntVal* val)
    {
        if (auto constantVal = as<ConstantIntVal>(val))
        {
            return constantVal->getValue();
        }
        SLANG_UNEXPECTED("needed a known integer value");
        //return 0;
    }

    //

    // HLSLPatchType

    Val* getGenericArg(DeclRef<Decl> declRef, Index index)
    {
        auto subst = SubstitutionSet(declRef).findGenericAppDeclRef();
        if (index < subst->getArgs().getCount())
            return subst->getArgs()[index];
        return nullptr;
    }

    Type* HLSLPatchType::getElementType()
    {
        return as<Type>(getGenericArg(getDeclRef(), 0));
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return as<IntVal>(getGenericArg(getDeclRef(), 1));
    }

    // MeshOutputType
    // There's a subtle distinction between this and HLSLPatchType, the size
    // here is the max possible size of the array, it's free to change at
    // runtime. There's probably no circumstance where you'd want to be generic
    // between the two, so we don't deduplicate this code.

    Type* MeshOutputType::getElementType()
    {
        return as<Type>(getGenericArg(getDeclRef(), 0));
    }

    IntVal* MeshOutputType::getMaxElementCount()
    {
        return as<IntVal>(getGenericArg(getDeclRef(), 1));
    }

    // Constructors for types

    ArrayExpressionType* getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType,
        IntVal*         elementCount)
    {
        return astBuilder->getArrayType(elementType, elementCount);
    }

    ArrayExpressionType* getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType)
    {
        return astBuilder->getArrayType(elementType, nullptr);
    }

    NamedExpressionType* getNamedType(
        ASTBuilder*                 astBuilder,
        DeclRef<TypeDefDecl> const& declRef)
    {
        DeclRef<TypeDefDecl> specializedDeclRef = createDefaultSubstitutionsIfNeeded(astBuilder, nullptr, declRef).as<TypeDefDecl>();

        return astBuilder->getOrCreate<NamedExpressionType>(specializedDeclRef);
    }
    
    FuncType* getFuncType(
        ASTBuilder*                     astBuilder,
        DeclRef<CallableDecl> const&    declRef)
    {
        List<Type*> paramTypes;
        auto resultType = getResultType(astBuilder, declRef);
        auto errorType = getErrorCodeType(astBuilder, declRef);
        for (auto paramDeclRef : getParameters(astBuilder, declRef))
        {
            auto paramDecl = paramDeclRef.getDecl();
            auto paramType = getParamType(astBuilder, paramDeclRef);
            if( paramDecl->findModifier<RefModifier>() )
            {
                paramType = astBuilder->getRefType(paramType);
            }
            else if (paramDecl->findModifier<ConstRefModifier>())
            {
                paramType = astBuilder->getConstRefType(paramType);
            }
            else if( paramDecl->findModifier<OutModifier>() )
            {
                if(paramDecl->findModifier<InOutModifier>() || paramDecl->findModifier<InModifier>())
                {
                    paramType = astBuilder->getInOutType(paramType);
                }
                else
                {
                    paramType = astBuilder->getOutType(paramType);
                }
            }
            paramTypes.add(paramType);
        }

        FuncType* funcType = astBuilder->getOrCreate<FuncType>(paramTypes.getArrayView(), resultType, errorType);
        return funcType;
    }

    GenericDeclRefType* getGenericDeclRefType(
        ASTBuilder*                 astBuilder,
        DeclRef<GenericDecl> const& declRef)
    {
        return astBuilder->getOrCreate<GenericDeclRefType>(declRef);
    }

    NamespaceType* getNamespaceType(
        ASTBuilder*                         astBuilder,
        DeclRef<NamespaceDeclBase> const&   declRef)
    {
        auto type = astBuilder->getOrCreate<NamespaceType>(declRef);
        return type;
    }

    SamplerStateType* getSamplerStateType(
        ASTBuilder*     astBuilder)
    {
        return astBuilder->getSamplerStateType();
    }

    SubtypeWitness* findThisTypeWitness(
        SubstitutionSet  substs,
        InterfaceDecl*  interfaceDecl)
    {
        auto lookupDeclRef = substs.findLookupDeclRef();
        if (!lookupDeclRef)
            return nullptr;
        if (lookupDeclRef->getSupDecl() == interfaceDecl)
        {
            return lookupDeclRef->getWitness();
        }
        return nullptr;
    }

    Val* _tryLookupConcreteAssociatedTypeFromThisTypeSubst(ASTBuilder* builder, DeclRef<Decl> declRef)
    {
        auto substDeclRef = declRef.as<AssocTypeDecl>();
        if (!substDeclRef)
            return nullptr;

        auto substAssocTypeDecl = substDeclRef.getDecl();

        if (auto lookupDeclRef = SubstitutionSet(substDeclRef).findLookupDeclRef())
        {
            if (auto interfaceDecl = as<InterfaceDecl>(substAssocTypeDecl->parentDecl))
            {
                if (lookupDeclRef->getSupDecl() == interfaceDecl)
                {
                    // We need to look up the declaration that satisfies
                    // the requirement named by the associated type.
                    Decl* requirementKey = substAssocTypeDecl;
                    RequirementWitness requirementWitness = tryLookUpRequirementWitness(builder, lookupDeclRef->getWitness(), requirementKey);
                    switch (requirementWitness.getFlavor())
                    {
                    default:
                        // No usable value was found, so there is nothing we can do.
                        break;

                    case RequirementWitness::Flavor::val:
                    {
                        auto satisfyingVal = requirementWitness.getVal();
                        return satisfyingVal;
                    }
                    break;
                    }

                    // Hard code implementation of T.Differential.Differential == T.Differential rule.
                    auto foldResult = [&]() -> Val*
                    {
                        auto builtinReq = substDeclRef.getDecl()->findModifier<BuiltinRequirementModifier>();
                        if (!builtinReq)
                            return nullptr;
                        if (builtinReq->kind != BuiltinRequirementKind::DifferentialType)
                            return nullptr;
                        // Is the concrete type a Differential associated type?
                        auto innerDeclRefType = as<DeclRefType>(lookupDeclRef->getWitness()->getSub());
                        if (!innerDeclRefType)
                            return nullptr;
                        auto innerBuiltinReq = innerDeclRefType->getDeclRef().getDecl()->findModifier<BuiltinRequirementModifier>();
                        if (!innerBuiltinReq)
                            return nullptr;
                        if (innerBuiltinReq->kind != BuiltinRequirementKind::DifferentialType)
                            return nullptr;
                        if (!innerDeclRefType->getDeclRef().equals(declRef))
                        {
                            auto result = _tryLookupConcreteAssociatedTypeFromThisTypeSubst(builder, innerDeclRefType->getDeclRef());
                            if (result)
                                return result;
                        }
                        return innerDeclRefType;
                    }();
                    if (foldResult)
                        return foldResult;
                }
            }
        }
        return nullptr;
    }

ModuleDecl* getModuleDecl(Decl* decl)
{
    for( auto dd = decl; dd; dd = dd->parentDecl )
    {
        if(auto moduleDecl = as<ModuleDecl>(dd))
            return moduleDecl;
    }
    return nullptr;
}

Module* getModule(Decl* decl)
{
    auto moduleDecl = getModuleDecl(decl);
    if(!moduleDecl)
        return nullptr;

    return moduleDecl->module;
}

Decl* getParentDecl(Decl* decl)
{
    decl = decl->parentDecl;
    while (as<GenericDecl>(decl))
        decl = decl->parentDecl;
    return decl;
}

Decl* getParentFunc(Decl* decl)
{
    while (decl)
    {
        if (as<FunctionDeclBase>(decl))
            return decl;
        decl = decl->parentDecl;
    }
    return nullptr;
}

static const ImageFormatInfo kImageFormatInfos[] =
{
#define SLANG_IMAGE_FORMAT_INFO(TYPE, COUNT, SIZE) SLANG_SCALAR_TYPE_##TYPE, uint8_t(COUNT), uint8_t(SIZE)
#define FORMAT(NAME, OTHER) \
    { SLANG_IMAGE_FORMAT_INFO OTHER, UnownedStringSlice::fromLiteral(#NAME) },
#include "slang-image-format-defs.h"
#undef FORMAT
#undef SLANG_IMAGE_FORMAT_INFO
};

bool findImageFormatByName(const UnownedStringSlice& name, ImageFormat* outFormat)
{
    for (Index i = 0; i < SLANG_COUNT_OF(kImageFormatInfos); ++i)
    {
        const auto& info = kImageFormatInfos[i];
        if (info.name == name)
        {
            *outFormat = ImageFormat(i);
            return true;
        }
    }
    return false;
}

// https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#id71
#define SLANG_VK_TO_IMAGE_FORMAT(x) \
    x(r11g11b10f, r11f_g11f_b10f) \
    x(rgb10a2, rgb10_a2) \
    x(rgb10a2ui, rgb10_a2ui) 

struct VkImageFormatInfo
{
    UnownedStringSlice name;
    ImageFormat format;
};
static const VkImageFormatInfo kVkImageFormatInfos[] =
{
#define SLANG_VK_IMAGE_FORMAT_INFO(name, format) { toSlice(#name), ImageFormat::format },
    SLANG_VK_TO_IMAGE_FORMAT(SLANG_VK_IMAGE_FORMAT_INFO)
};

static const auto kSNorm = UnownedStringSlice::fromLiteral("snorm");

bool findVkImageFormatByName(const UnownedStringSlice& name, ImageFormat* outFormat)
{
    // Handle names ending in snorm
    if (name.endsWith(kSNorm))
    {
        StringBuilder buf;
        //  format names end with snormal after a '_', so replace with that
        buf << name.head(name.getLength() - kSNorm.getLength()) << "_" << kSNorm;
        return findImageFormatByName(buf.getUnownedSlice(), outFormat);
    }
    
    // Handle the special cases
    for (const auto& vkInfo : kVkImageFormatInfos)
    {
        if (vkInfo.name == name)
        {
            *outFormat = vkInfo.format;
            return true;
        }
    }

    // Default to the regular lookup mechanism for everything else
    return findImageFormatByName(name, outFormat);
}

char const* getGLSLNameForImageFormat(ImageFormat format)
{
    return kImageFormatInfos[Index(format)].name.begin();
}

const ImageFormatInfo& getImageFormatInfo(ImageFormat format)
{
    return kImageFormatInfos[Index(format)];
}

char const* getTryClauseTypeName(TryClauseType c)
{
    switch (c)
    {
    case TryClauseType::None:
        return "None";
    case TryClauseType::Standard:
        return "Standard";
    case TryClauseType::Optional:
        return "Optional";
    case TryClauseType::Assert:
        return "Assert";
    default:
        return "Unknown";
    }
}

} // namespace Slang
