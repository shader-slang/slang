// emit.cpp
#include "emit.h"

#include "syntax.h"
#include "type-layout.h"

#include <assert.h>

#ifdef _WIN32
#include <d3dcompiler.h>
#pragma warning(disable:4996)
#endif

namespace Slang {

struct EmitContext
{
    StringBuilder sb;

    // Current source position for tracking purposes...
    CodePosition loc;
    CodePosition nextSourceLocation;
    bool needToUpdateSourceLocation;

    // The target language we want to generate code for
    CodeGenTarget target;

    // A set of words reserved by the target
    Dictionary<String, String> reservedWords;

    // For GLSL output, we can't emit traidtional `#line` directives
    // with a file path in them, so we maintain a map that associates
    // each path with a unique integer, and then we output those
    // instead.
    Dictionary<String, int> mapGLSLSourcePathToID;
    int glslSourceIDCount = 0;

    // We only want to emit each `import`ed module one time, so
    // we maintain a set of already-emitted modules.
    HashSet<ProgramSyntaxNode*> modulesAlreadyEmitted;

    // We track the original global-scope layout so that we can
    // find layout information for `import`ed parameters.
    //
    // TODO: This will probably change if we represent imports
    // explicitly in the layout data.
    StructTypeLayout*   globalStructLayout;
};

//

static void EmitDecl(EmitContext* context, RefPtr<Decl> decl);
static void EmitDecl(EmitContext* context, RefPtr<DeclBase> declBase);
static void EmitDeclUsingLayout(EmitContext* context, RefPtr<Decl> decl, RefPtr<VarLayout> layout);

static void EmitType(EmitContext* context, RefPtr<ExpressionType> type, Token const& nameToken);
static void EmitType(EmitContext* context, RefPtr<ExpressionType> type, String const& name);
static void EmitType(EmitContext* context, RefPtr<ExpressionType> type);

static void EmitType(EmitContext* context, TypeExp const& typeExp, Token const& nameToken);
static void EmitType(EmitContext* context, TypeExp const& typeExp, String const& name);
static void EmitType(EmitContext* context, TypeExp const& typeExp);

static void EmitExpr(EmitContext* context, RefPtr<ExpressionSyntaxNode> expr);
static void EmitStmt(EmitContext* context, RefPtr<StatementSyntaxNode> stmt);
static void EmitDeclRef(EmitContext* context, DeclRef<Decl> declRef);

static void advanceToSourceLocation(
    EmitContext*        context,
    CodePosition const& sourceLocation);

static void flushSourceLocationChange(
    EmitContext*        context);

// Low-level emit logic

static void emitRawTextSpan(EmitContext* context, char const* textBegin, char const* textEnd)
{
    // TODO(tfoley): Need to make "corelib" not use `int` for pointer-sized things...
    auto len = int(textEnd - textBegin);

    context->sb.Append(textBegin, len);
}

static void emitRawText(EmitContext* context, char const* text)
{
    emitRawTextSpan(context, text, text + strlen(text));
}

static void emitTextSpan(EmitContext* context, char const* textBegin, char const* textEnd)
{
    // If the source location has changed in a way that required update,
    // do it now!
    flushSourceLocationChange(context);

    // Emit the raw text
    emitRawTextSpan(context, textBegin, textEnd);

    // Update our logical position
    // TODO(tfoley): Need to make "corelib" not use `int` for pointer-sized things...
    auto len = int(textEnd - textBegin);
    context->loc.Col += len;
}

static void Emit(EmitContext* context, char const* textBegin, char const* textEnd)
{
    char const* spanBegin = textBegin;

    char const* spanEnd = spanBegin;
    for(;;)
    {
        if(spanEnd == textEnd)
        {
            // We have a whole range of text waiting to be flushed
            emitTextSpan(context, spanBegin, spanEnd);
            return;
        }

        auto c = *spanEnd++;

        if( c == '\n' )
        {
            // At the end of a line, we need to update our tracking
            // information on code positions
            emitTextSpan(context, spanBegin, spanEnd);
            context->loc.Line++;
            context->loc.Col = 1;

            // Start a new span for emit purposes
            spanBegin = spanEnd;
        }
    }
}

static void Emit(EmitContext* context, char const* text)
{
    Emit(context, text, text + strlen(text));
}

static void emit(EmitContext* context, String const& text)
{
    Emit(context, text.begin(), text.end());
}

static bool isReservedWord(EmitContext* context, String const& name)
{
    return context->reservedWords.TryGetValue(name) != nullptr;
}

static void emitName(
    EmitContext*        context,
    String const&       inName,
    CodePosition const& loc)
{
    String name = inName;

    // By default, we would like to emit a name in the generated
    // code exactly as it appeared in the soriginal program.
    // When that isn't possible, we'd like to emit a name as
    // close to the original as possible (to ensure that existing
    // debugging tools still work reasonably well).
    //
    // One reason why a name might not be allowed as-is is that
    // it could collide with a reserved word in the target language.
    // Another reason is that it might not follow a naming convention
    // imposed by the target (e.g., in GLSL names starting with
    // `gl_` or containing `__` are reserved).
    //
    // Given a name that should not be allowed, we want to
    // change it to a name that *is* allowed. e.g., by adding
    // `_` to the end of a reserved word.
    //
    // The next problem this creates is that the modified name
    // could not collide with an existing use of the same
    // (valid) name.
    //
    // For now we are going to solve this problem in a simple
    // and ad hoc fashion, but longer term we'll want to do
    // something sytematic.

    if (isReservedWord(context, name))
    {
        name = name + "_";
    }

    advanceToSourceLocation(context, loc);
    emit(context, name);
}

static void emitName(EmitContext* context, Token const& nameToken)
{
    emitName(context, nameToken.Content, nameToken.Position);
}

static void emitName(EmitContext* context, String const& name)
{
    emitName(context, name, CodePosition());
}

static void Emit(EmitContext* context, IntegerLiteralValue value)
{
    char buffer[32];
    sprintf(buffer, "%lld", value);
    Emit(context, buffer);
}


static void Emit(EmitContext* context, UInt value)
{
    char buffer[32];
    sprintf(buffer, "%llu", (unsigned long long)(value));
    Emit(context, buffer);
}

static void Emit(EmitContext* context, int value)
{
    char buffer[16];
    sprintf(buffer, "%d", value);
    Emit(context, buffer);
}

static void Emit(EmitContext* context, double value)
{
    // TODO(tfoley): need to print things in a way that can round-trip
    char buffer[128];
    sprintf(buffer, "%.20ff", value);
    Emit(context, buffer);
}

static void emitTokenWithLocation(EmitContext* context, Token const& token);

// Expressions

// Determine if an expression should not be emitted when it is the base of
// a member reference expression.
static bool IsBaseExpressionImplicit(EmitContext* /*context*/, RefPtr<ExpressionSyntaxNode> expr)
{
    // HACK(tfoley): For now, anything with a constant-buffer type should be
    // left implicit.

    // Look through any dereferencing that took place
    RefPtr<ExpressionSyntaxNode> e = expr;
    while (auto derefExpr = e.As<DerefExpr>())
    {
        e = derefExpr->base;
    }
    // Is the expression referencing a constant buffer?
    if (auto cbufferType = e->Type->As<ConstantBufferType>())
    {
        return true;
    }

    return false;
}

enum
{
    kPrecedence_None,
    kPrecedence_Comma,

    kPrecedence_Assign,
    kPrecedence_AddAssign = kPrecedence_Assign,
    kPrecedence_SubAssign = kPrecedence_Assign,
    kPrecedence_MulAssign = kPrecedence_Assign,
    kPrecedence_DivAssign = kPrecedence_Assign,
    kPrecedence_ModAssign = kPrecedence_Assign,
    kPrecedence_LshAssign = kPrecedence_Assign,
    kPrecedence_RshAssign = kPrecedence_Assign,
    kPrecedence_OrAssign = kPrecedence_Assign,
    kPrecedence_AndAssign = kPrecedence_Assign,
    kPrecedence_XorAssign = kPrecedence_Assign,

    kPrecedence_General = kPrecedence_Assign,

    kPrecedence_Conditional, // "ternary"
    kPrecedence_Or,
    kPrecedence_And,
    kPrecedence_BitOr,
    kPrecedence_BitXor,
    kPrecedence_BitAnd,

    kPrecedence_Eql,
    kPrecedence_Neq = kPrecedence_Eql,

    kPrecedence_Less,
    kPrecedence_Greater = kPrecedence_Less,
    kPrecedence_Leq = kPrecedence_Less,
    kPrecedence_Geq = kPrecedence_Less,

    kPrecedence_Lsh,
    kPrecedence_Rsh = kPrecedence_Lsh,

    kPrecedence_Add,
    kPrecedence_Sub = kPrecedence_Add,

    kPrecedence_Mul,
    kPrecedence_Div = kPrecedence_Mul,
    kPrecedence_Mod = kPrecedence_Mul,

    kPrecedence_Prefix,
    kPrecedence_Postfix,
    kPrecedence_Atomic = kPrecedence_Postfix
};

static void EmitExprWithPrecedence(EmitContext* context, RefPtr<ExpressionSyntaxNode> expr, int outerPrec);

static void EmitPostfixExpr(EmitContext* context, RefPtr<ExpressionSyntaxNode> expr)
{
    EmitExprWithPrecedence(context, expr, kPrecedence_Postfix);
}

static void EmitExpr(EmitContext* context, RefPtr<ExpressionSyntaxNode> expr)
{
    EmitExprWithPrecedence(context, expr, kPrecedence_General);
}

static bool MaybeEmitParens(EmitContext* context, int outerPrec, int prec)
{
    if (prec <= outerPrec)
    {
        Emit(context, "(");
        return true;
    }
    return false;
}

// When we are going to emit an expression in an l-value context,
// we may need to ignore certain constructs that the type-checker
// might have introduced, but which interfere with our ability
// to use it effectively in the target language
static RefPtr<ExpressionSyntaxNode> prepareLValueExpr(
    EmitContext*                    /*context*/,
    RefPtr<ExpressionSyntaxNode>    expr)
{
    for(;;)
    {
        if(auto typeCastExpr = expr.As<TypeCastExpressionSyntaxNode>())
        {
            expr = typeCastExpr->Expression;
        }
        // TODO: any other cases?
        else
        {
            return expr;
        }
    }

}

static void emitInfixExprImpl(
    EmitContext* context,
    int outerPrec,
    int prec,
    char const* op,
    RefPtr<InvokeExpressionSyntaxNode> binExpr,
    bool isAssign)
{
    bool needsClose = MaybeEmitParens(context, outerPrec, prec);

    auto left = binExpr->Arguments[0];
    if(isAssign)
    {
        left = prepareLValueExpr(context, left);
    }

    EmitExprWithPrecedence(context, left, prec);
    Emit(context, " ");
    Emit(context, op);
    Emit(context, " ");
    EmitExprWithPrecedence(context, binExpr->Arguments[1], prec);
    if (needsClose)
    {
        Emit(context, ")");
    }
}

static void EmitBinExpr(EmitContext* context, int outerPrec, int prec, char const* op, RefPtr<InvokeExpressionSyntaxNode> binExpr)
{
    emitInfixExprImpl(context, outerPrec, prec, op, binExpr, false);
}

static void EmitBinAssignExpr(EmitContext* context, int outerPrec, int prec, char const* op, RefPtr<InvokeExpressionSyntaxNode> binExpr)
{
    emitInfixExprImpl(context, outerPrec, prec, op, binExpr, true);
}

static void emitUnaryExprImpl(
    EmitContext* context,
    int outerPrec,
    int prec,
    char const* preOp,
    char const* postOp,
    RefPtr<InvokeExpressionSyntaxNode> expr,
    bool isAssign)
{
    bool needsClose = MaybeEmitParens(context, outerPrec, prec);
    Emit(context, preOp);

    auto arg = expr->Arguments[0];
    if(isAssign)
    {
        arg = prepareLValueExpr(context, arg);
    }

    EmitExprWithPrecedence(context, arg, prec);
    Emit(context, postOp);
    if (needsClose)
    {
        Emit(context, ")");
    }
}

static void EmitUnaryExpr(
    EmitContext* context,
    int outerPrec,
    int prec,
    char const* preOp,
    char const* postOp,
    RefPtr<InvokeExpressionSyntaxNode> expr)
{
    emitUnaryExprImpl(context, outerPrec, prec, preOp, postOp, expr, false);
}

static void EmitUnaryAssignExpr(
    EmitContext* context,
    int outerPrec,
    int prec,
    char const* preOp,
    char const* postOp,
    RefPtr<InvokeExpressionSyntaxNode> expr)
{
    emitUnaryExprImpl(context, outerPrec, prec, preOp, postOp, expr, true);
}

// Determine if a target intrinsic modifer is applicable to the target
// we are currently emitting code for.
static bool isTargetIntrinsicModifierApplicable(
    EmitContext*                    context,
    RefPtr<TargetIntrinsicModifier> modifier)
{
    auto const& targetToken = modifier->targetToken;

    // If no target name was specified, then the modifier implicitly
    // applies to all targets.
    if(targetToken.Type == TokenType::Unknown)
        return true;

    // Otherwise, we need to check if the target name matches what
    // we expect.
    auto const& targetName = targetToken.Content;

    switch(context->target)
    {
    default:
        assert(!"unexpected");
        return false;

    case CodeGenTarget::GLSL: return targetName == "glsl";
    case CodeGenTarget::HLSL: return targetName == "hlsl";
    }
}

// Find an intrinsic modifier appropriate to the current compilation target.
//
// If there are multiple such modifiers, this should return the best one.
static RefPtr<TargetIntrinsicModifier> findTargetIntrinsicModifier(
    EmitContext*                    context,
    RefPtr<ModifiableSyntaxNode>    syntax)
{
    RefPtr<TargetIntrinsicModifier> bestModifier;
    for(auto m : syntax->GetModifiersOfType<TargetIntrinsicModifier>())
    {
        if(!isTargetIntrinsicModifierApplicable(context, m))
            continue;

        // For now "better"-ness is defined as: a modifier
        // with a specified target is better than one without
        // (it is more specific)
        if(!bestModifier || bestModifier->targetToken.Type == TokenType::Unknown)
        {
            bestModifier = m;
        }
    }

    return bestModifier;
}

static String getStringOrIdentifierTokenValue(
    Token const& token)
{
    switch(token.Type)
    {
    default:
        assert(!"unexpected");
        return "";

    case TokenType::Identifier:
        return token.Content;

    case TokenType::StringLiteral:
        return getStringLiteralTokenValue(token);
        break;
    }
}

// Emit a call expression that doesn't involve any special cases,
// just an expression of the form `f(a0, a1, ...)`
static void emitSimpleCallExpr(
    EmitContext*                        context,
    RefPtr<InvokeExpressionSyntaxNode>  callExpr,
    int                                 outerPrec)
{
    bool needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Postfix);

    auto funcExpr = callExpr->FunctionExpr;
    if (auto funcDeclRefExpr = funcExpr.As<DeclRefExpr>())
    {
        auto declRef = funcDeclRefExpr->declRef;
        if (auto ctorDeclRef = declRef.As<ConstructorDecl>())
        {
            // We really want to emit a reference to the type begin constructed
            EmitType(context, callExpr->Type);
        }
        else
        {
            // default case: just emit the decl ref
            EmitExpr(context, funcExpr);
        }
    }
    else
    {
        // default case: just emit the expression
        EmitPostfixExpr(context, funcExpr);
    }

    Emit(context, "(");
    int argCount = callExpr->Arguments.Count();
    for (int aa = 0; aa < argCount; ++aa)
    {
        if (aa != 0) Emit(context, ", ");
        EmitExpr(context, callExpr->Arguments[aa]);
    }
    Emit(context, ")");

    if (needClose)
    {
        Emit(context, ")");
    }
}

static void emitCallExpr(
    EmitContext*                        context,
    RefPtr<InvokeExpressionSyntaxNode>  callExpr,
    int                                 outerPrec)
{
    auto funcExpr = callExpr->FunctionExpr;
    if (auto funcDeclRefExpr = funcExpr.As<DeclRefExpr>())
    {
        auto funcDeclRef = funcDeclRefExpr->declRef;
        auto funcDecl = funcDeclRef.getDecl();
        if(!funcDecl)
        {
            // This can occur when we are dealing with unchecked input syntax,
            // because we are in "rewriter" mode. In this case we should go
            // ahead and emit things in the form that they were written.
            if( auto infixExpr = callExpr.As<InfixExpr>() )
            {
                EmitBinExpr(
                    context,
                    outerPrec,
                    kPrecedence_Comma,
                    funcDeclRefExpr->name.Buffer(),
                    callExpr);
            }
            else if( auto prefixExpr = callExpr.As<PrefixExpr>() )
            {
                EmitUnaryExpr(
                    context,
                    outerPrec,
                    kPrecedence_Prefix,
                    funcDeclRefExpr->name.Buffer(),
                    "",
                    callExpr);
            }
            else if(auto postfixExpr = callExpr.As<PostfixExpr>())
            {
                EmitUnaryExpr(
                    context,
                    outerPrec,
                    kPrecedence_Postfix,
                    "",
                    funcDeclRefExpr->name.Buffer(),
                    callExpr);
            }
            else
            {
                emitSimpleCallExpr(context, callExpr, outerPrec);
            }
            return;
        }
        else if (auto intrinsicOpModifier = funcDecl->FindModifier<IntrinsicOpModifier>())
        {
            switch (intrinsicOpModifier->op)
            {
#define CASE(NAME, OP) case IntrinsicOp::NAME: EmitBinExpr(context, outerPrec, kPrecedence_##NAME, #OP, callExpr); return
            CASE(Mul, *);
            CASE(Div, / );
            CASE(Mod, %);
            CASE(Add, +);
            CASE(Sub, -);
            CASE(Lsh, << );
            CASE(Rsh, >> );
            CASE(Eql, == );
            CASE(Neq, != );
            CASE(Greater, >);
            CASE(Less, <);
            CASE(Geq, >= );
            CASE(Leq, <= );
            CASE(BitAnd, &);
            CASE(BitXor, ^);
            CASE(BitOr, | );
            CASE(And, &&);
            CASE(Or, || );
#undef CASE

#define CASE(NAME, OP) case IntrinsicOp::NAME: EmitBinAssignExpr(context, outerPrec, kPrecedence_##NAME, #OP, callExpr); return
            CASE(Assign, =);
            CASE(AddAssign, +=);
            CASE(SubAssign, -=);
            CASE(MulAssign, *=);
            CASE(DivAssign, /=);
            CASE(ModAssign, %=);
            CASE(LshAssign, <<=);
            CASE(RshAssign, >>=);
            CASE(OrAssign, |=);
            CASE(AndAssign, &=);
            CASE(XorAssign, ^=);
#undef CASE

        case IntrinsicOp::Sequence: EmitBinExpr(context, outerPrec, kPrecedence_Comma, ",", callExpr); return;

#define CASE(NAME, OP) case IntrinsicOp::NAME: EmitUnaryExpr(context, outerPrec, kPrecedence_Prefix, #OP, "", callExpr); return
            CASE(Neg, -);
            CASE(Not, !);
            CASE(BitNot, ~);
#undef CASE

#define CASE(NAME, OP) case IntrinsicOp::NAME: EmitUnaryAssignExpr(context, outerPrec, kPrecedence_Prefix, #OP, "", callExpr); return
            CASE(PreInc, ++);
            CASE(PreDec, --);
#undef CASE

#define CASE(NAME, OP) case IntrinsicOp::NAME: EmitUnaryAssignExpr(context, outerPrec, kPrecedence_Postfix, "", #OP, callExpr); return
            CASE(PostInc, ++);
            CASE(PostDec, --);
#undef CASE

            case IntrinsicOp::InnerProduct_Vector_Vector:
                // HLSL allows `mul()` to be used as a synonym for `dot()`,
                // so we need to translate to `dot` for GLSL
                if (context->target == CodeGenTarget::GLSL)
                {
                    Emit(context, "dot(");
                    EmitExpr(context, callExpr->Arguments[0]);
                    Emit(context, ", ");
                    EmitExpr(context, callExpr->Arguments[1]);
                    Emit(context, ")");
                    return;
                }
                break;

            case IntrinsicOp::InnerProduct_Matrix_Matrix:
            case IntrinsicOp::InnerProduct_Matrix_Vector:
            case IntrinsicOp::InnerProduct_Vector_Matrix:
                // HLSL exposes these with the `mul()` function, while GLSL uses ordinary
                // `operator*`.
                //
                // The other critical detail here is that the way we handle matrix
                // conventions requires that the operands to the product be swapped.
                if (context->target == CodeGenTarget::GLSL)
                {
                    Emit(context, "((");
                    EmitExpr(context, callExpr->Arguments[1]);
                    Emit(context, ") * (");
                    EmitExpr(context, callExpr->Arguments[0]);
                    Emit(context, "))");
                    return;
                }
                break;

            default:
                break;
            }
        }
        else if(auto targetIntrinsicModifier = findTargetIntrinsicModifier(context, funcDecl))
        {
            if(targetIntrinsicModifier->definitionToken.Type != TokenType::Unknown)
            {
                auto name = getStringOrIdentifierTokenValue(targetIntrinsicModifier->definitionToken);

                if(name.IndexOf('$') < 0)
                {
                    // Simple case: it is just an ordinary name, so we call it like a builtin.
                    //
                    // TODO: this case could probably handle things like operators, for generality?

                    emit(context, name);
                    Emit(context, "(");
                    int argCount = callExpr->Arguments.Count();
                    for (int aa = 0; aa < argCount; ++aa)
                    {
                        if (aa != 0) Emit(context, ", ");
                        EmitExpr(context, callExpr->Arguments[aa]);
                    }
                    Emit(context, ")");
                    return;
                }
                else
                {
                    // General case: we are going to emit some more complex text.

                    int argCount = callExpr->Arguments.Count();

                    Emit(context, "(");

                    char const* cursor = name.begin();
                    char const* end = name.end();
                    while(cursor != end)
                    {
                        char c = *cursor++;
                        if( c != '$' )
                        {
                            // Not an escape sequence
                            emitRawTextSpan(context, &c, &c+1);
                            continue;
                        }

                        assert(cursor != end);

                        char d = *cursor++;
                        assert(('0' <= d) && (d <= '9'));

                        int argIndex = d - '0';
                        assert((0 <= argIndex) && (argIndex < argCount));
                        Emit(context, "(");
                        EmitExpr(context, callExpr->Arguments[argIndex]);
                        Emit(context, ")");
                    }

                    Emit(context, ")");
                }

                return;
            }

            // TODO: emit as approperiate for this target

            // We might be calling an intrinsic subscript operation,
            // and should desugar it accordingly
            if(auto subscriptDeclRef = funcDeclRef.As<SubscriptDecl>())
            {
                // We expect any subscript operation to be invoked as a member,
                // so the function expression had better be in the correct form.
                if(auto memberExpr = funcExpr.As<MemberExpressionSyntaxNode>())
                {

                    Emit(context, "(");
                    EmitExpr(context, memberExpr->BaseExpression);
                    Emit(context, ")[");
                    int argCount = callExpr->Arguments.Count();
                    for (int aa = 0; aa < argCount; ++aa)
                    {
                        if (aa != 0) Emit(context, ", ");
                        EmitExpr(context, callExpr->Arguments[aa]);
                    }
                    Emit(context, "]");
                    return;
                }
            }
        }
    }

    // Fall through to default handling...
    emitSimpleCallExpr(context, callExpr, outerPrec);
}

static void emitStringLiteral(
    EmitContext*    context,
    String const&   value)
{
    emit(context, "\"");
    for (auto c : value)
    {
        // TODO: This needs a more complete implementation,
        // especially if we want to support Unicode.

        char buffer[] = { c, 0 };
        switch (c)
        {
        default:
            emit(context, buffer);
            break;

        case '\"': emit(context, "\\\"");
        case '\'': emit(context, "\\\'");
        case '\\': emit(context, "\\\\");
        case '\n': emit(context, "\\n");
        case '\r': emit(context, "\\r");
        case '\t': emit(context, "\\t");
        }
    }
    emit(context, "\"");
}

static void EmitExprWithPrecedence(EmitContext* context, RefPtr<ExpressionSyntaxNode> expr, int outerPrec)
{
    bool needClose = false;
    if (auto selectExpr = expr.As<SelectExpressionSyntaxNode>())
    {
        needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Conditional);

        EmitExprWithPrecedence(context, selectExpr->Arguments[0], kPrecedence_Conditional);
        Emit(context, " ? ");
        EmitExprWithPrecedence(context, selectExpr->Arguments[1], kPrecedence_Conditional);
        Emit(context, " : ");
        EmitExprWithPrecedence(context, selectExpr->Arguments[2], kPrecedence_Conditional);
    }
    else if (auto callExpr = expr.As<InvokeExpressionSyntaxNode>())
    {
        emitCallExpr(context, callExpr, outerPrec);
    }
    else if (auto memberExpr = expr.As<MemberExpressionSyntaxNode>())
    {
        needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Postfix);

        // TODO(tfoley): figure out a good way to reference
        // declarations that might be generic and/or might
        // not be generated as lexically nested declarations...

        // TODO(tfoley): also, probably need to special case
        // this for places where we are using a built-in...

        auto base = memberExpr->BaseExpression;
        if (IsBaseExpressionImplicit(context, base))
        {
            // don't emit the base expression
        }
        else
        {
            EmitExprWithPrecedence(context, memberExpr->BaseExpression, kPrecedence_Postfix);
            Emit(context, ".");
        }

        emitName(context, memberExpr->declRef.GetName());
    }
    else if (auto swizExpr = expr.As<SwizzleExpr>())
    {
        needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Postfix);

        EmitExprWithPrecedence(context, swizExpr->base, kPrecedence_Postfix);
        Emit(context, ".");
        static const char* kComponentNames[] = { "x", "y", "z", "w" };
        int elementCount = swizExpr->elementCount;
        for (int ee = 0; ee < elementCount; ++ee)
        {
            Emit(context, kComponentNames[swizExpr->elementIndices[ee]]);
        }
    }
    else if (auto indexExpr = expr.As<IndexExpressionSyntaxNode>())
    {
        needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Postfix);

        EmitExprWithPrecedence(context, indexExpr->BaseExpression, kPrecedence_Postfix);
        Emit(context, "[");
        EmitExpr(context, indexExpr->IndexExpression);
        Emit(context, "]");
    }
    else if (auto varExpr = expr.As<VarExpressionSyntaxNode>())
    {
        needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Atomic);

        // TODO: This won't be valid if we had to generate a qualified
        // reference for some reason.
        advanceToSourceLocation(context, varExpr->Position);

        // Because of the "rewriter" use case, it is possible that we will
        // be trying to emit an expression that hasn't been wired up to
        // any associated declaration. In that case, we will just emit
        // the variable name.
        //
        // TODO: A better long-term solution here is to have a distinct
        // case for an "unchecked" `NameExpr` that doesn't include
        // a declaration reference.

        if(varExpr->declRef)
        {
            EmitDeclRef(context, varExpr->declRef);
        }
        else
        {
            emitName(context, varExpr->name);
        }
    }
    else if (auto derefExpr = expr.As<DerefExpr>())
    {
        // TODO(tfoley): dereference shouldn't always be implicit
        EmitExprWithPrecedence(context, derefExpr->base, outerPrec);
    }
    else if (auto litExpr = expr.As<ConstantExpressionSyntaxNode>())
    {
        needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Atomic);

        char const* suffix = "";
        auto type = litExpr->Type.type;
        switch (litExpr->ConstType)
        {
        case ConstantExpressionSyntaxNode::ConstantType::Int:
            if(!type)
            {
                // Special case for "rewrite" mode
                emitTokenWithLocation(context, litExpr->token);
                break;
            }
            if(type->Equals(ExpressionType::GetInt()))
            {}
            else if(type->Equals(ExpressionType::GetUInt()))
            {
                suffix = "u";
            }
            else
            {
                assert(!"unimplemented");
            }
            Emit(context, litExpr->integerValue);
            Emit(context, suffix);
            break;


        case ConstantExpressionSyntaxNode::ConstantType::Float:
            if(!type)
            {
                // Special case for "rewrite" mode
                emitTokenWithLocation(context, litExpr->token);
                break;
            }
            if(type->Equals(ExpressionType::GetFloat()))
            {}
            else if(type->Equals(ExpressionType::getDoubleType()))
            {
                suffix = "l";
            }
            else
            {
                assert(!"unimplemented");
            }
            Emit(context, litExpr->floatingPointValue);
            Emit(context, suffix);
            break;

        case ConstantExpressionSyntaxNode::ConstantType::Bool:
            Emit(context, litExpr->integerValue ? "true" : "false");
            break;
        case ConstantExpressionSyntaxNode::ConstantType::String:
            emitStringLiteral(context, litExpr->stringValue);
            break;
        default:
            assert(!"unreachable");
            break;
        }
    }
    else if (auto castExpr = expr.As<TypeCastExpressionSyntaxNode>())
    {
        switch(context->target)
        {
        case CodeGenTarget::GLSL:
            // GLSL requires constructor syntax for all conversions
            EmitType(context, castExpr->Type);
            Emit(context, "(");
            EmitExpr(context, castExpr->Expression);
            Emit(context, ")");
            break;

        default:
            // HLSL (and C/C++) prefer cast syntax
            // (In fact, HLSL doesn't allow constructor syntax for some conversions it allows as a cast)
            needClose = MaybeEmitParens(context, outerPrec, kPrecedence_Prefix);

            Emit(context, "(");
            EmitType(context, castExpr->Type);
            Emit(context, ")(");
            EmitExpr(context, castExpr->Expression);
            Emit(context, ")");
            break;
        }

    }
    else if(auto initExpr = expr.As<InitializerListExpr>())
    {
        Emit(context, "{ ");
        for(auto& arg : initExpr->args)
        {
            EmitExpr(context, arg);
            Emit(context, ", ");
        }
        Emit(context, "}");
    }
    else
    {
        throw "unimplemented";
    }
    if (needClose)
    {
        Emit(context, ")");
    }
}

// Types

void Emit(EmitContext* context, RefPtr<IntVal> val)
{
    if(auto constantIntVal = val.As<ConstantIntVal>())
    {
        Emit(context, constantIntVal->value);
    }
    else if(auto varRefVal = val.As<GenericParamIntVal>())
    {
        EmitDeclRef(context, varRefVal->declRef);
    }
    else
    {
        assert(!"unimplemented");
    }
}

// represents a declarator for use in emitting types
struct EDeclarator
{
    enum class Flavor
    {
        Name,
        Array,
        UnsizedArray,
    };
    Flavor flavor;
    EDeclarator* next = nullptr;

    // Used for `Flavor::Name`
    String name;
    CodePosition loc;

    // Used for `Flavor::Array`
    IntVal* elementCount;
};

static void EmitDeclarator(EmitContext* context, EDeclarator* declarator)
{
    if (!declarator) return;

    Emit(context, " ");

    switch (declarator->flavor)
    {
    case EDeclarator::Flavor::Name:
        emitName(context, declarator->name, declarator->loc);
        break;

    case EDeclarator::Flavor::Array:
        EmitDeclarator(context, declarator->next);
        Emit(context, "[");
        if(auto elementCount = declarator->elementCount)
        {
            Emit(context, elementCount);
        }
        Emit(context, "]");
        break;

    case EDeclarator::Flavor::UnsizedArray:
        EmitDeclarator(context, declarator->next);
        Emit(context, "[]");
        break;

    default:
        assert(!"unreachable");
        break;
    }
}

static void emitGLSLTypePrefix(
    EmitContext*            context,
    RefPtr<ExpressionType>  type)
{
    if(auto basicElementType = type->As<BasicExpressionType>())
    {
        switch (basicElementType->BaseType)
        {
        case BaseType::Float:
            // no prefix
            break;

        case BaseType::Int:		Emit(context, "i");		break;
        case BaseType::UInt:	Emit(context, "u");		break;
        case BaseType::Bool:	Emit(context, "b");		break;
        default:
            assert(!"unreachable");
            break;
        }
    }
    else if(auto vectorType = type->As<VectorExpressionType>())
    {
        emitGLSLTypePrefix(context, vectorType->elementType);
    }
    else if(auto matrixType = type->As<MatrixExpressionType>())
    {
        emitGLSLTypePrefix(context, matrixType->getElementType());
    }
    else
    {
        assert(!"unreachable");
    }
}

static void emitHLSLTextureType(
    EmitContext*            context,
    RefPtr<TextureTypeBase> texType)
{
    switch(texType->getAccess())
    {
    case SLANG_RESOURCE_ACCESS_READ:
        break;

    case SLANG_RESOURCE_ACCESS_READ_WRITE:
        Emit(context, "RW");
        break;

    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        Emit(context, "RasterizerOrdered");
        break;

    case SLANG_RESOURCE_ACCESS_APPEND:
        Emit(context, "Append");
        break;

    case SLANG_RESOURCE_ACCESS_CONSUME:
        Emit(context, "Consume");
        break;

    default:
        assert(!"unreachable");
        break;
    }

    switch (texType->GetBaseShape())
    {
    case TextureType::Shape1D:		Emit(context, "Texture1D");		break;
    case TextureType::Shape2D:		Emit(context, "Texture2D");		break;
    case TextureType::Shape3D:		Emit(context, "Texture3D");		break;
    case TextureType::ShapeCube:	Emit(context, "TextureCube");	break;
    default:
        assert(!"unreachable");
        break;
    }

    if (texType->isMultisample())
    {
        Emit(context, "MS");
    }
    if (texType->isArray())
    {
        Emit(context, "Array");
    }
    Emit(context, "<");
    EmitType(context, texType->elementType);
    Emit(context, " >");
}

static void emitGLSLTextureOrTextureSamplerType(
    EmitContext*            context,
    RefPtr<TextureTypeBase> type,
    char const*             baseName)
{
    emitGLSLTypePrefix(context, type->elementType);

    Emit(context, baseName);
    switch (type->GetBaseShape())
    {
    case TextureType::Shape1D:		Emit(context, "1D");		break;
    case TextureType::Shape2D:		Emit(context, "2D");		break;
    case TextureType::Shape3D:		Emit(context, "3D");		break;
    case TextureType::ShapeCube:	Emit(context, "Cube");	break;
    default:
        assert(!"unreachable");
        break;
    }

    if (type->isMultisample())
    {
        Emit(context, "MS");
    }
    if (type->isArray())
    {
        Emit(context, "Array");
    }
}

static void emitGLSLTextureType(
    EmitContext*        context,
    RefPtr<TextureType> texType)
{
    emitGLSLTextureOrTextureSamplerType(context, texType, "texture");
}

static void emitGLSLTextureSamplerType(
    EmitContext*                context,
    RefPtr<TextureSamplerType>  type)
{
    emitGLSLTextureOrTextureSamplerType(context, type, "sampler");
}

static void emitGLSLImageType(
    EmitContext*            context,
    RefPtr<GLSLImageType>   type)
{
    emitGLSLTextureOrTextureSamplerType(context, type, "image");
}

static void emitTextureType(
    EmitContext*        context,
    RefPtr<TextureType> texType)
{
    switch(context->target)
    {
    case CodeGenTarget::HLSL:
        emitHLSLTextureType(context, texType);
        break;

    case CodeGenTarget::GLSL:
        emitGLSLTextureType(context, texType);
        break;

    default:
        assert(!"unreachable");
        break;
    }
}

static void emitTextureSamplerType(
    EmitContext*                context,
    RefPtr<TextureSamplerType>  type)
{
    switch(context->target)
    {
    case CodeGenTarget::GLSL:
        emitGLSLTextureSamplerType(context, type);
        break;

    default:
        assert(!"unreachable");
        break;
    }
}

static void emitImageType(
    EmitContext*            context,
    RefPtr<GLSLImageType>   type)
{
    switch(context->target)
    {
    case CodeGenTarget::HLSL:
        emitHLSLTextureType(context, type);
        break;

    case CodeGenTarget::GLSL:
        emitGLSLImageType(context, type);
        break;

    default:
        assert(!"unreachable");
        break;
    }
}

static void EmitType(EmitContext* context, RefPtr<ExpressionType> type, EDeclarator* declarator)
{
    if (auto basicType = type->As<BasicExpressionType>())
    {
        switch (basicType->BaseType)
        {
        case BaseType::Void:	Emit(context, "void");		break;
        case BaseType::Int:		Emit(context, "int");		break;
        case BaseType::Float:	Emit(context, "float");		break;
        case BaseType::UInt:	Emit(context, "uint");		break;
        case BaseType::Bool:	Emit(context, "bool");		break;
        default:
            assert(!"unreachable");
            break;
        }

        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto vecType = type->As<VectorExpressionType>())
    {
        switch(context->target)
        {
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                emitGLSLTypePrefix(context, vecType->elementType);
                Emit(context, "vec");
                Emit(context, vecType->elementCount);
            }
            break;

        case CodeGenTarget::HLSL:
            // TODO(tfoley): should really emit these with sugar
            Emit(context, "vector<");
            EmitType(context, vecType->elementType);
            Emit(context, ",");
            Emit(context, vecType->elementCount);
            Emit(context, ">");
            break;

        default:
            assert(!"unreachable");
            break;
        }

        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto matType = type->As<MatrixExpressionType>())
    {
        switch(context->target)
        {
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                emitGLSLTypePrefix(context, matType->getElementType());
                Emit(context, "mat");
                Emit(context, matType->getRowCount());
                // TODO(tfoley): only emit the next bit
                // for non-square matrix
                Emit(context, "x");
                Emit(context, matType->getColumnCount());
            }
            break;

        case CodeGenTarget::HLSL:
            // TODO(tfoley): should really emit these with sugar
            Emit(context, "matrix<");
            EmitType(context, matType->getElementType());
            Emit(context, ",");
            Emit(context, matType->getRowCount());
            Emit(context, ",");
            Emit(context, matType->getColumnCount());
            Emit(context, "> ");
            break;

        default:
            assert(!"unreachable");
            break;
        }

        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto texType = type->As<TextureType>())
    {
        emitTextureType(context, texType);
        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto textureSamplerType = type->As<TextureSamplerType>())
    {
        emitTextureSamplerType(context, textureSamplerType);
        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto imageType = type->As<GLSLImageType>())
    {
        emitImageType(context, imageType);
        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto samplerStateType = type->As<SamplerStateType>())
    {
        switch(context->target)
        {
        case CodeGenTarget::HLSL:
        default:
            switch (samplerStateType->flavor)
            {
            case SamplerStateType::Flavor::SamplerState:			Emit(context, "SamplerState");				break;
            case SamplerStateType::Flavor::SamplerComparisonState:	Emit(context, "SamplerComparisonState");	break;
            default:
                assert(!"unreachable");
                break;
            }
            break;

        case CodeGenTarget::GLSL:
            Emit(context, "sampler");
            break;
        }


        EmitDeclarator(context, declarator);
        return;
    }
    else if (auto declRefType = type->As<DeclRefType>())
    {
        EmitDeclRef(context,  declRefType->declRef);

        EmitDeclarator(context, declarator);
        return;
    }
    else if( auto arrayType = type->As<ArrayExpressionType>() )
    {
        EDeclarator arrayDeclarator;
        arrayDeclarator.next = declarator;

        if(arrayType->ArrayLength)
        {
            arrayDeclarator.flavor = EDeclarator::Flavor::Array;
            arrayDeclarator.elementCount = arrayType->ArrayLength.Ptr();
        }
        else
        {
            arrayDeclarator.flavor = EDeclarator::Flavor::UnsizedArray;
        }


        EmitType(context, arrayType->BaseType, &arrayDeclarator);
        return;
    }

    throw "unimplemented";
}

static void EmitType(
    EmitContext*            context,
    RefPtr<ExpressionType>  type,
    CodePosition const&     typeLoc,
    String const&           name,
    CodePosition const&     nameLoc)
{
    advanceToSourceLocation(context, typeLoc);

    EDeclarator nameDeclarator;
    nameDeclarator.flavor = EDeclarator::Flavor::Name;
    nameDeclarator.name = name;
    nameDeclarator.loc = nameLoc;
    EmitType(context, type, &nameDeclarator);
}


static void EmitType(EmitContext* context, RefPtr<ExpressionType> type, Token const& nameToken)
{
    EmitType(context, type, CodePosition(), nameToken.Content, nameToken.Position);
}


static void EmitType(EmitContext* context, RefPtr<ExpressionType> type, String const& name)
{
    EmitType(context, type, CodePosition(), name, CodePosition());
}

static void EmitType(EmitContext* context, RefPtr<ExpressionType> type)
{
    EmitType(context, type, nullptr);
}

static void EmitType(EmitContext* context, TypeExp const& typeExp, Token const& nameToken)
{
    EmitType(context, typeExp.type, typeExp.exp->Position, nameToken.Content, nameToken.Position);
}

static void EmitType(EmitContext* context, TypeExp const& typeExp, String const& name)
{
    EmitType(context, typeExp.type, typeExp.exp->Position, name, CodePosition());
}

static void EmitType(EmitContext* context, TypeExp const& typeExp)
{
    advanceToSourceLocation(context, typeExp.exp->Position);
    EmitType(context, typeExp.type, nullptr);
}


// Statements

// Emit a statement as a `{}`-enclosed block statement, but avoid adding redundant
// curly braces if the statement is itself a block statement.
static void EmitBlockStmt(EmitContext* context, RefPtr<StatementSyntaxNode> stmt)
{
    // TODO(tfoley): support indenting
    Emit(context, "{\n");
    if( auto blockStmt = stmt.As<BlockStatementSyntaxNode>() )
    {
        for (auto s : blockStmt->Statements)
        {
            EmitStmt(context, s);
        }
    }
    else
    {
        EmitStmt(context, stmt);
    }
    Emit(context, "}\n");
}

static void EmitLoopAttributes(EmitContext* context, RefPtr<StatementSyntaxNode> decl)
{
    // TODO(tfoley): There really ought to be a semantic checking step for attributes,
    // that turns abstract syntax into a concrete hierarchy of attribute types (e.g.,
    // a specific `LoopModifier` or `UnrollModifier`).

    for(auto attr : decl->GetModifiersOfType<HLSLUncheckedAttribute>())
    {
        if(attr->nameToken.Content == "loop")
        {
            Emit(context, "[loop]");
        }
        else if(attr->nameToken.Content == "unroll")
        {
            Emit(context, "[unroll]");
        }
    }
}

// Emit a `#line` directive to the output.
// Doesn't udpate state of source-location tracking.
static void emitLineDirective(
    EmitContext*        context,
    CodePosition const& sourceLocation)
{
    emitRawText(context, "\n#line ");

    char buffer[16];
    sprintf(buffer, "%d", sourceLocation.Line);
    emitRawText(context, buffer);

    emitRawText(context, " ");

    if(context->target == CodeGenTarget::GLSL)
    {
        auto path = sourceLocation.FileName;

        // GLSL doesn't support the traditional form of a `#line` directive without
        // an extension. Rather than depend on that extension we will output
        // a directive in the traditional GLSL fashion.
        //
        // TODO: Add some kind of configuration where we require the appropriate
        // extension and then emit a traditional line directive.

        int id = 0;
        if(!context->mapGLSLSourcePathToID.TryGetValue(path, id))
        {
            id = context->glslSourceIDCount++;
            context->mapGLSLSourcePathToID.Add(path, id);
        }

        sprintf(buffer, "%d", id);
        emitRawText(context, buffer);
    }
    else
    {
        // The simple case is to emit the path for the current source
        // location. We need to be a little bit careful with this,
        // because the path might include backslash characters if we
        // are on Windows, and we want to canonicalize those over
        // to forward slashes.
        //
        // TODO: Canonicalization like this should be done centrally
        // in a module that tracks source files.

        emitRawText(context, "\"");
        for(auto c : sourceLocation.FileName)
        {
            char charBuffer[] = { c, 0 };
            switch(c)
            {
            default:
                emitRawText(context, charBuffer);
                break;

            // The incoming file path might use `/` and/or `\\` as
            // a directory separator. We want to canonicalize this.
            //
            // TODO: should probably canonicalize paths to not use backslash somewhere else
            // in the compilation pipeline...
            case '\\':
                emitRawText(context, "/");
                break;
            }
        }
        emitRawText(context, "\"");
    }

    emitRawText(context, "\n");
}

// Emit a `#line` directive to the output, and also
// ensure that source location tracking information
// is correct based on the directive we just output.
static void emitLineDirectiveAndUpdateSourceLocation(
    EmitContext*        context,
    CodePosition const& sourceLocation)
{
    emitLineDirective(context, sourceLocation);
    
    context->loc.FileName = sourceLocation.FileName;
    context->loc.Line = sourceLocation.Line;
    context->loc.Col = 1;
}

static void emitLineDirectiveIfNeeded(
    EmitContext*        context,
    CodePosition const& sourceLocation)
{
    // Ignore invalid source locations
    if(sourceLocation.Line <= 0)
        return;

    // If we are currently emitting code at a source location with
    // a differnet file or line, *or* if the source location is
    // somehow later on the line than what we want to emit,
    // then we need to emit a new `#line` directive.
    if(sourceLocation.FileName != context->loc.FileName
        || sourceLocation.Line != context->loc.Line
        || sourceLocation.Col < context->loc.Col)
    {
        // Special case: if we are in the same file, and within a small number
        // of lines of the target location, then go ahead and output newlines
        // to get us caught up.
        enum { kSmallLineCount = 3 };
        auto lineDiff = sourceLocation.Line - context->loc.Line;
        if(sourceLocation.FileName == context->loc.FileName
            && sourceLocation.Line > context->loc.Line
            && lineDiff <= kSmallLineCount)
        {
            for(int ii = 0; ii < lineDiff; ++ii )
            {
                Emit(context, "\n");
            }
            assert(sourceLocation.Line == context->loc.Line);
        }
        else
        {
            // Go ahead and output a `#line` directive to get us caught up
            emitLineDirectiveAndUpdateSourceLocation(context, sourceLocation);
        }
    }

    // Now indent up to the appropriate column, so that error messages
    // that reference columns will be correct.
    //
    // TODO: This logic does not take into account whether indentation
    // came in as spaces or tabs, so there is necessarily going to be
    // coupling between how the downstream compiler counts columns,
    // and how we do.
    if(sourceLocation.Col > context->loc.Col)
    {
        int delta = sourceLocation.Col - context->loc.Col;
        for( int ii = 0; ii < delta; ++ii )
        {
            emitRawText(context, " ");
        }
        context->loc.Col = sourceLocation.Col;
    }
}

static void advanceToSourceLocation(
    EmitContext*        context,
    CodePosition const& sourceLocation)
{
    // Skip invalid locations
    if(sourceLocation.Line <= 0)
        return;

    context->needToUpdateSourceLocation = true;
    context->nextSourceLocation = sourceLocation;
}

static void flushSourceLocationChange(
    EmitContext*        context)
{
    if(!context->needToUpdateSourceLocation)
        return;

    // Note: the order matters here, because trying to update
    // the source location may involve outputting text that
    // advances the location, and outputting text is what
    // triggers this flush operation.
    context->needToUpdateSourceLocation = false;
    emitLineDirectiveIfNeeded(context, context->nextSourceLocation);
}

static void emitTokenWithLocation(EmitContext* context, Token const& token)
{
    if( token.Position.FileName.Length() != 0 )
    {
        advanceToSourceLocation(context, token.Position);
    }
    else
    {
        // If we don't have the original position info, we need to play
        // it safe and emit whitespace to line things up nicely

        if(token.flags & TokenFlag::AtStartOfLine)
            Emit(context, "\n");
        // TODO(tfoley): macro expansion can currently lead to whitespace getting dropped,
        // so we will just insert it aggressively, to play it safe.
        else //  if(token.flags & TokenFlag::AfterWhitespace)
            Emit(context, " ");
    }

    // Emit the raw textual content of the token
    emit(context, token.Content);
}

static void EmitUnparsedStmt(EmitContext* context, RefPtr<UnparsedStmt> stmt)
{
    // TODO: actually emit the tokens that made up the statement...
    Emit(context, "{\n");
    for( auto& token : stmt->tokens )
    {
        emitTokenWithLocation(context, token);
    }
    Emit(context, "}\n");
}

static void EmitStmt(EmitContext* context, RefPtr<StatementSyntaxNode> stmt)
{
    // Try to ensure that debugging can find the right location
    advanceToSourceLocation(context, stmt->Position);

    if (auto blockStmt = stmt.As<BlockStatementSyntaxNode>())
    {
        EmitBlockStmt(context, blockStmt);
        return;
    }
    else if( auto unparsedStmt = stmt.As<UnparsedStmt>() )
    {
        EmitUnparsedStmt(context, unparsedStmt);
        return;
    }
    else if (auto exprStmt = stmt.As<ExpressionStatementSyntaxNode>())
    {
        EmitExpr(context, exprStmt->Expression);
        Emit(context, ";\n");
        return;
    }
    else if (auto returnStmt = stmt.As<ReturnStatementSyntaxNode>())
    {
        Emit(context, "return");
        if (auto expr = returnStmt->Expression)
        {
            Emit(context, " ");
            EmitExpr(context, expr);
        }
        Emit(context, ";\n");
        return;
    }
    else if (auto declStmt = stmt.As<VarDeclrStatementSyntaxNode>())
    {
        EmitDecl(context, declStmt->decl);
        return;
    }
    else if (auto ifStmt = stmt.As<IfStatementSyntaxNode>())
    {
        Emit(context, "if(");
        EmitExpr(context, ifStmt->Predicate);
        Emit(context, ")\n");
        EmitBlockStmt(context, ifStmt->PositiveStatement);
        if(auto elseStmt = ifStmt->NegativeStatement)
        {
            Emit(context, "\nelse\n");
            EmitBlockStmt(context, elseStmt);
        }
        return;
    }
    else if (auto forStmt = stmt.As<ForStatementSyntaxNode>())
    {
        EmitLoopAttributes(context, forStmt);

        Emit(context, "for(");
        if (auto initStmt = forStmt->InitialStatement)
        {
            EmitStmt(context, initStmt);
        }
        else
        {
            Emit(context, ";");
        }
        if (auto testExp = forStmt->PredicateExpression)
        {
            EmitExpr(context, testExp);
        }
        Emit(context, ";");
        if (auto incrExpr = forStmt->SideEffectExpression)
        {
            EmitExpr(context, incrExpr);
        }
        Emit(context, ")\n");
        EmitBlockStmt(context, forStmt->Statement);
        return;
    }
    else if (auto whileStmt = stmt.As<WhileStatementSyntaxNode>())
    {
        EmitLoopAttributes(context, whileStmt);

        Emit(context, "while(");
        EmitExpr(context, whileStmt->Predicate);
        Emit(context, ")\n");
        EmitBlockStmt(context, whileStmt->Statement);
        return;
    }
    else if (auto doWhileStmt = stmt.As<DoWhileStatementSyntaxNode>())
    {
        EmitLoopAttributes(context, doWhileStmt);

        Emit(context, "do(");
        EmitBlockStmt(context, doWhileStmt->Statement);
        Emit(context, " while(");
        EmitExpr(context, doWhileStmt->Predicate);
        Emit(context, ")\n");
        return;
    }
    else if (auto discardStmt = stmt.As<DiscardStatementSyntaxNode>())
    {
        Emit(context, "discard;\n");
        return;
    }
    else if (auto emptyStmt = stmt.As<EmptyStatementSyntaxNode>())
    {
        return;
    }
    else if (auto switchStmt = stmt.As<SwitchStmt>())
    {
        Emit(context, "switch(");
        EmitExpr(context, switchStmt->condition);
        Emit(context, ")\n");
        EmitBlockStmt(context, switchStmt->body);
        return;
    }
    else if (auto caseStmt = stmt.As<CaseStmt>())
    {
        Emit(context, "case ");
        EmitExpr(context, caseStmt->expr);
        Emit(context, ":\n");
        return;
    }
    else if (auto defaultStmt = stmt.As<DefaultStmt>())
    {
        Emit(context, "default:{}\n");
        return;
    }
    else if (auto breakStmt = stmt.As<BreakStatementSyntaxNode>())
    {
        Emit(context, "break;\n");
        return;
    }
    else if (auto continueStmt = stmt.As<ContinueStatementSyntaxNode>())
    {
        Emit(context, "continue;\n");
        return;
    }

    throw "unimplemented";

}

// Declaration References

static void EmitVal(EmitContext* context, RefPtr<Val> val)
{
    if (auto type = val.As<ExpressionType>())
    {
        EmitType(context, type);
    }
    else if (auto intVal = val.As<IntVal>())
    {
        Emit(context, intVal);
    }
    else
    {
        // Note(tfoley): ignore unhandled cases for semantics for now...
//		assert(!"unimplemented");
    }
}

static void EmitDeclRef(EmitContext* context, DeclRef<Decl> declRef)
{
    // TODO: need to qualify a declaration name based on parent scopes/declarations

    // Emit the name for the declaration itself
    emitName(context, declRef.GetName());

    // If the declaration is nested directly in a generic, then
    // we need to output the generic arguments here
    auto parentDeclRef = declRef.GetParent();
    if (auto genericDeclRef = parentDeclRef.As<GenericDecl>())
    {
        // Only do this for declarations of appropriate flavors
        if(auto funcDeclRef = declRef.As<FunctionDeclBase>())
        {
            // Don't emit generic arguments for functions, because HLSL doesn't allow them
            return;
        }

        Substitutions* subst = declRef.substitutions.Ptr();
        Emit(context, "<");
        int argCount = subst->args.Count();
        for (int aa = 0; aa < argCount; ++aa)
        {
            if (aa != 0) Emit(context, ",");
            EmitVal(context, subst->args[aa]);
        }
        Emit(context, " >");
    }

}

// Declarations

// Emit any modifiers that should go in front of a declaration
static void EmitModifiers(EmitContext* context, RefPtr<Decl> decl)
{
    // Emit any GLSL `layout` modifiers first
    bool anyLayout = false;
    for( auto mod : decl->GetModifiersOfType<GLSLUnparsedLayoutModifier>())
    {
        if(!anyLayout)
        {
            Emit(context, "layout(");
            anyLayout = true;
        }
        else
        {
            Emit(context, ", ");
        }

        emit(context, mod->nameToken.Content);
        if(mod->valToken.Type != TokenType::Unknown)
        {
            Emit(context, " = ");
            emit(context, mod->valToken.Content);
        }
    }
    if(anyLayout)
    {
        Emit(context, ")\n");
    }

    for (auto mod = decl->modifiers.first; mod; mod = mod->next)
    {
        advanceToSourceLocation(context, mod->Position);

        if (0) {}

        #define CASE(TYPE, KEYWORD) \
            else if(auto mod_##TYPE = mod.As<TYPE>()) Emit(context, #KEYWORD " ")

        CASE(RowMajorLayoutModifier, row_major);
        CASE(ColumnMajorLayoutModifier, column_major);
        CASE(HLSLNoInterpolationModifier, nointerpolation);
        CASE(HLSLPreciseModifier, precise);
        CASE(HLSLEffectSharedModifier, shared);
        CASE(HLSLGroupSharedModifier, groupshared);
        CASE(HLSLStaticModifier, static);
        CASE(HLSLUniformModifier, uniform);
        CASE(HLSLVolatileModifier, volatile);

        CASE(InOutModifier, inout);
        CASE(InModifier, in);
        CASE(OutModifier, out);

        CASE(HLSLPointModifier, point);
        CASE(HLSLLineModifier, line);
        CASE(HLSLTriangleModifier, triangle);
        CASE(HLSLLineAdjModifier, lineadj);
        CASE(HLSLTriangleAdjModifier, triangleadj);

        CASE(HLSLLinearModifier, linear);
        CASE(HLSLSampleModifier, sample);
        CASE(HLSLCentroidModifier, centroid);

        CASE(ConstModifier, const);

        #undef CASE

        // TODO: eventually we should be checked these modifiers, but for
        // now we can emit them unchecked, I guess
        else if (auto uncheckedAttr = mod.As<HLSLAttribute>())
        {
            Emit(context, "[");
            emit(context, uncheckedAttr->nameToken.Content);
            auto& args = uncheckedAttr->args;
            auto argCount = args.Count();
            if (argCount != 0)
            {
                Emit(context, "(");
                for (int aa = 0; aa < argCount; ++aa)
                {
                    if (aa != 0) Emit(context, ", ");
                    EmitExpr(context, args[aa]);
                }
                Emit(context, ")");
            }
            Emit(context, "]");
        }

        else if(auto simpleModifier = mod.As<SimpleModifier>())
        {
            emit(context, simpleModifier->nameToken.Content);
            Emit(context, " ");
        }

        else
        {
            // skip any extra modifiers
        }
    }
}


typedef unsigned int ESemanticMask;
enum
{
    kESemanticMask_None = 0,

    kESemanticMask_NoPackOffset = 1 << 0,

    kESemanticMask_Default = kESemanticMask_NoPackOffset,
};

static void EmitSemantic(EmitContext* context, RefPtr<HLSLSemantic> semantic, ESemanticMask /*mask*/)
{
    if (auto simple = semantic.As<HLSLSimpleSemantic>())
    {
        Emit(context, ": ");
        emit(context, simple->name.Content);
    }
    else if(auto registerSemantic = semantic.As<HLSLRegisterSemantic>())
    {
        // Don't print out semantic from the user, since we are going to print the same thing our own way...
#if 0
        Emit(context, ": register(");
        Emit(context, registerSemantic->registerName.Content);
        if(registerSemantic->componentMask.Type != TokenType::Unknown)
        {
            Emit(context, ".");
            Emit(context, registerSemantic->componentMask.Content);
        }
        Emit(context, ")");
#endif
    }
    else if(auto packOffsetSemantic = semantic.As<HLSLPackOffsetSemantic>())
    {
        // Don't print out semantic from the user, since we are going to print the same thing our own way...
#if 0
        if(mask & kESemanticMask_NoPackOffset)
            return;

        Emit(context, ": packoffset(");
        Emit(context, packOffsetSemantic->registerName.Content);
        if(packOffsetSemantic->componentMask.Type != TokenType::Unknown)
        {
            Emit(context, ".");
            Emit(context, packOffsetSemantic->componentMask.Content);
        }
        Emit(context, ")");
#endif
    }
    else
    {
        assert(!"unimplemented");
    }
}


static void EmitSemantics(EmitContext* context, RefPtr<Decl> decl, ESemanticMask mask = kESemanticMask_Default )
{
    // Don't emit semantics if we aren't translating down to HLSL
    switch (context->target)
    {
    case CodeGenTarget::HLSL:
        break;

    default:
        return;
    }

    for (auto mod = decl->modifiers.first; mod; mod = mod->next)
    {
        auto semantic = mod.As<HLSLSemantic>();
        if (!semantic)
            continue;

        EmitSemantic(context, semantic, mask);
    }
}

static void EmitDeclsInContainer(EmitContext* context, RefPtr<ContainerDecl> container)
{
    for (auto member : container->Members)
    {
        EmitDecl(context, member);
    }
}

static void EmitDeclsInContainerUsingLayout(
    EmitContext*                context,
    RefPtr<ContainerDecl>       container,
    RefPtr<StructTypeLayout>    containerLayout)
{
    for (auto member : container->Members)
    {
        RefPtr<VarLayout> memberLayout;
        if( containerLayout->mapVarToLayout.TryGetValue(member.Ptr(), memberLayout) )
        {
            EmitDeclUsingLayout(context, member, memberLayout);
        }
        else
        {
            // No layout for this decl
            EmitDecl(context, member);
        }
    }
}

static void EmitTypeDefDecl(EmitContext* context, RefPtr<TypeDefDecl> decl)
{
    // TODO(tfoley): check if current compilation target even supports typedefs

    Emit(context, "typedef ");
    EmitType(context, decl->Type, decl->Name.Content);
    Emit(context, ";\n");
}

static void EmitStructDecl(EmitContext* context, RefPtr<StructSyntaxNode> decl)
{
    // Don't emit a declaration that was only generated implicitly, for
    // the purposes of semantic checking.
    if(decl->HasModifier<ImplicitParameterBlockElementTypeModifier>())
        return;

    Emit(context, "struct ");
    emitName(context, decl->Name);
    Emit(context, "\n{\n");

    // TODO(tfoley): Need to hoist members functions, etc. out to global scope
    EmitDeclsInContainer(context, decl);

    Emit(context, "};\n");
}

// Shared emit logic for variable declarations (used for parameters, locals, globals, fields)
static void EmitVarDeclCommon(EmitContext* context, DeclRef<VarDeclBase> declRef)
{
    EmitModifiers(context, declRef.getDecl());

    EmitType(context, GetType(declRef), declRef.getDecl()->getNameToken());

    EmitSemantics(context, declRef.getDecl());

    // TODO(tfoley): technically have to apply substitution here too...
    if (auto initExpr = declRef.getDecl()->Expr)
    {
        Emit(context, " = ");
        EmitExpr(context, initExpr);
    }
}

// Shared emit logic for variable declarations (used for parameters, locals, globals, fields)
static void EmitVarDeclCommon(EmitContext* context, RefPtr<VarDeclBase> decl)
{
    EmitVarDeclCommon(context, DeclRef<Decl>(decl.Ptr(), nullptr).As<VarDeclBase>());
}

// Emit a single `regsiter` semantic, as appropriate for a given resource-type-specific layout info
static void emitHLSLRegisterSemantic(
    EmitContext*                    context,
    VarLayout::ResourceInfo const&  info)
{
    if( info.kind == LayoutResourceKind::Uniform )
    {
        size_t offset = info.index;

        // The HLSL `c` register space is logically grouped in 16-byte registers,
        // while we try to traffic in byte offsets. That means we need to pick
        // a register number, based on the starting offset in 16-byte register
        // units, and then a "component" within that register, based on 4-byte
        // offsets from there. We cannot support more fine-grained offsets than that.

        Emit(context, ": packoffset(c");

        // Size of a logical `c` register in bytes
        auto registerSize = 16;

        // Size of each component of a logical `c` register, in bytes
        auto componentSize = 4;

        size_t startRegister = offset / registerSize;
        Emit(context, int(startRegister));

        size_t byteOffsetInRegister = offset % registerSize;

        // If this field doesn't start on an even register boundary,
        // then we need to emit additional information to pick the
        // right component to start from
        if (byteOffsetInRegister != 0)
        {
            // The value had better occupy a whole number of components.
            assert(byteOffsetInRegister % componentSize == 0);

            size_t startComponent = byteOffsetInRegister / componentSize;

            static const char* kComponentNames[] = {"x", "y", "z", "w"};
            Emit(context, ".");
            Emit(context, kComponentNames[startComponent]);
        }
        Emit(context, ")");
    }
    else
    {
        Emit(context, ": register(");
        switch( info.kind )
        {
        case LayoutResourceKind::ConstantBuffer:
            Emit(context, "b");
            break;
        case LayoutResourceKind::ShaderResource:
            Emit(context, "t");
            break;
        case LayoutResourceKind::UnorderedAccess:
            Emit(context, "u");
            break;
        case LayoutResourceKind::SamplerState:
            Emit(context, "s");
            break;
        default:
            assert(!"unexpected");
            break;
        }
        Emit(context, info.index);
        if(info.space)
        {
            Emit(context, ", space");
            Emit(context, info.space);
        }
        Emit(context, ")");
    }
}

// Emit all the `register` semantics that are appropriate for a particular variable layout
static void emitHLSLRegisterSemantics(
    EmitContext*        context,
    RefPtr<VarLayout>   layout)
{
    if (!layout) return;

    switch( context->target )
    {
    default:
        return;

    case CodeGenTarget::HLSL:
        break;
    }

    for( auto rr : layout->resourceInfos )
    {
        emitHLSLRegisterSemantic(context, rr);
    }
}

static void emitHLSLParameterBlockDecl(
    EmitContext*                    context,
    RefPtr<VarDeclBase>             varDecl,
    RefPtr<ParameterBlockType>      parameterBlockType,
    RefPtr<VarLayout>               layout)
{
    // The data type that describes where stuff in the constant buffer should go
    RefPtr<ExpressionType> dataType = parameterBlockType->elementType;

    // We expect/require the data type to be a user-defined `struct` type
    auto declRefType = dataType->As<DeclRefType>();
    assert(declRefType);

    // We expect to always have layout information
    assert(layout);

    // We expect the layout to be for a structured type...
    RefPtr<ParameterBlockTypeLayout> bufferLayout = layout->typeLayout.As<ParameterBlockTypeLayout>();
    assert(bufferLayout);

    RefPtr<StructTypeLayout> structTypeLayout = bufferLayout->elementTypeLayout.As<StructTypeLayout>();
    assert(structTypeLayout);

    if( auto constantBufferType = parameterBlockType->As<ConstantBufferType>() )
    {
        Emit(context, "cbuffer ");
    }
    else if( auto textureBufferType = parameterBlockType->As<TextureBufferType>() )
    {
        Emit(context, "tbuffer ");
    }

    if( auto reflectionNameModifier = varDecl->FindModifier<ParameterBlockReflectionName>() )
    {
        Emit(context, " ");
        emitName(context, reflectionNameModifier->nameToken);
    }

    EmitSemantics(context, varDecl, kESemanticMask_None);

    auto info = layout->FindResourceInfo(LayoutResourceKind::ConstantBuffer);
    assert(info);
    emitHLSLRegisterSemantic(context, *info);

    Emit(context, "\n{\n");
    if (auto structRef = declRefType->declRef.As<StructSyntaxNode>())
    {
        for (auto field : getMembersOfType<StructField>(structRef))
        {
            EmitVarDeclCommon(context, field);

            RefPtr<VarLayout> fieldLayout;
            structTypeLayout->mapVarToLayout.TryGetValue(field.getDecl(), fieldLayout);
            assert(fieldLayout);

            // Emit explicit layout annotations for every field
            for( auto rr : fieldLayout->resourceInfos )
            {
                auto kind = rr.kind;

                auto offsetResource = rr;

                if(kind != LayoutResourceKind::Uniform)
                {
                    // Add the base index from the cbuffer into the index of the field
                    //
                    // TODO(tfoley): consider maybe not doing this, since it actually
                    // complicates logic around constant buffers...

                    // If the member of the cbuffer uses a resource, it had better
                    // appear as part of the cubffer layout as well.
                    auto cbufferResource = layout->FindResourceInfo(kind);
                    assert(cbufferResource);

                    offsetResource.index += cbufferResource->index;
                    offsetResource.space += cbufferResource->space;
                }

                emitHLSLRegisterSemantic(context, offsetResource);
            }

            Emit(context, ";\n");
        }
    }
    Emit(context, "}\n");
}

static void
emitGLSLLayoutQualifier(
    EmitContext*                    context,
    VarLayout::ResourceInfo const&  info)
{
    switch(info.kind)
    {
    case LayoutResourceKind::Uniform:
        Emit(context, "layout(offset = ");
        Emit(context, info.index);
        Emit(context, ")\n");
        break;

    case LayoutResourceKind::VertexInput:
    case LayoutResourceKind::FragmentOutput:
        Emit(context, "layout(location = ");
        Emit(context, info.index);
        Emit(context, ")\n");
        break;

    case LayoutResourceKind::SpecializationConstant:
        Emit(context, "layout(constant_id = ");
        Emit(context, info.index);
        Emit(context, ")\n");
        break;

    case LayoutResourceKind::ConstantBuffer:
    case LayoutResourceKind::ShaderResource:
    case LayoutResourceKind::UnorderedAccess:
    case LayoutResourceKind::SamplerState:
    case LayoutResourceKind::DescriptorTableSlot:
        Emit(context, "layout(binding = ");
        Emit(context, info.index);
        if(info.space)
        {
            Emit(context, ", set = ");
            Emit(context, info.space);
        }
        Emit(context, ")\n");
        break;
    }
}

static void
emitGLSLLayoutQualifiers(
    EmitContext*                    context,
    RefPtr<VarLayout>               layout)
{
    if(!layout) return;

    switch( context->target )
    {
    default:
        return;

    case CodeGenTarget::GLSL:
        break;
    }

    for( auto info : layout->resourceInfos )
    {
        emitGLSLLayoutQualifier(context, info);
    }
}

static void emitGLSLParameterBlockDecl(
    EmitContext*                    context,
    RefPtr<VarDeclBase>             varDecl,
    RefPtr<ParameterBlockType>      parameterBlockType,
    RefPtr<VarLayout>               layout)
{
    // The data type that describes where stuff in the constant buffer should go
    RefPtr<ExpressionType> dataType = parameterBlockType->elementType;

    // We expect/require the data type to be a user-defined `struct` type
    auto declRefType = dataType->As<DeclRefType>();
    assert(declRefType);

    // We expect to always have layout information
    assert(layout);

    // We expect the layout to be for a structured type...
    RefPtr<ParameterBlockTypeLayout> bufferLayout = layout->typeLayout.As<ParameterBlockTypeLayout>();
    assert(bufferLayout);

    RefPtr<StructTypeLayout> structTypeLayout = bufferLayout->elementTypeLayout.As<StructTypeLayout>();
    assert(structTypeLayout);

    emitGLSLLayoutQualifiers(context, layout);

    EmitModifiers(context, varDecl);

    // Emit an apprpriate declaration keyword based on the kind of block
    if (parameterBlockType->As<ConstantBufferType>())
    {
        Emit(context, "uniform");
    }
    else if (parameterBlockType->As<GLSLInputParameterBlockType>())
    {
        Emit(context, "in");
    }
    else if (parameterBlockType->As<GLSLOutputParameterBlockType>())
    {
        Emit(context, "out");
    }
    else if (parameterBlockType->As<GLSLShaderStorageBufferType>())
    {
        Emit(context, "buffer");
    }
    else
    {
        assert(!"unexpected");
        Emit(context, "uniform");
    }

    if( auto reflectionNameModifier = varDecl->FindModifier<ParameterBlockReflectionName>() )
    {
        Emit(context, " ");
        emitName(context, reflectionNameModifier->nameToken);
    }

    Emit(context, "\n{\n");
    if (auto structRef = declRefType->declRef.As<StructSyntaxNode>())
    {
        for (auto field : getMembersOfType<StructField>(structRef))
        {
            RefPtr<VarLayout> fieldLayout;
            structTypeLayout->mapVarToLayout.TryGetValue(field.getDecl(), fieldLayout);
            assert(fieldLayout);

            // TODO(tfoley): We may want to emit *some* of these,
            // some of the time...
//            emitGLSLLayoutQualifiers(context, fieldLayout);

            EmitVarDeclCommon(context, field);

            Emit(context, ";\n");
        }
    }
    Emit(context, "}");

    if( varDecl->Name.Type != TokenType::Unknown )
    {
        Emit(context, " ");
        emitName(context, varDecl->Name);
    }

    Emit(context, ";\n");
}

static void emitParameterBlockDecl(
    EmitContext*				context,
    RefPtr<VarDeclBase>			varDecl,
    RefPtr<ParameterBlockType>  parameterBlockType,
    RefPtr<VarLayout>           layout)
{
    switch(context->target)
    {
    case CodeGenTarget::HLSL:
        emitHLSLParameterBlockDecl(context, varDecl, parameterBlockType, layout);
        break;

    case CodeGenTarget::GLSL:
        emitGLSLParameterBlockDecl(context, varDecl, parameterBlockType, layout);
        break;

    default:
        assert(!"unexpected");
        break;
    }
}

static void EmitVarDecl(EmitContext* context, RefPtr<VarDeclBase> decl, RefPtr<VarLayout> layout)
{
    // As a special case, a variable using a parameter block type
    // will be translated into a declaration using the more primitive
    // language syntax.
    //
    // TODO(tfoley): Be sure to unwrap arrays here, in the GLSL case.
    //
    // TODO(tfoley): Detect cases where we need to fall back to
    // ordinary variable declaration syntax in HLSL.
    //
    // TODO(tfoley): there might be a better way to detect this, e.g.,
    // with an attribute that gets attached to the variable declaration.
    if (auto parameterBlockType = decl->Type->As<ParameterBlockType>())
    {
        emitParameterBlockDecl(context, decl, parameterBlockType, layout);
        return;
    }

    emitGLSLLayoutQualifiers(context, layout);

    EmitVarDeclCommon(context, decl);

    emitHLSLRegisterSemantics(context, layout);

    Emit(context, ";\n");
}

static void EmitParamDecl(EmitContext* context, RefPtr<ParameterSyntaxNode> decl)
{
    EmitVarDeclCommon(context, decl);
}

static void EmitFuncDecl(EmitContext* context, RefPtr<FunctionSyntaxNode> decl)
{
    EmitModifiers(context, decl);

    // TODO: if a function returns an array type, or something similar that
    // isn't allowed by declarator syntax and/or language rules, we could
    // hypothetically wrap things in a `typedef` and work around it.

    EmitType(context, decl->ReturnType, decl->Name);

    Emit(context, "(");
    bool first = true;
    for (auto paramDecl : decl->getMembersOfType<ParameterSyntaxNode>())
    {
        if (!first) Emit(context, ", ");
        EmitParamDecl(context, paramDecl);
        first = false;
    }
    Emit(context, ")");

    EmitSemantics(context, decl);

    if (auto bodyStmt = decl->Body)
    {
        EmitBlockStmt(context, bodyStmt);
    }
    else
    {
        Emit(context, ";\n");
    }
}

static void emitGLSLPreprocessorDirectives(
    EmitContext*                context,
    RefPtr<ProgramSyntaxNode>   program)
{
    switch(context->target)
    {
    // Don't emit this stuff unless we are targetting GLSL
    default:
        return;

    case CodeGenTarget::GLSL:
        break;
    }

    if( auto versionDirective = program->FindModifier<GLSLVersionDirective>() )
    {
        // TODO(tfoley): Emit an appropriate `#line` directive...

        Emit(context, "#version ");
        emit(context, versionDirective->versionNumberToken.Content);
        if(versionDirective->glslProfileToken.Type != TokenType::Unknown)
        {
            Emit(context, " ");
            emit(context, versionDirective->glslProfileToken.Content);
        }
        Emit(context, "\n");
    }
    else
    {
        // No explicit version was given (probably because we are cross-compiling).
        //
        // We need to pick an appropriate version, ideally based on the features
        // that the shader ends up using.
        //
        // For now we just fall back to a reasonably recent version.

        Emit(context, "#version 420\n");
    }

    // TODO: when cross-compiling we may need to output additional `#extension` directives
    // based on the features that we have used.

    for( auto extensionDirective :  program->GetModifiersOfType<GLSLExtensionDirective>() )
    {
        // TODO(tfoley): Emit an appropriate `#line` directive...

        Emit(context, "#extension ");
        emit(context, extensionDirective->extensionNameToken.Content);
        Emit(context, " : ");
        emit(context, extensionDirective->dispositionToken.Content);
        Emit(context, "\n");
    }

    // TODO: handle other cases...
}

static void EmitProgram(
    EmitContext*                context,
    RefPtr<ProgramSyntaxNode>   program,
    RefPtr<ProgramLayout>       programLayout)
{
    // There may be global-scope modifiers that we should emit now
    emitGLSLPreprocessorDirectives(context, program);

    switch(context->target)
    {
    case CodeGenTarget::GLSL:
        {
            // TODO(tfoley): Need a plan for how to enable/disable these as needed...
//            Emit(context, "#extension GL_GOOGLE_cpp_style_line_directive : require\n");
        }
        break;

    default:
        break;
    }


    // Layout information for the global scope is either an ordinary
    // `struct` in the common case, or a constant buffer in the case
    // where there were global-scope uniforms.
    auto globalScopeLayout = programLayout->globalScopeLayout;
    if( auto globalStructLayout = globalScopeLayout.As<StructTypeLayout>() )
    {
        context->globalStructLayout = globalStructLayout.Ptr();

        // The `struct` case is easy enough to handle: we just
        // emit all the declarations directly, using their layout
        // information as a guideline.
        EmitDeclsInContainerUsingLayout(context, program, globalStructLayout);
    }
    else if(auto globalConstantBufferLayout = globalScopeLayout.As<ParameterBlockTypeLayout>())
    {
        // TODO: the `cbuffer` case really needs to be emitted very
        // carefully, but that is beyond the scope of what a simple rewriter
        // can easily do (without semantic analysis, etc.).
        //
        // The crux of the problem is that we need to collect all the
        // global-scope uniforms (but not declarations that don't involve
        // uniform storage...) and put them in a single `cbuffer` declaration,
        // so that we can give it an explicit location. The fields in that
        // declaration might use various type declarations, so we'd really
        // need to emit all the type declarations first, and that involves
        // some large scale reorderings.
        //
        // For now we will punt and just emit the declarations normally,
        // and hope that the global-scope block (`$Globals`) gets auto-assigned
        // the same location that we manually asigned it.

        auto elementTypeLayout = globalConstantBufferLayout->elementTypeLayout;
        auto elementTypeStructLayout = elementTypeLayout.As<StructTypeLayout>();

        // We expect all constant buffers to contain `struct` types for now
        assert(elementTypeStructLayout);

        context->globalStructLayout = elementTypeStructLayout.Ptr();

        EmitDeclsInContainerUsingLayout(
            context,
            program,
            elementTypeStructLayout);
    }
    else
    {
        assert(!"unexpected");
    }
}

static void EmitDeclImpl(EmitContext* context, RefPtr<Decl> decl, RefPtr<VarLayout> layout)
{
    // Don't emit code for declarations that came from the stdlib.
    //
    // TODO(tfoley): We probably need to relax this eventually,
    // since different targets might have different sets of builtins.
    if (decl->HasModifier<FromStdLibModifier>())
        return;


    // Try to ensure that debugging can find the right location
    advanceToSourceLocation(context, decl->Position);

    if (auto typeDefDecl = decl.As<TypeDefDecl>())
    {
        EmitTypeDefDecl(context, typeDefDecl);
        return;
    }
    else if (auto structDecl = decl.As<StructSyntaxNode>())
    {
        EmitStructDecl(context, structDecl);
        return;
    }
    else if (auto varDecl = decl.As<VarDeclBase>())
    {
        EmitVarDecl(context, varDecl, layout);
        return;
    }
    else if (auto funcDecl = decl.As<FunctionSyntaxNode>())
    {
        EmitFuncDecl(context, funcDecl);
        return;
    }
    else if (auto genericDecl = decl.As<GenericDecl>())
    {
        // Don't emit generic decls directly; we will only
        // ever emit particular instantiations of them.
        return;
    }
	else if (auto classDecl = decl.As<ClassSyntaxNode>())
	{
		return;
	}
    else if( auto importDecl = decl.As<ImportDecl>())
    {
        // When in "rewriter" mode, we need to emit the code of the imported
        // module in-place at the `import` site.

        auto moduleDecl = importDecl->importedModuleDecl.Ptr();

        // We might import the same module along two different paths,
        // so we need to be careful to only emit each module once
        // per output.
        if(!context->modulesAlreadyEmitted.Contains(moduleDecl))
        {
            // Add the module to our set before emitting it, just
            // in case a circular reference would lead us to
            // infinite recursion (but that shouldn't be allowed
            // in the first place).
            context->modulesAlreadyEmitted.Add(moduleDecl);

            // TODO: do we need to modify the code generation environment at
            // all when doing this recursive emit?

            EmitDeclsInContainerUsingLayout(context, moduleDecl, context->globalStructLayout);
        }

        return;
    }
    else if( auto emptyDecl = decl.As<EmptyDecl>() )
    {
        EmitModifiers(context, emptyDecl);
        Emit(context, ";\n");
        return;
    }
    throw "unimplemented";
}

static void EmitDecl(EmitContext* context, RefPtr<Decl> decl)
{
    EmitDeclImpl(context, decl, nullptr);
}

static void EmitDeclUsingLayout(EmitContext* context, RefPtr<Decl> decl, RefPtr<VarLayout> layout)
{
    EmitDeclImpl(context, decl, layout);
}

static void EmitDecl(EmitContext* context, RefPtr<DeclBase> declBase)
{
    if( auto decl = declBase.As<Decl>() )
    {
        EmitDecl(context, decl);
    }
    else if(auto declGroup = declBase.As<DeclGroup>())
    {
        for(auto d : declGroup->decls)
            EmitDecl(context, d);
    }
    else
    {
        throw "unimplemented";
    }
}

static void registerReservedWord(
    EmitContext*    context,
    String const&   name)
{
    context->reservedWords.Add(name, name);
}

static void registerReservedWords(
    EmitContext*    context)
{
#define WORD(NAME) registerReservedWord(context, #NAME)

    switch (context->target)
    {
    case CodeGenTarget::GLSL:
        WORD(attribute);
        WORD(const);
        WORD(uniform);
        WORD(varying);
        WORD(buffer);

        WORD(shared);
        WORD(coherent);
        WORD(volatile);
        WORD(restrict);
        WORD(readonly);
        WORD(writeonly);
        WORD(atomic_unit);
        WORD(layout);
        WORD(centroid);
        WORD(flat);
        WORD(smooth);
        WORD(noperspective);
        WORD(patch);
        WORD(sample);
        WORD(break);
        WORD(continue);
        WORD(do);
        WORD(for);
        WORD(while);
        WORD(switch);
        WORD(case);
        WORD(default);
        WORD(if);
        WORD(else);
        WORD(subroutine);
        WORD(in);
        WORD(out);
        WORD(inout);
        WORD(float);
        WORD(double);
        WORD(int);
        WORD(void);
        WORD(bool);
        WORD(true);
        WORD(false);
        WORD(invariant);
        WORD(precise);
        WORD(discard);
        WORD(return);

        WORD(lowp);
        WORD(mediump);
        WORD(highp);
        WORD(precision);
        WORD(struct);
        WORD(uint);

        WORD(common);
        WORD(partition);
        WORD(active);
        WORD(asm);
        WORD(class);
        WORD(union);
        WORD(enum);
        WORD(typedef);
        WORD(template);
        WORD(this);
        WORD(resource);

        WORD(goto);
        WORD(inline);
        WORD(noinline);
        WORD(public);
        WORD(static);
        WORD(extern);
        WORD(external);
        WORD(interface);
        WORD(long);
        WORD(short);
        WORD(half);
        WORD(fixed);
        WORD(unsigned);
        WORD(superp);
        WORD(input);
        WORD(output);
        WORD(filter);
        WORD(sizeof);
        WORD(cast);
        WORD(namespace);
        WORD(using);

#define CASE(NAME) \
    WORD(NAME ## 2); WORD(NAME ## 3); WORD(NAME ## 4)

        CASE(mat);
        CASE(dmat);
        CASE(mat2x);
        CASE(mat3x);
        CASE(mat4x);
        CASE(dmat2x);
        CASE(dmat3x);
        CASE(dmat4x);
        CASE(vec);
        CASE(ivec);
        CASE(bvec);
        CASE(dvec);
        CASE(uvec);
        CASE(hvec);
        CASE(fvec);

#undef CASE

#define CASE(NAME)          \
    WORD(NAME ## 1D);       \
    WORD(NAME ## 2D);       \
    WORD(NAME ## 3D);       \
    WORD(NAME ## Cube);     \
    WORD(NAME ## 1DArray);  \
    WORD(NAME ## 2DArray);  \
    WORD(NAME ## 3DArray);  \
    WORD(NAME ## CubeArray);\
    WORD(NAME ## 2DMS);     \
    WORD(NAME ## 2DMSArray) \
    /* end */

#define CASE2(NAME)     \
    CASE(NAME);         \
    CASE(i ## NAME);    \
    CASE(u ## NAME)     \
    /* end */

    CASE2(sampler);
    CASE2(image);
    CASE2(texture);

#undef CASE2
#undef CASE
        break;

    default:
        break;
    }
}

String emitProgram(
    ProgramSyntaxNode*  program,
    ProgramLayout*      programLayout,
    CodeGenTarget       target)
{
    // TODO(tfoley): only emit symbols on-demand, as needed by a particular entry point

    EmitContext context;
    context.target = target;

    registerReservedWords(&context);

    EmitProgram(&context, program, programLayout);

    String code = context.sb.ProduceString();

    return code;

#if 0
    // HACK(tfoley): Invoke the D3D HLSL compiler on the result, to validate it

#ifdef _WIN32
    {
        HMODULE d3dCompiler = LoadLibraryA("d3dcompiler_47");
        assert(d3dCompiler);

        pD3DCompile D3DCompile_ = (pD3DCompile)GetProcAddress(d3dCompiler, "D3DCompile");
        assert(D3DCompile_);

        ID3DBlob* codeBlob;
        ID3DBlob* diagnosticsBlob;
        HRESULT hr = D3DCompile_(
            code.begin(),
            code.Length(),
            "slang",
            nullptr,
            nullptr,
            "main",
            "ps_5_0",
            0,
            0,
            &codeBlob,
            &diagnosticsBlob);
        if (codeBlob) codeBlob->Release();
        if (diagnosticsBlob)
        {
            String diagnostics = (char const*) diagnosticsBlob->GetBufferPointer();
            fprintf(stderr, "%s", diagnostics.begin());
            OutputDebugStringA(diagnostics.begin());
            diagnosticsBlob->Release();
        }
        if (FAILED(hr))
        {
            int f = 9;
        }
    }

    #include <d3dcompiler.h>
#endif
#endif

}


} // namespace Slang
