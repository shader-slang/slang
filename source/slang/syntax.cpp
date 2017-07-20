#include "syntax.h"

#include "visitor.h"

#include <typeinfo>
#include <assert.h>

namespace Slang
{
    // BasicExpressionType

    bool BasicExpressionType::EqualsImpl(ExpressionType * type)
    {
        auto basicType = dynamic_cast<const BasicExpressionType*>(type);
        if (basicType == nullptr)
            return false;
        return basicType->BaseType == BaseType;
    }

    ExpressionType* BasicExpressionType::CreateCanonicalType()
    {
        // A basic type is already canonical, in our setup
        return this;
    }

    Slang::String BasicExpressionType::ToString()
    {
        Slang::StringBuilder res;

        switch (BaseType)
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

#define ABSTRACT_SYNTAX_CLASS(NAME, BASE) /* empty */
#define SYNTAX_CLASS(NAME, BASE)                                \
    void NAME::accept(NAME::Visitor* visitor, void* extra)      \
    { visitor->dispatch_##NAME(this, extra); }                  \
    void* SyntaxClassBase::Impl<NAME>::createFunc() { return new NAME(); }
#include "expr-defs.h"
#include "decl-defs.h"
#include "modifier-defs.h"
#include "stmt-defs.h"
#include "type-defs.h"
#include "val-defs.h"

#include "object-meta-end.h"

void ExpressionType::accept(IValVisitor* visitor, void* extra)
{
    accept((ITypeVisitor*)visitor, extra);
}

    // TypeExp

    bool TypeExp::Equals(ExpressionType* other)
    {
        return type->Equals(other);
    }

    bool TypeExp::Equals(RefPtr<ExpressionType> other)
    {
        return type->Equals(other.Ptr());
    }

    // BasicExpressionType

    BasicExpressionType* BasicExpressionType::GetScalarType()
    {
        return this;
    }

    //

    bool ExpressionType::Equals(ExpressionType * type)
    {
        return GetCanonicalType()->EqualsImpl(type->GetCanonicalType());
    }

    bool ExpressionType::Equals(RefPtr<ExpressionType> type)
    {
        return Equals(type.Ptr());
    }

    bool ExpressionType::EqualsVal(Val* val)
    {
        if (auto type = dynamic_cast<ExpressionType*>(val))
            return const_cast<ExpressionType*>(this)->Equals(type);
        return false;
    }

    NamedExpressionType* ExpressionType::AsNamedType()
    {
        return dynamic_cast<NamedExpressionType*>(this);
    }

    RefPtr<Val> ExpressionType::SubstituteImpl(Substitutions* subst, int* ioDiff)
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


    ExpressionType* ExpressionType::GetCanonicalType()
    {
        if (!this) return nullptr;
        ExpressionType* et = const_cast<ExpressionType*>(this);
        if (!et->canonicalType)
        {
            // TODO(tfoley): worry about thread safety here?
            et->canonicalType = et->CreateCanonicalType();
            SLANG_ASSERT(et->canonicalType);
        }
        return et->canonicalType;
    }

    bool ExpressionType::IsTextureOrSampler()
    {
        return IsTexture() || IsSampler();
    }
    bool ExpressionType::IsStruct()
    {
        auto declRefType = AsDeclRefType();
        if (!declRefType) return false;
        auto structDeclRef = declRefType->declRef.As<StructSyntaxNode>();
        if (!structDeclRef) return false;
        return true;
    }

#if 0
    RefPtr<ExpressionType> ExpressionType::Bool;
    RefPtr<ExpressionType> ExpressionType::UInt;
    RefPtr<ExpressionType> ExpressionType::Int;
    RefPtr<ExpressionType> ExpressionType::Float;
    RefPtr<ExpressionType> ExpressionType::Float2;
    RefPtr<ExpressionType> ExpressionType::Void;
#endif
    RefPtr<ExpressionType> ExpressionType::Error;
    RefPtr<ExpressionType> ExpressionType::initializerListType;
    RefPtr<ExpressionType> ExpressionType::Overloaded;

    Dictionary<int, RefPtr<ExpressionType>> ExpressionType::sBuiltinTypes;
    Dictionary<String, Decl*> ExpressionType::sMagicDecls;
    List<RefPtr<ExpressionType>> ExpressionType::sCanonicalTypes;

    void ExpressionType::Init()
    {
        Error = new ErrorType();
        initializerListType = new InitializerListType();
        Overloaded = new OverloadGroupType();
    }
    void ExpressionType::Finalize()
    {
        Error = nullptr;
        initializerListType = nullptr;
        Overloaded = nullptr;
        // Note(tfoley): This seems to be just about the only way to clear out a List<T>
        sCanonicalTypes = List<RefPtr<ExpressionType>>();
		sBuiltinTypes = Dictionary<int, RefPtr<ExpressionType>>();
		sMagicDecls = Dictionary<String, Decl*>();
    }
    bool ArrayExpressionType::EqualsImpl(ExpressionType * type)
    {
        auto arrType = type->AsArrayType();
        if (!arrType)
            return false;
        return (ArrayLength == arrType->ArrayLength && BaseType->Equals(arrType->BaseType.Ptr()));
    }
    ExpressionType* ArrayExpressionType::CreateCanonicalType()
    {
        auto canonicalBaseType = BaseType->GetCanonicalType();
        auto canonicalArrayType = new ArrayExpressionType();
        sCanonicalTypes.Add(canonicalArrayType);
        canonicalArrayType->BaseType = canonicalBaseType;
        canonicalArrayType->ArrayLength = ArrayLength;
        return canonicalArrayType;
    }
    int ArrayExpressionType::GetHashCode()
    {
        if (ArrayLength)
            return (BaseType->GetHashCode() * 16777619) ^ ArrayLength->GetHashCode();
        else
            return BaseType->GetHashCode();
    }
    Slang::String ArrayExpressionType::ToString()
    {
        if (ArrayLength)
            return BaseType->ToString() + "[" + ArrayLength->ToString() + "]";
        else
            return BaseType->ToString() + "[]";
    }

    // DeclRefType

    String DeclRefType::ToString()
    {
        return declRef.GetName();
    }

    int DeclRefType::GetHashCode()
    {
        return (declRef.GetHashCode() * 16777619) ^ (int)(typeid(this).hash_code());
    }

    bool DeclRefType::EqualsImpl(ExpressionType * type)
    {
        if (auto declRefType = type->AsDeclRefType())
        {
            return declRef.Equals(declRefType->declRef);
        }
        return false;
    }

    ExpressionType* DeclRefType::CreateCanonicalType()
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
        return DeclRefType::Create(substDeclRef);
    }

    static RefPtr<ExpressionType> ExtractGenericArgType(RefPtr<Val> val)
    {
        auto type = val.As<ExpressionType>();
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
    DeclRefType* DeclRefType::Create(DeclRef<Decl> declRef)
    {
        if (auto builtinMod = declRef.getDecl()->FindModifier<BuiltinTypeModifier>())
        {
            auto type = new BasicExpressionType(builtinMod->tag);
            type->declRef = declRef;
            return type;
        }
        else if (auto magicMod = declRef.getDecl()->FindModifier<MagicTypeModifier>())
        {
            Substitutions* subst = declRef.substitutions.Ptr();

            if (magicMod->name == "SamplerState")
            {
                auto type = new SamplerStateType();
                type->declRef = declRef;
                type->flavor = SamplerStateType::Flavor(magicMod->tag);
                return type;
            }
            else if (magicMod->name == "Vector")
            {
                SLANG_ASSERT(subst && subst->args.Count() == 2);
                auto vecType = new VectorExpressionType();
                vecType->declRef = declRef;
                vecType->elementType = ExtractGenericArgType(subst->args[0]);
                vecType->elementCount = ExtractGenericArgInteger(subst->args[1]);
                return vecType;
            }
            else if (magicMod->name == "Matrix")
            {
                SLANG_ASSERT(subst && subst->args.Count() == 3);
                auto matType = new MatrixExpressionType();
                matType->declRef = declRef;
                return matType;
            }
            else if (magicMod->name == "Texture")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new TextureType(
                    TextureType::Flavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "TextureSampler")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new TextureSamplerType(
                    TextureType::Flavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->name == "GLSLImageType")
            {
                SLANG_ASSERT(subst && subst->args.Count() >= 1);
                auto textureType = new GLSLImageType(
                    TextureType::Flavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->declRef = declRef;
                return textureType;
            }

            // TODO: eventually everything should follow this pattern,
            // and we can drive the dispatch with a table instead
            // of this ridiculously slow `if` cascade.

        #define CASE(n,T)													\
            else if(magicMod->name == #n) {									\
                auto type = new T();										\
                type->declRef = declRef;									\
                return type;												\
            }

            CASE(HLSLInputPatchType, HLSLInputPatchType)
            CASE(HLSLOutputPatchType, HLSLOutputPatchType)

        #undef CASE

            #define CASE(n,T)													\
                else if(magicMod->name == #n) {									\
                    SLANG_ASSERT(subst && subst->args.Count() == 1);			\
                    auto type = new T();										\
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
                    auto type = new T();										\
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
                SLANG_UNEXPECTED("unhandled type");
            }
        }
        else
        {
            return new DeclRefType(declRef);
        }
    }

    // OverloadGroupType

    String OverloadGroupType::ToString()
    {
        return "overload group";
    }

    bool OverloadGroupType::EqualsImpl(ExpressionType * /*type*/)
    {
        return false;
    }

    ExpressionType* OverloadGroupType::CreateCanonicalType()
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

    bool InitializerListType::EqualsImpl(ExpressionType * /*type*/)
    {
        return false;
    }

    ExpressionType* InitializerListType::CreateCanonicalType()
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

    bool ErrorType::EqualsImpl(ExpressionType* type)
    {
        if (auto errorType = type->As<ErrorType>())
            return true;
        return false;
    }

    ExpressionType* ErrorType::CreateCanonicalType()
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
        return declRef.GetName();
    }

    bool NamedExpressionType::EqualsImpl(ExpressionType * /*type*/)
    {
        SLANG_UNEXPECTED("unreachable");
        return false;
    }

    ExpressionType* NamedExpressionType::CreateCanonicalType()
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
        // TODO: a better approach than this
        if (declRef)
            return declRef.GetName();
        else
            return "/* unknown FuncType */";
    }

    bool FuncType::EqualsImpl(ExpressionType * type)
    {
        if (auto funcType = type->As<FuncType>())
        {
            return declRef == funcType->declRef;
        }
        return false;
    }

    ExpressionType* FuncType::CreateCanonicalType()
    {
        return this;
    }

    int FuncType::GetHashCode()
    {
        return declRef.GetHashCode();
    }

    // TypeType

    String TypeType::ToString()
    {
        StringBuilder sb;
        sb << "typeof(" << type->ToString() << ")";
        return sb.ProduceString();
    }

    bool TypeType::EqualsImpl(ExpressionType * t)
    {
        if (auto typeType = t->As<TypeType>())
        {
            return t->Equals(typeType->type);
        }
        return false;
    }

    ExpressionType* TypeType::CreateCanonicalType()
    {
        auto canType = new TypeType(type->GetCanonicalType());
        sCanonicalTypes.Add(canType);
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

    bool GenericDeclRefType::EqualsImpl(ExpressionType * type)
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

    ExpressionType* GenericDeclRefType::CreateCanonicalType()
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

    ExpressionType* MatrixExpressionType::getElementType()
    {
        return this->declRef.substitutions->args[0].As<ExpressionType>().Ptr();
    }

    IntVal* MatrixExpressionType::getRowCount()
    {
        return this->declRef.substitutions->args[1].As<IntVal>().Ptr();
    }

    IntVal* MatrixExpressionType::getColumnCount()
    {
        return this->declRef.substitutions->args[2].As<IntVal>().Ptr();
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
        return declRef.GetName();
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

    RefPtr<ExpressionType> DeclRefBase::Substitute(RefPtr<ExpressionType> type) const
    {
        // No substitutions? Easy.
        if (!substitutions)
            return type;

        // Otherwise we need to recurse on the type structure
        // and apply substitutions where it makes sense

        return type->Substitute(substitutions.Ptr()).As<ExpressionType>();
    }

    DeclRefBase DeclRefBase::Substitute(DeclRefBase declRef) const
    {
        if(!substitutions)
            return declRef;

        int diff = 0;
        return declRef.SubstituteImpl(substitutions.Ptr(), &diff);
    }

    RefPtr<ExpressionSyntaxNode> DeclRefBase::Substitute(RefPtr<ExpressionSyntaxNode> expr) const
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
    String const& DeclRefBase::GetName() const
    {
        return decl->Name.Content;
    }

    DeclRefBase DeclRefBase::GetParent() const
    {
        auto parentDecl = decl->ParentDecl;
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

    void RegisterBuiltinDecl(
        RefPtr<Decl>                decl,
        RefPtr<BuiltinTypeModifier> modifier)
    {
        auto type = DeclRefType::Create(DeclRef<Decl>(decl.Ptr(), nullptr));
        ExpressionType::sBuiltinTypes[(int)modifier->tag] = type;
    }

    void RegisterMagicDecl(
        RefPtr<Decl>                decl,
        RefPtr<MagicTypeModifier>   modifier)
    {
        ExpressionType::sMagicDecls[modifier->name] = decl.Ptr();
    }

    RefPtr<Decl> findMagicDecl(
        String const& name)
    {
        return ExpressionType::sMagicDecls[name].GetValue();
    }

    ExpressionType* ExpressionType::GetBool()
    {
        return sBuiltinTypes[(int)BaseType::Bool].GetValue().Ptr();
    }

    ExpressionType* ExpressionType::GetFloat()
    {
        return sBuiltinTypes[(int)BaseType::Float].GetValue().Ptr();
    }

    ExpressionType* ExpressionType::getDoubleType()
    {
        return sBuiltinTypes[(int)BaseType::Double].GetValue().Ptr();
    }

    ExpressionType* ExpressionType::GetInt()
    {
        return sBuiltinTypes[(int)BaseType::Int].GetValue().Ptr();
    }

    ExpressionType* ExpressionType::GetUInt()
    {
        return sBuiltinTypes[(int)BaseType::UInt].GetValue().Ptr();
    }

    ExpressionType* ExpressionType::GetVoid()
    {
        return sBuiltinTypes[(int)BaseType::Void].GetValue().Ptr();
    }

    ExpressionType* ExpressionType::getInitializerListType()
    {
        return initializerListType.Ptr();
    }

    ExpressionType* ExpressionType::GetError()
    {
        return ExpressionType::Error.Ptr();
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

    IntrinsicOp findIntrinsicOp(char const* name)
    {
        // TODO: need to make this faster by using a dictionary...

        if (0) {}
#define INTRINSIC(NAME) else if(strcmp(name, #NAME) == 0) return IntrinsicOp::NAME;
#include "intrinsic-defs.h"

        return IntrinsicOp::Unknown;
    }

    //

    // HLSLPatchType

    ExpressionType* HLSLPatchType::getElementType()
    {
        return this->declRef.substitutions->args[0].As<ExpressionType>().Ptr();
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return this->declRef.substitutions->args[1].As<IntVal>().Ptr();
    }
}