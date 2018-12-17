#include "syntax.h"

#include "compiler.h"
#include "visitor.h"

#include <typeinfo>
#include <assert.h>

namespace Slang
{
    // BasicExpressionType

    bool BasicExpressionType::EqualsImpl(Type * type)
    {
        auto basicType = dynamic_cast<const BasicExpressionType*>(type);
        if (basicType == nullptr)
            return false;
        return basicType->baseType == this->baseType;
    }

    RefPtr<Type> BasicExpressionType::CreateCanonicalType()
    {
        // A basic type is already canonical, in our setup
        return this;
    }

    // Generate dispatch logic and other definitions for all syntax classes
#define SYNTAX_CLASS(NAME, BASE) /* empty */
#include "object-meta-begin.h"

#include "syntax-base-defs.h"
#undef SYNTAX_CLASS
#undef ABSTRACT_SYNTAX_CLASS

#define ABSTRACT_SYNTAX_CLASS(NAME, BASE)   \
    template<>                              \
    SyntaxClassBase::ClassInfo const SyntaxClassBase::Impl<NAME>::kClassInfo = { #NAME, &SyntaxClassBase::Impl<BASE>::kClassInfo, nullptr };

#define SYNTAX_CLASS(NAME, BASE)                                                \
    void NAME::accept(NAME::Visitor* visitor, void* extra)                      \
    { visitor->dispatch_##NAME(this, extra); }                                  \
    template<>                                                                  \
    void* SyntaxClassBase::Impl<NAME>::createFunc() { return new NAME(); }      \
    SyntaxClass<NodeBase> NAME::getClass() { return Slang::getClass<NAME>(); }  \
    template<>                                                                  \
    SyntaxClassBase::ClassInfo const SyntaxClassBase::Impl<NAME>::kClassInfo = { #NAME, &SyntaxClassBase::Impl<BASE>::kClassInfo, &SyntaxClassBase::Impl<NAME>::createFunc };

template<>
SyntaxClassBase::ClassInfo const SyntaxClassBase::Impl<RefObject>::kClassInfo = { "RefObject", nullptr, nullptr };

ABSTRACT_SYNTAX_CLASS(NodeBase, RefObject);
ABSTRACT_SYNTAX_CLASS(SyntaxNodeBase, NodeBase);
ABSTRACT_SYNTAX_CLASS(SyntaxNode, SyntaxNodeBase);
ABSTRACT_SYNTAX_CLASS(ModifiableSyntaxNode, SyntaxNode);
ABSTRACT_SYNTAX_CLASS(DeclBase, ModifiableSyntaxNode);
ABSTRACT_SYNTAX_CLASS(Decl, DeclBase);
ABSTRACT_SYNTAX_CLASS(Stmt, ModifiableSyntaxNode);
ABSTRACT_SYNTAX_CLASS(Val, NodeBase);
ABSTRACT_SYNTAX_CLASS(Type, Val);
ABSTRACT_SYNTAX_CLASS(Modifier, SyntaxNodeBase);
ABSTRACT_SYNTAX_CLASS(Expr, SyntaxNode);

ABSTRACT_SYNTAX_CLASS(Substitutions, SyntaxNode);
ABSTRACT_SYNTAX_CLASS(GenericSubstitution, Substitutions);
ABSTRACT_SYNTAX_CLASS(ThisTypeSubstitution, Substitutions);
ABSTRACT_SYNTAX_CLASS(GlobalGenericParamSubstitution, Substitutions);

#include "expr-defs.h"
#include "decl-defs.h"
#include "modifier-defs.h"
#include "stmt-defs.h"
#include "type-defs.h"
#include "val-defs.h"
#include "object-meta-end.h"

bool SyntaxClassBase::isSubClassOfImpl(SyntaxClassBase const& super) const
{
    SyntaxClassBase::ClassInfo const* info = classInfo;
    while (info)
    {
        if (info == super.classInfo)
            return true;

        info = info->baseClass;
    }

    return false;
}

void Type::accept(IValVisitor* visitor, void* extra)
{
    accept((ITypeVisitor*)visitor, extra);
}

    // TypeExp

    bool TypeExp::Equals(Type* other)
    {
        return type->Equals(other);
    }

    bool TypeExp::Equals(RefPtr<Type> other)
    {
        return type->Equals(other.Ptr());
    }

    // BasicExpressionType

    BasicExpressionType* BasicExpressionType::GetScalarType()
    {
        return this;
    }

    //

    bool Type::Equals(Type * type)
    {
        return GetCanonicalType()->EqualsImpl(type->GetCanonicalType());
    }

    bool Type::Equals(RefPtr<Type> type)
    {
        return Equals(type.Ptr());
    }

    bool Type::EqualsVal(Val* val)
    {
        if (auto type = dynamic_cast<Type*>(val))
            return const_cast<Type*>(this)->Equals(type);
        return false;
    }

    NamedExpressionType* Type::AsNamedType()
    {
        return dynamic_cast<NamedExpressionType*>(this);
    }

    RefPtr<Val> Type::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;
        auto canSubst = GetCanonicalType()->SubstituteImpl(subst, &diff);

        // If nothing changed, then don't drop any sugar that is applied
        if (!diff)
            return this;

        // If the canonical type changed, then we return a canonical type,
        // rather than try to re-construct any amount of sugar
        (*ioDiff)++;
        return canSubst;
    }


    Type* Type::GetCanonicalType()
    {
        SLANG_ASSERT(this);

        Type* et = const_cast<Type*>(this);
        if (!et->canonicalType)
        {
            // TODO(tfoley): worry about thread safety here?
            auto canType = et->CreateCanonicalType();
            et->canonicalType = canType;
            if (dynamic_cast<Type*>(et->canonicalType) != this)
                et->canonicalTypeRefPtr = canType;
            else
                canType.detach();
            SLANG_ASSERT(et->canonicalType);
        }
        return et->canonicalType;
    }

    bool Type::IsTextureOrSampler()
    {
        return IsTexture() || IsSampler();
    }
    bool Type::IsStruct()
    {
        auto declRefType = AsDeclRefType();
        if (!declRefType) return false;
        auto structDeclRef = declRefType->declRef.As<StructDecl>();
        if (!structDeclRef) return false;
        return true;
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
        return RefPtr<Type>(builtinTypes[(int)flavor]);
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
        auto stringTypeDecl = findMagicDecl(this, "StringType");
        return DeclRefType::Create(this, makeDeclRef<Decl>(stringTypeDecl));
    }

    Type* Session::getEnumTypeType()
    {
        auto enumTypeTypeDecl = findMagicDecl(this, "EnumTypeType");
        return DeclRefType::Create(this, makeDeclRef<Decl>(enumTypeTypeDecl));
    }


    RefPtr<PtrType> Session::getPtrType(
        RefPtr<Type>    valueType)
    {
        return getPtrType(valueType, "PtrType").As<PtrType>();
    }

        // Construct the type `Out<valueType>`
    RefPtr<OutType> Session::getOutType(RefPtr<Type> valueType)
    {
        return getPtrType(valueType, "OutType").As<OutType>();
    }

    RefPtr<InOutType> Session::getInOutType(RefPtr<Type> valueType)
    {
        return getPtrType(valueType, "InOutType").As<InOutType>();
    }

    RefPtr<RefType> Session::getRefType(RefPtr<Type> valueType)
    {
        return getPtrType(valueType, "RefType").As<RefType>();
    }

    RefPtr<PtrTypeBase> Session::getPtrType(RefPtr<Type> valueType, char const* ptrTypeName)
    {
        auto genericDecl = findMagicDecl(
            this, ptrTypeName).As<GenericDecl>();
        return getPtrType(valueType, genericDecl);
    }

    RefPtr<PtrTypeBase> Session::getPtrType(RefPtr<Type> valueType, GenericDecl* genericDecl)
    {
        auto typeDecl = genericDecl->inner;

        auto substitutions = new GenericSubstitution();
        substitutions->genericDecl = genericDecl;
        substitutions->args.Add(valueType);

        auto declRef = DeclRef<Decl>(typeDecl.Ptr(), substitutions);
        auto rsType = DeclRefType::Create(
            this,
            declRef);
        return rsType->As<PtrTypeBase>();
    }

    RefPtr<ArrayExpressionType> Session::getArrayType(
        Type*   elementType,
        IntVal* elementCount)
    {
        RefPtr<ArrayExpressionType> arrayType = new ArrayExpressionType();
        arrayType->setSession(this);
        arrayType->baseType = elementType;
        arrayType->ArrayLength = elementCount;
        return arrayType;
    }

    SyntaxClass<RefObject> Session::findSyntaxClass(Name* name)
    {
        SyntaxClass<RefObject> syntaxClass;
        if (mapNameToSyntaxClass.TryGetValue(name, syntaxClass))
            return syntaxClass;

        return SyntaxClass<RefObject>();
    }



    bool ArrayExpressionType::EqualsImpl(Type * type)
    {
        auto arrType = type->AsArrayType();
        if (!arrType)
            return false;
        return (areValsEqual(ArrayLength, arrType->ArrayLength) && baseType->Equals(arrType->baseType.Ptr()));
    }

    RefPtr<Val> ArrayExpressionType::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;
        auto elementType = baseType->SubstituteImpl(subst, &diff).As<Type>();
        auto arrlen = ArrayLength->SubstituteImpl(subst, &diff).As<IntVal>();
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

    RefPtr<Type> ArrayExpressionType::CreateCanonicalType()
    {
        auto canonicalElementType = baseType->GetCanonicalType();
        auto canonicalArrayType = getArrayType(
            canonicalElementType,
            ArrayLength);
        return canonicalArrayType;
    }
    int ArrayExpressionType::GetHashCode()
    {
        if (ArrayLength)
            return (baseType->GetHashCode() * 16777619) ^ ArrayLength->GetHashCode();
        else
            return baseType->GetHashCode();
    }
    Slang::String ArrayExpressionType::ToString()
    {
        if (ArrayLength)
            return baseType->ToString() + "[" + ArrayLength->ToString() + "]";
        else
            return baseType->ToString() + "[]";
    }

    // DeclRefType

    String DeclRefType::ToString()
    {
        return declRef.toString();
    }

    int DeclRefType::GetHashCode()
    {
        return (declRef.GetHashCode() * 16777619) ^ (int)(typeid(this).hash_code());
    }

    bool DeclRefType::EqualsImpl(Type * type)
    {
        if (auto declRefType = type->AsDeclRefType())
        {
            return declRef.Equals(declRefType->declRef);
        }
        return false;
    }

    RefPtr<Type> DeclRefType::CreateCanonicalType()
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
        return m_obj.As<WitnessTable>();
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
            return RequirementWitness(
                getVal()->Substitute(subst));
        }
    }

    RequirementWitness tryLookUpRequirementWitness(
        SubtypeWitness* subtypeWitness,
        Decl*           requirementKey)
    {
        if(auto declaredSubtypeWitness = dynamic_cast<DeclaredSubtypeWitness*>(subtypeWitness))
        {
            if(auto inheritanceDeclRef = declaredSubtypeWitness->declRef.As<InheritanceDecl>())
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
                    // The front-end will expand the generic appliation `followHero<Sidekick<Batman>>`
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

    RefPtr<Val> DeclRefType::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        if (!subst) return this;

        // the case we especially care about is when this type references a declaration
        // of a generic parameter, since that is what we might be substituting...
        if (auto genericTypeParamDecl = dynamic_cast<GenericTypeParamDecl*>(declRef.getDecl()))
        {
            // search for a substitution that might apply to us
            for(auto s = subst.substitutions; s; s = s->outer)
            {
                auto genericSubst = s.As<GenericSubstitution>();
                if(!genericSubst)
                    continue;

                // the generic decl associated with the substitution list must be
                // the generic decl that declared this parameter
                auto genericDecl = genericSubst->genericDecl;
                if (genericDecl != genericTypeParamDecl->ParentDecl)
                    continue;

                int index = 0;
                for (auto m : genericDecl->Members)
                {
                    if (m.Ptr() == genericTypeParamDecl)
                    {
                        // We've found it, so return the corresponding specialization argument
                        (*ioDiff)++;
                        return genericSubst->args[index];
                    }
                    else if (auto typeParam = m.As<GenericTypeParamDecl>())
                    {
                        index++;
                    }
                    else if (auto valParam = m.As<GenericValueParamDecl>())
                    {
                        index++;
                    }
                    else
                    {
                    }
                }
            }
        }
        else if (auto globalGenParam = dynamic_cast<GlobalGenericParamDecl*>(declRef.getDecl()))
        {
            // search for a substitution that might apply to us
            for(auto s = subst.substitutions; s; s = s->outer)
            {
                auto genericSubst = s.As<GlobalGenericParamSubstitution>();
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
        if(auto substAssocTypeDecl = substDeclRef.decl->As<AssocTypeDecl>())
        {
            for(auto s = substDeclRef.substitutions.substitutions; s; s = s->outer)
            {
                auto thisSubst = s.As<ThisTypeSubstitution>();
                if(!thisSubst)
                    continue;

                if(auto interfaceDecl = substAssocTypeDecl->ParentDecl->As<InterfaceDecl>())
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
        auto type = val.As<Type>();
        SLANG_RELEASE_ASSERT(type.Ptr());
        return type;
    }

    static RefPtr<IntVal> ExtractGenericArgInteger(RefPtr<Val> val)
    {
        auto intVal = val.As<IntVal>();
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
            RefPtr<Decl> parentDecl = dd->ParentDecl;
            if(!parentDecl)
                break;

            dd = parentDecl;

            if(auto genericParentDecl = parentDecl.As<GenericDecl>())
            {
                // Don't specialize any parameters of a generic.
                if(childDecl != genericParentDecl->inner)
                    break;

                // We have a generic ancestor, but do we have an substitutions for it?
                RefPtr<GenericSubstitution> foundSubst;
                for(auto s = declRef.substitutions.substitutions; s; s = s->outer)
                {
                    auto genSubst = s.As<GenericSubstitution>();
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
    DeclRefType* DeclRefType::Create(
        Session*        session,
        DeclRef<Decl>   declRef)
    {
        declRef = createDefaultSubstitutionsIfNeeded(session, declRef);

        if (auto builtinMod = declRef.getDecl()->FindModifier<BuiltinTypeModifier>())
        {
            auto type = new BasicExpressionType(builtinMod->tag);
            type->setSession(session);
            type->declRef = declRef;
            return type;
        }
        else if (auto magicMod = declRef.getDecl()->FindModifier<MagicTypeModifier>())
        {
            GenericSubstitution* subst = nullptr;
            for(auto s = declRef.substitutions.substitutions; s; s = s->outer)
            {
                if(auto genericSubst = s.As<GenericSubstitution>())
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
                SLANG_ASSERT(subst && subst->args.Count() == 2);
                auto vecType = new VectorExpressionType();
                vecType->setSession(session);
                vecType->declRef = declRef;
                vecType->elementType = ExtractGenericArgType(subst->args[0]);
                vecType->elementCount = ExtractGenericArgInteger(subst->args[1]);
                return vecType;
            }
            else if (magicMod->name == "Matrix")
            {
                SLANG_ASSERT(subst && subst->args.Count() == 3);
                auto matType = new MatrixExpressionType();
                matType->setSession(session);
                matType->declRef = declRef;
                return matType;
            }
            else if (magicMod->name == "Texture")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new TextureType(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "TextureSampler")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new TextureSamplerType(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "GLSLImageType")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
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
                    SLANG_ASSERT(subst && subst->args.Count() == 1);			\
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

                auto type = classInfo.createInstance();
                if (!type)
                {
                    SLANG_UNEXPECTED("constructor failure");
                }

                auto declRefType = dynamic_cast<DeclRefType*>(type);
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

    String OverloadGroupType::ToString()
    {
        return "overload group";
    }

    bool OverloadGroupType::EqualsImpl(Type * /*type*/)
    {
        return false;
    }

    RefPtr<Type> OverloadGroupType::CreateCanonicalType()
    {
        return this;
    }

    int OverloadGroupType::GetHashCode()
    {
        return (int)(int64_t)(void*)this;
    }

    // InitializerListType

    String InitializerListType::ToString()
    {
        return "initializer list";
    }

    bool InitializerListType::EqualsImpl(Type * /*type*/)
    {
        return false;
    }

    RefPtr<Type> InitializerListType::CreateCanonicalType()
    {
        return this;
    }

    int InitializerListType::GetHashCode()
    {
        return (int)(int64_t)(void*)this;
    }

    // ErrorType

    String ErrorType::ToString()
    {
        return "error";
    }

    bool ErrorType::EqualsImpl(Type* type)
    {
        if (auto errorType = type->As<ErrorType>())
            return true;
        return false;
    }

    RefPtr<Type> ErrorType::CreateCanonicalType()
    {
        return this;
    }

    RefPtr<Val> ErrorType::SubstituteImpl(SubstitutionSet /*subst*/, int* /*ioDiff*/)
    {
        return this;
    }

    int ErrorType::GetHashCode()
    {
        return (int)(int64_t)(void*)this;
    }


    // NamedExpressionType

    String NamedExpressionType::ToString()
    {
        return getText(declRef.GetName());
    }

    bool NamedExpressionType::EqualsImpl(Type * /*type*/)
    {
        SLANG_UNEXPECTED("unreachable");
        UNREACHABLE_RETURN(false);
    }

    RefPtr<Type> NamedExpressionType::CreateCanonicalType()
    {
        if (!innerType)
            innerType = GetType(declRef);
        return innerType->GetCanonicalType();
    }

    int NamedExpressionType::GetHashCode()
    {
        // Type equality is based on comparing canonical types,
        // so the hash code for a type needs to come from the
        // canonical version of the type. This really means
        // that `Type::GetHashCode()` should dispatch out to
        // something like `Type::GetHashCodeImpl()` on the
        // canonical version of a type, but it is less invasive
        // for now (and hopefully equivalent) to just have any
        // named types automaticlaly route hash-code requests
        // to their canonical type.
        return GetCanonicalType()->GetHashCode();
    }

    // FuncType

    String FuncType::ToString()
    {
        StringBuilder sb;
        sb << "(";
        UInt paramCount = getParamCount();
        for (UInt pp = 0; pp < paramCount; ++pp)
        {
            if (pp != 0) sb << ", ";
            sb << getParamType(pp)->ToString();
        }
        sb << ") -> ";
        sb << getResultType()->ToString();
        return sb.ProduceString();
    }

    bool FuncType::EqualsImpl(Type * type)
    {
        if (auto funcType = type->As<FuncType>())
        {
            auto paramCount = getParamCount();
            auto otherParamCount = funcType->getParamCount();
            if (paramCount != otherParamCount)
                return false;

            for (UInt pp = 0; pp < paramCount; ++pp)
            {
                auto paramType = getParamType(pp);
                auto otherParamType = funcType->getParamType(pp);
                if (!paramType->Equals(otherParamType))
                    return false;
            }

            if(!resultType->Equals(funcType->resultType))
                return false;

            // TODO: if we ever introduce other kinds
            // of qualification on function types, we'd
            // want to consider it here.
            return true;
        }
        return false;
    }

    RefPtr<Val> FuncType::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;

        // result type
        RefPtr<Type> substResultType = resultType->SubstituteImpl(subst, &diff).As<Type>();

        // parameter types
        List<RefPtr<Type>> substParamTypes;
        for( auto pp : paramTypes )
        {
            substParamTypes.Add(pp->SubstituteImpl(subst, &diff).As<Type>());
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

    RefPtr<Type> FuncType::CreateCanonicalType()
    {
        // result type
        RefPtr<Type> canResultType = resultType->GetCanonicalType();

        // parameter types
        List<RefPtr<Type>> canParamTypes;
        for( auto pp : paramTypes )
        {
            canParamTypes.Add(pp->GetCanonicalType());
        }

        RefPtr<FuncType> canType = new FuncType();
        canType->session = session;
        canType->resultType = resultType;
        canType->paramTypes = canParamTypes;

        return canType;
    }

    int FuncType::GetHashCode()
    {
        int hashCode = getResultType()->GetHashCode();
        UInt paramCount = getParamCount();
        hashCode = combineHash(hashCode, Slang::GetHashCode(paramCount));
        for (UInt pp = 0; pp < paramCount; ++pp)
        {
            hashCode = combineHash(
                hashCode,
                getParamType(pp)->GetHashCode());
        }
        return hashCode;
    }

    // TypeType

    String TypeType::ToString()
    {
        StringBuilder sb;
        sb << "typeof(" << type->ToString() << ")";
        return sb.ProduceString();
    }

    bool TypeType::EqualsImpl(Type * t)
    {
        if (auto typeType = t->As<TypeType>())
        {
            return t->Equals(typeType->type);
        }
        return false;
    }

    RefPtr<Type> TypeType::CreateCanonicalType()
    {
        auto canType = getTypeType(type->GetCanonicalType());
        return canType;
    }

    int TypeType::GetHashCode()
    {
        SLANG_UNEXPECTED("unreachable");
        UNREACHABLE_RETURN(0);
    }

    // GenericDeclRefType

    String GenericDeclRefType::ToString()
    {
        // TODO: what is appropriate here?
        return "<DeclRef<GenericDecl>>";
    }

    bool GenericDeclRefType::EqualsImpl(Type * type)
    {
        if (auto genericDeclRefType = type->As<GenericDeclRefType>())
        {
            return declRef.Equals(genericDeclRefType->declRef);
        }
        return false;
    }

    int GenericDeclRefType::GetHashCode()
    {
        return declRef.GetHashCode();
    }

    RefPtr<Type> GenericDeclRefType::CreateCanonicalType()
    {
        return this;
    }

    // ArithmeticExpressionType

    // VectorExpressionType

    String VectorExpressionType::ToString()
    {
        StringBuilder sb;
        sb << "vector<" << elementType->ToString() << "," << elementCount->ToString() << ">";
        return sb.ProduceString();
    }

    BasicExpressionType* VectorExpressionType::GetScalarType()
    {
        return elementType->AsBasicType();
    }

    //

    RefPtr<GenericSubstitution> findInnerMostGenericSubstitution(Substitutions* subst)
    {
        for(RefPtr<Substitutions> s = subst; s; s = s->outer)
        {
            if(auto genericSubst = s.As<GenericSubstitution>())
                return genericSubst;
        }
        return nullptr;
    }

    // MatrixExpressionType

    String MatrixExpressionType::ToString()
    {
        StringBuilder sb;
        sb << "matrix<" << getElementType()->ToString() << "," << getRowCount()->ToString() << "," << getColumnCount()->ToString() << ">";
        return sb.ProduceString();
    }

    BasicExpressionType* MatrixExpressionType::GetScalarType()
    {
        return getElementType()->AsBasicType();
    }

    Type* MatrixExpressionType::getElementType()
    {
        return findInnerMostGenericSubstitution(declRef.substitutions)->args[0].As<Type>().Ptr();
    }

    IntVal* MatrixExpressionType::getRowCount()
    {
        return findInnerMostGenericSubstitution(declRef.substitutions)->args[1].As<IntVal>().Ptr();
    }

    IntVal* MatrixExpressionType::getColumnCount()
    {
        return findInnerMostGenericSubstitution(declRef.substitutions)->args[2].As<IntVal>().Ptr();
    }

    // PtrTypeBase

    Type* PtrTypeBase::getValueType()
    {
        return findInnerMostGenericSubstitution(declRef.substitutions)->args[0].As<Type>().Ptr();
    }

    // GenericParamIntVal

    bool GenericParamIntVal::EqualsVal(Val* val)
    {
        if (auto genericParamVal = dynamic_cast<GenericParamIntVal*>(val))
        {
            return declRef.Equals(genericParamVal->declRef);
        }
        return false;
    }

    String GenericParamIntVal::ToString()
    {
        return getText(declRef.GetName());
    }

    int GenericParamIntVal::GetHashCode()
    {
        return declRef.GetHashCode() ^ 0xFFFF;
    }

    RefPtr<Val> GenericParamIntVal::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        // search for a substitution that might apply to us
        for(auto s = subst.substitutions; s; s = s->outer)
        {
            auto genSubst = s.As<GenericSubstitution>();
            if(!genSubst)
                continue;

            // the generic decl associated with the substitution list must be
            // the generic decl that declared this parameter
            auto genericDecl = genSubst->genericDecl;
            if (genericDecl != declRef.getDecl()->ParentDecl)
                continue;

            int index = 0;
            for (auto m : genericDecl->Members)
            {
                if (m.Ptr() == declRef.getDecl())
                {
                    // We've found it, so return the corresponding specialization argument
                    (*ioDiff)++;
                    return genSubst->args[index];
                }
                else if (auto typeParam = m.As<GenericTypeParamDecl>())
                {
                    index++;
                }
                else if (auto valParam = m.As<GenericValueParamDecl>())
                {
                    index++;
                }
                else
                {
                }
            }
        }

        // Nothing found: don't substittue.
        return this;
    }

    // Substitutions

    RefPtr<Substitutions> GenericSubstitution::applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)
    {
        SLANG_ASSERT(this);

        int diff = 0;

        if(substOuter != outer) diff++;

        List<RefPtr<Val>> substArgs;
        for (auto a : args)
        {
            substArgs.Add(a->SubstituteImpl(substSet, &diff));
        }

        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new GenericSubstitution();
        substSubst->genericDecl = genericDecl;
        substSubst->args = substArgs;
        substSubst->outer = substOuter;
        return substSubst;
    }

    bool GenericSubstitution::Equals(Substitutions* subst)
    {
        // both must be NULL, or non-NULL
        if (!this || !subst)
            return !this && !subst;
        auto genericSubst = dynamic_cast<GenericSubstitution*>(subst);
        if (!genericSubst)
            return false;
        if (genericDecl != genericSubst->genericDecl)
            return false;

        UInt argCount = args.Count();
        SLANG_RELEASE_ASSERT(args.Count() == genericSubst->args.Count());
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            if (!args[aa]->EqualsVal(genericSubst->args[aa].Ptr()))
                return false;
        }

        if (!outer)
            return !genericSubst->outer;

        if (!outer->Equals(genericSubst->outer.Ptr()))
            return false;

        return true;
    }

    RefPtr<Substitutions> ThisTypeSubstitution::applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)
    {
        SLANG_ASSERT(this);

        int diff = 0;

        if(substOuter != outer) diff++;
        auto substWitness = witness->SubstituteImpl(substSet, &diff).As<SubtypeWitness>();

        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new ThisTypeSubstitution();
        substSubst->interfaceDecl = interfaceDecl;
        substSubst->witness = substWitness;
        substSubst->outer = substOuter;
        return substSubst;
    }

    bool ThisTypeSubstitution::Equals(Substitutions* subst)
    {
        SLANG_ASSERT(this);
        if (!subst)
            return false;

        if (auto thisTypeSubst = dynamic_cast<ThisTypeSubstitution*>(subst))
        {
            return witness->EqualsVal(thisTypeSubst->witness);
        }
        return false;
    }

    int ThisTypeSubstitution::GetHashCode() const
    {
        return witness->GetHashCode();
    }

    RefPtr<Substitutions> GlobalGenericParamSubstitution::applySubstitutionsShallow(SubstitutionSet substSet, RefPtr<Substitutions> substOuter, int* ioDiff)
    {
        // if we find a GlobalGenericParamSubstitution in subst that references the same __generic_param decl
        // return a copy of that GlobalGenericParamSubstitution
        int diff = 0;

        if(substOuter != outer) diff++;

        auto substActualType = actualType->SubstituteImpl(substSet, &diff).As<Type>();

        List<ConstraintArg> substConstraintArgs;
        for(auto constraintArg : constraintArgs)
        {
            ConstraintArg substConstraintArg;
            substConstraintArg.decl = constraintArg.decl;
            substConstraintArg.val = constraintArg.val->SubstituteImpl(substSet, &diff);

            substConstraintArgs.Add(substConstraintArg);
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

    bool GlobalGenericParamSubstitution::Equals(Substitutions* subst)
    {
        if (!subst)
            return false;
        if (auto genSubst = dynamic_cast<GlobalGenericParamSubstitution*>(subst))
        {
            if (paramDecl != genSubst->paramDecl)
                return false;
            if (!actualType->EqualsVal(genSubst->actualType))
                return false;
            if (constraintArgs.Count() != genSubst->constraintArgs.Count())
                return false;
            for (UInt i = 0; i < constraintArgs.Count(); i++)
            {
                if (!constraintArgs[i].val->EqualsVal(genSubst->constraintArgs[i].val))
                    return false;
            }
            return true;
        }
        return false;
    }


    // DeclRefBase

    RefPtr<Type> DeclRefBase::Substitute(RefPtr<Type> type) const
    {
        // No substitutions? Easy.
        if (!substitutions)
            return type;

        // Otherwise we need to recurse on the type structure
        // and apply substitutions where it makes sense

        return type->Substitute(substitutions).As<Type>();
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
            if(auto interfaceDecl = dd->As<InterfaceDecl>())
                return interfaceDecl;

            dd = dd->ParentDecl;
        }
        return nullptr;
    }

    RefPtr<GlobalGenericParamSubstitution> findGlobalGenericSubst(
        RefPtr<Substitutions>   substs,
        GlobalGenericParamDecl* paramDecl)
    {
        for(auto s = substs; s; s = s->outer)
        {
            auto gSubst = s.As<GlobalGenericParamSubstitution>();
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
            auto specGlobalGenericSubst = specSubst.As<GlobalGenericParamSubstitution>();
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
        while(appGlobalGenericSubsts && !appGlobalGenericSubsts.As<GlobalGenericParamSubstitution>())
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
            auto appGlobalGenericSubst = appSubst.As<GlobalGenericParamSubstitution>();
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
    // based on a provided substituion set to be applied
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
        for(Decl* ancestorDecl = declToSpecialize; ancestorDecl; ancestorDecl = ancestorDecl->ParentDecl)
        {
            if(auto ancestorGenericDecl = ancestorDecl->As<GenericDecl>())
            {
                // The declaration is nested inside a generic.
                // Does it already have a specialization for that generic?
                if(auto specGenericSubst = substsToSpecialize.As<GenericSubstitution>())
                {
                    if(specGenericSubst->genericDecl == ancestorGenericDecl)
                    {
                        // Yes. We have an existing specialization, so we will
                        // keep one matching it in place.
                        int diff = 0;
                        auto restSubst = specializeSubstitutions(
                            ancestorGenericDecl->ParentDecl,
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
                    auto appGenericSubst = s.As<GenericSubstitution>();
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
                        ancestorGenericDecl->ParentDecl,
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
            else if(auto ancestorInterfaceDecl = ancestorDecl->As<InterfaceDecl>())
            {
                // The task is basically the same as for the generic case:
                // We want to see if there is any existing substitution that
                // applies to this declaration, and use that if possible.

                // The declaration is nested inside a generic.
                // Does it already have a specialization for that generic?
                if(auto specThisTypeSubst = substsToSpecialize.As<ThisTypeSubstitution>())
                {
                    if(specThisTypeSubst->interfaceDecl == ancestorInterfaceDecl)
                    {
                        // Yes. We have an existing specialization, so we will
                        // keep one matching it in place.
                        int diff = 0;
                        auto restSubst = specializeSubstitutions(
                            ancestorInterfaceDecl->ParentDecl,
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
                    auto appThisTypeSubst = s.As<ThisTypeSubstitution>();
                    if(!appThisTypeSubst)
                        continue;

                    if(appThisTypeSubst->interfaceDecl != ancestorInterfaceDecl)
                        continue;

                    int diff = 0;
                    auto restSubst = specializeSubstitutions(
                        ancestorInterfaceDecl->ParentDecl,
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
        // substitutiosn in `substToSpecialize`, because otherwise they
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
        // in a paarticular implementation.
        //
        // I have only kept that logic in `DeclRefType::SubstituteImpl`,
        // but it may turn out it is needed here too.

        return substDeclRef;
    }


    // Check if this is an equivalent declaration reference to another
    bool DeclRefBase::Equals(DeclRefBase const& declRef) const
    {
        if (decl != declRef.decl)
            return false;
        if (!substitutions.Equals(declRef.substitutions))
            return false;

        return true;
    }

    // Convenience accessors for common properties of declarations
    Name* DeclRefBase::GetName() const
    {
        return decl->nameAndLoc.name;
    }

    DeclRefBase DeclRefBase::GetParent() const
    {
        auto parentDecl = decl->ParentDecl;
        if (!parentDecl)
            return DeclRefBase();

        // Default is to apply the same set of substitutions/specializations
        // to the parent declaration as were applied to the child.
        RefPtr<Substitutions> substToApply = substitutions.substitutions;

        if(auto interfaceDecl = dynamic_cast<InterfaceDecl*>(decl))
        {
            // The declaration being referenced is an `interface` declaration,
            // and there might be a this-type substitution in place.
            // A reference to the parent of the interface declaration
            // should not include that substitution.
            if(auto thisTypeSubst = substToApply.As<ThisTypeSubstitution>())
            {
                if(thisTypeSubst->interfaceDecl == interfaceDecl)
                {
                    // Strip away that specializations that apply to the interface.
                    substToApply = thisTypeSubst->outer;
                }
            }
        }

        if (auto parentGenericDecl = dynamic_cast<GenericDecl*>(parentDecl))
        {
            // The parent of this declaration is a generic, which means
            // that the decl-ref to the current declaration might include
            // substitutiosn that specialize the generic parameters.
            // A decl-ref to the parent generic should *not* include
            // those substitutions.
            //
            if(auto genericSubst = substToApply.As<GenericSubstitution>())
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

    int DeclRefBase::GetHashCode() const
    {
        return combineHash(PointerHash<1>::GetHashCode(decl), substitutions.GetHashCode());
    }

    // Val

    RefPtr<Val> Val::Substitute(SubstitutionSet subst)
    {
        SLANG_ASSERT(this);
        if (!subst) return this;
        int diff = 0;
        return SubstituteImpl(subst, &diff);
    }

    RefPtr<Val> Val::SubstituteImpl(SubstitutionSet /*subst*/, int* /*ioDiff*/)
    {
        // Default behavior is to not substitute at all
        return this;
    }

    // IntVal

    IntegerLiteralValue GetIntVal(RefPtr<IntVal> val)
    {
        if (auto constantVal = val.As<ConstantIntVal>())
        {
            return constantVal->value;
        }
        SLANG_UNEXPECTED("needed a known integer value");
        return 0;
    }

    // ConstantIntVal

    bool ConstantIntVal::EqualsVal(Val* val)
    {
        if (auto intVal = dynamic_cast<ConstantIntVal*>(val))
            return value == intVal->value;
        return false;
    }

    String ConstantIntVal::ToString()
    {
        return String(value);
    }

    int ConstantIntVal::GetHashCode()
    {
        return (int) value;
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
        session->magicDecls[modifier->name] = decl.Ptr();
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
        return findInnerMostGenericSubstitution(declRef.substitutions)->args[0].As<Type>().Ptr();
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return findInnerMostGenericSubstitution(declRef.substitutions)->args[1].As<IntVal>().Ptr();
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
        arrayType->ArrayLength = elementCount;
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
        DeclRef<TypeDefDecl> specializedDeclRef = createDefaultSubstitutionsIfNeeded(session, declRef).As<TypeDefDecl>();

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
            if( paramDecl->FindModifier<RefModifier>() )
            {
                paramType = session->getRefType(paramType);
            }
            else if( paramDecl->FindModifier<OutModifier>() )
            {
                if(paramDecl->FindModifier<InOutModifier>() || paramDecl->FindModifier<InModifier>())
                {
                    paramType = session->getInOutType(paramType);
                }
                else
                {
                    paramType = session->getOutType(paramType);
                }
            }
            funcType->paramTypes.Add(paramType);
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

    RefPtr<SamplerStateType> getSamplerStateType(
        Session*        session)
    {
        auto samplerStateType = new SamplerStateType();
        samplerStateType->setSession(session);
        return samplerStateType;
    }

    // TODO: should really have a `type.cpp` and a `witness.cpp`

    bool TypeEqualityWitness::EqualsVal(Val* val)
    {
        auto otherWitness = dynamic_cast<TypeEqualityWitness*>(val);
        if (!otherWitness)
            return false;
        return sub->Equals(otherWitness->sub);
    }

    RefPtr<Val> TypeEqualityWitness::SubstituteImpl(SubstitutionSet subst, int * ioDiff)
    {
        RefPtr<TypeEqualityWitness> rs = new TypeEqualityWitness();
        rs->sub = sub->SubstituteImpl(subst, ioDiff).As<Type>();
        rs->sup = sup->SubstituteImpl(subst, ioDiff).As<Type>();
        return rs;
    }

    String TypeEqualityWitness::ToString()
    {
        return "TypeEqualityWitness(" + sub->ToString() + ")";
    }

    int TypeEqualityWitness::GetHashCode()
    {
        return sub->GetHashCode();
    }

    bool DeclaredSubtypeWitness::EqualsVal(Val* val)
    {
        auto otherWitness = dynamic_cast<DeclaredSubtypeWitness*>(val);
        if(!otherWitness)
            return false;

        return sub->Equals(otherWitness->sub)
            && sup->Equals(otherWitness->sup)
            && declRef.Equals(otherWitness->declRef);
    }

    RefPtr<ThisTypeSubstitution> findThisTypeSubstitution(
        Substitutions*  substs,
        InterfaceDecl*  interfaceDecl)
    {
        for(RefPtr<Substitutions> s = substs; s; s = s->outer)
        {
            auto thisTypeSubst = s.As<ThisTypeSubstitution>();
            if(!thisTypeSubst)
                continue;

            if(thisTypeSubst->interfaceDecl != interfaceDecl)
                continue;

            return thisTypeSubst;
        }

        return nullptr;
    }

    RefPtr<Val> DeclaredSubtypeWitness::SubstituteImpl(SubstitutionSet subst, int * ioDiff)
    {
        if (auto genConstraintDeclRef = declRef.As<GenericTypeConstraintDecl>())
        {
            auto genConstraintDecl = genConstraintDeclRef.getDecl();

            // search for a substitution that might apply to us
            for(auto s = subst.substitutions; s; s = s->outer)
            {
                if(auto genericSubst = s.As<GenericSubstitution>())
                {
                    // the generic decl associated with the substitution list must be
                    // the generic decl that declared this parameter
                    auto genericDecl = genericSubst->genericDecl;
                    if (genericDecl != genConstraintDecl->ParentDecl)
                        continue;

                    bool found = false;
                    UInt index = 0;
                    for (auto m : genericDecl->Members)
                    {
                        if (auto constraintParam = m.As<GenericTypeConstraintDecl>())
                        {
                            if (constraintParam.Ptr() == declRef.getDecl())
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
                        auto ordinaryParamCount = genericDecl->getMembersOfType<GenericTypeParamDecl>().Count() +
                            genericDecl->getMembersOfType<GenericValueParamDecl>().Count();
                        SLANG_ASSERT(index + ordinaryParamCount < genericSubst->args.Count());
                        return genericSubst->args[index + ordinaryParamCount];
                    }
                }
                else if(auto globalGenericSubst = s.As<GlobalGenericParamSubstitution>())
                {
                    // check if the substitution is really about this global generic type parameter
                    if (globalGenericSubst->paramDecl != genConstraintDecl->ParentDecl)
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
        auto substSub = sub->SubstituteImpl(subst, &diff).As<Type>();
        auto substSup = sup->SubstituteImpl(subst, &diff).As<Type>();
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
        if (auto substTypeConstraintDecl = substDeclRef.decl->As<GenericTypeConstraintDecl>())
        {
            if (auto substAssocTypeDecl = substTypeConstraintDecl->ParentDecl->As<AssocTypeDecl>())
            {
                if (auto interfaceDecl = substAssocTypeDecl->ParentDecl->As<InterfaceDecl>())
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

    String DeclaredSubtypeWitness::ToString()
    {
        StringBuilder sb;
        sb << "DeclaredSubtypeWitness(";
        sb << this->sub->ToString();
        sb << ", ";
        sb << this->sup->ToString();
        sb << ", ";
        sb << this->declRef.toString();
        sb << ")";
        return sb.ProduceString();
    }

    int DeclaredSubtypeWitness::GetHashCode()
    {
        return declRef.GetHashCode();
    }

    // TransitiveSubtypeWitness

    bool TransitiveSubtypeWitness::EqualsVal(Val* val)
    {
        auto otherWitness = dynamic_cast<TransitiveSubtypeWitness*>(val);
        if(!otherWitness)
            return false;

        return sub->Equals(otherWitness->sub)
            && sup->Equals(otherWitness->sup)
            && subToMid->EqualsVal(otherWitness->subToMid)
            && midToSup.Equals(otherWitness->midToSup);
    }

    RefPtr<Val> TransitiveSubtypeWitness::SubstituteImpl(SubstitutionSet subst, int * ioDiff)
    {
        int diff = 0;

        RefPtr<Type> substSub = sub->SubstituteImpl(subst, &diff).As<Type>();
        RefPtr<Type> substSup = sup->SubstituteImpl(subst, &diff).As<Type>();
        RefPtr<SubtypeWitness> substSubToMid = subToMid->SubstituteImpl(subst, &diff).As<SubtypeWitness>();
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

    String TransitiveSubtypeWitness::ToString()
    {
        // Note: we only print the constituent
        // witnesses, and rely on them to print
        // the starting and ending types.
        StringBuilder sb;
        sb << "TransitiveSubtypeWitness(";
        sb << this->subToMid->ToString();
        sb << ", ";
        sb << this->midToSup.toString();
        sb << ")";
        return sb.ProduceString();
    }

    int TransitiveSubtypeWitness::GetHashCode()
    {
        auto hash = sub->GetHashCode();
        hash = combineHash(hash, sup->GetHashCode());
        hash = combineHash(hash, subToMid->GetHashCode());
        hash = combineHash(hash, midToSup.GetHashCode());
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

    bool SubstitutionSet::Equals(SubstitutionSet substSet) const
    {
        if(!substitutions || !substSet.substitutions)
            return substitutions == substSet.substitutions;

        return substitutions->Equals(substSet.substitutions);
    }

    int SubstitutionSet::GetHashCode() const
    {
        int rs = 0;
        if (substitutions)
            rs = combineHash(rs, substitutions->GetHashCode());
        return rs;
    }

    // ExtractExistentialType

    String ExtractExistentialType::ToString()
    {
        String result;
        result.append(declRef.toString());
        result.append(".This");
        return result;
    }

    bool ExtractExistentialType::EqualsImpl(Type* type)
    {
        if( auto extractExistential = type->As<ExtractExistentialType>() )
        {
            return declRef.Equals(extractExistential->declRef);
        }
        return false;
    }

    int ExtractExistentialType::GetHashCode()
    {
        return declRef.GetHashCode();
    }

    RefPtr<Type> ExtractExistentialType::CreateCanonicalType()
    {
        return this;
    }

    RefPtr<Val> ExtractExistentialType::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
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

    bool ExtractExistentialSubtypeWitness::EqualsVal(Val* val)
    {
        if( auto extractWitness = val->dynamicCast<ExtractExistentialSubtypeWitness>() )
        {
            return declRef.Equals(extractWitness->declRef);
        }
        return false;
    }

    String ExtractExistentialSubtypeWitness::ToString()
    {
        String result;
        result.append("extractExistentialValue(");
        result.append(declRef.toString());
        result.append(")");
        return result;
    }

    int ExtractExistentialSubtypeWitness::GetHashCode()
    {
        return declRef.GetHashCode();
    }

    RefPtr<Val> ExtractExistentialSubtypeWitness::SubstituteImpl(SubstitutionSet subst, int* ioDiff)
    {
        int diff = 0;

        auto substDeclRef = declRef.SubstituteImpl(subst, &diff);
        auto substSub = sub->SubstituteImpl(subst, &diff).As<Type>();
        auto substSup = sup->SubstituteImpl(subst, &diff).As<Type>();

        if(!diff)
            return this;

        (*ioDiff)++;

        RefPtr<ExtractExistentialSubtypeWitness> substValue = new ExtractExistentialSubtypeWitness();
        substValue->declRef = declRef;
        substValue->sub = substSub;
        substValue->sup = substSup;
        return substValue;
    }



}

