// slang-ast-type.cpp
#include "slang-ast-builder.h"
#include "slang-ast-modifier.h"
#include <assert.h>
#include <typeinfo>

#include "slang-syntax.h"

#include "slang-generated-ast-macro.h"
namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool Type::equals(Type* type)
{
    return getCanonicalType(nullptr)->equalsImpl(type->getCanonicalType(nullptr));
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

Type* Type::_createCanonicalTypeOverride(SemanticsVisitor*)
{
    return as<Type>(defaultResolveImpl());
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
    auto canSubst = getCanonicalType(nullptr)->substituteImpl(astBuilder, subst, &diff);

    // If nothing changed, then don't drop any sugar that is applied
    if (!diff)
        return this;

    // If the canonical type changed, then we return a canonical type,
    // rather than try to re-construct any amount of sugar
    (*ioDiff)++;
    return canSubst;
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

Type* OverloadGroupType::_createCanonicalTypeOverride(SemanticsVisitor*)
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

Type* InitializerListType::_createCanonicalTypeOverride(SemanticsVisitor*)
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
    return as<ErrorType>(type);
}

Type* ErrorType::_createCanonicalTypeOverride(SemanticsVisitor*)
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
    return as<BottomType>(type);
}

Val* BottomType::_substituteImplOverride(
    ASTBuilder* /* astBuilder */, SubstitutionSet /*subst*/, int* /*ioDiff*/)
{
    return this;
}

HashCode BottomType::_getHashCodeOverride() { return HashCode(size_t(this)); }

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclRefType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void DeclRefType::_toTextOverride(StringBuilder& out)
{
    out << getDeclRef();
}

HashCode DeclRefType::_getHashCodeOverride()
{
    return (getDeclRef().getHashCode() * 16777619) ^ (HashCode)(typeid(this).hash_code());
}

bool DeclRefType::_equalsImplOverride(Type * type)
{
    if (auto declRefType = as<DeclRefType>(type))
    {
        return getDeclRef().equals(declRefType->getDeclRef());
    }
    return false;
}

Val* maybeSubstituteGenericParam(Val* paramVal, Decl* paramDecl, SubstitutionSet subst, int* ioDiff);

Val* DeclRefType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    if (!subst) return this;

    int diff = 0;
    DeclRef<Decl> substDeclRef = getDeclRef().substituteImpl(astBuilder, subst, &diff);

    // If this declref type is a direct reference to ThisType or a Generic parameter,
    // and `subst` provides an argument for it, then we should just return that argument.
    //
    if (as<DirectDeclRef>(substDeclRef.declRefBase))
    {
        if (auto thisDecl = as<ThisTypeDecl>(substDeclRef.getDecl()))
        {
            auto lookupDeclRef = subst.findLookupDeclRef();
            if (lookupDeclRef && lookupDeclRef->getSupDecl() == substDeclRef.getDecl()->parentDecl)
            {
                (*ioDiff)++;
                return lookupDeclRef->getLookupSource();
            }
        }
        else if (as<GenericTypeParamDecl>(substDeclRef.getDecl()) || as<GenericValueParamDecl>(substDeclRef.getDecl()))
        {
            auto resultVal = maybeSubstituteGenericParam(nullptr, substDeclRef.getDecl(), subst, ioDiff);
            if (resultVal)
            {
                (*ioDiff)++;
                return resultVal;
            }
        }
    }

    // If this type is a reference to an associated type declaration,
    // and the substitutions provide a "this type" substitution for
    // the outer interface, then try to replace the type with the
    // actual value of the associated type for the given implementation.
    //
    if (auto satisfyingVal = substDeclRef.declRefBase->resolve(nullptr))
    {
        if (satisfyingVal != getDeclRef())
        {
            *ioDiff += 1;
            return DeclRefType::create(astBuilder, substDeclRef);
        }
    }

    if (!diff)
        return this;

    // Make sure to record the difference!
    *ioDiff += diff;

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

BasicExpressionType* BasicExpressionType::_getScalarTypeOverride()
{
    return this;
}

static Val* _getGenericTypeArg(DeclRefBase* declRef, Index i)
{
    auto args = findInnerMostGenericArgs(SubstitutionSet(declRef));
    if (args.getCount() <= i)
        return nullptr;

    return args[i];
}

static Val* _getGenericTypeArg(DeclRefType* declRefType, Index i)
{
    return _getGenericTypeArg(declRefType->getDeclRefBase(), i);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TensorViewType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Type* TensorViewType::getElementType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VectorExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Type* VectorExpressionType::getElementType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}

IntVal* VectorExpressionType::getElementCount()
{
    return as<IntVal>(_getGenericTypeArg(this, 1));
}

void VectorExpressionType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("vector<") << getElementType() << toSlice(",") << getElementCount() << toSlice(">");
}

BasicExpressionType* VectorExpressionType::_getScalarTypeOverride()
{
    return as<BasicExpressionType>(getElementType());
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
    return as<Type>(_getGenericTypeArg(this, 0));
}

IntVal* MatrixExpressionType::getRowCount()
{
    return as<IntVal>(_getGenericTypeArg(this, 1));
}

IntVal* MatrixExpressionType::getColumnCount()
{
    return as<IntVal>(_getGenericTypeArg(this, 2));
}

Type* MatrixExpressionType::getRowType()
{
    if (!rowType)
    {
        rowType = getCurrentASTBuilder()->getVectorType(getElementType(), getColumnCount());
    }
    return rowType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArrayExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Type* ArrayExpressionType::getElementType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}

IntVal* ArrayExpressionType::getElementCount()
{
    return as<IntVal>(_getGenericTypeArg(this, 1));
}

void ArrayExpressionType::_toTextOverride(StringBuilder& out)
{
    out << getElementType();
    out.appendChar('[');
    if (!isUnsized())
    {
        out << getElementCount();
    }
    out.appendChar(']');
}

bool ArrayExpressionType::isUnsized()
{
    if (auto constSize = as<ConstantIntVal>(getElementCount()))
    {
        if (constSize->getValue() == kUnsizedArrayMagicLength)
            return true;
    }
    return false;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TypeType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TypeType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("typeof(") << getType() << toSlice(")");
}

bool TypeType::_equalsImplOverride(Type * t)
{
    if (auto typeType = as<TypeType>(t))
    {
        return t->equals(typeType->getType());
    }
    return false;
}

Type* TypeType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    return getCurrentASTBuilder()->getTypeType(getType()->getCanonicalType(semantics));
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
        return getDeclRef().equals(genericDeclRefType->getDeclRef());
    }
    return false;
}

HashCode GenericDeclRefType::_getHashCodeOverride()
{
    return getDeclRef().getHashCode();
}

Type* GenericDeclRefType::_createCanonicalTypeOverride(SemanticsVisitor*)
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NamespaceType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NamespaceType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("namespace ") << getDeclRef(); 
}

bool NamespaceType::_equalsImplOverride(Type * type)
{
    if (auto namespaceType = as<NamespaceType>(type))
    {
        return getDeclRef().equals(namespaceType->getDeclRef());
    }
    return false;
}

HashCode NamespaceType::_getHashCodeOverride()
{
    return getDeclRef().getHashCode();
}

Type* NamespaceType::_createCanonicalTypeOverride(SemanticsVisitor*)
{
    return this;
}

Type* DifferentialPairType::getPrimalType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PtrTypeBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Type* PtrTypeBase::getValueType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}

Type* OptionalType::getValueType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NamedExpressionType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NamedExpressionType::_toTextOverride(StringBuilder& out)
{
    if (getDeclRef().getDecl())
    {
        getDeclRef().declRefBase->toText(out);
    }
}

bool NamedExpressionType::_equalsImplOverride(Type * /*type*/)
{
    SLANG_UNEXPECTED("NamedExpressionType::_equalsImplOverride should be unreachable");
    //return false;
}

Type* NamedExpressionType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    auto canType = getType(getCurrentASTBuilder(), getDeclRef());
    if (canType)
        return canType->getCanonicalType(semantics);
    return getCurrentASTBuilder()->getErrorType();
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
    return getCanonicalType(nullptr)->getHashCode();
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
    Index paramCount = getParamCount();
    out << toSlice("(");
    for (Index pp = 0; pp < paramCount; ++pp)
    {
        if (pp != 0)
        {
            out << toSlice(", ");
        }
        out << getParamType(pp);
    }
    out << ") -> " << getResultType();

    if (!getErrorType()->equals(getCurrentASTBuilder()->getBottomType()))
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

        for (Index pp = 0; pp < paramCount; ++pp)
        {
            auto paramType = getParamType(pp);
            auto otherParamType = funcType->getParamType(pp);
            if (!paramType->equals(otherParamType))
                return false;
        }

        if (!getResultType()->equals(funcType->getResultType()))
            return false;

        if (!getErrorType()->equals(funcType->getErrorType()))
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
    Type* substResultType = as<Type>(getResultType()->substituteImpl(astBuilder, subst, &diff));

    // error type
    Type* substErrorType = as<Type>(getErrorType()->substituteImpl(astBuilder, subst, &diff));

    // parameter types
    List<Type*> substParamTypes;
    for (Index pp = 0; pp < getParamCount(); pp++ )
    {
        substParamTypes.add(as<Type>(getParamType(pp)->substituteImpl(astBuilder, subst, &diff)));
    }

    // early exit for no change...
    if (!diff)
        return this;

    (*ioDiff)++;
    FuncType* substType = astBuilder->getFuncType(substParamTypes.getArrayView(), substResultType, substErrorType);
    return substType;
}

Type* FuncType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    // result type
    Type* canResultType = getResultType()->getCanonicalType(semantics);
    Type* canErrorType = getErrorType()->getCanonicalType(semantics);

    // parameter types
    List<Type*> canParamTypes;
    for (Index pp = 0; pp < getParamCount(); pp++)
    {
        canParamTypes.add(getParamType(pp)->getCanonicalType(semantics));
    }

    FuncType* canType = getCurrentASTBuilder()->getFuncType(canParamTypes.getArrayView(), canResultType, canErrorType);
    return canType;
}

HashCode FuncType::_getHashCodeOverride()
{
    HashCode hashCode = getResultType()->getHashCode();
    Index paramCount = getParamCount();
    hashCode = combineHash(hashCode, Slang::getHashCode(paramCount));
    for (Index pp = 0; pp < paramCount; ++pp)
    {
        hashCode = combineHash(
            hashCode,
            getParamType(pp)->getHashCode());
    }
    combineHash(hashCode, getErrorType()->getHashCode());
    return hashCode;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TupleType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void TupleType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("(");
    for (Index pp = 0; pp < getOperandCount(); ++pp)
    {
        if (pp != 0)
            out << toSlice(", ");
        out << getOperand(pp);
    }
    out << toSlice(")");
}

bool TupleType::_equalsImplOverride(Type * type)
{
    if (const auto other = as<TupleType>(type))
    {
        auto paramCount = getOperandCount();
        auto otherParamCount = other->getOperandCount();
        if (paramCount != otherParamCount)
            return false;

        for (Index i = 0; i < getOperandCount(); ++i)
        {
            if(!as<Type>(getOperand(i))->equals(as<Type>(other->getOperand(i))))
                return false;
        }

        return true;
    }
    return false;
}

Val* TupleType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    // just recurse into the members
    List<Type*> substMemberTypes;
    for (Index m = 0; m < getMemberCount(); m++)
        substMemberTypes.add(as<Type>(getMember(m)->substituteImpl(astBuilder, subst, &diff)));

    // early exit for no change...
    if (!diff)
        return this;

    (*ioDiff)++;
    return astBuilder->getTupleType(substMemberTypes);
}

Type* TupleType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    // member types
    List<Type*> canMemberTypes;
    for (Index m = 0; m < getMemberCount(); m++)
    {
        canMemberTypes.add(getMember(m)->getCanonicalType(semantics));
    }

    return getCurrentASTBuilder()->getTupleType(canMemberTypes);
}

HashCode TupleType::_getHashCodeOverride()
{
    HashCode hashCode = Slang::getHashCode(kType);
    for (Index m = 0; m < getMemberCount(); m++)
        hashCode = combineHash(hashCode, getMember(m)->getHashCode());
    return hashCode;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExtractExistentialType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExtractExistentialType::_toTextOverride(StringBuilder& out)
{
    out << getDeclRef() << toSlice(".This");
}

bool ExtractExistentialType::_equalsImplOverride(Type* type)
{
    if (auto extractExistential = as<ExtractExistentialType>(type))
    {
        return getDeclRef().equals(extractExistential->getDeclRef());
    }
    return false;
}

HashCode ExtractExistentialType::_getHashCodeOverride()
{
    return combineHash(getDeclRef().getHashCode(), getOriginalInterfaceType()->getHashCode(), getOriginalInterfaceDeclRef().getHashCode());
}

Type* ExtractExistentialType::_createCanonicalTypeOverride(SemanticsVisitor*)
{
    return this;
}

Val* ExtractExistentialType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto substDeclRef = getDeclRef().substituteImpl(astBuilder, subst, &diff);
    auto substOriginalInterfaceType = getOriginalInterfaceType()->substituteImpl(astBuilder, subst, &diff);
    auto substOriginalInterfaceDeclRef = getOriginalInterfaceDeclRef().substituteImpl(astBuilder, subst, &diff);
    if (!diff)
        return this;

    (*ioDiff)++;

    ExtractExistentialType* substValue = astBuilder->getOrCreate<ExtractExistentialType>(
        substDeclRef, as<Type>(substOriginalInterfaceType), substOriginalInterfaceDeclRef);
    return substValue;
}

SubtypeWitness* ExtractExistentialType::getSubtypeWitness()
{
    if (auto cachedValue = this->cachedSubtypeWitness)
        return cachedValue;

    ExtractExistentialSubtypeWitness* openedWitness = getCurrentASTBuilder()->getOrCreate<ExtractExistentialSubtypeWitness>(this, getOriginalInterfaceType(), getDeclRef());
    this->cachedSubtypeWitness = openedWitness;
    return openedWitness;
}

DeclRef<ThisTypeDecl> ExtractExistentialType::getThisTypeDeclRef()
{
    if (auto cachedValue = this->cachedThisTypeDeclRef)
        return cachedValue;

    auto interfaceDecl = getOriginalInterfaceDeclRef().getDecl();

    SubtypeWitness* openedWitness = getSubtypeWitness();

    ThisTypeDecl* thisTypeDecl = nullptr;
    for (auto member : interfaceDecl->members)
        if (as<ThisTypeDecl>(member))
        {
            thisTypeDecl = as<ThisTypeDecl>(member);
            break;
        }
    SLANG_ASSERT(thisTypeDecl);

    DeclRef<ThisTypeDecl> specialiedInterfaceDeclRef = getCurrentASTBuilder()->getLookupDeclRef(openedWitness, thisTypeDecl);

    this->cachedThisTypeDeclRef = specialiedInterfaceDeclRef;
    return specialiedInterfaceDeclRef;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExistentialSpecializedType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExistentialSpecializedType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("__ExistentialSpecializedType(") << getBaseType();
    for (Index i = 0; i < getArgCount(); i++)
    {
        out << toSlice(", ") << getArg(i).val;
    }
    out << toSlice(")");
}

bool ExistentialSpecializedType::_equalsImplOverride(Type * type)
{
    auto other = as<ExistentialSpecializedType>(type);
    if (!other)
        return false;

    if (!getBaseType()->equals(other->getBaseType()))
        return false;

    auto argCount = getArgCount();
    if (argCount != other->getArgCount())
        return false;

    for (Index ii = 0; ii < argCount; ++ii)
    {
        auto arg = getArg(ii);
        auto otherArg = other->getArg(ii);

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
    hasher.hashObject(getBaseType());
    for (Index ii = 0; ii < getArgCount(); ++ii)
    {
        auto arg = getArg(ii);
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
        return type->getCanonicalType(nullptr);
    }
    // TODO: We may eventually need/want some sort of canonicalization
    // for non-type values, but for now there is nothing to do.
    return val;
}

Type* ExistentialSpecializedType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    ExpandedSpecializationArgs newArgs;

    for (Index ii = 0; ii < getArgCount(); ++ii)
    {
        auto arg = getArg(ii);
        ExpandedSpecializationArg canArg;
        canArg.val = _getCanonicalValue(arg.val);
        canArg.witness = _getCanonicalValue(arg.witness);
        newArgs.add(canArg);
    }

    ExistentialSpecializedType* canType = getCurrentASTBuilder()->getOrCreate<ExistentialSpecializedType>(
        getBaseType()->getCanonicalType(semantics),
        newArgs);

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

    auto substBaseType = as<Type>(getBaseType()->substituteImpl(astBuilder, subst, &diff));

    ExpandedSpecializationArgs substArgs;
    for (Index ii = 0; ii < getArgCount(); ++ii)
    {
        auto arg = getArg(ii);
        ExpandedSpecializationArg substArg;
        substArg.val = _substituteImpl(astBuilder, arg.val, subst, &diff);
        substArg.witness = _substituteImpl(astBuilder, arg.witness, subst, &diff);
        substArgs.add(substArg);
    }

    if (!diff)
        return this;

    (*ioDiff)++;

    ExistentialSpecializedType* substType = astBuilder->getOrCreate<ExistentialSpecializedType>(substBaseType, substArgs);
    return substType;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ThisType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

InterfaceDecl* ThisType::getInterfaceDecl()
{
    return dynamicCast<InterfaceDecl>(getDeclRefBase()->getDecl()->parentDecl);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AndType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void AndType::_toTextOverride(StringBuilder& out)
{
    out << getLeft() << toSlice(" & ") << getRight();
}

bool AndType::_equalsImplOverride(Type * type)
{
    auto other = as<AndType>(type);
    if (!other)
        return false;

    if(!getLeft()->equals(other->getLeft()))
        return false;
    if(!getRight()->equals(other->getRight()))
        return false;

    return true;
}

HashCode AndType::_getHashCodeOverride()
{
    Hasher hasher;
    hasher.hashObject(getLeft());
    hasher.hashObject(getRight());
    return hasher.getResult();
}

Type* AndType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
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

    auto canLeft = getLeft()->getCanonicalType(semantics);
    auto canRight = getRight()->getCanonicalType(semantics);
    auto canType = getCurrentASTBuilder()->getAndType(canLeft, canRight);
    return canType;
}

Val* AndType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;

    auto substLeft  = as<Type>(getLeft()->substituteImpl(astBuilder, subst, &diff));
    auto substRight = as<Type>(getRight()->substituteImpl(astBuilder, subst, &diff));

    if(!diff)
        return this;

    (*ioDiff)++;

    auto substType = getCurrentASTBuilder()->getAndType(substLeft, substRight);
    return substType;
}

// ModifiedType

void ModifiedType::_toTextOverride(StringBuilder& out)
{
    for( Index i = 0; i < getModifierCount(); i++ )
    {
        getModifier(i)->toText(out);
        out.appendChar(' ');
    }
    getBase()->toText(out);
}

bool ModifiedType::_equalsImplOverride(Type* type)
{
    auto other = as<ModifiedType>(type);
    if(!other)
        return false;

    if(!getBase()->equals(other->getBase()))
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
    auto modifierCount = getModifierCount();
    if(modifierCount != other->getModifierCount())
        return false;

    for (Index i = 0; i < modifierCount; ++i)
    {
        auto thisModifier = this->getModifier(i);
        auto otherModifier = other->getModifier(i);
        if(!thisModifier->equalsVal(otherModifier))
            return false;
    }
    return true;
}

HashCode ModifiedType::_getHashCodeOverride()
{
    Hasher hasher;
    hasher.hashObject(getBase());
    for (Index i = 0; i < getModifierCount(); ++i)
    {
        auto modifier = this->getModifier(i);
        hasher.hashObject(modifier);
    }
    return hasher.getResult();
}

Type* ModifiedType::_createCanonicalTypeOverride(SemanticsVisitor* semantics)
{
    List<Val*> modifiers;
    for (Index i = 0; i < getModifierCount(); ++i)
    {
        auto modifier = this->getModifier(i);
        modifiers.add(modifier);
    }
    ModifiedType* canonical = getCurrentASTBuilder()->getOrCreate<ModifiedType>(getBase()->getCanonicalType(semantics), modifiers.getArrayView());
    return canonical;
}

Val* ModifiedType::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    Type* substBase = as<Type>(getBase()->substituteImpl(astBuilder, subst, &diff));

    List<Val*> substModifiers;
    for (Index i = 0; i < getModifierCount(); ++i)
    {
        auto modifier = this->getModifier(i);
        auto substModifier = modifier->substituteImpl(astBuilder, subst, &diff);
        substModifiers.add(substModifier);
    }

    if(!diff)
        return this;

    *ioDiff = 1;

    ModifiedType* substType = getCurrentASTBuilder()->getOrCreate<ModifiedType>(substBase, substModifiers.getArrayView());
    return substType;
}

BaseType BasicExpressionType::getBaseType() const
{
    auto builtinType = getDeclRef().getDecl()->findModifier<BuiltinTypeModifier>();
    return builtinType->tag;
}

FeedbackType::Kind FeedbackType::getKind() const
{
    auto magicMod = getDeclRef().getDecl()->findModifier<MagicTypeModifier>();
    return FeedbackType::Kind(magicMod->tag);
}

TextureFlavor ResourceType::getFlavor() const
{
    auto magicMod = getDeclRef().getDecl()->findModifier<MagicTypeModifier>();
    return TextureFlavor(magicMod->tag);
}

SamplerStateFlavor SamplerStateType::getFlavor() const
{
    auto magicMod = getDeclRef().getDecl()->findModifier<MagicTypeModifier>();
    return SamplerStateFlavor(magicMod->tag);
}

Type* BuiltinGenericType::getElementType() const
{
    return as<Type>(_getGenericTypeArg(getDeclRefBase(), 0));
}

Type* ResourceType::getElementType()
{
    return as<Type>(_getGenericTypeArg(this, 0));
}

Val* TextureTypeBase::getSampleCount()
{
    return as<Type>(_getGenericTypeArg(this, 1));
}

Type* removeParamDirType(Type* type)
{
    for (auto paramDirType = as<ParamDirectionType>(type); paramDirType;)
    {
        type = paramDirType->getValueType();
        paramDirType = as<ParamDirectionType>(type);
    }
    return type;
}

} // namespace Slang
