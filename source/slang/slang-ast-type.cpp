// slang-ast-type.cpp
#include "slang-ast-builder.h"
#include "slang-ast-modifier.h"
#include <assert.h>
#include <typeinfo>

#include "slang-syntax.h"

#include "slang-generated-ast-macro.h"
namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Type* Type::_createCanonicalTypeOverride()
{
    return as<Type>(defaultResolveImpl());
}

Val* Type::_substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    int diff = 0;
    auto canonicalType = getCanonicalType();

    // If canonicalType is identical to this, then we shouldn't try to call
    // canonicalType->substituteImpl because that would lead to infinite recursion.
    if (canonicalType == this)
        return this;

    auto canSubst = canonicalType->substituteImpl(astBuilder, subst, &diff);

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

Type* OverloadGroupType::_createCanonicalTypeOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! InitializerListType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void InitializerListType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("initializer list");
}

Type* InitializerListType::_createCanonicalTypeOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ErrorType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ErrorType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("error");
}

Type* ErrorType::_createCanonicalTypeOverride()
{
    return this;
}

Val* ErrorType::_substituteImplOverride(ASTBuilder* /* astBuilder */, SubstitutionSet /*subst*/, int* /*ioDiff*/)
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BottomType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void BottomType::_toTextOverride(StringBuilder& out) { out << toSlice("never"); }

Val* BottomType::_substituteImplOverride(
    ASTBuilder* /* astBuilder */, SubstitutionSet /*subst*/, int* /*ioDiff*/)
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DeclRefType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void DeclRefType::_toTextOverride(StringBuilder& out)
{
    out << getDeclRef();
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
        if (as<ThisTypeDecl>(substDeclRef.getDecl()))
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
    if (auto satisfyingVal = substDeclRef.declRefBase->resolve())
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

IntVal* MatrixExpressionType::getLayout()
{
    return as<IntVal>(_getGenericTypeArg(this, 3));
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

Type* TypeType::_createCanonicalTypeOverride()
{
    return getCurrentASTBuilder()->getTypeType(getType()->getCanonicalType());
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! GenericDeclRefType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void GenericDeclRefType::_toTextOverride(StringBuilder& out)
{
    // TODO: what is appropriate here?
    out << toSlice("<DeclRef<GenericDecl>>");
}

Type* GenericDeclRefType::_createCanonicalTypeOverride()
{
    return this;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NamespaceType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void NamespaceType::_toTextOverride(StringBuilder& out)
{
    out << toSlice("namespace ") << getDeclRef(); 
}

Type* NamespaceType::_createCanonicalTypeOverride()
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

Type* NativeRefType::getValueType()
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

Type* NamedExpressionType::_createCanonicalTypeOverride()
{
    auto canType = getType(getCurrentASTBuilder(), getDeclRef());
    if (canType)
        return canType->getCanonicalType();
    return getCurrentASTBuilder()->getErrorType();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FuncType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ParameterDirection FuncType::getParamDirection(Index index)
{
    auto paramType = getParamType(index);
    if (as<RefType>(paramType))
    {
        return kParameterDirection_Ref;
    }
    else if (as<ConstRefType>(paramType))
    {
        return kParameterDirection_ConstRef;
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

Type* FuncType::_createCanonicalTypeOverride()
{
    // result type
    Type* canResultType = getResultType()->getCanonicalType();
    Type* canErrorType = getErrorType()->getCanonicalType();

    // parameter types
    List<Type*> canParamTypes;
    for (Index pp = 0; pp < getParamCount(); pp++)
    {
        canParamTypes.add(getParamType(pp)->getCanonicalType());
    }

    FuncType* canType = getCurrentASTBuilder()->getFuncType(canParamTypes.getArrayView(), canResultType, canErrorType);
    return canType;
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

Type* TupleType::_createCanonicalTypeOverride()
{
    // member types
    List<Type*> canMemberTypes;
    for (Index m = 0; m < getMemberCount(); m++)
    {
        canMemberTypes.add(getMember(m)->getCanonicalType());
    }

    return getCurrentASTBuilder()->getTupleType(canMemberTypes);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ExtractExistentialType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void ExtractExistentialType::_toTextOverride(StringBuilder& out)
{
    out << getDeclRef() << toSlice(".This");
}

Type* ExtractExistentialType::_createCanonicalTypeOverride()
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
        getBaseType()->getCanonicalType(),
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

DeclRef<InterfaceDecl> ThisType::getInterfaceDeclRef()
{
    return DeclRef<Decl>(getDeclRefBase()->getParent()).template as<InterfaceDecl>();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AndType !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void AndType::_toTextOverride(StringBuilder& out)
{
    out << getLeft() << toSlice(" & ") << getRight();
}

Type* AndType::_createCanonicalTypeOverride()
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

    auto canLeft = getLeft()->getCanonicalType();
    auto canRight = getRight()->getCanonicalType();
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

    auto substType = astBuilder->getAndType(substLeft, substRight);
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

Type* ModifiedType::_createCanonicalTypeOverride()
{
    List<Val*> modifiers;
    for (Index i = 0; i < getModifierCount(); ++i)
    {
        auto modifier = this->getModifier(i);
        modifiers.add(modifier);
    }
    ModifiedType* canonical = getCurrentASTBuilder()->getOrCreate<ModifiedType>(getBase()->getCanonicalType(), modifiers.getArrayView());
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

SlangResourceShape ResourceType::getBaseShape()
{
    auto shape = _getGenericTypeArg(getDeclRefBase(), 1);
    if (as<TextureShape1DType>(shape))
        return SLANG_TEXTURE_1D;
    else if (as<TextureShape2DType>(shape))
        return SLANG_TEXTURE_2D;
    else if (as<TextureShape3DType>(shape))
        return SLANG_TEXTURE_3D;
    else if (as<TextureShapeCubeType>(shape))
        return SLANG_TEXTURE_CUBE;
    else if (as<TextureShapeBufferType>(shape))
        return SLANG_TEXTURE_BUFFER;

    return SLANG_RESOURCE_NONE;
}

SlangResourceShape ResourceType::getShape()
{
    auto baseShape = (SlangResourceShape)getBaseShape();
    if (isArray())
        baseShape = (SlangResourceShape)((uint32_t)baseShape | SLANG_TEXTURE_ARRAY_FLAG);
    if (isMultisample())
        baseShape = (SlangResourceShape)((uint32_t)baseShape | SLANG_TEXTURE_MULTISAMPLE_FLAG);
    if (isShadow())
        baseShape = (SlangResourceShape)((uint32_t)baseShape | SLANG_TEXTURE_SHADOW_FLAG);
    if (isFeedback())
        baseShape = (SlangResourceShape)((uint32_t)baseShape | SLANG_TEXTURE_FEEDBACK_FLAG);
    return baseShape;
}

bool ResourceType::isArray()
{
    auto isArray = _getGenericTypeArg(this, kStdlibTextureIsArrayParameterIndex);
    if (auto constIntVal = as<ConstantIntVal>(isArray))
        return constIntVal->getValue() != 0;
    return false;
}

bool ResourceType::isMultisample()
{
    auto isMS = _getGenericTypeArg(this, kStdlibTextureIsMultisampleParameterIndex);
    if (auto constIntVal = as<ConstantIntVal>(isMS))
        return constIntVal->getValue() != 0;
    return false;
}

bool ResourceType::isShadow()
{
    auto isShadow = _getGenericTypeArg(this, kStdlibTextureIsShadowParameterIndex);
    if (auto constIntVal = as<ConstantIntVal>(isShadow))
        return constIntVal->getValue() != 0;
    return false;
}

bool ResourceType::isFeedback()
{
    auto access = _getGenericTypeArg(this, kStdlibTextureAccessParameterIndex);
    if (auto constIntVal = as<ConstantIntVal>(access))
        return constIntVal->getValue() == kStdlibResourceAccessFeedback;
    return false;
}

bool ResourceType::isCombined()
{
    auto combined = _getGenericTypeArg(this, kStdlibTextureIsCombinedParameterIndex);
    if (auto constIntVal = as<ConstantIntVal>(combined))
        return constIntVal->getValue() != 0;
    return false;
}

SlangResourceAccess ResourceType::getAccess()
{
    auto access = _getGenericTypeArg(this, kStdlibTextureAccessParameterIndex);
    if (auto constIntVal = as<ConstantIntVal>(access))
    {
        switch (constIntVal->getValue())
        {
        case kStdlibResourceAccessReadOnly:
            return SLANG_RESOURCE_ACCESS_READ;
        case kStdlibResourceAccessReadWrite:
            return SLANG_RESOURCE_ACCESS_READ_WRITE;
        case kStdlibResourceAccessRasterizerOrdered:
            return SLANG_RESOURCE_ACCESS_RASTER_ORDERED;
        case kStdlibResourceAccessFeedback:
            return SLANG_RESOURCE_ACCESS_FEEDBACK;
        default:
            break;
        }
    }
    return SLANG_RESOURCE_ACCESS_NONE;
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

void ResourceType::_toTextOverride(StringBuilder& out)
{
    auto tryPrintSimpleName = [&](String& outString) -> bool
        {
            StringBuilder resultSB;
            auto access = getAccess();
            switch (access)
            {
            case SLANG_RESOURCE_ACCESS_READ:
                break;
            case SLANG_RESOURCE_ACCESS_READ_WRITE:
                resultSB << "RW";;
                break;
            case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                resultSB << "RasterizerOrdered";
                break;
            case SLANG_RESOURCE_ACCESS_FEEDBACK:
                resultSB << "Feedback";
                break;
            default:
                return false;
            }
            auto combined = as<ConstantIntVal>(_getGenericTypeArg(this, 7));
            auto shapeVal = _getGenericTypeArg(this, 1);
            if (!as<TextureShapeType>(shapeVal))
                return false;
            auto shape = getBaseShape();
            if (!combined)
                return false;
            if (combined->getValue() != 0)
                resultSB << "Sampler";
            else
            {
                if (shape == SLANG_TEXTURE_BUFFER)
                    resultSB << "Buffer";
                else
                    resultSB << "Texture";
            }
            switch (shape)
            {
            case SLANG_TEXTURE_1D:
                resultSB << "1D";
                break;
            case SLANG_TEXTURE_2D:
                resultSB << "2D";
                break;
            case SLANG_TEXTURE_3D:
                resultSB << "3D";
                break;
            case SLANG_TEXTURE_CUBE:
                resultSB << "Cube";
                break;
            }
            auto isArrayVal = as<ConstantIntVal>(_getGenericTypeArg(this, 2));
            if (!isArrayVal)
                return false;
            if (isArray())
                resultSB << "Array";
            auto isMultisampleVal = as<ConstantIntVal>(_getGenericTypeArg(this, 3));
            if (!isMultisampleVal)
                return false;
            if (isMultisample())
                resultSB << "MS";
            auto isShadowVal = as<ConstantIntVal>(_getGenericTypeArg(this, 6));
            if (!isShadowVal)
                return false;
            if (isShadow())
                return false;
            auto elementType = getElementType();
            if (elementType)
            {
                resultSB << "<";
                resultSB << elementType->toString();
                auto sampleCount = _getGenericTypeArg(this, 4);
                if (auto constIntVal = as<ConstantIntVal>(sampleCount))
                {
                    if (constIntVal->getValue() != 0)
                        resultSB << ", " << constIntVal->getValue();
                }
                else
                {
                    return false;
                }
                resultSB << ">";
            }
            outString = resultSB.toString();
            return true;
        };

    String simpleName;

    if (tryPrintSimpleName(simpleName))
        out << simpleName;
    else
        DeclRefType::_toTextOverride(out);
}

Val* TextureTypeBase::getSampleCount()
{
    return as<Type>(_getGenericTypeArg(this, 4));
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

bool isNonCopyableType(Type* type)
{
    auto declRefType = as<DeclRefType>(type);
    if (!declRefType)
        return false;
    if (declRefType->getDeclRef().getDecl()->findModifier<NonCopyableTypeAttribute>())
        return true;
    return false;
}

} // namespace Slang
