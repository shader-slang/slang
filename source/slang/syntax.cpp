#include "syntax.h"
#include "syntax-visitors.h"
#include <typeinfo>
#include <assert.h>

namespace Slang
{
    namespace Compiler
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

        CoreLib::Basic::String BasicExpressionType::ToString()
        {
            CoreLib::Basic::StringBuilder res;

            switch (BaseType)
            {
            case Compiler::BaseType::Int:
                res.Append("int");
                break;
            case Compiler::BaseType::UInt:
                res.Append("uint");
                break;
            case Compiler::BaseType::UInt64:
                res.Append("uint64_t");
                break;
            case Compiler::BaseType::Bool:
                res.Append("bool");
                break;
            case Compiler::BaseType::Float:
                res.Append("float");
                break;
            case Compiler::BaseType::Void:
                res.Append("void");
                break;
            default:
                break;
            }
            return res.ProduceString();
        }

        RefPtr<SyntaxNode> ProgramSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitProgram(this);
        }

        RefPtr<SyntaxNode> FunctionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitFunction(this);
        }

        //

        RefPtr<SyntaxNode> ScopeDecl::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitScopeDecl(this);
        }

        //

        RefPtr<SyntaxNode> BlockStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitBlockStatement(this);
        }

        RefPtr<SyntaxNode> BreakStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitBreakStatement(this);
        }

        RefPtr<SyntaxNode> ContinueStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitContinueStatement(this);
        }

        RefPtr<SyntaxNode> DoWhileStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitDoWhileStatement(this);
        }

        RefPtr<SyntaxNode> EmptyStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitEmptyStatement(this);
        }

        RefPtr<SyntaxNode> ForStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitForStatement(this);
        }

        RefPtr<SyntaxNode> IfStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitIfStatement(this);
        }

        RefPtr<SyntaxNode> ReturnStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitReturnStatement(this);
        }

        RefPtr<SyntaxNode> VarDeclrStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitVarDeclrStatement(this);
        }

        RefPtr<SyntaxNode> Variable::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitDeclrVariable(this);
        }

        RefPtr<SyntaxNode> WhileStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitWhileStatement(this);
        }

        RefPtr<SyntaxNode> ExpressionStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitExpressionStatement(this);
        }

        RefPtr<SyntaxNode> ConstantExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitConstantExpression(this);
        }

        RefPtr<SyntaxNode> IndexExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitIndexExpression(this);
        }
        RefPtr<SyntaxNode> MemberExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitMemberExpression(this);
        }

        // SwizzleExpr

        RefPtr<SyntaxNode> SwizzleExpr::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitSwizzleExpression(this);
        }

        // DerefExpr

        RefPtr<SyntaxNode> DerefExpr::Accept(SyntaxVisitor * /*visitor*/)
        {
            // throw "unimplemented";
            return this;
        }

        //

        RefPtr<SyntaxNode> InvokeExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitInvokeExpression(this);
        }

        RefPtr<SyntaxNode> TypeCastExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitTypeCastExpression(this);
        }

        RefPtr<SyntaxNode> VarExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitVarExpression(this);
        }

        // OverloadedExpr

        RefPtr<SyntaxNode> OverloadedExpr::Accept(SyntaxVisitor * /*visitor*/)
        {
//			throw "unimplemented";
            return this;
        }

        //

        RefPtr<SyntaxNode> ParameterSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitParameter(this);
        }

        // UsingFileDecl

        RefPtr<SyntaxNode> UsingFileDecl::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitUsingFileDecl(this);
        }

        //

        RefPtr<SyntaxNode> StructField::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitStructField(this);
        }
        RefPtr<SyntaxNode> StructSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitStruct(this);
        }
        RefPtr<SyntaxNode> ClassSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitClass(this);
        }
        RefPtr<SyntaxNode> TypeDefDecl::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitTypeDefDecl(this);
        }

        RefPtr<SyntaxNode> DiscardStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitDiscardStatement(this);
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
                assert(et->canonicalType);
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
            auto structDeclRef = declRefType->declRef.As<StructDeclRef>();
            if (!structDeclRef) return false;
            return true;
        }

        bool ExpressionType::IsClass()
        {
            auto declRefType = AsDeclRefType();
            if (!declRefType) return false;
            auto classDeclRef = declRefType->declRef.As<ClassDeclRef>();
            if (!classDeclRef) return false;
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
        CoreLib::Basic::String ArrayExpressionType::ToString()
        {
            if (ArrayLength)
                return BaseType->ToString() + "[" + ArrayLength->ToString() + "]";
            else
                return BaseType->ToString() + "[]";
        }
        RefPtr<SyntaxNode> GenericAppExpr::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitGenericApp(this);
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
            if (auto genericTypeParamDecl = dynamic_cast<GenericTypeParamDecl*>(declRef.GetDecl()))
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
            DeclRef substDeclRef = declRef.SubstituteImpl(subst, &diff);

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
            assert(type.Ptr());
            return type;
        }

        static RefPtr<IntVal> ExtractGenericArgInteger(RefPtr<Val> val)
        {
            auto intVal = val.As<IntVal>();
            assert(intVal.Ptr());
            return intVal;
        }

        // TODO: need to figure out how to unify this with the logic
        // in the generic case...
        DeclRefType* DeclRefType::Create(DeclRef declRef)
        {
            if (auto builtinMod = declRef.GetDecl()->FindModifier<BuiltinTypeModifier>())
            {
                auto type = new BasicExpressionType(builtinMod->tag);
                type->declRef = declRef;
                return type;
            }
            else if (auto magicMod = declRef.GetDecl()->FindModifier<MagicTypeModifier>())
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
                    assert(subst && subst->args.Count() == 2);
                    auto vecType = new VectorExpressionType();
                    vecType->declRef = declRef;
                    vecType->elementType = ExtractGenericArgType(subst->args[0]);
                    vecType->elementCount = ExtractGenericArgInteger(subst->args[1]);
                    return vecType;
                }
                else if (magicMod->name == "Matrix")
                {
                    assert(subst && subst->args.Count() == 3);
                    auto matType = new MatrixExpressionType();
                    matType->declRef = declRef;
                    return matType;
                }
                else if (magicMod->name == "Texture")
                {
                    assert(subst && subst->args.Count() >= 1);
                    auto textureType = new TextureType(
                        TextureType::Flavor(magicMod->tag),
                        ExtractGenericArgType(subst->args[0]));
                    textureType->declRef = declRef;
                    return textureType;
                }
                else if (magicMod->name == "TextureSampler")
                {
                    assert(subst && subst->args.Count() >= 1);
                    auto textureType = new TextureSamplerType(
                        TextureType::Flavor(magicMod->tag),
                        ExtractGenericArgType(subst->args[0]));
                    textureType->declRef = declRef;
                    return textureType;
                }
                else if (magicMod->name == "GLSLImageType")
                {
                    assert(subst && subst->args.Count() >= 1);
                    auto textureType = new GLSLImageType(
                        TextureType::Flavor(magicMod->tag),
                        ExtractGenericArgType(subst->args[0]));
                    textureType->declRef = declRef;
                    return textureType;
                }

                #define CASE(n,T)													\
                    else if(magicMod->name == #n) {									\
                        assert(subst && subst->args.Count() == 1);					\
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

                CASE(PackedBuffer, PackedBufferType)
                CASE(Uniform, UniformBufferType)
                CASE(Patch, PatchType)

                CASE(HLSLBufferType, HLSLBufferType)
                CASE(HLSLStructuredBufferType, HLSLStructuredBufferType)
                CASE(HLSLRWBufferType, HLSLRWBufferType)
                CASE(HLSLRWStructuredBufferType, HLSLRWStructuredBufferType)
                CASE(HLSLAppendStructuredBufferType, HLSLAppendStructuredBufferType)
                CASE(HLSLConsumeStructuredBufferType, HLSLConsumeStructuredBufferType)
                CASE(HLSLInputPatchType, HLSLInputPatchType)
                CASE(HLSLOutputPatchType, HLSLOutputPatchType)

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
                    throw "unimplemented";
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
            assert(!"unreachable");
            return false;
        }

        ExpressionType* NamedExpressionType::CreateCanonicalType()
        {
            return declRef.GetType()->GetCanonicalType();
        }

        int NamedExpressionType::GetHashCode()
        {
            assert(!"unreachable");
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
            assert(!"unreachable");
            return 0;
        }

        // GenericDeclRefType

        String GenericDeclRefType::ToString()
        {
            // TODO: what is appropriate here?
            return "<GenericDeclRef>";
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

        //

#if 0
        String GetOperatorFunctionName(Operator op)
        {
            switch (op)
            {
            case Operator::Add:
            case Operator::AddAssign:
                return "+";
            case Operator::Sub:
            case Operator::SubAssign:
                return "-";
            case Operator::Neg:
                return "-";
            case Operator::Not:
                return "!";
            case Operator::BitNot:
                return "~";
            case Operator::PreInc:
            case Operator::PostInc:
                return "++";
            case Operator::PreDec:
            case Operator::PostDec:
                return "--";
            case Operator::Mul:
            case Operator::MulAssign:
                return "*";
            case Operator::Div:
            case Operator::DivAssign:
                return "/";
            case Operator::Mod:
            case Operator::ModAssign:
                return "%";
            case Operator::Lsh:
            case Operator::LshAssign:
                return "<<";
            case Operator::Rsh:
            case Operator::RshAssign:
                return ">>";
            case Operator::Eql:
                return "==";
            case Operator::Neq:
                return "!=";
            case Operator::Greater:
                return ">";
            case Operator::Less:
                return "<";
            case Operator::Geq:
                return ">=";
            case Operator::Leq:
                return "<=";
            case Operator::BitAnd:
            case Operator::AndAssign:
                return "&";
            case Operator::BitXor:
            case Operator::XorAssign:
                return "^";
            case Operator::BitOr:
            case Operator::OrAssign:
                return "|";
            case Operator::And:
                return "&&";
            case Operator::Or:
                return "||";
            case Operator::Sequence:
                return ",";
            case Operator::Select:
                return "?:";
            case Operator::Assign:
                return "=";
            default:
                return "";
            }
        }
#endif
        String OperatorToString(Operator op)
        {
            switch (op)
            {
            case Slang::Compiler::Operator::Neg:
                return "-";
            case Slang::Compiler::Operator::Not:
                return "!";
            case Slang::Compiler::Operator::PreInc:
                return "++";
            case Slang::Compiler::Operator::PreDec:
                return "--";
            case Slang::Compiler::Operator::PostInc:
                return "++";
            case Slang::Compiler::Operator::PostDec:
                return "--";
            case Slang::Compiler::Operator::Mul:
            case Slang::Compiler::Operator::MulAssign:
                return "*";
            case Slang::Compiler::Operator::Div:
            case Slang::Compiler::Operator::DivAssign:
                return "/";
            case Slang::Compiler::Operator::Mod:
            case Slang::Compiler::Operator::ModAssign:
                return "%";
            case Slang::Compiler::Operator::Add:
            case Slang::Compiler::Operator::AddAssign:
                return "+";
            case Slang::Compiler::Operator::Sub:
            case Slang::Compiler::Operator::SubAssign:
                return "-";
            case Slang::Compiler::Operator::Lsh:
            case Slang::Compiler::Operator::LshAssign:
                return "<<";
            case Slang::Compiler::Operator::Rsh:
            case Slang::Compiler::Operator::RshAssign:
                return ">>";
            case Slang::Compiler::Operator::Eql:
                return "==";
            case Slang::Compiler::Operator::Neq:
                return "!=";
            case Slang::Compiler::Operator::Greater:
                return ">";
            case Slang::Compiler::Operator::Less:
                return "<";
            case Slang::Compiler::Operator::Geq:
                return ">=";
            case Slang::Compiler::Operator::Leq:
                return "<=";
            case Slang::Compiler::Operator::BitAnd:
            case Slang::Compiler::Operator::AndAssign:
                return "&";
            case Slang::Compiler::Operator::BitXor:
            case Slang::Compiler::Operator::XorAssign:
                return "^";
            case Slang::Compiler::Operator::BitOr:
            case Slang::Compiler::Operator::OrAssign:
                return "|";
            case Slang::Compiler::Operator::And:
                return "&&";
            case Slang::Compiler::Operator::Or:
                return "||";
            case Slang::Compiler::Operator::Assign:
                return "=";
            default:
                return "ERROR";
            }
        }

        // TypeExp

        TypeExp TypeExp::Accept(SyntaxVisitor* visitor)
        {
            return visitor->VisitTypeExp(*this);
        }

        // BuiltinTypeModifier

        // MagicTypeModifier

        // GenericDecl

        RefPtr<SyntaxNode> GenericDecl::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitGenericDecl(this);
        }

        // GenericTypeParamDecl

        RefPtr<SyntaxNode> GenericTypeParamDecl::Accept(SyntaxVisitor * /*visitor*/) {
            //throw "unimplemented";
            return this;
        }

        // GenericTypeConstraintDecl

        RefPtr<SyntaxNode> GenericTypeConstraintDecl::Accept(SyntaxVisitor * visitor)
        {
            return this;
        }

        // GenericValueParamDecl

        RefPtr<SyntaxNode> GenericValueParamDecl::Accept(SyntaxVisitor * /*visitor*/) {
            //throw "unimplemented";
            return this;
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
                if (genericDecl != declRef.GetDecl()->ParentDecl)
                    continue;

                int index = 0;
                for (auto m : genericDecl->Members)
                {
                    if (m.Ptr() == declRef.GetDecl())
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

        // ExtensionDecl

        RefPtr<SyntaxNode> ExtensionDecl::Accept(SyntaxVisitor * visitor)
        {
            visitor->VisitExtensionDecl(this);
            return this;
        }

        // ConstructorDecl

        RefPtr<SyntaxNode> ConstructorDecl::Accept(SyntaxVisitor * visitor)
        {
            visitor->VisitConstructorDecl(this);
            return this;
        }

        // SubscriptDecl

        RefPtr<SyntaxNode> SubscriptDecl::Accept(SyntaxVisitor * visitor)
        {
            visitor->visitSubscriptDecl(this);
            return this;
        }

        // AccessorDecl

        RefPtr<SyntaxNode> AccessorDecl::Accept(SyntaxVisitor * visitor)
        {
            visitor->visitAccessorDecl(this);
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

            int argCount = args.Count();
            assert(args.Count() == subst->args.Count());
            for (int aa = 0; aa < argCount; ++aa)
            {
                if (!args[aa]->EqualsVal(subst->args[aa].Ptr()))
                    return false;
            }

            if (!outer->Equals(subst->outer.Ptr()))
                return false;

            return true;
        }


        // DeclRef

        RefPtr<ExpressionType> DeclRef::Substitute(RefPtr<ExpressionType> type) const
        {
            // No substitutions? Easy.
            if (!substitutions)
                return type;

            // Otherwise we need to recurse on the type structure
            // and apply substitutions where it makes sense

            return type->Substitute(substitutions.Ptr()).As<ExpressionType>();
        }

        DeclRef DeclRef::Substitute(DeclRef declRef) const
        {
            if(!substitutions)
                return declRef;

            int diff = 0;
            return declRef.SubstituteImpl(substitutions.Ptr(), &diff);
        }

        RefPtr<ExpressionSyntaxNode> DeclRef::Substitute(RefPtr<ExpressionSyntaxNode> expr) const
        {
            // No substitutions? Easy.
            if (!substitutions)
                return expr;

            assert(!"unimplemented");

            return expr;
        }


        DeclRef DeclRef::SubstituteImpl(Substitutions* subst, int* ioDiff)
        {
            if (!substitutions) return *this;

            int diff = 0;
            RefPtr<Substitutions> substSubst = substitutions->SubstituteImpl(subst, &diff);

            if (!diff)
                return *this;

            *ioDiff += diff;

            DeclRef substDeclRef;
            substDeclRef.decl = decl;
            substDeclRef.substitutions = substSubst;
            return substDeclRef;
        }


        // Check if this is an equivalent declaration reference to another
        bool DeclRef::Equals(DeclRef const& declRef) const
        {
            if (decl != declRef.decl)
                return false;

            if (!substitutions->Equals(declRef.substitutions.Ptr()))
                return false;

            return true;
        }

        // Convenience accessors for common properties of declarations
        String const& DeclRef::GetName() const
        {
            return decl->Name.Content;
        }

        DeclRef DeclRef::GetParent() const
        {
            auto parentDecl = decl->ParentDecl;
            if (auto parentGeneric = dynamic_cast<GenericDecl*>(parentDecl))
            {
                // We need to strip away one layer of specialization
                assert(substitutions);
                return DeclRef(parentGeneric, substitutions->outer);
            }
            else
            {
                // If the parent isn't a generic, then it must
                // use the same specializations as this declaration
                return DeclRef(parentDecl, substitutions);
            }

        }

        int DeclRef::GetHashCode() const
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

        int GetIntVal(RefPtr<IntVal> val)
        {
            if (auto constantVal = val.As<ConstantIntVal>())
            {
                return constantVal->value;
            }
            assert(!"unexpected");
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
            return value;
        }

        // SwitchStmt

        RefPtr<SyntaxNode> SwitchStmt::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitSwitchStmt(this);
        }

        RefPtr<SyntaxNode> CaseStmt::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitCaseStmt(this);
        }

        RefPtr<SyntaxNode> DefaultStmt::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitDefaultStmt(this);
        }

        // TraitDecl

        RefPtr<SyntaxNode> TraitDecl::Accept(SyntaxVisitor * visitor)
        {
            visitor->VisitTraitDecl(this);
            return this;
        }

        // TraitConformanceDecl

        RefPtr<SyntaxNode> TraitConformanceDecl::Accept(SyntaxVisitor * visitor)
        {
            visitor->VisitTraitConformanceDecl(this);
            return this;
        }

        // SharedTypeExpr

        RefPtr<SyntaxNode> SharedTypeExpr::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitSharedTypeExpr(this);
        }

        // OperatorExpressionSyntaxNode

#if 0
        void OperatorExpressionSyntaxNode::SetOperator(RefPtr<Scope> scope, Slang::Compiler::Operator op)
        {
            this->Operator = op;
            auto opExpr = new VarExpressionSyntaxNode();
            opExpr->Variable = GetOperatorFunctionName(Operator);
            opExpr->scope = scope;
            opExpr->Position = this->Position;
            this->FunctionExpr = opExpr;
        }
#endif

        RefPtr<SyntaxNode> OperatorExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
        {
            return visitor->VisitOperatorExpression(this);
        }

        // DeclGroup

        RefPtr<SyntaxNode> DeclGroup::Accept(SyntaxVisitor * visitor)
        {
            visitor->VisitDeclGroup(this);
            return this;
        }

        //

        void RegisterBuiltinDecl(
            RefPtr<Decl>                decl,
            RefPtr<BuiltinTypeModifier> modifier)
        {
            auto type = DeclRefType::Create(DeclRef(decl.Ptr(), nullptr));
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

        RefPtr<SyntaxNode> UnparsedStmt::Accept(SyntaxVisitor * visitor)
        {
            return this;
        }

        //

        RefPtr<SyntaxNode> InitializerListExpr::Accept(SyntaxVisitor * visitor)
        {
            return visitor->visitInitializerListExpr(this);
        }

        //

        RefPtr<SyntaxNode> ModifierDecl::Accept(SyntaxVisitor * visitor)
        {
            return this;
        }

        //

        RefPtr<SyntaxNode> EmptyDecl::Accept(SyntaxVisitor * visitor)
        {
            return this;
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
                assert(!"unexpected");
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

    }
}