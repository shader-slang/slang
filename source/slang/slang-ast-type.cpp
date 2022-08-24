// slang-ast-type.cpp
#include "slang-ast-builder.h"
#include <assert.h>
#include <typeinfo>

#include "slang-syntax.h"

#include "slang-generated-ast-macro.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Type* Type::createCanonicalType()
{
    SLANG_AST_NODE_VIRTUAL_CALL(Type, createCanonicalType, ())
}

bool Type::equals(Type* type)
{
    return getCanonicalType()->equalsImpl(type->getCanonicalType());
}

bool Type::equalsImpl(Type* type)
{
    SLANG_AST_NODE_VIRTUAL_CALL(Type, equalsImpl, (type))
}

bool Type::_equalsImplOverride(Type* type)
{
    SLANG_UNUSED(type)
    SLANG_UNEXPECTED("Type::_equalsImplOverride not overridden");
    //return false;
}

Type* Type::_createCanonicalTypeOverride()
{
    SLANG_UNEXPECTED("Type::_createCanonicalTypeOverride not overridden");
    //return Type*();
}

bool Type::_equalsValOverride(Val* val)
{
    if (auto type = dynamicCast<Type>(val))
        return const_cast<Type*>(this)->equals(type);
    return false;
}

Val* Type::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto canSubst = getCanonicalType()->substituteImpl(astBuilder, subst, &diff);

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

        SLANG_ASSERT(et->canonicalType);
    }
    return et->canonicalType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! OverloadGroupType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void OverloadGroupType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("overload group");
}

bool OverloadGroupType::_equalsImplOverride(Type * /*type*/)
{
    return false;
}

Type* OverloadGroupType::_createCanonicalTypeOverride()
{
    return this;
}

HashCode OverloadGroupType::_getHashCodeOverride()
{
    return (HashCode)(size_t(this));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! InitializerListType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void InitializerListType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("initializer list");
}

bool InitializerListType::_equalsImplOverride(Type * /*type*/)
{
    return false;
}

Type* InitializerListType::_createCanonicalTypeOverride()
{
    return this;
}

HashCode InitializerListType::_getHashCodeOverride()
{
    return (HashCode)(size_t(this));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ErrorType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ErrorType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("error");
}

bool ErrorType::_equalsImplOverride(Type* type)
{
    if (auto errorType = as<ErrorType>(type))
        return true;
    return false;
}

Type* ErrorType::_createCanonicalTypeOverride()
{
    return this;
}

Val* ErrorType::_substituteImplOverride(ASTBuilder* /* astBuilder */, SubstitutionSet /*subst*/, int* /*ioDiff*/)
{
    return this;
}

HashCode ErrorType::_getHashCodeOverride()
{
    return HashCode(size_t(this));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BottomType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void BottomType::_toTextOverride(StringBuilder& out) { out << toSlice("never"); }

bool BottomType::_equalsImplOverride(Type* type)
{
    if (auto bottomType = as<BottomType>(type))
        return true;
    return false;
}

Type* BottomType::_createCanonicalTypeOverride() { return this; }

Val* BottomType::_substituteImplOverride(
    ASTBuilder* /* astBuilder */, SubstitutionSet /*subst*/, int* /*ioDiff*/)
{
    return this;
}

HashCode BottomType::_getHashCodeOverride() { return HashCode(size_t(this)); }

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclRefType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void DeclRefType::_toTextOverride(StringBuilder& out)
{
    out << declRef;
}

HashCode DeclRefType::_getHashCodeOverride()
{
    return (declRef.getHashCode() * 16777619) ^ (HashCode)(typeid(this).hash_code());
}

bool DeclRefType::_equalsImplOverride(Type * type)
{
    if (auto declRefType = as<DeclRefType>(type))
    {
        return declRef.equals(declRefType->declRef);
    }
    return false;
}

Type* DeclRefType::_createCanonicalTypeOverride()
{
    // A declaration reference is already canonical
    return this;
}

Val* DeclRefType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    if (!subst) return this;

    // the case we especially care about is when this type references a declaration
    // of a generic parameter, since that is what we might be substituting...
    if (auto genericTypeParamDecl = as<GenericTypeParamDecl>(declRef.getDecl()))
    {
        // search for a substitution that might apply to us
        for (auto s = subst.substitutions; s; s = s->outer)
        {
            auto genericSubst = as<GenericSubstitution>(s);
            if (!genericSubst)
                continue;

            // the generic decl associated with the substitution list must be
            // the generic decl that declared this parameter
            auto genericDecl = genericSubst->genericDecl;
            if (genericDecl != genericTypeParamDecl->parentDecl)
                continue;

            int index = 0;
            for (auto m : genericDecl->members)
            {
                if (m == genericTypeParamDecl)
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
    int diff = 0;
    DeclRef<Decl> substDeclRef = declRef.substituteImpl(astBuilder, subst, &diff);

    if (!diff)
        return this;

    // Make sure to record the difference!
    *ioDiff += diff;

    // If this type is a reference to an associated type declaration,
    // and the substitutions provide a "this type" substitution for
    // the outer interface, then try to replace the type with the
    // actual value of the associated type for the given implementation.
    //
    if (auto substAssocTypeDecl = as<AssocTypeDecl>(substDeclRef.decl))
    {
        for (auto s = substDeclRef.substitutions.substitutions; s; s = s->outer)
        {
            auto thisSubst = as<ThisTypeSubstitution>(s);
            if (!thisSubst)
                continue;

            if (auto interfaceDecl = as<InterfaceDecl>(substAssocTypeDecl->parentDecl))
            {
                if (thisSubst->interfaceDecl == interfaceDecl)
                {
                    // We need to look up the declaration that satisfies
                    // the requirement named by the associated type.
                    Decl* requirementKey = substAssocTypeDecl;
                    RequirementWitness requirementWitness = tryLookUpRequirementWitness(astBuilder, thisSubst->witness, requirementKey);
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
                }
            }
        }
    }

    // Re-construct the type in case we are using a specialized sub-class
    return DeclRefType::create(astBuilder, substDeclRef);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArithmeticExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


BasicExpressionType* ArithmeticExpressionType::getScalarType()
{
    SLANG_AST_NODE_VIRTUAL_CALL(ArithmeticExpressionType, getScalarType, ())
}

BasicExpressionType* ArithmeticExpressionType::_getScalarTypeOverride()
{
    SLANG_UNEXPECTED("ArithmeticExpressionType::_getScalarTypeOverride not overridden");
    //return nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BasicExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool BasicExpressionType::_equalsImplOverride(Type * type)
{
    auto basicType = as<BasicExpressionType>(type);
    return basicType && basicType->baseType == this->baseType;
}

Type* BasicExpressionType::_createCanonicalTypeOverride()
{
    // A basic type is already canonical, in our setup
    return this;
}

BasicExpressionType* BasicExpressionType::_getScalarTypeOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VectorExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void VectorExpressionType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("vector<") << elementType << toSlice(",") << elementCount << toSlice(">");
}

BasicExpressionType* VectorExpressionType::_getScalarTypeOverride()
{
    return as<BasicExpressionType>(elementType);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MatrixExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void MatrixExpressionType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("matrix<") << getElementType() << toSlice(",") << getRowCount() << toSlice(",") << getColumnCount() << toSlice(">");
}

BasicExpressionType* MatrixExpressionType::_getScalarTypeOverride()
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

Type* MatrixExpressionType::getRowType()
{
    if (!rowType)
    {
        rowType = m_astBuilder->getVectorType(getElementType(), getColumnCount());
    }
    return rowType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArrayExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool ArrayExpressionType::_equalsImplOverride(Type* type)
{
    auto arrType = as<ArrayExpressionType>(type);
    if (!arrType)
        return false;
    return (areValsEqual(arrayLength, arrType->arrayLength) && baseType->equals(arrType->baseType));
}

Val* ArrayExpressionType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto elementType = as<Type>(baseType->substituteImpl(astBuilder, subst, &diff));
    IntVal* newArrayLength = nullptr;
    if (arrayLength)
    {
        newArrayLength = as<IntVal>(arrayLength->substituteImpl(astBuilder, subst, &diff));
        SLANG_ASSERT(newArrayLength);
    }
    if (diff)
    {
        *ioDiff = 1;
        auto rsType = getArrayType(
            astBuilder,
            elementType,
            newArrayLength);
        return rsType;
    }
    return this;
}

Type* ArrayExpressionType::_createCanonicalTypeOverride()
{
    auto canonicalElementType = baseType->getCanonicalType();
    auto canonicalArrayType = getASTBuilder()->getArrayType(
        canonicalElementType,
        arrayLength);
    return canonicalArrayType;
}

HashCode ArrayExpressionType::_getHashCodeOverride()
{
    if (arrayLength)
        return (baseType->getHashCode() * 16777619) ^ arrayLength->getHashCode();
    else
        return baseType->getHashCode();
}

void ArrayExpressionType::_toTextOverride(StringBuilder& out)
{
    out << baseType;
    out.appendChar('[');
    if (arrayLength)
    {
        out << arrayLength;
    }
    out.appendChar(']');
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TypeType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TypeType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("typeof(") << type << toSlice(")");
}

bool TypeType::_equalsImplOverride(Type * t)
{
    if (auto typeType = as<TypeType>(t))
    {
        return t->equals(typeType->type);
    }
    return false;
}

Type* TypeType::_createCanonicalTypeOverride()
{
    return getASTBuilder()->getTypeType(type->getCanonicalType());
}

HashCode TypeType::_getHashCodeOverride()
{
    SLANG_UNEXPECTED("TypeType::_getHashCodeOverride should be unreachable");
    //return HashCode(0);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GenericDeclRefType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void GenericDeclRefType::_toTextOverride(StringBuilder& out)
{
    // TODO: what is appropriate here?
    out << toSlice("<DeclRef<GenericDecl>>");
}

bool GenericDeclRefType::_equalsImplOverride(Type * type)
{
    if (auto genericDeclRefType = as<GenericDeclRefType>(type))
    {
        return declRef.equals(genericDeclRefType->declRef);
    }
    return false;
}

HashCode GenericDeclRefType::_getHashCodeOverride()
{
    return declRef.getHashCode();
}

Type* GenericDeclRefType::_createCanonicalTypeOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NamespaceType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NamespaceType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("namespace ") << declRef; 
}

bool NamespaceType::_equalsImplOverride(Type * type)
{
    if (auto namespaceType = as<NamespaceType>(type))
    {
        return declRef.equals(namespaceType->declRef);
    }
    return false;
}

HashCode NamespaceType::_getHashCodeOverride()
{
    return declRef.getHashCode();
}

Type* NamespaceType::_createCanonicalTypeOverride()
{
    return this;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PtrTypeBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Type* PtrTypeBase::getValueType()
{
    return as<Type>(findInnerMostGenericSubstitution(declRef.substitutions)->args[0]);
}

Type* OptionalType::getValueType()
{
    return as<Type>(findInnerMostGenericSubstitution(declRef.substitutions)->args[0]);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NamedExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NamedExpressionType::_toTextOverride(StringBuilder& out)
{
    Name* name = declRef.getName();
    if (name)
    {
        out << name->text;
    }
}

bool NamedExpressionType::_equalsImplOverride(Type * /*type*/)
{
    SLANG_UNEXPECTED("NamedExpressionType::_equalsImplOverride should be unreachable");
    //return false;
}

Type* NamedExpressionType::_createCanonicalTypeOverride()
{
    if (!innerType)
        innerType = getType(m_astBuilder, declRef);
    return innerType->getCanonicalType();
}

HashCode NamedExpressionType::_getHashCodeOverride()
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FuncType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ParameterDirection FuncType::getParamDirection(Index index)
{
    auto paramType = getParamType(index);
    if (as<RefType>(paramType))
    {
        return kParameterDirection_Ref;
    }
    else if (as<InOutType>(paramType))
    {
        return kParameterDirection_InOut;
    }
    else if (as<OutType>(paramType))
    {
        return kParameterDirection_Out;
    }
    else
    {
        return kParameterDirection_In;
    }
}

void FuncType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("(");
    Index paramCount = getParamCount();
    for (Index pp = 0; pp < paramCount; ++pp)
    {
        if (pp != 0)
        {
            out << toSlice(", ");
        }
        out << getParamType(pp);
    }
    out << toSlice(") -> ") << getResultType();

    if (!getErrorType()->equals(getASTBuilder()->getBottomType()))
    {
        out << " throws " << getErrorType();
    }
}

bool FuncType::_equalsImplOverride(Type * type)
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

        if (!resultType->equals(funcType->resultType))
            return false;

        if (!errorType->equals(funcType->errorType))
            return false;

        // TODO: if we ever introduce other kinds
        // of qualification on function types, we'd
        // want to consider it here.
        return true;
    }
    return false;
}

Val* FuncType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    // result type
    Type* substResultType = as<Type>(resultType->substituteImpl(astBuilder, subst, &diff));

    // error type
    Type* substErrorType = as<Type>(errorType->substituteImpl(astBuilder, subst, &diff));

    // parameter types
    List<Type*> substParamTypes;
    for (auto pp : paramTypes)
    {
        substParamTypes.add(as<Type>(pp->substituteImpl(astBuilder, subst, &diff)));
    }

    // early exit for no change...
    if (!diff)
        return this;

    (*ioDiff)++;
    FuncType* substType = astBuilder->create<FuncType>();
    substType->resultType = substResultType;
    substType->paramTypes = substParamTypes;
    substType->errorType = substErrorType;
    return substType;
}

Type* FuncType::_createCanonicalTypeOverride()
{
    // result type
    Type* canResultType = resultType->getCanonicalType();
    Type* canErrorType = errorType->getCanonicalType();

    // parameter types
    List<Type*> canParamTypes;
    for (auto pp : paramTypes)
    {
        canParamTypes.add(pp->getCanonicalType());
    }

    FuncType* canType = getASTBuilder()->create<FuncType>();
    canType->resultType = canResultType;
    canType->paramTypes = canParamTypes;
    canType->errorType = canErrorType;
    return canType;
}

HashCode FuncType::_getHashCodeOverride()
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
    combineHash(hashCode, getErrorType()->getHashCode());
    return hashCode;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExtractExistentialType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExtractExistentialType::_toTextOverride(StringBuilder& out)
{
    out << declRef << toSlice(".This");
}

bool ExtractExistentialType::_equalsImplOverride(Type* type)
{
    if (auto extractExistential = as<ExtractExistentialType>(type))
    {
        return declRef.equals(extractExistential->declRef);
    }
    return false;
}

HashCode ExtractExistentialType::_getHashCodeOverride()
{
    return combineHash(declRef.getHashCode(), originalInterfaceType->getHashCode(), originalInterfaceDeclRef.getHashCode());
}

Type* ExtractExistentialType::_createCanonicalTypeOverride()
{
    return this;
}

Val* ExtractExistentialType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto substDeclRef = declRef.substituteImpl(astBuilder, subst, &diff);
    auto substOriginalInterfaceType = originalInterfaceType->substituteImpl(astBuilder, subst, &diff);
    auto substOriginalInterfaceDeclRef = originalInterfaceDeclRef.substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;

    (*ioDiff)++;

    ExtractExistentialType* substValue = astBuilder->create<ExtractExistentialType>();
    substValue->declRef = substDeclRef;
    substValue->originalInterfaceType = as<Type>(substOriginalInterfaceType);
    substValue->originalInterfaceDeclRef = substOriginalInterfaceDeclRef;
    return substValue;
}

SubtypeWitness* ExtractExistentialType::getSubtypeWitness()
{
    if (auto cachedValue = this->cachedSubtypeWitness)
        return cachedValue;

    ExtractExistentialSubtypeWitness* openedWitness = m_astBuilder->create<ExtractExistentialSubtypeWitness>();
    openedWitness->sub = this;
    openedWitness->sup = originalInterfaceType;
    openedWitness->declRef = this->declRef;

    this->cachedSubtypeWitness = openedWitness;
    return openedWitness;
}

DeclRef<InterfaceDecl> ExtractExistentialType::getSpecializedInterfaceDeclRef()
{
    if (auto cachedValue = this->cachedSpecializedInterfaceDeclRef)
        return cachedValue;

    auto interfaceDecl = originalInterfaceDeclRef.getDecl();

    SubtypeWitness* openedWitness = getSubtypeWitness();

    ThisTypeSubstitution* openedThisType = m_astBuilder->create<ThisTypeSubstitution>();
    openedThisType->outer = originalInterfaceDeclRef.substitutions.substitutions;
    openedThisType->interfaceDecl = interfaceDecl;
    openedThisType->witness = openedWitness;

    DeclRef<InterfaceDecl> specialiedInterfaceDeclRef = DeclRef<InterfaceDecl>(interfaceDecl, openedThisType);

    this->cachedSpecializedInterfaceDeclRef = specialiedInterfaceDeclRef;
    return specialiedInterfaceDeclRef;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TaggedUnionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TaggedUnionType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("__TaggedUnion(");
    bool first = true;
    for (auto caseType : caseTypes)
    {
        if (!first)
        {
            out << toSlice(", ");
        }
        first = false;

        out << caseType;
    }
    out << toSlice(")");
}

bool TaggedUnionType::_equalsImplOverride(Type* type)
{
    auto taggedUnion = as<TaggedUnionType>(type);
    if (!taggedUnion)
        return false;

    auto caseCount = caseTypes.getCount();
    if (caseCount != taggedUnion->caseTypes.getCount())
        return false;

    for (Index ii = 0; ii < caseCount; ++ii)
    {
        if (!caseTypes[ii]->equals(taggedUnion->caseTypes[ii]))
            return false;
    }
    return true;
}

HashCode TaggedUnionType::_getHashCodeOverride()
{
    HashCode hashCode = 0;
    for (auto caseType : caseTypes)
    {
        hashCode = combineHash(hashCode, caseType->getHashCode());
    }
    return hashCode;
}

Type* TaggedUnionType::_createCanonicalTypeOverride()
{
    TaggedUnionType* canType = m_astBuilder->create<TaggedUnionType>();

    for (auto caseType : caseTypes)
    {
        auto canCaseType = caseType->getCanonicalType();
        canType->caseTypes.add(canCaseType);
    }

    return canType;
}

Val* TaggedUnionType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    List<Type*> substCaseTypes;
    for (auto caseType : caseTypes)
    {
        substCaseTypes.add(as<Type>(caseType->substituteImpl(astBuilder, subst, &diff)));
    }
    if (!diff)
        return this;

    (*ioDiff)++;

    TaggedUnionType* substType = astBuilder->create<TaggedUnionType>();
    substType->caseTypes.swapWith(substCaseTypes);
    return substType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExistentialSpecializedType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExistentialSpecializedType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("__ExistentialSpecializedType(") << baseType;
    for (auto arg : args)
    {
        out << toSlice(", ") << arg.val;
    }
    out << toSlice(")");
}

bool ExistentialSpecializedType::_equalsImplOverride(Type * type)
{
    auto other = as<ExistentialSpecializedType>(type);
    if (!other)
        return false;

    if (!baseType->equals(other->baseType))
        return false;

    auto argCount = args.getCount();
    if (argCount != other->args.getCount())
        return false;

    for (Index ii = 0; ii < argCount; ++ii)
    {
        auto arg = args[ii];
        auto otherArg = other->args[ii];

        if (!arg.val->equalsVal(otherArg.val))
            return false;

        if (!areValsEqual(arg.witness, otherArg.witness))
            return false;
    }
    return true;
}

HashCode ExistentialSpecializedType::_getHashCodeOverride()
{
    Hasher hasher;
    hasher.hashObject(baseType);
    for (auto arg : args)
    {
        hasher.hashObject(arg.val);
        if (auto witness = arg.witness)
            hasher.hashObject(witness);
    }
    return hasher.getResult();
}

static Val* _getCanonicalValue(Val* val)
{
    if (!val)
        return nullptr;
    if (auto type = as<Type>(val))
    {
        return type->getCanonicalType();
    }
    // TODO: We may eventually need/want some sort of canonicalization
    // for non-type values, but for now there is nothing to do.
    return val;
}

Type* ExistentialSpecializedType::_createCanonicalTypeOverride()
{
    ExistentialSpecializedType* canType = m_astBuilder->create<ExistentialSpecializedType>();

    canType->baseType = baseType->getCanonicalType();
    for (auto arg : args)
    {
        ExpandedSpecializationArg canArg;
        canArg.val = _getCanonicalValue(arg.val);
        canArg.witness = _getCanonicalValue(arg.witness);
        canType->args.add(canArg);
    }
    return canType;
}

static Val* _substituteImpl(ASTBuilder* astBuilder, Val* val, SubstitutionSet subst, int* ioDiff)
{
    if (!val) return nullptr;
    return val->substituteImpl(astBuilder, subst, ioDiff);
}

Val* ExistentialSpecializedType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substBaseType = as<Type>(baseType->substituteImpl(astBuilder, subst, &diff));

    ExpandedSpecializationArgs substArgs;
    for (auto arg : args)
    {
        ExpandedSpecializationArg substArg;
        substArg.val = _substituteImpl(astBuilder, arg.val, subst, &diff);
        substArg.witness = _substituteImpl(astBuilder, arg.witness, subst, &diff);
        substArgs.add(substArg);
    }

    if (!diff)
        return this;

    (*ioDiff)++;

    ExistentialSpecializedType* substType = astBuilder->create<ExistentialSpecializedType>();
    substType->baseType = substBaseType;
    substType->args = substArgs;
    return substType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ThisType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ThisType::_toTextOverride(StringBuilder& out)
{
    out << interfaceDeclRef << toSlice(".This");
}

bool ThisType::_equalsImplOverride(Type * type)
{
    auto other = as<ThisType>(type);
    if (!other)
        return false;

    if (!interfaceDeclRef.equals(other->interfaceDeclRef))
        return false;

    return true;
}

HashCode ThisType::_getHashCodeOverride()
{
    return combineHash(
        HashCode(typeid(*this).hash_code()),
        interfaceDeclRef.getHashCode());
}

Type* ThisType::_createCanonicalTypeOverride()
{
    ThisType* canType = m_astBuilder->create<ThisType>();

    // TODO: need to canonicalize the decl-ref
    canType->interfaceDeclRef = interfaceDeclRef;
    return canType;
}

Val* ThisType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substInterfaceDeclRef = interfaceDeclRef.substituteImpl(astBuilder, subst, &diff);

    auto thisTypeSubst = findThisTypeSubstitution(subst.substitutions, substInterfaceDeclRef.getDecl());
    if (thisTypeSubst)
    {
        return thisTypeSubst->witness->sub;
    }

    if (!diff)
        return this;

    (*ioDiff)++;

    ThisType* substType = m_astBuilder->create<ThisType>();
    substType->interfaceDeclRef = substInterfaceDeclRef;
    return substType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AndType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void AndType::_toTextOverride(StringBuilder& out)
{
    out << left << toSlice(" & ") << right;
}

bool AndType::_equalsImplOverride(Type * type)
{
    auto other = as<AndType>(type);
    if (!other)
        return false;

    if(!left->equals(other->left))
        return false;
    if(!right->equals(other->right))
        return false;

    return true;
}

HashCode AndType::_getHashCodeOverride()
{
    Hasher hasher;
    hasher.hashObject(left);
    hasher.hashObject(right);
    return hasher.getResult();
}

Type* AndType::_createCanonicalTypeOverride()
{
    AndType* canType = m_astBuilder->create<AndType>();

    // TODO: proper canonicalization of an `&` type relies on
    // several different things:
    //
    // * We need to re-associate types that might involve
    //   nesting of `&`, such as `(A & B) & (C & D)`, into
    //   a canonical form where the nesting is consistent
    //   (i.e., always left- or right-associative).
    //
    // * We need to commute types so that they are in a
    //   consistent order, so that `A & B` and `B & A` both
    //   result in the same canonicalization. This requirement
    //   implies that we must invent a total order on types.
    //
    // * We need to canonicalize `&` types where one of the
    //   elements might be implied by another. E.g., if we
    //   have `interface IDerived : IBase`, then a type like
    //   `IDerived & IBase` is equivalent to just `IDerived`
    //   because the presence of an `IBase` conformance is
    //   implied. A special case of the above is the possibility
    //   of duplicates in the list of types (e.g., `A & B & A`).
    //
    // * The previous requirement raises the problem that
    //   the relationships between `interface`s might either
    //   evolve over time, or be subject to `extension`
    //   declarations in other modules. The canonicalization
    //   algorithm must be clear about what information it
    //   is allowed to make use of, as this can/will affect
    //   binary interfaces (via mangled names).
    //
    // We are going to completely ignore these issues for
    // right now, in the name of getting something up and running.
    //
    canType->left = left->getCanonicalType();
    canType->right = right->getCanonicalType();

    return canType;
}

Val* AndType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substLeft  = as<Type>(left ->substituteImpl(astBuilder, subst, &diff));
    auto substRight = as<Type>(right->substituteImpl(astBuilder, subst, &diff));

    if(!diff)
        return this;

    (*ioDiff)++;

    AndType* substType = m_astBuilder->create<AndType>();
    substType->left = substLeft;
    substType->right = substRight;
    return substType;
}

// ModifiedType


void ModifiedType::_toTextOverride(StringBuilder& out)
{
    for( auto modifier : modifiers )
    {
        modifier->toText(out);
        out.appendChar(' ');
    }
    base->toText(out);
}

bool ModifiedType::_equalsImplOverride(Type* type)
{
    auto other = as<ModifiedType>(type);
    if(!other)
        return false;

    if(!base->equals(other->base))
        return false;

    // TODO: Eventually we need to put the `modifiers` into
    // a canonical ordering as part of creation of a `ModifiedType`,
    // so that two instances that apply the same modifiers to
    // the same type will have those modifiers in a matching order.
    //
    // The simplest way to achieve that ordering *for now* would
    // be to sort the array by the integer AST node type tag.
    // That approach would of course not scale to modifiers that
    // have any operands of their own.
    //
    // Note that we would *also* need the logic that creates a
    // `ModifiedType` to detect when the base type is itself a
    // `ModifiedType` and produce a single `ModifiedType` with
    // a combined list of modifiers and a non-`ModifiedType` as
    // its base type.
    //
    auto modifierCount = modifiers.getCount();
    if(modifierCount != other->modifiers.getCount())
        return false;

    for( Index i = 0; i < modifierCount; ++i )
    {
        auto thisModifier = this->modifiers[i];
        auto otherModifier = other->modifiers[i];
        if(!thisModifier->equalsVal(otherModifier))
            return false;
    }
    return true;
}

HashCode ModifiedType::_getHashCodeOverride()
{
    Hasher hasher;
    hasher.hashObject(base);
    for( auto modifier : modifiers )
    {
        hasher.hashObject(modifier);
    }
    return hasher.getResult();
}

Type* ModifiedType::_createCanonicalTypeOverride()
{
    ModifiedType* canonical = m_astBuilder->create<ModifiedType>();
    canonical->base = base->getCanonicalType();
    for( auto modifier : modifiers )
    {
        canonical->modifiers.add(modifier);
    }
    return canonical;
}

Val* ModifiedType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    Type* substBase = as<Type>(base->substituteImpl(astBuilder, subst, &diff));

    List<Val*> substModifiers;
    for( auto modifier : modifiers )
    {
        auto substModifier = modifier->substituteImpl(astBuilder, subst, &diff);
        substModifiers.add(substModifier);
    }

    if(!diff)
        return this;

    *ioDiff = 1;

    ModifiedType* substType = m_astBuilder->create<ModifiedType>();
    substType->base = substBase;
    substType->modifiers = _Move(substModifiers);
    return substType;
}


} // namespace Slang
