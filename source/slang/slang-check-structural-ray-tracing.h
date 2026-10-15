// slang-check-structural-ray-tracing.h
#pragma once

#include "compiler-core/slang-source-loc.h"
#include "core/slang-short-list.h"

namespace Slang
{
class Type;
class VarDeclBase;

/// Records a source use to check for restrictions on ray-tracing stage and stage-input types.
///
/// These restrictions concern shader implementations, such as SkyMiss implementing rt::IMissShader,
/// and input views, such as rt::MissInput<C>. This record does not mean the type is already known
/// to be one of those: ordinary types also reach the checker and pass these restrictions.
///
/// Checking whether a type implements a shader interface can require inheritance information
/// that is unfinished when the use is first encountered. During module checking, these records
/// wait in SharedSemanticsContext until declaration and body checking finishes; checking an
/// already-complete module can process a record immediately.
///
/// Each record refers to existing declarations or checked types. In particular, a variable keeps
/// its declaration so the later check can distinguish `let copy = input` from an uninitialized
/// input local. The examples below use C for a valid stage context and MyMiss for a miss shader.
struct RayTracingStageOrInputUse
{
    /// Describes how the source uses a type and which fields the check needs.
    /// Any kind can contain an ordinary type that passes the check; the kind alone does not mean
    /// that a use is invalid or that its type is a ray-tracing type.
    enum class Kind
    {
        /// Checks a function result, property type, or resolved non-constructor call result.
        ///
        /// Consider `rt::MissInput<C> makeInput();`. The function promises to return a stage
        /// view, which this API does not allow. A call to `makeDefault<rt::MissInput<C>>()`
        /// needs the same check on its specialized result even if the function declares T as
        /// its return type. The check also finds restricted stage/input types inside containers.
        ///
        /// Uses `type` and `location`. Variables and individual references to existing inputs
        /// are not recorded as Value uses; copying an existing view remains allowed.
        Value,

        /// Checks a parameter, local variable, global variable, or field declaration.
        ///
        /// Consider `void helper(in rt::MissInput<C> input) { let copy = input; }`.
        /// Both declarations are allowed: input receives the invocation view, and copy is
        /// initialized from it. An uninitialized local, a static local, a global, or a field
        /// cannot supply that state. The exceptions for `in` parameters and initialized locals
        /// apply to direct input types, not containers such as Box<rt::MissInput<C>>.
        ///
        /// Uses `variable`. The check reads its type, initializer, modifiers, scope, and source
        /// location from the declaration after normal checking, rather than saving copies here.
        Variable,

        /// Checks an expression that creates a value through a constructor or initialization.
        ///
        /// Consider `rt::MissInput<C>()` or `rt::MissInput<C> fresh = {};`. Neither can create
        /// the invocation state supplied by a stage adapter. This also covers implicit
        /// constructor conversions and default initialization supplied by `-zero-initialize`.
        /// Normal overload resolution decides the expression's meaning first:
        /// `rt::MissInput<C>(input)` is a same-type copy and is not a Construction use.
        ///
        /// Uses `type` and `location` from the checked constructor, initializer-list, or
        /// default-construction expression. These uses produce the construction diagnostic.
        Construction,

        /// Checks type arguments supplied to a generic call or generic type application.
        ///
        /// Consider `void make<T>() { T local = T(); }` and `make<rt::MissInput<C>>()`.
        /// The generic body was checked using T, so the call must reject an input argument
        /// that would let the body manufacture invocation state after specialization. A type
        /// application such as Factory<rt::MissInput<C>> needs this check even if Factory has
        /// no fields: its methods could still construct T.
        ///
        /// Uses `genericArguments`, checking direct stage/input types and concrete pack
        /// elements. `genericApplicationType`, when supplied, identifies type applications
        /// allowed to carry those arguments, such as rt::MissShaderList<MyMiss> or
        /// Optional<rt::MissInput<C>>. This is
        /// separate from the recursive field/container check used by Value and Variable.
        GenericArguments,
    };

    /// Retains one checked generic type argument and the place to report an invalid use.
    /// Value arguments and conformance witnesses are not stored in this list.
    struct Argument
    {
        /// The checked argument type, which can itself be a concrete type pack.
        Type* type = nullptr;

        /// The argument's source location for an explicit type application. Call substitutions,
        /// including inferred arguments, use the callee's location instead.
        SourceLoc location;
    };

    /// Selects the check and its active fields. Fields unused by that kind keep their defaults.
    Kind kind = Kind::Value;

    /// Required for Variable; points to the existing AST declaration owned by the compilation.
    /// Its checked type and initializer remain the source of truth for the deferred check.
    VarDeclBase* variable = nullptr;

    /// Required for Value and Construction; the checked result or constructed type.
    Type* type = nullptr;

    /// Diagnostic location for Value and Construction. Declaration-result checks use the type's
    /// location (or the declaration's when absent); expression checks use the expression or
    /// constructor callee. Variable uses obtain their location directly from `variable`.
    SourceLoc location;

    /// Optional for GenericArguments; the resulting type of a generic type application.
    /// The checker uses it to recognize stage/input applications and builtin shader lists that
    /// may carry stage/input arguments. For example, rt::MissShaderList<MyMiss> intentionally
    /// lists a shader type. Generic function calls leave this null; it is not their return type.
    Type* genericApplicationType = nullptr;

    /// Type arguments to inspect for GenericArguments, with locations retained before checking
    /// replaces the original generic-application syntax. All other kinds leave this empty.
    ShortList<Argument, 4> genericArguments;
};

} // namespace Slang
