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
    sb << getText(decl->getName());
}

void printDiagnosticArg(StringBuilder& sb, Type* type)
{
    sb << type->toString();
}

void printDiagnosticArg(StringBuilder& sb, Val* val)
{
    sb << val->toString();
}

void printDiagnosticArg(StringBuilder& sb, TypeExp const& type)
{
    sb << type.type->toString();
}

void printDiagnosticArg(StringBuilder& sb, QualType const& type)
{
    if (type.type)
        sb << type.type->toString();
    else
        sb << "<null>";
}

SourceLoc const& getDiagnosticPos(SyntaxNode const* syntax)
{
    return syntax->loc;
}

SourceLoc const& getDiagnosticPos(TypeExp const& typeExp)
{
    return typeExp.exp->loc;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! BasicExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool BasicExpressionType::equalsImpl(Type * type)
{
    auto basicType = as<BasicExpressionType>(type);
    return basicType && basicType->baseType == this->baseType;
}

RefPtr<Type> BasicExpressionType::createCanonicalType()
{
    // A basic type is already canonical, in our setup
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!  Free functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

const RefPtr<Decl>* adjustFilterCursorImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end)
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

const RefPtr<Decl>* getFilterCursorByIndexImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end, Index index)
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

Index getFilterCountImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end)
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

    // BasicExpressionType

    BasicExpressionType* BasicExpressionType::GetScalarType()
    {
        return this;
    }

    //

    Type::~Type()
    {
        // If the canonicalType !=nullptr AND it is not set to this (ie the canonicalType is another object)
        // then it needs to be released because it's owned by this object.
        if (canonicalType && canonicalType != this)
        {
            canonicalType->releaseReference();
        }
    }

    bool Type::equals(Type* type)
    {
        return getCanonicalType()->equalsImpl(type->getCanonicalType());
    }

    bool Type::equalsVal(Val* val)
    {
        if (auto type = dynamicCast<Type>(val))
            return const_cast<Type*>(this)->equals(type);
        return false;
    }

    RefPtr<Val> Type::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;
        auto canSubst = getCanonicalType()->substituteImpl(subst, &diff);

        // If nothing changed, then don't drop any sugar that is applied
        if (!diff)
            return this;

        // If the canonical type changed, then we return a canonical type,
        // rather than try to re-construct any amount of sugar
        (*ioDiff)++;
        return canSubst;
    }

    Type* Type::getCanonicalType()
    {
        Type* et = const_cast<Type*>(this);
        if (!et->canonicalType)
        {
            // TODO(tfoley): worry about thread safety here?
            auto canType = et->createCanonicalType();
            et->canonicalType = canType;

            // TODO(js): That this detachs when canType == this is a little surprising. It would seem
            // as if this would create a circular reference on the object, but in practice there are
            // no leaks so appears correct.
            // That the dtor only releases if != this, also makes it surprising.
            canType.detach();
            
            SLANG_ASSERT(et->canonicalType);
        }
        return et->canonicalType;
    }

    void Session::initializeTypes()
    {
        errorType = new ErrorType();
        errorType->setSession(this);

        initializerListType = new InitializerListType();
        initializerListType->setSession(this);

        overloadedType = new OverloadGroupType();
        overloadedType->setSession(this);
    }

    Type* Session::getBoolType()
    {
        return getBuiltinType(BaseType::Bool);
    }

    Type* Session::getHalfType()
    {
        return getBuiltinType(BaseType::Half);
    }

    Type* Session::getFloatType()
    {
        return getBuiltinType(BaseType::Float);
    }

    Type* Session::getDoubleType()
    {
        return getBuiltinType(BaseType::Double);
    }

    Type* Session::getIntType()
    {
        return getBuiltinType(BaseType::Int);
    }

    Type* Session::getInt64Type()
    {
        return getBuiltinType(BaseType::Int64);
    }

    Type* Session::getUIntType()
    {
        return getBuiltinType(BaseType::UInt);
    }

    Type* Session::getUInt64Type()
    {
        return getBuiltinType(BaseType::UInt64);
    }

    Type* Session::getVoidType()
    {
        return getBuiltinType(BaseType::Void);
    }

    Type* Session::getBuiltinType(BaseType flavor)
    {
        return builtinTypes[int(flavor)];
    }

    Type* Session::getInitializerListType()
    {
        return initializerListType;
    }

    Type* Session::getOverloadedType()
    {
        return overloadedType;
    }

    Type* Session::getErrorType()
    {
        return errorType;
    }

    Type* Session::getStringType()
    {
        if (stringType == nullptr)
        {
            auto stringTypeDecl = findMagicDecl(this, "StringType");
            stringType = DeclRefType::Create(this, makeDeclRef<Decl>(stringTypeDecl));
        }
        return stringType;
    }

    Type* Session::getEnumTypeType()
    {
        if (enumTypeType == nullptr)
        {
            auto enumTypeTypeDecl = findMagicDecl(this, "EnumTypeType");
            enumTypeType = DeclRefType::Create(this, makeDeclRef<Decl>(enumTypeTypeDecl));
        }
        return enumTypeType;
    }

    RefPtr<PtrType> Session::getPtrType(
        RefPtr<Type>    valueType)
    {
        return getPtrType(valueType, "PtrType").dynamicCast<PtrType>();
    }

        // Construct the type `Out<valueType>`
    RefPtr<OutType> Session::getOutType(RefPtr<Type> valueType)
    {
        return getPtrType(valueType, "OutType").dynamicCast<OutType>();
    }

    RefPtr<InOutType> Session::getInOutType(RefPtr<Type> valueType)
    {
        return getPtrType(valueType, "InOutType").dynamicCast<InOutType>();
    }

    RefPtr<RefType> Session::getRefType(RefPtr<Type> valueType)
    {
        return getPtrType(valueType, "RefType").dynamicCast<RefType>();
    }

    RefPtr<PtrTypeBase> Session::getPtrType(RefPtr<Type> valueType, char const* ptrTypeName)
    {
        auto genericDecl = findMagicDecl(this, ptrTypeName).dynamicCast<GenericDecl>();
        return getPtrType(valueType, genericDecl);
    }

    RefPtr<PtrTypeBase> Session::getPtrType(RefPtr<Type> valueType, GenericDecl* genericDecl)
    {
        auto typeDecl = genericDecl->inner;

        auto substitutions = new GenericSubstitution();
        substitutions->genericDecl = genericDecl;
        substitutions->args.add(valueType);

        auto declRef = DeclRef<Decl>(typeDecl.Ptr(), substitutions);
        auto rsType = DeclRefType::Create(
            this,
            declRef);
        return as<PtrTypeBase>( rsType);
    }

    RefPtr<ArrayExpressionType> Session::getArrayType(
        Type*   elementType,
        IntVal* elementCount)
    {
        RefPtr<ArrayExpressionType> arrayType = new ArrayExpressionType();
        arrayType->setSession(this);
        arrayType->baseType = elementType;
        arrayType->arrayLength = elementCount;
        return arrayType;
    }

    SyntaxClass<RefObject> Session::findSyntaxClass(Name* name)
    {
        SyntaxClass<RefObject> syntaxClass;
        if (mapNameToSyntaxClass.TryGetValue(name, syntaxClass))
            return syntaxClass;

        return SyntaxClass<RefObject>();
    }



    bool ArrayExpressionType::equalsImpl(Type* type)
    {
        auto arrType = as<ArrayExpressionType>(type);
        if (!arrType)
            return false;
        return (areValsEqual(arrayLength, arrType->arrayLength) && baseType->equals(arrType->baseType.Ptr()));
    }

    RefPtr<Val> ArrayExpressionType::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;
        auto elementType = baseType->substituteImpl(subst, &diff).as<Type>();
        auto arrlen = arrayLength->substituteImpl(subst, &diff).as<IntVal>();
        SLANG_ASSERT(arrlen);
        if (diff)
        {
            *ioDiff = 1;
            auto rsType = getArrayType(
                elementType,
                arrlen);
            return rsType;
        }
        return this;
    }

    RefPtr<Type> ArrayExpressionType::createCanonicalType()
    {
        auto canonicalElementType = baseType->getCanonicalType();
        auto canonicalArrayType = getArrayType(
            canonicalElementType,
            arrayLength);
        return canonicalArrayType;
    }
    HashCode ArrayExpressionType::getHashCode()
    {
        if (arrayLength)
            return (baseType->getHashCode() * 16777619) ^ arrayLength->getHashCode();
        else
            return baseType->getHashCode();
    }
    Slang::String ArrayExpressionType::toString()
    {
        if (arrayLength)
            return baseType->toString() + "[" + arrayLength->toString() + "]";
        else
            return baseType->toString() + "[]";
    }

    // DeclRefType

    String DeclRefType::toString()
    {
        return declRef.toString();
    }

    HashCode DeclRefType::getHashCode()
    {
        return (declRef.getHashCode() * 16777619) ^ (HashCode)(typeid(this).hash_code());
    }

    bool DeclRefType::equalsImpl(Type * type)
    {
        if (auto declRefType = as<DeclRefType>(type))
        {
            return declRef.equals(declRefType->declRef);
        }
        return false;
    }

    RefPtr<Type> DeclRefType::createCanonicalType()
    {
        // A declaration reference is already canonical
        return this;
    }

    //
    // RequirementWitness
    //

    RequirementWitness::RequirementWitness(RefPtr<Val> val)
        : m_flavor(Flavor::val)
        , m_obj(val)
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


    RequirementWitness RequirementWitness::specialize(SubstitutionSet const& subst)
    {
        switch(getFlavor())
        {
        default:
            SLANG_UNEXPECTED("unknown requirement witness flavor");
        case RequirementWitness::Flavor::none:
            return RequirementWitness();

        case RequirementWitness::Flavor::declRef:
            {
                int diff = 0;
                return RequirementWitness(
                    getDeclRef().SubstituteImpl(subst, &diff));
            }

        case RequirementWitness::Flavor::val:
            {
                auto val = getVal();
                SLANG_ASSERT(val);

                return RequirementWitness(
                    val->substitute(subst));
            }
        }
    }

    RequirementWitness tryLookUpRequirementWitness(
        SubtypeWitness* subtypeWitness,
        Decl*           requirementKey)
    {
        if(auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subtypeWitness))
        {
            if(auto inheritanceDeclRef = declaredSubtypeWitness->declRef.as<InheritanceDecl>())
            {
                // A conformance that was declared as part of an inheritance clause
                // will have built up a dictionary of the satisfying declarations
                // for each of its requirements.
                RequirementWitness requirementWitness;
                auto witnessTable = inheritanceDeclRef.getDecl()->witnessTable;
                if(witnessTable && witnessTable->requirementDictionary.TryGetValue(requirementKey, requirementWitness))
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
                    requirementWitness = requirementWitness.specialize(inheritanceDeclRef.substitutions);

                    return requirementWitness;
                }
            }
        }

        // TODO: should handle the transitive case here too

        return RequirementWitness();
    }

    RefPtr<Val> DeclRefType::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        if (!subst) return this;

        // the case we especially care about is when this type references a declaration
        // of a generic parameter, since that is what we might be substituting...
        if (auto genericTypeParamDecl = as<GenericTypeParamDecl>(declRef.getDecl()))
        {
            // search for a substitution that might apply to us
            for(auto s = subst.substitutions; s; s = s->outer)
            {
                auto genericSubst = s.as<GenericSubstitution>();
                if(!genericSubst)
                    continue;

                // the generic decl associated with the substitution list must be
                // the generic decl that declared this parameter
                auto genericDecl = genericSubst->genericDecl;
                if (genericDecl != genericTypeParamDecl->parentDecl)
                    continue;

                int index = 0;
                for (auto m : genericDecl->members)
                {
                    if (m.Ptr() == genericTypeParamDecl)
                    {
                        // We've found it, so return the corresponding specialization argument
                        (*ioDiff)++;
                        return genericSubst->args[index];
                    }
                    else if (auto typeParam = as<GenericTypeParamDecl>(m))
                    {
                        index++;
                    }
                    else if (auto valParam = as<GenericValueParamDecl>(m))
                    {
                        index++;
                    }
                    else
                    {
                    }
                }
            }
        }
        else if (auto globalGenParam = as<GlobalGenericParamDecl>(declRef.getDecl()))
        {
            // search for a substitution that might apply to us
            for(auto s = subst.substitutions; s; s = s->outer)
            {
                auto genericSubst = as<GlobalGenericParamSubstitution>(s);
                if(!genericSubst)
                    continue;

                if (genericSubst->paramDecl == globalGenParam)
                {
                    (*ioDiff)++;
                    return genericSubst->actualType;
                }
            }
        }
        int diff = 0;
        DeclRef<Decl> substDeclRef = declRef.SubstituteImpl(subst, &diff);

        if (!diff)
            return this;

        // Make sure to record the difference!
        *ioDiff += diff;

        // If this type is a reference to an associated type declaration,
        // and the substitutions provide a "this type" substitution for
        // the outer interface, then try to replace the type with the
        // actual value of the associated type for the given implementation.
        //
        if(auto substAssocTypeDecl = as<AssocTypeDecl>(substDeclRef.decl))
        {
            for(auto s = substDeclRef.substitutions.substitutions; s; s = s->outer)
            {
                auto thisSubst = s.as<ThisTypeSubstitution>();
                if(!thisSubst)
                    continue;

                if(auto interfaceDecl = as<InterfaceDecl>(substAssocTypeDecl->parentDecl))
                {
                    if(thisSubst->interfaceDecl == interfaceDecl)
                    {
                        // We need to look up the declaration that satisfies
                        // the requirement named by the associated type.
                        Decl* requirementKey = substAssocTypeDecl;
                        RequirementWitness requirementWitness = tryLookUpRequirementWitness(thisSubst->witness, requirementKey);
                        switch(requirementWitness.getFlavor())
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
                    }
                }
            }
        }

        // Re-construct the type in case we are using a specialized sub-class
        return DeclRefType::Create(getSession(), substDeclRef);
    }

    static RefPtr<Type> ExtractGenericArgType(RefPtr<Val> val)
    {
        auto type = val.as<Type>();
        SLANG_RELEASE_ASSERT(type.Ptr());
        return type;
    }

    static RefPtr<IntVal> ExtractGenericArgInteger(RefPtr<Val> val)
    {
        auto intVal = val.as<IntVal>();
        SLANG_RELEASE_ASSERT(intVal.Ptr());
        return intVal;
    }

    DeclRef<Decl> createDefaultSubstitutionsIfNeeded(
        Session*        session,
        DeclRef<Decl>   declRef)
    {
        // It is possible that `declRef` refers to a generic type,
        // but does not specify arguments for its generic parameters.
        // (E.g., this happens when referring to a generic type from
        // within its own member functions). To handle this case,
        // we will construct a default specialization at the use
        // site if needed.
        //
        // This same logic should also apply to declarations nested
        // more than one level inside of a generic (e.g., a `typdef`
        // inside of a generic `struct`).
        //
        // Similarly, it needs to work for multiple levels of
        // nested generics.
        //

        // We are going to build up a list of substitutions that need
        // to be applied to the decl-ref to make it specialized.
        RefPtr<Substitutions> substsToApply;
        RefPtr<Substitutions>* link = &substsToApply;

        RefPtr<Decl> dd = declRef.getDecl();
        for(;;)
        {
            RefPtr<Decl> childDecl = dd;
            RefPtr<Decl> parentDecl = dd->parentDecl;
            if(!parentDecl)
                break;

            dd = parentDecl;

            if(auto genericParentDecl = parentDecl.as<GenericDecl>())
            {
                // Don't specialize any parameters of a generic.
                if(childDecl != genericParentDecl->inner)
                    break;

                // We have a generic ancestor, but do we have an substitutions for it?
                RefPtr<GenericSubstitution> foundSubst;
                for(auto s = declRef.substitutions.substitutions; s; s = s->outer)
                {
                    auto genSubst = s.as<GenericSubstitution>();
                    if(!genSubst)
                        continue;

                    if(genSubst->genericDecl != genericParentDecl)
                        continue;

                    // Okay, we found a matching substitution,
                    // so there is nothing to be done.
                    foundSubst = genSubst;
                    break;
                }

                if(!foundSubst)
                {
                    RefPtr<Substitutions> newSubst = createDefaultSubsitutionsForGeneric(
                        session,
                        genericParentDecl,
                        nullptr);

                    *link = newSubst;
                    link = &newSubst->outer;
                }
            }
        }

        if(!substsToApply)
            return declRef;

        int diff = 0;
        return declRef.SubstituteImpl(substsToApply, &diff);
    }

    // TODO: need to figure out how to unify this with the logic
    // in the generic case...
    RefPtr<DeclRefType> DeclRefType::Create(
        Session*        session,
        DeclRef<Decl>   declRef)
    {
        declRef = createDefaultSubstitutionsIfNeeded(session, declRef);

        if (auto builtinMod = declRef.getDecl()->findModifier<BuiltinTypeModifier>())
        {
            auto type = new BasicExpressionType(builtinMod->tag);
            type->setSession(session);
            type->declRef = declRef;
            return type;
        }
        else if (auto magicMod = declRef.getDecl()->findModifier<MagicTypeModifier>())
        {
            GenericSubstitution* subst = nullptr;
            for(auto s = declRef.substitutions.substitutions; s; s = s->outer)
            {
                if(auto genericSubst = s.as<GenericSubstitution>())
                {
                    subst = genericSubst;
                    break;
                }
            }

            if (magicMod->name == "SamplerState")
            {
                auto type = new SamplerStateType();
                type->setSession(session);
                type->declRef = declRef;
                type->flavor = SamplerStateFlavor(magicMod->tag);
                return type;
            }
            else if (magicMod->name == "Vector")
            {
                SLANG_ASSERT(subst && subst->args.getCount() == 2);
                auto vecType = new VectorExpressionType();
                vecType->setSession(session);
                vecType->declRef = declRef;
                vecType->elementType = ExtractGenericArgType(subst->args[0]);
                vecType->elementCount = ExtractGenericArgInteger(subst->args[1]);
                return vecType;
            }
            else if (magicMod->name == "Matrix")
            {
                SLANG_ASSERT(subst && subst->args.getCount() == 3);
                auto matType = new MatrixExpressionType();
                matType->setSession(session);
                matType->declRef = declRef;
                return matType;
            }
            else if (magicMod->name == "Texture")
            {
                SLANG_ASSERT(subst && subst->args.getCount() >= 1);
                auto textureType = new TextureType(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "TextureSampler")
            {
                SLANG_ASSERT(subst && subst->args.getCount() >= 1);
                auto textureType = new TextureSamplerType(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "GLSLImageType")
            {
                SLANG_ASSERT(subst && subst->args.getCount() >= 1);
                auto textureType = new GLSLImageType(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }

            // TODO: eventually everything should follow this pattern,
            // and we can drive the dispatch with a table instead
            // of this ridiculously slow `if` cascade.

        #define CASE(n,T)													\
            else if(magicMod->name == #n) {									\
                auto type = new T();									    \
                type->setSession(session);                                  \
                type->declRef = declRef;									\
                return type;												\
            }

            CASE(HLSLInputPatchType, HLSLInputPatchType)
            CASE(HLSLOutputPatchType, HLSLOutputPatchType)

        #undef CASE

            #define CASE(n,T)													\
                else if(magicMod->name == #n) {									\
                    SLANG_ASSERT(subst && subst->args.getCount() == 1);			\
                    auto type = new T();									    \
                    type->setSession(session);                                  \
                    type->elementType = ExtractGenericArgType(subst->args[0]);	\
                    type->declRef = declRef;									\
                    return type;												\
                }

            CASE(ConstantBuffer, ConstantBufferType)
            CASE(TextureBuffer, TextureBufferType)
            CASE(ParameterBlockType, ParameterBlockType)
            CASE(GLSLInputParameterGroupType, GLSLInputParameterGroupType)
            CASE(GLSLOutputParameterGroupType, GLSLOutputParameterGroupType)
            CASE(GLSLShaderStorageBufferType, GLSLShaderStorageBufferType)

            CASE(HLSLStructuredBufferType, HLSLStructuredBufferType)
            CASE(HLSLRWStructuredBufferType, HLSLRWStructuredBufferType)
            CASE(HLSLRasterizerOrderedStructuredBufferType, HLSLRasterizerOrderedStructuredBufferType)
            CASE(HLSLAppendStructuredBufferType, HLSLAppendStructuredBufferType)
            CASE(HLSLConsumeStructuredBufferType, HLSLConsumeStructuredBufferType)

            CASE(HLSLPointStreamType, HLSLPointStreamType)
            CASE(HLSLLineStreamType, HLSLLineStreamType)
            CASE(HLSLTriangleStreamType, HLSLTriangleStreamType)

            #undef CASE

            // "magic" builtin types which have no generic parameters
            #define CASE(n,T)													\
                else if(magicMod->name == #n) {									\
                    auto type = new T();									    \
                    type->setSession(session);                                  \
                    type->declRef = declRef;									\
                    return type;												\
                }

            CASE(HLSLByteAddressBufferType, HLSLByteAddressBufferType)
            CASE(HLSLRWByteAddressBufferType, HLSLRWByteAddressBufferType)
            CASE(HLSLRasterizerOrderedByteAddressBufferType, HLSLRasterizerOrderedByteAddressBufferType)
            CASE(UntypedBufferResourceType, UntypedBufferResourceType)

            CASE(GLSLInputAttachmentType, GLSLInputAttachmentType)

            #undef CASE

            else
            {
                auto classInfo = session->findSyntaxClass(
                    session->getNamePool()->getName(magicMod->name));
                if (!classInfo.classInfo)
                {
                    SLANG_UNEXPECTED("unhandled type");
                }

                RefPtr<RefObject> type = classInfo.createInstance();
                if (!type)
                {
                    SLANG_UNEXPECTED("constructor failure");
                }

                auto declRefType = dynamicCast<DeclRefType>(type);
                if (!declRefType)
                {
                    SLANG_UNEXPECTED("expected a declaration reference type");
                }
                declRefType->session = session;
                declRefType->declRef = declRef;
                return declRefType;
            }
        }
        else
        {
            auto type = new DeclRefType(declRef);
            type->setSession(session);
            return type;
        }
    }

    // OverloadGroupType

    String OverloadGroupType::toString()
    {
        return "overload group";
    }

    bool OverloadGroupType::equalsImpl(Type * /*type*/)
    {
        return false;
    }

    RefPtr<Type> OverloadGroupType::createCanonicalType()
    {
        return this;
    }

    HashCode OverloadGroupType::getHashCode()
    {
        return (HashCode)(size_t(this));
    }

    // InitializerListType

    String InitializerListType::toString()
    {
        return "initializer list";
    }

    bool InitializerListType::equalsImpl(Type * /*type*/)
    {
        return false;
    }

    RefPtr<Type> InitializerListType::createCanonicalType()
    {
        return this;
    }

    HashCode InitializerListType::getHashCode()
    {
        return (HashCode)(size_t(this));
    }

    // ErrorType

    String ErrorType::toString()
    {
        return "error";
    }

    bool ErrorType::equalsImpl(Type* type)
    {
        if (auto errorType = as<ErrorType>(type))
            return true;
        return false;
    }

    RefPtr<Type> ErrorType::createCanonicalType()
    {
        return this;
    }

    RefPtr<Val> ErrorType::substituteImpl(SubstitutionSet /*subst*/, int* /*ioDiff*/)
    {
        return this;
    }

    HashCode ErrorType::getHashCode()
    {
        return HashCode(size_t(this));
    }


    // NamedExpressionType

    String NamedExpressionType::toString()
    {
        return getText(declRef.GetName());
    }

    bool NamedExpressionType::equalsImpl(Type * /*type*/)
    {
        SLANG_UNEXPECTED("unreachable");
        UNREACHABLE_RETURN(false);
    }

    RefPtr<Type> NamedExpressionType::createCanonicalType()
    {
        if (!innerType)
            innerType = GetType(declRef);
        return innerType->getCanonicalType();
    }

    HashCode NamedExpressionType::getHashCode()
    {
        // Type equality is based on comparing canonical types,
        // so the hash code for a type needs to come from the
        // canonical version of the type. This really means
        // that `Type::getHashCode()` should dispatch out to
        // something like `Type::getHashCodeImpl()` on the
        // canonical version of a type, but it is less invasive
        // for now (and hopefully equivalent) to just have any
        // named types automaticlaly route hash-code requests
        // to their canonical type.
        return getCanonicalType()->getHashCode();
    }

    // FuncType

    String FuncType::toString()
    {
        StringBuilder sb;
        sb << "(";
        UInt paramCount = getParamCount();
        for (UInt pp = 0; pp < paramCount; ++pp)
        {
            if (pp != 0) sb << ", ";
            sb << getParamType(pp)->toString();
        }
        sb << ") -> ";
        sb << getResultType()->toString();
        return sb.ProduceString();
    }

    bool FuncType::equalsImpl(Type * type)
    {
        if (auto funcType = as<FuncType>(type))
        {
            auto paramCount = getParamCount();
            auto otherParamCount = funcType->getParamCount();
            if (paramCount != otherParamCount)
                return false;

            for (UInt pp = 0; pp < paramCount; ++pp)
            {
                auto paramType = getParamType(pp);
                auto otherParamType = funcType->getParamType(pp);
                if (!paramType->equals(otherParamType))
                    return false;
            }

            if(!resultType->equals(funcType->resultType))
                return false;

            // TODO: if we ever introduce other kinds
            // of qualification on function types, we'd
            // want to consider it here.
            return true;
        }
        return false;
    }

    RefPtr<Val> FuncType::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;

        // result type
        RefPtr<Type> substResultType = resultType->substituteImpl(subst, &diff).as<Type>();

        // parameter types
        List<RefPtr<Type>> substParamTypes;
        for( auto pp : paramTypes )
        {
            substParamTypes.add(pp->substituteImpl(subst, &diff).as<Type>());
        }

        // early exit for no change...
        if(!diff)
            return this;

        (*ioDiff)++;
        RefPtr<FuncType> substType = new FuncType();
        substType->session = session;
        substType->resultType = substResultType;
        substType->paramTypes = substParamTypes;
        return substType;
    }

    RefPtr<Type> FuncType::createCanonicalType()
    {
        // result type
        RefPtr<Type> canResultType = resultType->getCanonicalType();

        // parameter types
        List<RefPtr<Type>> canParamTypes;
        for( auto pp : paramTypes )
        {
            canParamTypes.add(pp->getCanonicalType());
        }

        RefPtr<FuncType> canType = new FuncType();
        canType->session = session;
        canType->resultType = resultType;
        canType->paramTypes = canParamTypes;

        return canType;
    }

    HashCode FuncType::getHashCode()
    {
        HashCode hashCode = getResultType()->getHashCode();
        UInt paramCount = getParamCount();
        hashCode = combineHash(hashCode, Slang::getHashCode(paramCount));
        for (UInt pp = 0; pp < paramCount; ++pp)
        {
            hashCode = combineHash(
                hashCode,
                getParamType(pp)->getHashCode());
        }
        return hashCode;
    }

    // TypeType

    String TypeType::toString()
    {
        StringBuilder sb;
        sb << "typeof(" << type->toString() << ")";
        return sb.ProduceString();
    }

    bool TypeType::equalsImpl(Type * t)
    {
        if (auto typeType = as<TypeType>(t))
        {
            return t->equals(typeType->type);
        }
        return false;
    }

    RefPtr<Type> TypeType::createCanonicalType()
    {
        auto canType = getTypeType(type->getCanonicalType());
        return canType;
    }

    HashCode TypeType::getHashCode()
    {
        SLANG_UNEXPECTED("unreachable");
        UNREACHABLE_RETURN(0);
    }

    // GenericDeclRefType

    String GenericDeclRefType::toString()
    {
        // TODO: what is appropriate here?
        return "<DeclRef<GenericDecl>>";
    }

    bool GenericDeclRefType::equalsImpl(Type * type)
    {
        if (auto genericDeclRefType = as<GenericDeclRefType>(type))
        {
            return declRef.equals(genericDeclRefType->declRef);
        }
        return false;
    }

    HashCode GenericDeclRefType::getHashCode()
    {
        return declRef.getHashCode();
    }

    RefPtr<Type> GenericDeclRefType::createCanonicalType()
    {
        return this;
    }

    // NamespaceType

    String NamespaceType::toString()
    {
        String result;
        result.append("namespace ");
        result.append(declRef.toString());
        return result;
    }

    bool NamespaceType::equalsImpl(Type * type)
    {
        if (auto namespaceType = as<NamespaceType>(type))
        {
            return declRef.equals(namespaceType->declRef);
        }
        return false;
    }

    HashCode NamespaceType::getHashCode()
    {
        return declRef.getHashCode();
    }

    RefPtr<Type> NamespaceType::createCanonicalType()
    {
        return this;
    }

    // ArithmeticExpressionType

    // VectorExpressionType

    String VectorExpressionType::toString()
    {
        StringBuilder sb;
        sb << "vector<" << elementType->toString() << "," << elementCount->toString() << ">";
        return sb.ProduceString();
    }

    BasicExpressionType* VectorExpressionType::GetScalarType()
    {
        return as<BasicExpressionType>(elementType);
    }

    //

    RefPtr<GenericSubstitution> findInnerMostGenericSubstitution(Substitutions* subst)
    {
        for(RefPtr<Substitutions> s = subst; s; s = s->outer)
        {
            if(auto genericSubst = as<GenericSubstitution>(s))
                return genericSubst;
        }
        return nullptr;
    }

    // MatrixExpressionType

    String MatrixExpressionType::toString()
    {
        StringBuilder sb;
        sb << "matrix<" << getElementType()->toString() << "," << getRowCount()->toString() << "," << getColumnCount()->toString() << ">";
        return sb.ProduceString();
    }

    BasicExpressionType* MatrixExpressionType::GetScalarType()
    {
        return as<BasicExpressionType>(getElementType()); 
    }

    Type* MatrixExpressionType::getElementType()
    {
        return as<Type>(findInnerMostGenericSubstitution(declRef.substitutions)->args[0]);
    }

    IntVal* MatrixExpressionType::getRowCount()
    {
        return as<IntVal>(findInnerMostGenericSubstitution(declRef.substitutions)->args[1]);
    }

    IntVal* MatrixExpressionType::getColumnCount()
    {
        return as<IntVal>(findInnerMostGenericSubstitution(declRef.substitutions)->args[2]);
    }

    RefPtr<Type> MatrixExpressionType::getRowType()
    {
        if( !rowType )
        {
            rowType = getSession()->getVectorType(getElementType(), getColumnCount());
        }
        return rowType;
    }

    RefPtr<VectorExpressionType> Session::getVectorType(
        RefPtr<Type>    elementType,
        RefPtr<IntVal>  elementCount)
    {
        auto vectorGenericDecl = findMagicDecl(
            this, "Vector").as<GenericDecl>();
        auto vectorTypeDecl = vectorGenericDecl->inner;

        auto substitutions = new GenericSubstitution();
        substitutions->genericDecl = vectorGenericDecl.Ptr();
        substitutions->args.add(elementType);
        substitutions->args.add(elementCount);

        auto declRef = DeclRef<Decl>(vectorTypeDecl.Ptr(), substitutions);

        return DeclRefType::Create(
            this,
            declRef).as<VectorExpressionType>();
    }


    // PtrTypeBase

    Type* PtrTypeBase::getValueType()
    {
        return as<Type>(findInnerMostGenericSubstitution(declRef.substitutions)->args[0]);
    }

    // GenericParamIntVal

    bool GenericParamIntVal::equalsVal(Val* val)
    {
        if (auto genericParamVal = as<GenericParamIntVal>(val))
        {
            return declRef.equals(genericParamVal->declRef);
        }
        return false;
    }

    String GenericParamIntVal::toString()
    {
        return getText(declRef.GetName());
    }

    HashCode GenericParamIntVal::getHashCode()
    {
        return declRef.getHashCode() ^ HashCode(0xFFFF);
    }

    RefPtr<Val> GenericParamIntVal::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        // search for a substitution that might apply to us
        for(auto s = subst.substitutions; s; s = s->outer)
        {
            auto genSubst = s.as<GenericSubstitution>();
            if(!genSubst)
                continue;

            // the generic decl associated with the substitution list must be
            // the generic decl that declared this parameter
            auto genericDecl = genSubst->genericDecl;
            if (genericDecl != declRef.getDecl()->parentDecl)
                continue;

            int index = 0;
            for (auto m : genericDecl->members)
            {
                if (m.Ptr() == declRef.getDecl())
                {
                    // We've found it, so return the corresponding specialization argument
                    (*ioDiff)++;
                    return genSubst->args[index];
                }
                else if (auto typeParam = as<GenericTypeParamDecl>(m))
                {
                    index++;
                }
                else if (auto valParam = as<GenericValueParamDecl>(m))
                {
                    index++;
                }
                else
                {
                }
            }
        }

        // Nothing found: don't substitute.
        return this;
    }

    // ErrorIntVal

    bool ErrorIntVal::equalsVal(Val* val)
    {
        if( auto errorIntVal = as<ErrorIntVal>(val) )
        {
            return true;
        }
        return false;
    }

    String ErrorIntVal::toString()
    {
        return "<error>";
    }

    HashCode ErrorIntVal::getHashCode()
    {
        return HashCode(typeid(this).hash_code());
    }

    RefPtr<Val> ErrorIntVal::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        SLANG_UNUSED(subst);
        SLANG_UNUSED(ioDiff);
        return this;
    }

    // Substitutions

    RefPtr<Substitutions> GenericSubstitution::applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)
    {
        int diff = 0;

        if(substOuter != outer) diff++;

        List<RefPtr<Val>> substArgs;
        for (auto a : args)
        {
            substArgs.add(a->substituteImpl(substSet, &diff));
        }

        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new GenericSubstitution();
        substSubst->genericDecl = genericDecl;
        substSubst->args = substArgs;
        substSubst->outer = substOuter;
        return substSubst;
    }

    bool GenericSubstitution::equals(Substitutions* subst)
    {
        // both must be NULL, or non-NULL
        if (subst == nullptr)
            return false;
        if (this == subst)
            return true;

        auto genericSubst = as<GenericSubstitution>(subst);
        if (!genericSubst)
            return false;
        if (genericDecl != genericSubst->genericDecl)
            return false;

        Index argCount = args.getCount();
        SLANG_RELEASE_ASSERT(args.getCount() == genericSubst->args.getCount());
        for (Index aa = 0; aa < argCount; ++aa)
        {
            if (!args[aa]->equalsVal(genericSubst->args[aa].Ptr()))
                return false;
        }

        if (!outer)
            return !genericSubst->outer;

        if (!outer->equals(genericSubst->outer.Ptr()))
            return false;

        return true;
    }

    RefPtr<Substitutions> ThisTypeSubstitution::applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)
    {
        int diff = 0;

        if(substOuter != outer) diff++;

        // NOTE: Must use .as because we must have a smart pointer here to keep in scope.
        auto substWitness = witness->substituteImpl(substSet, &diff).as<SubtypeWitness>();
        
        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new ThisTypeSubstitution();
        substSubst->interfaceDecl = interfaceDecl;
        substSubst->witness = substWitness;
        substSubst->outer = substOuter;
        return substSubst;
    }

    bool ThisTypeSubstitution::equals(Substitutions* subst)
    {
        if (!subst)
            return false;
        if (subst == this)
            return true;

        if (auto thisTypeSubst = as<ThisTypeSubstitution>(subst))
        {
            // For our purposes, two this-type substitutions are
            // equivalent if they have the same type as `This`,
            // even if the specific witness values they use
            // might differ.
            //
            if(this->interfaceDecl != thisTypeSubst->interfaceDecl)
                return false;

            if(!this->witness->sub->equals(thisTypeSubst->witness->sub))
                return false;

            return true;
        }
        return false;
    }

    HashCode ThisTypeSubstitution::getHashCode() const
    {
        return witness->getHashCode();
    }

    RefPtr<Substitutions> GlobalGenericParamSubstitution::applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)
    {
        // if we find a GlobalGenericParamSubstitution in subst that references the same type_param decl
        // return a copy of that GlobalGenericParamSubstitution
        int diff = 0;

        if(substOuter != outer) diff++;

        auto substActualType = actualType->substituteImpl(substSet, &diff).as<Type>();

        List<ConstraintArg> substConstraintArgs;
        for(auto constraintArg : constraintArgs)
        {
            ConstraintArg substConstraintArg;
            substConstraintArg.decl = constraintArg.decl;
            substConstraintArg.val = constraintArg.val->substituteImpl(substSet, &diff);

            substConstraintArgs.add(substConstraintArg);
        }

        if(!diff)
            return this;

        (*ioDiff)++;

        RefPtr<GlobalGenericParamSubstitution> substSubst = new GlobalGenericParamSubstitution();
        substSubst->paramDecl = paramDecl;
        substSubst->actualType = substActualType;
        substSubst->constraintArgs = substConstraintArgs;
        substSubst->outer = substOuter;
        return substSubst;
    }

    bool GlobalGenericParamSubstitution::equals(Substitutions* subst)
    {
        if (!subst)
            return false;
        if (subst == this)
            return true;

        if (auto genSubst = as<GlobalGenericParamSubstitution>(subst))
        {
            if (paramDecl != genSubst->paramDecl)
                return false;
            if (!actualType->equalsVal(genSubst->actualType))
                return false;
            if (constraintArgs.getCount() != genSubst->constraintArgs.getCount())
                return false;
            for (Index i = 0; i < constraintArgs.getCount(); i++)
            {
                if (!constraintArgs[i].val->equalsVal(genSubst->constraintArgs[i].val))
                    return false;
            }
            return true;
        }
        return false;
    }


    // DeclRefBase

    RefPtr<Type> DeclRefBase::Substitute(RefPtr<Type> type) const
    {
        // Note that type can be nullptr, and so this function can return nullptr (although only correctly when no substitutions) 

        // No substitutions? Easy.
        if (!substitutions)
            return type;

        SLANG_ASSERT(type);

        // Otherwise we need to recurse on the type structure
        // and apply substitutions where it makes sense
        return type->substitute(substitutions).as<Type>();
    }

    DeclRefBase DeclRefBase::Substitute(DeclRefBase declRef) const
    {
        if(!substitutions)
            return declRef;

        int diff = 0;
        return declRef.SubstituteImpl(substitutions, &diff);
    }

    RefPtr<Expr> DeclRefBase::Substitute(RefPtr<Expr> expr) const
    {
        // No substitutions? Easy.
        if (!substitutions)
            return expr;

        SLANG_UNIMPLEMENTED_X("generic substitution into expressions");

        UNREACHABLE_RETURN(expr);
    }

    void buildMemberDictionary(ContainerDecl* decl);

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

    RefPtr<GlobalGenericParamSubstitution> findGlobalGenericSubst(
        RefPtr<Substitutions>   substs,
        GlobalGenericParamDecl* paramDecl)
    {
        for(auto s = substs; s; s = s->outer)
        {
            auto gSubst = s.as<GlobalGenericParamSubstitution>();
            if(!gSubst)
                continue;

            if(gSubst->paramDecl != paramDecl)
                continue;

            return gSubst;
        }

        return nullptr;
    }

    RefPtr<Substitutions> specializeSubstitutionsShallow(
        RefPtr<Substitutions>   substToSpecialize,
        RefPtr<Substitutions>   substsToApply,
        RefPtr<Substitutions>   restSubst,
        int*                    ioDiff)
    {
        SLANG_ASSERT(substToSpecialize);
        return substToSpecialize->applySubstitutionsShallow(substsToApply, restSubst, ioDiff);
    }

    RefPtr<Substitutions> specializeGlobalGenericSubstitutions(
        Decl*                               declToSpecialize,
        RefPtr<Substitutions>               substsToSpecialize,
        RefPtr<Substitutions>               substsToApply,
        int*                                ioDiff,
        HashSet<GlobalGenericParamDecl*>&   ioParametersFound)
    {
        // Any existing global-generic substitutions will trigger
        // a recursive case that skips the rest of the function.
        for(auto specSubst = substsToSpecialize; specSubst; specSubst = specSubst->outer)
        {
            auto specGlobalGenericSubst = specSubst.as<GlobalGenericParamSubstitution>();
            if(!specGlobalGenericSubst)
                continue;

            ioParametersFound.Add(specGlobalGenericSubst->paramDecl);

            int diff = 0;
            auto restSubst = specializeGlobalGenericSubstitutions(
                declToSpecialize,
                specSubst->outer,
                substsToApply,
                &diff,
                ioParametersFound);

            auto firstSubst = specializeSubstitutionsShallow(
                specGlobalGenericSubst,
                substsToApply,
                restSubst,
                &diff);

            *ioDiff += diff;
            return firstSubst;
        }

        // No more existing substitutions, so we know we can apply
        // our global generic substitutions without any special work.

        // We expect global generic substitutions to come at
        // the end of the list in all cases, so lets advance
        // until we see them.
        RefPtr<Substitutions> appGlobalGenericSubsts = substsToApply;
        while(appGlobalGenericSubsts && !appGlobalGenericSubsts.as<GlobalGenericParamSubstitution>())
            appGlobalGenericSubsts = appGlobalGenericSubsts->outer;


        // If there is nothing to apply, then we are done
        if(!appGlobalGenericSubsts)
            return nullptr;

        // Otherwise, it seems like something has to change.
        (*ioDiff)++;

        // If there were no parameters bound by the existing substitution,
        // then we can safely use the global generics from the to-apply set.
        if(ioParametersFound.Count() == 0)
            return appGlobalGenericSubsts;

        RefPtr<Substitutions> resultSubst;
        RefPtr<Substitutions>* link = &resultSubst;
        for(auto appSubst = appGlobalGenericSubsts; appSubst; appSubst = appSubst->outer)
        {
            auto appGlobalGenericSubst = appSubst.as<GlobalGenericParamSubstitution>();
            if(!appSubst)
                continue;

            // Don't include substitutions for parameters already handled.
            if(ioParametersFound.Contains(appGlobalGenericSubst->paramDecl))
                continue;

            RefPtr<GlobalGenericParamSubstitution> newSubst = new GlobalGenericParamSubstitution();
            newSubst->paramDecl = appGlobalGenericSubst->paramDecl;
            newSubst->actualType = appGlobalGenericSubst->actualType;
            newSubst->constraintArgs = appGlobalGenericSubst->constraintArgs;

            *link = newSubst;
            link = &newSubst->outer;
        }

        return resultSubst;
    }

    RefPtr<Substitutions> specializeGlobalGenericSubstitutions(
        Decl*                   declToSpecialize,
        RefPtr<Substitutions>   substsToSpecialize,
        RefPtr<Substitutions>   substsToApply,
        int*                    ioDiff)
    {
        // Keep track of any parameters already present in the
        // existing substitution.
        HashSet<GlobalGenericParamDecl*> parametersFound;
        return specializeGlobalGenericSubstitutions(declToSpecialize, substsToSpecialize, substsToApply, ioDiff, parametersFound);
    }


    // Construct new substitutions to apply to a declaration,
    // based on a provided substitution set to be applied
    RefPtr<Substitutions> specializeSubstitutions(
        Decl*                   declToSpecialize,
        RefPtr<Substitutions>   substsToSpecialize,
        RefPtr<Substitutions>   substsToApply,
        int*                    ioDiff)
    {
        // No declaration? Then nothing to specialize.
        if(!declToSpecialize)
            return nullptr;

        // No (remaining) substitutions to apply? Then we are done.
        if(!substsToApply)
            return substsToSpecialize;

        // Walk the hierarchy of the declaration to determine what specializations might apply.
        // We assume that the `substsToSpecialize` must be aligned with the ancestor
        // hierarchy of `declToSpecialize` such that if, e.g., the `declToSpecialize` is
        // nested directly in a generic, then `substToSpecialize` will either start with
        // the corresponding `GenericSubstitution` or there will be *no* generic substitutions
        // corresponding to that decl.
        for(Decl* ancestorDecl = declToSpecialize; ancestorDecl; ancestorDecl = ancestorDecl->parentDecl)
        {
            if(auto ancestorGenericDecl = as<GenericDecl>(ancestorDecl))
            {
                // The declaration is nested inside a generic.
                // Does it already have a specialization for that generic?
                if(auto specGenericSubst = as<GenericSubstitution>(substsToSpecialize))
                {
                    if(specGenericSubst->genericDecl == ancestorGenericDecl)
                    {
                        // Yes. We have an existing specialization, so we will
                        // keep one matching it in place.
                        int diff = 0;
                        auto restSubst = specializeSubstitutions(
                            ancestorGenericDecl->parentDecl,
                            specGenericSubst->outer,
                            substsToApply,
                            &diff);

                        auto firstSubst = specializeSubstitutionsShallow(
                            specGenericSubst,
                            substsToApply,
                            restSubst,
                            &diff);

                        *ioDiff += diff;
                        return firstSubst;
                    }
                }

                // If the declaration is not already specialized
                // for the given generic, then see if we are trying
                // to *apply* such specializations to it.
                //
                // TODO: The way we handle things right now with
                // "default" specializations, this case shouldn't
                // actually come up.
                //
                for(auto s = substsToApply; s; s = s->outer)
                {
                    auto appGenericSubst = as<GenericSubstitution>(s);
                    if(!appGenericSubst)
                        continue;

                    if(appGenericSubst->genericDecl != ancestorGenericDecl)
                        continue;

                    // The substitutions we are applying are trying
                    // to specialize this generic, but we don't already
                    // have a generic substitution in place.
                    // We will need to create one.

                    int diff = 0;
                    auto restSubst = specializeSubstitutions(
                        ancestorGenericDecl->parentDecl,
                        substsToSpecialize,
                        substsToApply,
                        &diff);

                    RefPtr<GenericSubstitution> firstSubst = new GenericSubstitution();
                    firstSubst->genericDecl = ancestorGenericDecl;
                    firstSubst->args = appGenericSubst->args;
                    firstSubst->outer = restSubst;

                    (*ioDiff)++;
                    return firstSubst;
                }
            }
            else if(auto ancestorInterfaceDecl = as<InterfaceDecl>(ancestorDecl))
            {
                // The task is basically the same as for the generic case:
                // We want to see if there is any existing substitution that
                // applies to this declaration, and use that if possible.

                // The declaration is nested inside a generic.
                // Does it already have a specialization for that generic?
                if(auto specThisTypeSubst = as<ThisTypeSubstitution>(substsToSpecialize))
                {
                    if(specThisTypeSubst->interfaceDecl == ancestorInterfaceDecl)
                    {
                        // Yes. We have an existing specialization, so we will
                        // keep one matching it in place.
                        int diff = 0;
                        auto restSubst = specializeSubstitutions(
                            ancestorInterfaceDecl->parentDecl,
                            specThisTypeSubst->outer,
                            substsToApply,
                            &diff);

                        auto firstSubst = specializeSubstitutionsShallow(
                            specThisTypeSubst,
                            substsToApply,
                            restSubst,
                            &diff);

                        *ioDiff += diff;
                        return firstSubst;
                    }
                }

                // Otherwise, check if we are trying to apply
                // a this-type substitution to the given interface
                //
                for(auto s = substsToApply; s; s = s->outer)
                {
                    auto appThisTypeSubst = s.as<ThisTypeSubstitution>();
                    if(!appThisTypeSubst)
                        continue;

                    if(appThisTypeSubst->interfaceDecl != ancestorInterfaceDecl)
                        continue;

                    int diff = 0;
                    auto restSubst = specializeSubstitutions(
                        ancestorInterfaceDecl->parentDecl,
                        substsToSpecialize,
                        substsToApply,
                        &diff);

                    RefPtr<ThisTypeSubstitution> firstSubst = new ThisTypeSubstitution();
                    firstSubst->interfaceDecl = ancestorInterfaceDecl;
                    firstSubst->witness = appThisTypeSubst->witness;
                    firstSubst->outer = restSubst;

                    (*ioDiff)++;
                    return firstSubst;
                }
            }
        }

        // If we reach here then we've walked the full hierarchy up from
        // `declToSpecialize` and either didn't run into an generic/interface
        // declarations, or we didn't find any attempt to specialize them
        // in either substitution.
        //
        // As an invariant, there should *not* be any generic or this-type
        // substitutions in `substToSpecialize`, because otherwise they
        // would be specializations that don't actually apply to the given
        // declaration.
        //
        // The remaining substitutions to apply, if any, should thus be
        // global-generic substitutions. And similarly, those are the
        // only remaining substitutions we really care about in
        // `substsToApply`.
        //
        // Note: this does *not* mean that `substsToApply` doesn't have
        // any generic or this-type substitutions; it just means that none
        // of them were applicable.
        //
        return specializeGlobalGenericSubstitutions(
            declToSpecialize,
            substsToSpecialize,
            substsToApply,
            ioDiff);
    }

    DeclRefBase DeclRefBase::SubstituteImpl(SubstitutionSet substSet, int* ioDiff)
    {
        // Nothing to do when we have no declaration.
        if(!decl)
            return *this;

        // Apply the given substitutions to any specializations
        // that have already been applied to this declaration.
        int diff = 0;

        auto substSubst = specializeSubstitutions(
            decl,
            substitutions.substitutions,
            substSet.substitutions,
            &diff);

        if (!diff)
            return *this;

        *ioDiff += diff;

        DeclRefBase substDeclRef;
        substDeclRef.decl = decl;
        substDeclRef.substitutions = substSubst;

        // TODO: The old code here used to try to translate a decl-ref
        // to an associated type in a decl-ref for the concrete type
        // in a particular implementation.
        //
        // I have only kept that logic in `DeclRefType::SubstituteImpl`,
        // but it may turn out it is needed here too.

        return substDeclRef;
    }


    // Check if this is an equivalent declaration reference to another
    bool DeclRefBase::equals(DeclRefBase const& declRef) const
    {
        if (decl != declRef.decl)
            return false;
        if (!substitutions.equals(declRef.substitutions))
            return false;

        return true;
    }

    // Convenience accessors for common properties of declarations
    Name* DeclRefBase::GetName() const
    {
        return decl->nameAndLoc.name;
    }

    SourceLoc DeclRefBase::getLoc() const
    {
        return decl->loc;
    }

    DeclRefBase DeclRefBase::GetParent() const
    {
        // Want access to the free function (the 'as' method by default gets priority)
        // Can access as method with this->as because it removes any ambiguity.
        using Slang::as;

        auto parentDecl = decl->parentDecl;
        if (!parentDecl)
            return DeclRefBase();

        // Default is to apply the same set of substitutions/specializations
        // to the parent declaration as were applied to the child.
        RefPtr<Substitutions> substToApply = substitutions.substitutions;

        if(auto interfaceDecl = as<InterfaceDecl>(decl))
        {
            // The declaration being referenced is an `interface` declaration,
            // and there might be a this-type substitution in place.
            // A reference to the parent of the interface declaration
            // should not include that substitution.
            if(auto thisTypeSubst = as<ThisTypeSubstitution>(substToApply))
            {
                if(thisTypeSubst->interfaceDecl == interfaceDecl)
                {
                    // Strip away that specializations that apply to the interface.
                    substToApply = thisTypeSubst->outer;
                }
            }
        }

        if (auto parentGenericDecl = as<GenericDecl>(parentDecl))
        {
            // The parent of this declaration is a generic, which means
            // that the decl-ref to the current declaration might include
            // substitutions that specialize the generic parameters.
            // A decl-ref to the parent generic should *not* include
            // those substitutions.
            //
            if(auto genericSubst = as<GenericSubstitution>(substToApply))
            {
                if(genericSubst->genericDecl == parentGenericDecl)
                {
                    // Strip away the specializations that were applied to the parent.
                    substToApply = genericSubst->outer;
                }
            }
        }

        return DeclRefBase(parentDecl, substToApply);
    }

    HashCode DeclRefBase::getHashCode() const
    {
        return combineHash(PointerHash<1>::getHashCode(decl), substitutions.getHashCode());
    }

    // Val

    RefPtr<Val> Val::substitute(SubstitutionSet subst)
    {
        if (!subst) return this;
        int diff = 0;
        return substituteImpl(subst, &diff);
    }

    RefPtr<Val> Val::substituteImpl(SubstitutionSet /*subst*/, int* /*ioDiff*/)
    {
        // Default behavior is to not substitute at all
        return this;
    }

    // IntVal

    IntegerLiteralValue GetIntVal(RefPtr<IntVal> val)
    {
        if (auto constantVal = as<ConstantIntVal>(val))
        {
            return constantVal->value;
        }
        SLANG_UNEXPECTED("needed a known integer value");
        return 0;
    }

    // ConstantIntVal

    bool ConstantIntVal::equalsVal(Val* val)
    {
        if (auto intVal = as<ConstantIntVal>(val))
            return value == intVal->value;
        return false;
    }

    String ConstantIntVal::toString()
    {
        return String(value);
    }

    HashCode ConstantIntVal::getHashCode()
    {
        return (HashCode) value;
    }

    //

    void registerBuiltinDecl(
        Session*                    session,
        RefPtr<Decl>                decl,
        RefPtr<BuiltinTypeModifier> modifier)
    {
        auto type = DeclRefType::Create(
            session,
            DeclRef<Decl>(decl.Ptr(), nullptr));
        session->builtinTypes[(int)modifier->tag] = type;
    }

    void registerMagicDecl(
        Session*                    session,
        RefPtr<Decl>                decl,
        RefPtr<MagicTypeModifier>   modifier)
    {
        // In some cases the modifier will have been applied to the
        // "inner" declaration of a `GenericDecl`, but what we
        // actually want to register is the generic itself.
        //
        auto declToRegister = decl;
        if(auto genericDecl = as<GenericDecl>(decl->parentDecl))
            declToRegister = genericDecl;

        session->magicDecls[modifier->name] = declToRegister.Ptr();
    }

    RefPtr<Decl> findMagicDecl(
        Session*        session,
        String const&   name)
    {
        return session->magicDecls[name].GetValue();
    }

    //

    SyntaxNodeBase* createInstanceOfSyntaxClassByName(
        String const&   name)
    {
        if(0) {}
    #define CASE(NAME) \
        else if(name == #NAME) return new NAME()

    CASE(GLSLBufferModifier);
    CASE(GLSLWriteOnlyModifier);
    CASE(GLSLReadOnlyModifier);
    CASE(GLSLPatchModifier);
    CASE(SimpleModifier);

    #undef CASE
        else
        {
            SLANG_UNEXPECTED("unhandled syntax class name");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    //

    // HLSLPatchType

    Type* HLSLPatchType::getElementType()
    {
        return as<Type>(findInnerMostGenericSubstitution(declRef.substitutions)->args[0]);
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return as<IntVal>(findInnerMostGenericSubstitution(declRef.substitutions)->args[1]);
    }

    // Constructors for types

    RefPtr<ArrayExpressionType> getArrayType(
        Type* elementType,
        IntVal*         elementCount)
    {
        auto session = elementType->getSession();
        auto arrayType = new ArrayExpressionType();
        arrayType->setSession(session);
        arrayType->baseType = elementType;
        arrayType->arrayLength = elementCount;
        return arrayType;
    }

    RefPtr<ArrayExpressionType> getArrayType(
        Type* elementType)
    {
        auto session = elementType->getSession();
        auto arrayType = new ArrayExpressionType();
        arrayType->setSession(session);
        arrayType->baseType = elementType;
        return arrayType;
    }

    RefPtr<NamedExpressionType> getNamedType(
        Session*                    session,
        DeclRef<TypeDefDecl> const& declRef)
    {
        DeclRef<TypeDefDecl> specializedDeclRef = createDefaultSubstitutionsIfNeeded(session, declRef).as<TypeDefDecl>();

        auto namedType = new NamedExpressionType(specializedDeclRef);
        namedType->setSession(session);
        return namedType;
    }

    RefPtr<TypeType> getTypeType(
        Type* type)
    {
        auto session = type->getSession();
        auto typeType = new TypeType(type);
        typeType->setSession(session);
        return typeType;
    }

    RefPtr<FuncType> getFuncType(
        Session*                        session,
        DeclRef<CallableDecl> const&    declRef)
    {
        RefPtr<FuncType> funcType = new FuncType();
        funcType->setSession(session);

        funcType->resultType = GetResultType(declRef);
        for (auto paramDeclRef : GetParameters(declRef))
        {
            auto paramDecl = paramDeclRef.getDecl();
            auto paramType = GetType(paramDeclRef);
            if( paramDecl->findModifier<RefModifier>() )
            {
                paramType = session->getRefType(paramType);
            }
            else if( paramDecl->findModifier<OutModifier>() )
            {
                if(paramDecl->findModifier<InOutModifier>() || paramDecl->findModifier<InModifier>())
                {
                    paramType = session->getInOutType(paramType);
                }
                else
                {
                    paramType = session->getOutType(paramType);
                }
            }
            funcType->paramTypes.add(paramType);
        }

        return funcType;
    }

    RefPtr<GenericDeclRefType> getGenericDeclRefType(
        Session*                    session,
        DeclRef<GenericDecl> const& declRef)
    {
        auto genericDeclRefType = new GenericDeclRefType(declRef);
        genericDeclRefType->setSession(session);
        return genericDeclRefType;
    }

    RefPtr<NamespaceType> getNamespaceType(
        Session*                            session,
        DeclRef<NamespaceDeclBase> const&   declRef)
    {
        auto type = new NamespaceType;
        type->setSession(session);
        type->declRef = declRef;
        return type;
    }

    RefPtr<SamplerStateType> getSamplerStateType(
        Session*        session)
    {
        auto samplerStateType = new SamplerStateType();
        samplerStateType->setSession(session);
        return samplerStateType;
    }

    // TODO: should really have a `type.cpp` and a `witness.cpp`

    bool TypeEqualityWitness::equalsVal(Val* val)
    {
        auto otherWitness = as<TypeEqualityWitness>(val);
        if (!otherWitness)
            return false;
        return sub->equals(otherWitness->sub);
    }

    RefPtr<Val> TypeEqualityWitness::substituteImpl(SubstitutionSet subst, int * ioDiff)
    {
        RefPtr<TypeEqualityWitness> rs = new TypeEqualityWitness();
        rs->sub = sub->substituteImpl(subst, ioDiff).as<Type>();
        rs->sup = sup->substituteImpl(subst, ioDiff).as<Type>();
        return rs;
    }

    String TypeEqualityWitness::toString()
    {
        return "TypeEqualityWitness(" + sub->toString() + ")";
    }

    HashCode TypeEqualityWitness::getHashCode()
    {
        return sub->getHashCode();
    }

    bool DeclaredSubtypeWitness::equalsVal(Val* val)
    {
        auto otherWitness = as<DeclaredSubtypeWitness>(val);
        if(!otherWitness)
            return false;

        return sub->equals(otherWitness->sub)
            && sup->equals(otherWitness->sup)
            && declRef.equals(otherWitness->declRef);
    }

    RefPtr<ThisTypeSubstitution> findThisTypeSubstitution(
        Substitutions*  substs,
        InterfaceDecl*  interfaceDecl)
    {
        for(RefPtr<Substitutions> s = substs; s; s = s->outer)
        {
            auto thisTypeSubst = as<ThisTypeSubstitution>(s);
            if(!thisTypeSubst)
                continue;

            if(thisTypeSubst->interfaceDecl != interfaceDecl)
                continue;

            return thisTypeSubst;
        }

        return nullptr;
    }

    RefPtr<Val> DeclaredSubtypeWitness::substituteImpl(SubstitutionSet subst, int * ioDiff)
    {
        if (auto genConstraintDeclRef = declRef.as<GenericTypeConstraintDecl>())
        {
            auto genConstraintDecl = genConstraintDeclRef.getDecl();

            // search for a substitution that might apply to us
            for(auto s = subst.substitutions; s; s = s->outer)
            {
                if(auto genericSubst = as<GenericSubstitution>(s))
                {
                    // the generic decl associated with the substitution list must be
                    // the generic decl that declared this parameter
                    auto genericDecl = genericSubst->genericDecl;
                    if (genericDecl != genConstraintDecl->parentDecl)
                        continue;

                    bool found = false;
                    Index index = 0;
                    for (auto m : genericDecl->members)
                    {
                        if (auto constraintParam = as<GenericTypeConstraintDecl>(m))
                        {
                            if (constraintParam == declRef.getDecl())
                            {
                                found = true;
                                break;
                            }
                            index++;
                        }
                    }
                    if (found)
                    {
                        (*ioDiff)++;
                        auto ordinaryParamCount = genericDecl->getMembersOfType<GenericTypeParamDecl>().getCount() +
                            genericDecl->getMembersOfType<GenericValueParamDecl>().getCount();
                        SLANG_ASSERT(index + ordinaryParamCount < genericSubst->args.getCount());
                        return genericSubst->args[index + ordinaryParamCount];
                    }
                }
                else if(auto globalGenericSubst = s.as<GlobalGenericParamSubstitution>())
                {
                    // check if the substitution is really about this global generic type parameter
                    if (globalGenericSubst->paramDecl != genConstraintDecl->parentDecl)
                        continue;

                    for(auto constraintArg : globalGenericSubst->constraintArgs)
                    {
                        if(constraintArg.decl.Ptr() != genConstraintDecl)
                            continue;

                        (*ioDiff)++;
                        return constraintArg.val;
                    }
                }
            }
        }

        // Perform substitution on the constituent elements.
        int diff = 0;
        auto substSub = sub->substituteImpl(subst, &diff).as<Type>();
        auto substSup = sup->substituteImpl(subst, &diff).as<Type>();
        auto substDeclRef = declRef.SubstituteImpl(subst, &diff);
        if (!diff)
            return this;

        (*ioDiff)++;

        // If we have a reference to a type constraint for an
        // associated type declaration, then we can replace it
        // with the concrete conformance witness for a concrete
        // type implementing the outer interface.
        //
        // TODO: It is a bit gross that we use `GenericTypeConstraintDecl` for
        // associated types, when they aren't really generic type *parameters*,
        // so we'll need to change this location in the code if we ever clean
        // up the hierarchy.
        //
        if (auto substTypeConstraintDecl = as<GenericTypeConstraintDecl>(substDeclRef.decl))
        {
            if (auto substAssocTypeDecl = as<AssocTypeDecl>(substTypeConstraintDecl->parentDecl))
            {
                if (auto interfaceDecl = as<InterfaceDecl>(substAssocTypeDecl->parentDecl))
                {
                    // At this point we have a constraint decl for an associated type,
                    // and we nee to see if we are dealing with a concrete substitution
                    // for the interface around that associated type.
                    if(auto thisTypeSubst = findThisTypeSubstitution(substDeclRef.substitutions, interfaceDecl))
                    {
                        // We need to look up the declaration that satisfies
                        // the requirement named by the associated type.
                        Decl* requirementKey = substTypeConstraintDecl;
                        RequirementWitness requirementWitness = tryLookUpRequirementWitness(thisTypeSubst->witness, requirementKey);
                        switch(requirementWitness.getFlavor())
                        {
                        default:
                            break;

                        case RequirementWitness::Flavor::val:
                            {
                                auto satisfyingVal = requirementWitness.getVal();
                                return satisfyingVal;
                            }
                        }
                    }
                }
            }
        }




        RefPtr<DeclaredSubtypeWitness> rs = new DeclaredSubtypeWitness();
        rs->sub = substSub;
        rs->sup = substSup;
        rs->declRef = substDeclRef;
        return rs;
    }

    String DeclaredSubtypeWitness::toString()
    {
        StringBuilder sb;
        sb << "DeclaredSubtypeWitness(";
        sb << this->sub->toString();
        sb << ", ";
        sb << this->sup->toString();
        sb << ", ";
        sb << this->declRef.toString();
        sb << ")";
        return sb.ProduceString();
    }

    HashCode DeclaredSubtypeWitness::getHashCode()
    {
        return declRef.getHashCode();
    }

    // TransitiveSubtypeWitness

    bool TransitiveSubtypeWitness::equalsVal(Val* val)
    {
        auto otherWitness = as<TransitiveSubtypeWitness>(val);
        if(!otherWitness)
            return false;

        return sub->equals(otherWitness->sub)
            && sup->equals(otherWitness->sup)
            && subToMid->equalsVal(otherWitness->subToMid)
            && midToSup.equals(otherWitness->midToSup);
    }

    RefPtr<Val> TransitiveSubtypeWitness::substituteImpl(SubstitutionSet subst, int * ioDiff)
    {
        int diff = 0;

        RefPtr<Type> substSub = sub->substituteImpl(subst, &diff).as<Type>();
        RefPtr<Type> substSup = sup->substituteImpl(subst, &diff).as<Type>();
        RefPtr<SubtypeWitness> substSubToMid = subToMid->substituteImpl(subst, &diff).as<SubtypeWitness>();
        DeclRef<Decl> substMidToSup = midToSup.SubstituteImpl(subst, &diff);

        // If nothing changed, then we can bail out early.
        if (!diff)
            return this;

        // Something changes, so let the caller know.
        (*ioDiff)++;

        // TODO: are there cases where we can simplify?
        //
        // In principle, if either `subToMid` or `midToSub` turns into
        // a reflexive subtype witness, then we could drop that side,
        // and just return the other one (this would imply that `sub == mid`
        // or `mid == sup` after substitutions).
        //
        // In the long run, is it also possible that if `sub` gets resolved
        // to a concrete type *and* we decide to flatten out the inheritance
        // graph into a linearized "class precedence list" stored in any
        // aggregate type, then we could potentially just redirect to point
        // to the appropriate inheritance decl in the original type.
        //
        // For now I'm going to ignore those possibilities and hope for the best.

        // In the simple case, we just construct a new transitive subtype
        // witness, and we move on with life.
        RefPtr<TransitiveSubtypeWitness> result = new TransitiveSubtypeWitness();
        result->sub = substSub;
        result->sup = substSup;
        result->subToMid = substSubToMid;
        result->midToSup = substMidToSup;
        return result;
    }

    String TransitiveSubtypeWitness::toString()
    {
        // Note: we only print the constituent
        // witnesses, and rely on them to print
        // the starting and ending types.
        StringBuilder sb;
        sb << "TransitiveSubtypeWitness(";
        sb << this->subToMid->toString();
        sb << ", ";
        sb << this->midToSup.toString();
        sb << ")";
        return sb.ProduceString();
    }

    HashCode TransitiveSubtypeWitness::getHashCode()
    {
        auto hash = sub->getHashCode();
        hash = combineHash(hash, sup->getHashCode());
        hash = combineHash(hash, subToMid->getHashCode());
        hash = combineHash(hash, midToSup.getHashCode());
        return hash;
    }

    //

    String DeclRefBase::toString() const
    {
        if (!decl) return "";

        auto name = decl->getName();
        if (!name) return "";

        // TODO: need to print out substitutions too!
        return name->text;
    }

    bool SubstitutionSet::equals(const SubstitutionSet& substSet) const
    {
        if (substitutions == substSet.substitutions)
        {
            return true;
        }
        if (substitutions == nullptr || substSet.substitutions == nullptr)
        {
            return false;
        }
        return substitutions->equals(substSet.substitutions);
    }

    HashCode SubstitutionSet::getHashCode() const
    {
        HashCode rs = 0;
        if (substitutions)
            rs = combineHash(rs, substitutions->getHashCode());
        return rs;
    }

    // ExtractExistentialType

    String ExtractExistentialType::toString()
    {
        String result;
        result.append(declRef.toString());
        result.append(".This");
        return result;
    }

    bool ExtractExistentialType::equalsImpl(Type* type)
    {
        if( auto extractExistential = as<ExtractExistentialType>(type) )
        {
            return declRef.equals(extractExistential->declRef);
        }
        return false;
    }

    HashCode ExtractExistentialType::getHashCode()
    {
        return declRef.getHashCode();
    }

    RefPtr<Type> ExtractExistentialType::createCanonicalType()
    {
        return this;
    }

    RefPtr<Val> ExtractExistentialType::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;
        auto substDeclRef = declRef.SubstituteImpl(subst, &diff);
        if(!diff)
            return this;

        (*ioDiff)++;

        RefPtr<ExtractExistentialType> substValue = new ExtractExistentialType();
        substValue->declRef = declRef;
        return substValue;
    }

    // ExtractExistentialSubtypeWitness

    bool ExtractExistentialSubtypeWitness::equalsVal(Val* val)
    {
        if( auto extractWitness = as<ExtractExistentialSubtypeWitness>(val) )
        {
            return declRef.equals(extractWitness->declRef);
        }
        return false;
    }

    String ExtractExistentialSubtypeWitness::toString()
    {
        String result;
        result.append("extractExistentialValue(");
        result.append(declRef.toString());
        result.append(")");
        return result;
    }

    HashCode ExtractExistentialSubtypeWitness::getHashCode()
    {
        return declRef.getHashCode();
    }

    RefPtr<Val> ExtractExistentialSubtypeWitness::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;

        auto substDeclRef = declRef.SubstituteImpl(subst, &diff);
        auto substSub = sub->substituteImpl(subst, &diff).as<Type>();
        auto substSup = sup->substituteImpl(subst, &diff).as<Type>();

        if(!diff)
            return this;

        (*ioDiff)++;

        RefPtr<ExtractExistentialSubtypeWitness> substValue = new ExtractExistentialSubtypeWitness();
        substValue->declRef = declRef;
        substValue->sub = substSub;
        substValue->sup = substSup;
        return substValue;
    }

    //
    // TaggedUnionType
    //

    String TaggedUnionType::toString()
    {
        String result;
        result.append("__TaggedUnion(");
        bool first = true;
        for( auto caseType : caseTypes )
        {
            if(!first) result.append(", ");
            first = false;

            result.append(caseType->toString());
        }
        result.append(")");
        return result;
    }

    bool TaggedUnionType::equalsImpl(Type* type)
    {
        auto taggedUnion = as<TaggedUnionType>(type);
        if(!taggedUnion)
            return false;

        auto caseCount = caseTypes.getCount();
        if(caseCount != taggedUnion->caseTypes.getCount())
            return false;

        for( Index ii = 0; ii < caseCount; ++ii )
        {
            if(!caseTypes[ii]->equals(taggedUnion->caseTypes[ii]))
                return false;
        }
        return true;
    }

    HashCode TaggedUnionType::getHashCode()
    {
        HashCode hashCode = 0;
        for( auto caseType : caseTypes )
        {
            hashCode = combineHash(hashCode, caseType->getHashCode());
        }
        return hashCode;
    }

    RefPtr<Type> TaggedUnionType::createCanonicalType()
    {
        RefPtr<TaggedUnionType> canType = new TaggedUnionType();
        canType->setSession(getSession());

        for( auto caseType : caseTypes )
        {
            auto canCaseType = caseType->getCanonicalType();
            canType->caseTypes.add(canCaseType);
        }

        return canType;
    }

    RefPtr<Val> TaggedUnionType::substituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;

        List<RefPtr<Type>> substCaseTypes;
        for( auto caseType : caseTypes )
        {
            substCaseTypes.add(caseType->substituteImpl(subst, &diff).as<Type>());
        }
        if(!diff)
            return this;

        (*ioDiff)++;

        RefPtr<TaggedUnionType> substType = new TaggedUnionType();
        substType->setSession(getSession());
        substType->caseTypes.swapWith(substCaseTypes);
        return substType;
    }

//
// TaggedUnionSubtypeWitness
//


bool TaggedUnionSubtypeWitness::equalsVal(Val* val)
{
    auto taggedUnionWitness = as<TaggedUnionSubtypeWitness>(val);
    if(!taggedUnionWitness)
        return false;

    auto caseCount = caseWitnesses.getCount();
    if(caseCount != taggedUnionWitness->caseWitnesses.getCount())
        return false;

    for(Index ii = 0; ii < caseCount; ++ii)
    {
        if(!caseWitnesses[ii]->equalsVal(taggedUnionWitness->caseWitnesses[ii]))
            return false;
    }

    return true;
}

String TaggedUnionSubtypeWitness::toString()
{
    String result;
    result.append("TaggedUnionSubtypeWitness(");
    bool first = true;
    for( auto caseWitness : caseWitnesses )
    {
        if(!first) result.append(", ");
        first = false;

        result.append(caseWitness->toString());
    }
    return result;
}

HashCode TaggedUnionSubtypeWitness::getHashCode()
{
    HashCode hash = 0;
    for( auto caseWitness : caseWitnesses )
    {
        hash = combineHash(hash, caseWitness->getHashCode());
    }
    return hash;
}

RefPtr<Val> TaggedUnionSubtypeWitness::substituteImpl(SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substSub = sub->substituteImpl(subst, &diff).as<Type>();
    auto substSup = sup->substituteImpl(subst, &diff).as<Type>();

    List<RefPtr<Val>> substCaseWitnesses;
    for( auto caseWitness : caseWitnesses )
    {
        substCaseWitnesses.add(caseWitness->substituteImpl(subst, &diff));
    }

    if(!diff)
        return this;

    (*ioDiff)++;

    RefPtr<TaggedUnionSubtypeWitness> substWitness = new TaggedUnionSubtypeWitness();
    substWitness->sub = substSub;
    substWitness->sup = substSup;
    substWitness->caseWitnesses.swapWith(substCaseWitnesses);
    return substWitness;
}

Module* getModule(Decl* decl)
{
    for( auto dd = decl; dd; dd = dd->parentDecl )
    {
        if(auto moduleDecl = as<ModuleDecl>(dd))
            return moduleDecl->module;
    }
    return nullptr;
}

bool findImageFormatByName(char const* name, ImageFormat* outFormat)
{
    static const struct
    {
        char const* name;
        ImageFormat format;
    } kFormats[] =
    {
#define FORMAT(NAME) { #NAME, ImageFormat::NAME },
#include "slang-image-format-defs.h"
    };

    for( auto item : kFormats )
    {
        if( strcmp(item.name, name) == 0 )
        {
            *outFormat = item.format;
            return true;
        }
    }

    return false;
}

char const* getGLSLNameForImageFormat(ImageFormat format)
{
    switch( format )
    {
    default: return "unhandled";
#define FORMAT(NAME) case ImageFormat::NAME: return #NAME;
#include "slang-image-format-defs.h"
    }
}

//
// ExistentialSpecializedType
//

String ExistentialSpecializedType::toString()
{
    String result;
    result.append("__ExistentialSpecializedType(");
    result.append(baseType->toString());
    for( auto arg : args )
    {
        result.append(", ");
        result.append(arg.val->toString());
    }
    result.append(")");
    return result;
}

bool ExistentialSpecializedType::equalsImpl(Type * type)
{
    auto other = as<ExistentialSpecializedType>(type);
    if(!other)
        return false;

    if(!baseType->equals(other->baseType))
        return false;

    auto argCount = args.getCount();
    if(argCount != other->args.getCount())
        return false;

    for( Index ii = 0; ii < argCount; ++ii )
    {
        auto arg = args[ii];
        auto otherArg = other->args[ii];

        if(!arg.val->equalsVal(otherArg.val))
            return false;

        if(!areValsEqual(arg.witness, otherArg.witness))
            return false;
    }
    return true;
}

HashCode ExistentialSpecializedType::getHashCode()
{
    Hasher hasher;
    hasher.hashObject(baseType);
    for(auto arg : args)
    {
        hasher.hashObject(arg.val);
        if(auto witness = arg.witness)
            hasher.hashObject(witness);
    }
    return hasher.getResult();
}

RefPtr<Val> getCanonicalValue(Val* val)
{
    if(!val)
        return nullptr;
    if(auto type = as<Type>(val))
    {
        return type->getCanonicalType();
    }
    // TODO: We may eventually need/want some sort of canonicalization
    // for non-type values, but for now there is nothing to do.
    return val;
}

RefPtr<Type> ExistentialSpecializedType::createCanonicalType()
{
    RefPtr<ExistentialSpecializedType> canType = new ExistentialSpecializedType();
    canType->setSession(getSession());

    canType->baseType = baseType->getCanonicalType();
    for( auto arg : args )
    {
        ExpandedSpecializationArg canArg;
        canArg.val = getCanonicalValue(arg.val);
        canArg.witness = getCanonicalValue(arg.witness);
        canType->args.add(canArg);
    }
    return canType;
}

RefPtr<Val> substituteImpl(Val* val, SubstitutionSet subst, int* ioDiff)
{
    if(!val) return nullptr;
    return val->substituteImpl(subst, ioDiff);
}

RefPtr<Val> ExistentialSpecializedType::substituteImpl(SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substBaseType = baseType->substituteImpl(subst, &diff).as<Type>();

    ExpandedSpecializationArgs substArgs;
    for( auto arg : args )
    {
        ExpandedSpecializationArg substArg;
        substArg.val = Slang::substituteImpl(arg.val, subst, &diff);
        substArg.witness = Slang::substituteImpl(arg.witness, subst, &diff);
        substArgs.add(substArg);
    }

    if(!diff)
        return this;

    (*ioDiff)++;

    RefPtr<ExistentialSpecializedType> substType = new ExistentialSpecializedType();
    substType->setSession(getSession());
    substType->baseType = substBaseType;
    substType->args = substArgs;
    return substType;
}

//
// ThisType
//

String ThisType::toString()
{
    String result;
    result.append(interfaceDeclRef.toString());
    result.append(".This");
    return result;
}

bool ThisType::equalsImpl(Type * type)
{
    auto other = as<ThisType>(type);
    if(!other)
        return false;

    if(!interfaceDeclRef.equals(other->interfaceDeclRef))
        return false;

    return true;
}

HashCode ThisType::getHashCode()
{
    return combineHash(
        HashCode(typeid(*this).hash_code()),
        interfaceDeclRef.getHashCode());
}

RefPtr<Type> ThisType::createCanonicalType()
{
    RefPtr<ThisType> canType = new ThisType();
    canType->setSession(getSession());

    // TODO: need to canonicalize the decl-ref
    canType->interfaceDeclRef = interfaceDeclRef;
    return canType;
}

RefPtr<Val> ThisType::substituteImpl(SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substInterfaceDeclRef = interfaceDeclRef.SubstituteImpl(subst, &diff);

    auto thisTypeSubst = findThisTypeSubstitution(subst.substitutions, substInterfaceDeclRef.getDecl());
    if( thisTypeSubst )
    {
        return thisTypeSubst->witness->sub;
    }

    if(!diff)
        return this;

    (*ioDiff)++;

    RefPtr<ThisType> substType = new ThisType();
    substType->setSession(getSession());
    substType->interfaceDeclRef = substInterfaceDeclRef;
    return substType;
}

} // namespace Slang
