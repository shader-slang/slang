// slang-check-expr.cpp
#include "slang-check-impl.h"

// This file contains semantic-checking logic for the various
// expression types in the AST.
//
// Note that some cases of expression checking are split
// of into their own files. Notably:
//
// * `slang-check-overload.cpp` is responsible for the logic of resolving overloaded calls
//
// * `slang-check-conversion.cpp` is responsible for the logic of handling type conversion/coercion

#include "core/slang-char-util.h"
#include "core/slang-math.h"
#include "core/slang-short-dictionary.h"
#include "core/slang-string-util.h"
#include "slang-ast-decl.h"
#include "slang-ast-natural-layout.h"
#include "slang-ast-print.h"
#include "slang-ast-support-types.h"
#include "slang-ast-synthesis.h"
#include "slang-lookup-spirv.h"
#include "slang-lookup.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
/// Returns whether `referencedDecl` belongs outside the lambda's own scope and needs a closure
/// field.
///
/// Generic parameter references and globals remain directly referenceable, while a `this`
/// expression whose receiver type is a generic parameter still denotes a value that must be
/// captured. The synthesized lambda declaration never captures itself.
static bool _isDeclCapturedByLambda(
    LambdaExpr* lambdaExpr,
    LambdaDecl* lambdaDecl,
    Expr* referencedExpr,
    Decl* referencedDecl);
static VarDeclBase* _findLambdaCaptureField(SemanticsVisitor* semantics, Expr* expr);
static bool _doesSizeOfLikeExprEvaluateValue(SizeOfLikeExpr* expr);
static bool _doesIsTypeExprEvaluateValue(IsTypeExpr* expr);

/// Records the initializer and result portions of a completed local-binding chain.
///
/// Lowering evaluates each `LetExpr` initializer before its body.
/// `_setLocalBindingStorageAccess()` builds the access path from the innermost binding outward so
/// each `Sequence` node refers to the complete path of its body.
static void _setLocalBindingStorageAccess(LetExpr* binding)
{
    SLANG_RELEASE_ASSERT(binding);
    SLANG_RELEASE_ASSERT(binding->decl);
    SLANG_RELEASE_ASSERT(binding->decl->initExpr);
    SLANG_RELEASE_ASSERT(binding->body);
    if (auto innerBinding = as<LetExpr>(binding->body))
        _setLocalBindingStorageAccess(innerBinding);

    binding->abstractStorageAccess = AbstractStorageAccessPath::makeSequence(
        binding->getASTBuilder(),
        binding->decl->initExpr->abstractStorageAccess,
        binding->body->abstractStorageAccess,
        AbstractStorageSequenceResult::SecondAccess);
}

/// Attaches `expression` as the result of a detached local-binding chain.
///
/// The bindings are already linked from outermost to innermost, and the innermost body must still
/// be empty. `_wrapExpressionInLocalBindings()` attaches that body, gives every binding the result
/// type, and records the ordered storage use of each initializer followed by the result.
static Expr* _wrapExpressionInLocalBindings(
    Expr* expression,
    LetExpr* outermostBinding)
{
    if (!outermostBinding)
        return expression;

    auto binding = outermostBinding;
    auto type = expression->type;
    for (;;)
    {
        binding->type = type;
        if (auto nextBinding = as<LetExpr>(binding->body))
        {
            binding = nextBinding;
            continue;
        }

        SLANG_RELEASE_ASSERT(!binding->body);
        binding->body = expression;
        _setLocalBindingStorageAccess(outermostBinding);
        return outermostBinding;
    }
}

/// Clones the prefix of an expression-local binding chain required by `referencedDecls`.
///
/// `checkBaseForMemberExpr()` represents temporaries introduced while checking a member qualifier
/// as a chain of `LetExpr` nodes. When the selected declaration is a static member of an
/// existential's opened type, the value qualifier is otherwise unevaluated, but the selected
/// declaration reference can still depend on one of those binding declarations.
/// `_prepareSelectedDeclRefExpr()` retains the chain through the deepest such declaration before
/// attaching the selected reference as its body.
///
/// `_cloneRequiredExprLocalBindingPrefix()` gives each clone the original binding declaration and
/// initializer. Canonical semantic values name the declarations by identity.
/// `_cloneRequiredExprLocalBindingPrefix()` clones only the binding-chain topology so candidate
/// expressions do not share mutable wrappers; canonical semantic values continue to identify the
/// original declarations.
static LetExpr* _cloneRequiredExprLocalBindingPrefix(
    ASTBuilder* astBuilder,
    LetExpr* outermostBinding,
    const HashSet<Decl*>& referencedDecls)
{
    SLANG_RELEASE_ASSERT(astBuilder);
    SLANG_RELEASE_ASSERT(outermostBinding);

    LetExpr* deepestRequiredBinding = nullptr;
    auto binding = outermostBinding;
    for (;;)
    {
        SLANG_RELEASE_ASSERT(binding->decl);
        if (referencedDecls.contains(binding->decl))
            deepestRequiredBinding = binding;

        if (auto nextBinding = as<LetExpr>(binding->body))
        {
            binding = nextBinding;
            continue;
        }
        break;
    }

    if (!deepestRequiredBinding)
        return nullptr;

    LetExpr* retainedOutermostBinding = nullptr;
    LetExpr* retainedInnermostBinding = nullptr;
    for (binding = outermostBinding;;)
    {
        auto retainedBinding = astBuilder->create<LetExpr>(*binding);
        retainedBinding->body = nullptr;
        retainedBinding->type = QualType();
        retainedBinding->abstractStorageAccess = nullptr;
        if (retainedInnermostBinding)
            retainedInnermostBinding->body = retainedBinding;
        else
            retainedOutermostBinding = retainedBinding;
        retainedInnermostBinding = retainedBinding;

        if (binding == deepestRequiredBinding)
            break;
        binding = as<LetExpr>(binding->body);
        SLANG_RELEASE_ASSERT(binding);
    }

    SLANG_RELEASE_ASSERT(retainedOutermostBinding);
    SLANG_RELEASE_ASSERT(retainedInnermostBinding);
    return retainedOutermostBinding;
}

/// Gives a selected static-member reference the qualifier bindings required by its declaration.
///
/// Consider `makeExistential().StaticRequirement()`. Checking the qualifier opens its existential
/// value under a `LetExpr` chain. Selecting a static requirement does not evaluate the value merely
/// as a receiver, but the requirement's declaration reference can name the opened type or witness.
/// `qualifierBindings` is the expression-local binding prefix captured before
/// `ConstructLookupResultExpr()` replays the lookup breadcrumbs around the qualifier.
/// `_prepareSelectedDeclRefExpr()` clones that prefix through the last binding named by the selected
/// declaration reference. The resulting `StaticMemberExpr` contains only the qualifier's semantic
/// type; `_prepareSelectedDeclRefExpr()` does not retain the unevaluated value expression as hidden
/// provenance on that leaf.
static Expr* _prepareSelectedDeclRefExpr(
    SemanticsVisitor* semantics,
    DeclRefExpr* selectedExpr,
    LetExpr* qualifierBindings)
{
    SLANG_RELEASE_ASSERT(semantics);
    auto astBuilder = semantics->getASTBuilder();
    SLANG_RELEASE_ASSERT(astBuilder);
    SLANG_RELEASE_ASSERT(selectedExpr);

    auto staticMemberExpr = as<StaticMemberExpr>(selectedExpr);
    if (!staticMemberExpr)
        return selectedExpr;

    auto originalBaseExpr = staticMemberExpr->baseExpression;
    SLANG_RELEASE_ASSERT(originalBaseExpr);

    LetExpr* selectedBindings = nullptr;
    if (qualifierBindings)
    {
        auto dependencySource = selectedExpr->declRef.declRefBase;
        if (auto genericDeclRef = selectedExpr->declRef.as<GenericDecl>())
        {
            // A `GenericDecl` contains only the parameter list;
            // `checkGenericAppWithCheckedArgs()` later selects its inner callable.
            // `_prepareSelectedDeclRefExpr` inspects the inner declaration reference here because
            // `getMemberDeclRef()` applies the enclosing member substitutions, which can contain an
            // existential type or witness derived from the qualifier. Later argument checking can
            // add dependencies, but every dependency on the qualifier's expression-local bindings
            // remains reachable through those enclosing member substitutions.
            auto innerDecl = getInner(genericDeclRef);
            SLANG_RELEASE_ASSERT(innerDecl);
            dependencySource =
                astBuilder->getMemberDeclRef(genericDeclRef, innerDecl).declRefBase;
        }
        HashSet<Decl*> referencedDecls;
        collectReferencedDecls(semantics, dependencySource, referencedDecls);
        selectedBindings = _cloneRequiredExprLocalBindingPrefix(
            astBuilder,
            qualifierBindings,
            referencedDecls);
    }

    auto typeType = as<TypeType>(originalBaseExpr->type->getCanonicalType());
    SLANG_RELEASE_ASSERT(typeType);

    auto semanticBaseExpr = astBuilder->create<SharedTypeExpr>();
    semanticBaseExpr->base.type = typeType->getType();
    semanticBaseExpr->type = originalBaseExpr->type;
    semanticBaseExpr->loc = originalBaseExpr->loc;
    semanticBaseExpr->checked = true;
    staticMemberExpr->baseExpression = semanticBaseExpr;

    return _wrapExpressionInLocalBindings(staticMemberExpr, selectedBindings);
}

DeclRefType* SemanticsVisitor::getExprDeclRefType(Expr* expr)
{
    if (auto typetype = as<TypeType>(expr->type))
        return dynamicCast<DeclRefType>(typetype->getType());
    else
        return as<DeclRefType>(expr->type);
}

void SemanticsContext::ExprLocalScope::addBinding(LetExpr* binding)
{
    if (!m_innerMostBinding)
    {
        SLANG_ASSERT(!m_outerMostBinding);

        // If we haven't added any bindings, then `binding`
        // becomes both the inner-most and outer most.
        //
        m_innerMostBinding = binding;
        m_outerMostBinding = binding;
    }
    else
    {
        SLANG_ASSERT(m_outerMostBinding);

        // If we already have bindings, then `binding`
        // will become the new inner-most binding.
        //
        m_innerMostBinding->body = binding;
        m_innerMostBinding = binding;
    }
}


/// Move `expr` into a temporary variable and execute `func` on that variable.
///
/// Returns an expression that wraps both the creation and initialization of
/// the temporary, and the computation created by `func`.
///
template<typename F>
Expr* SemanticsVisitor::moveTemp(Expr* const& expr, F const& func)
{
    VarDecl* varDecl = m_astBuilder->create<VarDecl>();
    varDecl->parentDecl = nullptr;
    if (m_outerScope && m_outerScope->containerDecl)
        m_outerScope->containerDecl->addMember(varDecl);
    addModifier(varDecl, m_astBuilder->create<LocalTempVarModifier>());
    varDecl->checkState = DeclCheckState::DefinitionChecked;
    varDecl->nameAndLoc.loc = expr->loc;
    varDecl->initExpr = expr;
    varDecl->type.type = expr->type.type;

    auto varDeclRef = makeDeclRef(varDecl);

    LetExpr* letExpr = m_astBuilder->create<LetExpr>();
    letExpr->decl = varDecl;

    auto body = func(varDeclRef);
    Expr* result = body;
    if (auto exprLocalScope = getExprLocalScope())
    {
        // We want to add the `LetExpr` to the set of such expressions
        // in the local scope, so that it can be emitted properly.
        //
        exprLocalScope->addBinding(letExpr);
    }
    else
    {
        // If we somehow got in here and there wasn't an expression-local
        // scope established yet, it almost certainly represents an error.
        //
        SLANG_ASSERT(exprLocalScope);

        // As a fallback, though, we will try to wire up the `letExpr`
        // to surround the body directly and return that.
        //
        letExpr->body = body;
        letExpr->type = body->type;
        _setLocalBindingStorageAccess(letExpr);

        result = letExpr;
    }
    return result;
}

/// Calls `func` with a stable variable that contains the value of `expr`.
///
/// An immutable local, or a mutable local that has not been reassigned, already provides the
/// stable identity needed when an existential is opened more than once. Reusing that declaration
/// ensures that associated types such as `obj.Element` use the same `ExtractExistentialType`.
/// When `maybeMoveTemp` reuses a mutable local, it records `ExistentialOpenedOnVarModifier`.
/// `checkAssignWithCheckedOperands` records `VarReassignedModifier` after a later assignment, so
/// subsequent openings create a temporary with `moveTemp()` instead.
/// See https://github.com/shader-slang/slang/issues/10004.
///
/// When `openExistential()` calls `maybeMoveTemp()` for a lambda capture,
/// `_findLambdaCaptureField()` returns the closure field already assigned to `expr`. Without
/// reusing that field, each selected use of the same captured interface value would open through a
/// distinct temporary declaration and produce incompatible associated types. Every other
/// expression is evaluated once into a new temporary with `moveTemp()`.
///
template<typename F>
Expr* SemanticsVisitor::maybeMoveTemp(Expr* const& expr, F const& func)
{
    if (auto captureField = _findLambdaCaptureField(this, expr))
        return func(makeDeclRef(captureField));

    if (auto varExpr = as<VarExpr>(expr))
    {
        auto declRef = varExpr->declRef;
        if (auto letDeclRef = declRef.as<LetDecl>())
            return func(letDeclRef);
        if (auto varDeclRef = declRef.as<VarDecl>())
        {
            auto varDecl = varDeclRef.getDecl();
            // Skip module-level / non-local vars: they can belong to shared
            // built-in modules (e.g. gl_Position in glsl.meta.slang), and
            // modifier allocations tied to this compile's ASTBuilder would
            // dangle after the compile finishes.
            if (as<ModuleDecl>(varDecl->parentDecl))
                return moveTemp(expr, func);
            if (!varDecl->hasModifier<VarReassignedModifier>())
            {
                if (!varDecl->hasModifier<ExistentialOpenedOnVarModifier>())
                    addModifier(varDecl, m_astBuilder->create<ExistentialOpenedOnVarModifier>());
                return func(varDeclRef);
            }
        }
    }

    return moveTemp(expr, func);
}

/// Return an expression that represents "opening" the existential `expr`.
///
/// The type of `expr` must be an interface type, matching `interfaceDeclRef`.
///
/// If we scope down the PL theory to just the case that Slang cares about,
/// a value of an existential type like `IMover` is a tuple of:
///
///  * a concrete type `X`
///  * a witness `w` of the fact that `X` implements `IMover`
///  * a value `v` of type `X`
///
/// "Opening" an existential value is the process of decomposing a single
/// value `e : IMover` into the pieces `X`, `w`, and `v`.
///
/// Rather than return all those pieces individually, this operation
/// returns an expression that logically corresponds to `v`: an expression
/// of type `X`, where the type carries the knowledge that `X` implements `IMover`.
///
Expr* SemanticsVisitor::openExistential(Expr* expr, DeclRef<InterfaceDecl> interfaceDeclRef)
{
    // If `expr` refers to an immutable binding,
    // then we can use it directly. If it refers
    // to an arbitrary expression or a mutable
    // binding, we will move its value into an
    // immutable temporary so that we can use
    // it directly.
    //
    return maybeMoveTemp(
        expr,
        [&](DeclRef<VarDeclBase> varDeclRef)
        {
            // The interface type stored on the `ExtractExistentialType` should
            // be the bare interface, not a `ModifiedType` wrapping it (e.g. the
            // `no_diff` modifier that `[Differentiable]` adds to non-
            // differentiable return types). Otherwise downstream consumers that
            // look at the cached "original interface type" by `DeclRefType`
            // would not recognize it as an interface.
            ExtractExistentialType* openedType = m_astBuilder->getOrCreate<ExtractExistentialType>(
                varDeclRef,
                unwrapModifiedType(expr->type.type),
                interfaceDeclRef);

            ExtractExistentialValueExpr* openedValue =
                m_astBuilder->create<ExtractExistentialValueExpr>();
            openedValue->declRef = varDeclRef;
            openedValue->type = QualType(openedType);
            openedValue->originalExpr = expr;
            openedValue->checked = true;
            openedValue->abstractStorageAccess =
                AbstractStorageAccessPath::makeOpenedExistential(m_astBuilder, expr);
            // The result of opening an existential is an l-value
            // if the original existential is an l-value.
            //
            if (expr->type.isLeftValue)
            {
                // Marking the opened value as an l-value is the easy part.
                //
                openedValue->type.isLeftValue = true;

                // The more challenging bit is that in this case the `maybeMoveTemp()`
                // operation will have copied the original existential value into
                // a temporary.
                //
                // If this expression is used in an l-value context, then we need
                // to be able to generate code to "write back" the modified value
                // (which will be of `openedType`) to the original location named
                // by `expr` (an existential for `interfaceDeclRef`).
                //
            }

            return openedValue;
        });
}

/// If `expr` has existential type, then open it.
///
/// Returns an expression that opens `expr` if it had existential type.
/// Otherwise returns `expr` itself.
///
/// See `openExistential` for a discussion of what "opening" an
/// existential-type value means.
///
Expr* SemanticsVisitor::maybeOpenExistential(Expr* expr)
{
    // Look through `ModifiedType` (e.g. an interface return wrapped in
    // `no_diff` by the `[Differentiable]` checker) so the interface check
    // below succeeds. If we miss this, member lookup never produces an
    // `ExtractExistentialValueExpr`, the IR never emits the standard
    // existential-dispatch idiom, and a raw `this_type(...)` leaks all the
    // way to codegen.
    auto exprType = unwrapModifiedType(expr->type.type);

    if (auto declRefType = as<DeclRefType>(exprType))
    {
        if (auto interfaceDeclRef = declRefType->getDeclRef().as<InterfaceDecl>())
        {
            return openExistential(expr, interfaceDeclRef);
        }
    }

    // Default: apply the callback to the original expression;
    return expr;
}

Expr* SemanticsVisitor::maybeOpenRef(Expr* expr)
{
    auto exprType = expr->type.type;

    if (auto refType = as<ExplicitRefType>(exprType))
    {
        auto openRef = m_astBuilder->create<OpenRefExpr>();
        openRef->innerExpr = expr;

        // TODO(tfoley): The `QualType` constructor has its own
        // logic to determine the value category (e.g., whether
        // or not something is an l-value) when it is passed
        // a `Ref` type. It is unclear whether both this code
        // *and* that code are required, or if we can consolidate
        // the two.
        //
        // Note that here we change the actual `Type*` stored in
        // the `QualType` to be the underlying value type of the
        // reference, whereas the `QualType` constructor does not
        // perform such unwrapping.
        //
        openRef->type = QualType(refType);
        openRef->type.type = refType->getValueType();

        openRef->checked = true;
        openRef->loc = expr->loc;
        openRef->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            expr->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
        return openRef;
    }
    return expr;
}

Scope* SemanticsVisitor::getScope(SyntaxNode* node)
{
    while (auto declBase = as<Decl>(node))
    {
        if (auto container = as<ContainerDecl>(node))
        {
            if (container->ownedScope)
                return container->ownedScope;
        }
        node = declBase->parentDecl;
    }
    return nullptr;
}

static SourceLoc _getMemberOpLoc(Expr* expr)
{
    if (auto m = as<MemberExpr>(expr))
        return m->memberOperatorLoc;
    if (auto m = as<StaticMemberExpr>(expr))
        return m->memberOperatorLoc;
    return SourceLoc();
}

void addSiblingScopeForContainerDecl(
    ASTBuilder* builder,
    ContainerDecl* dest,
    ContainerDecl* source)
{
    addSiblingScopeForContainerDecl(builder, dest->ownedScope, source);
}

void addSiblingScopeForContainerDecl(ASTBuilder* builder, Scope* destScope, ContainerDecl* source)
{
    auto subScope = builder->create<Scope>();
    subScope->containerDecl = source;

    subScope->nextSibling = destScope->nextSibling;
    destScope->nextSibling = subScope;
}

bool isOwnModuleOrIncludedFileScope(ContainerDecl* containerDecl, ModuleDecl* moduleDecl)
{
    // The module's own scope, or one of its own `__include`d files (a `FileDecl`
    // whose parent is this module). Everything else on the module's sibling chain is
    // excluded from re-export: a `using`-spliced namespace (lookup-local, not part of
    // the imported surface), or a *foreign* module's `FileDecl` that a plain,
    // non-`__exported` transitive `import` put on the chain (its `parentDecl` is that
    // other module — `import` is non-transitive; only `__exported import` re-exports).
    // The `parentDecl == moduleDecl` conjunct is load-bearing: any foreign module's
    // `FileDecl` that lands on this chain has its `parentDecl` pointing at that other
    // module, so it is dropped regardless of how it arrived. Dropping the conjunct
    // would re-export those foreign files and make plain `import` transitive.
    // See shader-slang/slang#11443.
    if (containerDecl == moduleDecl)
        return true;
    return as<FileDecl>(containerDecl) && containerDecl->parentDecl == moduleDecl;
}

ContainerDecl* isStaticScopeDecl(Decl* decl)
{
    if (as<NamespaceDeclBase>(decl) || as<FileDecl>(decl))
        return as<ContainerDecl>(decl);
    return nullptr;
}

void SemanticsVisitor::diagnoseDeprecatedAndRemovedDeclRefUsage(
    DeclRef<Decl> declRef,
    SourceLoc loc,
    Expr* originalExpr)
{
    // Usage attributes depend on the language version of the module being checked.
    // We first look for that module in the current lexical scope.
    ModuleDecl* moduleDecl = getModuleDecl(getOuterScope());

    // Some semantic queries run outside a lexical module scope. We then try the module
    // recorded in the visitor's shared context.
    if (!moduleDecl)
    {
        auto shared = getShared();
        // Without a shared context, we have no module language version for this check.
        if (!shared)
            return;

        auto module = shared->getModule();
        // A shared context without a module likewise has no language version to apply.
        if (!module)
            return;
        moduleDecl = module->getModuleDecl();
    }

    // If neither context provides a module declaration, this query has no language version
    // against which to check usage attributes.
    if (!moduleDecl)
        return;

    // We can check a function use once during lookup and again in
    // `CheckInvokeExprWithCheckedOperands`. We avoid a repeated diagnostic only if that
    // invocation's checked function expression refers to the same declaration.
    auto originalAppExpr = as<AppExprBase>(originalExpr);
    auto originalAppFunDecl =
        originalAppExpr ? as<DeclRefExpr>(originalAppExpr->functionExpr) : nullptr;
    if (originalAppFunDecl)
    {
        auto originalDeclRef = originalAppFunDecl->declRef;
        if (originalDeclRef)
        {
            // For `int4(int2(1, 2), 3)`, the function expression refers to the type `int4`,
            // not its constructor. A type-use check does not diagnose constructor attributes.
            // We suppress a repeat only when the actual declarations are identical.
            if (originalDeclRef.getDecl() == declRef.getDecl())
                return;
        }
    }

    // We suppress declaration-use diagnostics at the declaration's name location.
    // Synthesized constructors refer to fields at those locations, so this rule avoids
    // reporting their generated field accesses.
    if (originalExpr)
    {
        if (declRef.getDecl()->getNameLoc() == originalExpr->loc)
            return;
    }

    // We check removal first because a removed declaration requires an error, even if it is
    // also deprecated. Diagnostics receive the declaration instead of its internal identifier
    // so `printDiagnosticArg` can use the original name of a renamed shader parameter.
    if (auto removedSinceAttr = declRef.getDecl()->findModifier<RemovedSinceAttribute>())
    {
        if (moduleDecl->languageVersion >= removedSinceAttr->sinceVersion)
        {
            getSink()->diagnose(Diagnostics::RemovedUsage{
                .decl = declRef.getDecl(),
                .sinceVersion = removedSinceAttr->sinceVersion,
                .message = removedSinceAttr->message,
                .location = loc});

            return;
        }
    }

    // If the declaration has not been removed in this language version, we report deprecation
    // as a warning. The declaration formatter supplies its user-visible name here as well.
    if (auto deprecatedAttr = declRef.getDecl()->findModifier<DeprecatedAttribute>())
    {
        getSink()->diagnose(Diagnostics::DeprecatedUsage{
            .decl = declRef.getDecl(),
            .message = deprecatedAttr->message,
            .location = loc});
    }
}

static bool isMutableGLSLBufferBlockVarExpr(Expr* expr)
{
    const auto derefExpr = as<DerefExpr>(expr);
    if (!derefExpr)
        return false;

    // For SSBO arrays, derefExpr is expected to be IndexExpr instead of VarExpr
    const auto indexExpr = as<IndexExpr>(derefExpr->base);

    const auto varExpr =
        indexExpr ? as<VarExpr>(indexExpr->baseExpression) : as<VarExpr>(derefExpr->base);
    // Check the declaration type
    if (!varExpr)
        return false;

    const auto varExprType = (indexExpr ? indexExpr->type : varExpr->type)->getCanonicalType();
    const auto ssbt = as<GLSLShaderStorageBufferType>(varExprType);
    if (!ssbt)
        return false;

    // Check the modifiers on the declaration
    const auto d = varExpr->declRef.getDecl();
    auto collection = d->findModifier<MemoryQualifierSetModifier>();
    if (collection &&
        collection->getMemoryQualifierBit() & MemoryQualifierSetModifier::Flags::kReadOnly)
        return false;

    return true;
}

/// Chooses the accessor operation needed for one semantic use of abstract storage.
///
/// Complete-value operations prefer value-oriented accessors, while projections prefer a stable
/// `ref` address. When no valid accessor plan exists, the returned failure identifies the missing
/// capability that `_checkAbstractStorageAccessPlan()` diagnoses.
AbstractStorageAccessPlan getAbstractStorageAccessPlan(
    ASTBuilder* astBuilder,
    DeclRef<ContainerDecl> storageDeclRef,
    AbstractStorageUse use)
{
    SLANG_RELEASE_ASSERT(astBuilder);
    SLANG_RELEASE_ASSERT(storageDeclRef);

    DeclRef<GetterDecl> getter;
    DeclRef<SetterDecl> setter;
    DeclRef<RefAccessorDecl> refAccessor;
    for (auto accessor : getMembersOfType<AccessorDecl>(
             astBuilder,
             storageDeclRef,
             MemberFilterStyle::Instance))
    {
        if (auto getterCandidate = accessor.as<GetterDecl>())
        {
            if (!getter)
                getter = getterCandidate;
            continue;
        }
        if (auto setterCandidate = accessor.as<SetterDecl>())
        {
            if (!setter)
                setter = setterCandidate;
            continue;
        }
        if (auto refCandidate = accessor.as<RefAccessorDecl>())
        {
            if (!refAccessor)
                refAccessor = refCandidate;
        }
    }

    DeclRef<AccessorDecl> valueReader =
        getter ? getter.as<AccessorDecl>() : refAccessor.as<AccessorDecl>();

    // Accessor preference is part of the observable storage semantics: a getter and a `ref`
    // accessor are allowed to produce different values. Complete-value operations prefer the
    // value-oriented getter or setter, while operations on a projection prefer a stable address.
    // The read/write cases preserve the setter as the authoritative complete-value writer, using
    // the getter (or `ref` as its fallback) to obtain the value that will be written back.
    //
    // Storage use                 Preferred plan                 Fallback
    // Complete-value read        getter                         ref
    // Complete-value borrow      getter                         ref
    // Complete-value write       setter                         ref
    // Complete read/write        value reader + setter          ref, when there is no setter
    // Stable address             ref                            none
    // Projected write            ref                            setter
    // Projected read/write       ref                            value reader + setter
    // Projected borrow           ref                            getter
    switch (use)
    {
    case AbstractStorageUse::ReadValue:
        if (getter)
            return AbstractStorageAccessPlan::makeReadValue(getter.as<AccessorDecl>());
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        break;

    case AbstractStorageUse::WriteValue:
        if (setter)
            return AbstractStorageAccessPlan::makeWriteValue(setter);
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        break;

    case AbstractStorageUse::BorrowValue:
        if (getter)
            return AbstractStorageAccessPlan::makeReadValue(getter.as<AccessorDecl>());
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        break;

    case AbstractStorageUse::ReadWriteValue:
        if (setter)
        {
            if (valueReader)
                return AbstractStorageAccessPlan::makeReadThenWriteBack(valueReader, setter);
        }
        else if (refAccessor)
        {
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        }
        break;

    case AbstractStorageUse::RequireAddress:
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        break;

    case AbstractStorageUse::WriteProjectedValue:
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        if (setter)
            return AbstractStorageAccessPlan::makeWriteValue(setter);
        break;

    case AbstractStorageUse::ModifyProjectedValue:
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        if (setter)
        {
            if (valueReader)
                return AbstractStorageAccessPlan::makeReadThenWriteBack(valueReader, setter);
        }
        break;

    case AbstractStorageUse::BorrowProjectedValue:
        if (refAccessor)
            return AbstractStorageAccessPlan::makeUseAddress(refAccessor);
        if (getter)
            return AbstractStorageAccessPlan::makeReadValue(getter.as<AccessorDecl>());
        break;

    default:
        SLANG_UNEXPECTED("unhandled abstract-storage use");
    }

    switch (use)
    {
    case AbstractStorageUse::ReadValue:
    case AbstractStorageUse::BorrowValue:
    case AbstractStorageUse::BorrowProjectedValue:
        return AbstractStorageAccessPlan::makeInvalid(
            AbstractStorageAccessFailure::CannotRead);

    case AbstractStorageUse::WriteValue:
    case AbstractStorageUse::WriteProjectedValue:
        return AbstractStorageAccessPlan::makeInvalid(
            AbstractStorageAccessFailure::CannotWrite);

    case AbstractStorageUse::RequireAddress:
        return AbstractStorageAccessPlan::makeInvalid(
            AbstractStorageAccessFailure::CannotProvideAddress);

    case AbstractStorageUse::ReadWriteValue:
    case AbstractStorageUse::ModifyProjectedValue:
        return AbstractStorageAccessPlan::makeInvalid(
            valueReader ? AbstractStorageAccessFailure::CannotWrite
                        : AbstractStorageAccessFailure::CannotRead);

    default:
        SLANG_UNEXPECTED("unhandled abstract-storage use");
        UNREACHABLE_RETURN(AbstractStorageAccessPlan::makeInvalid(
            AbstractStorageAccessFailure::CannotRead));
    }
}

/// Returns the semantic ABI of a setter's single new-value parameter.
ParamInfo getSetterNewValueParamInfo(
    ASTBuilder* astBuilder,
    DeclRef<SetterDecl> setterDeclRef)
{
    SLANG_RELEASE_ASSERT(astBuilder);
    SLANG_RELEASE_ASSERT(setterDeclRef);
    auto parameters = getParameters(astBuilder, setterDeclRef);
    SLANG_RELEASE_ASSERT(parameters.getCount() == 1);
    auto result = getParamInfo(astBuilder, parameters[0]);
    if (result.mode != ParamPassingMode::In)
        SLANG_RELEASE_ASSERT(result.mode == ParamPassingMode::BorrowIn);
    return result;
}

/// Returns whether a property declaration provides a way to write its value.
///
/// `_isPropertyReferenceLValue()` determines the property expression's value category; it does not
/// decide whether the selected accessor can use this particular receiver. Keeping those questions
/// separate lets `validateCallableReceiver()` diagnose a mutating setter or `ref` accessor
/// precisely instead of reducing the property to a generic non-l-value first.
static bool _isPropertyReferenceLValue(
    SemanticsVisitor* semantics,
    DeclRef<PropertyDecl> propertyDeclRef,
    Expr* baseExpr,
    QualType propertyReferenceType)
{
    SLANG_RELEASE_ASSERT(semantics);
    SLANG_RELEASE_ASSERT(propertyDeclRef);
    SLANG_RELEASE_ASSERT(baseExpr);

    if (isMutableGLSLBufferBlockVarExpr(baseExpr))
    {
        if (!propertyReferenceType.hasReadOnlyOnTarget)
            return true;
    }

    auto writePlan = getAbstractStorageAccessPlan(
        semantics->getASTBuilder(),
        propertyDeclRef.as<ContainerDecl>(),
        AbstractStorageUse::WriteValue);
    switch (writePlan.getKind())
    {
    case AbstractStorageAccessPlanKind::WriteValue:
    case AbstractStorageAccessPlanKind::UseAddress:
        return true;
    case AbstractStorageAccessPlanKind::Invalid:
        return false;
    default:
        SLANG_UNEXPECTED("invalid write plan for abstract storage");
    }

    UNREACHABLE_RETURN(false);
}

/// Determines whether a checked member reference names writable storage.
///
/// Ordinary members inherit the value category of their base, while properties and mutable GLSL
/// buffer blocks use their own storage rules.
static bool _isMemberReferenceLValue(SemanticsVisitor* semantics, MemberExpr* expr)
{
    SLANG_RELEASE_ASSERT(semantics);
    SLANG_RELEASE_ASSERT(expr);
    SLANG_RELEASE_ASSERT(expr->baseExpression);

    if (auto propertyDeclRef = expr->declRef.as<PropertyDecl>())
    {
        return _isPropertyReferenceLValue(
            semantics,
            propertyDeclRef,
            expr->baseExpression,
            expr->type);
    }

    if (as<DerefMemberExpr>(expr))
        return expr->type.isLeftValue;
    if (expr->baseExpression->type.isLeftValue)
        return expr->type.isLeftValue;

    if (!isMutableGLSLBufferBlockVarExpr(expr->baseExpression))
        return false;
    return !expr->type.hasReadOnlyOnTarget;
}

static bool _isDeclCapturedByLambda(
    LambdaExpr* lambdaExpr,
    LambdaDecl* lambdaDecl,
    Expr* referencedExpr,
    Decl* referencedDecl)
{
    if (!lambdaExpr)
        return false;
    if (!referencedDecl)
        return false;
    if (referencedDecl == lambdaDecl)
        return false;

    if (isGenericParam(referencedDecl) && !as<ThisExpr>(referencedExpr))
        return false;

    if (as<VarDeclBase>(referencedDecl))
    {
        if (isGlobalDecl(referencedDecl))
            return false;
    }

    auto lambdaScope = lambdaExpr->paramScopeDecl;
    SLANG_RELEASE_ASSERT(lambdaScope);
    return !referencedDecl->isChildOf(lambdaScope);
}

/// Returns the outer declaration that identifies a captured local or `this` reference.
///
/// For `this`, modifiers on the receiver value type are removed before selecting its type
/// declaration.
static Decl* _getLambdaCapturedDecl(Expr* expr)
{
    if (auto varExpr = as<VarExpr>(expr))
        return as<VarDeclBase>(varExpr->declRef.getDecl());

    if (auto thisExpr = as<ThisExpr>(expr))
    {
        auto thisTypeDecl = isDeclRefTypeOf<Decl>(unwrapModifiedType(thisExpr->type.type));
        return thisTypeDecl.getDecl();
    }

    return nullptr;
}

/// Finds the current lambda's capture field in `closureThis.field`, including through parentheses.
///
/// `_findLambdaCaptureField()` looks through parentheses so that repeated existential openings
/// reuse the capture field rather than introducing expression-local temporaries with distinct
/// opened types.
static VarDeclBase* _findLambdaCaptureField(SemanticsVisitor* semantics, Expr* expr)
{
    auto captureContext = semantics->getLambdaCaptureContext();
    if (!captureContext)
        return nullptr;

    while (auto parenExpr = as<ParenExpr>(expr))
        expr = parenExpr->base;

    auto memberExpr = as<MemberExpr>(expr);
    if (!memberExpr)
        return nullptr;
    if (!as<ThisExpr>(memberExpr->baseExpression))
        return nullptr;

    auto field = as<VarDeclBase>(memberExpr->declRef.getDecl());
    if (!field)
        return nullptr;
    if (captureContext->fieldToCapturedDecl.containsKey(field))
        return field;

    return nullptr;
}

/// Returns the expression that a lambda uses to refer to an outer declaration.
///
/// Consider `let read = () => object.method().field`. `SelectedExpressionFinalizer` determines
/// whether evaluation reaches the provisional capture: selecting a static `field` leaves
/// `method()` and its qualifier unevaluated and must not capture `object`. Captured values are
/// immutable in the closure, so `_getOrCreateLambdaCaptureReference()` makes the provisional
/// reference an r-value immediately. `SelectedExpressionFinalizer::attachCaptureField` adds its
/// field to the closure only if evaluation reaches the reference.
static Expr* _getOrCreateLambdaCaptureReference(
    ASTBuilder* astBuilder,
    LambdaExpr* lambdaExpr,
    LambdaDecl* lambdaDecl,
    LambdaCaptureContext* captureContext,
    Expr* expr,
    Decl* referencedDecl)
{
    if (!_isDeclCapturedByLambda(lambdaExpr, lambdaDecl, expr, referencedDecl))
        return expr;

    SLANG_RELEASE_ASSERT(lambdaDecl);
    SLANG_RELEASE_ASSERT(captureContext);
    expr->type.isLeftValue = false;

    VarDeclBase* captureField = nullptr;
    if (!captureContext->capturedDeclToField.tryGetValue(referencedDecl, captureField))
    {
        captureField = astBuilder->create<VarDecl>();
        if (as<VarDeclBase>(referencedDecl))
        {
            captureField->nameAndLoc = referencedDecl->nameAndLoc;
        }
        else
        {
            SLANG_RELEASE_ASSERT(as<ThisExpr>(expr));
            captureField->nameAndLoc.name = astBuilder->getNamePool()->getName("$this");
            captureField->nameAndLoc.loc = expr->loc;
        }
        SLANG_RELEASE_ASSERT(expr->type.type);
        captureField->type.type = expr->type.type;
        captureContext->capturedDeclToField.add(referencedDecl, captureField);
        captureContext->fieldToCapturedDecl.add(captureField, referencedDecl);
    }

    auto closureThisExpr = astBuilder->create<ThisExpr>();
    closureThisExpr->scope = lambdaDecl->ownedScope;
    closureThisExpr->type = QualType(DeclRefType::create(astBuilder, lambdaDecl));
    closureThisExpr->checked = true;

    auto resultExpr = astBuilder->create<MemberExpr>();
    resultExpr->declRef = captureField;
    resultExpr->baseExpression = closureThisExpr;
    resultExpr->type = expr->type;
    resultExpr->loc = expr->loc;
    resultExpr->type.isLeftValue = false;
    resultExpr->checked = true;
    return resultExpr;
}

static Expr* _captureThisForLambdaIfNeeded(SemanticsVisitor* semantics, ThisExpr* expr)
{
    return _getOrCreateLambdaCaptureReference(
        semantics->getASTBuilder(),
        semantics->getParentLambdaExpr(),
        semantics->getParentLambdaDecl(),
        semantics->getLambdaCaptureContext(),
        expr,
        _getLambdaCapturedDecl(expr));
}

DeclRefExpr* SemanticsVisitor::ConstructDeclRefExpr(
    DeclRef<Decl> declRef,
    Expr* baseExpr,
    Name* name,
    SourceLoc loc,
    Expr* originalExpr)
{
    // We first need the declaration's checked type and read/write qualifiers. The type query
    // also checks its header before we use declaration-specific semantic information below.
    auto type = GetTypeForDeclRef(declRef, loc);

    // Lookup can find a uniform shadow instead of the parameter the programmer declared.
    // We diagnose deprecated or removed parameter uses from that original declaration, even
    // when the expression must refer to mutable shadow storage.
    auto declRefForUsageDiagnostics = declRef;

    // We preserve an immutable alias's parameter identity in the checked expression.
    // Consider `[[vk::constant_id(7)]] uint count = 3;` followed by
    // `[numthreads(count, 1, 1)] void main() {}`. Attribute checking must receive a
    // reference to the parameter declaration that carries the specialization attribute.
    // `GetTypeForDeclRef` has checked the shadow's header before we inspect its alias flag.
    if (auto shadow = as<UniformParameterShadowVarDecl>(declRef.getDecl()))
    {
        auto parameter = shadow->uniformParameter;
        declRefForUsageDiagnostics = parameter->getDefaultDeclRef();

        // We canonicalize only immutable aliases. Mutable shadows must retain references to
        // their own declarations so lowering can access their private storage.
        if (shadow->shouldBeImmutableAlias)
        {
            // A legacy `cbuffer` shadow has the element struct type instead of the parameter's
            // `ConstantBuffer<T>` type. We retain that shadow to represent reads of the contents.
            // Other immutable aliases have the parameter's type and need no separate identity.
            if (shadow->getType()->equals(parameter->getType()))
            {
                // The parser creates both declarations at non-generic file or namespace scope,
                // so the parameter's default declaration reference preserves all needed context.
                declRef = parameter->getDefaultDeclRef();
            }
        }
    }

    // We report deprecation or removal at this use's location before constructing its
    // checked expression. For a uniform shadow, we check the parameter selected above.
    if (getSink())
        diagnoseDeprecatedAndRemovedDeclRefUsage(declRefForUsageDiagnostics, loc, originalExpr);

    // A `GenericDecl` is a parameter-list wrapper. Modifiers that determine instance or static
    // membership belong to its inner declaration, so expression construction classifies that
    // declaration instead of treating the wrapper itself as an instance member.
    auto declarationForStaticClassification = maybeGetInner(declRef.getDecl());
    SLANG_RELEASE_ASSERT(declarationForStaticClassification);

    // We now choose the expression kind. A reference without a base becomes a `VarExpr`.
    // With a base, we distinguish static member references from instance member accesses.
    if (baseExpr)
    {
        // A type-valued base, such as `Thing` in `Thing.member`, selects a static member.
        if (auto typeType = as<TypeType>(baseExpr->type->getCanonicalType()))
        {
            // Before forming the reference, we will check if the
            // member being referenced can even be used as a static
            // member, and if not we will diagnose an error.
            //
            // TODO: It is conceptually possible to allow static
            // references to many instance members, provided we
            // change the exposed type/signature.
            //
            // E.g., if we have:
            //
            //      struct Test { float getVal() { ... } }
            //
            // Then a reference to `Test.getVal` could be allowed,
            // and given a type of `(Test) -> float` to indicate
            // that it is an "unbound" instance method.
            //
            auto expr = m_astBuilder->create<StaticMemberExpr>();
            expr->loc = loc;
            expr->type = type;
            if (getSink() && !isDeclUsableAsStaticMember(declRef.getDecl()))
            {
                getSink()->diagnose(Diagnostics::StaticRefToNonStaticMember{
                    .type = typeType->getType(),
                    .member = declRef.getName(),
                    .location = loc});
                expr->type = m_astBuilder->getErrorType();
            }

            expr->baseExpression = baseExpr;
            expr->name = name;
            expr->declRef = declRef;
            expr->memberOperatorLoc = _getMemberOpLoc(originalExpr);
            if (auto propertyDeclRef = declRef.as<PropertyDecl>())
            {
                expr->abstractStorageAccess = AbstractStorageAccessPath::makeDirectStorage(
                    m_astBuilder,
                    nullptr,
                    propertyDeclRef.as<ContainerDecl>(),
                    loc);
            }
            return expr;
        }
        else if (isEffectivelyStatic(declarationForStaticClassification))
        {
            // A static declaration can also be accessed through a value, as in `value.member`.
            // We represent its base by the value's type so the resulting `StaticMemberExpr`
            // identifies the static declaration without requiring an instance value.
            auto baseExprType = baseExpr->type.type;
            SharedTypeExpr* baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
            baseTypeExpr->base.type = baseExprType;
            baseTypeExpr->type.type = m_astBuilder->getTypeType(baseExprType);
            baseTypeExpr->base.exp = baseExpr;
            auto expr = m_astBuilder->create<StaticMemberExpr>();
            expr->loc = loc;
            expr->type = type;
            expr->baseExpression = baseTypeExpr;
            expr->name = name;
            expr->declRef = declRef;
            expr->memberOperatorLoc = _getMemberOpLoc(originalExpr);
            if (auto propertyDeclRef = declRef.as<PropertyDecl>())
            {
                expr->abstractStorageAccess = AbstractStorageAccessPath::makeDirectStorage(
                    m_astBuilder,
                    nullptr,
                    propertyDeclRef.as<ContainerDecl>(),
                    loc);
            }
            return expr;
        }
        else
        {
            // The remaining case is an instance member accessed through a value. We combine
            // the member's qualifiers with the restrictions on that base value.
            auto expr = m_astBuilder->create<MemberExpr>();
            expr->loc = loc;
            expr->type = type;
            expr->baseExpression = baseExpr;
            expr->name = name;
            expr->declRef = declRef;
            expr->memberOperatorLoc = _getMemberOpLoc(originalExpr);

            // Reading a member of a write-only value would also read that base value. We
            // propagate the base's write-only restriction to the resulting member expression.
            expr->type.isWriteOnly = baseExpr->type.isWriteOnly || expr->type.isWriteOnly;

            expr->type.isLeftValue = _isMemberReferenceLValue(this, expr);
            if (auto propertyDeclRef = declRef.as<PropertyDecl>())
            {
                expr->abstractStorageAccess = AbstractStorageAccessPath::makeDirectStorage(
                    m_astBuilder,
                    baseExpr,
                    propertyDeclRef.as<ContainerDecl>(),
                    loc);
            }
            else if (declRef.as<VarDeclBase>())
            {
                expr->abstractStorageAccess = AbstractStorageAccessPath::makeProjection(
                    m_astBuilder,
                    baseExpr->abstractStorageAccess,
                    AbstractStorageProjectionMode::ReadModifyWrite);
            }
            return expr;
        }
    }
    else
    {
        // With no base, we construct a `VarExpr` using the checked declaration's qualifiers.
        auto expr = m_astBuilder->create<VarExpr>();
        expr->loc = loc;
        expr->name = name;
        expr->type = type;
        expr->declRef = declRef;
        if (auto propertyDeclRef = declRef.as<PropertyDecl>())
        {
            expr->abstractStorageAccess = AbstractStorageAccessPath::makeDirectStorage(
                m_astBuilder,
                nullptr,
                propertyDeclRef.as<ContainerDecl>(),
                loc);
        }
        // Generic applications and member expressions include tokens that are absent from
        // the resulting `VarExpr`. We retain `originalExpr` for the language server to locate
        // those tokens in the expression written by the programmer.
        if (as<GenericAppExpr>(originalExpr) || as<MemberExpr>(originalExpr) ||
            as<StaticMemberExpr>(originalExpr))
        {
            expr->originalExpr = originalExpr;
        }
        auto preparedExpr = _getOrCreateLambdaCaptureReference(
            m_astBuilder,
            m_parentLambdaExpr,
            m_parentLambdaDecl,
            m_lambdaCaptureContext,
            expr,
            as<VarDeclBase>(declRef.getDecl()));
        auto preparedDeclRefExpr = as<DeclRefExpr>(preparedExpr);
        SLANG_RELEASE_ASSERT(preparedDeclRefExpr);
        return preparedDeclRefExpr;
    }
}

Expr* SemanticsVisitor::constructDerefExpr(Expr* base, QualType elementType, SourceLoc loc)
{
    if (auto resPtrType = as<DescriptorHandleType>(base->type))
    {
        return coerceExpressionWithoutFinalization(
            CoercionSite::ExplicitCoercion,
            resPtrType->getElementType(),
            base,
            getSink());
    }

    auto derefExpr = m_astBuilder->create<DerefExpr>();
    derefExpr->loc = loc;
    derefExpr->base = base;
    derefExpr->type = QualType(elementType);
    derefExpr->checked = true;

    if (as<PtrType>(base->type))
    {
        // TODO(tfoley): It is not clear why this is being unconditionally
        // set to `true` when the `Ptr` types in the core module has an
        // `AccessQualifier` parameter that can be used to form a read-only pointer.
        //
        derefExpr->type.isLeftValue = true;
    }
    else if (as<ExplicitRefType>(base->type))
    {
        // TODO(tfoley): The code here is exploiting the ability of the
        // `QualType` constructor to compute the correct value category
        // for a reference type, so that we don't have to repeat that logic
        // here. That might not be the right place for that logic to live,
        // however, and so the code here might need updating sooner or
        // later.
        //
        derefExpr->type.isLeftValue = QualType(base->type.type).isLeftValue;
    }
    else if (isImmutableBufferType(base->type))
    {
        derefExpr->type.isLeftValue = false;
        derefExpr->type.isWriteOnly = false;
    }
    else
    {
        derefExpr->type.isLeftValue = base->type.isLeftValue;
        derefExpr->type.hasReadOnlyOnTarget = base->type.hasReadOnlyOnTarget;
        derefExpr->type.isWriteOnly = base->type.isWriteOnly;
    }

    derefExpr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
        m_astBuilder,
        base->abstractStorageAccess,
        AbstractStorageUse::ReadValue);

    return derefExpr;
}

Expr* SemanticsVisitor::ConstructDerefExpr(Expr* base, SourceLoc loc)
{
    auto elementType = getPointedToTypeIfCanImplicitDeref(base->type);
    SLANG_ASSERT(elementType);

    return constructDerefExpr(base, elementType, loc);
}

InvokeExpr* SemanticsVisitor::constructUncheckedInvokeExpr(
    Expr* callee,
    const List<Expr*>& arguments)
{
    auto result = m_astBuilder->create<InvokeExpr>();
    result->loc = callee->loc;
    result->functionExpr = callee;
    result->arguments.addRange(arguments);
    return result;
}

Expr* SemanticsVisitor::maybeUseSynthesizedDeclForLookupResult(
    LookupResultItem const& item,
    Expr* originalExpr)
{
    // If the only result from lookup is an entry in an interface decl, it could be that
    // the user is leaving out an explicit definition for the requirement and depending on
    // the compiler to synthesis the definition.
    // In this case, if the lookup is triggered from a location such that the satisfying
    // definition should be returned should it existed, we should create a placeholder decl for
    // the definition and return a reference to to newly created decl instead of the requirement
    // decl in the interface.
    switch (item.declRef.getDecl()->astNodeType)
    {
    case ASTNodeType::AssocTypeDecl:
        break;
    case ASTNodeType::FuncDecl:
        // We don't need to intercept lookup results with synthesized decls for methods,
        // because function lookups will only take place when we are checking the decl bodies.
        // At that point conformance check and synthesis is already done so they will always
        // resolve to the synthesized method.
        return nullptr;
    default:
        return nullptr;
    }

    // We need to check if the lookup should resolve to a definition in an implementation type
    // if it existed.
    // This will be the case when the lookup is initiated from the concrete implementation type
    // instead of directly from the Interface decl. The breadcrumbs of the lookup should provide
    // this information.

    // If no breadcrumbs existed, then the lookup should just resolve to the interface requirement.

    if (!item.breadcrumbs)
        return nullptr;

    // We will only ever need to synthesis a type to satisfy an associatedtype requirement.
    // In this case the lookup should have resolved to a known associatedtype decl.
    auto builtinAssocTypeAttr = item.declRef.getDecl()->findModifier<BuiltinRequirementModifier>();
    if (!builtinAssocTypeAttr)
        return nullptr;

    DeclRefType* subType = nullptr;

    // Check if we are reaching the associated type decl through inheritance from a concrete type.
    for (auto breadcrumb = item.breadcrumbs; breadcrumb; breadcrumb = breadcrumb->next)
    {
        switch (breadcrumb->getKind())
        {
        case LookupResultItem::Breadcrumb::Kind::SuperType:
            {
                auto witness = breadcrumb->getSubtypeWitness();
                if (auto subDeclRefType = as<DeclRefType>(witness->getSub()))
                {
                    if (!as<InterfaceDecl>(subDeclRefType->getDeclRef().getDecl()))
                    {
                        // Store the inner most concrete super type.
                        subType = subDeclRefType;
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    if (!subType)
        return nullptr;

    subType = as<DeclRefType>(subType->getCanonicalType());
    if (!subType)
        return nullptr;

    // Don't synthesize for generic parameters.
    auto parent = as<AggTypeDecl>(subType->getDeclRef().getDecl());
    if (!parent)
        return nullptr;

    // Don't synthesize for ThisType.
    if (as<ThisTypeDecl>(subType->getDeclRef().getDecl()))
        return nullptr;

    // If the inner most subtype is itself an associated type, then we're dealing
    // with an abstract type. There's not need to synthesize anythin at this point.
    //
    if (as<AssocTypeDecl>(subType->getDeclRef().getDecl()))
        return nullptr;

    // If we reach here, we are expecting a synthesized decl defined in `subType`.
    // Instead of returning a DeclRefExpr to the requirement decl, we synthesize a placeholder decl
    // in `subType` and return a DeclRefExpr to the synthesized decl.

    Decl* synthesizedDecl = nullptr;
    switch (builtinAssocTypeAttr->kind)
    {
    case BuiltinRequirementKind::DifferentialType:
        {
            if (!canStructBeUsedAsSelfDifferentialType(parent))
            {
                // Need to create a new struct type for the differential.
                //
                auto structDecl = m_astBuilder->create<StructDecl>();
                auto conformanceDecl = m_astBuilder->create<InheritanceDecl>();
                conformanceDecl->base.type = m_astBuilder->getDiffInterfaceType();
                structDecl->addMember(conformanceDecl);
                structDecl->parentDecl = parent;
                structDecl->ownedScope = m_astBuilder->create<Scope>();
                structDecl->ownedScope->containerDecl = structDecl;
                structDecl->ownedScope->parent = getScope(parent);

                synthesizedDecl = structDecl;
                auto typeDef = m_astBuilder->create<TypeAliasDecl>();
                typeDef->nameAndLoc.name = getName("Differential");
                typeDef->parentDecl = structDecl;
                addVisibilityModifier(structDecl, getDeclVisibility(parent));
                addVisibilityModifier(typeDef, getDeclVisibility(parent));

                auto synthDeclRef =
                    createDefaultSubstitutionsIfNeeded(m_astBuilder, this, makeDeclRef(structDecl));

                typeDef->type.type = DeclRefType::create(m_astBuilder, synthDeclRef);
                structDecl->addDirectMemberDecl(typeDef);

                synthesizedDecl->nameAndLoc.name = item.declRef.getName();
                synthesizedDecl->loc = parent->loc;
                parent->addDirectMemberDecl(synthesizedDecl);

                // Mark the newly synthesized decl as `ToBeSynthesized` so future checking can
                // differentiate it from user-provided definitions, and proceed to fill in its
                // definition.
                auto toBeSynthesized = m_astBuilder->create<ToBeSynthesizedModifier>();
                addModifier(synthesizedDecl, toBeSynthesized);
            }
            else
            {
                // There's no need for a new struct decl.
                // We can simply add a typealias to the existing concrete type.
                //
                auto typeDef = m_astBuilder->create<TypeAliasDecl>();
                typeDef->nameAndLoc.name = item.declRef.getName();

                // Compute the decl's type as if it is referred to from itself. This is important
                // because subType may have substitutions from the context it is used in, while this
                // synthesis step is local to the decl.
                //
                typeDef->type.type =
                    calcThisType(subType->getDeclRef().getDecl()->getDefaultDeclRef());

                addVisibilityModifier(typeDef, getDeclVisibility(parent));
                synthesizedDecl = parent;

                parent->addDirectMemberDecl(typeDef);

                markSelfDifferentialMembersOfType(parent, subType);
            }
            break;
        }
    default:
        return nullptr;
    }

    auto synthDeclMemberRef =
        m_astBuilder->getMemberDeclRef(subType->getDeclRef(), synthesizedDecl);
    return ConstructDeclRefExpr(
        synthDeclMemberRef,
        nullptr,
        item.declRef.getName(),
        originalExpr ? originalExpr->loc : SourceLoc(),
        originalExpr);
}

/// Returns whether `ConstructLookupResultExpr()` can omit an implicit receiver value before
/// replaying `followingBreadcrumb` for a declaration reference.
///
/// Consider `[BackwardDerivativeOf(method)]` inside the type that declares `method`. The attribute
/// needs the base-less declaration identity; an implicit `this` would instead bind a runtime
/// receiver. A following super-type step changes only the specialized declaration reference, so it
/// also needs no value. In contrast, a member or dereference step traverses runtime storage such as
/// `this.transparentField`; `ConstructLookupResultExpr()` must retain that receiver so checking the
/// reconstructed declaration reference can reject the capture.
static bool _canOmitImplicitReceiverValueForDeclarationReference(
    LookupResultItem::Breadcrumb* followingBreadcrumb)
{
    for (auto breadcrumb = followingBreadcrumb; breadcrumb; breadcrumb = breadcrumb->next)
    {
        if (breadcrumb->getKind() != LookupResultItem::Breadcrumb::Kind::SuperType)
            return false;
    }
    return true;
}

Expr* SemanticsVisitor::ConstructLookupResultExpr(
    LookupResultItem const& item,
    Expr* baseExpr,
    Name* name,
    SourceLoc loc,
    Expr* originalExpr)
{
    if (!item.declRef)
    {
        originalExpr->type = QualType(m_astBuilder->getErrorType());
        return originalExpr;
    }

    // We could be referencing a decl that will be synthesized. If so create a placeholder
    // and return a DeclRefExpr to it.
    if (auto lookupResultExpr = maybeUseSynthesizedDeclForLookupResult(item, originalExpr))
        return lookupResultExpr;

    // Capture the expression-local bindings before replaying breadcrumbs.
    // `ConstructLookupResultExpr()` turns a super-type breadcrumb into a storage-preserving cast,
    // but the selected declaration reference can still name a temporary declared by this prefix.
    LetExpr* qualifierBindings = as<LetExpr>(baseExpr);
    if (auto sharedTypeExpr = as<SharedTypeExpr>(baseExpr))
        qualifierBindings = as<LetExpr>(sharedTypeExpr->base.exp);

    // If we collected any breadcrumbs, then these represent
    // additional segments of the lookup path that we need
    // to expand here.
    auto bb = baseExpr;
    for (auto breadcrumb = item.breadcrumbs; breadcrumb; breadcrumb = breadcrumb->next)
    {
        auto baseBeforeBreadcrumb = bb;
        switch (breadcrumb->getKind())
        {
        case LookupResultItem::Breadcrumb::Kind::Member:
            bb = ConstructDeclRefExpr(
                breadcrumb->getMemberDeclRef(), bb, name, loc, originalExpr);
            break;

        case LookupResultItem::Breadcrumb::Kind::Deref:
            bb = ConstructDerefExpr(bb, loc);
            break;

        case LookupResultItem::Breadcrumb::Kind::SuperType:
            {
                // A `SuperType` step can precede a static member. In that case the selected
                // declaration reference already carries the specialization and there is no value
                // base to convert.
                if (bb)
                {
                    // `_lookUpDirectAndTransparentMembers()` recorded the subtype witness;
                    // `ConstructLookupResultExpr()` uses it to perform the deferred value
                    // conversion.
                    auto witness = breadcrumb->getSubtypeWitness();
                    auto expr = as<CastToSuperTypeExpr>(
                        createStoragePreservingCastToSuperTypeExpr(
                            witness->getSup(),
                            bb,
                            witness));
                    SLANG_RELEASE_ASSERT(expr);

                    bb = expr;
                }
            }
            break;

        case LookupResultItem::Breadcrumb::Kind::ThisType:
            {
                // An implicit `This` base can only start a lookup path.
                SLANG_RELEASE_ASSERT(bb == nullptr);

                auto thisType = calcThisType(breadcrumb->getThisTypeDeclRef());
                auto typeExpr = m_astBuilder->create<SharedTypeExpr>();
                typeExpr->type.type = m_astBuilder->getTypeType(thisType);
                typeExpr->base.type = thisType;
                bb = typeExpr;
            }
            break;

        case LookupResultItem::Breadcrumb::Kind::ThisValue:
            {
                // An implicit `this` base can only start a lookup path.
                SLANG_RELEASE_ASSERT(bb == nullptr);

                bool omitReceiverValue =
                    m_allowStaticReferenceToNonStaticMember &&
                    _canOmitImplicitReceiverValueForDeclarationReference(breadcrumb->next);
                if (!omitReceiverValue)
                {
                    auto thisExpr = m_astBuilder->create<ThisExpr>();
                    thisExpr->loc = loc;
                    thisExpr->scope = breadcrumb->getLookupOriginScope();

                    // `_lookUpInScopes()` records only whether a receiver value is available.
                    // `checkThisExpr()` determines the reconstructed value's type and mutability.
                    bb = checkThisExpr(thisExpr);
                    bb->checked = true;
                }
            }
            break;

        default:
            SLANG_UNREACHABLE("all cases handle");
        }
        if (getShared()->isInLanguageServer())
        {
            // Only implicit breadcrumb expressions have their locations cleared. Replaying a
            // transparent `SuperType` breadcrumb can leave `bb` pointing at the programmer-written
            // base expression, whose location language-server AST lookup must retain.
            if (bb && bb != baseBeforeBreadcrumb)
            {
                bb->loc = SourceLoc();
            }
        }
    }

    auto resultExpr = _prepareSelectedDeclRefExpr(
        this,
        ConstructDeclRefExpr(item.declRef, bb, name, loc, originalExpr),
        qualifierBindings);
    // `ConstructLookupResultExpr()` does not produce an `InvokeExpr` for a property reference.
    // Register its accessors here so that lowering can later select the getter or setter without
    // losing the derivative associations needed by the enclosing differentiable function.
    if (m_parentDifferentiableAttr && item.declRef.as<PropertyDecl>())
    {
        registerAssociatedMethods(this, item.declRef);
    }
    return resultExpr;
}

void SemanticsVisitor::suggestCompletionItems(
    CompletionSuggestions::ScopeKind scopeKind,
    LookupResult const& lookupResult)
{
    auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
    suggestions.clear();
    suggestions.scopeKind = scopeKind;
    for (auto item : lookupResult)
    {
        suggestions.candidateItems.add(item);
    }
}


Expr* SemanticsVisitor::createLookupResultExpr(
    Name* name,
    LookupResult const& lookupResult,
    Expr* baseExpr,
    SourceLoc loc,
    Expr* originalExpr)
{
    if (lookupResult.isOverloaded())
    {
        auto overloadedExpr = m_astBuilder->create<OverloadedExpr>();
        overloadedExpr->name = name;
        overloadedExpr->loc = loc;
        overloadedExpr->type = QualType(m_astBuilder->getOverloadedType());
        overloadedExpr->base = baseExpr;
        overloadedExpr->lookupResult2 = lookupResult;
        overloadedExpr->originalExpr = originalExpr;
        return overloadedExpr;
    }
    else
    {
        return ConstructLookupResultExpr(lookupResult.item, baseExpr, name, loc, originalExpr);
    }
}

static DeclVisibility _getTypeVisibility(
    Type* type,
    ShortDictionary<Type*, DeclVisibility>& typeVisibilityCache)
{
    if (auto cachedVisibility = typeVisibilityCache.tryGetValue(type))
        return *cachedVisibility;

    DeclVisibility visibility = DeclVisibility::Public;
    if (auto declRefType = as<DeclRefType>(type))
    {
        visibility = getDeclVisibility(declRefType->getDeclRef().getDecl());
        auto args = findInnerMostGenericArgs(SubstitutionSet(declRefType->getDeclRef()));
        for (auto arg : args)
        {
            if (auto typeArg = as<DeclRefType>(arg))
                visibility =
                    Math::Min(visibility, _getTypeVisibility(typeArg, typeVisibilityCache));
        }
    }

    // Cache only completed results. Consider `Pair<T, T>`: both arguments point to the same
    // canonical type DAG, so the second edge should reuse the visibility collected through the
    // first instead of traversing the entire DAG again.
    typeVisibilityCache.add(type, visibility);
    return visibility;
}

DeclVisibility SemanticsVisitor::getTypeVisibility(Type* type)
{
    // ShortDictionary rather than Dictionary: most types checked here have little or no shared
    // structure, so the cache typically ends up with only a handful of entries. See the
    // ShortDictionary doc comment and #12139.
    ShortDictionary<Type*, DeclVisibility> typeVisibilityCache;
    return _getTypeVisibility(type, typeVisibilityCache);
}

bool SemanticsVisitor::isDeclVisibleFromScope(DeclRef<Decl> declRef, Scope* scope)
{
    auto visibility = getDeclVisibility(declRef.getDecl());
    if (visibility == DeclVisibility::Public)
        return true;
    if (visibility == DeclVisibility::Internal)
    {
        // Check that the decl is in the same module as the scope.
        auto declModule = getModuleDecl(declRef.getDecl());
        if (declModule == getModuleDecl(scope))
            return true;
    }
    if (visibility == DeclVisibility::Private)
    {
        // Check that the decl is in the same or parent container decl as scope.
        Decl* parentContainer = declRef.getDecl();
        for (; parentContainer; parentContainer = parentContainer->parentDecl)
        {
            if (as<AggTypeDeclBase>(parentContainer))
                break;
            if (as<NamespaceDeclBase>(parentContainer))
                break;
        }

        for (auto s = scope; s; s = s->parent)
        {
            if (s->containerDecl == parentContainer)
                return true;
        }

        auto parentAggTypeDecl = as<AggTypeDeclBase>(parentContainer);
        if (!parentAggTypeDecl)
            return false;

        // If the decl isn't in the same syntactic container as the scope, we still need to
        // consider extensions that apply to the same type.
        //
        // An `ExtensionDecl` is represented as its own `AggTypeDeclBase`, but extension members
        // are semantically members of the type being extended. For private access, that means
        // `extension S { private ... }` should be visible from `S` and from other extensions that
        // apply to `S`, even though the declaration's parent is the extension node rather than the
        // original `S` declaration.
        //
        struct ContainerTargetTypeResolver
        {
            SemanticsVisitor* visitor;
            ASTBuilder* astBuilder;

            Type* getTargetTypeForContainer(AggTypeDeclBase* containerDecl)
            {
                if (auto extensionDecl = as<ExtensionDecl>(containerDecl))
                {
                    auto extensionDeclRef =
                        visitor->getDefaultDeclRef(extensionDecl).as<ExtensionDecl>();
                    if (!extensionDeclRef)
                        return nullptr;

                    auto targetType = getTargetType(astBuilder, extensionDeclRef);
                    if (auto targetDeclRefType = as<DeclRefType>(targetType))
                    {
                        if (auto targetCallableDeclRef =
                                targetDeclRefType->getDeclRef().as<CallableDecl>())
                        {
                            // Auto-diff synthesizes derivative members as extensions on a
                            // function-as-type. When that function is a member of an aggregate,
                            // private access should still be governed by the member function's
                            // owning aggregate, not by the synthetic function type itself.
                            //
                            // The callable can itself be declared in an extension, so resolve the
                            // parent aggregate through this helper recursively rather than using
                            // the extension declaration as the semantic container.
                            if (auto parentAggTypeDecl =
                                    getParentAggTypeDeclBase(targetCallableDeclRef.getDecl()))
                            {
                                return getTargetTypeForContainer(parentAggTypeDecl);
                            }
                        }
                    }

                    return targetType;
                }

                return DeclRefType::create(astBuilder, visitor->getDefaultDeclRef(containerDecl));
            }
        };

        ContainerTargetTypeResolver targetTypeResolver = {this, m_astBuilder};

        auto privateDeclContainerType =
            targetTypeResolver.getTargetTypeForContainer(parentAggTypeDecl);
        if (!privateDeclContainerType)
            return false;

        auto doesContainerTargetTypeMatch = [&](AggTypeDeclBase* scopeAggTypeDecl) -> bool
        {
            auto scopeContainerType =
                targetTypeResolver.getTargetTypeForContainer(scopeAggTypeDecl);
            if (!scopeContainerType)
                return false;

            if (privateDeclContainerType->equals(scopeContainerType))
                return true;

            if (auto parentExtensionDecl = as<ExtensionDecl>(parentAggTypeDecl))
            {
                if (applyExtensionToType(parentExtensionDecl, scopeContainerType))
                    return true;
            }

            return false;
        };

        // Walk the lexical scope chain looking for an enclosing aggregate/extension and compare
        // its semantic type with the declaration's semantic type.
        // `applyExtensionToType` handles generic extensions whose target can be specialized to the
        // current scope's aggregate type.
        //
        for (auto s = scope; s; s = s->parent)
        {
            if (auto scopeAggTypeDecl = as<AggTypeDeclBase>(s->containerDecl))
            {
                if (doesContainerTargetTypeMatch(scopeAggTypeDecl))
                    return true;
            }
        }
        return false;
    }
    return false;
}

LookupResult SemanticsVisitor::filterLookupResultByVisibility(const LookupResult& lookupResult)
{
    if (!m_outerScope)
        return lookupResult;
    LookupResult filteredResult;
    for (auto item : lookupResult)
    {
        if (isDeclVisibleFromScope(item.declRef, m_outerScope))
            AddToLookupResult(filteredResult, item);
    }
    return filteredResult;
}

LookupResult SemanticsVisitor::filterLookupResultByVisibilityAndDiagnose(
    const LookupResult& lookupResult,
    SourceLoc loc,
    bool& outDiagnosed)
{
    outDiagnosed = false;
    auto result = filterLookupResultByVisibility(lookupResult);
    if (lookupResult.isValid() && !result.isValid())
    {
        getSink()->diagnose(Diagnostics::DeclIsNotVisible{
            .decl = lookupResult.item.declRef.getDecl(),
            .location = loc});
        outDiagnosed = true;

        if (getShared()->isInLanguageServer())
        {
            // When in language server mode, return the unfiltered result so we can still
            // provide language service around it.
            return lookupResult;
        }
    }
    return result;
}

bool SemanticsVisitor::isWitnessUncheckedOptional(SubtypeWitness* witness)
{
    auto declaredWitness = as<DeclaredSubtypeWitness>(witness);
    if (!declaredWitness)
        return false;

    auto decl = declaredWitness->getDeclRef().getDecl();
    if (!decl || !decl->hasModifier<OptionalConstraintModifier>())
        return false;

    // Okay, we've found an optional subtype witness. This result needs
    // to be removed if we're not inside a block that directly checks
    // if (sub is sup)
    auto sub = witness->getSub();
    auto sup = witness->getSup();

    for (auto outerStmtInfo = m_outerStmts; outerStmtInfo; outerStmtInfo = outerStmtInfo->next)
    {
        auto outerStmt = outerStmtInfo->stmt;
        auto ifStmt = as<IfStmt>(outerStmt);

        if (!ifStmt)
            continue;

        IsTypeExpr* isType = as<IsTypeExpr>(ifStmt->predicate);
        if (!isType)
            continue;
        VarExpr* var = as<VarExpr>(isType->value);
        if (!var)
            continue;
        TypeType* typeType = as<TypeType>(var->type);

        // var->type works for `variable is Interface`, while
        // typeType->getType() is for `T is Interface`.
        auto type = typeType ? typeType->getType() : var->type.type;
        if (type == sub && isType->typeExpr.type == sup)
        {
            return false;
        }
    }

    // If we got this far, it's both an optional witness and there's no
    // statement checking its validity.
    return true;
}

LookupResult SemanticsVisitor::filterLookupResultByCheckedOptional(const LookupResult& lookupResult)
{
    LookupResult filteredResult;
    for (auto item : lookupResult)
    {
        bool optionalConstraintsChecked = true;

        for (auto breadcrumb = item.breadcrumbs; breadcrumb; breadcrumb = breadcrumb->next)
        {
            if (breadcrumb->getKind() != LookupResultItem::Breadcrumb::Kind::SuperType)
                continue;

            auto witness = breadcrumb->getSubtypeWitness();
            if (isWitnessUncheckedOptional(witness))
            {
                optionalConstraintsChecked = false;
                break;
            }
        }

        if (optionalConstraintsChecked)
            AddToLookupResult(filteredResult, item);
    }
    return filteredResult;
}

LookupResult SemanticsVisitor::filterLookupResultByCheckedOptionalAndDiagnose(
    const LookupResult& lookupResult,
    SourceLoc loc,
    bool& outDiagnosed)
{
    auto result = filterLookupResultByCheckedOptional(lookupResult);
    if (lookupResult.isValid() && !result.isValid())
    {
        getSink()->diagnose(Diagnostics::RequiredConstraintIsNotChecked{
            .decl = lookupResult.item.declRef.getDecl(),
            .location = loc});
        outDiagnosed = true;

        if (getShared()->isInLanguageServer())
        {
            return lookupResult;
        }
    }
    return result;
}

LookupResult SemanticsVisitor::resolveOverloadedLookup(
    LookupResult const& inResult,
    Type* targetType)
{
    // If the result isn't actually overloaded, it is fine as-is
    if (!inResult.isValid())
        return inResult;
    if (!inResult.isOverloaded())
        return inResult;

    // If this is a lookup for a completion request token (to generate completion candidate list),
    // don't apply expected-type filtering on the lookup result so we can report the entire
    // candidate list in the language server.
    bool shouldSkipCoercionFilter =
        getLinkage()->contentAssistInfo.checkingMode == ContentAssistCheckingMode::Completion &&
        inResult.getName() == getSession()->getCompletionRequestTokenName();

    // We are going to build up a list of items to return.
    List<LookupResultItem> items;
    for (auto item : inResult.items)
    {
        // First we check if the item is coercible to targetType.
        // And skip if it doesn't.
        if (targetType && !shouldSkipCoercionFilter)
        {
            auto declType = GetTypeForDeclRef(item.declRef, SourceLoc());
            if (!canCoerce(targetType, declType, nullptr, nullptr))
                continue;
        }

        // For each item we consider adding, we will compare it
        // to those items we've already added.
        //
        // If any of the existing items is "better" than `item`,
        // then we will skip adding `item`.
        //
        // If `item` is "better" than any of the existing items,
        // we will remove those from `items`.
        //
        bool shouldAdd = true;
        for (Index ii = 0; ii < items.getCount(); ++ii)
        {
            int cmp = CompareLookupResultItems(item, items[ii]);
            if (cmp < 0)
            {
                // The new `item` is strictly better
                items.fastRemoveAt(ii);
                --ii;
            }
            else if (cmp > 0)
            {
                // The existing item is strictly better
                shouldAdd = false;
            }
        }
        if (shouldAdd)
        {
            items.add(item);
        }
    }

    // The resulting `items` list should be all those items
    // that were neither better nor worse than one another.
    //
    // If no candidates are passing coercion check, return unresolved lookup result
    // and leave downstream logic to diagnose and error.
    //

    if (items.getCount() == 0)
        return inResult;

    LookupResult result;
    for (auto item : items)
    {
        AddToLookupResult(result, item);
    }
    return result;
}

bool SemanticsVisitor::maybeDiagnoseAmbiguousReference(Expr* expr)
{
    if (auto overloadExpr = as<OverloadedExpr>(expr))
    {
        if (overloadExpr->lookupResult2.isValid() &&
            !as<NamespaceDecl>(overloadExpr->lookupResult2.item.declRef.getDecl()))
        {
            diagnoseAmbiguousReference(overloadExpr);
            return true;
        }
    }
    return false;
}

void SemanticsVisitor::diagnoseAmbiguousReference(
    OverloadedExpr* overloadedExpr,
    LookupResult const& lookupResult)
{
    getSink()->diagnose(Diagnostics::AmbiguousReference{
        .name = getText(lookupResult.items[0].declRef.getName()),
        .location = overloadedExpr->loc});

    for (auto item : lookupResult.items)
    {
        String declString = ASTPrinter::getDeclSignatureString(item, m_astBuilder);
        getSink()->diagnose(Diagnostics::OverloadCandidate{
            .candidate = declString,
            .location = item.declRef.getLoc()});
    }
}

void SemanticsVisitor::diagnoseAmbiguousReference(Expr* expr)
{
    if (auto overloadedExpr = as<OverloadedExpr>(expr))
    {
        diagnoseAmbiguousReference(overloadedExpr, overloadedExpr->lookupResult2);
    }
    else
    {
        getSink()->diagnose(Diagnostics::AmbiguousExpression{.expr = expr});
    }
    expr->type = m_astBuilder->getErrorType();
}

Expr* SemanticsVisitor::_resolveOverloadedExprImpl(
    OverloadedExpr* overloadedExpr,
    LookupMask mask,
    Type* targetType,
    DiagnosticSink* diagSink)
{
    auto lookupResult = overloadedExpr->lookupResult2;
    if (!lookupResult.isValid() || !lookupResult.isOverloaded())
        return overloadedExpr;

    // Take the lookup result we had, and refine it based on what is expected in context.
    //
    // E.g., if there is both a type and a variable named `Foo`, but in context we know
    // that a type is expected, then we can disambiguate by assuming the type is intended.
    //
    lookupResult = refineLookup(lookupResult, mask);

    // Try to filter out overload candidates based on which ones are "better" than one another.
    lookupResult = resolveOverloadedLookup(lookupResult, targetType);

    if (!lookupResult.isValid())
    {
        // If we didn't find any symbols after filtering, then just
        // use the original and report errors that way
        return overloadedExpr;
    }

    if (!lookupResult.isOverloaded())
    {
        // If there is only a single item left in the lookup result,
        // then we can proceed to use that item alone as the resolved
        // expression.
        //
        return ConstructLookupResultExpr(
            lookupResult.item,
            overloadedExpr->base,
            overloadedExpr->name,
            overloadedExpr->loc,
            overloadedExpr);
    }

    // Otherwise, we weren't able to resolve the overloading given
    // the information available in context.
    //
    // If the client is asking for us to emit diagnostics about
    // this fact, we should do so here:
    //
    if (diagSink)
    {
        diagnoseAmbiguousReference(overloadedExpr, lookupResult);

        // TODO(tfoley): should we construct a new ErrorExpr here?
        return CreateErrorExpr(overloadedExpr);
    }
    else
    {
        // If the client isn't trying to *force* overload resolution
        // to complete just yet (e.g., they are just trying out one
        // candidate for an overloaded call site), then we return
        // the input expression as-is.
        //
        return overloadedExpr;
    }
}

Expr* SemanticsVisitor::maybeResolveOverloadedExpr(
    Expr* expr,
    LookupMask mask,
    Type* targetType,
    DiagnosticSink* diagSink)
{
    if (IsErrorExpr(expr))
        return expr;

    if (auto overloadedExpr = as<OverloadedExpr>(expr))
    {
        return _resolveOverloadedExprImpl(overloadedExpr, mask, targetType, diagSink);
    }
    else
    {
        return expr;
    }
}

Expr* SemanticsVisitor::resolveOverloadedExpr(
    OverloadedExpr* overloadedExpr,
    Type* targetType,
    LookupMask mask)
{
    return _resolveOverloadedExprImpl(overloadedExpr, mask, targetType, getSink());
}

Type* SemanticsVisitor::tryGetDifferentialValueType(ASTBuilder* builder, Type* type)
{
    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto builtinRequirement =
                declRefType->getDeclRef().getDecl()->findModifier<BuiltinRequirementModifier>())
        {
            if (builtinRequirement->kind == BuiltinRequirementKind::DifferentialType ||
                builtinRequirement->kind == BuiltinRequirementKind::DifferentialPtrType)
            {
                // We are trying to get differential type from a differential type.
                // The result is itself.
                return type;
            }
        }
    }

    type = resolveType(type);
    auto witness = as<SubtypeWitness>(
        tryGetInterfaceConformanceWitness(type, builder->getDifferentiableInterfaceType()));

    if (witness)
    {
        // If we're dealing with an interface type that requires conformance to one of the
        // differentiable type interfaces, then we'll use the differentiable type interface
        // itself as the derivative type.
        //
        if (auto declRefType = as<DeclRefType>(type))
            if (auto interfaceDeclRef = declRefType->getDeclRef().as<InterfaceDecl>())
                return witness->getSup();

        auto diffTypeLookupResult = lookUpMember(
            getASTBuilder(),
            this,
            getName("Differential"),
            type,
            nullptr,
            Slang::LookupMask::type,
            Slang::LookupOptions::None);

        diffTypeLookupResult = resolveOverloadedLookup(diffTypeLookupResult, nullptr);

        if (!diffTypeLookupResult.isValid())
        {
            return nullptr;
        }
        else if (diffTypeLookupResult.isOverloaded())
        {
            return nullptr;
        }
        else
        {
            SharedTypeExpr* baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
            baseTypeExpr->base.type = type;
            baseTypeExpr->type.type = m_astBuilder->getTypeType(type);

            NameLoc nameLoc = NameLoc();
            if (auto declRefType = as<DeclRefType>(type))
            {
                nameLoc.name = declRefType->getDeclRef().getName();
                nameLoc.loc = declRefType->getDeclRef().getLoc();
            }

            auto diffTypeExpr = ConstructLookupResultExpr(
                diffTypeLookupResult.item,
                baseTypeExpr,
                nameLoc.name,
                nameLoc.loc,
                baseTypeExpr);

            return resolveType(ExtractTypeFromTypeRepr(diffTypeExpr));
        }
    }
    return nullptr;
}

Type* SemanticsVisitor::tryGetDifferentialType(ASTBuilder* builder, Type* type)
{
    if (auto ptrType = as<PtrTypeBase>(type))
    {
        if (as<SubtypeWitness>(tryGetInterfaceConformanceWitness(
                type,
                builder->getDifferentiableRefInterfaceType())))
        {
            auto diffTypeLookupResult = lookUpMember(
                getASTBuilder(),
                this,
                getName("Differential"),
                type,
                nullptr,
                Slang::LookupMask::type,
                Slang::LookupOptions::NoDeref);

            diffTypeLookupResult = resolveOverloadedLookup(diffTypeLookupResult, nullptr);

            if (diffTypeLookupResult.isValid())
            {
                SharedTypeExpr* baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
                baseTypeExpr->base.type = type;
                baseTypeExpr->type.type = m_astBuilder->getTypeType(type);

                auto diffTypeExpr = ConstructLookupResultExpr(
                    diffTypeLookupResult.item,
                    baseTypeExpr,
                    ptrType->getDeclRef().getName(),
                    ptrType->getDeclRef().getLoc(),
                    baseTypeExpr);

                return resolveType(ExtractTypeFromTypeRepr(diffTypeExpr));
            }
        }

        auto baseDiffType = tryGetDifferentialType(builder, ptrType->getValueType());
        if (!baseDiffType)
            return nullptr;
        return builder->getPtrType(baseDiffType, ptrType->getClass().getName());
    }
    else if (auto arrayType = as<ArrayExpressionType>(type))
    {
        auto baseDiffType = tryGetDifferentialType(builder, arrayType->getElementType());
        if (!baseDiffType)
            return nullptr;
        return builder->getArrayType(baseDiffType, arrayType->getElementCount());
    }

    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto builtinRequirement =
                declRefType->getDeclRef().getDecl()->findModifier<BuiltinRequirementModifier>())
        {
            if (builtinRequirement->kind == BuiltinRequirementKind::DifferentialType ||
                builtinRequirement->kind == BuiltinRequirementKind::DifferentialPtrType)
            {
                // We are trying to get differential type from a differential type.
                // The result is itself.
                return type;
            }
        }
        type = resolveType(type);
        auto witness = as<SubtypeWitness>(
            tryGetInterfaceConformanceWitness(type, builder->getDifferentiableInterfaceType()));
        if (!witness)
            witness = as<SubtypeWitness>(tryGetInterfaceConformanceWitness(
                type,
                builder->getDifferentiableRefInterfaceType()));
        if (witness)
        {
            // If we're dealing with an interface type that requires conformance to one of the
            // differentiable type interfaces, then we'll use the differentiable type interface
            // itself as the derivative type.
            //
            if (auto innerDeclRefType = as<DeclRefType>(type))
                if (auto interfaceDeclRef = innerDeclRefType->getDeclRef().as<InterfaceDecl>())
                    return witness->getSup();

            auto diffTypeLookupResult = lookUpMember(
                getASTBuilder(),
                this,
                getName("Differential"),
                type,
                nullptr,
                Slang::LookupMask::type,
                Slang::LookupOptions::None);

            diffTypeLookupResult = resolveOverloadedLookup(diffTypeLookupResult, nullptr);

            if (!diffTypeLookupResult.isValid())
            {
                return nullptr;
            }
            else if (diffTypeLookupResult.isOverloaded())
            {
                return nullptr;
            }
            else
            {
                SharedTypeExpr* baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
                baseTypeExpr->base.type = type;
                baseTypeExpr->type.type = m_astBuilder->getTypeType(type);

                auto diffTypeExpr = ConstructLookupResultExpr(
                    diffTypeLookupResult.item,
                    baseTypeExpr,
                    declRefType->getDeclRef().getName(),
                    declRefType->getDeclRef().getLoc(),
                    baseTypeExpr);

                return resolveType(ExtractTypeFromTypeRepr(diffTypeExpr));
            }
        }
    }

    if (auto typePack = as<ConcreteTypePack>(type))
    {
        bool anyDifferentiableElement = false;
        List<Type*> diffTypes;
        for (Index i = 0; i < typePack->getTypeCount(); i++)
        {
            auto t = typePack->getElementType(i);
            auto diffType = tryGetDifferentialType(builder, t);
            if (!diffType)
                diffType = m_astBuilder->getVoidType();
            else
                anyDifferentiableElement = true;
            diffTypes.add(diffType);
        }
        if (anyDifferentiableElement)
            return builder->getTypePack(diffTypes.getArrayView());
    }
    return nullptr;
}

bool SemanticsVisitor::canStructBeUsedAsSelfDifferentialType(AggTypeDecl* aggTypeDecl)
{
    // A struct can be used as its own differential type if all its members are differentiable, and
    // none of the member is decorated with "no_diff", and their differential types are the same as
    // the original types.
    //
    bool canBeUsed = true;
    for (auto varDecl : aggTypeDecl->getDirectMemberDeclsOfType<VarDecl>())
    {
        // Try to get the differential type of the member.
        Type* diffType = tryGetDifferentialType(getASTBuilder(), varDecl->getType());
        if (!diffType || !diffType->equals(varDecl->getType()) ||
            varDecl->findModifier<NoDiffModifier>())
        {
            canBeUsed = false;
            break;
        }
    }
    return canBeUsed;
}

void SemanticsVisitor::markSelfDifferentialMembersOfType(AggTypeDecl* parent, Type* type)
{
    // TODO: Handle extensions.
    // Add derivative member attributes to all the fields pointing to themselves.
    for (auto member : parent->getMembersOfType<VarDeclBase>())
    {
        auto derivativeMemberModifier = m_astBuilder->create<DerivativeMemberAttribute>();
        auto fieldLookupExpr = m_astBuilder->create<StaticMemberExpr>();
        fieldLookupExpr->type.type = member->getType();

        auto baseTypeExpr = m_astBuilder->create<SharedTypeExpr>();
        baseTypeExpr->base.type = type;
        auto baseTypeType = m_astBuilder->getOrCreate<TypeType>(type);
        baseTypeExpr->type.type = baseTypeType;
        fieldLookupExpr->baseExpression = baseTypeExpr;

        fieldLookupExpr->declRef = makeDeclRef(member);

        derivativeMemberModifier->memberDeclRef = fieldLookupExpr;
        addModifier(member, derivativeMemberModifier);
    }
}

void SemanticsVisitor::checkDerivativeMemberAttributeReferences(
    VarDeclBase* varDecl,
    DerivativeMemberAttribute* derivativeMemberAttr)
{
    if (derivativeMemberAttr->memberDeclRef)
    {
        // Already checked! This usually happens if this attribute is synthesized by the compiler.
        return;
    }

    SLANG_ASSERT(derivativeMemberAttr->args.getCount() == 1);
    auto checkedExpr =
        dispatchExpr(derivativeMemberAttr->args[0], allowStaticReferenceToNonStaticMember());

    auto memberType = varDecl->type.type; // All types must be fully checked by now.
    auto diffType = getDifferentialType(m_astBuilder, memberType, varDecl->loc);
    auto thisType = calcThisType(makeDeclRef(varDecl->parentDecl));
    if (!thisType)
        return; // Diagnostic should have been emitted previously.

    auto diffThisType = getDifferentialType(m_astBuilder, thisType, derivativeMemberAttr->loc);
    if (!diffThisType)
        return; // Diagnostic should have been emitted previously.

    if (auto declRefExpr = as<DeclRefExpr>(checkedExpr))
    {
        derivativeMemberAttr->memberDeclRef = declRefExpr;
        if (!diffType->equals(declRefExpr->type))
        {
            getSink()->diagnose(Diagnostics::TypeMismatch{
                .expectedType = diffType,
                .actualType = declRefExpr->type,
                .expr = declRefExpr});
        }
        if (!varDecl->parentDecl)
        {
            getSink()->diagnose(Diagnostics::AttributeNotApplicable{
                .attrName = derivativeMemberAttr->getKeywordName(),
                .attr = derivativeMemberAttr});
        }
        if (auto memberExpr = as<StaticMemberExpr>(declRefExpr))
        {
            auto baseExprType = memberExpr->baseExpression->type.type;
            if (auto typeType = as<TypeType>(baseExprType))
            {
                if (diffThisType->equals(typeType->getType()))
                {
                    return;
                }
            }
        }
    }
    getSink()->diagnose(
        Diagnostics::DerivativeMemberAttributeMustNameAMemberInExpectedDifferentialType{
            .diffType = diffThisType,
            .attr = derivativeMemberAttr->loc});
}

Type* SemanticsVisitor::getDifferentialType(ASTBuilder* builder, Type* type, SourceLoc loc)
{
    auto result = tryGetDifferentialType(builder, type);
    if (!result)
    {
        getSink()->diagnose(Diagnostics::TypeDoesntImplementInterfaceRequirement{
            .type = type,
            .member = "Differential",
            .location = loc});
        return m_astBuilder->getErrorType();
    }
    return result;
}


void SemanticsVisitor::maybeRegisterDifferentiableType(
    ASTBuilder* builder,
    Type* type,
    SourceLoc diagnosticLoc)
{
    if (!builder->isDifferentiableInterfaceAvailable())
    {
        return;
    }

    if (!m_parentDifferentiableAttr)
    {
        return;
    }

    if (diagnosticLoc.isValid())
    {
        m_parentDifferentiableAttr->m_typeRegistrationDiagnosticLoc = diagnosticLoc;
    }

    maybeRegisterDifferentiableTypeImplRecursive(builder, type);
}


Val* maybeRegisterVal(
    SemanticsVisitor* visitor,
    Type* baseForLookup,
    Val* baseForRegistry,
    Name* name,
    AnnotationKind kind)
{
    auto lookupResult = lookUpMember(
        visitor->getASTBuilder(),
        visitor,
        name,
        baseForLookup,
        visitor->getOuterScope(),
        LookupMask::Default);
    lookupResult = visitor->resolveOverloadedLookup(lookupResult);

    if (!lookupResult.isOverloaded() && lookupResult.isValid())
    {
        visitor->getParentDifferentiableAttribute()->addAssocVal(
            baseForRegistry,
            (SlangInt)kind,
            lookupResult.item.declRef);
        return (Val*)lookupResult.item.declRef;
    }

    return nullptr;
};

Val* maybeRegisterWitness(
    SemanticsVisitor* visitor,
    Type* baseForLookup,
    Val* baseForRegistry,
    Type* superType,
    AnnotationKind kind)
{
    if (auto witness = visitor->tryGetSubtypeWitness(baseForLookup, superType))
    {
        visitor->getParentDifferentiableAttribute()->addAssocVal(
            baseForRegistry,
            (SlangInt)kind,
            witness);
        return (Val*)witness;
    }

    return nullptr;
}

void SemanticsVisitor::maybeRegisterDifferentiableTypeImplRecursive(ASTBuilder* builder, Type* type)
{
    // Recursively visit the tree of type and register all differentiable types along the way.

    if (as<TypeType>(type))
        return;
    if (!type)
        return;

    if (m_parentDifferentiableAttr->m_typeRegistrationRecursionDepth >= kMaxTypeNestingDepth)
    {
        if (!m_parentDifferentiableAttr->m_typeRegistrationDepthExceeded && getSink())
        {
            Diagnostics::MaximumTypeNestingLevelExceeded diag = {};
            diag.location = m_parentDifferentiableAttr->m_typeRegistrationDiagnosticLoc.isValid()
                                ? m_parentDifferentiableAttr->m_typeRegistrationDiagnosticLoc
                                : m_parentDifferentiableAttr->loc;
            getSink()->diagnose(diag);
            m_parentDifferentiableAttr->m_typeRegistrationDepthExceeded = true;
        }
        return;
    }
    m_parentDifferentiableAttr->m_typeRegistrationRecursionDepth++;
    SLANG_DEFER(m_parentDifferentiableAttr->m_typeRegistrationRecursionDepth--);

    // Have we already registered this type? If so we can exit now.
    if (m_parentDifferentiableAttr->m_typeRegistrationWorkingSet.contains(type))
        return;

    m_parentDifferentiableAttr->m_typeRegistrationWorkingSet.add(type);

    // Check for special cases such as PtrTypeBase<T> or Array<T>
    // This could potentially be handled later by simply defining extensions
    // for Ptr<T:IDifferentiable> etc..
    //
    if (auto ptrType = as<PtrTypeBase>(type))
    {
        maybeRegisterDifferentiableTypeImplRecursive(builder, ptrType->getValueType());
        return;
    }

    if (auto arrayType = as<ArrayExpressionType>(type))
    {
        maybeRegisterDifferentiableTypeImplRecursive(builder, arrayType->getElementType());
        // Fall through to register the array type itself.
    }

    if (auto matrixType = as<MatrixExpressionType>(type))
    {
        // Matrix types are lowered using sub-vector (row) types, so we need to
        // register the row vector type as well.
        maybeRegisterDifferentiableTypeImplRecursive(builder, matrixType->getRowType());
        // Fall through to register the matrix type itself.
    }

    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); i++)
            maybeRegisterDifferentiableTypeImplRecursive(builder, typePack->getElementType(i));
        return;
    }

    if (auto declRefType = as<DeclRefType>(type))
    {
        // TODO: Why? If an agg-type's member is accessed in the function, it should
        // come with an expr that uses that member, and that expr's type should be registered
        // separately.
        //
        if (auto aggTypeDeclRef = declRefType->getDeclRef().as<AggTypeDecl>())
        {
            foreachDirectOrExtensionMemberOfType<InheritanceDecl>(
                this,
                aggTypeDeclRef,
                [&](DeclRef<InheritanceDecl> member)
                {
                    auto subType = DeclRefType::create(m_astBuilder, member);
                    maybeRegisterDifferentiableTypeImplRecursive(m_astBuilder, subType);
                });
            foreachDirectOrExtensionMemberOfType<VarDeclBase>(
                this,
                aggTypeDeclRef,
                [&](DeclRef<VarDeclBase> member)
                {
                    auto fieldType = getType(m_astBuilder, member);
                    maybeRegisterDifferentiableTypeImplRecursive(m_astBuilder, fieldType);
                });
        }
        SubstitutionSet(declRefType->getDeclRef())
            .forEachSubstitutionArg(
                [&](Val* arg)
                {
                    if (auto typeArg = as<Type>(arg))
                    {
                        maybeRegisterDifferentiableTypeImplRecursive(m_astBuilder, typeArg);
                    }
                });
    }

    bool hasDiffValueConformance = false;
    if (isDeclRefTypeOf<InterfaceDecl>(type))
    {
        // Existential types. There's not a proper way to represent
        // the differential of an existential type (yet).
        //
        // We'll 'cheat' a bit and register it as IDifferentiable.
        //
        // The backend will treat it as if its a tuple of existentials.
        //
        if (auto witness = tryGetInterfaceConformanceWitness(
                type,
                getASTBuilder()->getDifferentiableInterfaceType()))
        {
            this->getParentDifferentiableAttribute()->addAssocVal(
                type,
                (SlangInt)AnnotationKind::DifferentialPairType,
                getCurrentASTBuilder()->getDifferentialPairType(type, witness));
            this->getParentDifferentiableAttribute()->addAssocVal(
                type,
                (SlangInt)AnnotationKind::DifferentialType,
                getCurrentASTBuilder()->getDifferentiableInterfaceType());
            // Leave the rest unregistered for now. The backend will take care of it.
        }
        else if (tryGetInterfaceConformanceWitness(
                     type,
                     getASTBuilder()->getDifferentiableRefInterfaceType()))
        {
            // Unsupported at the moment.
            SLANG_UNEXPECTED("existential differentiable pointer types not supported");
        }
    }
    else if (isAbstractTypePack(type))
    {
        // For now, skip registering this.
        //
        // TODO: We will need to handle this properly, or there will be
        // corner cases that crash (e.g. when a type is passed in within a pack from a
        // non-differentiable context)
        //
    }
    else
    {
        // Lower associated information for a regular type.

        if (auto witness = tryGetInterfaceConformanceWitness(
                type,
                getASTBuilder()->getDifferentiableInterfaceType()))
        {
            hasDiffValueConformance = true;
            maybeRegisterVal(
                this,
                type,
                type,
                getName("Differential"),
                AnnotationKind::DifferentialType);
            this->getParentDifferentiableAttribute()->addAssocVal(
                type,
                (SlangInt)AnnotationKind::DifferentialPairType,
                getCurrentASTBuilder()->getDifferentialPairType(type, witness));
            maybeRegisterVal(this, type, type, getName("dzero"), AnnotationKind::DifferentialZero);
            maybeRegisterVal(this, type, type, getName("dadd"), AnnotationKind::DifferentialAdd);
        }

        if (auto witness = tryGetInterfaceConformanceWitness(
                type,
                getASTBuilder()->getDifferentiableRefInterfaceType()))
        {
            if (hasDiffValueConformance)
            {
                if (auto declRefType = as<DeclRefType>(type))
                    getSink()->diagnose(
                        Diagnostics::TypeCannotConformToBothValueAndPointerDiffInterfaces{
                            .type = type,
                            .decl = declRefType->getDeclRef().getDecl()});
                return;
            }
            maybeRegisterVal(
                this,
                type,
                type,
                getName("Differential"),
                AnnotationKind::DifferentialPtrType);
            this->getParentDifferentiableAttribute()->addAssocVal(
                type,
                (SlangInt)AnnotationKind::DifferentialPtrPairType,
                getCurrentASTBuilder()->getDifferentialPtrPairType(type, witness));
        }
    }
}

// This checks that if a differentiable function access a non-diff type "This", in such case we
// want to provide a non-error diagnostic to the user to notify that there could be an unexpected
// behavior because every member access will not have derivative computed for it. User can use
// [NoDiffThis] to clarify that this is intended.
void SemanticsVisitor::maybeCheckMissingNoDiffThis(Expr* expr)
{
    if (auto memberExpr = as<MemberExpr>(expr))
    {
        auto thisExpr = as<ThisExpr>(memberExpr->baseExpression);
        if (thisExpr && isTypeDifferentiable(memberExpr->type.type))
        {
            auto noDiffThisAttr = this->m_parentFunc->findModifier<NoDiffThisAttribute>();
            if (isTypeDifferentiable(calcThisType(thisExpr->type.type)) ||
                (noDiffThisAttr && !noDiffThisAttr->isSynthesized))
            {
                return;
            }

            getSink()->diagnose(Diagnostics::NoDerivativeOnNonDifferentiableThisType{
                .memberDecl = memberExpr->declRef.getDecl(),
                .func = this->m_parentFunc,
                .member = memberExpr});
        }
    }
}

Expr* SemanticsVisitor::CheckTerm(Expr* term)
{
    // If we have already checked the expr, don't check again.
    if (term->checked)
    {
        return term;
    }

    auto checkedTerm = _CheckTerm(term);
    checkedTerm->checked = true;

    // Differentiable type checking.
    // TODO: This can be super slow.
    if (this->m_parentFunc && this->m_parentFunc->findModifier<DifferentiableAttribute>())
    {
        maybeRegisterDifferentiableType(getASTBuilder(), checkedTerm->type.type, checkedTerm->loc);

        if (!this->m_parentFunc->findModifier<TreatAsDifferentiableAttribute>())
        {
            maybeCheckMissingNoDiffThis(checkedTerm);
        }
    }

    return checkedTerm;
}

Expr* SemanticsVisitor::checkPotentiallyUnevaluatedTerm(Expr* term)
{
    SemanticsVisitor visitor(withPotentiallyUnevaluatedTerm());
    return visitor.CheckTerm(term);
}

/// Wraps `expression` in the temporary bindings accumulated in `localScope`.
static Expr* _wrapExpressionInLocalBindings(
    Expr* expression,
    SemanticsContext::ExprLocalScope& localScope)
{
    return _wrapExpressionInLocalBindings(expression, localScope.getOuterMostBinding());
}

Expr* SemanticsVisitor::_CheckTerm(Expr* term)
{
    if (!term)
        return nullptr;

    // The process of checking a term/expression can end up introducing
    // temporaries that need to be added to an outer scope. When jumping
    // into expression checking, we want to check if we already have such
    // a scope in place. If we do, we will re-use it for any sub-expressions.
    // If not, we need to create one.
    //
    if (getExprLocalScope())
    {
        return dispatchExpr(term, *this);
    }

    ExprLocalScope exprLocalScope;

    Expr* checkedTerm = dispatchExpr(term, withExprLocalScope(&exprLocalScope));

    if (IsErrorExpr(checkedTerm))
        return checkedTerm;

    return _wrapExpressionInLocalBindings(checkedTerm, exprLocalScope);
}

Expr* SemanticsVisitor::CreateErrorExpr(Expr* expr)
{
    if (!expr)
    {
        expr = m_astBuilder->create<IncompleteExpr>();
    }
    expr->type = QualType(m_astBuilder->getErrorType());
    return expr;
}

bool SemanticsVisitor::IsErrorExpr(Expr* expr)
{
    // TODO: we may want other cases here...

    if (const auto errorType = as<ErrorType>(expr->type); errorType)
        return true;

    return false;
}

Expr* SemanticsVisitor::GetBaseExpr(Expr* expr)
{
    if (auto parenExpr = as<ParenExpr>(expr))
    {
        return GetBaseExpr(parenExpr->base);
    }
    else if (auto memberExpr = as<MemberExpr>(expr))
    {
        return memberExpr->baseExpression;
    }
    else if (auto staticMemberExpr = as<StaticMemberExpr>(expr))
    {
        return staticMemberExpr->baseExpression;
    }
    else if (auto overloadedExpr = as<OverloadedExpr>(expr))
    {
        return overloadedExpr->base;
    }
    else if (auto overloadedExpr2 = as<OverloadedExpr2>(expr))
    {
        return overloadedExpr2->base;
    }
    else if (auto genApp = as<GenericAppExpr>(expr))
    {
        return GetBaseExpr(genApp->functionExpr);
    }
    else if (auto partiallyApplied = as<PartiallyAppliedGenericExpr>(expr))
    {
        return GetBaseExpr(partiallyApplied->baseExpr);
    }
    return nullptr;
}

DeclRefExpr* SemanticsVisitor::getDirectCallableDeclRefExpr(Expr* expr)
{
    auto declRefExpr =
        as<DeclRefExpr>(getCallableExprThroughTransparentWrappers(expr));
    if (!declRefExpr)
        return nullptr;
    if (!declRefExpr->declRef.as<CallableDecl>())
        return nullptr;
    return declRefExpr;
}

void SemanticsVisitor::replaceSelectedCallableThroughTransparentWrappers(
    Expr*& expr,
    Expr* replacement)
{
    SLANG_RELEASE_ASSERT(expr);
    SLANG_RELEASE_ASSERT(replacement);

    List<Expr*> wrappers;
    getCallableExprThroughTransparentWrappers(expr, &wrappers);
    if (wrappers.getCount() == 0)
    {
        expr = replacement;
        return;
    }

    auto innerWrapper = wrappers.getLast();
    if (auto parenExpr = as<ParenExpr>(innerWrapper))
    {
        parenExpr->base = replacement;
    }
    else if (auto genericAppExpr = as<GenericAppExpr>(innerWrapper))
    {
        genericAppExpr->functionExpr = replacement;
    }
    else
    {
        auto letExpr = as<LetExpr>(innerWrapper);
        SLANG_RELEASE_ASSERT(letExpr);
        letExpr->body = replacement;
    }

    Expr* selectedResult = replacement;
    Index wrapperIndex = wrappers.getCount();
    while (wrapperIndex)
    {
        auto wrapper = wrappers[--wrapperIndex];
        wrapper->type = selectedResult->type;
        if (auto parenExpr = as<ParenExpr>(wrapper))
        {
            SLANG_RELEASE_ASSERT(parenExpr->base == selectedResult);
            parenExpr->abstractStorageAccess = selectedResult->abstractStorageAccess;
        }
        else if (auto genericAppExpr = as<GenericAppExpr>(wrapper))
        {
            SLANG_RELEASE_ASSERT(genericAppExpr->functionExpr == selectedResult);
            genericAppExpr->abstractStorageAccess = selectedResult->abstractStorageAccess;
        }
        else
        {
            auto letExpr = as<LetExpr>(wrapper);
            SLANG_RELEASE_ASSERT(letExpr);
            SLANG_RELEASE_ASSERT(letExpr->body == selectedResult);
            _setLocalBindingStorageAccess(letExpr);
        }
        selectedResult = wrapper;
    }
    expr = selectedResult;
}

Expr* SemanticsExprVisitor::visitIncompleteExpr(IncompleteExpr* expr)
{
    expr->type = m_astBuilder->getErrorType();
    return expr;
}

Expr* SemanticsExprVisitor::visitBoolLiteralExpr(BoolLiteralExpr* expr)
{
    expr->type = m_astBuilder->getBoolType();
    return expr;
}

Expr* SemanticsExprVisitor::visitNullPtrLiteralExpr(NullPtrLiteralExpr* expr)
{
    expr->type = m_astBuilder->getNullPtrType();
    return expr;
}

Expr* SemanticsExprVisitor::visitNoneLiteralExpr(NoneLiteralExpr* expr)
{
    expr->type = m_astBuilder->getNoneType();
    return expr;
}

Expr* SemanticsExprVisitor::visitIntegerLiteralExpr(IntegerLiteralExpr* expr)
{
    // The expression might already have a type, determined by its suffix.
    // It it doesn't, we will give it a default type.
    //
    // TODO: We should be careful to pick a "big enough" type
    // based on the size of the value (e.g., don't try to stuff
    // a constant in an `int` if it requires 64 or more bits).
    //
    // The long-term solution here is to give a type to a literal
    // based on the context where it is used, but that requires
    // a more sophisticated type system than we have today.
    //
    if (!expr->type.type)
    {
        expr->type = m_astBuilder->getBuiltinType(expr->suffixType);

        // Check if we have an overflow diagnostics pending
        if (expr->signedMinimumIntException &&
            (expr->suffixType == BaseType::UInt64 || expr->suffixType == BaseType::UIntPtr) &&
            (expr->value == INT64_MIN))
        {
            getSink()->diagnose(Diagnostics::IntegerLiteralTooLarge{.location = expr->loc});
        }
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitFloatingPointLiteralExpr(FloatingPointLiteralExpr* expr)
{
    if (!expr->type.type)
    {
        expr->type = m_astBuilder->getBuiltinType(expr->suffixType);
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitStringLiteralExpr(StringLiteralExpr* expr)
{
    expr->type = m_astBuilder->getStringType();
    return expr;
}

IntVal* SemanticsVisitor::getIntVal(IntegerLiteralExpr* expr)
{
    return m_astBuilder->getIntVal(expr->type.type, expr->value);
}

IntVal* SemanticsVisitor::tryConstantFoldExpr(
    SubstExpr<InvokeExpr> invokeExpr,
    ConstantFoldingKind kind,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    // We need all the operands to the expression

    // Check if the callee is an operation that is amenable to constant-folding.
    //
    // For right now we will look for calls to intrinsic functions, and then inspect
    // their names (this is bad and slow).
    auto funcDeclRefExpr = getBaseExpr(invokeExpr).as<DeclRefExpr>();
    if (!funcDeclRefExpr)
        return nullptr;

    // The builtin-operator fast path produces a `BuiltinOperatorExpr` (folded separately by
    // `tryConstantFoldBuiltinOperatorExpr`), so anything reaching here is an ordinary call:
    // it must resolve to a decl carrying an intrinsic-op or implicit-conversion modifier.
    auto funcDeclRef = getDeclRef(m_astBuilder, funcDeclRefExpr);
    if (!funcDeclRef)
        return nullptr;
    auto intrinsicMod = funcDeclRef.getDecl()->findModifier<IntrinsicOpModifier>();
    ImplicitConversionModifier* implicitCast =
        funcDeclRef.getDecl()->findModifier<ImplicitConversionModifier>();
    if (!intrinsicMod && !implicitCast)
    {
        // We can't constant fold anything that doesn't map to a builtin
        // operation right now.
        //
        // TODO: we should really allow constant-folding for anything
        // that can be lowered to our bytecode...
        return nullptr;
    }

    // Let's not constant-fold operations with more than a certain number of arguments, for
    // simplicity
    static const int kMaxArgs = 8;
    auto argCount = getArgCount(invokeExpr);
    if (argCount > kMaxArgs)
        return nullptr;

    // Before checking the operation name, let's look at the arguments
    IntVal* argVals[kMaxArgs];
    IntegerLiteralValue constArgVals[kMaxArgs];
    bool allConst = true;
    for (Index a = 0; a < argCount; ++a)
    {
        auto argExpr = getArg(invokeExpr, a);
        auto argVal = tryFoldIntegerConstantExpression(argExpr, kind, circularityInfo);
        if (!argVal)
            return nullptr;

        argVals[a] = argVal;

        if (auto constArgVal = as<ConstantIntVal>(argVal))
        {
            constArgVals[a] = constArgVal->getValue();
        }
        else
        {
            allConst = false;
        }
    }

    if (!allConst)
    {
        // We support a very limited number of operations
        // on "constants" that aren't actually known, to be able to handle a generic
        // that takes an integer `N` but then constructs a vector of size `N+1`.
        //
        // The hard part there is implementing the rules for value unification in the
        // presence of more complicated `IntVal` subclasses, like `SumIntVal`. You'd
        // need inference to be smart enough to know that `2 + N` and `N + 2` are the
        // same value, as are `N + M + 1 + 1` and `M + 2 + N`.
        //
        // This is done by constructing a 'PolynomialIntVal' and rely on its
        // `canonicalize` operation.
        if (implicitCast)
        {
            // We cannot support casting in this case.
            return nullptr;
        }

        auto opName = funcDeclRef.getName();

        // handle binary operators
        if (opName == getName("-"))
        {
            if (argCount == 1)
            {
                return PolynomialIntVal::neg(m_astBuilder, argVals[0]);
            }
            else if (argCount == 2)
            {
                return PolynomialIntVal::sub(m_astBuilder, argVals[0], argVals[1]);
            }
        }
        else if (opName == getName("+"))
        {
            if (argCount == 1)
            {
                return argVals[0];
            }
            else if (argCount == 2)
            {
                return PolynomialIntVal::add(m_astBuilder, argVals[0], argVals[1]);
            }
        }
        else if (opName == getName("*"))
        {
            if (argCount == 2)
            {
                return PolynomialIntVal::mul(m_astBuilder, argVals[0], argVals[1]);
            }
        }
        else
        {
            // A symbolic builtin operator from a *resolved* operator call (one the fast path
            // doesn't rewrite to a `BuiltinOperatorExpr`: `?:`/`&&`/`||`, or operators on
            // operands like enums/generic `T` that aren't builtin scalar/vector/matrix) folds
            // via the decl-free `BuiltinOperationIntVal`, keyed on the operator enum, which
            // re-evaluates once its operands become concrete. This is the same representation
            // the fast path's `BuiltinOperatorExpr` folds to, so there is exactly one `IntVal`
            // form per operator regardless of which path reached it.
            auto opKind = getBuiltinOperationKindFromString(
                getText(opName).getUnownedSlice(),
                argCount == 1 ? OperatorArity::Unary : OperatorArity::Binary);
            if (opKind == BuiltinOperationKind::Unknown)
                return nullptr;
            return m_astBuilder->getOrCreate<BuiltinOperationIntVal>(
                invokeExpr.getExpr()->type.type,
                opKind,
                makeArrayView(argVals, argCount));
        }
        // A `+`/`-`/`*` with an unexpected argument count falls through to here.
        return nullptr;
    }

    // At this point, all the operands had simple integer values, so we are golden.
    IntegerLiteralValue resultValue = 0;
    // If this is an implicit cast, we can try to fold.
    if (implicitCast)
    {
        auto targetBasicType = as<BasicExpressionType>(invokeExpr.getExpr()->type.type);
        if (!targetBasicType)
            return nullptr;
        auto foldVal = as<IntVal>(
            TypeCastIntVal::tryFoldImpl(m_astBuilder, targetBasicType, argVals[0], getSink()));
        if (foldVal)
            return foldVal;
        auto result = m_astBuilder->getTypeCastIntVal(targetBasicType, argVals[0]);
        return result;
    }
    else
    {
        auto opName = funcDeclRef.getName();

        // handle unary and binary operators
        if (opName == getName("-"))
        {
            if (argCount == 1)
            {
                resultValue = -constArgVals[0];
            }
            else if (argCount == 2)
            {
                resultValue = constArgVals[0] - constArgVals[1];
            }
        }
        else if (opName == getName("+"))
        {
            if (argCount == 1)
            {
                resultValue = constArgVals[0];
            }
            else if (argCount == 2)
            {
                resultValue = static_cast<IntegerLiteralValue>(
                    static_cast<uint64_t>(constArgVals[0]) +
                    static_cast<uint64_t>(constArgVals[1]));
            }
        }
        else if (opName == getName("!"))
        {
            resultValue = constArgVals[0] == 0;
        }
        else if (opName == getName("~"))
        {
            resultValue = ~constArgVals[0];
        }
        else if (opName == getName("&&"))
        {
            if (argCount != 2)
                return nullptr;
            resultValue = (constArgVals[0] != 0) && (constArgVals[1] != 0);
        }
        else if (opName == getName("||"))
        {
            if (argCount != 2)
                return nullptr;
            resultValue = (constArgVals[0] != 0) || (constArgVals[1] != 0);
        }

        // Unsigned/bitwise operators (bit pattern preserved, uint64_t cast is correct)
#define CASE_UINT(OP)                                                                         \
    else if (opName == getName(#OP)) do                                                       \
    {                                                                                         \
        if (argCount != 2)                                                                    \
            return nullptr;                                                                   \
        resultValue =                                                                         \
            static_cast<uint64_t>(constArgVals[0]) OP static_cast<uint64_t>(constArgVals[1]); \
    }                                                                                         \
    while (0)

        CASE_UINT(*);
        CASE_UINT(&);
        CASE_UINT(|);
        CASE_UINT(^);
        CASE_UINT(!=);
        CASE_UINT(==);
#undef CASE_UINT

        // Signed comparison operators (uint64_t cast would break e.g. (-1 < 0))
        else if (opName == getName(">="))
        {
            if (argCount != 2)
                return nullptr;
            resultValue = constArgVals[0] >= constArgVals[1];
        }
        else if (opName == getName("<="))
        {
            if (argCount != 2)
                return nullptr;
            resultValue = constArgVals[0] <= constArgVals[1];
        }
        else if (opName == getName("<"))
        {
            if (argCount != 2)
                return nullptr;
            resultValue = constArgVals[0] < constArgVals[1];
        }
        else if (opName == getName(">"))
        {
            if (argCount != 2)
                return nullptr;
            resultValue = constArgVals[0] > constArgVals[1];
        }

        // Shift operators: guard negative count (UB), use modulo for width
        else if (opName == getName("<<"))
        {
            if (argCount != 2)
                return nullptr;
            if (constArgVals[1] < 0)
                return nullptr;
            const auto shiftCount =
                static_cast<std::make_unsigned_t<IRIntegerValue>>(constArgVals[1]) %
                std::numeric_limits<std::make_unsigned_t<IRIntegerValue>>::digits;
            resultValue = static_cast<IntegerLiteralValue>(
                static_cast<std::make_unsigned_t<IntegerLiteralValue>>(constArgVals[0])
                << shiftCount);
        }
        else if (opName == getName(">>"))
        {
            if (argCount != 2)
                return nullptr;
            if (constArgVals[1] < 0)
                return nullptr;
            const auto shiftCount =
                static_cast<std::make_unsigned_t<IRIntegerValue>>(constArgVals[1]) %
                std::numeric_limits<std::make_unsigned_t<IRIntegerValue>>::digits;
            resultValue = constArgVals[0] >> shiftCount;
        }
        // binary operators with chance of divide-by-zero
        // TODO: issue a suitable error in that case
#define CASE(OP)                                          \
    else if (opName == getName(#OP)) do                   \
    {                                                     \
        if (argCount != 2)                                \
            return nullptr;                               \
        if (!constArgVals[1])                             \
            return nullptr;                               \
        resultValue = constArgVals[0] OP constArgVals[1]; \
    }                                                     \
    while (0)
        CASE(/);
        CASE(%);
#undef CASE
        else if (opName == getName("?:"))
        {
            if (argCount != 3)
                return nullptr;
            if (constArgVals[0] != 0)
                resultValue = constArgVals[1];
            else
                resultValue = constArgVals[2];
        }
        // TODO(tfoley): more cases
        else
        {
            return nullptr;
        }
    }

    IntVal* result = m_astBuilder->getIntVal(invokeExpr.getExpr()->type.type, resultValue);
    return result;
}

bool SemanticsVisitor::_checkForCircularityInConstantFolding(
    DeclRefBase* declRef,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    auto decl = declRef ? declRef->getDecl() : nullptr;
    if (circularityInfo && circularityInfo->depth >= kMaxTypeNestingDepth)
    {
        Diagnostics::GenericEvaluationRecursionLimitExceeded diag = {};
        diag.decl = decl;
        diag.budget = int(kMaxTypeNestingDepth);
        getSink()->diagnose(diag);
        return true;
    }

    for (auto info = circularityInfo; info; info = info->next)
    {
        if (declRef == info->declRef)
        {
            getSink()->diagnose(Diagnostics::VariableUsedInItsOwnDefinition{.decl = decl});
            return true;
        }
    }

    return false;
}

IntVal* SemanticsVisitor::tryConstantFoldDeclRef(
    DeclRef<VarDeclBase> const& declRef,
    ConstantFoldingKind kind,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    auto decl = declRef.getDecl();

    if (_checkForCircularityInConstantFolding(declRef, circularityInfo))
        return nullptr;

    // In HLSL, `const` is used to mark compile-time constant expressions.
    if (!decl->hasModifier<ConstModifier>())
        return nullptr;

    // The values of specialization constants aren't known at compile time even
    // if they're marked `const`.
    if (decl->hasModifier<SpecializationConstantAttribute>() ||
        decl->hasModifier<VkConstantIdAttribute>())
    {
        if (kind == ConstantFoldingKind::SpecializationConstant)
        {
            // Float-to-inst casts cannot be`OpSpecConstOp` operations in SPIR-V,
            // which means they need to be local instructions can cannot be hoisted to the
            // global scope. Deduplication logic is run for `IntVal`s however and without hoisting
            // instructions using this `IntVal` will trigger error. Hence we emit error here
            // to not allow such cases.
            //
            // Note that float-to-inst casts for non-`IntVal`s are allowed.
            if (!isValidCompileTimeConstantType(decl->getType()))
            {
                getSink()->diagnose(Diagnostics::IntValFromNonIntSpecConstEncountered{
                    .location = declRef.getLoc()});
                return nullptr;
            }

            return m_astBuilder->getOrCreate<DeclRefIntVal>(
                declRef.substitute(m_astBuilder, decl->getType()),
                declRef);
        }
        // Don't fold on other folding passes, we don't actually know the
        // values.
        return nullptr;
    }

    if (decl->hasModifier<ExternModifier>())
    {
        // Extern const is not considered compile-time constant by the front-end.
        if (kind == ConstantFoldingKind::CompileTime)
            return nullptr;
        // But if we are OK with link-time constants, we can still fold it into a val.
        auto rs = m_astBuilder->getOrCreate<DeclRefIntVal>(
            declRef.substitute(m_astBuilder, declRef.getDecl()->getType()),
            declRef);
        return rs;
    }

    if (isInterfaceRequirement(decl))
    {
        auto witness =
            findThisTypeWitness(SubstitutionSet(declRef), as<InterfaceDecl>(decl->parentDecl));

        auto foldType = declRef.substitute(m_astBuilder, decl->type.type);
        auto val = WitnessLookupIntVal::tryFold(m_astBuilder, witness, decl, foldType);

        // A signature-type-position fold (e.g. `float[VALUE::COUNT]`) can run before the
        // conforming type's witness table is built, leaving a symbolic result; ensure its
        // conformances and re-fold so the value matches the concrete constant the in-body
        // path produces.
        if (as<WitnessLookupIntVal>(val))
        {
            SLANG_ASSERT(witness);
            if (auto subDeclRefType = as<DeclRefType>(witness->getSub()))
            {
                ensureDecl(
                    subDeclRefType->getDeclRef().getDecl(),
                    DeclCheckState::ReadyForConformances);
                val = WitnessLookupIntVal::tryFold(m_astBuilder, witness, decl, foldType);
            }
        }
        return as<IntVal>(val);
    }

    if (!getInitExpr(m_astBuilder, declRef))
        return nullptr;

    ensureDecl(declRef.getDecl(), DeclCheckState::DefinitionChecked);
    ConstantFoldingCircularityInfo newCircularityInfo(declRef, circularityInfo);
    return tryConstantFoldExpr(getInitExpr(m_astBuilder, declRef), kind, &newCircularityInfo);
}

IntVal* SemanticsVisitor::tryConstantFoldBuiltinOperatorExpr(
    SubstExpr<BuiltinOperatorExpr> expr,
    ConstantFoldingKind kind,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    auto e = expr.getExpr();
    const Index argCount = e->arguments.getCount();
    List<IntVal*> argVals;
    for (Index a = 0; a < argCount; ++a)
    {
        auto argVal = tryFoldIntegerConstantExpression(
            SubstExpr<Expr>(e->arguments[a], expr.getSubsts()),
            kind,
            circularityInfo);
        if (!argVal)
            return nullptr;
        argVals.add(argVal);
    }
    auto resultType = as<Type>(e->type.type->substitute(m_astBuilder, expr.getSubsts()));
    auto op = e->op;

    // If all operands are concrete, fold to a constant directly. Pass the operator expression's
    // location so a divide-by-zero diagnostic points at the offending operator (`1 / 0`) rather
    // than being location-less.
    if (auto folded = as<IntVal>(BuiltinOperationIntVal::tryFoldImpl(
            m_astBuilder,
            resultType,
            op,
            argVals,
            getSink(),
            e->loc)))
        return folded;

    // Otherwise the result is symbolic. `+`/`-`/`*`/unary-`-` use `PolynomialIntVal` so value
    // unification can canonicalize (e.g. `N+1` == `1+N`); the rest use the decl-free
    // `BuiltinOperationIntVal`, which re-folds once its operands become concrete.
    switch (op)
    {
    case BuiltinOperationKind::Add:
        return PolynomialIntVal::add(m_astBuilder, argVals[0], argVals[1]);
    case BuiltinOperationKind::Sub:
        return PolynomialIntVal::sub(m_astBuilder, argVals[0], argVals[1]);
    case BuiltinOperationKind::Mul:
        return PolynomialIntVal::mul(m_astBuilder, argVals[0], argVals[1]);
    case BuiltinOperationKind::Neg:
        return PolynomialIntVal::neg(m_astBuilder, argVals[0]);
    default:
        return m_astBuilder->getOrCreate<BuiltinOperationIntVal>(
            resultType,
            op,
            argVals.getArrayView());
    }
}

IntVal* SemanticsVisitor::tryConstantFoldExpr(
    SubstExpr<Expr> expr,
    ConstantFoldingKind kind,
    ConstantFoldingCircularityInfo* circularityInfo)
{

    // Unwrap any "identity" expressions
    while (auto parenExpr = expr.as<ParenExpr>())
    {
        expr = getBaseExpr(parenExpr);
    }

    if (auto intLitExpr = expr.as<IntegerLiteralExpr>())
    {
        return getIntVal(intLitExpr);
    }

    if (auto boolLitExpr = expr.as<BoolLiteralExpr>())
    {
        // If it's a boolean, we allow promotion to int.
        const IntegerLiteralValue value = IntegerLiteralValue(boolLitExpr.getExpr()->value);
        return m_astBuilder->getIntVal(m_astBuilder->getBoolType(), value);
    }

    if (auto arrayLengthExpr = expr.as<GetArrayLengthExpr>())
    {
        if (arrayLengthExpr.getExpr()->arrayExpr && arrayLengthExpr.getExpr()->arrayExpr->type)
        {
            auto type = arrayLengthExpr.getExpr()->arrayExpr->type.type->substitute(
                m_astBuilder,
                expr.getSubsts());
            if (auto arrayType = as<ArrayExpressionType>(type))
            {
                if (!arrayType->isUnsized())
                {
                    if (auto val = as<IntVal>(arrayType->getElementCount()))
                        return val;
                }
            }
        }
    }

    if (auto sizeOfLikeExpr = expr.as<SizeOfLikeExpr>())
    {
        // sizedType is populated by visitSizeOfLikeExpr during expression visiting,
        // which may not have run yet if we are constant-folding during header checking.
        // Check the expression now to ensure it is populated.
        if (!sizeOfLikeExpr.getExpr()->sizedType)
            CheckTerm(sizeOfLikeExpr.getExpr());

        if (!sizeOfLikeExpr.getExpr()->sizedType)
            return nullptr;

        auto type = as<Type>(
            sizeOfLikeExpr.getExpr()->sizedType->substitute(m_astBuilder, expr.getSubsts()));

        if (sizeOfLikeExpr.getExpr()->dataLayoutType)
        {
            auto dataLayoutType = as<Type>(sizeOfLikeExpr.getExpr()->dataLayoutType->substitute(
                m_astBuilder,
                expr.getSubsts()));
            // we can only constant-fold sizeof-like expressions when in
            // natural/scalar data layout.
            if (!as<ScalarDataLayoutType>(dataLayoutType))
                return nullptr;
        }

        if (auto sizeOfExpr = expr.as<SizeOfExpr>())
        {
            return as<IntVal>(SizeOfIntVal::tryFold(m_astBuilder, expr.getExpr()->type.type, type));
        }
        else if (auto alignOfExpr = expr.as<AlignOfExpr>())
        {
            return as<IntVal>(
                AlignOfIntVal::tryFold(m_astBuilder, expr.getExpr()->type.type, type));
        }
        else if (auto countOfExpr = expr.as<CountOfExpr>())
        {
            // For value packs, sizedType is ValuePackType. Try to fold
            // the value expression to get the pack reference instead.
            Val* countArg = type;
            if (as<ValuePackType>(type))
            {
                auto valExprFolded = tryConstantFoldExpr(
                    SubstExpr<Expr>(countOfExpr.getExpr()->value, expr.getSubsts()),
                    kind,
                    circularityInfo);
                if (valExprFolded)
                    countArg = valExprFolded;
            }
            return as<IntVal>(
                CountOfIntVal::tryFold(m_astBuilder, expr.getExpr()->type.type, countArg));
        }
    }

    if (auto packQueryExpr = expr.as<PackQueryExpr>())
    {
        auto foldedOperand = tryConstantFoldExpr(
            SubstExpr<Expr>(packQueryExpr.getExpr()->value, expr.getSubsts()),
            kind,
            circularityInfo);
        if (!foldedOperand)
            return nullptr;

        if (as<FirstExpr>(packQueryExpr.getExpr()))
            return as<IntVal>(m_astBuilder->getFirstElement(foldedOperand));
        if (as<LastExpr>(packQueryExpr.getExpr()))
            return as<IntVal>(m_astBuilder->getLastElement(foldedOperand));
        if (as<TrimFirstExpr>(packQueryExpr.getExpr()))
            return as<IntVal>(m_astBuilder->getTrimFirstPack(foldedOperand));
        return as<IntVal>(m_astBuilder->getTrimLastPack(foldedOperand));
    }

    if (auto shapePackExpr = expr.as<ShapePackTransformExpr>())
    {
        if (auto shapeConcatExpr = as<ShapeConcatExpr>(shapePackExpr.getExpr()))
        {
            auto leftPack = tryConstantFoldExpr(
                SubstExpr<Expr>(shapeConcatExpr->getArg(0), expr.getSubsts()),
                kind,
                circularityInfo);
            auto rightPack = tryConstantFoldExpr(
                SubstExpr<Expr>(shapeConcatExpr->getArg(1), expr.getSubsts()),
                kind,
                circularityInfo);
            auto axis = as<IntVal>(tryConstantFoldExpr(
                SubstExpr<Expr>(shapeConcatExpr->getArg(2), expr.getSubsts()),
                kind,
                circularityInfo));
            if (!leftPack || !rightPack || !axis)
                return nullptr;

            return as<IntVal>(m_astBuilder->getShapeConcatIntValPack(leftPack, rightPack, axis));
        }

        if (auto shapePermuteExpr = as<ShapePermuteExpr>(shapePackExpr.getExpr()))
        {
            auto valuePack = tryConstantFoldExpr(
                SubstExpr<Expr>(shapePermuteExpr->getArg(0), expr.getSubsts()),
                kind,
                circularityInfo);
            auto orderPack = tryConstantFoldExpr(
                SubstExpr<Expr>(shapePermuteExpr->getArg(1), expr.getSubsts()),
                kind,
                circularityInfo);
            if (!valuePack || !orderPack)
                return nullptr;

            return as<IntVal>(m_astBuilder->getShapePermuteIntValPack(valuePack, orderPack));
        }

        if (auto shapeSwapExpr = as<ShapeSwapExpr>(shapePackExpr.getExpr()))
        {
            auto valuePack = tryConstantFoldExpr(
                SubstExpr<Expr>(shapeSwapExpr->getArg(0), expr.getSubsts()),
                kind,
                circularityInfo);
            auto dim0 = as<IntVal>(tryConstantFoldExpr(
                SubstExpr<Expr>(shapeSwapExpr->getArg(1), expr.getSubsts()),
                kind,
                circularityInfo));
            auto dim1 = as<IntVal>(tryConstantFoldExpr(
                SubstExpr<Expr>(shapeSwapExpr->getArg(2), expr.getSubsts()),
                kind,
                circularityInfo));
            if (!valuePack || !dim0 || !dim1)
                return nullptr;

            return as<IntVal>(m_astBuilder->getShapeSwapIntValPack(valuePack, dim0, dim1));
        }

        auto shapeReduceExpr = as<ShapeReduceExpr>(shapePackExpr.getExpr());
        SLANG_ASSERT(shapeReduceExpr);

        auto valuePack = tryConstantFoldExpr(
            SubstExpr<Expr>(shapeReduceExpr->getArg(0), expr.getSubsts()),
            kind,
            circularityInfo);
        auto axis = as<IntVal>(tryConstantFoldExpr(
            SubstExpr<Expr>(shapeReduceExpr->getArg(1), expr.getSubsts()),
            kind,
            circularityInfo));
        if (!valuePack || !axis)
            return nullptr;

        return as<IntVal>(m_astBuilder->getShapeReduceIntValPack(valuePack, axis));
    }

    // `each D` where D is a value pack parameter produces an EachIntVal.
    if (auto eachExpr = expr.as<EachExpr>())
    {
        if (auto baseExpr = eachExpr.getExpr()->baseExpr)
        {
            if (auto valPackType = as<ValuePackType>(baseExpr->type))
            {
                if (auto baseDeclRefExpr = as<DeclRefExpr>(baseExpr))
                {
                    auto baseDeclRef = getDeclRef(m_astBuilder, baseDeclRefExpr);
                    if (auto packParamRef = baseDeclRef.as<GenericValuePackParamDecl>())
                    {
                        auto elementType = valPackType->getElementType();
                        auto packRef =
                            m_astBuilder->getOrCreate<DeclRefIntVal>(valPackType, packParamRef);
                        Val* substPackRef = packRef->substitute(m_astBuilder, expr.getSubsts());
                        return m_astBuilder->getEachIntVal(elementType, substPackRef);
                    }
                }
            }
        }
    }

    // A PackExpr with integer elements folds to a ConcreteIntValPack.
    if (auto packExpr = expr.as<PackExpr>())
    {
        ShortList<IntVal*> elements;
        for (auto arg : packExpr.getExpr()->args)
        {
            auto elementVal =
                tryConstantFoldExpr(SubstExpr<Expr>(arg, expr.getSubsts()), kind, circularityInfo);
            if (!elementVal)
                return nullptr;
            elements.add(elementVal);
        }
        return m_astBuilder->getIntValPack(elements.getArrayView().arrayView);
    }

    // A TupleExpr with integer elements also folds to a ConcreteIntValPack so
    // tuple-backed pack queries can participate in integer-constant contexts.
    if (auto tupleExpr = expr.as<TupleExpr>())
    {
        ShortList<IntVal*> elements;
        for (auto elementExpr : tupleExpr.getExpr()->elements)
        {
            auto elementVal = tryConstantFoldExpr(
                SubstExpr<Expr>(elementExpr, expr.getSubsts()),
                kind,
                circularityInfo);
            if (!elementVal)
                return nullptr;
            elements.add(elementVal);
        }
        return m_astBuilder->getIntValPack(elements.getArrayView().arrayView);
    }

    // `expand <expr>` where the expr involves value packs produces an ExpandIntValPack.
    if (auto expandExpr = expr.as<ExpandExpr>())
    {
        if (auto expandType = as<ExpandType>(getType(m_astBuilder, expandExpr)))
        {
            auto patternVal = tryConstantFoldExpr(
                SubstExpr<Expr>(expandExpr.getExpr()->baseExpr, expr.getSubsts()),
                kind,
                circularityInfo);
            if (patternVal)
            {
                ShortList<Val*> capturedPacks;
                for (Index i = 0; i < expandType->getCapturedPackCount(); i++)
                    capturedPacks.add(expandType->getCapturedPack(i));
                return as<IntVal>(m_astBuilder->getExpandIntValPack(
                    patternVal,
                    capturedPacks.getArrayView().arrayView));
            }
        }
    }

    // it is possible that we are referring to a generic value param
    if (auto declRefExpr = expr.as<DeclRefExpr>())
    {
        auto declRef = getDeclRef(m_astBuilder, declRefExpr);

        if (auto genericValParamRef = declRef.as<GenericValueParamDecl>())
        {
            Val* valResult = m_astBuilder->getOrCreate<DeclRefIntVal>(
                declRef.substitute(m_astBuilder, genericValParamRef.getDecl()->getType()),
                genericValParamRef);
            valResult = valResult->substitute(m_astBuilder, expr.getSubsts());
            return as<IntVal>(valResult);
        }

        if (auto genericValPackParamRef = declRef.as<GenericValuePackParamDecl>())
        {
            Val* valResult = m_astBuilder->getOrCreate<DeclRefIntVal>(
                genericValPackParamRef.getDecl()->getType(),
                genericValPackParamRef);
            valResult = valResult->substitute(m_astBuilder, expr.getSubsts());
            return as<IntVal>(valResult);
        }

        // We may also need to check for references to variables that
        // are defined in a way that can be used as a constant expression:
        if (auto varRef = declRef.as<VarDeclBase>())
        {
            return tryConstantFoldDeclRef(varRef, kind, circularityInfo);
        }
        else if (auto enumRef = declRef.as<EnumCaseDecl>())
        {
            auto enumTypeDecl = enumRef.getParent().getDecl();
            if (enumTypeDecl && !enumTypeDecl->checkState.isBeingChecked())
            {
                ensureDecl(enumRef.getParent(), DeclCheckState::DefinitionChecked);
            }

            // The cases in an `enum` declaration can also be used as constant expressions,
            if (auto tagExpr = getTagExpr(m_astBuilder, enumRef))
            {
                auto enumCaseDecl = enumRef.getDecl();
                if (_checkForCircularityInConstantFolding(enumRef, circularityInfo))
                    return nullptr;

                ConstantFoldingCircularityInfo newCircularityInfo(enumRef, circularityInfo);
                auto intVal = as<IntVal>(tryConstantFoldExpr(tagExpr, kind, &newCircularityInfo));
                if (!intVal)
                    return nullptr;
                return as<IntVal>(
                    m_astBuilder->getTypeCastIntVal(enumCaseDecl->getType(), intVal)->resolve());
            }
        }
    }

    SubstExpr<Expr> typeCastOperand;
    if (auto typeCastExpr = expr.as<TypeCastExpr>())
        typeCastOperand = getArg(typeCastExpr, 0);
    else if (auto builtinCastExpr = expr.as<BuiltinCastExpr>())
        typeCastOperand = getBaseExpr(builtinCastExpr);

    if (typeCastOperand)
    {
        auto substType = getType(m_astBuilder, expr);
        if (!substType)
            return nullptr;
        if (!isValidCompileTimeConstantType(substType))
            return nullptr;

        IntVal* val = tryConstantFoldExpr(typeCastOperand, kind, circularityInfo);
        if (!val)
        {
            if (auto floatLitExpr = typeCastOperand.as<FloatingPointLiteralExpr>())
            {
                // When explicitly casting from float type to integer type, let's fold it as
                // an integer value.
                const IntegerLiteralValue value =
                    IntegerLiteralValue(floatLitExpr.getExpr()->value);
                val = m_astBuilder->getIntVal(substType, value);
            }
        }

        if (val)
        {
            if (!expr.getExpr()->type)
                return nullptr;
            auto foldVal =
                as<IntVal>(TypeCastIntVal::tryFoldImpl(m_astBuilder, substType, val, getSink()));
            if (foldVal)
                return foldVal;
            auto result = m_astBuilder->getTypeCastIntVal(substType, val);
            return result;
        }
    }
    else if (auto builtinOpExpr = expr.as<BuiltinOperatorExpr>())
    {
        return tryConstantFoldBuiltinOperatorExpr(builtinOpExpr, kind, circularityInfo);
    }
    else if (auto invokeExpr = expr.as<InvokeExpr>())
    {
        auto val = tryConstantFoldExpr(invokeExpr, kind, circularityInfo);
        if (val)
            return val;
    }
    else if (auto indexExpr = expr.as<IndexExpr>())
    {
        return tryFoldIndexExpr(indexExpr.getExpr(), kind, circularityInfo);
    }
    // Note: FloatBitCastExpr (__floatAsInt) is folded directly in visitFloatBitCastExpr
    // and replaced with an IntegerLiteralExpr, so we don't need to handle it here.
    return nullptr;
}

IntVal* SemanticsVisitor::tryFoldIndexExpr(
    SubstExpr<IndexExpr> expr,
    ConstantFoldingKind kind,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    // Ad-hoc constant folding for index expressions.
    // TOOD: we should generalize this by extending `Val` to support compile-time constants that are
    // not just integers, but also arrays and structs etc, so that we can independently fold
    // the base expression and the index expression, and then form a ElementExtractVal() from an
    // index expr.
    // For now we just specialize case for array expression that is an initialization list.
    // And this won't work if the array is a link-time constant.
    //
    auto declRefExpr = as<DeclRefExpr>(expr.getExpr()->baseExpression);
    if (!declRefExpr)
        return nullptr;
    auto varDecl = as<VarDecl>(declRefExpr->declRef.getDecl());
    if (!varDecl)
        return nullptr;
    auto type = varDecl->getType();
    if (!type)
        return nullptr;
    auto arrayType = as<ArrayExpressionType>(type);
    if (!arrayType)
        return nullptr;
    if (!varDecl->hasModifier<ConstModifier>())
        return nullptr;
    if (isGlobalDecl(varDecl) && !varDecl->hasModifier<HLSLStaticModifier>())
        return nullptr;
    if (!varDecl->initExpr)
        return nullptr;
    auto arrayContentExpr = as<InitializerListExpr>(varDecl->initExpr);
    if (!arrayContentExpr)
        return nullptr;
    if (expr.getExpr()->indexExprs.getCount() != 1)
        return nullptr;
    auto indexVal = as<ConstantIntVal>(
        tryFoldIntegerConstantExpression(expr.getExpr()->indexExprs[0], kind, circularityInfo));
    if (!indexVal)
        return nullptr;
    auto index = indexVal->getValue();
    if (index < 0 || index >= arrayContentExpr->args.getCount())
        return nullptr;
    auto elementExpr = arrayContentExpr->args[Index(index)];
    return tryFoldIntegerConstantExpression(elementExpr, kind, circularityInfo);
}

IntVal* SemanticsVisitor::tryFoldIntegerConstantExpression(
    SubstExpr<Expr> expr,
    ConstantFoldingKind kind,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    // Check if type is acceptable for an integer constant expression
    //
    if (!isValidCompileTimeConstantType(getType(m_astBuilder, expr)))
        return nullptr;

    // Consider operations that we might be able to constant-fold...
    //
    return tryConstantFoldExpr(expr, kind, circularityInfo);
}

IntVal* SemanticsVisitor::foldIntegerConstantExpressionFromFinalizedExpr(
    Expr* expr,
    ConstantFoldingKind kind,
    DiagnosticSink* sink,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    SLANG_RELEASE_ASSERT(expr);

    if (IsErrorExpr(expr))
        return nullptr;

    auto result = tryFoldIntegerConstantExpression(expr, kind, circularityInfo);
    if (!result && sink)
        sink->diagnose(Diagnostics::ExpectedIntegerConstantNotConstant{.location = expr->loc});
    return result;
}

IntVal* SemanticsVisitor::CheckIntegerConstantExpression(
    Expr* inExpr,
    IntegerConstantExpressionCoercionType coercionType,
    Type* expectedType,
    ConstantFoldingKind kind,
    DiagnosticSink* sink)
{
    return CheckIntegerConstantExpression(inExpr, coercionType, expectedType, kind, sink, nullptr);
}

IntVal* SemanticsVisitor::CheckIntegerConstantExpression(
    Expr* inExpr,
    IntegerConstantExpressionCoercionType coercionType,
    Type* expectedType,
    ConstantFoldingKind kind,
    DiagnosticSink* sink,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    auto finalizedExpr = inExpr;
    return checkAndFinalizeIntegerConstantExpression(
        finalizedExpr,
        coercionType,
        expectedType,
        kind,
        sink,
        circularityInfo);
}

IntVal* SemanticsVisitor::checkAndFinalizeIntegerConstantExpression(
    Expr*& ioExpr,
    IntegerConstantExpressionCoercionType coercionType,
    Type* expectedType,
    ConstantFoldingKind kind,
    DiagnosticSink* sink,
    ConstantFoldingCircularityInfo* circularityInfo)
{
    // No need to issue further errors if the expression didn't even type-check.
    if (IsErrorExpr(ioExpr))
        return nullptr;

    Type* targetType = nullptr;
    switch (coercionType)
    {
    case IntegerConstantExpressionCoercionType::SpecificType:
        SLANG_RELEASE_ASSERT(expectedType);
        targetType = expectedType;
        break;
    case IntegerConstantExpressionCoercionType::AnyInteger:
        if (isScalarIntegerType(ioExpr->type) || isEnumType(ioExpr->type))
            targetType = ioExpr->type.type;
        else
            targetType = m_astBuilder->getIntType();
        break;
    default:
        SLANG_UNEXPECTED("unhandled integer constant coercion mode");
    }

    // This call is the only operation here that finalizes the selected expression and records its
    // `ReadValue` use. Contextual coercion can first select an overload; finalization then commits
    // that selected path exactly once.
    ioExpr = coerceAndFinalizeForStorageUse(
        CoercionSite::General,
        targetType,
        ioExpr,
        sink,
        AbstractStorageUse::ReadValue);

    // No need to issue further errors if the type coercion failed.
    if (IsErrorExpr(ioExpr))
        return nullptr;

    return foldIntegerConstantExpressionFromFinalizedExpr(ioExpr, kind, sink, circularityInfo);
}

IntVal* SemanticsVisitor::CheckIntegerConstantExpression(
    Expr* inExpr,
    IntegerConstantExpressionCoercionType coercionType,
    Type* expectedType,
    ConstantFoldingKind kind)
{
    return CheckIntegerConstantExpression(inExpr, coercionType, expectedType, kind, getSink());
}

IntVal* SemanticsVisitor::CheckEnumConstantExpression(Expr* expr, ConstantFoldingKind kind)
{
    // No need to issue further errors if the expression didn't even type-check.
    if (IsErrorExpr(expr))
        return nullptr;

    // No need to issue further errors if the type coercion failed.
    if (IsErrorExpr(expr))
        return nullptr;

    auto result = tryConstantFoldExpr(expr, kind, nullptr);
    if (!result)
    {
        getSink()->diagnose(Diagnostics::ExpectedIntegerConstantNotConstant{.location = expr->loc});
    }
    return result;
}

/// Returns the expression beneath any parentheses around an index's storage projection base.
///
/// Parentheses preserve the l-value and storage denoted by their operand. For example,
/// `(storage.item.xy)[1]` projects the same swizzled property storage as `storage.item.xy[1]`.
/// `_getSubscriptStorageProjection()` therefore classifies the underlying swizzle instead of
/// treating parentheses as a request for a direct address.
static Expr* _getStorageProjectionBaseExpr(Expr* expr)
{
    while (auto parenExpr = as<ParenExpr>(expr))
    {
        expr = parenExpr->base;
    }
    return expr;
}

/// Returns the storage-projection facts for a checked subscript.
///
/// Consider `value.xy[1]`. A constant in-range index selects `value.y`, so `visitIndexExpr()` can
/// preserve the selected component inside a deferred property value and then use getter/setter
/// writeback. For a dynamic index into `value.xy`, `_getSubscriptStorageProjection()` records that
/// the index was not constant. `visitIndexExpr()` can copy the selected value for
/// `BorrowProjectedValue`, but it diagnoses a nonconstant index when an `out`, `inout`, `ref`, or
/// other use requires one component's address. An ordinary subscript requires its base's direct
/// address because lowering does not retain an arbitrary indexed write-back operation.
static IndexExprStorageProjection _getSubscriptStorageProjection(
    SemanticsVisitor* semantics,
    IndexExpr* subscriptExpr)
{
    SLANG_RELEASE_ASSERT(subscriptExpr->indexExprs.getCount() == 1);

    auto projectionBase = _getStorageProjectionBaseExpr(subscriptExpr->baseExpression);
    Index elementCount = 0;
    if (auto swizzleExpr = as<SwizzleExpr>(projectionBase))
    {
        if (isDirectAddressTupleElementSwizzle(swizzleExpr))
            return IndexExprStorageProjection::makeDirectAddress();
        elementCount = swizzleExpr->elementIndices.getCount();
    }
    else if (auto matrixSwizzleExpr = as<MatrixSwizzleExpr>(projectionBase))
        elementCount = matrixSwizzleExpr->elementCount;
    else
        return IndexExprStorageProjection::makeDirectAddress();

    auto indexValue = as<ConstantIntVal>(semantics->tryConstantFoldExpr(
        SubstExpr<Expr>(subscriptExpr->indexExprs[0]),
        SemanticsVisitor::ConstantFoldingKind::CompileTime,
        nullptr));
    if (!indexValue)
        return IndexExprStorageProjection::makeNonConstantSwizzleElement();
    if (indexValue->getValue() < 0)
        return IndexExprStorageProjection::makeDirectAddress();
    if (indexValue->getValue() >= elementCount)
        return IndexExprStorageProjection::makeDirectAddress();

    return IndexExprStorageProjection::makeRetainedSwizzleElement(Index(indexValue->getValue()));
}

/// Returns how a checked index projection maps writes to abstract storage behind its base.
static AbstractStorageProjectionMode _getAbstractStorageProjectionMode(
    const IndexExprStorageProjection& projection)
{
    if (projection.hasRetainedSwizzleElement())
        return AbstractStorageProjectionMode::ReadModifyWrite;
    return AbstractStorageProjectionMode::RequireAddressForWrite;
}

Expr* SemanticsVisitor::CheckSimpleSubscriptExpr(IndexExpr* subscriptExpr, Type* elementType)
{
    auto baseExpr = subscriptExpr->baseExpression;
    if (subscriptExpr->indexExprs.getCount() < 1)
    {
        getSink()->diagnose(Diagnostics::NotEnoughArguments{
            .got = subscriptExpr->indexExprs.getCount(),
            .expected = 1,
            .location = subscriptExpr->loc});
        return CreateErrorExpr(subscriptExpr);
    }
    else if (subscriptExpr->indexExprs.getCount() > 1)
    {
        getSink()->diagnose(Diagnostics::TooManyArguments{
            .got = subscriptExpr->indexExprs.getCount(),
            .expected = 1,
            .location = subscriptExpr->loc});
        return CreateErrorExpr(subscriptExpr);
    }

    for (auto& expr : subscriptExpr->indexExprs)
    {
        expr = CheckTerm(expr);
    }
    auto& indexExpr = subscriptExpr->indexExprs[0];

    auto intTargetType = getMatchingIntType(indexExpr->type.type);
    indexExpr = coerceExpressionWithoutFinalization(
        CoercionSite::Argument, intTargetType, indexExpr, getSink());

    subscriptExpr->type = QualType(elementType);

    // TODO(tfoley): need to be more careful about this stuff
    subscriptExpr->type.isLeftValue = baseExpr->type.isLeftValue;

    if (as<PtrTypeBase>(baseExpr->type.type))
    {
        auto baseAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            baseExpr->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
        auto indexAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            indexExpr->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
        subscriptExpr->abstractStorageAccess = AbstractStorageAccessPath::makeSequence(
            m_astBuilder,
            baseAccess,
            indexAccess,
            AbstractStorageSequenceResult::SecondAccess);
    }
    else
    {
        subscriptExpr->setStorageProjection(
            _getSubscriptStorageProjection(this, subscriptExpr));
        auto baseAccess = AbstractStorageAccessPath::makeProjection(
            m_astBuilder,
            baseExpr->abstractStorageAccess,
            _getAbstractStorageProjectionMode(subscriptExpr->getStorageProjection()));
        subscriptExpr->abstractStorageAccess = AbstractStorageAccessPath::makeSequence(
            m_astBuilder,
            baseAccess,
            indexExpr->abstractStorageAccess,
            AbstractStorageSequenceResult::FirstAccess);
    }

    return subscriptExpr;
}

void registerAssociatedMethods(SemanticsVisitor* context, DeclRef<Decl> declRef)
{
    // A subscript or property denotes storage, while its accessors are the functions that are
    // actually called. Register every accessor because the getter-versus-setter decision is
    // intentionally deferred until lowering materializes the storage reference.
    if (declRef.as<SubscriptDecl>() || declRef.as<PropertyDecl>())
    {
        for (auto accessorDeclRef :
             getMembersOfType<AccessorDecl>(context->getASTBuilder(), declRef.as<ContainerDecl>()))
        {
            registerAssociatedMethods(context, accessorDeclRef);
        }
        return;
    }

    // Lower witness for ForwardDifferentiable for this function.
    // First we'll turn it into a func-as-type-expr, then check that
    // to get the function reference as a type, and then get the witness
    // from that.
    //
    // TODO: Make this more general, by registering the interfaces that we
    // need to check against as 'Decl' in the parent functions' body &
    // lowering all of them in CheckTerm()
    //
    auto funcAsType = DeclRefType::create(context->getASTBuilder(), declRef);

    // Revised AD 2.0 lowering.
    {
        if (auto fwdDiffVal = maybeRegisterVal(
                context,
                funcAsType,
                declRef.declRefBase,
                context->getName("fwd_diff"),
                AnnotationKind::ForwardDerivative))
        {
            auto effectiveDiffVal = fwdDiffVal;
            auto funcAliasDeclRef = DeclRef<FuncAliasDecl>(as<DeclRefBase>(fwdDiffVal));
            if (funcAliasDeclRef)
            {
                effectiveDiffVal = substituteDeclRef(
                                       SubstitutionSet(funcAliasDeclRef),
                                       getCurrentASTBuilder(),
                                       funcAliasDeclRef.getDecl()->targetDeclRef)
                                       .as<FunctionDeclBase>();
            }

            // Create a type for the fwd_diff function to look up its own conformances.
            auto fwdDiffType =
                DeclRefType::create(context->getASTBuilder(), as<DeclRefBase>(effectiveDiffVal));

            // Lower the fwd_diff : IForwardDifferentiable witness table
            maybeRegisterWitness(
                context,
                fwdDiffType,
                declRef.declRefBase,
                context->getForwardDiffFuncInterfaceType(fwdDiffType),
                AnnotationKind::FwdDiffForwardDerivativeWitnessTable);

            // Lower the fwd_diff : IBackwardDifferentiable witness table
            maybeRegisterWitness(
                context,
                fwdDiffType,
                declRef.declRefBase,
                context->getBackwardDiffFuncInterfaceType(fwdDiffType),
                AnnotationKind::FwdDiffBackwardDerivativeWitnessTable);
        }

        if (maybeRegisterVal(
                context,
                funcAsType,
                declRef.declRefBase,
                context->getName("apply_bwd"),
                AnnotationKind::BackwardDerivativeApply))
        {
            auto bwdCallableType = as<DeclRefBase>(maybeRegisterVal(
                context,
                funcAsType,
                declRef.declRefBase,
                context->getName("BwdCallable"),
                AnnotationKind::BackwardDerivativeContext));
            if (!bwdCallableType)
            {
                context->getSink()->diagnose(Diagnostics::Unexpected{
                    .message = "failed to find 'BwdCallable' type for backward derivative context",
                    .location = declRef.getLoc()});
                return;
            }

            maybeRegisterVal(
                context,
                DeclRefType::create(getCurrentASTBuilder(), bwdCallableType),
                declRef.declRefBase,
                context->getName("()"),
                AnnotationKind::BackwardDerivativePropagate);

            maybeRegisterVal(
                context,
                funcAsType,
                declRef.declRefBase,
                context->getName("remat"),
                AnnotationKind::BackwardDerivativeContextRemat);
        }
    }
}

/// Registers derivative associations when `callableExpr` directly selects a declaration.
///
/// Callable selection may preserve parentheses, a generic application, or qualifier-local
/// bindings around the selected declaration reference. Calls through other function-valued
/// expressions have no declaration whose derivative associations can be registered here.
static void registerAssociatedMethodsForSelectedCallable(
    SemanticsVisitor* context,
    Expr* callableExpr)
{
    auto declRefExpr =
        as<DeclRefExpr>(getCallableExprThroughTransparentWrappers(callableExpr));
    if (!declRefExpr || !declRefExpr->declRef)
        return;
    registerAssociatedMethods(context, getDeclRef(context->getASTBuilder(), declRefExpr));
}

Expr* SemanticsExprVisitor::visitIndexExpr(IndexExpr* subscriptExpr)
{
    SemanticsExprVisitor baseVisitor(withPotentiallyUnevaluatedTerm());
    bool needDeref = false;
    auto baseExpr = baseVisitor.checkBaseForMemberExpr(
        subscriptExpr->baseExpression,
        CheckBaseContext::Subscript,
        needDeref);

    // If the base expression is a type, it means that this is an array declaration,
    // then we should disable short-circuit in case there is logical expression in
    // the subscript
    // A semantic modifier changes how later passes consume a value, but it does not change the
    // declarations or built-in indexing operations provided by the value's base type. Member
    // lookup applies the same rule in `visitMemberExpr`.
    auto baseType = unwrapModifiedType(baseExpr->type.Ptr());
    auto baseTypeType = as<TypeType>(baseType);
    auto subVisitor = (baseTypeType && m_shouldShortCircuitLogicExpr)
                          ? SemanticsVisitor(disableShortCircuitLogicalExpr())
                          : *this;

    for (auto& arg : subscriptExpr->indexExprs)
    {
        arg = subVisitor.CheckTerm(arg);
    }

    // If anything went wrong in the base expression,
    // then just move along...
    if (IsErrorExpr(baseExpr))
        return CreateErrorExpr(subscriptExpr);

    subscriptExpr->baseExpression = baseExpr;

    // Otherwise, we need to look at the type of the base expression,
    // to figure out how subscripting should work.
    if (baseTypeType)
    {
        // We are trying to "index" into a type, so we have an expression like `float[2]`
        // which should be interpreted as resolving to an array type.

        IntVal* elementCount = nullptr;
        if (subscriptExpr->indexExprs.getCount() == 1)
        {
            elementCount = CheckIntegerConstantExpression(
                subscriptExpr->indexExprs[0],
                IntegerConstantExpressionCoercionType::AnyInteger,
                nullptr,
                ConstantFoldingKind::SpecializationConstant);

            // Validate that array size is non-negative.
            if (auto constElementCount = as<ConstantIntVal>(elementCount))
            {
                if (constElementCount->getValue() < 0)
                {
                    getSink()->diagnose(Diagnostics::InvalidArraySize{
                        .location = subscriptExpr->indexExprs[0]->loc});
                    return CreateErrorExpr(subscriptExpr);
                }
            }
        }
        else if (subscriptExpr->indexExprs.getCount() != 0)
        {
            getSink()->diagnose(
                Diagnostics::MultiDimensionalArrayNotSupported{.expr = subscriptExpr});
        }

        auto elementType = CoerceToUsableType(TypeExp(baseExpr, baseTypeType->getType()), nullptr);
        auto arrayType = getArrayType(m_astBuilder, elementType, elementCount);

        subscriptExpr->type = QualType(m_astBuilder->getTypeType(arrayType));
        return subscriptExpr;
    }
    else if (auto baseArrayType = as<ArrayExpressionType>(baseType))
    {
        return CheckSimpleSubscriptExpr(subscriptExpr, baseArrayType->getElementType());
    }
    else if (auto vecType = as<VectorExpressionType>(baseType))
    {
        return CheckSimpleSubscriptExpr(subscriptExpr, vecType->getElementType());
    }
    else if (auto matType = as<MatrixExpressionType>(baseType))
    {
        // TODO(tfoley): We shouldn't go and recompute
        // row types over and over like this... :(
        auto rowType = createVectorType(matType->getElementType(), matType->getColumnCount());

        return CheckSimpleSubscriptExpr(subscriptExpr, rowType);
    }

    // Default behavior is to look at all available `__subscript`
    // declarations on the type and try to call one of them.

    auto operatorName = getSubscriptOperatorName(m_astBuilder);

    LookupResult lookupResult = lookUpMember(
        m_astBuilder,
        this,
        operatorName,
        baseType,
        m_outerScope,
        LookupMask::Default,
        LookupOptions::NoDeref);
    bool diagnosed = false;
    lookupResult =
        filterLookupResultByVisibilityAndDiagnose(lookupResult, subscriptExpr->loc, diagnosed);
    if (!lookupResult.isValid())
    {
        if (!diagnosed)
        {
            if (!maybeDiagnoseAmbiguousReference(baseExpr))
            {
                getSink()->diagnose(
                    Diagnostics::SubscriptNonArray{.type = baseType, .expr = subscriptExpr});
            }
        }
        return CreateErrorExpr(subscriptExpr);
    }
    auto subscriptFuncExpr = createLookupResultExpr(
        operatorName,
        lookupResult,
        subscriptExpr->baseExpression,
        subscriptExpr->loc,
        subscriptExpr);

    InvokeExpr* subscriptCallExpr = m_astBuilder->create<InvokeExpr>();
    subscriptCallExpr->loc = subscriptExpr->loc;
    subscriptCallExpr->functionExpr = subscriptFuncExpr;
    subscriptCallExpr->arguments.addRange(subscriptExpr->indexExprs);
    subscriptCallExpr->argumentDelimeterLocs.addRange(subscriptExpr->argumentDelimeterLocs);

    auto checkedCallExpr = CheckInvokeExprWithCheckedOperands(subscriptCallExpr);

    if (m_parentDifferentiableAttr)
    {
        if (auto checkedInvokeExpr = as<InvokeExpr>(checkedCallExpr))
        {
            // Register types for final resolved invoke arguments again.
            for (auto& arg : checkedInvokeExpr->arguments)
                maybeRegisterDifferentiableType(m_astBuilder, arg->type.type);

            registerAssociatedMethodsForSelectedCallable(this, checkedInvokeExpr->functionExpr);
        }
    }

    return checkedCallExpr;
}

Expr* SemanticsExprVisitor::visitParenExpr(ParenExpr* expr)
{
    auto base = expr->base;
    base = CheckTerm(base);

    expr->base = base;
    expr->type = base->type;
    expr->abstractStorageAccess = base->abstractStorageAccess;
    return expr;
}

Expr* SemanticsExprVisitor::visitTupleExpr(TupleExpr* expr)
{
    List<Type*> elementTypes;
    for (auto& element : expr->elements)
    {
        element = CheckTerm(element);
        auto elementType = element->type.type;
        if (auto concreteTypePack = as<ConcreteTypePack>(elementType))
        {
            // We need to flatten the type pack into a tuple type
            for (Index i = 0; i < concreteTypePack->getTypeCount(); i++)
            {
                elementTypes.add(concreteTypePack->getElementType(i));
            }
        }
        else
        {
            elementTypes.add(element->type.type);
        }
    }
    expr->type = m_astBuilder->getTupleType(elementTypes.getArrayView());
    return expr;
}

void SemanticsVisitor::maybeDiagnoseConstVariableAssignment(Expr* expr)
{
    // We will try to handle expressions of the form:
    //
    //      e ::= "this"
    //          | e . name
    //          | e [ expr ]
    //
    // We will unwrap the `e.name` and `e[expr]` cases in a loop.
    Expr* e = expr;
    for (;;)
    {
        if (auto memberExpr = as<MemberExpr>(e))
        {
            e = memberExpr->baseExpression;
        }
        else if (auto subscriptExpr = as<IndexExpr>(e))
        {
            e = subscriptExpr->baseExpression;
        }
        else if (as<ThisExpr>(e))
        {
            break;
        }
        else
        {
            return;
        }
    }

    // Check if we're trying to assign to a non-l-value (const variable, immutable member, etc.)
    if (!expr->type.isLeftValue)
    {
        getSink()->diagnose(Diagnostics::AttemptingToAssignToConstVariable{.expr = expr});
    }
}

static AbstractStorageUse _getDirectAssignmentValueUse(
    ASTBuilder* astBuilder,
    Expr* destination);

Expr* SemanticsVisitor::checkAssignWithCheckedOperands(AssignExpr* expr)
{
    if (expr->right->type.isWriteOnly)
        getSink()->diagnose(Diagnostics::ReadingFromWriteOnly{.expr = expr->right});

    auto assignedValueUse = _getDirectAssignmentValueUse(m_astBuilder, expr->left);
    expr->left = checkAbstractStorageUse(expr->left, AbstractStorageUse::WriteValue);
    expr->left = maybeOpenRef(expr->left);
    auto type = expr->left->type;
    if (auto atomicType = as<AtomicType>(type))
    {
        type = atomicType->getElementType();
    }
    auto right = maybeOpenRef(expr->right);
    expr->right = coerceAndFinalizeForStorageUse(
        CoercionSite::Assignment,
        type,
        right,
        getSink(),
        assignedValueUse);

    // Track reassignment of `VarDecl`s for single-assignment detection.
    // After reassignment, `maybeMoveTemp` will fall through to `moveTemp`
    // (creating a fresh temporary) instead of reusing the variable directly.
    if (auto varExpr = as<VarExpr>(expr->left))
    {
        if (auto varDecl = as<VarDecl>(varExpr->declRef.getDecl()))
        {
            // Skip module-level vars: they can be shared across compilations
            // (e.g. glsl.meta.slang builtins) and must not be mutated with
            // per-compile-arena modifiers.
            if (!as<LetDecl>(varDecl))
            {
                if (!as<ModuleDecl>(varDecl->parentDecl))
                {
                    if (!varDecl->hasModifier<VarReassignedModifier>())
                        addModifier(varDecl, m_astBuilder->create<VarReassignedModifier>());
                }
            }
        }
    }

    if (!expr->left->type.isLeftValue)
    {
        if (as<ErrorType>(type))
        {
            // Don't report an l-value issue on an erroneous expression
        }
        else
        {
            // Provide a more helpful diagnostic about const variable assignment
            maybeDiagnoseConstVariableAssignment(expr->left);

            getSink()->diagnose(Diagnostics::AssignNonLvalue{.expr = expr});
        }
    }
    expr->type = type;
    expr->abstractStorageAccess = AbstractStorageAccessPath::makeAssignmentResult(
        m_astBuilder,
        expr->left->abstractStorageAccess);
    return expr;
}

AbstractStorageUse getAbstractStorageUseForParameter(ParamPassingMode mode)
{
    switch (mode)
    {
    case ParamPassingMode::In:
        return AbstractStorageUse::ReadValue;
    case ParamPassingMode::Out:
        return AbstractStorageUse::WriteValue;
    case ParamPassingMode::BorrowIn:
        return AbstractStorageUse::BorrowValue;
    case ParamPassingMode::BorrowInOut:
        return AbstractStorageUse::ReadWriteValue;
    case ParamPassingMode::Ref:
        return AbstractStorageUse::RequireAddress;
    default:
        SLANG_UNEXPECTED("unhandled parameter-passing mode");
        UNREACHABLE_RETURN(AbstractStorageUse::ReadValue);
    }
}

AbstractStorageUse getAbstractStorageUseForSetterNewValue(ParamInfo const& paramInfo)
{
    if (paramInfo.mode == ParamPassingMode::BorrowIn)
    {
        if (isNonCopyableType(paramInfo.type))
            return AbstractStorageUse::RequireAddress;
    }
    return getAbstractStorageUseForParameter(paramInfo.mode);
}

AbstractStorageUse getAbstractStorageUseForAccessorReceiver(
    DeclRef<AccessorDecl> accessorDeclRef,
    ParamPassingMode mode)
{
    SLANG_RELEASE_ASSERT(accessorDeclRef);
    if (accessorDeclRef.as<RefAccessorDecl>())
    {
        if (mode == ParamPassingMode::BorrowInOut)
            return AbstractStorageUse::RequireAddress;
    }
    return getAbstractStorageUseForParameter(mode);
}

AbstractStorageUse getAbstractStorageUseForImplicitCastOperand(ParamPassingMode mode)
{
    switch (mode)
    {
    case ParamPassingMode::Out:
        return AbstractStorageUse::WriteProjectedValue;
    case ParamPassingMode::BorrowInOut:
        return AbstractStorageUse::ModifyProjectedValue;
    default:
        SLANG_UNEXPECTED("parameter mode does not support implicit-cast writeback");
        UNREACHABLE_RETURN(AbstractStorageUse::ReadValue);
    }
}

AbstractStorageUse getAbstractStorageUseForExplicitReference(QualType referenceType)
{
    SLANG_RELEASE_ASSERT(as<ExplicitRefType>(referenceType.type));
    return referenceType.isLeftValue ? AbstractStorageUse::RequireAddress
                                     : AbstractStorageUse::ReadValue;
}

AbstractStorageAccessPath* AbstractStorageAccessPath::_allocate(ASTBuilder* astBuilder)
{
    SLANG_RELEASE_ASSERT(astBuilder);
    auto memory = astBuilder->getArena().allocate(sizeof(AbstractStorageAccessPath));
    return new (memory) AbstractStorageAccessPath();
}

AbstractStorageAccess AbstractStorageAccessPath::makeDirectStorage(
    ASTBuilder* astBuilder,
    Expr* receiver,
    DeclRef<ContainerDecl> storageDeclRef,
    SourceLoc location)
{
    SLANG_RELEASE_ASSERT(storageDeclRef);
    auto path = _allocate(astBuilder);
    path->m_kind = AbstractStorageAccessPathKind::DirectStorage;
    path->m_expression = receiver;
    path->m_storageDeclRef = storageDeclRef;
    path->m_location = location;
    return path;
}

AbstractStorageAccess AbstractStorageAccessPath::makeProjection(
    ASTBuilder* astBuilder,
    AbstractStorageAccess baseAccess,
    AbstractStorageProjectionMode mode)
{
    if (!baseAccess)
        return nullptr;

    auto path = _allocate(astBuilder);
    path->m_kind = AbstractStorageAccessPathKind::Projection;
    path->m_firstAccess = baseAccess;
    path->m_projectionMode = mode;
    return path;
}

AbstractStorageAccess AbstractStorageAccessPath::makeFixedOperandUse(
    ASTBuilder* astBuilder,
    AbstractStorageAccess operandAccess,
    AbstractStorageUse operandUse)
{
    if (!operandAccess)
        return nullptr;

    auto path = _allocate(astBuilder);
    path->m_kind = AbstractStorageAccessPathKind::FixedOperandUse;
    path->m_firstAccess = operandAccess;
    path->m_fixedOperandUse = operandUse;
    return path;
}

AbstractStorageAccess AbstractStorageAccessPath::makeOpenedExistential(
    ASTBuilder* astBuilder,
    Expr* originalExpression)
{
    SLANG_RELEASE_ASSERT(originalExpression);
    auto path = _allocate(astBuilder);
    path->m_kind = AbstractStorageAccessPathKind::OpenedExistential;
    path->m_expression = originalExpression;
    return path;
}

AbstractStorageAccess AbstractStorageAccessPath::makeSequence(
    ASTBuilder* astBuilder,
    AbstractStorageAccess firstAccess,
    AbstractStorageAccess secondAccess,
    AbstractStorageSequenceResult result)
{
    if (!firstAccess)
    {
        if (result == AbstractStorageSequenceResult::SecondAccess)
            return secondAccess;
        return makeFixedOperandUse(astBuilder, secondAccess, AbstractStorageUse::ReadValue);
    }
    if (!secondAccess)
    {
        if (result == AbstractStorageSequenceResult::FirstAccess)
            return firstAccess;
        return makeFixedOperandUse(astBuilder, firstAccess, AbstractStorageUse::ReadValue);
    }

    auto path = _allocate(astBuilder);
    path->m_kind = AbstractStorageAccessPathKind::Sequence;
    path->m_firstAccess = firstAccess;
    path->m_secondAccess = secondAccess;
    path->m_sequenceResult = result;
    return path;
}

AbstractStorageAccess AbstractStorageAccessPath::makeAssignmentResult(
    ASTBuilder* astBuilder,
    AbstractStorageAccess destinationAccess)
{
    if (!destinationAccess)
        return nullptr;

    auto path = _allocate(astBuilder);
    path->m_kind = AbstractStorageAccessPathKind::AssignmentResult;
    path->m_firstAccess = destinationAccess;
    return path;
}

AbstractStorageUse getProjectedAbstractStorageUse(AbstractStorageUse use)
{
    switch (use)
    {
    case AbstractStorageUse::DiscardResult:
        return AbstractStorageUse::DiscardResult;
    case AbstractStorageUse::ReadValue:
        return AbstractStorageUse::ReadValue;
    case AbstractStorageUse::WriteValue:
    case AbstractStorageUse::ReadWriteValue:
    case AbstractStorageUse::WriteProjectedValue:
    case AbstractStorageUse::ModifyProjectedValue:
        return AbstractStorageUse::ModifyProjectedValue;
    case AbstractStorageUse::BorrowValue:
    case AbstractStorageUse::BorrowProjectedValue:
        return AbstractStorageUse::BorrowProjectedValue;
    case AbstractStorageUse::RequireAddress:
        return AbstractStorageUse::RequireAddress;
    default:
        SLANG_UNEXPECTED("unhandled abstract-storage use");
        UNREACHABLE_RETURN(AbstractStorageUse::ReadValue);
    }
}

/// Returns whether `use` can replace all or part of the abstract-storage value.
static bool _useWritesAbstractStorage(AbstractStorageUse use)
{
    switch (use)
    {
    case AbstractStorageUse::WriteValue:
    case AbstractStorageUse::WriteProjectedValue:
    case AbstractStorageUse::ReadWriteValue:
    case AbstractStorageUse::ModifyProjectedValue:
        return true;

    case AbstractStorageUse::DiscardResult:
    case AbstractStorageUse::ReadValue:
    case AbstractStorageUse::BorrowValue:
    case AbstractStorageUse::RequireAddress:
    case AbstractStorageUse::BorrowProjectedValue:
        return false;

    default:
        SLANG_UNEXPECTED("unhandled abstract-storage use");
        UNREACHABLE_RETURN(false);
    }
}

/// Returns the use imposed on a projection's base.
///
/// Fields and fixed swizzles can update abstract storage by reading the complete value and writing
/// it back. A projection that lowering cannot retain in that representation instead requires a
/// direct address. Reads and borrows do not need that distinction.
static AbstractStorageUse _getProjectionBaseUse(
    AbstractStorageUse use,
    bool requiresDirectAddress)
{
    if (requiresDirectAddress)
    {
        if (_useWritesAbstractStorage(use))
            return AbstractStorageUse::RequireAddress;
    }
    return getProjectedAbstractStorageUse(use);
}

static bool _checkAbstractStorageAccess(
    SemanticsVisitor* semantics,
    AbstractStorageAccess access,
    AbstractStorageUse use);

/// Checks one accessor call and the access path that supplies its receiver.
///
/// `_checkAbstractStorageAccessor()` interprets the accessor's effective receiver mode to decide
/// how the call consumes `receiverExpr`. It validates that use first and then applies the same
/// abstract-storage rules recursively to any property or subscript reference inside `receiverExpr`.
static bool _checkAbstractStorageAccessor(
    SemanticsVisitor* semantics,
    Expr* receiverExpr,
    DeclRef<AccessorDecl> accessorDeclRef,
    SourceLoc location)
{
    SLANG_RELEASE_ASSERT(accessorDeclRef);

    auto thisParamInfo = semantics->findEffectiveThisParamInfo(accessorDeclRef);
    if (!thisParamInfo)
        return true;

    if (!semantics->validateCallableReceiver(
            receiverExpr,
            accessorDeclRef,
            location))
    {
        return false;
    }

    return _checkAbstractStorageAccess(
        semantics,
        receiverExpr->abstractStorageAccess,
        getAbstractStorageUseForAccessorReceiver(accessorDeclRef, thisParamInfo->mode));
}

/// Checks every accessor call in the plan used to consume one property or subscript reference.
static bool _checkAbstractStorageAccessPlan(
    SemanticsVisitor* semantics,
    Expr* receiverExpr,
    DeclRef<ContainerDecl> storageDeclRef,
    AbstractStorageUse use,
    SourceLoc location)
{
    auto plan = getAbstractStorageAccessPlan(
        semantics->getASTBuilder(),
        storageDeclRef,
        use);
    auto checkAccessorReceiver = [&](DeclRef<AccessorDecl> accessorDeclRef)
    {
        return _checkAbstractStorageAccessor(
            semantics,
            receiverExpr,
            accessorDeclRef,
            location);
    };

    switch (plan.getKind())
    {
    case AbstractStorageAccessPlanKind::Invalid:
        {
            auto storageDecl = storageDeclRef.getDecl();
            if (!as<PropertyDecl>(storageDecl))
                SLANG_RELEASE_ASSERT(as<SubscriptDecl>(storageDecl));
            auto storageKind = semantics->getASTBuilder()->getNamePool()->getName(
                as<PropertyDecl>(storageDecl) ? "property" : "subscript");
            switch (plan.getFailure())
            {
            case AbstractStorageAccessFailure::CannotRead:
                semantics->getSink()->diagnose(Diagnostics::AbstractStorageCannotBeRead{
                    .storageKind = storageKind,
                    .location = location});
                break;

            case AbstractStorageAccessFailure::CannotWrite:
                semantics->getSink()->diagnose(Diagnostics::AbstractStorageCannotBeWritten{
                    .storageKind = storageKind,
                    .location = location});
                break;

            case AbstractStorageAccessFailure::CannotProvideAddress:
                semantics->getSink()->diagnose(Diagnostics::AbstractStorageHasNoAddress{
                    .storageKind = storageKind,
                    .location = location});
                break;

            default:
                SLANG_UNEXPECTED("unhandled abstract-storage access failure");
            }
            return false;
        }

    case AbstractStorageAccessPlanKind::ReadValue:
        return checkAccessorReceiver(plan.getReadAccessor());

    case AbstractStorageAccessPlanKind::WriteValue:
        return checkAccessorReceiver(plan.getWriteAccessor().as<AccessorDecl>());

    case AbstractStorageAccessPlanKind::UseAddress:
        return checkAccessorReceiver(plan.getAddressAccessor().as<AccessorDecl>());

    case AbstractStorageAccessPlanKind::ReadThenWriteBack:
        if (!checkAccessorReceiver(plan.getReadAccessor()))
            return false;
        return checkAccessorReceiver(plan.getWriteAccessor().as<AccessorDecl>());

    default:
        SLANG_UNEXPECTED("unhandled abstract-storage access plan");
        UNREACHABLE_RETURN(false);
    }
}

/// Validates the abstract-storage path carried by a checked expression.
///
/// For each node, this interpreter either selects accessor calls or maps the enclosing use to one
/// or two earlier path segments. The exhaustive switch is the only semantic interpreter for this
/// representation. The expression-checking routine for each path node calls the corresponding
/// `AbstractStorageAccessPath::make*()` factory to record that operation, so this interpreter does
/// not need to rediscover expression kinds.
static bool _checkAbstractStorageAccess(
    SemanticsVisitor* semantics,
    AbstractStorageAccess access,
    AbstractStorageUse use)
{
    if (!access)
        return true;

    auto path = access;
    switch (path->getKind())
    {
    case AbstractStorageAccessPathKind::DirectStorage:
        return _checkAbstractStorageAccessPlan(
            semantics,
            path->getStorageReceiver(),
            path->getStorageDeclRef(),
            use == AbstractStorageUse::DiscardResult ? AbstractStorageUse::ReadValue : use,
            path->getStorageLocation());

    case AbstractStorageAccessPathKind::Projection:
        {
            const bool requiresDirectAddress =
                path->getProjectionMode() ==
                AbstractStorageProjectionMode::RequireAddressForWrite;
            return _checkAbstractStorageAccess(
                semantics,
                path->getOperandAccess(),
                _getProjectionBaseUse(use, requiresDirectAddress));
        }

    case AbstractStorageAccessPathKind::FixedOperandUse:
        return _checkAbstractStorageAccess(
            semantics,
            path->getOperandAccess(),
            path->getFixedOperandUse());

    case AbstractStorageAccessPathKind::OpenedExistential:
        if (use == AbstractStorageUse::RequireAddress)
        {
            semantics->getSink()->diagnose(Diagnostics::InterfaceValueHasNoAddress{
                .expression = path->getOpenedExistentialOriginalExpression()});
            return false;
        }

        // The `LetExpr` that stabilizes an opened existential owns the read from its initializer.
        // `originalExpr` names only the destination used when lowering writes a modified concrete
        // value back into the existential. Validating it for a read here would check the same
        // getter twice; a non-writing use has no additional access to validate.
        if (!_useWritesAbstractStorage(use))
            return true;
        return _checkAbstractStorageAccess(
            semantics,
            path->getOpenedExistentialOriginalExpression()->abstractStorageAccess,
            AbstractStorageUse::WriteValue);

    case AbstractStorageAccessPathKind::Sequence:
        {
            const bool firstIsResult =
                path->getSequenceResult() == AbstractStorageSequenceResult::FirstAccess;
            bool firstIsValid = _checkAbstractStorageAccess(
                semantics,
                path->getFirstAccess(),
                firstIsResult ? use : AbstractStorageUse::ReadValue);
            bool secondIsValid = _checkAbstractStorageAccess(
                semantics,
                path->getSecondAccess(),
                firstIsResult ? AbstractStorageUse::ReadValue : use);
            if (!firstIsValid)
                return false;
            return secondIsValid;
        }

    case AbstractStorageAccessPathKind::AssignmentResult:
        // Assignment has already written the destination. Its l-value result delegates any later
        // use to that destination, while discarding the result requires no additional access.
        if (use == AbstractStorageUse::DiscardResult)
            return true;
        return _checkAbstractStorageAccess(
            semantics,
            path->getOperandAccess(),
            use);

    default:
        SLANG_UNEXPECTED("unhandled abstract-storage access path");
        UNREACHABLE_RETURN(false);
    }
}

Expr* SemanticsVisitor::checkAbstractStorageUse(Expr* expr, AbstractStorageUse use)
{
    if (!expr)
        return expr;
    if (IsErrorExpr(expr))
        return expr;

    if (!_checkAbstractStorageAccess(this, expr->abstractStorageAccess, use))
        return CreateErrorExpr(expr);
    return expr;
}

Expr* SemanticsVisitor::checkAndResolveExpressionWithoutFinalization(Expr* uncheckedExpr)
{
    auto checkedTerm = CheckTerm(uncheckedExpr);

    // Expression checking requires one selected value rather than an unresolved overload set.
    return maybeResolveOverloadedExpr(
        checkedTerm,
        LookupMask::Default,
        getSink());
}

Expr* SemanticsVisitor::checkExpressionWithoutStorageUse(Expr* uncheckedExpr)
{
    auto checkedExpr = checkAndResolveExpressionWithoutFinalization(uncheckedExpr);

    // Overload resolution has now selected one expression path.
    // `checkExpressionWithoutStorageUse()` performs its sole finalization. It finalizes a
    // function-typed result as a selected callable even when
    // its use is delayed, as in `let f = value.method`; finalizing it as an ordinary value would
    // lose the receiver validation that belongs to that selection.
    checkedExpr = finalizeSelectedExpression(
        checkedExpr,
        getSelectedExpressionUseForType(checkedExpr->type.type),
        checkedExpr->loc);

    // `checkExpressionWithoutStorageUse()` leaves abstract-storage validation to the invoking
    // context. `CheckExpr` consumes the result as a value; assignment, return, and argument
    // checking can instead apply their required storage use.
    return checkedExpr;
}

Expr* SemanticsVisitor::CheckExpr(Expr* uncheckedExpr)
{
    auto checkedExpr = checkExpressionWithoutStorageUse(uncheckedExpr);
    return checkAbstractStorageUse(checkedExpr, AbstractStorageUse::ReadValue);
}

Expr* SemanticsVisitor::checkExpressionForDiscard(Expr* uncheckedExpr)
{
    auto checkedExpr = checkExpressionWithoutStorageUse(uncheckedExpr);
    return checkAbstractStorageUse(checkedExpr, AbstractStorageUse::DiscardResult);
}

/// Returns the storage use required for the value assigned directly to `destination`.
///
/// When a setter's `newValue` parameter uses `BorrowIn`, `_getDirectAssignmentValueUse()` returns
/// the use that preserves any addressable `ref` path on the assigned expression. A sequence
/// delegates to the access designated as its result. Every other path kind assigns through a
/// projection or ordinary storage and consumes an r-value.
static AbstractStorageUse _getDirectAssignmentValueUse(
    ASTBuilder* astBuilder,
    AbstractStorageAccess access)
{
    SLANG_RELEASE_ASSERT(astBuilder);
    if (!access)
        return AbstractStorageUse::ReadValue;

    auto path = access;
    switch (path->getKind())
    {
    case AbstractStorageAccessPathKind::DirectStorage:
        {
            auto writePlan = getAbstractStorageAccessPlan(
                astBuilder,
                path->getStorageDeclRef(),
                AbstractStorageUse::WriteValue);
            if (writePlan.getKind() != AbstractStorageAccessPlanKind::WriteValue)
                return AbstractStorageUse::ReadValue;

            auto newValueParam =
                getSetterNewValueParamInfo(astBuilder, writePlan.getWriteAccessor());
            return getAbstractStorageUseForSetterNewValue(newValueParam);
        }

    case AbstractStorageAccessPathKind::Sequence:
        return _getDirectAssignmentValueUse(
            astBuilder,
            path->getSequenceResult() == AbstractStorageSequenceResult::FirstAccess
                ? path->getFirstAccess()
                : path->getSecondAccess());

    case AbstractStorageAccessPathKind::AssignmentResult:
        return _getDirectAssignmentValueUse(astBuilder, path->getOperandAccess());

    default:
        return AbstractStorageUse::ReadValue;
    }
}

static AbstractStorageUse _getDirectAssignmentValueUse(
    ASTBuilder* astBuilder,
    Expr* destination)
{
    SLANG_RELEASE_ASSERT(destination);
    return _getDirectAssignmentValueUse(astBuilder, destination->abstractStorageAccess);
}

Expr* SemanticsExprVisitor::visitAssignExpr(AssignExpr* expr)
{
    // A function-typed assignment destination names writable storage rather than a callable to
    // select. `finalizeSelectedExpression()` therefore resolves it as a value while leaving its
    // storage use to assignment.
    expr->left = checkAndResolveExpressionWithoutFinalization(expr->left);
    expr->left =
        finalizeSelectedExpression(expr->left, SelectedExpressionUse::Value, SourceLoc());
    expr->right = CheckTerm(expr->right);

    return checkAssignWithCheckedOperands(expr);
}

/// Returns whether a writable implicit conversion can translate between two scalar types.
static bool _canLValueCoerceScalarType(Type* fromType, Type* toType)
{
    auto fromBasicType = as<BasicExpressionType>(fromType);
    auto toBasicType = as<BasicExpressionType>(toType);

    if (!fromBasicType)
        return false;
    if (!toBasicType)
        return false;

    const auto& fromTypeInfo = BaseTypeInfo::getInfo(fromBasicType->getBaseType());
    const auto& toTypeInfo = BaseTypeInfo::getInfo(toBasicType->getBaseType());

    // Writable implicit casts preserve storage size so writeback can reinterpret the same bytes.
    // The current representation supports only same-size integer casts; different sizes would
    // require a separate temporary and a conversion in each direction.
    if (fromTypeInfo.sizeInBytes != toTypeInfo.sizeInBytes)
        return false;
    return (fromTypeInfo.flags & toTypeInfo.flags & BaseTypeInfo::Flag::Integer) != 0;
}

/// Returns whether writable-argument conversion supports this scalar, vector, or matrix type pair.
static bool _canUseImplicitCastTypePairForWritableArgument(Type* fromType, Type* toType)
{
    // By the time `classifyImplicitCastForWritableArgument()` receives `castExpr`, `_coerce()` has
    // selected a valid implicit-conversion overload, so its vector or matrix dimensions are
    // compatible. `_canUseImplicitCastTypePairForWritableArgument()` therefore compares only the
    // scalar representations.
    SLANG_ASSERT(fromType != toType);
    if (fromType->astNodeType == toType->astNodeType)
    {
        if (auto fromMatrixType = as<MatrixExpressionType>(fromType))
        {
            return _canLValueCoerceScalarType(
                fromMatrixType->getElementType(),
                static_cast<MatrixExpressionType*>(toType)->getElementType());
        }
        else if (auto fromVectorType = as<VectorExpressionType>(fromType))
        {
            return _canLValueCoerceScalarType(
                fromVectorType->getScalarType(),
                static_cast<VectorExpressionType*>(toType)->getScalarType());
        }
    }
    return _canLValueCoerceScalarType(fromType, toType);
}

WritableImplicitCastKind SemanticsVisitor::classifyImplicitCastForWritableArgument(
    ImplicitCastExpr* castExpr)
{
    SLANG_RELEASE_ASSERT(castExpr);
    SLANG_RELEASE_ASSERT(castExpr->arguments.getCount() == 1);

    auto fromType = castExpr->arguments[0]->type.type;
    auto toType = castExpr->type.type;
    if (fromType->equals(toType))
        return WritableImplicitCastKind::UnsupportedTypePair;
    if (!_canUseImplicitCastTypePairForWritableArgument(fromType, toType))
        return WritableImplicitCastKind::UnsupportedTypePair;

    auto functionExpr = castExpr->functionExpr;
    while (auto genericAppExpr = as<GenericAppExpr>(functionExpr))
        functionExpr = genericAppExpr->functionExpr;

    auto declRefExpr = as<DeclRefExpr>(functionExpr);
    auto intrinsicModifier =
        declRefExpr ? declRefExpr->declRef.getDecl()->findModifier<IntrinsicOpModifier>() : nullptr;
    if (!intrinsicModifier)
        return WritableImplicitCastKind::UnsupportedConversion;

    switch (intrinsicModifier->op)
    {
    case kIROp_IntCast:
    case kIROp_BuiltinCast:
        return WritableImplicitCastKind::ReversibleIntrinsic;
    default:
        return WritableImplicitCastKind::UnsupportedConversion;
    }
}


void SemanticsVisitor::compareMemoryQualifierOfParamToArgument(ParamDecl* paramIn, Expr* argIn)
{
    auto arg = as<VarExpr>(argIn);
    if (!paramIn || !arg)
        return;

    auto argDeclRef = arg->declRef;
    if (!argDeclRef)
        return;
    auto argDecl = argDeclRef.getDecl();
    auto argMemMods = argDecl->findModifier<MemoryQualifierSetModifier>();
    if (!argMemMods)
        return;
    uint32_t argQualifiers = argMemMods->getMemoryQualifierBit();

    uint32_t paramQualifiers = 0;
    auto paramMemMods = paramIn->findModifier<MemoryQualifierSetModifier>();
    if (paramMemMods)
        paramQualifiers = paramMemMods->getMemoryQualifierBit();

    if (argQualifiers & MemoryQualifierSetModifier::Flags::kCoherent &&
        !(paramQualifiers & MemoryQualifierSetModifier::Flags::kCoherent))
        getSink()->diagnose(Diagnostics::ArgumentHasMoreMemoryQualifiersThanParam{
            .qualifier = "coherent",
            .arg = arg});
    if (argQualifiers & MemoryQualifierSetModifier::Flags::kReadOnly &&
        !(paramQualifiers & MemoryQualifierSetModifier::Flags::kReadOnly))
        getSink()->diagnose(Diagnostics::ArgumentHasMoreMemoryQualifiersThanParam{
            .qualifier = "readonly",
            .arg = arg});
    if (argQualifiers & MemoryQualifierSetModifier::Flags::kWriteOnly &&
        !(paramQualifiers & MemoryQualifierSetModifier::Flags::kWriteOnly))
        getSink()->diagnose(Diagnostics::ArgumentHasMoreMemoryQualifiersThanParam{
            .qualifier = "writeonly",
            .arg = arg});
    if (argQualifiers & MemoryQualifierSetModifier::Flags::kVolatile &&
        !(paramQualifiers & MemoryQualifierSetModifier::Flags::kVolatile))
        getSink()->diagnose(Diagnostics::ArgumentHasMoreMemoryQualifiersThanParam{
            .qualifier = "volatile",
            .arg = arg});
    // dropping a `restrict` qualifier from arguments is allowed in GLSL with memory qualifiers
}

/// Returns the callable reached by recursively resolving function aliases.
static DeclRef<CallableDecl> _resolveCallableAliases(DeclRef<CallableDecl> declRef)
{
    if (auto funcAliasDecl = as<FuncAliasDecl>(declRef.getDecl()))
    {
        auto target = substituteDeclRef(
                          SubstitutionSet(declRef),
                          getCurrentASTBuilder(),
                          funcAliasDecl->targetDeclRef)
                          .as<CallableDecl>();
        return _resolveCallableAliases(target);
    }

    return declRef;
}

/// Returns the bound instance method transformed by a selected differentiation expression.
///
/// Differentiation IR names a function declaration and does not represent a captured receiver.
/// For example, `__fwd_diff(value.method)` ultimately contains a `MemberExpr` for `value.method`,
/// which lowering would otherwise turn into a `BoundCallable`. In contrast, the unbound spelling
/// `__fwd_diff(Type::method)` contains a `StaticMemberExpr`; its receiver is an explicit parameter
/// of the transformed function and is supported.
static MemberExpr* _getBoundInstanceMethodDifferentiationOperand(
    DifferentiateExpr* expr,
    CallableDecl** outCallableDecl)
{
    SLANG_RELEASE_ASSERT(expr);
    SLANG_RELEASE_ASSERT(outCallableDecl);
    *outCallableDecl = nullptr;

    auto operand = getInnerMostExprFromHigherOrderExpr(expr);
    auto memberExpr = as<MemberExpr>(operand);
    if (!memberExpr)
        return nullptr;

    auto referencedDecl = memberExpr->declRef.getDecl();
    if (auto genericDecl = as<GenericDecl>(referencedDecl))
        referencedDecl = genericDecl->inner;

    auto callableDecl = as<CallableDecl>(referencedDecl);
    if (!callableDecl || isEffectivelyStatic(callableDecl))
        return nullptr;

    *outCallableDecl = callableDecl;
    return memberExpr;
}

/// Diagnoses a selected differentiation operand that captures an instance receiver.
static bool _diagnoseBoundInstanceMethodDifferentiationOperand(
    SemanticsVisitor* semantics,
    DifferentiateExpr* expr)
{
    CallableDecl* callableDecl = nullptr;
    auto memberExpr =
        _getBoundInstanceMethodDifferentiationOperand(expr, &callableDecl);
    if (!memberExpr)
        return false;

    semantics->getSink()->diagnose(
        Diagnostics::NonStaticMemberFunctionNotAllowedAsDiffOperand{
            .func = callableDecl,
            .expr = memberExpr});
    return true;
}

/// Converts `hof(fn)` into the corresponding `fn.hof` lookup.
///
/// Parentheses, checked generic applications, and retained local bindings are transparent to
/// callable selection. The conversion preserves those wrappers, including the evaluation of any
/// retained binding initializers, while replacing their selected callable.
static Expr* convertHigherOrderExprToLookup(
    SemanticsVisitor* visitor,
    HigherOrderInvokeExpr* resultExpr)
{
    Name* lookupName = nullptr;
    if (as<ForwardDifferentiateExpr>(resultExpr))
    {
        lookupName = visitor->getName("fwd_diff");
    }
    else if (as<BackwardDifferentiateExpr>(resultExpr))
    {
        lookupName = visitor->getName("bwd_diff");
    }
    else if (as<ApplyForBwdExpr>(resultExpr))
    {
        lookupName = visitor->getName("apply_bwd");
    }
    else
    {
        visitor->getSink()->diagnose(
            Diagnostics::InternalCompilerError{.location = resultExpr->loc});
        return resultExpr;
    }
    auto selectedBaseFunction =
        getCallableExprThroughTransparentWrappers(resultExpr->baseFunction);
    if (auto hofExpr = as<HigherOrderInvokeExpr>(selectedBaseFunction))
    {
        auto convertedBaseFunction = convertHigherOrderExprToLookup(visitor, hofExpr);
        visitor->replaceSelectedCallableThroughTransparentWrappers(
            resultExpr->baseFunction,
            convertedBaseFunction);
        selectedBaseFunction =
            getCallableExprThroughTransparentWrappers(convertedBaseFunction);
    }

    if (visitor->IsErrorExpr(resultExpr->baseFunction))
        return visitor->CreateErrorExpr(resultExpr);

    if (auto declRefExpr = as<DeclRefExpr>(selectedBaseFunction))
    {
        DeclRef<Decl> callableDeclRef = declRefExpr->declRef;
        if (auto callableRef = callableDeclRef.as<CallableDecl>())
            callableDeclRef = _resolveCallableAliases(callableRef);

        auto funcAsType = DeclRefType::create(visitor->getASTBuilder(), callableDeclRef);

        auto result = lookUpMember(
            visitor->getASTBuilder(),
            visitor,
            lookupName,
            funcAsType,
            visitor->getOuterScope(),
            LookupMask::Default,
            LookupOptions::NoDeref);
        result = visitor->resolveOverloadedLookup(result);
        bool diagnosed = false;
        result =
            visitor->filterLookupResultByVisibilityAndDiagnose(result, resultExpr->loc, diagnosed);
        result = visitor->filterLookupResultByCheckedOptionalAndDiagnose(
            result,
            resultExpr->loc,
            diagnosed);

        if (result.isValid() && !result.isOverloaded())
        {
            if (auto funcAliasDeclRef = result.item.declRef.as<FuncAliasDecl>())
            {
                result.item.declRef = substituteDeclRef(
                                          SubstitutionSet(result.item.declRef),
                                          getCurrentASTBuilder(),
                                          funcAliasDeclRef.getDecl()->targetDeclRef)
                                          .as<CallableDecl>();
            }

            auto lookupResultExpr = visitor->createLookupResultExpr(
                lookupName,
                result,
                declRefExpr,
                resultExpr->loc,
                resultExpr);

            if (selectedBaseFunction == resultExpr->baseFunction)
                return lookupResultExpr;

            visitor->replaceSelectedCallableThroughTransparentWrappers(
                resultExpr->baseFunction,
                lookupResultExpr);
            return resultExpr->baseFunction;
        }
        else if (result.isOverloaded())
        {
            auto overloadedExpr = visitor->getASTBuilder()->create<OverloadedExpr>();
            overloadedExpr->loc = resultExpr->loc;
            visitor->diagnoseAmbiguousReference(overloadedExpr, result);
            return visitor->CreateErrorExpr(resultExpr);
        }
        else
        {
            if (!diagnosed && !visitor->IsErrorExpr(resultExpr->baseFunction))
            {
                visitor->getSink()->diagnose(Diagnostics::NoMemberOfNameInType{
                    .name = lookupName,
                    .type = funcAsType,
                    .expr = resultExpr});
            }
            return visitor->CreateErrorExpr(resultExpr);
        }
    }
    else
    {
        visitor->getSink()->diagnose(
            Diagnostics::InternalCompilerError{.location = resultExpr->loc});
        return resultExpr;
    }
}

/// Rebuilds the callable-transparent wrappers around one distributed overload candidate.
///
/// Consider `__fwd_diff((Type::method))` when `method` is overloaded. The parentheses belong to
/// every distributed candidate, but each candidate needs its own wrapper nodes because overload
/// checking later updates their selected types independently. Wrapper rebuilding reuses a retained
/// `LetExpr`'s initializer and declaration, then recomposes its access path with the new candidate
/// body.
Expr* rebuildCallableTransparentWrappers(
    ASTBuilder* astBuilder,
    Expr* wrappedExpr,
    Expr* replacement)
{
    List<Expr*> wrappers;
    getCallableExprThroughTransparentWrappers(wrappedExpr, &wrappers);

    Expr* result = replacement;
    for (Index i = wrappers.getCount() - 1; i >= 0; --i)
    {
        auto wrapper = wrappers[i];
        if (auto parenExpr = as<ParenExpr>(wrapper))
        {
            auto rebuiltExpr = astBuilder->create<ParenExpr>(*parenExpr);
            rebuiltExpr->base = result;
            rebuiltExpr->type = result->type;
            rebuiltExpr->abstractStorageAccess = result->abstractStorageAccess;
            result = rebuiltExpr;
        }
        else if (auto genericAppExpr = as<GenericAppExpr>(wrapper))
        {
            auto rebuiltExpr = astBuilder->create<GenericAppExpr>(*genericAppExpr);
            rebuiltExpr->functionExpr = result;
            rebuiltExpr->type = result->type;
            rebuiltExpr->abstractStorageAccess = result->abstractStorageAccess;
            result = rebuiltExpr;
        }
        else
        {
            auto letExpr = as<LetExpr>(wrapper);
            SLANG_RELEASE_ASSERT(letExpr);
            auto rebuiltExpr = astBuilder->create<LetExpr>(*letExpr);
            rebuiltExpr->body = result;
            rebuiltExpr->type = result->type;
            _setLocalBindingStorageAccess(rebuiltExpr);
            result = rebuiltExpr;
        }
    }
    return result;
}

// Peel implicit casts and parentheses from an expression.
static Expr* _peelCastsAndParens(Expr* expr)
{
    for (;;)
    {
        if (!expr)
            return nullptr;
        if (auto castExpr = as<TypeCastExpr>(expr))
        {
            if (castExpr->arguments.getCount() == 1)
            {
                expr = castExpr->arguments[0];
                continue;
            }
        }
        if (auto parenExpr = as<ParenExpr>(expr))
        {
            expr = parenExpr->base;
            continue;
        }
        return expr;
    }
}

// Check whether two expressions refer to the same storage location by
// comparing their structure in lockstep. Handles the receiver object (`this` == `this`), bare
// variable / static-member references, member
// accesses (s.x == s.x, but not s.x == s.y), and subscripts with matching
// constant indices (arr[0] == arr[0], but not arr[0] == arr[1]). Returns
// false for anything it can't prove equal.
static bool _exprsDefinitelyAlias(Expr* a, Expr* b)
{
    a = _peelCastsAndParens(a);
    b = _peelCastsAndParens(b);
    if (!a || !b)
        return false;

    // Same receiver object: `this` vs `this`. There is exactly one `this` in a
    // given method body, so any two `ThisExpr` nodes necessarily refer to the
    // same object; that is why this returns true without comparing them further
    // (there is no per-`this` identity to compare, unlike a named variable).
    // Inside a method an unqualified member `x` is rewritten to
    // `MemberExpr(base=ThisExpr, decl=x)`, so the MemberExpr recursion below
    // bottoms out here on the two `this` bases; this is what still diagnoses
    // `twoInoutInt(x, x)` (i.e. `this.x` aliasing itself). ThisExpr does not
    // derive from DeclRefExpr, so it needs its own case.
    if (as<ThisExpr>(a))
        return as<ThisExpr>(b) != nullptr;

    // Same member of the same base: s.x vs s.x, but not s.x vs t.x. Checked
    // before the bare-DeclRefExpr case below because MemberExpr derives from
    // DeclRefExpr, and that branch would ignore the base object. (DerefMemberExpr
    // for buffer-element member access derives from MemberExpr, so it lands here.)
    if (auto aMember = as<MemberExpr>(a))
    {
        auto bMember = as<MemberExpr>(b);
        if (!bMember)
            return false;
        if (aMember->declRef.getDecl() != bMember->declRef.getDecl())
            return false;
        return _exprsDefinitelyAlias(aMember->baseExpression, bMember->baseExpression);
    }

    // Same bare declaration reference: a bare variable (VarExpr) or static
    // member (StaticMemberExpr), which have no base object to compare.
    if (auto aDeclRef = as<DeclRefExpr>(a))
    {
        auto bDeclRef = as<DeclRefExpr>(b);
        // A bare DeclRefExpr is a VarExpr or StaticMemberExpr — any DeclRefExpr
        // that is not a (non-static) MemberExpr, which was already handled above.
        bool bIsBareDeclRef = bDeclRef && !as<MemberExpr>(b);
        return bIsBareDeclRef && aDeclRef->declRef.getDecl() == bDeclRef->declRef.getDecl();
    }

    // Same element of the same base: arr[0] vs arr[0].
    if (auto aIndex = as<IndexExpr>(a))
    {
        auto bIndex = as<IndexExpr>(b);
        if (!bIndex)
            return false;
        if (aIndex->indexExprs.getCount() != 1 || bIndex->indexExprs.getCount() != 1)
            return false;
        // Only compare constant integer indices; dynamic indices are
        // conservatively treated as non-aliasing (may be different).
        auto aLit = as<IntegerLiteralExpr>(aIndex->indexExprs[0]);
        auto bLit = as<IntegerLiteralExpr>(bIndex->indexExprs[0]);
        if (!aLit || !bLit || aLit->value != bLit->value)
            return false;
        return _exprsDefinitelyAlias(aIndex->baseExpression, bIndex->baseExpression);
    }

    return false;
}

static bool _isOutInOutOrRefParam(FuncType* funcType, Index paramIndex)
{
    auto paramType = funcType->getParamTypeWithModeWrapper(paramIndex);
    return as<OutParamTypeBase>(paramType) || as<RefParamType>(paramType);
}

static const char* _getDirectionString(FuncType* funcType, Index paramIndex)
{
    auto paramType = funcType->getParamTypeWithModeWrapper(paramIndex);
    if (as<OutParamType>(paramType))
        return "out";
    if (as<BorrowInOutParamType>(paramType))
        return "inout";
    if (as<RefParamType>(paramType))
        return "ref";
    return "in";
}

void SemanticsVisitor::_checkAliasedOutArguments(
    InvokeExpr* invoke,
    FuncType* funcType,
    List<DeclRef<ParamDecl>> const& paramDeclRefs)
{
    // Operator expressions (compound assignments like `a += a`, prefix/postfix
    // `++a`, etc.) desugar into function calls with `inout` parameters but have
    // well-defined semantics even when the operand appears on both sides.
    // Skip the aliasing check for those.
    if (as<OperatorExpr>(invoke))
        return;

    Index argCount = invoke->arguments.getCount();
    Index paramCount = funcType->getParamCount();
    Index checkCount = Math::Min(argCount, paramCount);

    // For each pair of arguments, check if they refer to the same storage
    // and at least one parameter is out/inout/ref. We compare expressions
    // structurally: bare variables (a == a), member accesses (s.x == s.x
    // but not s.x == s.y), and constant-index subscripts (arr[0] == arr[0]
    // but not arr[0] == arr[1]).
    for (Index i = 0; i < checkCount; ++i)
    {
        bool iIsOut = _isOutInOutOrRefParam(funcType, i);

        for (Index j = i + 1; j < checkCount; ++j)
        {
            // At least one of the two must be out/inout/ref.
            bool jIsOut = _isOutInOutOrRefParam(funcType, j);
            if (!iIsOut && !jIsOut)
                continue;

            if (!_exprsDefinitelyAlias(invoke->arguments[i], invoke->arguments[j]))
                continue;

            // Both arguments refer to the same variable and at least
            // one is out/inout/ref. Put the out/inout/ref parameter
            // first in the diagnostic for clarity.
            Index first = iIsOut ? i : j;
            Index second = iIsOut ? j : i;

            Name* paramNameFirst = nullptr;
            Name* paramNameSecond = nullptr;
            if (paramDeclRefs.getCount() > first)
                paramNameFirst = paramDeclRefs[first].getName();
            if (paramDeclRefs.getCount() > second)
                paramNameSecond = paramDeclRefs[second].getName();

            getSink()->diagnose(Diagnostics::PotentiallyAliasedOutParameter{
                .direction1 = _getDirectionString(funcType, first),
                .param1 = paramNameFirst,
                .direction2 = _getDirectionString(funcType, second),
                .param2 = paramNameSecond,
                .firstArg = invoke->arguments[first]});
            // One diagnostic per outer argument avoids redundant pairwise warnings.
            break;
        }
    }
}

Expr* SemanticsVisitor::CheckInvokeExprWithCheckedOperands(InvokeExpr* expr)
{
    auto rs = ResolveInvoke(expr);
    if (auto invoke = as<InvokeExpr>(rs))
    {
        if (!invoke->functionExpr)
            return rs;

        // if this is still an invoke expression, test arguments passed to inout/out parameter
        // are LValues.
        //
        // A special case that allows us to skip this validation is when `expr` is an identical type
        // cast, e.g. `(T)(funcThatReturnsT())`. In this case `ResolveInvoke(expr)` will simply
        // return the argument expr `funcThatReturnsT()`. And we can skip rerunning any `out` param
        // validation logic on the inner expr.
        if (expr->arguments.getCount() == 1 && invoke == expr->arguments[0])
            return rs;

        if (auto funcType = as<FuncType>(invoke->functionExpr->type))
        {
            if (!funcType->getErrorType()->equals(m_astBuilder->getBottomType()))
            {
                // If the callee throws, make sure we are inside a try clause.
                if (m_enclosingTryClauseType == TryClauseType::None)
                {
                    getSink()->diagnose(
                        Diagnostics::MustUseTryClauseToCallAThrowFunc{.invoke = invoke});
                }
            }

            auto funcDeclRefExpr = as<DeclRefExpr>(invoke->functionExpr);
            List<DeclRef<ParamDecl>> paramDeclRefs;
            if (funcDeclRefExpr)
            {
                if (auto callableDeclRef = funcDeclRefExpr->declRef.as<CallableDecl>())
                {
                    paramDeclRefs =
                        getParametersForCallableSignature(m_astBuilder, callableDeclRef);
                }
            }

            Index paramCount = funcType->getParamCount();

            for (Index pp = 0; pp < paramCount; ++pp)
            {
                auto paramType = funcType->getParamTypeWithModeWrapper(pp);
                Expr* argExpr = nullptr;
                ParamDecl* paramDecl = nullptr;
                if (pp < invoke->arguments.getCount())
                {
                    argExpr = invoke->arguments[pp];
                    if (paramDeclRefs.getCount() > pp)
                        paramDecl = paramDeclRefs[pp].getDecl();
                }
                compareMemoryQualifierOfParamToArgument(paramDecl, argExpr);

                if (as<OutParamTypeBase>(paramType) || as<RefParamType>(paramType))
                {
                    // A writable parameter ordinarily requires an l-value of its exact type. The
                    // branch below also accepts a reversible, same-size integer conversion for an
                    // `out` or `inout` parameter. A `ref` parameter keeps the exact-type rule
                    // because it must preserve the caller's original address.
                    if (argExpr)
                    {
                        if (!argExpr->type.isLeftValue)
                        {
                            auto implicitCastExpr = as<ImplicitCastExpr>(argExpr);
                            auto writableCastKind = WritableImplicitCastKind::UnsupportedConversion;
                            if (implicitCastExpr)
                            {
                                writableCastKind =
                                    classifyImplicitCastForWritableArgument(implicitCastExpr);
                            }

                            // A reversible implicit cast can adapt an `out` or `inout` argument by
                            // writing the call's result back through the cast. A `ref` argument
                            // cannot use this adaptation because it must preserve the original
                            // address, including for atomic operations. The cast operand must also
                            // be an l-value so that the write-back has a destination.
                            bool canUseImplicitCastAsWritableArgument = implicitCastExpr != nullptr;
                            if (canUseImplicitCastAsWritableArgument)
                            {
                                canUseImplicitCastAsWritableArgument =
                                    implicitCastExpr->arguments.getCount() == 1;
                            }
                            if (canUseImplicitCastAsWritableArgument)
                            {
                                canUseImplicitCastAsWritableArgument =
                                    as<OutParamTypeBase>(paramType) != nullptr;
                            }
                            if (canUseImplicitCastAsWritableArgument)
                            {
                                canUseImplicitCastAsWritableArgument =
                                    implicitCastExpr->arguments[0]->type.isLeftValue;
                            }
                            if (canUseImplicitCastAsWritableArgument)
                            {
                                canUseImplicitCastAsWritableArgument =
                                    writableCastKind ==
                                    WritableImplicitCastKind::ReversibleIntrinsic;
                            }

                            if (canUseImplicitCastAsWritableArgument)
                            {
                                // Consider this example:
                                //
                                // ```slang
                                // void update(inout int value);
                                // uint value;
                                // update(value);
                                // ```
                                //
                                // `CheckInvokeExprWithCheckedOperands()` records an
                                // `InOutImplicitCastExpr`. Lowering initializes an `int` temporary
                                // from `value`, calls `update` with that temporary, converts the
                                // result back to `uint`, and writes it to `value`.

                                TypeCastExpr* lValueImplicitCast;

                                // An `out` argument does not need an input conversion, while an
                                // `inout` argument does.
                                if (as<OutType>(paramType))
                                {
                                    lValueImplicitCast =
                                        getASTBuilder()->create<OutImplicitCastExpr>(
                                            *implicitCastExpr);
                                }
                                else
                                {
                                    lValueImplicitCast =
                                        getASTBuilder()->create<InOutImplicitCastExpr>(
                                            *implicitCastExpr);
                                }

                                // `CheckInvokeExprWithCheckedOperands()` records on the wrapper how
                                // lowering must initialize and write back the temporary used for
                                // this writable conversion.
                                invoke->arguments[pp] = lValueImplicitCast;
                            }
                            else if (!as<ErrorType>(argExpr->type))
                            {
                                // An address-of intrinsic gets a more specific diagnostic before
                                // the general writable-argument error.
                                auto funcDeclRef = funcDeclRefExpr
                                                       ? getDeclRef(m_astBuilder, funcDeclRefExpr)
                                                       : DeclRef<Decl>();
                                if (funcDeclRef)
                                {
                                    auto knownBuiltinAttr =
                                        funcDeclRef.getDecl()
                                            ->findModifier<KnownBuiltinAttribute>();
                                    if (knownBuiltinAttr)
                                    {
                                        if (auto constantIntVal =
                                                as<ConstantIntVal>(knownBuiltinAttr->name))
                                        {
                                            if (constantIntVal->getValue() ==
                                                (int)KnownBuiltinDeclName::OperatorAddressOf)
                                            {
                                                getSink()->diagnose(
                                                    Diagnostics::CannotTakeConstantPointers{
                                                        .expr = argExpr});
                                            }
                                        }
                                    }
                                }

                                getSink()->diagnose(Diagnostics::ArgumentExpectedLvalue{
                                    .param = String(pp),
                                    .arg = argExpr});


                                if (implicitCastExpr && implicitCastExpr->arguments.getCount() == 1)
                                {
                                    if (as<RefParamType>(paramType))
                                    {
                                        // A converted `ref` argument would not preserve the
                                        // original storage identity required by atomic operations.
                                        getSink()->diagnose(
                                            Diagnostics::ImplicitCastUsedAsLvalueRef{
                                                .from = implicitCastExpr->arguments[0]->type.type,
                                                .to = implicitCastExpr->type.type,
                                                .expr = argExpr});
                                    }
                                    else if (
                                        writableCastKind ==
                                        WritableImplicitCastKind::UnsupportedTypePair)
                                    {
                                        // Writable conversion currently supports same-size integer
                                        // scalars and same-size vectors or matrices of integers.
                                        getSink()->diagnose(
                                            Diagnostics::ImplicitCastUsedAsLvalueType{
                                                .from = implicitCastExpr->arguments[0]->type.type,
                                                .to = implicitCastExpr->type.type,
                                                .expr = argExpr});
                                    }
                                    else
                                    {
                                        // Every other rejected conversion uses the general
                                        // diagnostic.
                                        getSink()->diagnose(Diagnostics::ImplicitCastUsedAsLvalue{
                                            .from = implicitCastExpr->arguments[0]->type.type,
                                            .to = implicitCastExpr->type.type,
                                            .expr = argExpr});
                                    }
                                }

                                maybeDiagnoseConstVariableAssignment(argExpr);
                            }
                        }
                    }
                    else
                    {
                        // There are two ways we could get here, both involving
                        // a call where the number of argument expressions is
                        // less than the number of parameters on the callee:
                        //
                        // 1. There might be fewer arguments than parameters
                        // because the trailing parameters should be defaulted
                        //
                        // 2. There might be fewer arguments than parameters
                        // because the call is incorrect.
                        //
                        // In case (2) an error would have already been diagnosed,
                        // and we don't want to emit another cascading error here.
                        //
                        // In case (1) this implies the user declared an `out`
                        // or `inout` parameter with a default argument expression.
                        // That should be an error, but it should be detected
                        // on the declaration instead of here at the use site.
                        //
                        // Thus, it makes sense to ignore this case here.
                    }
                }
            }

            // Calls with provably aliased writable arguments have undefined behavior (issue
            // #10699).
            _checkAliasedOutArguments(invoke, funcType, paramDeclRefs);

            if (!IsErrorExpr(invoke))
            {
                if (auto higherOrderInvoke = as<DifferentiateExpr>(invoke->functionExpr))
                {
                    // The overload-distribution path retained every viable candidate. Overload
                    // resolution has now selected one transformed function, so this is the first
                    // point where a bound instance method can be rejected without changing
                    // overload ranking.
                    if (_diagnoseBoundInstanceMethodDifferentiationOperand(
                            this,
                            higherOrderInvoke))
                    {
                        return CreateErrorExpr(invoke);
                    }
                    invoke->functionExpr = convertHigherOrderExprToLookup(this, higherOrderInvoke);
                }
            }
        }
    }
    rs->checked = true;
    return rs;
}


Expr* SemanticsExprVisitor::visitSelectExpr(SelectExpr* expr)
{
    auto result = visitInvokeExpr(expr);
    if (as<ErrorType>(result->type.type))
        return result;
    auto invokeExpr = as<InvokeExpr>(result);
    if (!result)
        return result;
    if (invokeExpr->arguments.getCount() != 3)
        return result;

    if (as<BasicExpressionType>(invokeExpr->arguments[0]->type.type))
    {
        auto newArgs = invokeExpr->arguments;
        expr->arguments.clear();
        expr->arguments = newArgs;
        expr->type = invokeExpr->type;
        return expr;
    }

    if (getParentDifferentiableAttribute())
    {
        // If we are in a differentiable func, issue
        // a diagnostic on use of non short-circuiting select.
        getSink()->diagnose(
            Diagnostics::UseOfNonShortCircuitingOperatorInDiffFunc{.location = expr->loc});
    }
    else
    {
        // For all other functions, we issue a warning for deprecation of vector-typed ?: operator.
        getSink()->diagnose(Diagnostics::UseOfNonShortCircuitingOperator{.location = expr->loc});
    }
    return result;
}

bool SemanticsExprVisitor::isGLSLOperatorScope()
{
    return getShared()->isGLSLOperatorScope();
}

// Decompose a builtin numeric type into (base element type, shape). `outRows`/`outCols`
// describe the shape: both null => scalar, rows set & cols null => vector<rows>, both set =>
// matrix<rows,cols>. Returns false if `type` is not a builtin scalar/vector/matrix.
static bool _getBuiltinCompositeTypeShape(
    Type* type,
    BaseType& outBase,
    IntVal*& outRows,
    IntVal*& outCols)
{
    outRows = nullptr;
    outCols = nullptr;
    Type* elementType = type;
    if (auto vecType = as<VectorExpressionType>(type))
    {
        outRows = vecType->getElementCount();
        elementType = vecType->getElementType();
    }
    else if (auto matType = as<MatrixExpressionType>(type))
    {
        outRows = matType->getRowCount();
        outCols = matType->getColumnCount();
        elementType = matType->getElementType();
    }
    auto basic = as<BasicExpressionType>(elementType);
    if (!basic)
        return false;
    outBase = basic->getBaseType();
    return true;
}

// When both bitwise/shift operands are builtin scalar/vector/matrix types and at least one has a
// floating-point element type, return that floating-point operand's type (such an operation has no
// integer interpretation and is rejected with a dedicated diagnostic). Returns null when either
// operand is non-builtin -- so user-defined `operator OP` (e.g. `operator|(float, MyType)`) and
// generics fall through to overload resolution -- or when neither is floating-point (`int << uint`,
// `bool | bool`).
static Type* _isBuiltinFloatingPointBitwiseOperands(Type* left, Type* right)
{
    BaseType leftBase, rightBase;
    IntVal *leftRows, *leftCols, *rightRows, *rightCols;
    if (!_getBuiltinCompositeTypeShape(left, leftBase, leftRows, leftCols))
        return nullptr;
    if (!_getBuiltinCompositeTypeShape(right, rightBase, rightRows, rightCols))
        return nullptr;
    if ((BaseTypeInfo::getInfo(leftBase).flags & BaseTypeInfo::Flag::FloatingPoint) != 0)
        return left;
    if ((BaseTypeInfo::getInfo(rightBase).flags & BaseTypeInfo::Flag::FloatingPoint) != 0)
        return right;
    return nullptr;
}

// Computes the common element base type for `a OP b`, following the "usual arithmetic
// conversions": float beats int; among floats the larger size wins; among ints the larger
// size wins and on a size tie the unsigned type wins; bool promotes to the other operand.
static BaseType unifyBaseType(BaseType a, BaseType b)
{
    if (a == b)
        return a;
    if (a == BaseType::Bool)
        return b; // bool promotes to the other operand's type
    if (b == BaseType::Bool)
        return a;
    const auto& ia = BaseTypeInfo::getInfo(a);
    const auto& ib = BaseTypeInfo::getInfo(b);
    bool aFloat = (ia.flags & BaseTypeInfo::Flag::FloatingPoint) != 0;
    bool bFloat = (ib.flags & BaseTypeInfo::Flag::FloatingPoint) != 0;
    if (aFloat && bFloat)
        return (ia.sizeInBytes >= ib.sizeInBytes) ? a : b;
    if (aFloat)
        return a; // float beats int
    if (bFloat)
        return b;
    // Both are integers.
    if (ia.sizeInBytes != ib.sizeInBytes)
        return (ia.sizeInBytes > ib.sizeInBytes) ? a : b; // larger size wins (keeps its sign)
    // Same size, differing signedness: the unsigned type wins.
    bool aSigned = (ia.flags & BaseTypeInfo::Flag::Signed) != 0;
    return aSigned ? b : a;
}

Type* SemanticsExprVisitor::substituteElementOfCompositeType(Type* target, Type* newElementType)
{
    if (auto v = as<VectorExpressionType>(target))
        return createVectorType(newElementType, v->getElementCount());
    if (auto m = as<MatrixExpressionType>(target))
        return m_astBuilder
            ->getMatrixType(newElementType, m->getRowCount(), m->getColumnCount(), m->getLayout());
    // Otherwise `target` must be a builtin scalar, whose element is the type itself. This
    // function is only ever called with builtin scalar/vector/matrix operand types; anything
    // else is a caller bug.
    SLANG_RELEASE_ASSERT(as<BasicExpressionType>(target));
    return newElementType;
}

Type* SemanticsExprVisitor::coerceOperandsOfBuiltinBinaryExpr(
    Expr* leftArg,
    Expr* rightArg,
    Expr*& outLeftArg,
    Expr*& outRightArg)
{
    outLeftArg = leftArg;
    outRightArg = rightArg;

    // Same builtin type on both sides: nothing to coerce.
    if (leftArg->type.type->equals(rightArg->type.type))
        return leftArg->type.type;

    // The broadcast result type with the common element base, matching the candidate overload
    // resolution would have selected. Null => not a fast-pathable pair of builtin numeric
    // scalar/vector/matrix operands.
    Type* commonType = getBuiltinArithmeticCommonType(leftArg->type.type, rightArg->type.type);
    if (!commonType)
        return nullptr;
    BaseType commonBase;
    IntVal *cRows, *cCols;
    _getBuiltinCompositeTypeShape(commonType, commonBase, cRows, cCols);
    Type* commonElementType = m_astBuilder->getBuiltinType(commonBase);

    // Coerce each operand to its *own* shape with the common element base, converting only the
    // element type and never the shape. Keeping the operands in their mixed vector/scalar (or
    // matrix/scalar) form preserves the canonical IR that backends optimize -- e.g. a
    // `vector * scalar` stays a two-shape `mul`, which SPIR-V lowers to `OpVectorTimesScalar`
    // rather than a splat followed by a component-wise multiply. Because the common element base
    // is the wider / no-narrowing one, the conversions here are not narrowing, so they do not
    // emit the "implicit conversion not recommended" warning (which would break the
    // warning-fatal core module bootstrap).
    Type* leftTarget = substituteElementOfCompositeType(leftArg->type.type, commonElementType);
    Type* rightTarget = substituteElementOfCompositeType(rightArg->type.type, commonElementType);
    if (!leftArg->type.type->equals(leftTarget))
    {
        auto c = coerceExpressionWithoutFinalization(
            CoercionSite::Argument,
            leftTarget,
            leftArg,
            getSink());
        if (IsErrorExpr(c))
            return nullptr;
        outLeftArg = c;
    }
    if (!rightArg->type.type->equals(rightTarget))
    {
        auto c = coerceExpressionWithoutFinalization(
            CoercionSite::Argument,
            rightTarget,
            rightArg,
            getSink());
        if (IsErrorExpr(c))
            return nullptr;
        outRightArg = c;
    }
    return commonType;
}

SemanticsExprVisitor::BuiltinArithmeticElementFamily SemanticsExprVisitor::
    classifyBuiltinArithmeticElementType(Type* elementType)
{
    if (auto basicType = as<BasicExpressionType>(elementType))
    {
        auto baseType = basicType->getBaseType();
        auto flags = BaseTypeInfo::getInfo(baseType).flags;
        BuiltinArithmeticElementFamily family;
        family.isInteger = (flags & BaseTypeInfo::Flag::Integer) != 0;
        family.isFloat = (flags & BaseTypeInfo::Flag::FloatingPoint) != 0;
        family.isBool = (baseType == BaseType::Bool);
        return family;
    }

    // Not a concrete builtin scalar. Check whether a generic parameter's declared constraint
    // (or, in principle, any other type) conforms to one of the sealed builtin marker
    // interfaces; see the declaration comment on `BuiltinArithmeticElementFamily` for why that
    // is a sound basis for the fast path. `tryGetInterfaceConformanceWitness` goes through
    // `SharedSemanticsContext::tryGetSubtypeWitnessFromCache`, so repeated calls for the same
    // `elementType` -- exactly what a generic type parameter reused across many call sites
    // produces -- are cache hits after the first.
    //
    // A constraint declared `where optional T : I` (`OptionalConstraintModifier`) is not proof
    // that `T` conforms to `I`: the whole point of an optional constraint is that a given
    // instantiation of `T` may or may not satisfy it, and code outside a `if (T is I)` guard
    // must not assume it does (see `isWitnessUncheckedOptional`, which the same "is this
    // conformance actually established here" question already relies on for member lookup).
    // Treating an unchecked optional witness as sufficient here would let a generic function
    // like `compute<T>(T a, T b) where optional T : __BuiltinFloatingPointType { return a + b;
    // }` take the builtin fast path and emit `a + b` verbatim for a `T` that never proved it
    // supports `+`, instead of falling through to overload resolution the way it must.
    auto isProvenConformance = [this](SubtypeWitness* witness)
    { return witness && !isWitnessUncheckedOptional(witness); };

    // Each accessor returns null before the core module is available to search (see
    // `SharedASTBuilder::getBuiltinIntegerInterfaceType` and its siblings); the classification for
    // that family is then left unknown rather than querying conformance against a null interface
    // type.
    auto astBuilder = getASTBuilder();
    BuiltinArithmeticElementFamily family;
    if (auto integerInterface = astBuilder->getBuiltinIntegerInterfaceType())
        family.isInteger =
            isProvenConformance(tryGetInterfaceConformanceWitness(elementType, integerInterface));
    if (auto floatInterface = astBuilder->getBuiltinFloatingPointInterfaceType())
        family.isFloat =
            isProvenConformance(tryGetInterfaceConformanceWitness(elementType, floatInterface));
    // `__BuiltinLogicalType` is implemented by `bool` AND every builtin integer type (it also
    // backs bitwise-operator codegen elsewhere), so proving conformance to it does not prove the
    // element is `bool` -- only `family.isLogical`, not `family.isBool`, may be set here. A
    // generic parameter can never prove `isBool`: there is no sealed marker interface `bool`
    // alone implements, so only the concrete-type branch above (`baseType == BaseType::Bool`)
    // ever sets it.
    if (auto logicalInterface = astBuilder->getBuiltinLogicalInterfaceType())
        family.isLogical =
            isProvenConformance(tryGetInterfaceConformanceWitness(elementType, logicalInterface));
    return family;
}

/// Builds the storage path for operator operands that are consumed as values in source order.
///
/// Consider `left && right`: validation visits `left` before `right`, and each operand is a value
/// read regardless of how the resulting `bool` is consumed. `_makeReadValueOperandSequence()`
/// constructs a path that represents that static validation order; it does not claim that
/// short-circuit evaluation executes `right` at runtime.
static AbstractStorageAccess _makeReadValueOperandSequence(
    ASTBuilder* astBuilder,
    List<Expr*> const& operands)
{
    AbstractStorageAccess result = nullptr;
    for (auto operand : operands)
    {
        auto operandRead = AbstractStorageAccessPath::makeFixedOperandUse(
            astBuilder,
            operand->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
        result = AbstractStorageAccessPath::makeSequence(
            astBuilder,
            result,
            operandRead,
            AbstractStorageSequenceResult::SecondAccess);
    }
    return result;
}

Expr* SemanticsExprVisitor::convertToBuiltinArithmeticOp(InvokeExpr* expr)
{
    // Recognize a builtin arithmetic (`+ - * / %`), comparison (`< > <= >=`), equality
    // (`== !=`), bitwise/shift (`& | ^ << >>`), or unary (`- ! ~`) operator on builtin
    // integer/floating-point/bool scalar, vector, or matrix operands, and rewrite it to a
    // `BuiltinOperatorExpr` (carrying the resolved `BuiltinOperationKind`) for direct IR
    // lowering / constant folding, skipping generic `operator OP` overload resolution. Returns
    // null to leave the expression for normal resolution. The operator-name is mapped to a
    // `BuiltinOperationKind` once (here), and everything downstream keys off the kind.
    //
    // "Builtin" element type is not limited to a concrete `BasicExpressionType`:
    // `classifyBuiltinArithmeticElementType` also recognizes a generic type parameter
    // constrained to a sealed builtin marker interface (`T : __BuiltinFloatingPointType`, as in
    // issue #12458's reproducer), because every legal instantiation of such a parameter is
    // guaranteed to be a concrete builtin scalar too. Without that, an operator on a
    // generic-typed operand falls through to full generic overload resolution on every visible
    // `operator OP` overload -- measured at ~83% of semantic-checking time on that reproducer,
    // because the wrong candidates it rejects each still pay for generic argument inference.

    // Unary prefix operators: `-x` (negate), `!x` (logical-not, bool), `~x` (bitwise-not, int).
    if (as<PrefixExpr>(expr) && expr->arguments.getCount() == 1)
    {
        auto uVarExpr = as<VarExpr>(expr->functionExpr);
        if (!uVarExpr || !uVarExpr->name)
            return nullptr;
        auto uKind = getBuiltinOperationKindFromString(
            getText(uVarExpr->name).getUnownedSlice(),
            OperatorArity::Unary);
        bool isNeg = (uKind == BuiltinOperationKind::Neg);
        bool isLogicalNot = (uKind == BuiltinOperationKind::Not);
        bool isBitNot = (uKind == BuiltinOperationKind::BitNot);
        if (!isNeg && !isLogicalNot && !isBitNot)
            return nullptr;

        auto arg = expr->arguments[0];
        if (!arg->type.type)
            return nullptr;
        Type* uOperandType = arg->type.type;
        // In GLSL operator scope the `glsl` module owns matrix operator semantics, so leave
        // matrix operands to normal resolution (see the binary case for the full rationale).
        if (isGLSLOperatorScope() && as<MatrixExpressionType>(uOperandType))
            return nullptr;
        Type* uElementType = uOperandType;
        if (auto v = as<VectorExpressionType>(uOperandType))
            uElementType = v->getElementType();
        else if (auto m = as<MatrixExpressionType>(uOperandType))
            uElementType = m->getElementType();
        auto uFamily = classifyBuiltinArithmeticElementType(uElementType);
        if (!uFamily.isKnown())
            return nullptr;
        bool uInt = uFamily.isInteger;
        bool uFloat = uFamily.isFloat;
        // Deliberately `isBool` (strict: concrete `bool` only), not `isLogical`: `!` must
        // produce a `bool`-shaped result, and a generic parameter constrained only to
        // `__BuiltinLogicalType` may be instantiated with an integer, for which `isBool` is
        // false (see `BuiltinArithmeticElementFamily`'s field comments) -- correctly declining
        // the fast path here, the same way a concrete non-bool operand already does.
        bool uBool = uFamily.isBool;
        // `-` => signed/float negate; `~` => integer bitwise-not; `!` => bool logical-not.
        bool uEligible = isNeg ? (uInt || uFloat) : (isBitNot ? uInt : /*isLogicalNot*/ uBool);
        if (!uEligible)
        {
            // `~` on a builtin floating-point operand has no integer interpretation; diagnose it
            // with the same error as the binary case (issue #11648) instead of a confusing "no
            // overload for 'operator~'". Non-builtin operands already returned above.
            if (isBitNot && uFloat)
            {
                getSink()->diagnose(Diagnostics::BitwiseOperatorRequiresIntegerOperands{
                    .name = uVarExpr->name,
                    .type = uOperandType,
                    .expr = expr});
                return CreateErrorExpr(expr);
            }
            return nullptr;
        }

        auto node = m_astBuilder->create<BuiltinOperatorExpr>();
        node->op = uKind;
        node->arguments.add(arg);
        node->type = QualType(uOperandType);
        node->loc = expr->loc;
        node->abstractStorageAccess =
            _makeReadValueOperandSequence(m_astBuilder, node->arguments);
        // Register the operand/result types in a differentiable scope regardless of the operator
        // (matching the breadth of the pre-fast-path `visitInvokeExpr`): the operand of a `!`/`~`
        // is not itself differentiable, but `maybeRegisterDifferentiableType` is a no-op for
        // non-differentiable types, so registering unconditionally just preserves the prior
        // behavior for any differentiable operand without special-casing the operator.
        if (m_parentDifferentiableAttr)
        {
            maybeRegisterDifferentiableType(m_astBuilder, arg->type.type, arg->loc);
            maybeRegisterDifferentiableType(m_astBuilder, uOperandType, expr->loc);
        }
        return node;
    }

    // Only an infix binary operator `a OP b`.
    if (!as<InfixExpr>(expr) || expr->arguments.getCount() != 2)
        return nullptr;
    auto varExpr = as<VarExpr>(expr->functionExpr);
    if (!varExpr || !varExpr->name)
        return nullptr;
    auto kind = getBuiltinOperationKindFromString(
        getText(varExpr->name).getUnownedSlice(),
        OperatorArity::Binary);

    // Classify the operator by kind (enum, not text). `Unknown` covers operators with no
    // builtin fast-path form, notably the short-circuiting `&&`/`||`.
    bool isArithmetic = kind == BuiltinOperationKind::Add || kind == BuiltinOperationKind::Sub ||
                        kind == BuiltinOperationKind::Mul || kind == BuiltinOperationKind::Div ||
                        kind == BuiltinOperationKind::Mod;
    bool isComparison = kind == BuiltinOperationKind::Eql || kind == BuiltinOperationKind::Neq ||
                        kind == BuiltinOperationKind::Less ||
                        kind == BuiltinOperationKind::Greater ||
                        kind == BuiltinOperationKind::Leq || kind == BuiltinOperationKind::Geq;
    bool isBitwise = kind == BuiltinOperationKind::BitAnd || kind == BuiltinOperationKind::BitOr ||
                     kind == BuiltinOperationKind::BitXor || kind == BuiltinOperationKind::Lsh ||
                     kind == BuiltinOperationKind::Rsh;
    if (!isArithmetic && !isComparison && !isBitwise)
        return nullptr;
    bool isEquality = kind == BuiltinOperationKind::Eql || kind == BuiltinOperationKind::Neq;
    bool isShift = kind == BuiltinOperationKind::Lsh || kind == BuiltinOperationKind::Rsh;

    auto leftArg = expr->arguments[0];
    auto rightArg = expr->arguments[1];
    if (!leftArg->type.type || !rightArg->type.type)
        return nullptr;

    // GLSL operator scope only overrides matrix operators (algebraic products) and vector
    // equality (`vec == vec` / `!=` -> scalar `bool`); bail to normal resolution for those so
    // the glsl module's overloads apply. Everything else is identical to HLSL and stays
    // fast-pathed.
    if (isGLSLOperatorScope())
    {
        bool leftMat = as<MatrixExpressionType>(leftArg->type.type) != nullptr;
        bool rightMat = as<MatrixExpressionType>(rightArg->type.type) != nullptr;
        bool anyVec = as<VectorExpressionType>(leftArg->type.type) != nullptr ||
                      as<VectorExpressionType>(rightArg->type.type) != nullptr;
        if (leftMat || rightMat)
            return nullptr;
        if (isEquality && anyVec)
            return nullptr;
    }

    // A bitwise/shift operator with a builtin floating-point operand has no integer interpretation.
    // This check precedes the mixed-shift early return and common-type coercion below, so it also
    // diagnoses mixed-type shifts (`float << int`, `int << float`) that skip common-type promotion
    // instead of letting them fall through to a confusing "ambiguous"/"no overload" error (issue
    // #11648). The both-builtin predicate leaves user `operator OP` and generics untouched.
    if (isBitwise)
    {
        if (Type* floatOperandType =
                _isBuiltinFloatingPointBitwiseOperands(leftArg->type.type, rightArg->type.type))
        {
            getSink()->diagnose(Diagnostics::BitwiseOperatorRequiresIntegerOperands{
                .name = varExpr->name,
                .type = floatOperandType,
                .expr = expr});
            return CreateErrorExpr(expr);
        }
    }

    // Shift operators do not promote to a common type: `a << b` keeps the type of `a` (the
    // shift amount `b` is converted independently). That asymmetry is not modeled by the
    // common-type rule, so leave mixed-type shifts to overload resolution.
    if (isShift && !leftArg->type.type->equals(rightArg->type.type))
        return nullptr;
    // Promote mixed-type operands to the common operand type (and carry the coerced operands
    // back onto the expression). Null => not a fast-pathable pair of builtin numeric operands.
    Type* operandType = coerceOperandsOfBuiltinBinaryExpr(leftArg, rightArg, leftArg, rightArg);
    if (!operandType)
        return nullptr;
    expr->arguments[0] = leftArg;
    expr->arguments[1] = rightArg;

    Type* elementType = operandType;
    VectorExpressionType* vecType = nullptr;
    MatrixExpressionType* matType = nullptr;
    if ((vecType = as<VectorExpressionType>(operandType)))
        elementType = vecType->getElementType();
    else if ((matType = as<MatrixExpressionType>(operandType)))
        elementType = matType->getElementType();

    // `vector<T,N> == vector<T,N>` / `!=` has a stdlib overload
    // (`glsl.meta.slang`'s `operator==<T:__BuiltinArithmeticType/__BuiltinLogicalType,N>`,
    // `[OverloadRank(15/14)]`) that reduces the per-component comparison to a single `bool`
    // via `all(equal(...))`/`any(notEqual(...))`. Its `[require(...)]` list spans every target
    // Slang emits to, so it is visible to overload resolution regardless of
    // `isGLSLOperatorScope()` -- unlike the matrix-operator and concrete-vector-equality cases
    // handled by the `isGLSLOperatorScope()` check above, which really are GLSL-scope-specific.
    // For a GENERIC element type there is no competing builtin form to prefer instead: the raw
    // per-component comparison this fast path would otherwise produce is only ever reachable by
    // constructing a `BuiltinOperatorExpr` directly, never through a declarable stdlib overload,
    // so before this function recognized generic element types, `vector<T,N> == vector<T,N>`
    // for an abstract `T` had nowhere else to resolve to and always went through that stdlib
    // overload. Decline here so it still does -- only a *concrete* element type keeps this
    // function's pre-existing (GLSL-scope-gated) `==`/`!=` behavior on vectors unchanged.
    if (isEquality && vecType && !as<BasicExpressionType>(elementType))
        return nullptr;

    auto family = classifyBuiltinArithmeticElementType(elementType);
    if (!family.isKnown())
        return nullptr;
    bool isIntegerBase = family.isInteger;
    bool isFloatBase = family.isFloat;
    bool isBoolBase = family.isBool;
    bool isLogicalBase = family.isLogical;
    // Some operators do not apply to every element type. For example, it is invalid to apply a
    // bitwise operator to a floating-point operand, and arithmetic does not apply to `bool`. When
    // the element type is not valid for the operator family we return null, so the expression
    // falls back to normal overload resolution (which will either find a user-provided overload
    // or produce the appropriate diagnostic) instead of being lowered as a builtin operation.
    // (Floating-point bitwise/shift operands are rejected earlier with a dedicated diagnostic, so
    // a non-integer bitwise operand reaching here is `bool`, which still resolves via `ILogical`.)
    //   - bitwise/shift (`& | ^ << >> ~`): integer only;
    //   - equality (`== !=`): integer, floating-point, bool, or (for a generic element type)
    //     anything conforming to `__BuiltinLogicalType` -- `isLogicalBase` is checked here, not
    //     just `isBoolBase`, because `kIROp_Eql`/`kIROp_Neq` are valid on that whole family
    //     uniformly, unlike unary logical-not (see `uBool` above, which deliberately does NOT
    //     accept `isLogical`);
    //   - arithmetic (`+ - * / %`) and ordering comparison (`< > <= >=`): integer or float.
    bool eligible;
    if (isBitwise)
        eligible = isIntegerBase;
    else if (isEquality)
        eligible = isIntegerBase || isFloatBase || isBoolBase || isLogicalBase;
    else
        eligible = isIntegerBase || isFloatBase;
    if (!eligible)
        return nullptr;

    // Result type: arithmetic/bitwise preserve the operand type; comparison yields a
    // boolean of matching shape (scalar -> bool, vector<T,N> -> vector<bool,N>,
    // matrix<T,R,C> -> matrix<bool,R,C>).
    QualType resultType;
    if (isComparison)
    {
        Type* boolType = m_astBuilder->getBoolType();
        if (vecType)
            resultType = QualType(createVectorType(boolType, vecType->getElementCount()));
        else if (matType)
            resultType = QualType(m_astBuilder->getMatrixType(
                boolType,
                matType->getRowCount(),
                matType->getColumnCount(),
                matType->getLayout()));
        else
            resultType = QualType(boolType);
    }
    else
    {
        resultType = QualType(operandType);
    }

    // Produce a dedicated `BuiltinOperatorExpr` carrying the `kind` resolved at the top of this
    // function. Every downstream consumer (IR lowering, constant folding via
    // `BuiltinOperationIntVal`, for-loop trip-count inference) reads the kind from the node
    // rather than re-parsing the operator name. The original `InvokeExpr`'s (already-checked,
    // possibly element-coerced) operands are carried over verbatim.
    auto node = m_astBuilder->create<BuiltinOperatorExpr>();
    node->op = kind;
    node->arguments.add(leftArg);
    node->arguments.add(rightArg);
    node->type = resultType;
    node->loc = expr->loc;
    node->abstractStorageAccess = _makeReadValueOperandSequence(m_astBuilder, node->arguments);
    // Only `Mod` reads this (to choose `FRem` over `IRem`), but it is resolved here regardless of
    // `kind`: `elementType` may be an abstract generic parameter by the time IR lowering runs,
    // which no longer carries a concrete `BaseType` to classify.
    node->elementTypeIsFloatingPoint = isFloatBase;

    // Register the operand/result types in a differentiable scope, regardless of the operator
    // family, matching the breadth of the pre-fast-path `visitInvokeExpr` (which walked all
    // operands for every operator). A comparison's boolean result and an integer bitwise
    // operand are not differentiable, but `maybeRegisterDifferentiableType` is a no-op for
    // non-differentiable types, so registering unconditionally just ensures a differentiable
    // operand type (e.g. comparing two `IDifferentiable` values to gate a branch) still has its
    // conformance registered, without special-casing the operator family.
    if (m_parentDifferentiableAttr)
    {
        maybeRegisterDifferentiableType(m_astBuilder, leftArg->type.type, leftArg->loc);
        maybeRegisterDifferentiableType(m_astBuilder, rightArg->type.type, rightArg->loc);
        maybeRegisterDifferentiableType(m_astBuilder, resultType.type, expr->loc);
    }
    return node;
}

// See the declaration in slang-check-impl.h: computes the common operand type that overload
// resolution would converge on for `left OP right` (the usual arithmetic conversions, with
// scalar/vector/matrix broadcast), or null when the operands are not both builtin numeric
// scalar/vector/matrix types or are not broadcast-compatible.
Type* SemanticsExprVisitor::getBuiltinArithmeticCommonType(Type* left, Type* right)
{
    BaseType leftBase, rightBase;
    IntVal *leftRows, *leftCols, *rightRows, *rightCols;
    if (!_getBuiltinCompositeTypeShape(left, leftBase, leftRows, leftCols))
        return nullptr;
    if (!_getBuiltinCompositeTypeShape(right, rightBase, rightRows, rightCols))
        return nullptr;

    // Only well-known numeric base types are handled here; anything else (Void and other
    // exotic kinds) falls back to overload resolution.
    auto isHandledBase = [](BaseType bt)
    {
        const auto& info = BaseTypeInfo::getInfo(bt);
        return bt == BaseType::Bool || (info.flags & (BaseTypeInfo::Flag::Integer |
                                                      BaseTypeInfo::Flag::FloatingPoint)) != 0;
    };
    if (!isHandledBase(leftBase) || !isHandledBase(rightBase))
        return nullptr;

    BaseType commonBase = unifyBaseType(leftBase, rightBase);
    Type* commonElementType = m_astBuilder->getBuiltinType(commonBase);

    bool leftIsScalar = (leftRows == nullptr);
    bool rightIsScalar = (rightRows == nullptr);
    bool leftIsMatrix = (leftCols != nullptr);
    bool rightIsMatrix = (rightCols != nullptr);

    // Scalars broadcast to the vector or matrix shape of the other operand.
    if (leftIsScalar && rightIsScalar)
    {
        return commonElementType;
    }
    if (leftIsMatrix || rightIsMatrix)
    {
        // matrix OP matrix (extents must match), or matrix OP scalar / scalar OP matrix.
        if (leftIsMatrix && rightIsMatrix)
        {
            if (!leftRows->equals(rightRows) || !leftCols->equals(rightCols))
                return nullptr;
        }
        else if (!leftIsScalar && !rightIsScalar)
        {
            // matrix mixed with a vector is not a builtin component-wise operation.
            return nullptr;
        }
        MatrixExpressionType* matSource = as<MatrixExpressionType>(leftIsMatrix ? left : right);
        return m_astBuilder->getMatrixType(
            commonElementType,
            matSource->getRowCount(),
            matSource->getColumnCount(),
            matSource->getLayout());
    }
    // At least one is a vector and neither is a matrix.
    if (!leftIsScalar && !rightIsScalar)
    {
        // vector OP vector: extents must match.
        if (!leftRows->equals(rightRows))
            return nullptr;
    }
    IntVal* elementCount = leftIsScalar ? rightRows : leftRows;
    return createVectorType(commonElementType, elementCount);
}

Expr* SemanticsExprVisitor::convertToLogicOperatorExpr(InvokeExpr* expr)
{
    LogicOperatorShortCircuitExpr* newExpr = nullptr;

    // If the logic expression is inside the generic parameter list, it cannot support short-circuit
    // which will generate the ifelse branch.
    if (!m_shouldShortCircuitLogicExpr)
    {
        return nullptr;
    }

    if (auto varExpr = as<VarExpr>(expr->functionExpr))
    {
        if ((getText(varExpr->name) == "&&") || (getText(varExpr->name) == "||"))
        {
            // We only use short-circuiting in scalar input, will fall back
            // to non-short-circuiting in vector input.
            bool shortCircuitSupport = true;
            for (auto& arg : expr->arguments)
            {
                if (!as<BasicExpressionType>(arg->type.type))
                {
                    shortCircuitSupport = false;
                }
            }

            if (!shortCircuitSupport)
            {
                return nullptr;
            }

            // We do the cast in the 2nd pass because we want to leave it for 'visitInvokeExpr'
            // to handle if this expression doesn't support short-circuiting.
            for (auto& arg : expr->arguments)
            {
                arg = coerceExpressionWithoutFinalization(
                    CoercionSite::Argument,
                    m_astBuilder->getBoolType(),
                    arg,
                    getSink());
            }

            expr->functionExpr = CheckTerm(expr->functionExpr);
            newExpr = m_astBuilder->create<LogicOperatorShortCircuitExpr>();
            if (varExpr->name->text == "&&")
            {
                newExpr->flavor = LogicOperatorShortCircuitExpr::Flavor::And;
            }
            else
            {
                newExpr->flavor = LogicOperatorShortCircuitExpr::Flavor::Or;
            }
            newExpr->loc = expr->loc;
            newExpr->functionExpr = expr->functionExpr;
            newExpr->type = m_astBuilder->getBoolType();
            newExpr->arguments = expr->arguments;
            newExpr->abstractStorageAccess =
                _makeReadValueOperandSequence(m_astBuilder, newExpr->arguments);
        }
    }

    return newExpr;
}

Expr* SemanticsExprVisitor::visitBuiltinOperatorExpr(BuiltinOperatorExpr* expr)
{
    // Already produced fully-checked (operator kind, operands, and result type resolved) by
    // `convertToBuiltinArithmeticOp`; there is nothing further to check.
    return expr;
}

Expr* SemanticsExprVisitor::visitInvokeExpr(InvokeExpr* expr)
{
    // Expression checking rewrites `functionExpr`, while language-server queries need the callee
    // as written. Capture that syntax before checking any part of the invocation.
    if (!expr->originalFunctionExpr)
        expr->originalFunctionExpr = expr->functionExpr;
    auto treatAsDifferentiableExpr = m_treatAsDifferentiableExpr;
    m_treatAsDifferentiableExpr = nullptr;
    // `visitInvokeExpr()` checks each written argument without finalizing its selected path before
    // `ResolveInvoke()` selects the callable. `finalizeSelectedExpression()` later commits only the
    // selected invocation path, so capture attachment and callable-receiver validation remain
    // deferred until selection.
    SemanticsVisitor operandVisitor(withPotentiallyUnevaluatedTerm());
    for (auto& arg : expr->arguments)
    {
        arg = operandVisitor.checkAndResolveExpressionWithoutFinalization(arg);
    }

    // if the expression is '&&' or '||', we will convert it
    // to use short-circuit evaluation.
    if (auto convertedExpr = convertToLogicOperatorExpr(expr))
        return convertedExpr;

    // Fast path: a builtin arithmetic/comparison/bitwise/shift/unary operator on
    // scalar/vector/matrix operands (`a + b`, `a < b`, `v * s`, `-x`, etc.; same or mixed
    // builtin type) is rewritten to a `BuiltinOperatorExpr` and skips generic operator
    // overload resolution.
    if (auto convertedExpr = convertToBuiltinArithmeticOp(expr))
        return convertedExpr;

    // Before Slang 2026, a comma outside a for-loop side-effect expression is likely an accidental
    // comma operator. In Slang 2026 and later, the same syntax can construct a tuple.
    if (!isSlang2026OrLater(this))
    {
        bool exprIsInfixExpr = false;
        InfixExpr* infixExpr = nullptr;
        if (auto candidateInfixExpr = as<InfixExpr>(expr))
        {
            exprIsInfixExpr = true;
            infixExpr = candidateInfixExpr;
        }

        bool functionExprIsVarExpr = false;
        VarExpr* varExpr = nullptr;
        if (exprIsInfixExpr)
        {
            if (auto candidateVarExpr = as<VarExpr>(infixExpr->functionExpr))
            {
                functionExprIsVarExpr = true;
                varExpr = candidateVarExpr;
            }
        }

        if (functionExprIsVarExpr && varExpr->name && varExpr->name->text == ",")
        {
            if (!getInForLoopSideEffect() && !m_parentExpandExpr)
            {
                getSink()->diagnose(Diagnostics::CommaOperatorUsedInExpression{.expr = infixExpr});
            }
        }
    }

    expr->functionExpr = checkPotentiallyUnevaluatedTerm(expr->functionExpr);

    if (auto baseType = as<DeclRefType>(expr->functionExpr->type))
    {
        // A callee with a `DeclRefType` is a functor. `visitInvokeExpr()` looks up its `operator()`
        // member and replaces `functionExpr` with that selected callable.
        auto operatorName = getName("()");

        bool needDeref = false;
        expr->functionExpr = maybeInsertImplicitOpForMemberBase(
            expr->functionExpr,
            CheckBaseContext::Member,
            needDeref);

        LookupResult lookupResult = lookUpMember(
            m_astBuilder,
            this,
            operatorName,
            expr->functionExpr->type,
            m_outerScope,
            LookupMask::Default,
            LookupOptions::NoDeref);
        bool diagnosed = false;
        lookupResult =
            filterLookupResultByVisibilityAndDiagnose(lookupResult, expr->loc, diagnosed);
        if (!lookupResult.isValid())
        {
            if (!diagnosed)
                getSink()->diagnose(
                    Diagnostics::CallOperatorNotFound{.type = baseType, .expr = expr});
            return CreateErrorExpr(expr);
        }
        auto callFuncExpr = createLookupResultExpr(
            operatorName,
            lookupResult,
            expr->functionExpr,
            expr->loc,
            expr->functionExpr);
        expr->functionExpr = callFuncExpr;
    }

    m_treatAsDifferentiableExpr = treatAsDifferentiableExpr;

    // If we are in a differentiable function, register differential witness tables involved in
    // this call.
    if (m_parentFunc && m_parentFunc->hasModifier<DifferentiableAttribute>())
    {
        for (auto& arg : expr->arguments)
        {
            maybeRegisterDifferentiableType(m_astBuilder, arg->type.type, arg->loc);
        }
    }

    auto checkedExpr = CheckInvokeExprWithCheckedOperands(expr);

    // Perform additional validation for known built-in functions.
    maybeCheckKnownBuiltinInvocation(checkedExpr);

    if (m_parentDifferentiableAttr)
    {
        if (auto checkedInvokeExpr = as<InvokeExpr>(checkedExpr))
        {
            // Register types for final resolved invoke arguments again.
            for (auto& arg : expr->arguments)
            {
                maybeRegisterDifferentiableType(m_astBuilder, arg->type.type, arg->loc);
            }

            registerAssociatedMethodsForSelectedCallable(this, checkedInvokeExpr->functionExpr);
        }
        maybeRegisterDifferentiableType(m_astBuilder, checkedExpr->type.type, checkedExpr->loc);
    }
    return checkedExpr;
}

// Finds the in-scope identifier whose spelling is closest to `name`, to power a
// "did you mean ...?" suggestion when `name` failed to resolve. Walks the scope
// chain (and each scope's sibling chain) collecting the names of direct members,
// computes the case-insensitive Levenshtein distance to each, and returns the
// single closest candidate within a small distance threshold. Returns nullptr if
// nothing is close enough (so the caller can simply omit the suggestion), or if
// two candidates tie at the closest distance (so the output never depends on
// import/scope-walk order).
//
// Only direct members of the lexical scope chain are considered; inherited and
// extension members reached via the `this`-parameter breadcrumb in real lookup
// are deliberately not searched, to keep this off the hot path of successful
// lookups. `semantics` is used to skip candidates the user could not access.
static Name* findClosestInScopeName(
    SemanticsVisitor* semantics,
    Name* name,
    Scope* scope,
    Decl* declToExclude)
{
    if (!name)
        return nullptr;

    const UnownedStringSlice target = getUnownedStringSliceText(name);

    // Scale the allowed edit distance with the identifier length and cap it, so
    // that we only offer genuinely-close names (e.g. `lenght` -> `length`, or
    // `f_a` -> `f_b`) and never a wildly different one (e.g. the keyword `case`
    // -> the module `core`, a distance-2 edit on a 4-char name). The heuristic is
    // roughly one edit per three characters with a floor of one. Names shorter
    // than 3 chars are too short to suggest against; and we also refuse
    // pathologically long names, since suggestions are advisory and not worth an
    // O(N*M) Levenshtein DP per candidate on an adversarial multi-kilobyte
    // identifier.
    if (target.getLength() < 3 || target.getLength() > 256)
        return nullptr;
    const Index maxDistance = Math::Min<Index>(3, Math::Max<Index>(1, target.getLength() / 3));

    Index bestDistance = maxDistance + 1;
    // The *distinct* candidate names sharing the current best distance. Names are
    // deduped (the "nub" of the candidate list) so that, e.g., two overloads
    // both called `length` count once: they would print the identical
    // suggestion, so they are not a genuine ambiguity. A suggestion is offered
    // only when this set ends up with exactly one name.
    HashSet<Name*> bestNames;

    for (Scope* s = scope; s; s = s->parent)
    {
        for (Scope* sib = s; sib; sib = sib->nextSibling)
        {
            auto containerDecl = sib->containerDecl;
            if (!containerDecl)
                continue;

            // Don't suggest names from the core module. Its global scope holds
            // thousands of builtins, so almost any identifier finds a spurious
            // close match there (e.g. `instance` -> `distance`); restricting to
            // user-written declarations keeps suggestions quiet and relevant.
            // Skipping the whole container here (rather than per-member) also
            // avoids iterating — and, for any on-demand-deserialized core
            // module, materializing — its members via `getDirectMemberDecls()`.
            if (isFromCoreModule(containerDecl))
                continue;

            for (auto candidateDecl : containerDecl->getDirectMemberDecls())
            {
                Name* candidateName = candidateDecl->getName();
                if (!candidateName || candidateName == name)
                    continue;

                // Skip the declaration currently being checked, mirroring the
                // `getDeclToExcludeFromLookup()` exclusion that real lookup
                // applies: suggesting it would name something the same lookup
                // path still cannot resolve.
                if (candidateDecl == declToExclude)
                    continue;

                const UnownedStringSlice candidateText = getUnownedStringSliceText(candidateName);
                if (candidateText.getLength() == 0)
                    continue;

                // Cheap length pre-filter: |lenA - lenB| is a lower bound on the
                // edit distance, so skip candidates that cannot possibly be
                // within `maxDistance` before paying for the O(lenA*lenB) DP (and
                // before forcing any lazy member materialization downstream).
                if (Math::Abs(candidateText.getLength() - target.getLength()) > maxDistance)
                    continue;

                // Don't suggest a declaration the user could not have referenced:
                // real lookup already filtered inaccessible (`private`/`internal`)
                // candidates, so offering one as a "did you mean" would name a
                // forbidden symbol (and leak imported module contents via typos).
                if (!semantics->isDeclVisibleFromScope(makeDeclRef(candidateDecl), scope))
                    continue;

                const Index distance =
                    StringUtil::calcLevenshteinDistanceCaseInsensitive(target, candidateText);
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestNames.clear();
                    bestNames.add(candidateName);
                }
                else if (distance == bestDistance)
                {
                    bestNames.add(candidateName);
                }
            }
        }
    }

    // Offer a suggestion only when there is a single, unambiguous closest name.
    // Multiple distinct names at the best distance would make the chosen one
    // depend on import/scope-walk order, so suppress the suggestion instead.
    if (bestDistance > maxDistance)
        return nullptr;
    if (bestNames.getCount() != 1)
        return nullptr;
    Name* best = nullptr;
    for (auto n : bestNames)
        best = n;
    return best;
}

Expr* SemanticsExprVisitor::visitVarExpr(VarExpr* expr)
{
    // If we've already resolved this expression, don't try again.
    if (expr->declRef)
    {
        if (!expr->type)
            expr->type = GetTypeForDeclRef(expr->declRef, expr->loc);
        return _getOrCreateLambdaCaptureReference(
            m_astBuilder,
            m_parentLambdaExpr,
            m_parentLambdaDecl,
            m_lambdaCaptureContext,
            expr,
            as<VarDeclBase>(expr->declRef.getDecl()));
    }
    expr->type = QualType(m_astBuilder->getErrorType());
    auto lookupResult = lookUp(
        m_astBuilder,
        this,
        expr->name,
        expr->scope,
        LookupMask::Default,
        false,
        getDeclToExcludeFromLookup(),
        getExcludeTransparentMembersFromLookup());

    bool diagnosed = false;
    lookupResult = filterLookupResultByVisibilityAndDiagnose(lookupResult, expr->loc, diagnosed);

    if (expr->name == getSession()->getCompletionRequestTokenName())
    {
        auto scopeKind = CompletionSuggestions::ScopeKind::Expr;
        if (!m_parentFunc)
            scopeKind = CompletionSuggestions::ScopeKind::Decl;
        suggestCompletionItems(scopeKind, lookupResult);
        return expr;
    }

    Expr* resultExpr = expr;

    if (lookupResult.isValid())
    {
        return createLookupResultExpr(expr->name, lookupResult, nullptr, expr->loc, expr);
    }

    if (!diagnosed)
    {
        // If a similarly-spelled identifier is in scope, attach a "did you mean
        // 'length'?" note to the error; this turns a bare "undefined identifier
        // 'lenght'" into an actionable hint. The note only renders when
        // `suggestionLocation` is valid, so a null suggestion is harmless.
        auto suggestion =
            findClosestInScopeName(this, expr->name, expr->scope, getDeclToExcludeFromLookup());
        getSink()->diagnose(Diagnostics::UndefinedIdentifier{
            .name = expr->name,
            .suggestion = suggestion,
            .location = expr->loc,
            .suggestionLocation = suggestion ? expr->loc : SourceLoc{}});
    }

    return resultExpr;
}

/// Returns whether a size-like expression evaluates its operand at runtime.
///
/// Only `countof` on a value pack consumes the value. Ordinary `sizeof`, `alignof`, and array
/// `countof` expressions use only the checked type.
static bool _doesSizeOfLikeExprEvaluateValue(SizeOfLikeExpr* expr)
{
    if (!as<CountOfExpr>(expr))
        return false;
    return as<ValuePackType>(expr->sizedType) != nullptr;
}

/// Returns whether an `is` expression evaluates its operand at runtime.
///
/// Without a witness, lowering compares the checked operand and target types without evaluating
/// the operand. An optional declared witness likewise makes the result depend only on the witness;
/// every other witness performs a runtime check of the value.
static bool _doesIsTypeExprEvaluateValue(IsTypeExpr* expr)
{
    if (!expr->witnessArg)
        return false;

    auto declaredWitness = as<DeclaredSubtypeWitness>(expr->witnessArg);
    if (!declaredWitness)
        return true;
    return !declaredWitness->isOptional();
}

static DeclRef<CallableDecl> _getHigherOrderBaseFunctionDeclRef(
    SemanticsVisitor* semantics,
    Expr* funcExpr);

/// Implements `finalizeSelectedExpression()` for one selected expression path.
///
/// Consider a lambda that calls one overload of `value.method()`.
/// `ConstructLookupResultExpr()` and `AddOverloadCandidates()` can provisionally build paths for
/// several overload candidates, including capture references and bound receivers. After overload
/// resolution selects one candidate, this visitor walks only that expression path and leaves
/// discarded candidates, generic arguments, and unevaluated static qualifiers untouched. During
/// that walk, `attachCaptureField()` commits provisional lambda captures, `validateCallable()`
/// validates the effective receiver of the selected callable, and `finalizeConsumedValue()`
/// validates the abstract-storage uses implied by evaluating the selected path.
struct SelectedExpressionFinalizer : ExprVisitor<SelectedExpressionFinalizer, Expr*>
{
    SemanticsVisitor* semantics;
    LambdaDecl* lambdaDecl;
    LambdaCaptureContext* captureContext;
    ASTBuilder* astBuilder;
    DiagnosticSink* sink;

    SelectedExpressionFinalizer(SemanticsVisitor* inSemantics)
        : semantics(inSemantics)
        , lambdaDecl(inSemantics->getParentLambdaDecl())
        , captureContext(inSemantics->getLambdaCaptureContext())
        , astBuilder(inSemantics->getASTBuilder())
        , sink(inSemantics->getSink())
    {
    }

    /// Adds the provisional field used by one selected capture reference to the closure.
    void attachCaptureField(VarDeclBase* captureField, Expr* captureUseExpr)
    {
        if (!captureContext)
            return;
        if (!captureField)
            return;

        if (!captureContext->fieldToCapturedDecl.containsKey(captureField))
            return;

        SLANG_RELEASE_ASSERT(lambdaDecl);
        if (captureField->parentDecl)
        {
            SLANG_RELEASE_ASSERT(captureField->parentDecl == lambdaDecl);
            return;
        }

        lambdaDecl->addMember(captureField);

        attachReferencedDeclarations(captureField->type.type, captureUseExpr);

        if (!sink)
            return;

        if (isNonCopyableType(captureField->type.type))
        {
            sink->diagnose(Diagnostics::NonCopyableTypeCapturedInLambda{
                .type = captureField->type.type,
                .expr = captureUseExpr});
        }
    }

    /// Adds the closure field associated with `declaration`, if one exists.
    ///
    /// References in a selected expression can name either the outer declaration or its
    /// provisional closure field. Both forms must attach the same field.
    void attachDeclarationDependency(Decl* declaration, Expr* captureUseExpr)
    {
        if (!captureContext)
            return;
        if (!declaration)
            return;

        if (auto captureField = as<VarDeclBase>(declaration))
        {
            if (captureContext->fieldToCapturedDecl.containsKey(captureField))
            {
                attachCaptureField(captureField, captureUseExpr);
                return;
            }
        }

        VarDeclBase* captureField = nullptr;
        if (captureContext->capturedDeclToField.tryGetValue(declaration, captureField))
            attachCaptureField(captureField, captureUseExpr);
    }

    /// Adds capture fields for the capture-eligible declarations referenced by `node`.
    void attachReferencedDeclarations(NodeBase* node, Expr* captureUseExpr)
    {
        if (!captureContext)
            return;
        if (!node)
            return;

        HashSet<Decl*> referencedDecls;
        collectReferencedDecls(semantics, node, referencedDecls);
        for (auto declaration : referencedDecls)
            attachDeclarationDependency(declaration, captureUseExpr);
    }

    /// Returns whether `expr` has no selected runtime subtree to finalize.
    ///
    /// Type-valued expressions are compile-time syntax. Overload or generic resolution must select
    /// the remaining cases before a later `SelectedExpressionFinalizer` invocation can finalize
    /// them.
    bool isUnselectedOrTypeExpression(Expr* expr)
    {
        if (as<TypeType>(expr->type.type))
            return true;
        if (as<OverloadedExpr>(expr))
            return true;
        if (as<OverloadedExpr2>(expr))
            return true;
        if (as<PartiallyAppliedGenericExpr>(expr))
            return true;
        return false;
    }

    /// Finalizes `expr` according to the way its selected path is consumed.
    Expr* finalize(Expr* expr, SelectedExpressionUse use, SourceLoc callableLocation)
    {
        if (!expr)
            return expr;
        if (semantics->IsErrorExpr(expr))
            return expr;
        if (isUnselectedOrTypeExpression(expr))
            return expr;

        // Lowering consumes the checked result type of every selected runtime expression. If that
        // type was opened from an existential value, finalization must capture the value that owns
        // its dynamic type and witness table in the closure. This `finalize` call attaches those
        // dependencies once, so the individual `visit*` methods do not repeat that work.
        attachReferencedDeclarations(expr->type.type, expr);

        // `SelectedExpressionFinalizer::finalize()` follows the selected expression through these
        // wrappers. It finalizes each child as an ordinary value, then validates the complete
        // wrapper exactly once for a callable use, including any retained local-binding prefix.
        if (auto parenExpr = as<ParenExpr>(expr))
        {
            parenExpr->base = finalizeValue(parenExpr->base);
            parenExpr->abstractStorageAccess = parenExpr->base->abstractStorageAccess;
            return use != SelectedExpressionUse::Value
                       ? validateCallable(parenExpr, use, callableLocation)
                       : parenExpr;
        }
        if (auto genericAppExpr = as<GenericAppExpr>(expr))
        {
            genericAppExpr->functionExpr = finalizeValue(genericAppExpr->functionExpr);
            genericAppExpr->abstractStorageAccess =
                genericAppExpr->functionExpr->abstractStorageAccess;
            return use != SelectedExpressionUse::Value
                       ? validateCallable(genericAppExpr, use, callableLocation)
                       : genericAppExpr;
        }
        if (auto letExpr = as<LetExpr>(expr))
        {
            SLANG_RELEASE_ASSERT(letExpr->decl);
            SLANG_RELEASE_ASSERT(letExpr->decl->initExpr);
            SLANG_RELEASE_ASSERT(letExpr->body);
            letExpr->decl->initExpr = finalize(
                letExpr->decl->initExpr,
                SelectedExpressionUse::Value,
                SourceLoc());
            letExpr->body = finalizeValue(letExpr->body);
            _setLocalBindingStorageAccess(letExpr);
            return use != SelectedExpressionUse::Value
                       ? validateCallable(letExpr, use, callableLocation)
                       : letExpr;
        }

        expr = dispatch(expr);
        if (use == SelectedExpressionUse::Value)
            return expr;
        if (semantics->IsErrorExpr(expr))
            return expr;

        return validateCallable(expr, use, callableLocation);
    }

    Expr* finalizeValue(Expr* expr)
    {
        return finalize(expr, SelectedExpressionUse::Value, SourceLoc());
    }

    /// Finalizes an evaluated operand according to its checked result type.
    ///
    /// For a result-forming wrapper, finalization leaves the operand's abstract-storage use to the
    /// wrapper's recorded access path. It still finalizes a function-typed operand as a callable so
    /// that finalization validates the receiver of a bound method hidden below the wrapper.
    Expr* finalizeOperandSelection(Expr* expr)
    {
        if (!expr)
            return expr;
        return finalize(
            expr,
            semantics->getSelectedExpressionUseForType(expr->type.type),
            expr->loc);
    }

    void finalizeValues(List<Expr*>& expressions)
    {
        for (auto& expression : expressions)
            expression = finalizeValue(expression);
    }

    /// Finalizes an expression selected as an ordinary consumed value.
    ///
    /// A selected function-typed expression can carry a callable receiver that must be validated
    /// before the enclosing aggregate consumes the resulting function value.
    bool finalizeConsumedValue(Expr*& expr)
    {
        expr = finalizeOperandSelection(expr);
        expr = semantics->checkAbstractStorageUse(expr, AbstractStorageUse::ReadValue);
        return !semantics->IsErrorExpr(expr);
    }

    /// Finalizes and consumes an expression used as an immediate call operand.
    ///
    /// Finalization validates the receiver of the already selected callable. The subsequent
    /// `checkAbstractStorageUse` call separately reads the expression that produced that callable,
    /// such as a function-valued property or a retained local binding.
    bool finalizeCallableOperand(Expr*& expr, SourceLoc location)
    {
        expr = finalize(expr, SelectedExpressionUse::CallableInvocation, location);
        expr = semantics->checkAbstractStorageUse(expr, AbstractStorageUse::ReadValue);
        return !semantics->IsErrorExpr(expr);
    }

    Expr* visitExpr(Expr*)
    {
        SLANG_UNEXPECTED("selected-expression finalization lacks an expression classification");
        UNREACHABLE_RETURN(nullptr);
    }

    Expr* visitIncompleteExpr(IncompleteExpr* expr) { return expr; }
    Expr* visitLiteralExpr(LiteralExpr* expr) { return expr; }
    Expr* visitDefaultConstructExpr(DefaultConstructExpr* expr) { return expr; }
    Expr* visitMakeArrayFromElementExpr(MakeArrayFromElementExpr* expr) { return expr; }
    Expr* visitThisExpr(ThisExpr* expr) { return expr; }
    Expr* visitReturnValExpr(ReturnValExpr* expr) { return expr; }
    Expr* visitLambdaExpr(LambdaExpr* expr) { return expr; }
    Expr* visitFuncAsTypeExpr(FuncAsTypeExpr* expr) { return expr; }
    Expr* visitFuncTypeOfExpr(FuncTypeOfExpr* expr) { return expr; }
    Expr* visitThisTypeExpr(ThisTypeExpr* expr) { return expr; }
    Expr* visitHLSLUnsignedTypeExpr(HLSLUnsignedTypeExpr* expr) { return expr; }

    /// Validates the receiver on the direct member selected as a callable.
    Expr* validateCallable(
        Expr* callableExpr,
        SelectedExpressionUse use,
        SourceLoc location)
    {
        SLANG_RELEASE_ASSERT(use != SelectedExpressionUse::Value);
        auto directCallableExpr = semantics->getDirectCallableDeclRefExpr(callableExpr);
        auto memberExpr = as<MemberExpr>(directCallableExpr);
        if (memberExpr)
        {
            if (!memberExpr->declRef.as<SubscriptDecl>())
            {
                auto callableDeclRef = memberExpr->declRef.as<CallableDecl>();
                SLANG_RELEASE_ASSERT(callableDeclRef);

                if (auto thisParamInfo = semantics->findEffectiveThisParamInfo(callableDeclRef))
                {
                    auto diagnosticLocation = location.isValid() ? location : callableExpr->loc;
                    if (!semantics->validateCallableReceiver(
                            memberExpr->baseExpression,
                            callableDeclRef,
                            diagnosticLocation))
                    {
                        return semantics->CreateErrorExpr(callableExpr);
                    }

                    auto receiverUse =
                        use == SelectedExpressionUse::CallableInvocation
                            ? getAbstractStorageUseForParameter(thisParamInfo->mode)
                            : AbstractStorageUse::DiscardResult;
                    if (!_checkAbstractStorageAccess(
                            semantics,
                            memberExpr->baseExpression->abstractStorageAccess,
                            receiverUse))
                        return semantics->CreateErrorExpr(callableExpr);
                }
            }
        }

        return callableExpr;
    }

    Expr* visitExprWithArgsBase(ExprWithArgsBase* expr)
    {
        for (auto& argument : expr->arguments)
            argument = finalizeOperandSelection(argument);
        return expr;
    }

    Expr* visitIndexExpr(IndexExpr* expr)
    {
        expr->baseExpression = finalizeValue(expr->baseExpression);
        finalizeValues(expr->indexExprs);
        return expr;
    }

    Expr* visitDerefExpr(DerefExpr* expr)
    {
        expr->base = finalizeValue(expr->base);
        return expr;
    }

    Expr* visitSwizzleExpr(SwizzleExpr* expr)
    {
        expr->base = finalizeValue(expr->base);
        return expr;
    }

    Expr* visitMatrixSwizzleExpr(MatrixSwizzleExpr* expr)
    {
        expr->base = finalizeValue(expr->base);
        return expr;
    }

    Expr* visitOpenRefExpr(OpenRefExpr* expr)
    {
        expr->innerExpr = finalizeValue(expr->innerExpr);
        return expr;
    }

    Expr* visitTryExpr(TryExpr* expr)
    {
        expr->base = finalizeValue(expr->base);
        return expr;
    }

    Expr* visitTreatAsDifferentiableExpr(TreatAsDifferentiableExpr* expr)
    {
        expr->innerExpr = finalizeOperandSelection(expr->innerExpr);
        return expr;
    }

    Expr* visitAssignExpr(AssignExpr* expr)
    {
        // Assignment checking finalizes the selected destination before accessor validation and
        // finalizes the assignment right-hand side after coercion. Revisiting either child here
        // would repeat assignment checking and selected-expression finalization.
        return expr;
    }

    Expr* visitModifierCastExpr(ModifierCastExpr* expr)
    {
        expr->valueArg = finalizeOperandSelection(expr->valueArg);
        return expr;
    }

    Expr* visitTupleExpr(TupleExpr* expr)
    {
        for (auto& element : expr->elements)
        {
            if (!finalizeConsumedValue(element))
                return semantics->CreateErrorExpr(expr);
        }
        return expr;
    }

    Expr* visitInitializerListExpr(InitializerListExpr* expr)
    {
        for (auto& argument : expr->args)
        {
            if (!finalizeConsumedValue(argument))
                return semantics->CreateErrorExpr(expr);
        }
        return expr;
    }

    Expr* visitCastOptionalExpr(CastOptionalExpr* expr)
    {
        expr->valueArg = finalizeOperandSelection(expr->valueArg);
        expr->innerCoercedExpr = finalizeOperandSelection(expr->innerCoercedExpr);
        return expr;
    }

    Expr* visitPackExpr(PackExpr* expr)
    {
        for (auto& argument : expr->args)
        {
            if (!finalizeConsumedValue(argument))
                return semantics->CreateErrorExpr(expr);
        }
        return expr;
    }

    Expr* visitExpandExpr(ExpandExpr* expr)
    {
        expr->baseExpr = finalizeValue(expr->baseExpr);
        return expr;
    }

    Expr* visitEachExpr(EachExpr* expr)
    {
        expr->baseExpr = finalizeValue(expr->baseExpr);
        return expr;
    }

    Expr* visitAddressOfExpr(AddressOfExpr* expr)
    {
        expr->arg = finalizeValue(expr->arg);
        return expr;
    }

    Expr* visitBuiltinCastExpr(BuiltinCastExpr* expr)
    {
        expr->base = finalizeOperandSelection(expr->base);
        return expr;
    }

    Expr* visitFloatBitCastExpr(FloatBitCastExpr* expr)
    {
        expr->value = finalizeValue(expr->value);
        return expr;
    }

    Expr* visitPackQueryExpr(PackQueryExpr* expr)
    {
        expr->value = finalizeValue(expr->value);
        return expr;
    }

    Expr* visitShapePackTransformExpr(ShapePackTransformExpr* expr)
    {
        finalizeValues(expr->args);
        return expr;
    }

    Expr* visitDetachExpr(DetachExpr* expr)
    {
        expr->inner = finalizeOperandSelection(expr->inner);
        return expr;
    }

    Expr* visitVarExpr(VarExpr* expr)
    {
        attachReferencedDeclarations(expr->declRef.declRefBase, expr);
        return expr;
    }

    Expr* visitMemberExpr(MemberExpr* expr)
    {
        expr->baseExpression =
            finalize(expr->baseExpression, SelectedExpressionUse::Value, SourceLoc());
        attachCaptureField(as<VarDeclBase>(expr->declRef.getDecl()), expr);
        attachReferencedDeclarations(expr->declRef.declRefBase, expr);
        return expr;
    }

    Expr* visitStaticMemberExpr(StaticMemberExpr* expr)
    {
        attachReferencedDeclarations(expr->declRef.declRefBase, expr);
        return expr;
    }

    Expr* visitInvokeExpr(InvokeExpr* expr)
    {
        const bool callableIsValid = finalizeCallableOperand(expr->functionExpr, expr->loc);
        for (auto& argument : expr->arguments)
            argument = finalizeOperandSelection(argument);

        if (!callableIsValid)
            return semantics->CreateErrorExpr(expr);
        return expr;
    }

    /// Finalizes the selected callable and returns whether its higher-order operator remains valid.
    bool finalizeHigherOrderBaseFunction(HigherOrderInvokeExpr* expr)
    {
        return finalizeCallableOperand(expr->baseFunction, expr->loc);
    }

    Expr* visitHigherOrderInvokeExpr(HigherOrderInvokeExpr* expr)
    {
        if (!finalizeHigherOrderBaseFunction(expr))
            return semantics->CreateErrorExpr(expr);

        // A contextual conversion can select a higher-order expression without an immediate call.
        // This visitor enforces the same differentiation-operand contract here as on the
        // invocation path above.
        if (auto differentiateExpr = as<DifferentiateExpr>(expr))
        {
            if (_diagnoseBoundInstanceMethodDifferentiationOperand(
                    semantics,
                    differentiateExpr))
            {
                return semantics->CreateErrorExpr(expr);
            }
        }
        return expr;
    }

    Expr* visitDispatchKernelExpr(DispatchKernelExpr* expr)
    {
        if (!finalizeHigherOrderBaseFunction(expr))
            return semantics->CreateErrorExpr(expr);

        // `IRDispatchKernel` identifies a kernel function but has no representation for an
        // implicit receiver. Reject both `value.kernel` and `Type::kernel` after overload
        // selection, using the selected declaration's effective receiver rather than lookup-path
        // metadata. A static method selected through a value has no receiver and remains valid.
        auto callableDeclRef = _getHigherOrderBaseFunctionDeclRef(semantics, expr->baseFunction);
        if (callableDeclRef && semantics->findEffectiveThisParamInfo(callableDeclRef))
        {
            sink->diagnose(
                Diagnostics::NonStaticMemberFunctionNotAllowedAsDispatchKernelOperand{
                    .func = callableDeclRef.getDecl(),
                    .expr = expr->baseFunction});
            return semantics->CreateErrorExpr(expr);
        }

        expr->dispatchSize = semantics->checkAbstractStorageUse(
            finalizeValue(expr->dispatchSize),
            AbstractStorageUse::ReadValue);
        expr->threadGroupSize = semantics->checkAbstractStorageUse(
            finalizeValue(expr->threadGroupSize),
            AbstractStorageUse::ReadValue);
        if (semantics->IsErrorExpr(expr->dispatchSize))
            return semantics->CreateErrorExpr(expr);
        if (semantics->IsErrorExpr(expr->threadGroupSize))
            return semantics->CreateErrorExpr(expr);
        return expr;
    }

    Expr* visitAggTypeCtorExpr(AggTypeCtorExpr* expr)
    {
        for (auto& argument : expr->arguments)
            argument = finalizeOperandSelection(argument);
        return expr;
    }

    Expr* visitGetArrayLengthExpr(GetArrayLengthExpr* expr)
    {
        auto arrayType = as<ArrayExpressionType>(expr->arrayExpr->type.type);
        SLANG_RELEASE_ASSERT(arrayType);
        attachReferencedDeclarations(arrayType->getElementCount(), expr);
        return expr;
    }

    Expr* visitSizeOfLikeExpr(SizeOfLikeExpr* expr)
    {
        if (as<CountOfExpr>(expr))
        {
            if (auto arrayType = as<ArrayExpressionType>(expr->sizedType))
            {
                attachReferencedDeclarations(arrayType->getElementCount(), expr);
                return expr;
            }
        }

        attachReferencedDeclarations(expr->sizedType, expr);
        attachReferencedDeclarations(expr->dataLayoutType, expr);
        if (_doesSizeOfLikeExprEvaluateValue(expr))
            expr->value = finalizeOperandSelection(expr->value);
        return expr;
    }

    Expr* visitIsTypeExpr(IsTypeExpr* expr)
    {
        if (expr->constantVal)
            return expr;

        attachReferencedDeclarations(expr->typeExpr.type, expr);
        if (expr->witnessArg)
            attachReferencedDeclarations(expr->witnessArg, expr);
        else
            attachReferencedDeclarations(expr->value->type.type, expr);

        if (_doesIsTypeExprEvaluateValue(expr))
            expr->value = finalizeOperandSelection(expr->value);
        return expr;
    }

    Expr* visitAsTypeExpr(AsTypeExpr* expr)
    {
        attachReferencedDeclarations(expr->witnessArg, expr);
        expr->value = finalizeOperandSelection(expr->value);
        return expr;
    }

    Expr* visitMakeOptionalExpr(MakeOptionalExpr* expr)
    {
        expr->value = finalizeOperandSelection(expr->value);
        return expr;
    }

    Expr* visitCastToSuperTypeExpr(CastToSuperTypeExpr* expr)
    {
        attachReferencedDeclarations(expr->witnessArg, expr);
        expr->valueArg = finalizeOperandSelection(expr->valueArg);
        return expr;
    }

    Expr* visitExtractExistentialValueExpr(ExtractExistentialValueExpr* expr)
    {
        attachCaptureField(expr->declRef.getDecl(), expr->originalExpr);
        attachReferencedDeclarations(expr->declRef.declRefBase, expr->originalExpr);
        return expr;
    }

    Expr* visitMakeRefExpr(MakeRefExpr* expr)
    {
        expr->base = finalize(expr->base, SelectedExpressionUse::Value, SourceLoc());
        return expr;
    }

    bool finalizeSPIRVAsmOperand(SPIRVAsmExpr* expr, SPIRVAsmOperand& operand)
    {
        switch (operand.flavor)
        {
        case SPIRVAsmOperand::BuiltinVar:
        case SPIRVAsmOperand::SlangType:
        case SPIRVAsmOperand::SampledType:
            attachReferencedDeclarations(operand.type.type, expr);
            return true;

        case SPIRVAsmOperand::SlangValueAddr:
            operand.expr = semantics->checkAbstractStorageUse(
                finalizeValue(operand.expr),
                AbstractStorageUse::RequireAddress);
            return !semantics->IsErrorExpr(operand.expr);

        case SPIRVAsmOperand::SlangValue:
        case SPIRVAsmOperand::SlangImmediateValue:
        case SPIRVAsmOperand::ImageType:
        case SPIRVAsmOperand::SampledImageType:
        case SPIRVAsmOperand::ConvertTexel:
        case SPIRVAsmOperand::RayPayloadFromLocation:
        case SPIRVAsmOperand::RayAttributeFromLocation:
        case SPIRVAsmOperand::RayCallableFromLocation:
            operand.expr = semantics->checkAbstractStorageUse(
                finalizeValue(operand.expr),
                AbstractStorageUse::ReadValue);
            return !semantics->IsErrorExpr(operand.expr);

        default:
            SLANG_RELEASE_ASSERT(!operand.expr);
            return true;
        }
    }

    Expr* visitSPIRVAsmExpr(SPIRVAsmExpr* expr)
    {
        for (auto& inst : expr->insts)
        {
            if (!finalizeSPIRVAsmOperand(expr, inst.opcode))
                return semantics->CreateErrorExpr(expr);
            for (auto& operand : inst.operands)
            {
                if (!finalizeSPIRVAsmOperand(expr, operand))
                    return semantics->CreateErrorExpr(expr);
            }
        }
        return expr;
    }

    Expr* visitSharedTypeExpr(SharedTypeExpr* expr) { return expr; }
    Expr* visitAndTypeExpr(AndTypeExpr* expr) { return expr; }
    Expr* visitModifiedTypeExpr(ModifiedTypeExpr* expr) { return expr; }
    Expr* visitPointerTypeExpr(PointerTypeExpr* expr) { return expr; }
    Expr* visitFuncTypeExpr(FuncTypeExpr* expr) { return expr; }
    Expr* visitTupleTypeExpr(TupleTypeExpr* expr) { return expr; }
    Expr* visitPackBranchTypeExpr(PackBranchTypeExpr* expr) { return expr; }
    Expr* visitOverloadedExpr(OverloadedExpr* expr) { return expr; }
    Expr* visitOverloadedExpr2(OverloadedExpr2* expr) { return expr; }
    Expr* visitPartiallyAppliedGenericExpr(PartiallyAppliedGenericExpr* expr) { return expr; }
};

Expr* SemanticsVisitor::finalizeSelectedExpression(
    Expr* expr,
    SelectedExpressionUse use,
    SourceLoc callableLocation)
{
    if (!expr)
        return expr;
    if (isCheckingPotentiallyUnevaluatedTerm())
        return expr;

    SelectedExpressionFinalizer finalizer(this);
    return finalizer.finalize(expr, use, callableLocation);
}

SelectedExpressionUse SemanticsVisitor::getSelectedExpressionUseForType(Type* type)
{
    return as<FuncType>(type) ? SelectedExpressionUse::CallableValue
                              : SelectedExpressionUse::Value;
}

Type* SemanticsVisitor::_toDifferentialParamType(Type* primalParamType)
{
    // This function is invoked on parameter types that could
    // still be wrapped to represent a parameter-passing mode
    // like `ref`, `out`, etc.
    //
    // We need to intercept these cases here, and ensure that
    // the wrapper is not exposed to other parts of the front-end
    // code, because they only exist to encode the parameter-passing
    // mode, and are not a proper part of the Slang type system
    // (at least not at this time).
    //
    if (auto primalParamWrapperType = as<ParamPassingModeType>(primalParamType))
    {
        // Some parameter-passing modes do not naturally lend themselves
        // to being differentiated - most notably, `ref` parameters.
        // We will detect those cases here, and handle them as a parameter
        // of a non-differentiable type would be handled.
        //
        // TODO(tfoley): With the introduction of `IDifferentiablePtrType`,
        // it is possible that something like a `ref` parameter could also
        // support autodiff, but it is not clear what a correct
        // one-size-fits-all behavior should be in that case.
        //
        if (as<RefParamType>(primalParamType))
            return primalParamWrapperType;

        // Given a primal type that is a wrapper like `Out<T>`, we can
        // extract the underlying primal value type `T`, and determine
        // what the differential type value type corresponding to `T`
        // should be.
        //
        auto primalValueType = primalParamWrapperType->getValueType();
        auto diffValueType = _toDifferentialParamType(primalValueType);

        // Once we have created the appropriate differential value type,
        // we will form the differential parameter type by wrapping
        // the differential value type in the same wrapper that had
        // been used for the primal type.
        //
        if (as<OutType>(primalParamWrapperType))
        {
            return m_astBuilder->getOutParamType(diffValueType);
        }
        else if (as<BorrowInOutParamType>(primalParamWrapperType))
        {
            return m_astBuilder->getBorrowInOutParamType(diffValueType);
        }
        else if (as<BorrowInParamType>(primalParamWrapperType))
        {
            return m_astBuilder->getConstRefParamType(diffValueType);
        }
        else
        {
            SLANG_UNEXPECTED("unhandled parameter-passing mode");
            UNREACHABLE_RETURN(diffValueType);
        }
    }

    if (auto diffPairType = tryGetDifferentialPairType(primalParamType))
        return diffPairType;
    else
        return primalParamType;
}

Type* SemanticsVisitor::tryGetDifferentialPairType(Type* primalType)
{
    if (auto modifiedType = as<ModifiedType>(primalType))
    {
        if (modifiedType->findModifier<NoDiffModifierVal>())
            return nullptr;
    }

    if (auto typePack = as<ConcreteTypePack>(primalType))
    {
        // The differential pair of a type pack should be a type pack of differential pairs.
        List<Type*> diffTypes;
        for (Index i = 0; i < typePack->getTypeCount(); i++)
        {
            auto t = typePack->getElementType(i);
            auto diffPairType = tryGetDifferentialPairType(t);
            diffTypes.add((diffPairType ? diffPairType : t));
        }
        return m_astBuilder->getTypePack(diffTypes.getArrayView());
    }
    else if (isAbstractTypePack(primalType))
    {
        // The differential pair of an abstract type pack P should be `expand
        // DifferentialPair<each P>`.
        auto eachType = m_astBuilder->getEachType(primalType);
        auto diffPairEachType = tryGetDifferentialPairType(eachType);
        if (!diffPairEachType)
            diffPairEachType = eachType;

        if (auto expandType = as<ExpandType>(primalType))
        {
            List<Val*> capturedPacks;
            for (Index i = 0; i < expandType->getCapturedPackCount(); i++)
            {
                capturedPacks.add(expandType->getCapturedPack(i));
            }
            return m_astBuilder->getExpandType(diffPairEachType, capturedPacks.getArrayView());
        }
        else
        {
            Val* primalVal = primalType;
            return m_astBuilder->getExpandType(diffPairEachType, makeArrayViewSingle(primalVal));
        }
    }

    // Get a reference to the builtin 'IDifferentiable' interface
    auto differentiableInterface = getASTBuilder()->getDifferentiableInterfaceType();
    auto differentiableRefInterface = getASTBuilder()->getDifferentiableRefInterfaceType();

    // Check if the provided type inherits from IDifferentiable.
    // If not, return the original type.
    if (auto conformanceWitness = isTypeDifferentiable(primalType))
    {
        if (conformanceWitness->getSup() == differentiableInterface)
        {
            return m_astBuilder->getDifferentialPairType(primalType, conformanceWitness);
        }
        else if (conformanceWitness->getSup() == differentiableRefInterface)
        {
            return m_astBuilder->getDifferentialPtrPairType(primalType, conformanceWitness);
        }
    }

    return nullptr;
}

Type* SemanticsVisitor::getForwardDiffFuncType(
    FuncType* originalType,
    std::optional<ParamInfo> thisParamInfo)
{
    // Resolve diff type here.
    // Note that this type checking needs to be in sync with
    // the auto-generation logic in slang-ir-diff-diff.cpp
    List<Type*> paramTypes;

    auto resultType = originalType->getResultType();
    if (auto resultPairType = tryGetDifferentialPairType(resultType))
        resultType = resultPairType;

    // No support for differentiating function that throw errors, for now.
    SLANG_ASSERT(originalType->getErrorType()->equals(m_astBuilder->getBottomType()));
    auto errorType = originalType->getErrorType();

    if (thisParamInfo)
    {
        auto differentiatedThisMode = getDifferentiatedThisParamMode(thisParamInfo->mode);
        auto thisParamType =
            getParamTypeWithModeWrapper(m_astBuilder, thisParamInfo->type, differentiatedThisMode);
        if (auto diffThisType = _toDifferentialParamType(thisParamType))
        {
            auto [diffThisValueType, diffThisMode] =
                getParamInfoFromTypeWithModeWrapper(diffThisType);
            paramTypes.add(
                getParamTypeWithModeWrapper(m_astBuilder, diffThisValueType, diffThisMode));
        }
    }

    for (Index i = 0; i < originalType->getParamCount(); i++)
    {
        if (auto jvpParamType =
                _toDifferentialParamType(originalType->getParamTypeWithModeWrapper(i)))
            paramTypes.add(jvpParamType);
    }

    FuncType* diffType =
        m_astBuilder->getFuncType(paramTypes.getArrayView(), resultType, errorType);

    return diffType;
}

Type* SemanticsVisitor::getBackwardDiffFuncType(
    FuncType* originalType,
    std::optional<ParamInfo> thisParamInfo)
{
    // Resolve backward diff type here.
    // Note that this type checking needs to be in sync with
    // the auto-generation logic in slang-ir-jvp-diff.cpp
    List<Type*> paramTypes;

    // The backward diff return type is void
    //
    auto resultType = m_astBuilder->getVoidType();

    // No support for differentiating function that throw errors, for now.
    SLANG_ASSERT(originalType->getErrorType()->equals(m_astBuilder->getBottomType()));
    auto errorType = originalType->getErrorType();

    // Handle the effective `this` parameter of a member method.
    if (thisParamInfo)
    {
        Type* differentiatedThisType = nullptr;
        auto differentiatedThisMode = getDifferentiatedThisParamMode(thisParamInfo->mode);
        if (auto diffPairType = tryGetDifferentialPairType(thisParamInfo->type))
        {
            differentiatedThisType = diffPairType;
            // A value pair must be passed as writable storage so that reverse-mode AD can
            // accumulate its differential. Pointer pairs already carry the required indirection,
            // and the remaining differentiated modes retain their parameter-passing behavior.
            if (as<DifferentialPairType>(diffPairType) &&
                differentiatedThisMode == ParamPassingMode::In)
            {
                differentiatedThisMode = ParamPassingMode::BorrowInOut;
            }
        }
        else
        {
            differentiatedThisType = doesTypeHaveNoDiffModifier(thisParamInfo->type)
                                         ? thisParamInfo->type
                                         : m_astBuilder->getModifiedType(
                                               thisParamInfo->type,
                                               {m_astBuilder->getNoDiffModifierVal()});
        }
        paramTypes.add(getParamTypeWithModeWrapper(
            m_astBuilder,
            differentiatedThisType,
            differentiatedThisMode));
    }

    for (Index i = 0; i < originalType->getParamCount(); i++)
    {
        auto paramValType = originalType->getParamValueType(i);
        auto paramPassingMode = originalType->getParamPassingMode(i);

        switch (paramPassingMode)
        {
        case ParamPassingMode::Out:
            {
                auto diffElementType = tryGetDifferentialValueType(m_astBuilder, paramValType);
                if (diffElementType)
                    paramTypes.add(diffElementType);

                break;
            }
        case ParamPassingMode::In:
            {
                if (auto diffPairValType = tryGetDifferentialPairType(paramValType))
                {
                    if (as<DifferentialPairType>(diffPairValType))
                        paramTypes.add(m_astBuilder->getBorrowInOutParamType(diffPairValType));
                    else if (as<DifferentialPtrPairType>(diffPairValType))
                        paramTypes.add(diffPairValType);
                }
                else
                {
                    paramTypes.add(m_astBuilder->getModifiedType(
                        paramValType,
                        {m_astBuilder->getNoDiffModifierVal()}));
                }

                break;
            }
        case ParamPassingMode::BorrowInOut:
            {
                if (auto diffPairValType = tryGetDifferentialPairType(paramValType))
                {
                    paramTypes.add(m_astBuilder->getBorrowInOutParamType(diffPairValType));
                }
                else
                {
                    paramTypes.add(m_astBuilder->getModifiedType(
                        paramValType,
                        {m_astBuilder->getNoDiffModifierVal()}));
                }

                break;
            }
        case ParamPassingMode::BorrowIn:
            {
                if (auto diffPairValType = tryGetDifferentialPairType(paramValType))
                {
                    paramTypes.add(m_astBuilder->getConstRefParamType(diffPairValType));
                }
                else
                {
                    paramTypes.add(m_astBuilder->getConstRefParamType(m_astBuilder->getModifiedType(
                        paramValType,
                        {m_astBuilder->getNoDiffModifierVal()})));
                }

                break;
            }
        case ParamPassingMode::Ref:
            {
                // Not allowed..
                SLANG_UNEXPECTED("ref parameter not allowed in backward diff function");
            }

            break;
        }
    }

    // Last parameter is the initial derivative of the original return type
    auto dOutType = tryGetDifferentialValueType(m_astBuilder, originalType->getResultType());
    if (dOutType)
        paramTypes.add(dOutType);

    return m_astBuilder->getFuncType(paramTypes.getArrayView(), resultType, errorType);
}

/// Returns the callable declaration ultimately transformed by `funcExpr`.
///
/// A generic reference names its `GenericDecl` wrapper, so the returned member declaration
/// reference maps that wrapper to its inner callable while preserving the wrapper's substitutions.
static DeclRef<CallableDecl> _getHigherOrderBaseFunctionDeclRef(
    SemanticsVisitor* semantics,
    Expr* funcExpr)
{
    auto innerExpr = getInnerMostExprFromHigherOrderExpr(funcExpr);
    auto declRefExpr = as<DeclRefExpr>(innerExpr);
    if (!declRefExpr)
        return DeclRef<CallableDecl>();

    auto declRef = declRefExpr->declRef;
    if (auto genericDeclRef = declRef.as<GenericDecl>())
    {
        auto innerDecl = genericDeclRef.getDecl()->inner;
        SLANG_RELEASE_ASSERT(innerDecl);
        declRef = semantics->getASTBuilder()->getMemberDeclRef(
            genericDeclRef,
            innerDecl);
    }
    return declRef.as<CallableDecl>();
}

/// Returns receiver information when a higher-order expression ultimately transforms an unbound
/// instance method.
///
/// Each nested differentiation layer treats the receiver as a receiver again, rather than as an
/// ordinary parameter inherited from the inner function type. For example,
/// `__fwd_diff(__fwd_diff(Type::method))` has receiver-related parameters for both derivative
/// layers.
static std::optional<ParamInfo> _getHigherOrderThisParamInfo(
    SemanticsVisitor* semantics,
    Expr* funcExpr)
{
    auto innerExpr = getInnerMostExprFromHigherOrderExpr(funcExpr);
    if (!as<StaticMemberExpr>(innerExpr))
        return std::nullopt;

    auto callableDeclRef = _getHigherOrderBaseFunctionDeclRef(semantics, funcExpr);
    if (!callableDeclRef)
        return std::nullopt;

    return semantics->findEffectiveThisParamInfo(callableDeclRef);
}

/// Centralizes construction of higher-order expression types and receiver-derived parameter
/// metadata.
///
/// Each subclass implements its operator-specific type transformation in
/// `fillHigherOrderInvokeExprImpl`. The shared construction routine invokes that transformation and
/// records the resulting type together with `receiverDerivedParameterCount`.
struct HigherOrderInvokeExprCheckingActions
{
    /// Creates the concrete expression for this higher-order operator.
    virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) = 0;

    /// Fills one higher-order expression and records its receiver-derived parameters.
    ///
    /// Common initialization and metadata recording stay here so the function type and
    /// `receiverDerivedParameterCount` remain consistent for every concrete operator.
    void fillHigherOrderInvokeExpr(
        HigherOrderInvokeExpr* resultExpr,
        SemanticsVisitor* semantics,
        Expr* funcExpr,
        std::optional<ParamInfo> thisParamInfo)
    {
        resultExpr->baseFunction = funcExpr;
        resultExpr->newParameterNames.clear();
        resultExpr->receiverDerivedParameterCount = 0;

        auto prependsReceiver = fillHigherOrderInvokeExprImpl(
            resultExpr,
            semantics,
            funcExpr,
            thisParamInfo);
        if (as<FuncType>(resultExpr->type.type))
        {
            setReceiverDerivedParameterCount(resultExpr, funcExpr, prependsReceiver);
        }
        else
        {
            SLANG_RELEASE_ASSERT(semantics->IsErrorExpr(resultExpr));
        }
    }

    /// Fills the operator-specific type and parameter names.
    ///
    /// Returns `true` exactly when this operator layer prepends one receiver-derived parameter.
    virtual bool fillHigherOrderInvokeExprImpl(
        HigherOrderInvokeExpr* resultExpr,
        SemanticsVisitor* semantics,
        Expr* funcExpr,
        std::optional<ParamInfo> thisParamInfo) = 0;

    /// Returns the stored function type, or the inner callable type of a generic reference.
    FuncType* getBaseFunctionType(SemanticsVisitor* semantics, Expr* funcExpr)
    {
        if (auto funcType = as<FuncType>(funcExpr->type.type))
            return funcType;
        auto astBuilder = semantics->getASTBuilder();
        if (auto declRefExpr =
                as<DeclRefExpr>(getCallableExprThroughTransparentWrappers(funcExpr)))
        {
            if (auto baseFuncGenericDeclRef = declRefExpr->declRef.as<GenericDecl>())
            {
                DeclRef<Decl> unspecializedInnerRef = createDefaultSubstitutionsIfNeeded(
                    astBuilder,
                    semantics,
                    astBuilder->getMemberDeclRef(
                        baseFuncGenericDeclRef,
                        getInner(baseFuncGenericDeclRef)));
                auto callableDeclRef = unspecializedInnerRef.as<CallableDecl>();
                if (!callableDeclRef)
                    return nullptr;
                auto funcType = getFuncType(astBuilder, callableDeclRef);
                return funcType;
            }
        }
        return nullptr;
    }

    /// Sets the number of leading parameters derived from a receiver.
    ///
    /// For `__fwd_diff(__fwd_diff(Type::method))`, the outer type inherits the inner receiver
    /// parameters and may prepend another receiver-related parameter. Centralizing this step keeps
    /// every higher-order operator's type and prefix count in sync.
    void setReceiverDerivedParameterCount(
        HigherOrderInvokeExpr* resultExpr,
        Expr* funcExpr,
        bool prependsReceiver)
    {
        Count count = prependsReceiver ? 1 : 0;
        if (auto baseHigherOrderExpr =
                as<HigherOrderInvokeExpr>(getCallableExprThroughTransparentWrappers(funcExpr)))
            count += baseHigherOrderExpr->receiverDerivedParameterCount;

        auto resultFuncType = as<FuncType>(resultExpr->type.type);
        SLANG_RELEASE_ASSERT(resultFuncType);
        SLANG_RELEASE_ASSERT(count <= resultFuncType->getParamCount());
        resultExpr->receiverDerivedParameterCount = uint32_t(count);
    }
};

struct ForwardDifferentiateExprCheckingActions : HigherOrderInvokeExprCheckingActions
{
    virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
    {
        return semantics->getASTBuilder()->create<ForwardDifferentiateExpr>();
    }
    bool fillHigherOrderInvokeExprImpl(
        HigherOrderInvokeExpr* resultDiffExpr,
        SemanticsVisitor* semantics,
        Expr* funcExpr,
        std::optional<ParamInfo> thisParamInfo) override
    {
        auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
        if (!baseFuncType)
        {
            resultDiffExpr->type = semantics->getASTBuilder()->getErrorType();
            semantics->getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = funcExpr});
            return false;
        }
        resultDiffExpr->type =
            semantics->getForwardDiffFuncType(baseFuncType, thisParamInfo);
        if (auto funcDeclRef = _getHigherOrderBaseFunctionDeclRef(semantics, funcExpr))
        {
            if (thisParamInfo)
                resultDiffExpr->newParameterNames.add(semantics->getName("this"));
            for (auto paramDeclRef :
                 getParametersForCallableSignature(semantics->getASTBuilder(), funcDeclRef))
            {
                resultDiffExpr->newParameterNames.add(paramDeclRef.getName());
            }
        }
        return thisParamInfo.has_value();
    }
};

struct BackwardDifferentiateExprCheckingActions : HigherOrderInvokeExprCheckingActions
{
    virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
    {
        return semantics->getASTBuilder()->create<BackwardDifferentiateExpr>();
    }
    bool fillHigherOrderInvokeExprImpl(
        HigherOrderInvokeExpr* resultDiffExpr,
        SemanticsVisitor* semantics,
        Expr* funcExpr,
        std::optional<ParamInfo> thisParamInfo) override
    {
        auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
        if (!baseFuncType)
        {
            resultDiffExpr->type = semantics->getASTBuilder()->getErrorType();
            semantics->getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = funcExpr});
            return false;
        }
        resultDiffExpr->type =
            semantics->getBackwardDiffFuncType(baseFuncType, thisParamInfo);
        if (auto funcDeclRef = _getHigherOrderBaseFunctionDeclRef(semantics, funcExpr))
        {
            if (thisParamInfo)
                resultDiffExpr->newParameterNames.add(semantics->getName("this"));
            for (auto paramDeclRef :
                 getParametersForCallableSignature(semantics->getASTBuilder(), funcDeclRef))
            {
                auto param = paramDeclRef.getDecl();
                if (param->findModifier<NoDiffModifier>() &&
                    getParamPassingMode(param) == ParamPassingMode::Out)
                {
                    continue;
                }
                resultDiffExpr->newParameterNames.add(param->getName());
            }
            resultDiffExpr->newParameterNames.add(semantics->getName("resultGradient"));
        }
        return thisParamInfo.has_value();
    }
};

template<typename ExprASTType>
struct PassthroughHigherOrderExprCheckingActionsBase : HigherOrderInvokeExprCheckingActions
{
    virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
    {
        return semantics->getASTBuilder()->create<ExprASTType>();
    }
    bool fillHigherOrderInvokeExprImpl(
        HigherOrderInvokeExpr* resultExpr,
        SemanticsVisitor* semantics,
        Expr* funcExpr,
        std::optional<ParamInfo> /* thisParamInfo */) override
    {
        auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
        if (!baseFuncType)
        {
            resultExpr->type = semantics->getASTBuilder()->getErrorType();
            semantics->getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = funcExpr});
            return false;
        }
        resultExpr->type = baseFuncType;
        if (auto funcDeclRef = _getHigherOrderBaseFunctionDeclRef(semantics, funcExpr))
        {
            for (auto paramDeclRef :
                 getParametersForCallableSignature(semantics->getASTBuilder(), funcDeclRef))
            {
                resultExpr->newParameterNames.add(paramDeclRef.getName());
            }
        }
        return false;
    }
};

/// Checks one higher-order operator while preserving overloads of its base function.
///
/// For an overloaded base, `_checkHigherOrderInvokeExpr()` produces one higher-order expression per
/// candidate and stores the actual function type and number of receiver-derived leading parameters
/// on each one. `getHigherOrderOverloadComparisonType()` substitutes `in` for those parameters only
/// in the type used by overload resolution.
static Expr* _checkHigherOrderInvokeExpr(
    SemanticsVisitor* semantics,
    HigherOrderInvokeExpr* expr,
    HigherOrderInvokeExprCheckingActions* actions)
{
    // The subvisitor permits an unbound instance method such as `Type::method` as the operand.
    SemanticsVisitor subVisitor(semantics->allowStaticReferenceToNonStaticMember());
    expr->baseFunction = subVisitor.dispatchExpr(expr->baseFunction, subVisitor);

    auto selectedBaseFunction =
        getCallableExprThroughTransparentWrappers(expr->baseFunction);
    auto resolvedBaseFunction = subVisitor.maybeResolveOverloadedExpr(
        selectedBaseFunction,
        LookupMask::Function,
        nullptr);
    if (resolvedBaseFunction != selectedBaseFunction)
    {
        semantics->replaceSelectedCallableThroughTransparentWrappers(
            expr->baseFunction,
            resolvedBaseFunction);
        selectedBaseFunction = resolvedBaseFunction;
    }

    if (semantics->IsErrorExpr(selectedBaseFunction))
        return semantics->CreateErrorExpr(expr);

    auto astBuilder = semantics->getASTBuilder();

    // `_checkHigherOrderInvokeExpr()` distributes the higher-order operator over an overloaded
    // operand's candidates. `ConstructLookupResultExpr()` reconstructs each candidate and
    // `fillHigherOrderInvokeExpr()` stores its checked higher-order type.
    if (auto overloadedExpr = as<OverloadedExpr>(selectedBaseFunction))
    {
        OverloadedExpr2* result = astBuilder->create<OverloadedExpr2>();
        for (auto item : overloadedExpr->lookupResult2)
        {
            auto lookupResultExpr = subVisitor.ConstructLookupResultExpr(
                item,
                overloadedExpr->base,
                overloadedExpr->name,
                overloadedExpr->loc,
                overloadedExpr->originalExpr);
            auto candidateExpr = actions->createHigherOrderInvokeExpr(semantics);
            actions->fillHigherOrderInvokeExpr(
                candidateExpr,
                semantics,
                rebuildCallableTransparentWrappers(
                    astBuilder,
                    expr->baseFunction,
                    lookupResultExpr),
                _getHigherOrderThisParamInfo(semantics, lookupResultExpr));
            candidateExpr->loc = expr->loc;
            result->candidateExprs.add(candidateExpr);
        }
        result->type.type = astBuilder->getOverloadedType();
        result->loc = expr->loc;
        return result;
    }
    else if (auto overloadedExpr2 = as<OverloadedExpr2>(selectedBaseFunction))
    {
        OverloadedExpr2* result = astBuilder->create<OverloadedExpr2>();
        for (auto item : overloadedExpr2->candidateExprs)
        {
            auto candidateExpr = actions->createHigherOrderInvokeExpr(semantics);
            actions->fillHigherOrderInvokeExpr(
                candidateExpr,
                semantics,
                rebuildCallableTransparentWrappers(astBuilder, expr->baseFunction, item),
                _getHigherOrderThisParamInfo(semantics, item));
            candidateExpr->loc = expr->loc;
            result->candidateExprs.add(candidateExpr);
        }
        result->type.type = astBuilder->getOverloadedType();
        result->loc = expr->loc;
        return result;
    }

    // An overload set must remain intact until ordinary call resolution selects a candidate. Once
    // the operand is known to be a single differentiation target, however, a bound instance method
    // cannot be represented by differentiation IR and must not reach lowering.
    if (auto differentiateExpr = as<DifferentiateExpr>(expr))
    {
        if (_diagnoseBoundInstanceMethodDifferentiationOperand(semantics, differentiateExpr))
            return semantics->CreateErrorExpr(expr);
    }

    actions->fillHigherOrderInvokeExpr(
        expr,
        semantics,
        expr->baseFunction,
        _getHigherOrderThisParamInfo(semantics, expr->baseFunction));
    return expr;
}

Expr* SemanticsExprVisitor::visitForwardDifferentiateExpr(ForwardDifferentiateExpr* expr)
{
    ForwardDifferentiateExprCheckingActions actions;
    return _checkHigherOrderInvokeExpr(this, expr, &actions);
}

Expr* SemanticsExprVisitor::visitBackwardDifferentiateExpr(BackwardDifferentiateExpr* expr)
{
    BackwardDifferentiateExprCheckingActions actions;
    return _checkHigherOrderInvokeExpr(this, expr, &actions);
}

struct ApplyForBwdExprCheckingActions : HigherOrderInvokeExprCheckingActions
{
    virtual HigherOrderInvokeExpr* createHigherOrderInvokeExpr(SemanticsVisitor* semantics) override
    {
        return semantics->getASTBuilder()->create<ApplyForBwdExpr>();
    }
    bool fillHigherOrderInvokeExprImpl(
        HigherOrderInvokeExpr* resultExpr,
        SemanticsVisitor* semantics,
        Expr* funcExpr,
        std::optional<ParamInfo> thisParamInfo) override
    {
        auto baseFuncType = getBaseFunctionType(semantics, funcExpr);
        if (!baseFuncType)
        {
            resultExpr->type = semantics->getASTBuilder()->getErrorType();
            semantics->getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = funcExpr});
            return false;
        }
        // `__apply(fn)` keeps the base function's parameter types instead of wrapping them in
        // `DifferentialPair`, so overload resolution compares the original argument types.
        if (thisParamInfo)
        {
            List<Type*> paramTypes;
            paramTypes.add(getParamTypeWithModeWrapper(
                semantics->getASTBuilder(),
                thisParamInfo->type,
                thisParamInfo->mode));
            for (Index i = 0; i < baseFuncType->getParamCount(); i++)
                paramTypes.add(baseFuncType->getParamTypeWithModeWrapper(i));

            resultExpr->type = semantics->getASTBuilder()->getFuncType(
                paramTypes.getArrayView(),
                baseFuncType->getResultType(),
                baseFuncType->getErrorType());
        }
        else
        {
            resultExpr->type = baseFuncType;
        }
        if (auto funcDeclRef = _getHigherOrderBaseFunctionDeclRef(semantics, funcExpr))
        {
            if (thisParamInfo)
                resultExpr->newParameterNames.add(semantics->getName("this"));
            for (auto paramDeclRef :
                 getParametersForCallableSignature(semantics->getASTBuilder(), funcDeclRef))
            {
                resultExpr->newParameterNames.add(paramDeclRef.getName());
            }
        }
        return thisParamInfo.has_value();
    }
};

FuncType* SemanticsVisitor::getHigherOrderOverloadComparisonType(
    HigherOrderInvokeExpr* expr,
    FuncType* funcType)
{
    SLANG_RELEASE_ASSERT(expr);
    SLANG_RELEASE_ASSERT(funcType);

    auto receiverDerivedParameterCount = expr->receiverDerivedParameterCount;
    if (receiverDerivedParameterCount == 0)
        return funcType;

    List<Type*> parameterTypes;
    for (auto parameterType : funcType->getParamTypes())
        parameterTypes.add(parameterType);

    bool changed = false;
    SLANG_RELEASE_ASSERT(
        receiverDerivedParameterCount <= uint32_t(parameterTypes.getCount()));
    for (Index i = 0; i < receiverDerivedParameterCount; ++i)
    {
        auto parameterInfo = getParamInfoFromTypeWithModeWrapper(parameterTypes[i]);
        if (parameterInfo.mode == ParamPassingMode::In)
            continue;

        parameterTypes[i] =
            getParamTypeWithModeWrapper(m_astBuilder, parameterInfo.type, ParamPassingMode::In);
        changed = true;
    }

    if (!changed)
        return funcType;

    return m_astBuilder->getFuncType(
        parameterTypes.getArrayView(),
        funcType->getResultType(),
        funcType->getErrorType());
}

Expr* SemanticsExprVisitor::visitApplyForBwdExpr(ApplyForBwdExpr* expr)
{
    if (!getOptionSet().getBoolOption(CompilerOptionName::ExperimentalFeature))
    {
        getSink()->diagnose(Diagnostics::ApplyForBwdRequiresExperimentalFeature{.expr = expr});

        // Warnings allow compilation to continue; a non-callable placeholder prevents later
        // stages from treating the gated syntax as an apply expression.
        auto placeholderExpr = getASTBuilder()->create<DefaultConstructExpr>();
        placeholderExpr->loc = expr->loc;
        placeholderExpr->type = getASTBuilder()->getVoidType();
        return placeholderExpr;
    }

    ApplyForBwdExprCheckingActions actions;
    return _checkHigherOrderInvokeExpr(this, expr, &actions);
}

Expr* SemanticsExprVisitor::visitFuncAsTypeExpr(FuncAsTypeExpr* expr)
{
    auto context = withPotentiallyUnevaluatedTerm()
                       .allowStaticReferenceToNonStaticMember();
    SemanticsVisitor visitor(context);
    expr->base = visitor.CheckTerm(expr->base);

    if (auto overloadedExpr = as<OverloadedExpr>(expr->base))
        expr->base = maybeResolveOverloadedExpr(overloadedExpr, LookupMask::Function, nullptr);

    // Expect a func-decl-ref expr
    auto declRefExpr = as<DeclRefExpr>(expr->base);
    if (!declRefExpr)
    {
        // Diagnose.
        getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = expr->base});
    }

    auto funcDeclRef = declRefExpr->declRef.as<CallableDecl>();
    if (!funcDeclRef)
    {
        // Diagnose.
        getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = expr->base});
    }

    auto declRefType = DeclRefType::create(m_astBuilder, funcDeclRef);

    // Create a shared type expr.
    auto typeExpr = m_astBuilder->create<SharedTypeExpr>();
    auto typetype = m_astBuilder->getOrCreate<TypeType>(declRefType);
    typeExpr->type = typetype;

    return typeExpr;
}

Expr* SemanticsExprVisitor::visitFuncTypeOfExpr(FuncTypeOfExpr* expr)
{
    expr->base = checkPotentiallyUnevaluatedTerm(expr->base);
    auto funcType = as<FuncType>(expr->base->type.type);
    if (!funcType)
    {
        getSink()->diagnose(Diagnostics::ExpectedFunction{.expr = expr->base});
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }
    else
    {
        // Create a shared type expr.
        auto typeExpr = m_astBuilder->create<SharedTypeExpr>();
        auto typetype = m_astBuilder->getOrCreate<TypeType>(funcType);
        typeExpr->type = typetype;
        return typeExpr;
    }
}

Expr* SemanticsExprVisitor::visitPrimalSubstituteExpr(PrimalSubstituteExpr* expr)
{
    PassthroughHigherOrderExprCheckingActionsBase<PrimalSubstituteExpr> actions;
    return _checkHigherOrderInvokeExpr(this, expr, &actions);
}

Expr* SemanticsExprVisitor::visitDispatchKernelExpr(DispatchKernelExpr* expr)
{
    auto isInt3Type = [this](Type* type)
    {
        auto vectorType = as<VectorExpressionType>(type);
        if (!vectorType)
            return false;
        if (!isIntegerBaseType(getVectorBaseType(vectorType)))
            return false;
        auto constElementCount = as<ConstantIntVal>(vectorType->getElementCount());
        if (!constElementCount)
            return false;
        return constElementCount->getValue() == 3;
    };
    expr->dispatchSize = dispatchExpr(expr->dispatchSize, *this);
    if (!isInt3Type(expr->dispatchSize->type.type))
    {
        auto uint3Type = m_astBuilder->getVectorType(
            m_astBuilder->getUIntType(),
            m_astBuilder->getIntVal(m_astBuilder->getIntType(), 3));
        getSink()->diagnose(Diagnostics::TypeMismatch{
            .expectedType = uint3Type,
            .actualType = expr->dispatchSize->type,
            .expr = expr->dispatchSize});
    }
    expr->threadGroupSize = dispatchExpr(expr->threadGroupSize, *this);
    if (!isInt3Type(expr->threadGroupSize->type.type))
    {
        auto uint3Type = m_astBuilder->getVectorType(
            m_astBuilder->getUIntType(),
            m_astBuilder->getIntVal(m_astBuilder->getIntType(), 3));
        getSink()->diagnose(Diagnostics::TypeMismatch{
            .expectedType = uint3Type,
            .actualType = expr->threadGroupSize->type,
            .expr = expr->threadGroupSize});
    }
    PassthroughHigherOrderExprCheckingActionsBase<DispatchKernelExpr> actions;
    return _checkHigherOrderInvokeExpr(this, expr, &actions);
}

Expr* SemanticsExprVisitor::visitTreatAsDifferentiableExpr(TreatAsDifferentiableExpr* expr)
{
    auto subContext = withTreatAsDifferentiable(expr).allowDroppingDerivatives();
    expr->innerExpr = dispatchExpr(expr->innerExpr, subContext);
    expr->type = expr->innerExpr->type;
    expr->abstractStorageAccess = expr->innerExpr->abstractStorageAccess;

    auto innerExpr = expr->innerExpr;
    while (auto parenExpr = as<ParenExpr>(innerExpr))
    {
        innerExpr = parenExpr->base;
    }
    if (!as<InvokeExpr>(innerExpr) && !as<IndexExpr>(innerExpr))
    {
        getSink()->diagnose(Diagnostics::InvalidUseOfNoDiff{.expr = expr});
    }
    else if (!m_parentDifferentiableAttr)
    {
        getSink()->diagnose(Diagnostics::CannotUseNoDiffInNonDifferentiableFunc{.expr = expr});
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitGetArrayLengthExpr(GetArrayLengthExpr* expr)
{
    expr->arrayExpr = checkPotentiallyUnevaluatedTerm(expr->arrayExpr);
    if (auto arrType = as<ArrayExpressionType>(expr->arrayExpr->type))
    {
        expr->type = m_astBuilder->getIntType();
        if (arrType->isUnsized())
        {
            getSink()->diagnose(Diagnostics::InvalidArraySize{.location = expr->loc});
        }
    }
    else
    {
        if (!as<ErrorType>(expr->arrayExpr->type))
        {
            getSink()->diagnose(Diagnostics::ExpectedArrayExpression{
                .actualType = expr->arrayExpr->type,
                .expr = expr->arrayExpr});
        }
        expr->type = m_astBuilder->getErrorType();
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitDefaultConstructExpr(DefaultConstructExpr* expr)
{
    return expr;
}

Expr* SemanticsExprVisitor::visitDetachExpr(DetachExpr* expr)
{
    expr->inner = CheckTerm(expr->inner);
    expr->type = getTypeWithModifier(
        expr->inner->type,
        getCurrentASTBuilder()->getOrCreate<NoDiffModifierVal>());
    expr->type.isLeftValue = false;
    expr->type.isWriteOnly = false;
    expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
        m_astBuilder,
        expr->inner->abstractStorageAccess,
        AbstractStorageUse::ReadValue);
    return expr;
}


static bool _isSizeOfType(Type* type)
{
    if (!type)
    {
        return false;
    }

    // A `ModifiedType`'s modifiers are layout-transparent, so whether a type has
    // a size is decided entirely by its base: `sizeof(unorm float4)` is just
    // `sizeof(float4)`. This matters because the modifier survives into the type
    // of an ordinary value — loading from a `RWTexture2D<unorm float4>` yields a
    // `unorm float4` — so rejecting the wrapper here would reject `sizeof` on a
    // value the user never spelled a modifier on. Unwrapping keeps the list
    // below about *kinds* of type rather than repeating each kind in a modified
    // form.
    type = unwrapModifiedType(type);

    if (as<ArithmeticExpressionType>(type) || as<ArrayExpressionType>(type) ||
        as<PtrTypeBase>(type) || as<TupleType>(type) || as<GenericDeclRefType>(type))
    {
        return true;
    }

    if (as<DeclRefType>(type))
    {
        return true;
    }

    return false;
}

static bool _isTypeOrValValidForCountOf(Type* type)
{
    if (!type)
    {
        return false;
    }

    if (isTypePack(type))
    {
        return true;
    }

    if (as<TupleType>(type))
    {
        return true;
    }

    if (auto arrayType = as<ArrayExpressionType>(type))
    {
        // Only a fixed-size array has a statically known element count. An
        // unsized array has none, so `countof` on it is not a compile-time
        // constant and must be diagnosed here rather than lowered to a
        // `kIROp_CountOf` that no pass can fold and no backend can emit.
        return !arrayType->isUnsized();
    }

    if (as<ValuePackType>(type))
    {
        return true;
    }

    return false;
}

static bool _isTypeOrValValidForPackQuery(Type* type)
{
    if (!type)
        return false;

    if (isTypePack(type))
        return true;

    if (as<TupleType>(type))
        return true;

    if (as<ValuePackType>(type))
        return true;

    return false;
}

static bool _isTypeOrValValidForPackBranch(Type* type)
{
    return _isTypeOrValValidForPackQuery(type);
}

static bool _isExactIntType(Type* type)
{
    if (!type)
        return false;
    type = unwrapModifiedType(type);
    if (auto basicType = as<BasicExpressionType>(type))
        return basicType->getBaseType() == BaseType::Int;
    return false;
}

static bool _isTupleOfExactInt(TupleType* tupleType)
{
    if (!tupleType)
        return false;
    for (Index i = 0; i < tupleType->getMemberCount(); ++i)
    {
        if (!_isExactIntType(tupleType->getMember(i)))
            return false;
    }
    return true;
}

enum class ShapePackOperandKind
{
    Invalid,
    ValuePack,
    Tuple,
};

struct ShapePackOperandInfo
{
    ShapePackOperandKind kind = ShapePackOperandKind::Invalid;
    Type* type = nullptr;
    ValuePackType* valuePackType = nullptr;
    TupleType* tupleType = nullptr;
    Index rank = 0;
};

static ShapePackOperandInfo _getShapePackOperandInfo(Type* type)
{
    ShapePackOperandInfo info;
    info.type = type;

    if (auto valuePackType = as<ValuePackType>(type))
    {
        if (_isExactIntType(valuePackType->getElementType()))
        {
            info.kind = ShapePackOperandKind::ValuePack;
            info.valuePackType = valuePackType;
        }
        return info;
    }

    if (auto tupleType = as<TupleType>(type))
    {
        if (_isTupleOfExactInt(tupleType))
        {
            info.kind = ShapePackOperandKind::Tuple;
            info.tupleType = tupleType;
            info.rank = tupleType->getMemberCount();
        }
        return info;
    }

    return info;
}

static bool _tryGetShapePackRank(Type* type, Val* foldedVal, Index& outRank)
{
    if (auto tupleType = as<TupleType>(type))
    {
        outRank = tupleType->getMemberCount();
        return true;
    }

    if (auto concreteIntValPack = as<ConcreteIntValPack>(foldedVal))
    {
        outRank = concreteIntValPack->getCount();
        return true;
    }

    return false;
}

static Type* _createIntTupleType(ASTBuilder* astBuilder, Index rank)
{
    ShortList<Type*> elementTypes;
    for (Index i = 0; i < rank; ++i)
        elementTypes.add(astBuilder->getIntType());
    return astBuilder->getTupleType(elementTypes.getArrayView().arrayView);
}

bool SemanticsVisitor::hasNonEmptyPackConstraint(Decl* decl)
{
    auto genericDecl = decl ? as<GenericDecl>(decl->parentDecl) : nullptr;
    if (!genericDecl)
        return false;

    for (auto constraintDecl :
         genericDecl->getDirectMemberDeclsOfType<NonEmptyPackConstraintDecl>())
    {
        if (auto packDeclRefExpr = as<DeclRefExpr>(constraintDecl->packExpr))
        {
            if (getDeclRef(m_astBuilder, packDeclRefExpr).getDecl() == decl)
            {
                return true;
            }
        }
    }
    return false;
}

VariadicPackCardinality SemanticsVisitor::getPackCardinality(Val* packVal)
{
    if (!packVal)
        return VariadicPackCardinality::Unknown;

    if (auto packType = as<Type>(packVal))
        packVal = unwrapModifiedType(packType);

    for (auto assumedInfo = m_assumedNonEmptyPack; assumedInfo; assumedInfo = assumedInfo->next)
    {
        if (assumedInfo->pack == packVal)
            return VariadicPackCardinality::NonEmpty;
    }

    if (auto cached = getShared()->m_packCardinalityCache.tryGetValue(packVal))
        return *cached;

    auto mergeCaptureCardinality = [&](auto packWithCaptures) -> VariadicPackCardinality
    {
        VariadicPackCardinality result = VariadicPackCardinality::Unknown;
        for (Index i = 0; i < packWithCaptures->getCapturedPackCount(); ++i)
        {
            auto capturedPack = packWithCaptures->getCapturedPack(i);
            auto capturedCardinality = getPackCardinality(capturedPack);
            if (capturedCardinality == VariadicPackCardinality::Empty)
                return VariadicPackCardinality::Empty;
            if (capturedCardinality == VariadicPackCardinality::NonEmpty)
                result = VariadicPackCardinality::NonEmpty;
        }
        return result;
    };

    VariadicPackCardinality result = VariadicPackCardinality::Unknown;

    result = getPackCardinalityFromStructure(
        packVal,
        [&](Val* operand) { return getPackCardinality(operand); });
    if (result != VariadicPackCardinality::Unknown)
    {
        getShared()->m_packCardinalityCache[packVal] = result;
        return result;
    }

    if (auto expandType = as<ExpandType>(packVal))
    {
        result = mergeCaptureCardinality(expandType);
        getShared()->m_packCardinalityCache[packVal] = result;
        return result;
    }

    if (auto expandIntValPack = as<ExpandIntValPack>(packVal))
    {
        result = mergeCaptureCardinality(expandIntValPack);
        getShared()->m_packCardinalityCache[packVal] = result;
        return result;
    }

    if (as<TrimFirstTypePack>(packVal) || as<TrimLastTypePack>(packVal) ||
        as<TrimFirstIntValPack>(packVal) || as<TrimLastIntValPack>(packVal))
    {
        result = VariadicPackCardinality::Unknown;
        getShared()->m_packCardinalityCache[packVal] = result;
        return result;
    }

    if (auto declRefType = as<DeclRefType>(packVal))
    {
        if (auto typePackParam = declRefType->getDeclRef().as<GenericTypePackParamDecl>())
        {
            result = hasNonEmptyPackConstraint(typePackParam.getDecl())
                         ? VariadicPackCardinality::NonEmpty
                         : VariadicPackCardinality::Unknown;
            getShared()->m_packCardinalityCache[packVal] = result;
            return result;
        }
    }

    if (auto declRefIntVal = as<DeclRefIntVal>(packVal))
    {
        if (auto valuePackParam = declRefIntVal->getDeclRef().as<GenericValuePackParamDecl>())
        {
            result = hasNonEmptyPackConstraint(valuePackParam.getDecl())
                         ? VariadicPackCardinality::NonEmpty
                         : VariadicPackCardinality::Unknown;
            getShared()->m_packCardinalityCache[packVal] = result;
            return result;
        }
    }

    getShared()->m_packCardinalityCache[packVal] = result;
    return result;
}

bool SemanticsVisitor::isKnownNonEmptyPack(Val* packVal)
{
    return getPackCardinality(packVal) == VariadicPackCardinality::NonEmpty;
}

Expr* SemanticsExprVisitor::visitSizeOfLikeExpr(SizeOfLikeExpr* sizeOfLikeExpr)
{
    auto valueExpr = checkPotentiallyUnevaluatedTerm(sizeOfLikeExpr->value);
    sizeOfLikeExpr->value = valueExpr;
    sizeOfLikeExpr->type = m_astBuilder->getIntType();

    Type* type = nullptr;

    if (as<TypeType>(valueExpr->type))
    {
        TypeExp typeExp;
        typeExp.exp = valueExpr;

        auto properTypeExpr = CoerceToProperType(typeExp);

        type = properTypeExpr.type;
    }
    else
    {
        // Is this a proper type?
        TypeExp typeExp(valueExpr->type);
        TypeExp properType = tryCoerceToProperType(typeExp);

        type = properType.type;
    }

    if (as<CountOfExpr>(sizeOfLikeExpr))
    {
        if (!_isTypeOrValValidForCountOf(type))
        {
            getSink()->diagnose(Diagnostics::CountOfArgumentIsInvalid{.expr = sizeOfLikeExpr});

            sizeOfLikeExpr->type = m_astBuilder->getErrorType();
            return sizeOfLikeExpr;
        }
    }
    else
    {
        if (!_isSizeOfType(type))
        {
            getSink()->diagnose(Diagnostics::SizeOfArgumentIsInvalid{.expr = sizeOfLikeExpr});

            sizeOfLikeExpr->type = m_astBuilder->getErrorType();
            return sizeOfLikeExpr;
        }

        Type* dataLayoutType = nullptr;
        if (sizeOfLikeExpr->dataLayout)
        {
            auto dataLayoutExpr =
                checkPotentiallyUnevaluatedTerm(sizeOfLikeExpr->dataLayout);
            sizeOfLikeExpr->dataLayout = dataLayoutExpr;
            if (as<TypeType>(dataLayoutExpr->type))
            {
                TypeExp typeExp;
                typeExp.exp = dataLayoutExpr;
                auto properTypeExpr = CoerceToProperType(typeExp);
                dataLayoutType = properTypeExpr.type;
            }
        }
        else
        {
            dataLayoutType = m_astBuilder->getScalarLayoutType();
        }

        SubtypeWitness* witness =
            dataLayoutType ? as<SubtypeWitness>(tryGetInterfaceConformanceWitness(
                                 dataLayoutType,
                                 m_astBuilder->getSharedASTBuilder()->getIBufferDataLayoutType()))
                           : nullptr;
        if (!dataLayoutType || !witness)
        {
            getSink()->diagnose(
                Diagnostics::SizeOfDataLayoutIsInvalid{.expr = sizeOfLikeExpr->dataLayout});

            sizeOfLikeExpr->type = m_astBuilder->getErrorType();
            return sizeOfLikeExpr;
        }

        sizeOfLikeExpr->dataLayoutType = dataLayoutType;

        // Note: DescriptorHandle size is target-dependent (uint64_t for spvBindlessTextureNV,
        // uint2 otherwise). The size calculation is deferred to IR level where target
        // capabilities are available. See slang-ir-peephole.cpp for the resolution logic.
    }

    sizeOfLikeExpr->sizedType = type;
    if (_doesSizeOfLikeExprEvaluateValue(sizeOfLikeExpr))
    {
        sizeOfLikeExpr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            sizeOfLikeExpr->value->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
    }

    return sizeOfLikeExpr;
}

Expr* SemanticsExprVisitor::visitPackQueryExpr(PackQueryExpr* packQueryExpr)
{
    auto valueExpr = dispatch(packQueryExpr->value);
    packQueryExpr->value = valueExpr;

    auto queryName = getPackQueryName(packQueryExpr);
    bool isTypeExpr = false;
    Type* operandType = nullptr;

    if ([[maybe_unused]] auto typeType = as<TypeType>(valueExpr->type))
    {
        isTypeExpr = true;
        TypeExp typeExp;
        typeExp.exp = valueExpr;
        operandType = CoerceToProperType(typeExp).type;
    }
    else
    {
        operandType = valueExpr->type.type;
    }

    if (!_isTypeOrValValidForPackQuery(operandType))
    {
        getSink()->diagnose(
            Diagnostics::PackQueryArgumentIsInvalid{.queryName = queryName, .expr = packQueryExpr});
        packQueryExpr->type = m_astBuilder->getErrorType();
        return packQueryExpr;
    }

    Val* foldedOperandVal =
        tryConstantFoldExpr(SubstExpr<Expr>(valueExpr), ConstantFoldingKind::CompileTime, nullptr);
    Val* cardinalitySource = foldedOperandVal;
    if (!cardinalitySource && (isTypeExpr || isTypePack(operandType) || as<TupleType>(operandType)))
        cardinalitySource = operandType;

    auto packCardinality = cardinalitySource ? getPackCardinality(cardinalitySource)
                                             : VariadicPackCardinality::Unknown;

    auto isFirstOrLastQuery = [&]()
    { return as<FirstExpr>(packQueryExpr) || as<LastExpr>(packQueryExpr); };
    auto applyPackQueryToType = [&](Type* type) -> Type*
    {
        if (as<FirstExpr>(packQueryExpr))
            return m_astBuilder->getFirstElement(type);
        if (as<LastExpr>(packQueryExpr))
            return m_astBuilder->getLastElement(type);
        if (as<TrimFirstExpr>(packQueryExpr))
            return m_astBuilder->getTrimFirstPack(type);
        return m_astBuilder->getTrimLastPack(type);
    };

    if (isFirstOrLastQuery())
    {
        if (packCardinality == VariadicPackCardinality::Empty)
        {
            getSink()->diagnose(Diagnostics::EmptyPackQueryIsInvalid{
                .queryName = queryName,
                .expr = packQueryExpr});
            packQueryExpr->type = m_astBuilder->getErrorType();
            return packQueryExpr;
        }

        if (packCardinality != VariadicPackCardinality::NonEmpty)
        {
            getSink()->diagnose(Diagnostics::PackQueryRequiresNonEmptyPack{
                .queryName = queryName,
                .expr = packQueryExpr});
            packQueryExpr->type = m_astBuilder->getErrorType();
            return packQueryExpr;
        }
    }

    if (isTypeExpr)
    {
        Type* resultType = applyPackQueryToType(operandType);
        packQueryExpr->type = m_astBuilder->getTypeType(resultType);
        return packQueryExpr;
    }

    // A term-valued pack query evaluates the complete operand before selecting or trimming its
    // elements. The query result is an r-value, so every later use has already committed a read of
    // the operand's abstract storage.
    packQueryExpr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
        m_astBuilder,
        valueExpr->abstractStorageAccess,
        AbstractStorageUse::ReadValue);

    if (auto valuePackType = as<ValuePackType>(operandType))
    {
        if (isFirstOrLastQuery())
            packQueryExpr->type = QualType(valuePackType->getElementType());
        else
            packQueryExpr->type = QualType(valuePackType);
        return packQueryExpr;
    }

    if (!isTypeExpr && !as<TupleType>(operandType))
    {
        // This branch handles term-valued expressions whose checked type is a type pack rather than
        // a `ValuePackType` or `TupleType`, e.g. `expand Wrapper<each T>`. Value-pack queries are
        // handled above, and tuple-valued queries are handled separately.
        SLANG_ASSERT(isTypePack(operandType));
        packQueryExpr->type = QualType(applyPackQueryToType(operandType));
        return packQueryExpr;
    }

    Type* resultType = applyPackQueryToType(operandType);
    packQueryExpr->type = QualType(resultType);
    return packQueryExpr;
}

Expr* SemanticsExprVisitor::visitShapePackTransformExpr(ShapePackTransformExpr* shapePackExpr)
{
    for (auto& arg : shapePackExpr->args)
        arg = dispatch(arg);

    auto opName = getShapePackTransformName(shapePackExpr);
    auto diagnoseInvalidArguments = [&]()
    {
        getSink()->diagnose(Diagnostics::ShapePackArgumentIsInvalid{
            .opName = opName,
            .location = shapePackExpr->loc});
        shapePackExpr->type = m_astBuilder->getErrorType();
        return shapePackExpr;
    };

    auto tryFoldArg = [&](Index argIndex) -> Val*
    {
        return tryConstantFoldExpr(
            SubstExpr<Expr>(shapePackExpr->getArg(argIndex)),
            ConstantFoldingKind::CompileTime,
            nullptr);
    };

    if (auto shapeConcatExpr = as<ShapeConcatExpr>(shapePackExpr))
    {
        auto leftInfo = _getShapePackOperandInfo(shapeConcatExpr->getArg(0)->type.type);
        auto rightInfo = _getShapePackOperandInfo(shapeConcatExpr->getArg(1)->type.type);
        if (leftInfo.kind == ShapePackOperandKind::Invalid ||
            rightInfo.kind == ShapePackOperandKind::Invalid ||
            !_isExactIntType(shapeConcatExpr->getArg(2)->type.type))
        {
            return diagnoseInvalidArguments();
        }

        auto leftVal = tryFoldArg(0);
        auto rightVal = tryFoldArg(1);
        auto axisVal = as<IntVal>(tryFoldArg(2));
        if (!leftVal || !rightVal || !axisVal)
            return diagnoseInvalidArguments();

        Index leftRank = 0;
        Index rightRank = 0;
        auto hasLeftRank = _tryGetShapePackRank(leftInfo.type, leftVal, leftRank);
        auto hasRightRank = _tryGetShapePackRank(rightInfo.type, rightVal, rightRank);
        if (hasLeftRank && hasRightRank && leftRank != rightRank)
        {
            getSink()->diagnose(Diagnostics::ShapePackRankMismatch{
                .opName = opName,
                .leftRank = (int)leftRank,
                .rightRank = (int)rightRank,
                .location = shapeConcatExpr->loc});
            shapeConcatExpr->type = m_astBuilder->getErrorType();
            return shapeConcatExpr;
        }

        IntegerLiteralValue axisValue = 0;
        Index knownRank = 0;
        if (hasLeftRank)
            knownRank = leftRank;
        else if (hasRightRank)
            knownRank = rightRank;
        if ((hasLeftRank || hasRightRank) && tryGetConstantIntVal(axisVal, axisValue) &&
            (axisValue < 0 || axisValue >= knownRank))
        {
            getSink()->diagnose(Diagnostics::ShapePackAxisOutOfRange{
                .opName = opName,
                .axis = (int)axisValue,
                .rank = (int)knownRank,
                .location = shapeConcatExpr->loc});
            shapeConcatExpr->type = m_astBuilder->getErrorType();
            return shapeConcatExpr;
        }

        if ((hasLeftRank || hasRightRank) && knownRank == 0)
        {
            getSink()->diagnose(Diagnostics::ShapePackNoValidAxis{
                .opName = opName,
                .rank = 0,
                .location = shapeConcatExpr->loc});
            shapeConcatExpr->type = m_astBuilder->getErrorType();
            return shapeConcatExpr;
        }

        if (auto leftConcretePack = as<ConcreteIntValPack>(leftVal))
        {
            if (auto rightConcretePack = as<ConcreteIntValPack>(rightVal))
            {
                if (tryGetConstantIntVal(axisVal, axisValue) &&
                    leftConcretePack->getCount() == rightConcretePack->getCount() &&
                    axisValue >= 0 && axisValue < leftConcretePack->getCount())
                {
                    for (Index i = 0; i < leftConcretePack->getCount(); ++i)
                    {
                        if (i != axisValue && areProvablyDifferentShapeElements(
                                                  leftConcretePack->getElement(i),
                                                  rightConcretePack->getElement(i)))
                        {
                            getSink()->diagnose(Diagnostics::ShapeConcatNonAxisMismatch{
                                .axis = (int)axisValue,
                                .dimIndex = (int)i,
                                .location = shapeConcatExpr->loc});
                            shapeConcatExpr->type = m_astBuilder->getErrorType();
                            return shapeConcatExpr;
                        }
                    }
                }
                else if (!hasAnyPotentialConcatAxis(leftConcretePack, rightConcretePack))
                {
                    getSink()->diagnose(Diagnostics::ShapeConcatNoValidAxis{
                        .rank = (int)leftConcretePack->getCount(),
                        .location = shapeConcatExpr->loc});
                    shapeConcatExpr->type = m_astBuilder->getErrorType();
                    return shapeConcatExpr;
                }
            }
        }

        if (leftInfo.kind == ShapePackOperandKind::Tuple &&
            rightInfo.kind == ShapePackOperandKind::Tuple)
            shapeConcatExpr->type = QualType(_createIntTupleType(m_astBuilder, leftInfo.rank));
        else
            shapeConcatExpr->type =
                QualType(leftInfo.valuePackType ? leftInfo.valuePackType : rightInfo.valuePackType);
        return shapeConcatExpr;
    }

    if (auto shapePermuteExpr = as<ShapePermuteExpr>(shapePackExpr))
    {
        auto valueInfo = _getShapePackOperandInfo(shapePermuteExpr->getArg(0)->type.type);
        auto orderInfo = _getShapePackOperandInfo(shapePermuteExpr->getArg(1)->type.type);
        if (valueInfo.kind == ShapePackOperandKind::Invalid ||
            orderInfo.kind == ShapePackOperandKind::Invalid)
        {
            return diagnoseInvalidArguments();
        }

        auto valuePack = tryFoldArg(0);
        auto orderPack = tryFoldArg(1);
        if (!valuePack || !orderPack)
            return diagnoseInvalidArguments();

        Index valueRank = 0;
        Index orderRank = 0;
        auto hasValueRank = _tryGetShapePackRank(valueInfo.type, valuePack, valueRank);
        auto hasOrderRank = _tryGetShapePackRank(orderInfo.type, orderPack, orderRank);
        if (hasValueRank && hasOrderRank && valueRank != orderRank)
        {
            getSink()->diagnose(Diagnostics::ShapePermuteOrderLengthMismatch{
                .orderRank = (int)orderRank,
                .valueRank = (int)valueRank,
                .location = shapePermuteExpr->loc});
            shapePermuteExpr->type = m_astBuilder->getErrorType();
            return shapePermuteExpr;
        }

        if (auto concreteOrderPack = as<ConcreteIntValPack>(orderPack))
        {
            Index firstPosition = 0;
            Index secondPosition = 0;
            IntegerLiteralValue orderIndex = 0;
            bool hasConcreteIndex = false;
            if (tryFindProvableDuplicateOrderIndices(
                    concreteOrderPack,
                    firstPosition,
                    secondPosition,
                    orderIndex,
                    hasConcreteIndex))
            {
                if (hasConcreteIndex)
                {
                    getSink()->diagnose(Diagnostics::ShapePermuteDuplicateIndex{
                        .indexValue = (int)orderIndex,
                        .firstPosition = (int)firstPosition,
                        .secondPosition = (int)secondPosition,
                        .location = shapePermuteExpr->loc});
                }
                else
                {
                    getSink()->diagnose(Diagnostics::ShapePermuteDuplicateEquivalentIndex{
                        .firstPosition = (int)firstPosition,
                        .secondPosition = (int)secondPosition,
                        .location = shapePermuteExpr->loc});
                }
                shapePermuteExpr->type = m_astBuilder->getErrorType();
                return shapePermuteExpr;
            }

            for (Index i = 0; i < concreteOrderPack->getCount(); ++i)
            {
                orderIndex = 0;
                if (!tryGetConstantIntVal(concreteOrderPack->getElement(i), orderIndex))
                    continue;

                if (hasValueRank && (orderIndex < 0 || orderIndex >= valueRank))
                {
                    getSink()->diagnose(Diagnostics::ShapePermuteIndexOutOfRange{
                        .indexValue = (int)orderIndex,
                        .indexPosition = (int)i,
                        .rank = (int)valueRank,
                        .location = shapePermuteExpr->loc});
                    shapePermuteExpr->type = m_astBuilder->getErrorType();
                    return shapePermuteExpr;
                }
            }
        }

        if (valueInfo.kind == ShapePackOperandKind::Tuple)
            shapePermuteExpr->type = QualType(_createIntTupleType(m_astBuilder, valueInfo.rank));
        else
            shapePermuteExpr->type = QualType(valueInfo.valuePackType);
        return shapePermuteExpr;
    }

    if (auto shapeSwapExpr = as<ShapeSwapExpr>(shapePackExpr))
    {
        auto valueInfo = _getShapePackOperandInfo(shapeSwapExpr->getArg(0)->type.type);
        if (valueInfo.kind == ShapePackOperandKind::Invalid ||
            !_isExactIntType(shapeSwapExpr->getArg(1)->type.type) ||
            !_isExactIntType(shapeSwapExpr->getArg(2)->type.type))
        {
            return diagnoseInvalidArguments();
        }

        auto valuePack = tryFoldArg(0);
        auto dim0 = as<IntVal>(tryFoldArg(1));
        auto dim1 = as<IntVal>(tryFoldArg(2));
        if (!valuePack || !dim0 || !dim1)
            return diagnoseInvalidArguments();

        Index valueRank = 0;
        auto hasValueRank = _tryGetShapePackRank(valueInfo.type, valuePack, valueRank);
        IntegerLiteralValue dimValue = 0;
        if (hasValueRank && tryGetConstantIntVal(dim0, dimValue) &&
            (dimValue < 0 || dimValue >= valueRank))
        {
            getSink()->diagnose(Diagnostics::ShapePackAxisOutOfRange{
                .opName = opName,
                .axis = (int)dimValue,
                .rank = (int)valueRank,
                .location = shapeSwapExpr->loc});
            shapeSwapExpr->type = m_astBuilder->getErrorType();
            return shapeSwapExpr;
        }

        if (hasValueRank && tryGetConstantIntVal(dim1, dimValue) &&
            (dimValue < 0 || dimValue >= valueRank))
        {
            getSink()->diagnose(Diagnostics::ShapePackAxisOutOfRange{
                .opName = opName,
                .axis = (int)dimValue,
                .rank = (int)valueRank,
                .location = shapeSwapExpr->loc});
            shapeSwapExpr->type = m_astBuilder->getErrorType();
            return shapeSwapExpr;
        }

        if (hasValueRank && valueRank == 0)
        {
            getSink()->diagnose(Diagnostics::ShapePackNoValidAxis{
                .opName = opName,
                .rank = 0,
                .location = shapeSwapExpr->loc});
            shapeSwapExpr->type = m_astBuilder->getErrorType();
            return shapeSwapExpr;
        }

        if (valueInfo.kind == ShapePackOperandKind::Tuple)
            shapeSwapExpr->type = QualType(_createIntTupleType(m_astBuilder, valueInfo.rank));
        else
            shapeSwapExpr->type = QualType(valueInfo.valuePackType);
        return shapeSwapExpr;
    }

    auto shapeReduceExpr = as<ShapeReduceExpr>(shapePackExpr);
    SLANG_ASSERT(shapeReduceExpr);

    auto valueInfo = _getShapePackOperandInfo(shapeReduceExpr->getArg(0)->type.type);
    if (valueInfo.kind == ShapePackOperandKind::Invalid ||
        !_isExactIntType(shapeReduceExpr->getArg(1)->type.type))
    {
        return diagnoseInvalidArguments();
    }

    auto valuePack = tryFoldArg(0);
    auto axis = as<IntVal>(tryFoldArg(1));
    if (!valuePack || !axis)
        return diagnoseInvalidArguments();

    Index valueRank = 0;
    auto hasValueRank = _tryGetShapePackRank(valueInfo.type, valuePack, valueRank);
    IntegerLiteralValue axisValue = 0;
    if (hasValueRank && tryGetConstantIntVal(axis, axisValue) &&
        (axisValue < 0 || axisValue >= valueRank))
    {
        getSink()->diagnose(Diagnostics::ShapePackAxisOutOfRange{
            .opName = opName,
            .axis = (int)axisValue,
            .rank = (int)valueRank,
            .location = shapeReduceExpr->loc});
        shapeReduceExpr->type = m_astBuilder->getErrorType();
        return shapeReduceExpr;
    }

    if (hasValueRank && valueRank == 0)
    {
        getSink()->diagnose(Diagnostics::ShapePackNoValidAxis{
            .opName = opName,
            .rank = 0,
            .location = shapeReduceExpr->loc});
        shapeReduceExpr->type = m_astBuilder->getErrorType();
        return shapeReduceExpr;
    }

    if (valueInfo.kind == ShapePackOperandKind::Tuple)
        shapeReduceExpr->type = QualType(_createIntTupleType(m_astBuilder, valueInfo.rank));
    else
        shapeReduceExpr->type = QualType(valueInfo.valuePackType);
    return shapeReduceExpr;
}

Expr* SemanticsExprVisitor::visitFloatBitCastExpr(FloatBitCastExpr* expr)
{
    if (!expr->value)
    {
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    // Visit the value expression
    auto valueExpr = dispatch(expr->value);
    expr->value = valueExpr;

    // Determine the floating-point type from the expression
    auto valueType = valueExpr->type.type;
    BaseType floatBaseType = BaseType::Void;
    Type* resultType = nullptr;

    if (auto basicType = as<BasicExpressionType>(valueType))
    {
        switch (basicType->getBaseType())
        {
        case BaseType::Half:
            floatBaseType = BaseType::Half;
            resultType = m_astBuilder->getInt16Type();
            break;
        case BaseType::Float:
            floatBaseType = BaseType::Float;
            resultType = m_astBuilder->getIntType(); // int32
            break;
        case BaseType::Double:
            floatBaseType = BaseType::Double;
            resultType = m_astBuilder->getInt64Type();
            break;
        default:
            break;
        }
    }

    if (floatBaseType == BaseType::Void)
    {
        getSink()->diagnose(Diagnostics::FloatBitCastTypeMismatch{
            .intrinsic = "__floatAsInt",
            .expectedType = "half, float, or double",
            .expr = expr});
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    expr->type = resultType;

    // Try to constant fold - __floatAsInt must be evaluated at compile time
    FloatingPointLiteralExpr* floatLitExpr = nullptr;

    // Check if it's a direct floating-point literal
    if (auto lit = as<FloatingPointLiteralExpr>(valueExpr))
    {
        floatLitExpr = lit;
    }
    // Check if it's a type cast like float(x), half(x), double(x)
    else if (auto invokeExpr = as<InvokeExpr>(valueExpr))
    {
        if (invokeExpr->arguments.getCount() == 1)
        {
            floatLitExpr = as<FloatingPointLiteralExpr>(invokeExpr->arguments[0]);
            if (!floatLitExpr)
            {
                // Also check for integer literal being cast to float
                if (auto intLitExpr = as<IntegerLiteralExpr>(invokeExpr->arguments[0]))
                {
                    double floatVal = (double)intLitExpr->value;
                    IntegerLiteralValue resultValue = 0;

                    switch (floatBaseType)
                    {
                    case BaseType::Half:
                        resultValue = (int16_t)FloatToHalf((float)floatVal);
                        break;
                    case BaseType::Float:
                        resultValue = FloatAsInt((float)floatVal);
                        break;
                    case BaseType::Double:
                        resultValue = DoubleAsInt64(floatVal);
                        break;
                    default:
                        break;
                    }

                    // Create the folded integer literal expression
                    auto foldedExpr = m_astBuilder->create<IntegerLiteralExpr>();
                    foldedExpr->loc = expr->loc;
                    foldedExpr->type = QualType(resultType);
                    foldedExpr->value = resultValue;
                    return foldedExpr;
                }
            }
        }
    }

    if (floatLitExpr)
    {
        double floatVal = floatLitExpr->value;
        IntegerLiteralValue resultValue = 0;

        switch (floatBaseType)
        {
        case BaseType::Half:
            resultValue = (int16_t)FloatToHalf((float)floatVal);
            break;
        case BaseType::Float:
            resultValue = FloatAsInt((float)floatVal);
            break;
        case BaseType::Double:
            resultValue = DoubleAsInt64(floatVal);
            break;
        default:
            break;
        }

        // Create the folded integer literal expression
        auto foldedExpr = m_astBuilder->create<IntegerLiteralExpr>();
        foldedExpr->loc = expr->loc;
        foldedExpr->type = QualType(resultType);
        foldedExpr->value = resultValue;
        return foldedExpr;
    }

    // Could not constant fold - emit error
    getSink()->diagnose(Diagnostics::FloatBitCastRequiresConstant{.expr = expr});
    expr->type = m_astBuilder->getErrorType();
    return expr;
}

// Determines if we have a valid `AddressOf` target.
// Target to validate is `baseExpr`.
// Original type is `targetType`.
static PtrType* getValidTypeForAddressOf(
    SemanticsVisitor* visitor,
    ASTBuilder* m_astBuilder,
    Expr* baseExpr,
    Type* targetType)
{
    // Addressable syntax must still denote writable storage. In particular, a local captured by
    // value keeps its `VarExpr` shape until the enclosing expression is known to be evaluated, but
    // its prepared value category is already immutable.
    if (!baseExpr->type.isLeftValue)
        return nullptr;

    // If our base is a variable like expression, we should check if this expr is a
    // block of memory we allow getting the address of.
    if (auto declRefExpr = as<DeclRefExpr>(baseExpr))
    {
        visitor->ensureDecl(declRefExpr->declRef, DeclCheckState::DefinitionChecked);
        if (auto varDeclRef = as<VarDeclBase>(declRefExpr->declRef))
        {
            auto variableType = varDeclRef.substitute(m_astBuilder, targetType);
            auto varDecl = varDeclRef.getDecl();
            bool hasVulkanHitObjectAttributesAttribute = false;
            bool hasHLSLGroupSharedModifier = false;
            for (auto modifier : varDecl->modifiers)
            {
                if (as<VulkanHitObjectAttributesAttribute>(modifier))
                    hasVulkanHitObjectAttributesAttribute = true;
                else if (as<HLSLGroupSharedModifier>(modifier))
                    hasHLSLGroupSharedModifier = true;

                if (hasVulkanHitObjectAttributesAttribute || hasHLSLGroupSharedModifier)
                    break;
            }

            // Handle variables tagged as [__vulkanHitObjectAttributes].
            // This support is needed for an internal "hack" Slang uses
            // for raytracing with `__allocHitObjectAttributes`.
            if (hasVulkanHitObjectAttributesAttribute)
            {
                return m_astBuilder->getPtrType(
                    variableType,
                    AccessQualifier::ReadWrite,
                    AddressSpace::Generic,
                    m_astBuilder->getDefaultLayoutType());
            }
            // Handle 'groupshared' variables.
            else if (hasHLSLGroupSharedModifier)
            {
                return m_astBuilder->getPtrType(
                    variableType,
                    AccessQualifier::ReadWrite,
                    AddressSpace::GroupShared,
                    m_astBuilder->getDefaultLayoutType());
            }
            else
            {
                // UserPointer is correct here even though this covers all
                // remaining variables (including function-locals):
                // - On GPU targets, the IR validation pass
                //   (validateAndRemoveAssumeAddress) rejects function-local
                //   addresses before they reach codegen, so only device-memory
                //   variables survive, for which UserPointer is semantically
                //   right.
                // - On CPU/CUDA targets, address spaces are irrelevant (flat
                //   memory), and downstream passes that special-case
                //   UserPointer (addr-inst elimination, redundancy removal,
                //   etc.) handle it conservatively/correctly.
                return m_astBuilder->getPtrType(
                    variableType,
                    AccessQualifier::ReadWrite,
                    AddressSpace::UserPointer,
                    m_astBuilder->getDefaultLayoutType());
            }
        }
    }

    // If our base is a variable like expression, which comes from a deref-like operation,
    // we should check if we are able to return a pointer from that base.
    auto getPtrTypeFromBaseOfDerefLikeOperation = [&](Expr* baseExpr) -> PtrType*
    {
        auto declRefExpr = as<DeclRefExpr>(baseExpr);
        if (!declRefExpr)
            return nullptr;
        visitor->ensureDecl(declRefExpr->declRef, DeclCheckState::DefinitionChecked);
        auto varDeclRef = as<VarDeclBase>(declRefExpr->declRef);
        if (!varDeclRef)
            return nullptr;

        auto variableType = varDeclRef.substitute(m_astBuilder, targetType);

        auto ptrType = as<PtrType>(getType(m_astBuilder, varDeclRef));
        if (!ptrType)
            return nullptr;

        return m_astBuilder->getPtrType(
            variableType,
            ptrType->getAccessQualifier(),
            ptrType->getAddressSpace(),
            ptrType->getDataLayout());
    };

    // This logic handles the recursive lookup of "does our operation lead up
    // to an addressessable (can take the address-of) section of memory".
    if (auto indexExpr = as<IndexExpr>(baseExpr))
    {
        // If a user chooses to index into an array, we should check if the base
        // expression is something we can get the address-of.
        return getValidTypeForAddressOf(
            visitor,
            m_astBuilder,
            indexExpr->baseExpression,
            targetType);
    }
    else if (auto memberExpr = as<MemberExpr>(baseExpr))
    {
        // If a user chooses to get a member of a base, we should check if the base
        // is something we can get the address-of.
        if (as<VarDeclBase>(memberExpr->declRef))
            return getValidTypeForAddressOf(
                visitor,
                m_astBuilder,
                memberExpr->baseExpression,
                targetType);
    }
    else if (auto derefExpr = as<DerefExpr>(baseExpr))
    {
        // If a user deref's a variable-like-expression, we should
        // check if this is a base expression we can get the address-of.
        return getPtrTypeFromBaseOfDerefLikeOperation(derefExpr->base);
    }
    else if (auto invokeExpr = as<InvokeExpr>(baseExpr))
    {
        // A subscript such as `buf[i]` desugars to an `InvokeExpr` of the subscript's
        // `ref` accessor. We allow taking its address only for accessors whose intrinsic
        // op names an addressable location: a pointer's `GetOffsetPtr`, or a mutable
        // structured buffer's `RWStructuredBufferGetElementPtr`.
        auto functionMemberExpr = as<MemberExpr>(invokeExpr->functionExpr);
        if (!functionMemberExpr)
            return nullptr;
        auto subscriptDecl = as<SubscriptDecl>(functionMemberExpr->declRef.getDecl());
        if (!subscriptDecl)
            return nullptr;
        bool isOffsetIntrinsicOp = false;
        bool isStructuredBufferElementPtrOp = false;
        for (auto refAccessor : subscriptDecl->getMembersOfType<RefAccessorDecl>())
        {
            auto intrinsicOp = refAccessor->findModifier<IntrinsicOpModifier>();
            if (!intrinsicOp)
                continue;
            if (intrinsicOp->op == kIROp_GetOffsetPtr)
                isOffsetIntrinsicOp = true;
            else if (intrinsicOp->op == kIROp_RWStructuredBufferGetElementPtr)
                isStructuredBufferElementPtrOp = true;
        }

        // A subscript declares at most one of these two ref-accessor intrinsic ops: a
        // `Ptr<T>`-like subscript uses `GetOffsetPtr`, a mutable structured buffer's uses
        // `RWStructuredBufferGetElementPtr`, and those accessors live on different, unrelated
        // types. Assert they are mutually exclusive so the ordered checks below read as
        // exhaustive rather than priority-dependent — if both were somehow set, the
        // `GetOffsetPtr` branch would silently win and the structured-buffer branch (with its
        // release-assert) would never run.
        SLANG_ASSERT(!(isOffsetIntrinsicOp && isStructuredBufferElementPtrOp));

        // Address of a pointer element: `ptr[i]` where `ptr` is a `Ptr<T>`-like value. The
        // base is a pointer-typed variable, so `getPtrTypeFromBaseOfDerefLikeOperation`
        // recovers its pointer type; that helper does not apply to the buffer case below
        // because a structured buffer is not itself a `Ptr`-typed value.
        if (isOffsetIntrinsicOp)
            return getPtrTypeFromBaseOfDerefLikeOperation(functionMemberExpr->baseExpression);

        // Address of a mutable structured-buffer element: `buf[i]` where `buf` is an
        // `RWStructuredBuffer` or `RasterizerOrderedStructuredBuffer`. These two types are also
        // addressable via `&buf[i]` through `operator&`, and `__getAddress(buf[i])` must be
        // equivalent to `&buf[i]`, so it produces the same pointer type here. The AST pointer is
        // typed `UserPointer` (== `AddressSpace.Device`) to match how `&buf[i]` is typed at the
        // AST level; the SPIR-V address-space specialization pass later reconciles the surviving
        // slot to the element's real logical `StorageBuffer` space. The layout is
        // `DefaultDataLayout` to match
        // `&buf[i]` (the element offset is resolved from the buffer type's own layout at IR
        // generation, so this pointer's layout argument does not affect stride).
        if (isStructuredBufferElementPtrOp)
        {
            // The `kIROp_RWStructuredBufferGetElementPtr` ref accessor is generated by only
            // one place in the core module — the `kMutableStructuredBufferCases` template
            // (`hlsl.meta.slang`), which emits it for exactly `RWStructuredBuffer` and
            // `RasterizerOrderedStructuredBuffer`. Read-only `StructuredBuffer` has a
            // `get`-only subscript (no `ref`), so it never reaches here. The base type is
            // therefore always one of the two mutable structured-buffer types; assert that
            // invariant so a future accessor reusing this op surfaces rather than silently
            // producing a spurious E31160.
            auto baseType = unwrapModifiedType(functionMemberExpr->baseExpression->type.type);
            SLANG_RELEASE_ASSERT(
                as<HLSLRWStructuredBufferType>(baseType) ||
                as<HLSLRasterizerOrderedStructuredBufferType>(baseType));
            return m_astBuilder->getPtrType(
                targetType,
                AccessQualifier::ReadWrite,
                AddressSpace::UserPointer,
                m_astBuilder->getDefaultLayoutType());
        }

        return nullptr;
    }
    else if (auto swizzleExpr = as<SwizzleExpr>(baseExpr))
    {
        // Only allow swizzle of 1 element since otherwise
        // we may have a non-contiguous swizzle
        // (`val.xxy` is non contiguous).
        if (swizzleExpr->elementIndices.getCount() > 1)
            return nullptr;

        // Check if the base expression is something we can get the address-of.
        return getValidTypeForAddressOf(visitor, m_astBuilder, swizzleExpr->base, targetType);
    }
    return nullptr;
}

Expr* SemanticsExprVisitor::visitAddressOfExpr(AddressOfExpr* expr)
{
    expr->arg = CheckTerm(expr->arg);
    expr->arg = checkAbstractStorageUse(expr->arg, AbstractStorageUse::RequireAddress);

    // This address-of feature is purely experimental and for prototyping.
    // Only allow known expressions.
    expr->type =
        getValidTypeForAddressOf(this, m_astBuilder, expr->arg, getType(m_astBuilder, expr->arg));
    if (!expr->type)
    {
        getSink()->diagnose(Diagnostics::InvalidAddressOf{.location = expr->loc});
        expr->type = m_astBuilder->getErrorType();
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitBuiltinCastExpr(BuiltinCastExpr* expr)
{
    // All builtin cast exprs should already be checked.
    return expr;
}

Expr* SemanticsExprVisitor::visitTypeCastExpr(TypeCastExpr* expr)
{
    if (expr->type)
        return expr;

    // Check the term we are applying first
    auto funcExpr = expr->functionExpr;
    funcExpr = CheckTerm(funcExpr);

    // Now ensure that the term represents a (proper) type.
    TypeExp typeExp;
    typeExp.exp = funcExpr;
    typeExp = CheckProperType(typeExp);

    expr->functionExpr = typeExp.exp;
    expr->type.type = typeExp.type;

    // Next check the argument expression (there should be only one)
    for (auto& arg : expr->arguments)
    {
        arg = CheckTerm(arg);
    }

    if (auto declRefType = as<DeclRefType>(typeExp.type); declRefType && !isSlang202cOrLater(this))
    {
        // SLANG <=2026 LEGACY FEATURE:
        //
        // As a backwards-compatibility feature for HLSL, we will allow for a cast
        // to a `struct` type from a literal zero, with the semantics of default initialization.
        //
        // In Slang 2026, a warning is issued to encourage migrating away from
        // this feature.
        if (const auto structDeclRef = as<StructDecl>(declRefType->getDeclRef()))
        {
            if (expr->arguments.getCount() == 1)
            {
                auto arg = expr->arguments[0];
                if (auto intLitArg = as<IntegerLiteralExpr>(arg))
                {
                    if (getIntegerLiteralValue(intLitArg->token, getSink()) == 0)
                    {
                        // At this point we have confirmed that the cast
                        // has the right form, so we want to apply our special case.
                        //
                        // TODO: If/when we allow for user-defined initializer/constructor
                        // definitions we would have to be careful here because it is
                        // possible that the target type has defined an initializer/constructor
                        // that takes a single `int` parmaeter and means to call that instead.
                        //
                        // For now that should be a non-issue, and in a pinch such a user
                        // could use `T(0)` instead of `(T) 0` to get around this special
                        // HLSL legacy feature.

                        // We will type-check code like:
                        //
                        //      MyStruct s = (MyStruct) 0;
                        //
                        // the same as:
                        //
                        //      MyStruct s = {};
                        //
                        // That is, we construct an empty initializer list, and then coerce
                        // that initializer list expression to the desired type (letting
                        // the code for handling initializer lists work out all of the
                        // details of what is/isn't valid). This choice means we get
                        // to benefit from the existing codegen support for initializer
                        // lists, rather than needing the `(MyStruct) 0` idiom to be
                        // special-cased in later stages of the compiler.
                        //
                        // Note: we use an empty initializer list `{}` instead of an
                        // initializer list with a single zero `{0}`, which is semantically
                        // significant if the first field of `MyStruct` had its own
                        // default initializer defined as part of the `struct` definition.
                        // Basically we have chosen to interpret the "cast from zero" syntax
                        // as sugar for default initialization, and *not* specifically
                        // for zero-initialization. That choice could be revisited if
                        // users express displeasure. For now there isn't enough usage
                        // of explicit default initializers for `struct` fields to
                        // make this a major concern (since they aren't supported in HLSL).
                        //
                        InitializerListExpr* initListExpr =
                            m_astBuilder->create<InitializerListExpr>();
                        initListExpr->loc = expr->loc;
                        initListExpr->useCStyleInitialization = false;
                        auto checkedInitListExpr = visitInitializerListExpr(initListExpr);

                        // In Slang 2026 mode, warn that a cast from literal 0
                        // changes semantics for regular structs (i.e., anything
                        // that the user defines).
                        //
                        // We don't warn about casts from literal 0 to core
                        // module types (e.g., float, vector, etc). The default
                        // initializers of these types have the zeroing
                        // semantics.
                        if (isSlang2026OrLater(this) && !isFromCoreModule(structDeclRef.getDecl()))
                            getSink()->diagnose(
                                Diagnostics::DeprecatedStructCastFromZero{.expr = expr});

                        return coerceExpressionWithoutFinalization(
                            CoercionSite::General,
                            typeExp.type,
                            checkedInitListExpr,
                            getSink());
                    }
                }
            }
        }
    }


    // Now process this like any other explicit call (so casts
    // and constructor calls are semantically equivalent).
    //
    auto checkedExpr = CheckInvokeExprWithCheckedOperands(expr);

    if (m_parentDifferentiableAttr)
    {
        if (auto checkedInvokeExpr = as<InvokeExpr>(checkedExpr))
        {
            // Register types for final resolved invoke arguments again.
            for (auto& arg : checkedInvokeExpr->arguments)
            {
                maybeRegisterDifferentiableType(m_astBuilder, arg->type.type);
            }

            registerAssociatedMethodsForSelectedCallable(this, checkedInvokeExpr->functionExpr);
        }
    }

    return checkedExpr;
}

Expr* SemanticsExprVisitor::visitTryExpr(TryExpr* expr)
{
    auto prevTryClauseType = m_enclosingTryClauseType;
    m_enclosingTryClauseType = expr->tryClauseType;
    expr->base = CheckTerm(expr->base);
    m_enclosingTryClauseType = prevTryClauseType;
    expr->type = expr->base->type;
    if (as<ErrorType>(expr->type))
        return expr;

    auto parentFunc = this->m_parentFunc;
    auto base = as<InvokeExpr>(expr->base);
    if (!base)
    {
        getSink()->diagnose(Diagnostics::TryClauseMustApplyToInvokeExpr{.expr = expr});
        return expr;
    }

    auto callee = as<DeclRefExpr>(base->functionExpr);
    if (!callee)
    {
        getSink()->diagnose(Diagnostics::CalleeOfTryCallMustBeFunc{.expr = expr});
        return expr;
    }

    // A call that is not an `ErrorType` has a callee that overload resolution typed as a
    // `FuncType`, whatever kind of declaration or value it names (see
    // `CompleteOverloadCandidate`), with the error type substituted for this call site.
    // The `MustUseTryClauseToCallAThrowFunc` check in `CheckInvokeExprWithCheckedOperands`
    // reads the same `FuncType`.
    auto calleeFuncType = as<FuncType>(callee->type);
    SLANG_RELEASE_ASSERT(calleeFuncType);
    auto calleeDecl = callee->declRef.getDecl();
    auto calleeErrorType = calleeFuncType->getErrorType();
    if (calleeErrorType->equals(m_astBuilder->getBottomType()))
    {
        getSink()->diagnose(
            Diagnostics::TryInvokeCalleeShouldThrow{.callee = calleeDecl, .expr = expr});
        return expr;
    }
    Stmt* catchStmt = findMatchingCatchStmt(calleeErrorType);

    if (FindOuterStmt<DeferStmt>(catchStmt))
    {
        // 'try' may jump outside a defer statement, which isn't allowed for
        // now.
        getSink()->diagnose(Diagnostics::UncaughtTryInsideDefer{.expr = expr});
        return expr;
    }

    if (!catchStmt)
    {
        // Uncaught try.
        if (!parentFunc)
        {
            getSink()->diagnose(Diagnostics::UncaughtTryCallInNonThrowFunc{.expr = expr});
            return expr;
        }
        if (parentFunc->errorType->equals(m_astBuilder->getBottomType()))
        {
            getSink()->diagnose(Diagnostics::UncaughtTryCallInNonThrowFunc{.expr = expr});
            return expr;
        }
        if (!parentFunc->errorType->equals(calleeErrorType))
        {
            getSink()->diagnose(Diagnostics::ErrorTypeOfCalleeIncompatibleWithCaller{
                .calleeErrorType = calleeErrorType,
                .callee = calleeDecl,
                .callerErrorType = parentFunc->errorType,
                .expr = expr});
            return expr;
        }
    }
    return expr;
}

static bool _isTypeParametric(Type* type)
{
    if (!type)
        return false;
    if (as<ThisType>(type))
        return true;
    if (auto declRefType = as<DeclRefType>(type))
    {
        auto decl = declRefType->getDeclRef().getDecl();
        if (as<GenericTypeParamDeclBase>(decl) || as<AssocTypeDecl>(decl) ||
            as<ThisTypeDecl>(decl) || as<GlobalGenericParamDecl>(decl))
            return true;
        // Check generic arguments for parametric types.
        // E.g., Container<T> contains the generic param T.
        if (auto genericApp = as<GenericAppDeclRef>(declRefType->getDeclRefBase()))
        {
            for (Index i = 0; i < genericApp->getArgCount(); i++)
            {
                if (auto argType = as<Type>(genericApp->getArg(i)))
                {
                    if (_isTypeParametric(argType))
                        return true;
                }
            }
        }
    }
    return false;
}

Expr* SemanticsExprVisitor::visitIsTypeExpr(IsTypeExpr* expr)
{
    SemanticsVisitor unevaluatedVisitor(withPotentiallyUnevaluatedTerm());
    expr->typeExpr = unevaluatedVisitor.CheckProperType(expr->typeExpr);
    auto originalVal = unevaluatedVisitor.CheckTerm(expr->value);
    expr->type = m_astBuilder->getBoolType();
    expr->value = originalVal;

    auto valueType = expr->value->type.type;
    if (auto typeType = as<TypeType>(valueType))
        valueType = typeType->getType();
    auto unwrappedValueType = unwrapModifiedType(valueType);
    auto valueInterfaceType = isInterfaceType(unwrappedValueType) ? unwrappedValueType : valueType;

    // If value is a subtype of `type`, then this expr is always true.
    auto witness = isSubtype(valueType, expr->typeExpr.type, IsSubTypeOptions::None);
    auto declWitness = as<DeclaredSubtypeWitness>(witness);
    bool optionalWitness = declWitness && declWitness->isOptional();

    if (witness && !optionalWitness)
    {
        // Instead of returning a BoolLiteralExpr, we use a field to indicate this scenario,
        // so that the language server can still see the original syntax tree.
        expr->constantVal = m_astBuilder->create<BoolLiteralExpr>();
        expr->constantVal->type = m_astBuilder->getBoolType();
        expr->constantVal->value = true;
        expr->constantVal->loc = expr->loc;
        return expr;
    }

    // Check if the right-hand side type is an interface type. For 'is'
    // statements, that's only allowed if it's related to an optional
    // constraint.
    if (isInterfaceType(expr->typeExpr.type) && !optionalWitness)
    {
        getSink()->diagnose(Diagnostics::IsOperatorCannotUseInterfaceAsRhs{.expr = expr});
        return expr;
    }

    // Otherwise, if the target type is a subtype of value->type, we need to grab the
    // subtype witness for runtime checks.

    expr->value = maybeOpenExistential(originalVal);
    expr->witnessArg =
        witness ? witness : tryGetSubtypeWitness(expr->typeExpr.type, valueInterfaceType);
    if (expr->witnessArg)
    {
        // For now we can only support the scenario where `expr->value` is an interface type.
        if (!optionalWitness && !isInterfaceType(valueInterfaceType))
        {
            getSink()->diagnose(Diagnostics::IsOperatorValueMustBeInterfaceType{.expr = expr});
        }
        if (_doesIsTypeExprEvaluateValue(expr))
        {
            expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
                m_astBuilder,
                expr->value->abstractStorageAccess,
                AbstractStorageUse::ReadValue);
        }
        return expr;
    }

    // If we reach here with no witness and both the value type and target type are concrete
    // (not generic type parameters, associated types, or ThisType), the `is` check is always
    // statically false and the user is using `is` incorrectly on unrelated concrete types.
    // Note: _isTypeParametric only handles DeclRefType-based types (incl. recursive generic args).
    // Builtin composite types like arrays or vectors with embedded generic params are not checked,
    // but these are unlikely to appear in `is`/`as` expressions in practice.
    if (!as<ErrorType>(valueInterfaceType) && !as<ErrorType>(expr->typeExpr.type) &&
        !isInterfaceType(valueInterfaceType) && !_isTypeParametric(valueInterfaceType) &&
        !_isTypeParametric(expr->typeExpr.type))
    {
        getSink()->diagnose(Diagnostics::IsAsOnUnrelatedConcreteTypes{.expr = expr});
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitAsTypeExpr(AsTypeExpr* expr)
{
    TypeExp typeExpr;
    typeExpr.exp = expr->typeExpr;
    typeExpr = CheckProperType(typeExpr);

    // Check if the right-hand side type is an interface type
    if (isInterfaceType(typeExpr.type))
    {
        getSink()->diagnose(Diagnostics::AsOperatorCannotUseInterfaceAsRhs{.expr = expr});
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    expr->value = CheckTerm(expr->value);
    auto valueType = expr->value->type.type;
    auto unwrappedValueType = unwrapModifiedType(valueType);
    auto valueInterfaceType = isInterfaceType(unwrappedValueType) ? unwrappedValueType : valueType;

    // Reject `expr as OpaqueType` (and structs containing opaque fields) because
    // Optional<T> cannot wrap resource/opaque types.
    if (typeTransitivelyContainsOpaqueHandle(this, typeExpr.type))
    {
        getSink()->diagnose(
            Diagnostics::OptionalCannotWrapResourceType{.type = typeExpr.type, .expr = expr});
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    auto optType = m_astBuilder->getOptionalType(typeExpr.type);
    expr->type = optType;

    // If value is a subtype of `type`, then this expr is equivalent to a CastToSuperTypeExpr.
    if (auto witness = tryGetSubtypeWitness(valueType, typeExpr.type))
    {
        auto castToSuperType = createCastToSuperTypeExpr(typeExpr.type, expr->value, witness);
        auto makeOptional = m_astBuilder->create<MakeOptionalExpr>();
        makeOptional->loc = expr->loc;
        makeOptional->type = optType;
        makeOptional->value = castToSuperType;
        makeOptional->typeExpr = typeExpr.exp;
        makeOptional->checked = true;
        makeOptional->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            castToSuperType->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
        return makeOptional;
    }

    // If target type is an interface type, we will obtain the witness here for
    // runtime casting.
    expr->witnessArg = tryGetSubtypeWitness(typeExpr.type, valueInterfaceType);
    if (expr->witnessArg)
    {
        // For now we can only support the scenario where `expr->value` is an interface type.
        if (!isInterfaceType(valueInterfaceType))
        {
            getSink()->diagnose(Diagnostics::IsOperatorValueMustBeInterfaceType{.expr = expr});
        }
        expr->value = maybeOpenExistential(expr->value);
        expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            expr->value->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
        return expr;
    }

    // If we reach here with no witness and both the value type and target type are concrete
    // (not generic type parameters, associated types, or ThisType), the `as` cast always
    // fails and the user is using `as` incorrectly on unrelated concrete types.
    if (!as<ErrorType>(valueInterfaceType) && !as<ErrorType>(typeExpr.type) &&
        !isInterfaceType(valueInterfaceType) && !_isTypeParametric(valueInterfaceType) &&
        !_isTypeParametric(typeExpr.type))
    {
        getSink()->diagnose(Diagnostics::IsAsOnUnrelatedConcreteTypes{.expr = expr});
    }

    expr->typeExpr = typeExpr.exp;
    expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
        m_astBuilder,
        expr->value->abstractStorageAccess,
        AbstractStorageUse::ReadValue);
    return expr;
}


Expr* SemanticsExprVisitor::visitExpandExpr(ExpandExpr* expr)
{
    OrderedHashSet<Val*> capturedPackSet;
    auto subContext = this->withParentExpandExpr(expr, &capturedPackSet);
    expr->baseExpr = dispatchExpr(expr->baseExpr, subContext);

    Type* patternType = nullptr;
    bool isTypeExpr = false;
    if (auto typeType = as<TypeType>(expr->baseExpr->type))
    {
        patternType = typeType->getType();
        isTypeExpr = true;
    }
    else
    {
        patternType = expr->baseExpr->type;
    }
    if (as<ErrorType>(patternType))
    {
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }
    if (subContext.getCapturedPacks()->getCount() == 0)
    {
        getSink()->diagnose(Diagnostics::ExpandTermCapturesNoTypePacks{.expr = expr});
    }
    List<Val*> capturedPacks;
    for (auto capturedVal : capturedPackSet)
    {
        capturedPacks.add(capturedVal);
    }
    auto expandType = m_astBuilder->getExpandType(patternType, capturedPacks.getArrayView());
    if (isTypeExpr)
        expr->type = m_astBuilder->getTypeType(expandType);
    else
    {
        expr->type = QualType(expandType);
        expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
            m_astBuilder,
            expr->baseExpr->abstractStorageAccess,
            AbstractStorageUse::ReadValue);
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitEachExpr(EachExpr* expr)
{
    if (!m_parentExpandExpr)
    {
        getSink()->diagnose(Diagnostics::EachExprMustBeInsideExpandExpr{.expr = expr});
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    expr->baseExpr = CheckTerm(expr->baseExpr);
    bool isTypeNode = false;
    Type* baseType = nullptr;
    if (auto typeType = as<TypeType>(expr->baseExpr->type))
    {
        isTypeNode = true;
        baseType = typeType->getType();
    }
    else
    {
        baseType = expr->baseExpr->type;
    }
    if (as<ErrorType>(baseType))
    {
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    // Check if this is a value pack parameter reference (e.g. `each D` where D is
    // `let each D : int`).
    if (!isTypeNode)
    {
        if (auto valPackType = as<ValuePackType>(baseType))
        {
            SLANG_ASSERT(m_capturedPacks);
            if (auto declRefExpr = as<DeclRefExpr>(expr->baseExpr))
            {
                auto declRef = getDeclRef(m_astBuilder, declRefExpr);
                m_capturedPacks->add(
                    m_astBuilder->getOrCreate<DeclRefIntVal>(valPackType, declRef));
            }
            expr->type = QualType(valPackType->getElementType());
            expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
                m_astBuilder,
                expr->baseExpr->abstractStorageAccess,
                AbstractStorageUse::ReadValue);
            return expr;
        }
    }

    if (isTypeNode)
    {
        if (!isTypePack(baseType) && !as<TupleType>(baseType))
        {
            goto error;
        }
    }
    else
    {
        if (!isTypePack(baseType) && !as<TupleType>(baseType))
            goto error;
    }

    if (auto tupleType = as<TupleType>(baseType))
        baseType = tupleType->getTypePack();

    {
        SLANG_ASSERT(m_capturedPacks);
        if (auto baseExpandType = as<ExpandType>(baseType))
        {
            for (Index i = 0; i < baseExpandType->getCapturedPackCount(); i++)
            {
                auto capturedPack = baseExpandType->getCapturedPack(i);
                m_capturedPacks->add(capturedPack);
            }
        }
        else
        {
            m_capturedPacks->add(baseType);
        }
        auto eachType = m_astBuilder->getEachType(baseType);
        if (isTypeNode)
            expr->type = m_astBuilder->getTypeType(eachType);
        else
        {
            expr->type = QualType(eachType);
            expr->abstractStorageAccess = AbstractStorageAccessPath::makeFixedOperandUse(
                m_astBuilder,
                expr->baseExpr->abstractStorageAccess,
                AbstractStorageUse::ReadValue);
        }
        return expr;
    }
error:;
    expr->type = m_astBuilder->getErrorType();
    if (!as<ErrorType>(baseType))
    {
        getSink()->diagnose(Diagnostics::ExpectTypePackAfterEach{.expr = expr});
    }
    return expr;
}

Expr* SemanticsExprVisitor::visitLambdaExpr(LambdaExpr* lambdaExpr)
{
    ASTSynthesizer synthesizer = ASTSynthesizer(m_astBuilder, getNamePool());
    synthesizer.pushContainerScope(m_outerScope->containerDecl);

    LambdaCaptureContext captureContext;
    ensureAllDeclsRec(lambdaExpr->paramScopeDecl, DeclCheckState::DefinitionChecked);
    LambdaDecl* lambdaStructDecl = m_astBuilder->create<LambdaDecl>();
    addModifier(lambdaStructDecl, m_astBuilder->create<SynthesizedModifier>());
    synthesizer.pushScopeForContainer(lambdaStructDecl);
    lambdaStructDecl->loc = lambdaExpr->loc;
    StringBuilder nameBuilder;
    nameBuilder << "_slang_Lambda_";
    if (m_parentFunc)
    {
        nameBuilder << getText(m_parentFunc->getName());
        nameBuilder << "_";
        nameBuilder << m_parentFunc->getDirectMemberDeclCount();
        m_parentFunc->addMember(lambdaStructDecl);
    }
    else
    {
        m_outerScope->containerDecl->addMember(lambdaStructDecl);
    }
    auto name = getName(nameBuilder.getBuffer());
    lambdaStructDecl->nameAndLoc.name = name;
    lambdaStructDecl->nameAndLoc.loc = lambdaExpr->loc;

    auto funcDecl = m_astBuilder->create<FuncDecl>();
    synthesizer.pushScopeForContainer(funcDecl);
    funcDecl->loc = lambdaExpr->loc;
    funcDecl->nameAndLoc.name = getName("()");
    funcDecl->nameAndLoc.loc = lambdaExpr->loc;
    lambdaStructDecl->addMember(funcDecl);
    lambdaStructDecl->funcDecl = funcDecl;
    addModifier(funcDecl, m_astBuilder->create<SynthesizedModifier>());
    auto subContext = withParentLambdaExpr(lambdaExpr, lambdaStructDecl, &captureContext)
                          .withParentFunc(funcDecl)
                          .allowDroppingDerivatives();

    // Checking the body both infers the result type and prepares provisional captures. This is the
    // definitive body check for the synthesized call operator; declaration checking must not
    // revisit the same statements after the inferred signature has been installed.
    dispatchStmt(lambdaExpr->bodyStmt, subContext);

    // A lambda with no return expression has a `void` result.
    if (!funcDecl->returnType.type)
        funcDecl->returnType.type = m_astBuilder->getVoidType();

    synthesizer.popScope();
    synthesizer.popScope();

    funcDecl->body = lambdaExpr->bodyStmt;
    for (auto param : lambdaExpr->paramScopeDecl->getDirectMemberDecls())
    {
        funcDecl->addMember(param);
    }

    // LambdaDecl should inherit from `IFunc<>`.
    if (funcDecl->returnType.type)
    {
        auto genApp = m_astBuilder->create<GenericAppExpr>();
        genApp->functionExpr = synthesizer.emitVarExpr(getName("IFunc"));
        auto returnTypeExp = synthesizer.emitStaticTypeExpr(funcDecl->returnType.type);
        genApp->arguments.add(returnTypeExp);
        for (auto param : getMembersOfType<ParamDecl>(m_astBuilder, lambdaExpr->paramScopeDecl))
        {
            auto paramType = getParamTypeWithModeWrapper(m_astBuilder, param);
            auto paramTypeExp = synthesizer.emitStaticTypeExpr(paramType);
            genApp->arguments.add(paramTypeExp);
        }
        auto inheritanceDecl = m_astBuilder->create<InheritanceDecl>();
        inheritanceDecl->base.exp = genApp;
        lambdaStructDecl->addMember(inheritanceDecl);
    }

    // The manual body check above precedes the declaration phases that need its inferred result
    // type. `ensureDecl()` advances the synthesized function through those phases, then
    // `setCheckState()` records that its definition has already been checked. Later recursive
    // declaration checking can proceed to capability analysis without checking the body a second
    // time.
    ensureDecl(funcDecl, DeclCheckState::AttributesChecked, &subContext);
    funcDecl->setCheckState(DeclCheckState::DefinitionChecked);

    // The synthesized closure now has enough type information to check its constructor signature
    // and `IFunc` witness.
    ensureDecl(lambdaStructDecl, DeclCheckState::AttributesChecked);

    // Return an expr that represents `SynthesizedLambdaStruct.__init(captured_args...)`.
    List<Expr*> args;
    for (auto capturedField : getMembersOfType<VarDecl>(m_astBuilder, lambdaStructDecl))
    {
        Decl* capturedDecl = nullptr;
        SLANG_RELEASE_ASSERT(
            captureContext.fieldToCapturedDecl.tryGetValue(capturedField.getDecl(), capturedDecl));
        if (auto capturedVarDecl = as<VarDeclBase>(capturedDecl))
        {
            args.add(synthesizer.emitVarExpr(capturedVarDecl));
        }
        else
        {
            args.add(synthesizer.emitThisExpr());
        }
    }
    auto resultLambdaObj = synthesizer.emitCtorInvokeExpr(
        synthesizer.emitStaticTypeExpr(DeclRefType::create(m_astBuilder, lambdaStructDecl)),
        _Move(args));
    resultLambdaObj->loc = lambdaExpr->loc;
    auto checkedResultExpr = dispatchExpr(resultLambdaObj, *this);
    return checkedResultExpr;
}

void SemanticsExprVisitor::maybeCheckKnownBuiltinInvocation(Expr* invokeExpr)
{
    auto checkedInvokeExpr = as<InvokeExpr>(invokeExpr);
    if (!checkedInvokeExpr)
        return;
    auto declRefFuncExpr = as<DeclRefExpr>(checkedInvokeExpr->functionExpr);
    if (!declRefFuncExpr)
        return;
    auto callee = declRefFuncExpr->declRef.getDecl();
    if (!callee)
        return;
    auto knownBuiltinAttr = callee->findModifier<KnownBuiltinAttribute>();
    if (!knownBuiltinAttr)
        return;
    if (auto constantIntVal = as<ConstantIntVal>(knownBuiltinAttr->name))
    {
        if (constantIntVal->getValue() == (int)KnownBuiltinDeclName::GetAttributeAtVertex)
        {
            if (checkedInvokeExpr->arguments.getCount() != 2)
                return;
            auto vertexAttributeArg = checkedInvokeExpr->arguments[0];
            auto vertexAttributeArgDeclRefExpr = as<DeclRefExpr>(vertexAttributeArg);
            if (!vertexAttributeArgDeclRefExpr)
            {
                getSink()->diagnose(Diagnostics::GetAttributeAtVertexMustReferToPerVertexInput{
                    .location = invokeExpr->loc});
                return;
            }
            auto vertexAttributeArgDecl = vertexAttributeArgDeclRefExpr->declRef.getDecl();
            if (!vertexAttributeArgDecl)
                return;
            if (!vertexAttributeArgDecl->findModifier<PerVertexModifier>() &&
                !vertexAttributeArgDecl->findModifier<HLSLNoInterpolationModifier>())
            {
                getSink()->diagnose(Diagnostics::GetAttributeAtVertexMustReferToPerVertexInput{
                    .location = vertexAttributeArgDeclRefExpr->loc});
                return;
            }
        }
    }
}

Expr* SemanticsVisitor::maybeDereference(Expr* inExpr, CheckBaseContext checkBaseContext)
{
    Expr* expr = inExpr;
    for (;;)
    {
        auto baseType = expr->type;
        if (as<PtrType>(baseType))
        {
            if (checkBaseContext == CheckBaseContext::Subscript)
                return expr;
        }
        auto elementType = getPointedToTypeIfCanImplicitDeref(baseType);
        if (!elementType)
            return expr;
        expr = constructDerefExpr(expr, elementType, inExpr->loc);
    }
}

Expr* SemanticsVisitor::CheckMatrixSwizzleExpr(
    MemberExpr* memberRefExpr,
    Type* baseElementType,
    IntegerLiteralValue baseElementRowCount,
    IntegerLiteralValue baseElementColCount)
{
    // We can have up to 4 swizzles of two elements each
    MatrixCoord elementCoords[4];
    int elementCount = 0;

    bool anyDuplicates = false;
    int zeroIndexOffset = -1;

    String swizzleText = getText(memberRefExpr->name);
    auto cursor = swizzleText.begin();

    // The contents of the string are 0-terminated
    // Every update to cursor corresponds to a check against 0-termination
    while (*cursor)
    {
        // Throw out swizzling with more than 4 output elements
        if (elementCount >= 4)
        {
            return nullptr;
        }
        MatrixCoord elementCoord = {0, 0};

        // Check for the preceding underscore
        if (*cursor++ != '_')
        {
            return nullptr;
        }

        // Check for one or zero indexing
        if (*cursor == 'm')
        {
            // Can't mix one and zero indexing
            if (zeroIndexOffset == 1)
            {
                return nullptr;
            }
            zeroIndexOffset = 0;
            // Increment the index since we saw 'm'
            cursor++;
        }
        else
        {
            // Can't mix one and zero indexing
            if (zeroIndexOffset == 0)
            {
                return nullptr;
            }
            zeroIndexOffset = 1;
        }

        // Check for the ij components
        for (Index j = 0; j < 2; j++)
        {
            auto ch = *cursor++;

            if (ch < '0' || ch > '4')
            {
                return nullptr;
            }
            const int subIndex = ch - '0' - zeroIndexOffset;

            // Check the limit for either the row or column, depending on the step
            IntegerLiteralValue elementLimit;
            if (j == 0)
            {
                elementLimit = baseElementRowCount;
                elementCoord.row = subIndex;
            }
            else
            {
                elementLimit = baseElementColCount;
                elementCoord.col = subIndex;
            }
            // Make sure the index is in range for the source type
            // Account for off-by-one and reject 0 if oneIndexed
            if (subIndex >= elementLimit || subIndex < 0)
            {
                return nullptr;
            }
        }
        // Check if we've seen this index before
        for (int ee = 0; ee < elementCount; ee++)
        {
            if (elementCoords[ee] == elementCoord)
                anyDuplicates = true;
        }

        // add to our list...
        elementCoords[elementCount] = elementCoord;
        elementCount++;
    }

    MatrixSwizzleExpr* swizExpr = m_astBuilder->create<MatrixSwizzleExpr>();
    swizExpr->loc = memberRefExpr->loc;
    swizExpr->base = memberRefExpr->baseExpression;
    swizExpr->memberOpLoc = memberRefExpr->memberOperatorLoc;
    swizExpr->checked = true;

    // Store our list in the actual AST node
    for (int ee = 0; ee < elementCount; ++ee)
    {
        swizExpr->elementCoords[ee] = elementCoords[ee];
    }
    swizExpr->elementCount = elementCount;

    if (elementCount == 1)
    {
        // single-component swizzle produces a scalar
        //
        // Note(tfoley): the official HLSL rules seem to be that it produces
        // a one-component vector, which is then implicitly convertible to
        // a scalar, but that seems like it just adds complexity.
        swizExpr->type = QualType(baseElementType);
    }
    else
    {
        // TODO(tfoley): would be nice to "re-sugar" type
        // here if the input type had a sugared name...
        swizExpr->type = QualType(createVectorType(
            baseElementType,
            m_astBuilder->getIntVal(m_astBuilder->getIntType(), elementCount)));
    }

    // A swizzle names writable storage only when its base does and no component is repeated.
    swizExpr->type.isLeftValue = memberRefExpr->baseExpression->type.isLeftValue;
    if (anyDuplicates)
        swizExpr->type.isLeftValue = false;
    swizExpr->abstractStorageAccess = AbstractStorageAccessPath::makeProjection(
        m_astBuilder,
        swizExpr->base->abstractStorageAccess,
        AbstractStorageProjectionMode::ReadModifyWrite);

    return swizExpr;
}

Expr* SemanticsVisitor::CheckMatrixSwizzleExpr(
    MemberExpr* memberRefExpr,
    Type* baseElementType,
    IntVal* baseRowCount,
    IntVal* baseColCount)
{
    if (auto constantRowCount = as<ConstantIntVal>(baseRowCount))
    {
        if (auto constantColCount = as<ConstantIntVal>(baseColCount))
        {
            return CheckMatrixSwizzleExpr(
                memberRefExpr,
                baseElementType,
                constantRowCount->getValue(),
                constantColCount->getValue());
        }
    }
    return nullptr;
}

Expr* SemanticsVisitor::checkTupleSwizzleExpr(MemberExpr* memberExpr, TupleType* baseTupleType)
{
    UInt tupleElementCount = (UInt)baseTupleType->getMemberCount();
    if (tupleElementCount == 0)
        return checkGeneralMemberLookupExpr(memberExpr, baseTupleType);

    String swizzleText = getText(memberExpr->name);
    auto span = swizzleText.getUnownedSlice();
    Index pos = 0;

    ShortList<uint32_t> elementCoords;

    bool anyDuplicates = false;

    // The contents of the string are 0-terminated
    // Every update to cursor corresponds to a check against 0-termination
    while (pos < span.getLength())
    {
        uint32_t elementCoord;

        // Check for the preceding underscore
        if (span[pos] != '_')
        {
            return checkGeneralMemberLookupExpr(memberExpr, baseTupleType);
        }
        pos++;

        // Parse index.
        if (pos >= span.getLength())
        {
            // Unexpected end of swizzle string, fallback to
            // member lookup.
            return checkGeneralMemberLookupExpr(memberExpr, baseTupleType);
        }

        auto ch = span[pos];

        if (!CharUtil::isDigit(ch))
        {
            // An invalid character in the swizzle is an error, fallback to
            // member lookup.
            return checkGeneralMemberLookupExpr(memberExpr, baseTupleType);
        }
        elementCoord = (uint32_t)StringUtil::parseIntAndAdvancePos(span, pos);

        if (elementCoord >= tupleElementCount)
        {
            getSink()->diagnose(Diagnostics::InvalidSwizzleExpr{
                .pattern = swizzleText,
                .type = baseTupleType,
                .expr = memberExpr});
            return CreateErrorExpr(memberExpr);
        }

        // Check if we've seen this index before
        for (int ee = 0; ee < elementCoords.getCount(); ee++)
        {
            if (elementCoords[ee] == elementCoord)
                anyDuplicates = true;
        }

        // add to our list...
        elementCoords.add(elementCoord);
    }

    SwizzleExpr* swizExpr = m_astBuilder->create<SwizzleExpr>();
    swizExpr->loc = memberExpr->loc;
    swizExpr->base = memberExpr->baseExpression;
    swizExpr->elementIndices = _Move(elementCoords);
    swizExpr->memberOpLoc = memberExpr->memberOperatorLoc;

    if (swizExpr->elementIndices.getCount() == 1)
    {
        // single-component swizzle produces a scalar
        //
        swizExpr->type = QualType(baseTupleType->getMember(swizExpr->elementIndices[0]));
    }
    else
    {
        List<Type*> types;
        for (auto index : swizExpr->elementIndices)
        {
            types.add(baseTupleType->getMember(index));
        }
        swizExpr->type = QualType(m_astBuilder->getTupleType(types.getArrayView()));
    }

    // A swizzle names writable storage only when its base does and no element is repeated.
    swizExpr->type.isLeftValue = memberExpr->baseExpression->type.isLeftValue;
    if (anyDuplicates)
        swizExpr->type.isLeftValue = false;
    auto projectionMode = swizExpr->elementIndices.getCount() == 1
                              ? AbstractStorageProjectionMode::RequireAddressForWrite
                              : AbstractStorageProjectionMode::ReadModifyWrite;
    swizExpr->abstractStorageAccess = AbstractStorageAccessPath::makeProjection(
        m_astBuilder,
        swizExpr->base->abstractStorageAccess,
        projectionMode);
    return swizExpr;
}

Expr* SemanticsVisitor::CheckSwizzleExpr(
    MemberExpr* memberRefExpr,
    Type* baseElementType,
    IntegerLiteralValue baseElementCount)
{
    IntegerLiteralValue limitElement = baseElementCount;

    ShortList<uint32_t, 4> elementIndices;

    bool anyDuplicates = false;
    bool anyError = false;

    auto swizzleText = getText(memberRefExpr->name);

    for (Index i = 0; i < swizzleText.getLength(); i++)
    {
        auto ch = swizzleText[i];
        int elementIndex = -1;
        switch (ch)
        {
        case 'x':
        case 'r':
            elementIndex = 0;
            break;
        case 'y':
        case 'g':
            elementIndex = 1;
            break;
        case 'z':
        case 'b':
            elementIndex = 2;
            break;
        case 'w':
        case 'a':
            elementIndex = 3;
            break;
        default:
            // An invalid character in the swizzle is an error
            anyError = true;
            break;
        }

        // TODO(tfoley): GLSL requires that all component names
        // come from the same "family"...

        // Make sure the index is in range for the source type
        if (elementIndex >= limitElement)
        {
            anyError = true;
            break;
        }

        // If elementCount is already at 4 stop trying to assign a swizzle element and send an
        // error, we cannot have more valid swizzle elements than 4.
        if (elementIndices.getCount() >= 4)
        {
            anyError = true;
            break;
        }

        // Check if we've seen this index before
        for (int ee = 0; ee < elementIndices.getCount(); ee++)
        {
            if (elementIndices[ee] == (UInt)elementIndex)
                anyDuplicates = true;
        }

        // add to our list...
        elementIndices.add(elementIndex);
    }

    if (anyError)
    {
        return nullptr;
    }

    SwizzleExpr* swizExpr = m_astBuilder->create<SwizzleExpr>();
    swizExpr->loc = memberRefExpr->loc;
    swizExpr->base = memberRefExpr->baseExpression;
    swizExpr->memberOpLoc = memberRefExpr->memberOperatorLoc;
    swizExpr->elementIndices = _Move(elementIndices);

    if (swizExpr->elementIndices.getCount() == 1)
    {
        // single-component swizzle produces a scalar
        //
        // Note(tfoley): the official HLSL rules seem to be that it produces
        // a one-component vector, which is then implicitly convertible to
        // a scalar, but that seems like it just adds complexity.
        swizExpr->type = QualType(baseElementType);
    }
    else
    {
        // TODO(tfoley): would be nice to "re-sugar" type
        // here if the input type had a sugared name...
        swizExpr->type = QualType(createVectorType(
            baseElementType,
            m_astBuilder->getIntVal(
                m_astBuilder->getIntType(),
                swizExpr->elementIndices.getCount())));
    }

    // A swizzle can be used as an l-value as long as there
    // were no duplicates in the list of components
    swizExpr->type.isLeftValue = !anyDuplicates;
    if (swizExpr->type.isLeftValue)
        swizExpr->type.isLeftValue = swizExpr->base != nullptr;
    if (swizExpr->type.isLeftValue)
        swizExpr->type.isLeftValue = swizExpr->base->type != nullptr;
    if (swizExpr->type.isLeftValue)
        swizExpr->type.isLeftValue = swizExpr->base->type.isLeftValue;
    swizExpr->abstractStorageAccess = AbstractStorageAccessPath::makeProjection(
        m_astBuilder,
        swizExpr->base->abstractStorageAccess,
        AbstractStorageProjectionMode::ReadModifyWrite);

    return swizExpr;
}

Expr* SemanticsVisitor::CheckSwizzleExpr(
    MemberExpr* memberRefExpr,
    Type* baseElementType,
    IntVal* baseElementCount)
{
    if (auto constantElementCount = as<ConstantIntVal>(baseElementCount))
    {
        return CheckSwizzleExpr(memberRefExpr, baseElementType, constantElementCount->getValue());
    }
    else
    {
        return nullptr;
    }
}

Expr* SemanticsVisitor::_lookupStaticMember(DeclRefExpr* expr, Expr* baseExpression)
{
    LookupResult globalLookupResult;
    bool hasErrors = false;
    Expr* base = nullptr;

    // Keep track of namespace scopes we've already looked up in to avoid producing
    // duplicates.
    HashSet<ContainerDecl*> processedNamespaceScopes;

    auto handleLeafCase = [&](DeclRef<Decl> baseDeclRef, Type* type)
    {
        auto aggTypeDeclRef = as<AggTypeDeclBase>(baseDeclRef);

        if (auto namespaceDeclRef = as<NamespaceDeclBase>(baseDeclRef))
        {
            // We are looking up a namespace member.
            //
            // We should lookup in all sibling scopes of the namespace.
            // Another detail here is that we need to skip scopes that are transitively
            // imported. For example, given:
            // ```
            //     module a;
            //     namespace ns { int f_a(); }
            //
            //     module b;
            //     namespace ns { int f_b(); } // will have a sibling scope that refers to
            //     a::ns.
            //
            //     module c;
            //     import b;
            //     void test() {ns.f_a(); // should not be valid, because c does not import a. }
            // ```
            // Note that this logic doesn't work nicely with __exported import, but we should
            // consider deprecate this feature anyway.
            //
            auto namespaceModule = getModuleDecl(namespaceDeclRef.getDecl());
            auto thisModule =
                m_outerScope ? getModuleDecl(m_outerScope->containerDecl) : namespaceModule;

            for (auto scope = namespaceDeclRef.getDecl()->ownedScope; scope;
                 scope = scope->nextSibling)
            {
                auto namespaceDecl = as<NamespaceDeclBase>(scope->containerDecl);
                if (!namespaceDecl)
                    continue;
                if (thisModule != namespaceModule &&
                    namespaceModule != getModuleDecl(namespaceDecl))
                    continue;
                if (processedNamespaceScopes.add(scope->containerDecl))
                {
                    LookupResult nsLookupResult = lookUpDirectAndTransparentMembers(
                        m_astBuilder,
                        this,
                        expr->name,
                        namespaceDecl,
                        DeclRef(namespaceDecl),
                        LookupMask::Default,
                        getDeclToExcludeFromLookup());
                    AddToLookupResult(globalLookupResult, nsLookupResult);
                }
            }
        }
        else if (aggTypeDeclRef || type)
        {
            // We are looking up a member inside a type.
            // We want to be careful here because we should only find members
            // that are implicitly or explicitly `static`.
            //
            if (type == nullptr)
                type = DeclRefType::create(m_astBuilder, aggTypeDeclRef);

            if (as<ErrorType>(type))
            {
                return;
            }

            LookupResult lookupResult = lookUpMember(
                m_astBuilder,
                this,
                expr->name,
                type,
                m_outerScope,
                LookupMask::Default,
                LookupOptions::NoDeref);

            // We need to confirm that whatever member we
            // are trying to refer to is usable via static reference.
            //
            // TODO: eventually we might allow a non-static
            // member to be adapted by turning it into something
            // like a closure that takes the missing `this` parameter.
            //
            // E.g., a static reference to a method could be treated
            // as a value with a function type, where the first parameter
            // is `type`.
            //
            // The biggest challenge there is that we'd need to arrange
            // to generate "dispatcher" functions that could be used
            // to implement that function, in the case where we are
            // making a static reference to some kind of polymorphic declaration.
            //
            // (Also, static references to fields/properties would get even
            // harder, because you'd have to know whether a getter/setter/ref-er
            // is needed).
            //
            // For now let's just be expedient and disallow all of that, because
            // we can always add it back in later.

            // If the lookup result is valid, then we want to filter
            // it to just those candidates that can be referenced statically,
            // and ignore any that would only be allowed as instance members.
            //
            if (lookupResult.isValid())
            {
                // We track both the usable items, and whether or
                // not there were any non-static items that need
                // to be ignored.
                //
                bool anyNonStatic = false;
                List<LookupResultItem> staticItems;
                for (auto item : lookupResult)
                {
                    // Is this item usable as a static member?
                    if (isUsableAsStaticMember(item))
                    {
                        // If yes, then it will be part of the output.
                        staticItems.add(item);
                    }
                    else
                    {
                        // If no, then we might need to output an error.
                        anyNonStatic = true;
                    }
                }

                // Was there anything non-static in the list?
                if (anyNonStatic)
                {
                    // If we had some static items, then that's okay,
                    // we just want to use our newly-filtered list.
                    if (staticItems.getCount())
                    {
                        lookupResult.items = staticItems;
                        lookupResult.item = staticItems[0];
                    }
                    else
                    {
                        // Otherwise, it is time to report an error.
                        getSink()->diagnose(Diagnostics::StaticRefToNonStaticMember{
                            .type = type,
                            .member = expr->name,
                            .location = expr->loc});
                        hasErrors = true;
                        return;
                    }
                }
                // If there were no non-static items, then the `items`
                // array already represents what we'd get by filtering...

                AddToLookupResult(globalLookupResult, lookupResult);
                base = baseExpression;
            }
        }
        else if (auto subscriptDeclRef = baseDeclRef.as<SubscriptDecl>())
        {
            // Accessors are declarations nested in a subscript, so a qualified lookup such as
            // `Type::__subscript::get` should look directly in the resolved subscript rather than
            // in its function type.
            LookupResult lookupResult = lookUpDirectAndTransparentMembers(
                m_astBuilder,
                this,
                expr->name,
                subscriptDeclRef.getDecl(),
                subscriptDeclRef,
                LookupMask::Default,
                getDeclToExcludeFromLookup());

            AddToLookupResult(globalLookupResult, lookupResult);
            // `baseExpression` is the checked inner `Type::__subscript` reference, so it must
            // retain the original type expression as its base. The accessor reference must use
            // that type as its unbound base instead of the function-valued subscript expression.
            base = GetBaseExpr(baseExpression);
            SLANG_RELEASE_ASSERT(base);
        }
        else if (auto callableDecl = as<CallableDecl>(baseDeclRef))
        {
            // Make a decl-ref-type out of the CallableDecl.
            auto funcAsType = DeclRefType::create(m_astBuilder, callableDecl);
            LookupResult lookupResult = lookUpMember(
                m_astBuilder,
                this,
                expr->name,
                funcAsType,
                m_outerScope,
                LookupMask::Default,
                LookupOptions::NoDeref);

            AddToLookupResult(globalLookupResult, lookupResult);
            base = baseExpression;
        }
    };

    auto handleLeafExpr = [&](Expr* e)
    {
        if (auto nsType = as<NamespaceType>(e->type))
            handleLeafCase(nsType->getDeclRef(), nsType);
        else if (auto aggType = as<DeclRefType>(e->type))
            handleLeafCase(aggType->getDeclRef(), aggType);
        else if (as<TypeType>(e->type))
        {
            auto properType = CoerceToProperType(TypeExp(e));
            if (properType.type)
                handleLeafCase(DeclRef<Decl>(), properType.type);
        }
        else if (as<FuncType>(e->type))
        {
            // Callable selection may retain a `LetExpr` that evaluates the qualifier once.
            // Inspect its declaration-reference leaf here; `handleLeafCase` keeps the original
            // `baseExpression` so the selected member still evaluates that binding.
            auto declRefExpr =
                as<DeclRefExpr>(getCallableExprThroughTransparentWrappers(e));
            SLANG_RELEASE_ASSERT(declRefExpr);
            handleLeafCase(declRefExpr->declRef, nullptr);
        }
    };

    auto& baseType = baseExpression->type;
    if (as<ErrorType>(baseType))
    {
        return CreateErrorExpr(expr);
    }

    if (auto overloaded = as<OverloadedExpr>(baseExpression))
    {
        for (auto candidate : overloaded->lookupResult2.items)
            handleLeafCase(candidate.declRef, nullptr);
    }
    else if (auto overloaded2 = as<OverloadedExpr2>(baseExpression))
    {
        for (auto candidate : overloaded2->candidateExprs)
        {
            handleLeafExpr(candidate);
        }
    }
    else
    {
        handleLeafExpr(baseExpression);
    }

    bool diagnosed = false;
    globalLookupResult =
        filterLookupResultByVisibilityAndDiagnose(globalLookupResult, expr->loc, diagnosed);
    diagnosed |= hasErrors;
    if (!globalLookupResult.isValid())
    {
        return lookupMemberResultFailure(expr, baseType, diagnosed);
    }

    if (expr->name == getSession()->getCompletionRequestTokenName())
    {
        suggestCompletionItems(CompletionSuggestions::ScopeKind::Member, globalLookupResult);
    }
    return createLookupResultExpr(expr->name, globalLookupResult, base, expr->loc, expr);
}

Expr* SemanticsExprVisitor::visitStaticMemberExpr(StaticMemberExpr* expr)
{
    SemanticsExprVisitor baseVisitor(withPotentiallyUnevaluatedTerm());
    bool needDeref = false;
    expr->baseExpression = baseVisitor.checkBaseForMemberExpr(
        expr->baseExpression,
        CheckBaseContext::Member,
        needDeref);

    return _lookupStaticMember(expr, expr->baseExpression);
}

// Return true if `interfaceDecl` directly declares a requirement named `requirementName` that is
// visible from `scope` and, when `isStaticAccess`, usable as a static member. A value access `v.m`
// reaches an instance, static, or associated-type requirement (Slang projects an associated type
// through a value, e.g. `v.Element`), so it accepts any of them. A generic requirement such as
// `static int makeGen<U>(U)` is a `GenericDecl` whose `static` modifier is on the inner decl;
// `isDeclUsableAsStaticMember` looks through the wrapper.
static bool doesInterfaceDeclareUsableRequirement(
    SemanticsVisitor* semantics,
    InterfaceDecl* interfaceDecl,
    Name* requirementName,
    bool isStaticAccess,
    Scope* scope)
{
    for (auto requirement : interfaceDecl->getDirectMemberDeclsOfName(requirementName))
    {
        if (isStaticAccess && !semantics->isDeclUsableAsStaticMember(requirement))
            continue;
        if (semantics->isDeclVisibleFromScope(makeDeclRef(requirement), scope))
            return true;
    }
    return false;
}

// Collect the interfaces declared directly in the containers on `scope`'s parent and sibling
// chains (excluding the core module) that are visible from `scope` and directly declare a
// requirement named `requirementName` satisfying `doesInterfaceDeclareUsableRequirement`,
// appending each one once to `outInterfaces`. Each entry is the declaration the interface's name
// refers to: the `GenericDecl` for a generic interface, the `InterfaceDecl` otherwise.
//
// The core module is skipped (as `findClosestInScopeName` does) because it is implicitly in scope
// for every program, so its common requirement names (`equals`, `lessThan`, ...) would follow
// almost any typo. Interfaces from the module being checked and from modules it imports, including
// a standard module such as `slang.numerics`, are kept.
//
// Requirements inherited from a base interface are not matched: a base interface is usually
// visible wherever a derived one is, so naming the interface that declares the member directly is
// enough.
static void collectVisibleInterfacesDeclaringRequirement(
    SemanticsVisitor* semantics,
    Name* requirementName,
    bool isStaticAccess,
    Scope* scope,
    List<Decl*>& outInterfaces)
{
    // A member access whose identifier is missing (e.g. `v.` while typing in the language server)
    // has no name to search for.
    if (!requirementName)
        return;

    // An interface is reached twice when its container is linked into the walk twice, e.g. a
    // namespace that encloses `scope` and is also brought in by `using namespace`.
    HashSet<Decl*> seen;
    for (Scope* s = scope; s; s = s->parent)
    {
        for (Scope* sib = s; sib; sib = sib->nextSibling)
        {
            auto containerDecl = sib->containerDecl;
            if (!containerDecl)
                continue;
            if (isFromCoreModule(containerDecl))
                continue;

            for (auto decl : containerDecl->getDirectMemberDecls())
            {
                auto interfaceDecl = as<InterfaceDecl>(maybeGetInner(decl));
                // An `interface` keyword with no name (e.g. `interface { ... }`) still parses into
                // a nameless `InterfaceDecl`, which the language server keeps checking.
                if (!interfaceDecl || !decl->getName())
                    continue;
                if (!semantics->isDeclVisibleFromScope(makeDeclRef(decl), scope))
                    continue;
                if (!doesInterfaceDeclareUsableRequirement(
                        semantics,
                        interfaceDecl,
                        requirementName,
                        isStaticAccess,
                        scope))
                    continue;
                if (seen.add(decl))
                    outInterfaces.add(decl);
            }
        }
    }
}

bool SemanticsVisitor::doesNameResolveToDecl(Name* name, Scope* scope, Decl* decl)
{
    Decl* resolvedDecl = nullptr;
    for (auto item : lookUp(m_astBuilder, this, name, scope))
    {
        if (!isDeclVisibleFromScope(item.declRef, scope))
            continue;
        auto itemDecl = item.declRef.getDecl();
        if (resolvedDecl && resolvedDecl != itemDecl)
            return false;
        resolvedDecl = itemDecl;
    }
    return resolvedDecl == decl;
}

void SemanticsVisitor::maybeSuggestMissingGenericConstraintForMemberLookup(
    DeclRefExpr* expr,
    QualType const& baseType)
{
    // A `::` access is always a static lookup, even on a value (`v::m`, whose `baseType` is the
    // value's type). A `.` access is static when its base is a type (`T.m`, a `TypeType`).
    bool isStaticAccess = as<StaticMemberExpr>(expr) != nullptr;
    Type* type = baseType.type;
    if (auto typeType = as<TypeType>(type))
    {
        type = typeType->getType();
        isStaticAccess = true;
    }

    // The note proposes `where <Param> : <Interface>`, so the base must be a generic type
    // parameter the user declared and can constrain. Other constrainable bases (an associated type
    // `T.Assoc`, a global `type_param`) would need different wording.
    auto declRefType = as<DeclRefType>(type);
    if (!declRefType)
        return;
    auto genericParamDeclRef = declRefType->getDeclRef().as<GenericTypeParamDecl>();
    if (!genericParamDeclRef)
        return;
    auto genericParamDecl = genericParamDeclRef.getDecl();

    // In an interface method with a default body, `this` has the type of the `This` parameter of
    // the synthesized `InterfaceDefaultImplDecl`. The user cannot write a `where` clause for it;
    // the fix there is to make the interface inherit the other one.
    if (auto defaultImplDecl = as<InterfaceDefaultImplDecl>(genericParamDecl->parentDecl);
        defaultImplDecl && defaultImplDecl->thisTypeDecl == genericParamDecl)
        return;

    // Candidates are found and their requirements checked for visibility from the failed access
    // (`m_outerScope`, the scope member lookup used), which is nested inside the generic that owns
    // the parameter. Whether a candidate can be *named* is a separate question, answered below at
    // the constraint site.
    List<Decl*> candidates;
    collectVisibleInterfacesDeclaringRequirement(
        this,
        expr->name,
        isStaticAccess,
        m_outerScope,
        candidates);
    if (candidates.getCount() == 0)
        return;

    // A constraint is written on the generic declaration that owns the parameter, so we only
    // suggest an interface whose unqualified name, looked up from there, resolves to exactly that
    // interface. Consider `float3 read<IHasNormal, T>(T value) { return value.getNormal(); }`: the
    // interface `IHasNormal` declares `getNormal`, but inside `read` that name means the first
    // generic parameter, so suggesting `where T : IHasNormal` would not help. Two same-named
    // interfaces at the same lookup level (e.g. one imported by `using namespace`) make the name
    // ambiguous, and neither is suggested.
    Scope* constraintScope = getScope(genericParamDecl);
    List<Decl*> interfaceDecls;
    for (auto candidate : candidates)
    {
        if (doesNameResolveToDecl(candidate->getName(), constraintScope, candidate))
            interfaceDecls.add(candidate);
    }

    // Each surviving name resolves to its own interface, so the names are distinct and sorting by
    // them gives an order that does not depend on the scope walk.
    interfaceDecls.sort([](Decl* left, Decl* right)
                        { return left->getName()->text < right->getName()->text; });

    auto genericParamName = genericParamDecl->getName();
    for (auto interfaceDecl : interfaceDecls)
    {
        // A generic interface needs type arguments we cannot infer here, so it gets advisory
        // wording rather than a `where` clause that would not compile as printed.
        if (as<GenericDecl>(interfaceDecl))
        {
            getSink()->diagnose(Diagnostics::SuggestGenericInterfaceConstraintForMissingMember{
                .genericParam = genericParamName,
                .interfaceName = interfaceDecl->getName(),
                .member = expr->name,
                .location = expr->loc});
        }
        else
        {
            getSink()->diagnose(Diagnostics::SuggestConstraintForMissingMember{
                .genericParam = genericParamName,
                .interfaceName = interfaceDecl->getName(),
                .member = expr->name,
                .location = expr->loc});
        }
    }
}

Expr* SemanticsVisitor::lookupMemberResultFailure(
    DeclRefExpr* expr,
    QualType const& baseType,
    bool supressDiagnostic)
{
    // Check it's a member expression
    SLANG_ASSERT(as<StaticMemberExpr>(expr) || as<MemberExpr>(expr));

    expr->type = QualType(m_astBuilder->getErrorType());
    if (!supressDiagnostic)
    {
        if (!maybeDiagnoseAmbiguousReference(GetBaseExpr(expr)))
        {
            getSink()->diagnose(Diagnostics::NoMemberOfNameInType{
                .name = expr->name,
                .type = baseType.type,
                .expr = expr});
            maybeSuggestMissingGenericConstraintForMemberLookup(expr, baseType);
        }
    }
    return expr;
}

Expr* SemanticsVisitor::maybeInsertImplicitOpForMemberBase(
    Expr* baseExpr,
    CheckBaseContext checkBaseContext,
    bool& outNeedDeref)
{
    // In case our base expression is still overloaded, we can perform
    // some more refinement.
    //
    // Handle the case of an overloaded base expression
    // here, in case we can use the name of the member to
    // disambiguate which of the candidates is meant, or if
    // we can return an overloaded result.
    //
    if (auto overloadedExpr = as<OverloadedExpr>(baseExpr))
    {
        // If a member (dynamic or static) lookup result contains both the actual definition
        // and the interface definition obtained from inheritance, we want to filter out
        // the interface definitions.
        LookupResult filteredLookupResult;
        for (auto lookupResult : overloadedExpr->lookupResult2.items)
        {
            bool shouldRemove = false;
            if (lookupResult.declRef.getParent().as<InterfaceDecl>())
            {
                shouldRemove = true;
            }
            if (lookupResult.declRef.getDecl()->hasModifier<ExtensionExternVarModifier>())
                shouldRemove = true;
            if (!shouldRemove)
            {
                AddToLookupResult(filteredLookupResult, lookupResult);
            }
        }
        baseExpr = createLookupResultExpr(
            overloadedExpr->name,
            filteredLookupResult,
            overloadedExpr->base,
            overloadedExpr->loc,
            overloadedExpr);
        // TODO: handle other cases of OverloadedExpr that need filtering.
    }

    // Dereference after resolving overloaded expressions, so that the
    // concrete type is available for pointer-like dereferences.
    auto derefExpr = maybeDereference(baseExpr, checkBaseContext);
    if (derefExpr != baseExpr)
        outNeedDeref = true;
    baseExpr = derefExpr;

    // If the base of the member lookup has an interface type
    // *without* a suitable this-type substitution, then we are
    // trying to perform lookup on a value of existential type,
    // and we should "open" the existential here so that we
    // can expose its structure.
    //
    baseExpr = maybeOpenExistential(baseExpr);


    return baseExpr;
}

Expr* SemanticsVisitor::checkBaseForMemberExpr(
    Expr* inBaseExpr,
    CheckBaseContext checkBaseContext,
    bool& outNeedDeref)
{
    ExprLocalScope localScope;
    SemanticsVisitor scopedVisitor(withExprLocalScope(&localScope));

    auto baseExpr = scopedVisitor.CheckTerm(inBaseExpr);
    if (IsErrorExpr(baseExpr))
        return baseExpr;

    auto resultBaseExpr = scopedVisitor.maybeInsertImplicitOpForMemberBase(
        baseExpr,
        checkBaseContext,
        outNeedDeref);

    // We might want to register differentiability on any implicit ops that we add in.
    if (scopedVisitor.m_parentFunc)
    {
        if (scopedVisitor.m_parentFunc->findModifier<DifferentiableAttribute>())
        {
            scopedVisitor.maybeRegisterDifferentiableType(
                getASTBuilder(),
                resultBaseExpr->type.type,
                resultBaseExpr->loc);
        }
    }

    return _wrapExpressionInLocalBindings(resultBaseExpr, localScope);
}

Expr* SemanticsVisitor::checkGeneralMemberLookupExpr(MemberExpr* expr, Type* baseType)
{
    LookupResult lookupResult =
        lookUpMember(m_astBuilder, this, expr->name, baseType, m_outerScope);
    bool diagnosed = false;
    lookupResult = filterLookupResultByVisibilityAndDiagnose(lookupResult, expr->loc, diagnosed);
    lookupResult =
        filterLookupResultByCheckedOptionalAndDiagnose(lookupResult, expr->loc, diagnosed);
    if (!lookupResult.isValid())
    {
        return lookupMemberResultFailure(expr, baseType, diagnosed);
    }
    if (expr->name == getSession()->getCompletionRequestTokenName())
    {
        suggestCompletionItems(CompletionSuggestions::ScopeKind::Member, lookupResult);
        if (expr->baseExpression)
        {
            if (auto vectorType = as<VectorExpressionType>(expr->baseExpression->type))
            {
                auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
                suggestions.scopeKind = CompletionSuggestions::ScopeKind::Swizzle;
                suggestions.elementCount[1] = 0;
                suggestions.swizzleBaseType = vectorType;
                if (auto elementCount = as<ConstantIntVal>(vectorType->getElementCount()))
                    suggestions.elementCount[0] = elementCount->getValue();
                else
                    suggestions.elementCount[0] = 1;
            }
            else if (auto scalarType = as<BasicExpressionType>(expr->baseExpression->type))
            {
                auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
                suggestions.scopeKind = CompletionSuggestions::ScopeKind::Swizzle;
                suggestions.elementCount[1] = 0;
                suggestions.elementCount[0] = 1;
                suggestions.swizzleBaseType = scalarType;
            }
            else if (auto matrixType = as<MatrixExpressionType>(expr->baseExpression->type))
            {
                auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
                suggestions.clear();
                suggestions.scopeKind = CompletionSuggestions::ScopeKind::Swizzle;
                suggestions.swizzleBaseType = matrixType;
                suggestions.elementCount[0] = 0;
                suggestions.elementCount[1] = 0;
                if (auto rowCount = as<ConstantIntVal>(matrixType->getRowCount()))
                    suggestions.elementCount[0] = rowCount->getValue();
                if (auto colCount = as<ConstantIntVal>(matrixType->getColumnCount()))
                    suggestions.elementCount[1] = colCount->getValue();
            }
            else if (auto tupleType = as<TupleType>(expr->baseExpression->type))
            {
                auto& suggestions = getLinkage()->contentAssistInfo.completionSuggestions;
                suggestions.scopeKind = CompletionSuggestions::ScopeKind::Swizzle;
                suggestions.elementCount[0] = tupleType->getMemberCount();
                suggestions.elementCount[1] = 0;
                suggestions.swizzleBaseType = tupleType;
            }
        }
    }
    return createLookupResultExpr(expr->name, lookupResult, expr->baseExpression, expr->loc, expr);
}

Expr* SemanticsExprVisitor::visitMemberExpr(MemberExpr* expr)
{
    SemanticsExprVisitor baseVisitor(withPotentiallyUnevaluatedTerm());
    bool needDeref = false;
    expr->baseExpression = baseVisitor.checkBaseForMemberExpr(
        expr->baseExpression,
        CheckBaseContext::Member,
        needDeref);

    if (!needDeref && as<DerefMemberExpr>(expr) && !as<PtrType>(expr->baseExpression->type))
    {
        // The user is trying to use the `->` operator on something that can't be
        // dereferenced, so we should diagnose that.
        if (!as<ErrorType>(expr->baseExpression->type))
            getSink()->diagnose(Diagnostics::CannotDereferenceType{
                .type = expr->baseExpression->type.type,
                .location = expr->memberOperatorLoc});
    }

    auto baseType = expr->baseExpression->type;

    // If we are looking up through a modified type, just pass straight
    // through the inner type.
    if (auto modifiedType = as<ModifiedType>(baseType))
        baseType = modifiedType->getBase();

    // Try handle swizzle-able types (scalar,vector,matrix) first.
    // If checking as a swizzle failed for these types,
    // we will fallback to normal member lookup.
    //
    if (auto baseScalarType = as<BasicExpressionType>(baseType))
    {
        // Treat scalar like a 1-element vector when swizzling
        auto swizzle = CheckSwizzleExpr(expr, baseScalarType, 1);
        if (swizzle)
            return swizzle;
    }
    else if (auto baseVecType = as<VectorExpressionType>(baseType))
    {
        auto swizzle =
            CheckSwizzleExpr(expr, baseVecType->getElementType(), baseVecType->getElementCount());
        if (swizzle)
            return swizzle;
    }
    else if (auto baseMatrixType = as<MatrixExpressionType>(baseType))
    {
        auto swizzle = CheckMatrixSwizzleExpr(
            expr,
            baseMatrixType->getElementType(),
            baseMatrixType->getRowCount(),
            baseMatrixType->getColumnCount());
        if (swizzle)
            return swizzle;
    }

    if (as<NamespaceType>(baseType))
    {
        return _lookupStaticMember(expr, expr->baseExpression);
    }
    else if (const auto typeType = as<TypeType>(baseType); typeType)
    {
        return _lookupStaticMember(expr, expr->baseExpression);
    }
    else if (as<OverloadedExpr>(expr->baseExpression))
    {
        return _lookupStaticMember(expr, expr->baseExpression);
    }
    else if (as<OverloadedExpr2>(expr->baseExpression))
    {
        return _lookupStaticMember(expr, expr->baseExpression);
    }
    else if (auto baseTupleType = as<TupleType>(baseType))
    {
        return checkTupleSwizzleExpr(expr, baseTupleType);
    }
    else if (as<ErrorType>(baseType))
    {
        return CreateErrorExpr(expr);
    }
    else if (as<FuncType>(baseType))
    {
        return _lookupStaticMember(expr, expr->baseExpression);
    }
    else
    {
        return checkGeneralMemberLookupExpr(expr, baseType);
    }
}

Expr* SemanticsExprVisitor::visitMakeArrayFromElementExpr(MakeArrayFromElementExpr* expr)
{
    return expr;
}

Expr* SemanticsExprVisitor::visitInitializerListExpr(InitializerListExpr* expr)
{
    // If we are assigned a type, expr has already been legalized
    if (expr->type)
        return expr;

    // When faced with an initializer list, we first just check the sub-expressions blindly.
    // Actually making them conform to a desired type will wait for when we know the desired
    // type based on context.

    for (auto& arg : expr->args)
    {
        arg = CheckTerm(arg);
    }

    expr->type = m_astBuilder->getInitializerListType();

    return expr;
}

/// Checks an object-oriented `this` expression and creates its lambda capture reference if needed.
///
/// `ConstructLookupResultExpr()` uses `checkThisExpr()` while replaying a `ThisValue` breadcrumb
/// for a candidate path. `SelectedExpressionFinalizer` attaches the capture field later, after
/// overload resolution has discarded unused candidates.
Expr* SemanticsVisitor::checkThisExpr(ThisExpr* expr)
{
    // `this` starts as an r-value. An enclosing constructor can make it writable, and the enclosing
    // aggregate supplies its type. An instance callable instead supplies both its type and value
    // category.
    expr->type.isLeftValue = false;

    // The outward scope search finds the constructor, callable, aggregate, or extension that
    // supplies `this`.
    auto scope = expr->scope;
    while (scope)
    {
        auto containerDecl = scope->containerDecl;

        if (as<ConstructorDecl>(containerDecl))
        {
            expr->type.isLeftValue = true;
        }
        else if (auto funcDeclBase = as<FunctionDeclBase>(containerDecl))
        {
            std::optional<ParamInfo> thisParamInfo;
            bool isCheckingSignature = funcDeclBase->checkState.isBeingChecked() &&
                                       !funcDeclBase->isChecked(DeclCheckState::SignatureChecked);
            if (isCheckingSignature)
            {
                // `checkAndAttachEffectiveThisParamInfo()` publishes the receiver information
                // before signature checking examines default arguments and array bounds. An
                // explicit or implicit `this` in those expressions reads the published attribute
                // without recursively ensuring the method.
                SLANG_RELEASE_ASSERT(
                    funcDeclBase->checkState.getState() == DeclCheckState::ScopesWired);
                if (auto attribute = funcDeclBase->findModifier<ThisParamInfoAttribute>())
                    thisParamInfo = attribute->info;
            }
            else
            {
                thisParamInfo = findEffectiveThisParamInfo(getDefaultDeclRef(funcDeclBase));
            }

            // Every callable is a lexical boundary for `this`. A callable without published
            // receiver information must not inherit an unrelated receiver from an outer scope.
            if (!thisParamInfo)
            {
                if (auto sink = getSink())
                {
                    sink->diagnose(
                        Diagnostics::ThisExpressionInCallableWithoutReceiver{.expr = expr});
                }
                return CreateErrorExpr(expr);
            }

            expr->type.type = thisParamInfo->type;
            expr->type.isLeftValue = isThisExprWritable(
                getDefaultDeclRef(funcDeclBase).as<CallableDecl>(),
                *thisParamInfo);
            return _captureThisForLambdaIfNeeded(this, expr);
        }
        else if (auto typeOrExtensionDecl = as<AggTypeDeclBase>(containerDecl))
        {
            expr->type.type = calcThisType(makeDeclRef(typeOrExtensionDecl));
            return _captureThisForLambdaIfNeeded(this, expr);
        }
        else if (auto defaultImplDecl = as<InterfaceDefaultImplDecl>(containerDecl))
        {
            expr->type.type =
                DeclRefType::create(m_astBuilder, DeclRef<Decl>(defaultImplDecl->thisTypeDecl));
            return _captureThisForLambdaIfNeeded(this, expr);
        }
#if 0
            else if (auto aggTypeDecl = as<AggTypeDecl>(containerDecl))
            {
                ensureDecl(aggTypeDecl, DeclCheckState::CanUseAsType);

                // Okay, we are using `this` in the context of an
                // aggregate type, so the expression should be
                // of the corresponding type.
                expr->type.type = DeclRefType::Create(
                    getSession(),
                    makeDeclRef(aggTypeDecl));
                return expr;
            }
            else if (auto extensionDecl = as<ExtensionDecl>(containerDecl))
            {
                ensureDecl(extensionDecl, DeclCheckState::CanUseExtensionTargetType);

                // When `this` is used in the context of an `extension`
                // declaration, then it should refer to an instance of
                // the type being extended.
                //
                // TODO: There is potentially a small gotcha here that
                // lookup through such a `this` expression should probably
                // prioritize members declared in the current extension
                // if there are multiple extensions in scope that add
                // members with the same name...
                //
                expr->type.type = extensionDecl->targetType.type;
                return expr;
            }
#endif

        scope = scope->parent;
    }

    if (auto sink = getSink())
        sink->diagnose(Diagnostics::ThisExpressionOutsideOfTypeDecl{.expr = expr});

    return CreateErrorExpr(expr);
}

Expr* SemanticsExprVisitor::visitThisExpr(ThisExpr* expr)
{
    return checkThisExpr(expr);
}

Expr* SemanticsExprVisitor::visitThisTypeExpr(ThisTypeExpr* expr)
{
    auto scope = expr->scope;
    while (scope)
    {
        auto containerDecl = scope->containerDecl;
        if (auto typeOrExtensionDecl = as<AggTypeDeclBase>(containerDecl))
        {
            auto thisType = calcThisType(makeDeclRef(typeOrExtensionDecl));
            auto thisTypeType = m_astBuilder->getTypeType(thisType);

            expr->type.type = thisTypeType;
            return expr;
        }
        else if (auto defaultImplDecl = as<InterfaceDefaultImplDecl>(containerDecl))
        {
            expr->type.type =
                DeclRefType::create(m_astBuilder, DeclRef<Decl>(defaultImplDecl->thisTypeDecl));
            return expr;
        }
        scope = scope->parent;
    }

    getSink()->diagnose(Diagnostics::ThisTypeOutsideOfTypeDecl{.expr = expr});
    return CreateErrorExpr(expr);
}

Expr* SemanticsExprVisitor::visitThisInterfaceExpr(ThisInterfaceExpr* expr)
{
    auto scope = expr->scope;

    auto containerDecl = findParentInterfaceDecl(scope->containerDecl);

    // ThisInterfaceExpr can only be synthesized by the compiler during parsing
    // an interface decl with default implementation, so container must always
    // be an interface decl.
    SLANG_ASSERT(containerDecl);
    expr->declRef =
        createDefaultSubstitutionsIfNeeded(m_astBuilder, this, getDefaultDeclRef(containerDecl));
    expr->type = m_astBuilder->getTypeType(DeclRefType::create(m_astBuilder, expr->declRef));
    return expr;
}

Expr* SemanticsExprVisitor::visitCastToSuperTypeExpr(CastToSuperTypeExpr* expr)
{
    updateCastToSuperTypeStorageInfo(m_astBuilder, expr);
    return expr;
}

Expr* SemanticsExprVisitor::visitReturnValExpr(ReturnValExpr* expr)
{
    auto scope = expr->scope;
    if (scope)
    {
        auto parentFunc = as<CallableDecl>(getParentFunc(scope->containerDecl));
        if (parentFunc)
        {
            if (as<ErrorType>(parentFunc->returnType.type))
            {
                expr->type = parentFunc->returnType.type;
                return expr;
            }
            if (isNonCopyableType(parentFunc->returnType.type))
            {
                expr->type.isLeftValue = true;
                expr->type.type = parentFunc->returnType.type;
                return expr;
            }
        }
    }
    getSink()->diagnose(Diagnostics::ReturnValNotAvailable{.expr = expr});
    expr->type = getASTBuilder()->getErrorType();
    return expr;
}

Expr* SemanticsExprVisitor::visitAndTypeExpr(AndTypeExpr* expr)
{
    // The left and right sides of an `&` for types must both be types.
    //
    expr->left = CheckProperType(expr->left);
    expr->right = CheckProperType(expr->right);

    // TODO: We should enforce some rules here about what is allowed
    // for the `left` and `right` types.
    //
    // For now, the right rule is that they probably need to either
    // be interfaces, or conjunctions thereof.
    //
    // Eventually it may be valuable to support more flexible
    // types in conjunctions, especialy in cases where inheritance
    // gets involved.

    // The result of this expression is an `AndType`, which we need
    // to wrap in a `TypeType` to indicate that the result is the type
    // itself and not a value of  that type.
    //
    auto andType = m_astBuilder->getAndType(expr->left.type, expr->right.type);
    expr->type = m_astBuilder->getTypeType(andType);

    return expr;
}

Expr* SemanticsExprVisitor::visitPointerTypeExpr(PointerTypeExpr* expr)
{
    expr->base = CheckProperType(expr->base);
    if (as<ErrorType>(expr->base.type))
        expr->type = expr->base.type;
    auto ptrType = m_astBuilder->getPtrType(
        expr->base.type,
        AccessQualifier::ReadWrite,
        AddressSpace::UserPointer,
        m_astBuilder->getDefaultLayoutType());
    expr->type = m_astBuilder->getTypeType(ptrType);
    return expr;
}

Expr* SemanticsExprVisitor::visitModifiedTypeExpr(ModifiedTypeExpr* expr)
{
    // The base type should be a proper type (not an expression, generic, etc.)
    //
    expr->base = CheckProperType(expr->base);
    auto baseType = expr->base.type;

    // We will check the modifiers that were applied to the type expression
    // one by one, and collect a list of the ones that should modify the
    // resulting `Type`.
    //
    List<Val*> modifierVals;
    for (auto modifier : expr->modifiers)
    {
        if (auto matrixLayoutModifier = as<MatrixLayoutModifier>(modifier))
        {
            if (auto matrixType = as<MatrixExpressionType>(baseType))
            {
                if (as<ColumnMajorLayoutModifier>(matrixLayoutModifier))
                {
                    baseType = m_astBuilder->getMatrixType(
                        matrixType->getElementType(),
                        matrixType->getRowCount(),
                        matrixType->getColumnCount(),
                        m_astBuilder->getIntVal(
                            matrixType->getLayout()->getType(),
                            kMatrixLayoutMode_ColumnMajor));
                }
                else
                {
                    baseType = m_astBuilder->getMatrixType(
                        matrixType->getElementType(),
                        matrixType->getRowCount(),
                        matrixType->getColumnCount(),
                        m_astBuilder->getIntVal(
                            matrixType->getLayout()->getType(),
                            kMatrixLayoutMode_RowMajor));
                }
                expr->type = m_astBuilder->getTypeType(baseType);
            }
            else
            {
                getSink()->diagnose(Diagnostics::MatrixLayoutModifierOnNonMatrixType{
                    .type = baseType,
                    .location = matrixLayoutModifier->loc});
            }
            continue;
        }
        auto modifierVal = checkTypeModifier(modifier, baseType);
        if (!modifierVal)
            continue;
        modifierVals.add(modifierVal);
    }

    if (modifierVals.getCount())
    {
        auto modifiedType = m_astBuilder->getModifiedType(baseType, modifierVals);
        expr->type = m_astBuilder->getTypeType(modifiedType);
    }
    else if (expr->type == nullptr)
    {
        // It is possible that all modifiers were pruned in case the modifier
        // list contained only modifiers that triggered diagnostics (e.g.,
        // ConstModifier, HLSLVolatileModifier). We'll set the type here to
        // avoid further diagnostics later in the pipeline.
        expr->type = m_astBuilder->getTypeType(baseType);
    }

    return expr;
}

Val* SemanticsExprVisitor::checkTypeModifier(Modifier* modifier, Type* type)
{
    SLANG_UNUSED(type);

    if (const auto unormModifier = as<UNormModifier>(modifier); unormModifier)
    {
        // TODO: validate that `type` is either `float` or a vector of `float`s
        return m_astBuilder->getUNormModifierVal();
    }
    else if (const auto snormModifier = as<SNormModifier>(modifier); snormModifier)
    {
        // TODO: validate that `type` is either `float` or a vector of `float`s
        return m_astBuilder->getSNormModifierVal();
    }
    else if (const auto noDiffModifier = as<NoDiffModifier>(modifier); noDiffModifier)
    {
        return m_astBuilder->getNoDiffModifierVal();
    }
    else if (as<ConstModifier>(modifier))
    {
        getSink()->diagnose(Diagnostics::ConstNotAllowedOnType{.location = modifier->loc});
        return nullptr;
    }
    else if (as<HLSLVolatileModifier>(modifier))
    {
        getSink()->diagnose(Diagnostics::VolatileNotAllowedOnType{.location = modifier->loc});
        return nullptr;
    }
    else if (as<GLSLVolatileModifier>(modifier))
    {
        // Note: 'volatile' adds both HLSLVolatileModifier and GLSLVolatileModifier.
        // We're already diagnosing on HLSLVolatileModifier.
        return nullptr;
    }
    else
    {
        getSink()->diagnose(Diagnostics::ModifierNotAllowed{.modifier = modifier});
        return nullptr;
    }
}

Expr* SemanticsExprVisitor::visitFuncTypeExpr(FuncTypeExpr* expr)
{
    // The input and output to a function type must both be types
    for (auto& t : expr->parameters)
        t = CheckProperType(t);
    expr->result = CheckProperType(expr->result);

    // TODO: Kind checking? Where are we stopping someone passing
    // constraints around as value-inhabitable types

    // The result of this expression is a `FuncType`, which we need
    // to wrap in a `TypeType` to indicate that the result is the type
    // itself and not a value of that type.
    List<Type*> types;
    types.reserve(expr->parameters.getCount());
    for (const auto& t : expr->parameters)
        types.add(t.type);
    auto funcType = m_astBuilder->getFuncType(types.getArrayView(), expr->result.type);
    expr->type = m_astBuilder->getTypeType(funcType);

    return expr;
}

Expr* SemanticsExprVisitor::visitTupleTypeExpr(TupleTypeExpr* expr)
{
    // All tuple members must be types
    for (auto& t : expr->members)
        t = CheckProperType(t);

    // As in the other cases above, wrap in TypeType
    List<Type*> types;
    types.reserve(expr->members.getCount());
    for (auto t : expr->members)
        types.add(t.type);
    auto tupleType = m_astBuilder->getTupleType(types.getArrayView());
    expr->type = m_astBuilder->getTypeType(tupleType);

    return expr;
}

Expr* SemanticsExprVisitor::visitPackBranchTypeExpr(PackBranchTypeExpr* expr)
{
    expr->packOperand.exp = CheckTerm(expr->packOperand.exp);

    bool isTypeExpr = false;
    Type* operandType = nullptr;
    if ([[maybe_unused]] auto typeType = as<TypeType>(expr->packOperand.exp->type))
    {
        isTypeExpr = true;
        operandType = CoerceToProperType(expr->packOperand).type;
    }
    else
    {
        operandType = expr->packOperand.exp->type.type;
    }

    if (!_isTypeOrValValidForPackBranch(operandType))
    {
        getSink()->diagnose(
            Diagnostics::PackQueryArgumentIsInvalid{.queryName = "__packBranch", .expr = expr});
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    Val* packOperandVal = tryConstantFoldExpr(
        SubstExpr<Expr>(expr->packOperand.exp),
        ConstantFoldingKind::CompileTime,
        nullptr);
    if (!packOperandVal && isTypeExpr)
        packOperandVal = operandType;

    if (!packOperandVal)
    {
        // For term-valued packs we need an actual symbolic pack `Val`, not just the
        // pack's type. Falling back to `ValuePackType` (or a tuple value's `TupleType`)
        // would erase which pack expression we branched on, so distinct packs with the
        // same element type/shape could alias to the same `PackBranchType`.
        if (as<ValuePackType>(operandType) || as<TupleType>(operandType))
        {
            getSink()->diagnose(
                Diagnostics::PackQueryArgumentIsInvalid{.queryName = "__packBranch", .expr = expr});
            expr->type = m_astBuilder->getErrorType();
            return expr;
        }

        packOperandVal = operandType;
    }

    auto packCardinality = getPackCardinality(packOperandVal);

    auto checkNonEmptyBranchType = [&]() -> TypeExp
    {
        AssumedNonEmptyPackInfo assumedNonEmptyPackInfo;
        assumedNonEmptyPackInfo.pack = packOperandVal;
        assumedNonEmptyPackInfo.next = m_assumedNonEmptyPack;
        SemanticsVisitor subVisitor(this->withAssumedNonEmptyPack(&assumedNonEmptyPackInfo));
        return subVisitor.CheckProperType(expr->nonEmptyType);
    };

    if (packCardinality == VariadicPackCardinality::Empty)
    {
        expr->emptyType = CheckProperType(expr->emptyType);
        expr->type = m_astBuilder->getTypeType(expr->emptyType.type);
        return expr;
    }

    if (packCardinality == VariadicPackCardinality::NonEmpty)
    {
        expr->nonEmptyType = checkNonEmptyBranchType();
        expr->type = m_astBuilder->getTypeType(expr->nonEmptyType.type);
        return expr;
    }

    expr->emptyType = CheckProperType(expr->emptyType);
    expr->nonEmptyType = checkNonEmptyBranchType();
    if (as<ErrorType>(expr->emptyType.type) || as<ErrorType>(expr->nonEmptyType.type))
    {
        expr->type = m_astBuilder->getErrorType();
        return expr;
    }

    auto packBranchType = m_astBuilder->getPackBranchType(
        packOperandVal,
        expr->emptyType.type,
        expr->nonEmptyType.type);
    expr->type = m_astBuilder->getTypeType(packBranchType);
    return expr;
}

Expr* SemanticsExprVisitor::visitSPIRVAsmExpr(SPIRVAsmExpr* expr)
{
    //
    // Firstly, get the info for this op, the opcode has already been
    // discovered by the parser
    //
    const auto& spirvInfo = getSession()->spirvCoreGrammarInfo;

    // We will iterate over all the operands in all the insts and check
    // them. Setting `failed` makes us return an error-typed expression via
    // `CreateErrorExpr` at the end of this function; a caller only keeps that
    // expression away from IR lowering (which aborts on an ErrorType) when the
    // sink's error count is non-zero. So every site that sets `failed` must
    // diagnose an *error*, never a warning — a warning-severity `failed` path
    // is what caused the abort in #12497. (The lone warning in this function,
    // SpirvLayoutSensitiveTypeInAsm, deliberately does not set `failed`.)
    bool failed = false;

    // Track %id's that have been defined in this asm block.
    HashSet<Name*> definedIds;

    for (auto& inst : expr->insts)
    {
        // It's not automatically a failure to not have info, we just won't
        // be able to deduce types for operands
        const auto opcode = SpvOp(inst.opcode.knownValue);
        const auto opInfo = spirvInfo->opInfos.lookup(opcode);

        if (opInfo && opInfo->numOperandTypes == 0 && inst.operands.getCount())
        {
            // Per the `failed` invariant above, this diagnoses an error (not
            // the parser's E29106 semicolon-hint warning): the opcode takes no
            // operands, so this is a definite error rather than a recovery guess.
            failed = true;
            getSink()->diagnose(Diagnostics::SpirvInstructionTakesNoOperands{
                .opcode = inst.opcode.token.getContent(),
                .location = inst.opcode.token.loc});
            continue;
        }
        int resultIdIndex = -1;
        if (opInfo)
        {
            resultIdIndex = opInfo->resultIdIndex;
        }
        else if (inst.opcode.flavor == SPIRVAsmOperand::TruncateMarker)
        {
            // If this is __truncate, register the result id in the third operand.
            resultIdIndex = 1;
        }
        else
        {
            // If there is no opInfo, just register all Ids as defined.
            for (auto& operand : inst.operands)
            {
                if (operand.flavor == SPIRVAsmOperand::Id)
                {
                    definedIds.add(operand.token.getName());
                }
            }
        }

        // Register result ID.
        if (resultIdIndex != -1)
        {
            if (inst.operands.getCount() <= resultIdIndex)
            {
                failed = true;
                getSink()->diagnose(Diagnostics::SpirvInstructionWithNotEnoughOperands{
                    .opcode = inst.opcode.token.getContent(),
                    .location = inst.opcode.token.loc});
                continue;
            }
            auto& resultIdOperand = inst.operands[resultIdIndex];

            if (!definedIds.add(resultIdOperand.token.getName()))
            {
                failed = true;
                getSink()->diagnose(Diagnostics::SpirvIdRedefinition{
                    .id = resultIdOperand.token.getContent(),
                    .location = inst.opcode.token.loc});
                continue;
            }
        }

        const bool isLast = &inst == &expr->insts.getLast();
        for (Index operandIndex = 0; operandIndex < inst.operands.getCount(); ++operandIndex)
        {
            // Clamp to the end of the type info array, because the last one will be any
            // variable operands
            const auto invalidOperandKind = SPIRVCoreGrammarInfo::OperandKind{0xff};
            const auto operandType =
                opInfo.has_value()
                    ? opInfo
                          ->operandTypes[std::min(operandIndex, Index(opInfo->numOperandTypes) - 1)]
                    : invalidOperandKind;
            const auto baseOperandType =
                spirvInfo->operandKindUnderneathIds.lookup(operandType).value_or(operandType);
            const auto needsIdWrapper = baseOperandType != operandType;

            const auto check = [&](const auto& go, auto& operand) -> void
            {
                if (operand.flavor == SPIRVAsmOperand::SlangType ||
                    operand.flavor == SPIRVAsmOperand::SampledType)
                {
                    // This is a $$type operand or __sampledType(T)
                    // operand, fill in its TypeExp member.
                    TypeExp& typeExpr = operand.type;
                    typeExpr.exp = operand.expr;
                    typeExpr = CheckProperType(typeExpr);
                    operand.expr = typeExpr.exp;
                }
                else if (
                    operand.flavor == SPIRVAsmOperand::SlangValue ||
                    operand.flavor == SPIRVAsmOperand::SlangImmediateValue ||
                    operand.flavor == SPIRVAsmOperand::SlangValueAddr ||
                    operand.flavor == SPIRVAsmOperand::ImageType ||
                    operand.flavor == SPIRVAsmOperand::SampledImageType ||
                    operand.flavor == SPIRVAsmOperand::ConvertTexel ||
                    operand.flavor == SPIRVAsmOperand::RayPayloadFromLocation ||
                    operand.flavor == SPIRVAsmOperand::RayAttributeFromLocation ||
                    operand.flavor == SPIRVAsmOperand::RayCallableFromLocation)
                {
                    // This is a $expr operand, check the expr
                    operand.expr = dispatch(operand.expr);
                }
                else if (operand.flavor == SPIRVAsmOperand::ResultMarker)
                {
                    // This is the <result-id> marker, check that it only
                    // appears in the last instruction.

                    // TODO: We could consider relaxing this, because SPIR-V
                    // does have forward references for decorations and such
                    if (!isLast)
                    {
                        getSink()->diagnose(
                            Diagnostics::MisplacedResultIdMarker{.location = operand.token.loc});
                        getSink()->diagnose(Diagnostics::ConsiderOpCopyObject{});
                    }
                }
                else if (operand.flavor == SPIRVAsmOperand::NamedValue)
                {
                    // First try and look it up with the knowledge of this operand's type
                    auto enumValue =
                        spirvInfo->allEnums.lookup({baseOperandType, operand.token.getContent()});
                    // Then fall back to with the type prefix
                    if (!enumValue)
                        enumValue =
                            spirvInfo->allEnumsWithTypePrefix.lookup(operand.token.getContent());
                    // Then see if it's an opcode (for OpSpecialize)
                    if (!enumValue)
                        enumValue = spirvInfo->opcodes.lookup(operand.token.getContent());
                    if (inst.opcode.knownValue == SpvOpExtInst)
                    {
                        if (!enumValue)
                        {
                            GLSLstd450 val;
                            if (lookupGLSLstd450(operand.token.getContent(), val))
                            {
                                enumValue = (SpvWord)val;
                            }
                        }
                    }
                    if (!enumValue)
                    {
                        failed = true;
                        getSink()->diagnose(Diagnostics::SpirvUnableToResolveName{
                            .name = operand.token.getContent(),
                            .location = operand.token.loc});
                        return;
                    }

                    operand.knownValue = *enumValue;
                    operand.wrapInId = needsIdWrapper;
                }
                else if (operand.flavor == SPIRVAsmOperand::BuiltinVar)
                {
                    operand.type = CheckProperType(operand.type);
                    auto builtinVarKind =
                        spirvInfo->allEnums.lookup(SPIRVCoreGrammarInfo::QualifiedEnumName{
                            spirvInfo->operandKinds.lookup(UnownedStringSlice("BuiltIn")).value(),
                            operand.token.getContent()});
                    if (!builtinVarKind)
                    {
                        failed = true;
                        getSink()->diagnose(Diagnostics::SpirvUnableToResolveName{
                            .name = operand.token.getContent(),
                            .location = operand.token.loc});
                        return;
                    }
                    operand.knownValue = builtinVarKind.value();
                }
                else if (operand.flavor == SPIRVAsmOperand::Id)
                {
                    if (!definedIds.contains(operand.token.getName()))
                    {
                        failed = true;
                        getSink()->diagnose(Diagnostics::SpirvUndefinedId{
                            .id = operand.token.getContent(),
                            .location = operand.token.loc});
                        return;
                    }
                }
                if (operand.bitwiseOrWith.getCount() &&
                    operand.flavor != SPIRVAsmOperand::Literal &&
                    operand.flavor != SPIRVAsmOperand::NamedValue)
                {
                    failed = true;
                    getSink()->diagnose(
                        Diagnostics::SpirvNonConstantBitwiseOr{.location = operand.token.loc});
                }
                for (auto& o : operand.bitwiseOrWith)
                {
                    if (o.flavor != SPIRVAsmOperand::Literal &&
                        o.flavor != SPIRVAsmOperand::NamedValue)
                    {
                        failed = true;
                        getSink()->diagnose(
                            Diagnostics::SpirvNonConstantBitwiseOr{.location = operand.token.loc});
                    }
                    go(go, o);
                    operand.knownValue |= o.knownValue;
                }
            };

            check(check, inst.operands[operandIndex]);
        }

        if (opcode == SpvOpTypeArray || opcode == SpvOpTypeRuntimeArray ||
            opcode == SpvOpTypePointer)
        {
            getSink()->diagnose(Diagnostics::SpirvLayoutSensitiveTypeInAsm{
                .opcode = inst.opcode.token.getContent(),
                .location = inst.opcode.token.loc});
        }
    }

    if (failed)
        return CreateErrorExpr(expr);

    // Assign the type of this expression from the type of the last
    // instruction, otherwise void
    if (expr->insts.getCount())
    {
        // TODO: we trust that this is correct, but could should verify
        const auto lastOperands = expr->insts.getLast().operands;
        if (lastOperands.getCount() >= 2 && lastOperands[0].flavor == SPIRVAsmOperand::SlangType &&
            lastOperands[1].flavor == SPIRVAsmOperand::ResultMarker)
        {
            expr->type = lastOperands[0].type.type;
        }
    }
    if (!expr->type)
        expr->type = m_astBuilder->getVoidType();

    return expr;
}
} // namespace Slang
