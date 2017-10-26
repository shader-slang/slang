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
        auto genericDecl = findMagicDecl(
            this, "PtrType").As<GenericDecl>();
        auto typeDecl = genericDecl->inner;
               
        auto substitutions = new Substitutions();
        substitutions->genericDecl = genericDecl.Ptr();
        substitutions->args.Add(valueType);

        auto declRef = DeclRef<Decl>(typeDecl.Ptr(), substitutions);

        return DeclRefType::Create(
            this,
            declRef)->As<PtrType>();
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
                // the generic decl associated with the substitution list must be
                // the generic decl that declared this parameter
                auto genericDecl = s->genericDecl;
                if (genericDecl != genericTypeParamDecl->ParentDecl)
                    continue;

                int index = 0;
                for (auto m : genericDecl->Members)
                {
                    if (m.Ptr() == genericTypeParamDecl)
                    {
                        // We've found it, so return the corresponding specialization argument
                        (*ioDiff)++;
                        return s->args[index];
                    }
                    else if(auto typeParam = m.As<GenericTypeParamDecl>())
                    {
                        index++;
                    }
                    else if(auto valParam = m.As<GenericValueParamDecl>())
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

        if( auto genericParent = declRef.GetParent().As<GenericDecl>() )
        {
            auto subst = declRef.substitutions;
            if( !subst || subst->genericDecl != genericParent.decl )
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
            Substitutions* subst = declRef.substitutions.Ptr();

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
            CASE(GLSLInputParameterBlockType, GLSLInputParameterBlockType)
            CASE(GLSLOutputParameterBlockType, GLSLOutputParameterBlockType)
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
        return false;
    }

    Type* NamedExpressionType::CreateCanonicalType()
    {
        return GetType(declRef)->GetCanonicalType();
    }

    int NamedExpressionType::GetHashCode()
    {
        SLANG_UNEXPECTED("unreachable");
        return 0;
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
        return 0;
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
        return this->declRef.substitutions->args[0].As<Type>().Ptr();
    }

    IntVal* MatrixExpressionType::getRowCount()
    {
        return this->declRef.substitutions->args[1].As<IntVal>().Ptr();
    }

    IntVal* MatrixExpressionType::getColumnCount()
    {
        return this->declRef.substitutions->args[2].As<IntVal>().Ptr();
    }

    // PtrTypeBase

    Type* PtrTypeBase::getValueType()
    {
        return this->declRef.substitutions->args[0].As<Type>().Ptr();
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
            // the generic decl associated with the substitution list must be
            // the generic decl that declared this parameter
            auto genericDecl = s->genericDecl;
            if (genericDecl != declRef.getDecl()->ParentDecl)
                continue;

            int index = 0;
            for (auto m : genericDecl->Members)
            {
                if (m.Ptr() == declRef.getDecl())
                {
                    // We've found it, so return the corresponding specialization argument
                    (*ioDiff)++;
                    return s->args[index];
                }
                else if(auto typeParam = m.As<GenericTypeParamDecl>())
                {
                    index++;
                }
                else if(auto valParam = m.As<GenericValueParamDecl>())
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

    RefPtr<Substitutions> Substitutions::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        if (!this) return nullptr;

        int diff = 0;
        auto outerSubst = outer->SubstituteImpl(subst, &diff);

        List<RefPtr<Val>> substArgs;
        for (auto a : args)
        {
            substArgs.Add(a->SubstituteImpl(subst, &diff));
        }

        if (!diff) return this;

        (*ioDiff)++;
        auto substSubst = new Substitutions();
        substSubst->genericDecl = genericDecl;
        substSubst->args = substArgs;
        return substSubst;
    }

    bool Substitutions::Equals(Substitutions* subst)
    {
        // both must be NULL, or non-NULL
        if (!this || !subst)
            return !this && !subst;

        if (genericDecl != subst->genericDecl)
            return false;

        UInt argCount = args.Count();
        SLANG_RELEASE_ASSERT(args.Count() == subst->args.Count());
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            if (!args[aa]->EqualsVal(subst->args[aa].Ptr()))
                return false;
        }

        if (!outer->Equals(subst->outer.Ptr()))
            return false;

        return true;
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

        return expr;
    }


    DeclRefBase DeclRefBase::SubstituteImpl(Substitutions* subst, int* ioDiff)
    {
        if (!substitutions) return *this;

        int diff = 0;
        RefPtr<Substitutions> substSubst = substitutions->SubstituteImpl(subst, &diff);

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
            if (substitutions && substitutions->genericDecl == parentDecl)
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
            return nullptr;
        }
    }

    //

    // HLSLPatchType

    Type* HLSLPatchType::getElementType()
    {
        return this->declRef.substitutions->args[0].As<Type>().Ptr();
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return this->declRef.substitutions->args[1].As<IntVal>().Ptr();
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

    // IRProxyVal

    bool IRProxyVal::EqualsVal(Val* val)
    {
        auto otherProxy = dynamic_cast<IRProxyVal*>(val);
        if(!otherProxy)
            return false;

        return this->inst == otherProxy->inst;
    }

    String IRProxyVal::ToString()
    {
        return "IRProxyVal(...)";
    }

    int IRProxyVal::GetHashCode()
    {
        auto hash = Slang::GetHashCode(inst);
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



}
