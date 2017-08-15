// lower.cpp
#include "lower-to-ir.h"

#include "ir.h"
#include "type-layout.h"
#include "visitor.h"

namespace Slang
{

struct SharedIRGenContext
{
    EntryPointRequest*  entryPoint;
    ProgramLayout*      programLayout;
    CodeGenTarget       target;
};

struct LoweredExprInfo
{
    enum class Flavor
    {
        Value,
    };

    static LoweredExprInfo createValue(IRValue* value)
    {
        LoweredExprInfo result;
        result.flavor = Flavor::Value;
        result.value = value;
        return result;
    }

    Flavor flavor;
    union
    {
        IRValue*    value;
    };
};

struct IRGenContext
{
    Dictionary<Decl*, LoweredExprInfo> declValues;

    IRBuilder* irBuilder;
};

struct LoweredValInfo
{
};

struct LoweredTypeInfo
{
    enum class Flavor
    {
        Type,
    };

    union
    {
        IRType* type;
    };
    Flavor flavor;
};

LoweredTypeInfo lowerType(
    IRGenContext*   context,
    Type*           type);

static LoweredTypeInfo lowerType(
    IRGenContext*   context,
    QualType const& type)
{
    return lowerType(context, type.type);
}

LoweredExprInfo lowerExpr(
    IRGenContext*   context,
    Expr*           expr);

//

struct ValLoweringVisitor : ValVisitor<ValLoweringVisitor, LoweredValInfo, LoweredTypeInfo>
{
    IRGenContext* context;

    LoweredValInfo visitVal(Val* val)
    {
        SLANG_UNIMPLEMENTED_X("value lowering");
    }

    LoweredTypeInfo visitType(Type* type)
    {
        SLANG_UNIMPLEMENTED_X("type lowering");
    }

};

LoweredTypeInfo lowerType(
    IRGenContext*   context,
    Type*           type)
{
    ValLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatchType(type);
}

#if 0
struct LoweringVisitor
    : ExprVisitor<LoweringVisitor, LoweredExpr>
    , StmtVisitor<LoweringVisitor, void>
    , DeclVisitor<LoweringVisitor, LoweredDecl>
    , ValVisitor<LoweringVisitor, RefPtr<Val>, RefPtr<Type>>
#endif


//

struct ExprLoweringVisitor : ExprVisitor<ExprLoweringVisitor, LoweredExprInfo>
{
    IRGenContext* context;

    LoweredExprInfo visitVarExpr(VarExpr* expr)
    {
        LoweredExprInfo info;
        if(context->declValues.TryGetValue(expr->declRef.getDecl(), info))
            return info;

        throw 99;

        return LoweredExprInfo();
    }

    LoweredExprInfo visitOverloadedExpr(OverloadedExpr* expr)
    {
        SLANG_UNEXPECTED("overloaded expressions should not occur in checked AST");
    }

    LoweredExprInfo visitInitializerListExpr(InitializerListExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for initializer list expression");
    }

    IRType* getIRType(LoweredTypeInfo const& typeInfo)
    {
        switch( typeInfo.flavor )
        {
        case LoweredTypeInfo::Flavor::Type:
            return typeInfo.type;
        }
    }

    LoweredExprInfo visitConstantExpr(ConstantExpr* expr)
    {
        auto type = getIRType(lowerType(context, expr->type));

        switch( expr->ConstType )
        {
        case ConstantExpr::ConstantType::Bool:
            return LoweredExprInfo::createValue(context->irBuilder->getBoolValue(expr->integerValue != 0));
        case ConstantExpr::ConstantType::Int:
            return LoweredExprInfo::createValue(context->irBuilder->getIntValue(type, expr->integerValue));
        case ConstantExpr::ConstantType::Float:
            return LoweredExprInfo::createValue(context->irBuilder->getFloatValue(type, expr->floatingPointValue));
        case ConstantExpr::ConstantType::String:
            break;
        }

        SLANG_UNEXPECTED("unexpected constant type");
    }

    LoweredExprInfo visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for aggregate type constructor expression");
    }

    LoweredExprInfo visitInvokeExpr(InvokeExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for invoke expression");
    }

    LoweredExprInfo visitIndexExpr(IndexExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for subscript expression");
    }

    LoweredExprInfo visitMemberExpr(MemberExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for subscript expression");
    }

    LoweredExprInfo visitSwizzleExpr(SwizzleExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for swizzle expression");
    }

    LoweredExprInfo visitDerefExpr(DerefExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for deref expression");
    }

    LoweredExprInfo visitTypeCastExpr(TypeCastExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for type cast expression");
    }

    LoweredExprInfo visitSelectExpr(SelectExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("codegen for select expression");
    }

    LoweredExprInfo visitGenericAppExpr(GenericAppExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("generic application expression during code generation");
    }

    LoweredExprInfo visitSharedTypeExpr(SharedTypeExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("shared type expression during code generation");
    }

    LoweredExprInfo visitAssignExpr(AssignExpr* expr)
    {
        SLANG_UNIMPLEMENTED_X("shared type expression during code generation");
    }

    LoweredExprInfo visitParenExpr(ParenExpr* expr)
    {
        return lowerExpr(context, expr->base);
    }
};

LoweredExprInfo lowerExpr(
    IRGenContext*   context,
    Expr*           expr)
{
    ExprLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(expr);
}

struct LoweredDeclInfo
{};

struct DeclLoweringVisitor : DeclVisitor<DeclLoweringVisitor, LoweredDeclInfo>
{
    IRGenContext* context;

    LoweredDeclInfo visitDeclBase(DeclBase* decl)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
    }

    LoweredDeclInfo visitDecl(Decl* decl)
    {
        SLANG_UNIMPLEMENTED_X("decl catch-all");
    }
};

LoweredDeclInfo lowerDecl(
    IRGenContext*   context,
    Decl*           decl)
{
    DeclLoweringVisitor visitor;
    visitor.context = context;
    return visitor.dispatch(decl);
}

EntryPointLayout* findEntryPointLayout(
    SharedIRGenContext* shared,
    EntryPointRequest*  entryPointRequest)
{
    for( auto entryPointLayout : shared->programLayout->entryPoints )
    {
        if(entryPointLayout->entryPoint->getName() != entryPointRequest->name)
            continue;

        if(entryPointLayout->profile != entryPointRequest->profile)
            continue;

        // TODO: can't easily filter on translation unit here...
        // Ideally the `EntryPointRequest` should get filled in with a pointer
        // the specific function declaration that represents the entry point.

        return entryPointLayout.Ptr();
    }

    return nullptr;
}

static void lowerEntryPointToIR(
    IRGenContext*       context,
    EntryPointRequest*  entryPointRequest,
    EntryPointLayout*   entryPointLayout)
{
    auto entryPointFunc = entryPointLayout->entryPoint;

    // TODO: entry point lowering is probably *not* just like lowering a function...

    lowerDecl(context, entryPointFunc);
}

void lowerEntryPointToIR(
    EntryPointRequest*  entryPoint,
    ProgramLayout*      programLayout,
    CodeGenTarget       target)
{
    SharedIRGenContext sharedContextStorage;
    SharedIRGenContext* sharedContext = &sharedContextStorage;

    IRGenContext contextStorage;
    IRGenContext* context = &contextStorage;

    auto entryPointLayout = findEntryPointLayout(sharedContext, entryPoint);

    lowerEntryPointToIR(context, entryPoint, entryPointLayout);

}

}
