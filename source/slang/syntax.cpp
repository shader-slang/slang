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

    Type* BasicExpressionType::CreateCanonicalType()
    {
        // A basic type is already canonical, in our setup
        return this;
    }

    Slang::String BasicExpressionType::ToString()
    {
        Slang::StringBuilder res;

        switch (this->baseType)
        {
        case Slang::BaseType::Int:
            res.Append("int");
            break;
        case Slang::BaseType::UInt:
            res.Append("uint");
            break;
        case Slang::BaseType::UInt64:
            res.Append("uint64_t");
            break;
        case Slang::BaseType::Bool:
            res.Append("bool");
            break;
        case Slang::BaseType::Float:
            res.Append("float");
            break;
        case Slang::BaseType::Void:
            res.Append("void");
            break;
        default:
            break;
        }
        return res.ProduceString();
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

    RefPtr<Val> Type::SubstituteImpl(Substitutions* subst, int* ioDiff)
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
        if (!this) return nullptr;
        Type* et = const_cast<Type*>(this);
        if (!et->canonicalType)
        {
            // TODO(tfoley): worry about thread safety here?
            et->canonicalType = et->CreateCanonicalType();
            if (dynamic_cast<Type*>(et->canonicalType) != this)
                et->canonicalTypeRefPtr = et->canonicalType;
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

        irBasicBlockType = new IRBasicBlockType();
        irBasicBlockType->setSession(this);
    }

    Type* Session::getBoolType()
    {
        return getBuiltinType(BaseType::Bool);
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

    Type* Session::getUIntType()
    {
        return getBuiltinType(BaseType::UInt);
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

    Type* Session::getIRBasicBlockType()
    {
        return irBasicBlockType;
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

        return DeclRefType::Create(
            this,
            declRef)->As<PtrTypeBase>();
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


    RefPtr<GroupSharedType> Session::getGroupSharedType(RefPtr<Type> valueType)
    {
        RefPtr<GroupSharedType> groupSharedType = new GroupSharedType();
        groupSharedType->setSession(this);
        groupSharedType->valueType = valueType;
        return groupSharedType;
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
        return (ArrayLength == arrType->ArrayLength && baseType->Equals(arrType->baseType.Ptr()));
    }

    Type* ArrayExpressionType::CreateCanonicalType()
    {
        auto canonicalElementType = baseType->GetCanonicalType();
        auto canonicalArrayType = getArrayType(
            canonicalElementType,
            ArrayLength);
        session->canonicalTypes.Add(canonicalArrayType);
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

    // GroupSharedType

    Slang::String GroupSharedType::ToString()
    {
        return "@ThreadGroup " + valueType->ToString();
    }

    bool GroupSharedType::EqualsImpl(Type * type)
    {
        auto t = type->As<GroupSharedType>();
        if (!t)
            return false;
        return valueType->Equals(t->valueType);
    }

    Type* GroupSharedType::CreateCanonicalType()
    {
        auto canonicalValueType = valueType->GetCanonicalType();
        auto canonicalGroupSharedType = getSession()->getGroupSharedType(canonicalValueType);
        session->canonicalTypes.Add(canonicalGroupSharedType);
        return canonicalGroupSharedType;
    }

    int GroupSharedType::GetHashCode()
    {
        return combineHash(
            valueType->GetHashCode(),
            (int)(typeid(this).hash_code()));
    }

    // DeclRefType

    String DeclRefType::ToString()
    {
        return getText(declRef.GetName());
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

    Type* DeclRefType::CreateCanonicalType()
    {
        // A declaration reference is already canonical
        return this;
    }

    RefPtr<Val> DeclRefType::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        if (!subst) return this;

        // the case we especially care about is when this type references a declaration
        // of a generic parameter, since that is what we might be substituting...
        if (auto genericTypeParamDecl = dynamic_cast<GenericTypeParamDecl*>(declRef.getDecl()))
        {
            // search for a substitution that might apply to us
            for (auto s = subst; s; s = s->outer.Ptr())
            {
                if (auto genericSubst = dynamic_cast<GenericSubstitution*>(s))
                {
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
        }
        // the second case we care about is when this decl type refers to an associatedtype decl
        // we want to replace it with the actual associated type
        else if (auto assocTypeDecl = dynamic_cast<AssocTypeDecl*>(declRef.getDecl()))
        {
            auto thisSubst = getThisTypeSubst(declRef, false);
            auto oldSubstSrc = thisSubst ? thisSubst->sourceType : nullptr;
            bool restore = false;
            if (thisSubst && thisSubst->sourceType.Ptr() == dynamic_cast<Val*>(this))
                thisSubst->sourceType = nullptr;
            auto newSubst = substituteSubstitutions(declRef.substitutions, subst, ioDiff);
            if (restore)
                thisSubst->sourceType = oldSubstSrc;
            if (auto thisTypeSubst = newSubst.As<ThisTypeSubstitution>())
            {
                if (thisTypeSubst->sourceType)
                {
                    if (auto aggTypeDeclRef = thisTypeSubst->sourceType.As<DeclRefType>()->declRef.As<AggTypeDecl>())
                    {
                        Decl * targetType = nullptr;
                        if (aggTypeDeclRef.getDecl()->memberDictionary.TryGetValue(assocTypeDecl->getName(), targetType))
                        {
                            if (auto typeDefDecl = dynamic_cast<TypeDefDecl*>(targetType))
                            {
                                DeclRef<TypeDefDecl> targetTypeDeclRef(typeDefDecl, aggTypeDeclRef.substitutions);
                                return GetType(targetTypeDeclRef);
                            }
                            else if (auto targetAggType = dynamic_cast<AggTypeDecl*>(targetType))
                            {
                                return DeclRefType::Create(getSession(), DeclRef<Decl>(targetAggType, aggTypeDeclRef.substitutions));
                            }
                            else
                            {
                                SLANG_UNIMPLEMENTED_X("unknown assoctype implementation type.");
                            }
                        }
                    }
                }
            }
        }
        else if (auto globalGenParam = dynamic_cast<GlobalGenericParamDecl*>(declRef.getDecl()))
        {
            // search for a substitution that might apply to us
            for (auto s = subst; s; s = s->outer.Ptr())
            {
                if (auto genericSubst = dynamic_cast<GlobalGenericParamSubstitution*>(s))
                {
                    if (genericSubst->paramDecl == globalGenParam)
                    {
                        return genericSubst->actualType;
                    }
                }
            }
        }
        int diff = 0;
        DeclRef<Decl> substDeclRef = declRef.SubstituteImpl(subst, &diff);

        if (!diff)
            return this;

        // Make sure to record the difference!
        *ioDiff += diff;

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

    // TODO: need to figure out how to unify this with the logic
    // in the generic case...
    DeclRefType* DeclRefType::Create(
        Session*        session,
        DeclRef<Decl>   declRef)
    {
        // It is possible that `declRef` refers to a generic type,
        // but does not specify arguments for its generic parameters.
        // (E.g., this happens when referring to a generic type from
        // within its own member functions). To handle this case,
        // we will construct a default specialization at the use
        // site if needed.

        if (auto genericParent = declRef.GetParent().As<GenericDecl>())
        {
            auto subst = declRef.substitutions;
            // try find a substitution targeting this generic decl
            bool substFound = false;
            while (subst)
            {
                if (auto genSubst = dynamic_cast<GenericSubstitution*>(subst.Ptr()))
                {
                    if (genSubst->genericDecl == genericParent.decl)
                    {
                        substFound = true;
                        break;
                    }
                }
                subst = subst->outer;
            }
            // we did not find an existing substituion, create a default one
            if (!substFound)
            {
                declRef.substitutions = createDefaultSubstitutions(
                    session,
                    declRef.decl,
                    subst);
            }
        }

        if (auto builtinMod = declRef.getDecl()->FindModifier<BuiltinTypeModifier>())
        {
            auto type = new BasicExpressionType(builtinMod->tag);
            type->setSession(session);
            type->declRef = declRef;
            return type;
        }
        else if (auto magicMod = declRef.getDecl()->FindModifier<MagicTypeModifier>())
        {
            GenericSubstitution* subst = declRef.substitutions.As<GenericSubstitution>().Ptr();

            if (magicMod->name == "SamplerState")
            {
                auto type = new SamplerStateType();
                type->setSession(session);
                type->declRef = declRef;
                type->flavor = SamplerStateType::Flavor(magicMod->tag);
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
                    TextureType::Flavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "TextureSampler")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new TextureSamplerType(
                    TextureType::Flavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->setSession(session);
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "GLSLImageType")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new GLSLImageType(
                    TextureType::Flavor(magicMod->tag),
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
            CASE(HLSLAppendStructuredBufferType, HLSLAppendStructuredBufferType)
            CASE(HLSLConsumeStructuredBufferType, HLSLConsumeStructuredBufferType)

            CASE(HLSLPointStreamType, HLSLPointStreamType)
            CASE(HLSLLineStreamType, HLSLPointStreamType)
            CASE(HLSLTriangleStreamType, HLSLPointStreamType)

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

    Type* OverloadGroupType::CreateCanonicalType()
    {
        return this;
    }

    int OverloadGroupType::GetHashCode()
    {
        return (int)(int64_t)(void*)this;
    }

    // IRBasicBlockType

    String IRBasicBlockType::ToString()
    {
        return "Block";
    }

    bool IRBasicBlockType::EqualsImpl(Type * /*type*/)
    {
        return false;
    }

    Type* IRBasicBlockType::CreateCanonicalType()
    {
        return this;
    }

    int IRBasicBlockType::GetHashCode()
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

    Type* InitializerListType::CreateCanonicalType()
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

    Type* ErrorType::CreateCanonicalType()
    {
        return  this;
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

    Type* NamedExpressionType::CreateCanonicalType()
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

    RefPtr<Val> FuncType::SubstituteImpl(Substitutions* subst, int* ioDiff)
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

    Type* FuncType::CreateCanonicalType()
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

        session->canonicalTypes.Add(canType);

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

    Type* TypeType::CreateCanonicalType()
    {
        auto canType = getTypeType(type->GetCanonicalType());
        session->canonicalTypes.Add(canType);
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

    Type* GenericDeclRefType::CreateCanonicalType()
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
        return this->declRef.substitutions.As<GenericSubstitution>()->args[0].As<Type>().Ptr();
    }

    IntVal* MatrixExpressionType::getRowCount()
    {
        return this->declRef.substitutions.As<GenericSubstitution>()->args[1].As<IntVal>().Ptr();
    }

    IntVal* MatrixExpressionType::getColumnCount()
    {
        return this->declRef.substitutions.As<GenericSubstitution>()->args[2].As<IntVal>().Ptr();
    }

    // PtrTypeBase

    Type* PtrTypeBase::getValueType()
    {
        return this->declRef.substitutions.As<GenericSubstitution>()->args[0].As<Type>().Ptr();
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

    RefPtr<Val> GenericParamIntVal::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        // search for a substitution that might apply to us
        for (auto s = subst; s; s = s->outer.Ptr())
        {
            if (auto genSubst = dynamic_cast<GenericSubstitution*>(s))
            {
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
        }

        // Nothing found: don't substittue.
        return this;
    }

    // Substitutions

    RefPtr<Substitutions> GenericSubstitution::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        if (!this) return nullptr;

        int diff = 0;
        auto outerSubst = outer ? outer->SubstituteImpl(subst, &diff) : nullptr;

        List<RefPtr<Val>> substArgs;
        for (auto a : args)
        {
            substArgs.Add(a->SubstituteImpl(subst, &diff));
        }

        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new GenericSubstitution();
        substSubst->genericDecl = genericDecl;
        substSubst->args = substArgs;
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
            return !subst->outer || subst->outer.As<ThisTypeSubstitution>();

        if (!outer->Equals(subst->outer.Ptr()))
            return false;

        return true;
    }

    RefPtr<Substitutions> ThisTypeSubstitution::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        if (!this) return nullptr;

        int diff = 0;
        RefPtr<Substitutions> outerSubst = outer ? outer->SubstituteImpl(subst, &diff) : nullptr;
        RefPtr<Val> newSourceType;
        if (sourceType)
            newSourceType = sourceType->SubstituteImpl(subst, &diff);
        else
        {
            // this_type is a free variable, use this_type from subst
            auto psubst = subst;
            while (psubst)
            {
                if (auto pthisSubst = dynamic_cast<ThisTypeSubstitution*>(subst))
                {
                    diff++;
                    newSourceType = pthisSubst->sourceType;
                    break;
                }
                psubst = psubst->outer;
            }
        }
        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new ThisTypeSubstitution();
        substSubst->sourceType = newSourceType;
        substSubst->outer = outerSubst;
        return substSubst;
    }

    bool ThisTypeSubstitution::Equals(Substitutions* subst)
    {
        if (!subst)
            return true;
        if (subst && dynamic_cast<ThisTypeSubstitution*>(subst))
            return true;
        return false;
    }

    RefPtr<Substitutions> GlobalGenericParamSubstitution::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        // if we find a GlobalGenericParamSubstitution in subst that references the same __generic_param decl
        // return a copy of that GlobalGenericParamSubstitution
        int diff = 0;
        RefPtr<Substitutions> outerSubst = outer ? outer->SubstituteImpl(subst, &diff) : nullptr;
        while (subst)
        {
            if (auto gSubst = dynamic_cast<GlobalGenericParamSubstitution*>(subst))
            {
                if (gSubst->paramDecl == paramDecl)
                {
                    // substitute only if we are really different
                    if (!gSubst->actualType->EqualsVal(actualType))
                    {
                        RefPtr<GlobalGenericParamSubstitution> rs = new GlobalGenericParamSubstitution(*gSubst);
                        rs->outer = outerSubst;
                        return rs;
                    }
                }
            }
            subst = subst->outer;
        }
        if (diff)
        {
            *ioDiff++;
            RefPtr<GlobalGenericParamSubstitution> rs = new GlobalGenericParamSubstitution(*this);
            rs->outer = outerSubst;
            return rs;
        }
        return this;
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
            if (witnessTables.Count() != genSubst->witnessTables.Count())
                return false;
            for (UInt i = 0; i < witnessTables.Count(); i++)
            {
                if (!witnessTables[i].Key->Equals(genSubst->witnessTables[i].Key))
                    return false;
                if (!witnessTables[i].Value->EqualsVal(genSubst->witnessTables[i].Value))
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

        return type->Substitute(substitutions.Ptr()).As<Type>();
    }

    DeclRefBase DeclRefBase::Substitute(DeclRefBase declRef) const
    {
        if(!substitutions)
            return declRef;

        int diff = 0;
        return declRef.SubstituteImpl(substitutions.Ptr(), &diff);
    }

    RefPtr<Expr> DeclRefBase::Substitute(RefPtr<Expr> expr) const
    {
        // No substitutions? Easy.
        if (!substitutions)
            return expr;

        SLANG_UNIMPLEMENTED_X("generic substitution into expressions");

        UNREACHABLE_RETURN(expr);
    }

    bool hasGlobalGenericSubst(Substitutions * destSubst, GlobalGenericParamSubstitution * genSubst)
    {
        while (destSubst)
        {
            if (auto globalParamSubst = dynamic_cast<GlobalGenericParamSubstitution*>(destSubst))
            {
                if (globalParamSubst->paramDecl == genSubst->paramDecl)
                    return true;
            }
            destSubst = destSubst->outer;
        }
        return false;
    }
    void insertGlobalGenericSubstitutions(RefPtr<Substitutions> & destSubst, Substitutions * srcSubst, int * ioDiff)
    {
        int diff = 0;
        while (srcSubst)
        {
            if (auto globalGenSubst = dynamic_cast<GlobalGenericParamSubstitution*>(srcSubst))
            {
                if (!hasGlobalGenericSubst(destSubst, globalGenSubst))
                {
                    RefPtr<GlobalGenericParamSubstitution> cpyGlobalGenSubst = new GlobalGenericParamSubstitution(*globalGenSubst);
                    cpyGlobalGenSubst->outer = nullptr;
                    insertSubstAtBottom(destSubst, cpyGlobalGenSubst);
                    diff = 1;
                }
            }
            srcSubst = srcSubst->outer;
        }
        *ioDiff += diff;
    }

    DeclRefBase DeclRefBase::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        int diff = 0;
        RefPtr<Substitutions> substSubst = substituteSubstitutions(substitutions, subst, &diff);

        if (!diff)
            return *this;

        *ioDiff += diff;

        DeclRefBase substDeclRef;
        substDeclRef.decl = decl;
        substDeclRef.substitutions = substSubst;
        return substDeclRef;
    }


    // Check if this is an equivalent declaration reference to another
    bool DeclRefBase::Equals(DeclRefBase const& declRef) const
    {
        if (decl != declRef.decl)
            return false;
        if (!substitutions)
            return !declRef.substitutions || declRef.substitutions.As<ThisTypeSubstitution>();
        if (!substitutions->Equals(declRef.substitutions.Ptr()))
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

        if (auto parentGeneric = dynamic_cast<GenericDecl*>(parentDecl))
        {
            auto genSubst = substitutions.As<GenericSubstitution>();
            if (genSubst && genSubst->genericDecl == parentDecl)
            {
                // We strip away the specializations that were applied to
                // the parent, since we were asked for a reference *to* the parent.
                return DeclRefBase(parentGeneric, substitutions->outer);
            }
            else
            {
                // Either we don't have specializations, or the inner-most
                // specializations didn't apply to the parent decl. This
                // can happen if we are looking at an unspecialized
                // declaration that is a child of a generic.
                return DeclRefBase(parentGeneric, substitutions);
            }
        }
        else
        {
            // If the parent isn't a generic, then it must
            // use the same specializations as this declaration
            return DeclRefBase(parentDecl, substitutions);
        }

    }

    int DeclRefBase::GetHashCode() const
    {
        auto rs = PointerHash<1>::GetHashCode(decl);
        if (substitutions)
        {
            rs *= 16777619;
            rs ^= substitutions->GetHashCode();
        }
        return rs;
    }

    // Val

    RefPtr<Val> Val::Substitute(Substitutions* subst)
    {
        if (!this) return nullptr;
        if (!subst) return this;
        int diff = 0;
        return SubstituteImpl(subst, &diff);
    }

    RefPtr<Val> Val::SubstituteImpl(Substitutions* /*subst*/, int* /*ioDiff*/)
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
        return this->declRef.substitutions.As<GenericSubstitution>()->args[0].As<Type>().Ptr();
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return this->declRef.substitutions.As<GenericSubstitution>()->args[1].As<IntVal>().Ptr();
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
        auto namedType = new NamedExpressionType(declRef);
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
        for (auto pp : GetParameters(declRef))
        {
            funcType->paramTypes.Add(GetType(pp));
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

    bool DeclaredSubtypeWitness::EqualsVal(Val* val)
    {
        auto otherWitness = dynamic_cast<DeclaredSubtypeWitness*>(val);
        if(!otherWitness)
            return false;

        return sub->Equals(otherWitness->sub)
            && sup->Equals(otherWitness->sup)
            && declRef.Equals(otherWitness->declRef);
    }

    RefPtr<Val> DeclaredSubtypeWitness::SubstituteImpl(Substitutions* subst, int * ioDiff)
    {
        if (auto genConstraintDecl = declRef.As<GenericTypeConstraintDecl>())
        {
            // search for a substitution that might apply to us
            for (auto s = subst; s; s = s->outer.Ptr())
            {
                if (auto genericSubst = dynamic_cast<GenericSubstitution*>(s))
                {
                    // the generic decl associated with the substitution list must be
                    // the generic decl that declared this parameter
                    auto genericDecl = genericSubst->genericDecl;
                    if (genericDecl != genConstraintDecl.getDecl()->ParentDecl)
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
                else if (auto globalGenParamSubst = dynamic_cast<GlobalGenericParamSubstitution*>(s))
                {
                    // we have a GlobalGenericParamSubstitution, this substitution will provide
                    // a concrete IRWitnessTable for a generic global variable
                    auto supType = GetSup(genConstraintDecl);

                    // check if the substitution is really about this global generic type parameter
                    if (globalGenParamSubst->paramDecl != genConstraintDecl.getDecl()->ParentDecl)
                        continue;

                    // find witness table for the required interface
                    for (auto witness : globalGenParamSubst->witnessTables)
                        if (witness.Key->EqualsVal(supType))
                        {
                            (*ioDiff)++;
                            return witness.Value;
                        }
                }
            }
        }
        RefPtr<DeclaredSubtypeWitness> rs = new DeclaredSubtypeWitness();
        rs->sub = sub->SubstituteImpl(subst, ioDiff).As<Type>();
        rs->sup = sup->SubstituteImpl(subst, ioDiff).As<Type>();
        rs->declRef = declRef.SubstituteImpl(subst, ioDiff);
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
        auto hash = sub->GetHashCode();
        hash = combineHash(hash, sup->GetHashCode());
        hash = combineHash(hash, declRef.GetHashCode());
        return hash;
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
            && midToSup->EqualsVal(otherWitness->midToSup);
    }

    RefPtr<Val> TransitiveSubtypeWitness::SubstituteImpl(Substitutions* subst, int * ioDiff)
    {
        int diff = 0;

        RefPtr<Type> substSub = sub->SubstituteImpl(subst, &diff).As<Type>();
        RefPtr<Type> substSup = sup->SubstituteImpl(subst, &diff).As<Type>();
        RefPtr<SubtypeWitness> substSubToMid = subToMid->SubstituteImpl(subst, &diff).As<SubtypeWitness>();
        RefPtr<SubtypeWitness> substMidToSup = midToSup->SubstituteImpl(subst, &diff).As<SubtypeWitness>();

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
        sb << this->midToSup->ToString();
        sb << ")";
        return sb.ProduceString();
    }

    int TransitiveSubtypeWitness::GetHashCode()
    {
        auto hash = sub->GetHashCode();
        hash = combineHash(hash, sup->GetHashCode());
        hash = combineHash(hash, subToMid->GetHashCode());
        hash = combineHash(hash, midToSup->GetHashCode());
        return hash;
    }

    // IRProxyVal

    bool IRProxyVal::EqualsVal(Val* val)
    {
        auto otherProxy = dynamic_cast<IRProxyVal*>(val);
        if(!otherProxy)
            return false;

        return this->inst.usedValue == otherProxy->inst.usedValue;
    }

    String IRProxyVal::ToString()
    {
        return "IRProxyVal(...)";
    }

    int IRProxyVal::GetHashCode()
    {
        auto hash = Slang::GetHashCode(inst.usedValue);
        return hash;
    }

    //

    String DeclRefBase::toString() const
    {
        StringBuilder sb;
        sb << this->getDecl()->getName()->text;
        // TODO: need to print out substitutions too!
        return sb.ProduceString();
    }

    void insertSubstAtBottom(RefPtr<Substitutions> & substHead, RefPtr<Substitutions> substToInsert)
    {
        if (!substHead)
        {
            substHead = substToInsert;
            return;
        }
        auto subst = substHead;
        RefPtr<Substitutions> lastSubst = subst;
        while (subst->outer)
        {
            lastSubst = subst;
            subst = subst->outer;
        }
        lastSubst->outer = substToInsert;
    }

    void insertSubstAtTop(DeclRefBase & declRef, RefPtr<Substitutions> substToInsert)
    {
        substToInsert->outer = declRef.substitutions;
        declRef.substitutions = substToInsert;
    }

    RefPtr<ThisTypeSubstitution> getThisTypeSubst(DeclRefBase & declRef, bool insertSubstEntry)
    {
        RefPtr<ThisTypeSubstitution> thisSubst;
        auto subst = declRef.substitutions;
        while (subst)
        {
            if (auto s = subst.As<ThisTypeSubstitution>())
            {
                thisSubst = s;
                break;
            }
            subst = subst->outer;
        }
        if (!thisSubst)
        {
            thisSubst = new ThisTypeSubstitution();
            if (insertSubstEntry)
            {
                insertSubstAtTop(declRef, thisSubst);
            }
        }
        return thisSubst;
    }

    RefPtr<ThisTypeSubstitution> getNewThisTypeSubst(DeclRefBase & declRef)
    {
        auto oldSubst = getThisTypeSubst(declRef, false);
        if (oldSubst)
            removeSubstitution(declRef, oldSubst);
        return getThisTypeSubst(declRef, true);
    }

    void removeSubstitution(DeclRefBase & declRef, RefPtr<Substitutions> toRemove)
    {
        if (!declRef.substitutions)
            return;
        if (toRemove == declRef.substitutions)
        {
            declRef.substitutions = declRef.substitutions->outer;
            return;
        }
        auto prev = declRef.substitutions;
        auto subst = prev->outer;
        while (subst)
        {
            if (subst == toRemove)
            {
                prev->outer = subst->outer;
                break;
            }
            prev = subst;
            subst = subst->outer;
        }
    }

    bool hasGenericSubstitutions(RefPtr<Substitutions> subst)
    {
        auto p = subst.Ptr();
        while (p)
        {
            if (dynamic_cast<GenericSubstitution*>(p))
                return true;
            p = p->outer.Ptr();
        }
        return false;
    }

    RefPtr<GenericSubstitution> getGenericSubstitution(RefPtr<Substitutions> subst)
    {
        auto p = subst.Ptr();
        while (p)
        {
            if (auto genSubst = dynamic_cast<GenericSubstitution*>(p))
                return genSubst;
            p = p->outer.Ptr();
        }
        return nullptr;
    }

    RefPtr<Substitutions> substituteSubstitutions(RefPtr<Substitutions> oldSubst, Substitutions * subst, int * ioDiff)
    {
        if (oldSubst)
            oldSubst = oldSubst->SubstituteImpl(subst, ioDiff);

        RefPtr<Substitutions> newSubst = oldSubst;
        insertGlobalGenericSubstitutions(newSubst, subst, ioDiff);
        return newSubst;
    }
}
