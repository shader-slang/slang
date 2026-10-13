// slang-check.h
#pragma once

// This file provides the public interface to the semantic
// checking infrastructure in `slang-check-*`.

#include "slang-syntax.h"

namespace Slang
{
class DiagnosticSink;
class EntryPoint;
class Linkage;
class Module;
class ShaderCompiler;
class ShaderLinkInfo;
class ShaderSymbol;

class TranslationUnitRequest;

constexpr UInt kMaxTypeNestingDepth = 128;

bool isGlobalShaderParameter(VarDeclBase* decl);
bool isFromCoreModule(Decl* decl);

void registerBuiltinDecl(SharedASTBuilder* sharedASTBuilder, Decl* decl);
void registerBuiltinDecl(ASTBuilder* astBuilder, Decl* decl);

void registerBuiltinDecls(Session* session, Decl* decl);

void collectBuiltinDeclsThatNeedRegistration(ModuleDecl* moduleDecl, List<Decl*>& outDecls);

Type* unwrapArrayType(Type* type);
Type* unwrapModifiedType(Type* type);

OrderedDictionary<GenericTypeParamDeclBase*, List<Type*>> getCanonicalGenericConstraints(
    ASTBuilder* builder,
    DeclRef<ContainerDecl> genericDecl);
OrderedDictionary<Type*, List<Type*>> getCanonicalGenericConstraints2(
    ASTBuilder* builder,
    DeclRef<ContainerDecl> genericDecl);

/// Describes how one semantic operation uses a property or subscript's abstract storage.
enum class AbstractStorageUse
{
    /// The consumer evaluates the expression but does not materialize its result.
    DiscardResult,
    /// The consumer needs the complete value.
    ReadValue,
    /// The consumer replaces the complete value without reading its old value.
    WriteValue,
    /// The consumer reads the complete value and may replace it.
    ReadWriteValue,
    /// The consumer borrows the complete value without modifying it.
    BorrowValue,
    /// The consumer requires a stable address and cannot use getter/setter writeback.
    RequireAddress,
    /// The consumer writes through a conversion or projection without reading the old value.
    WriteProjectedValue,
    /// The consumer reads and writes through a conversion or projection.
    ModifyProjectedValue,
    /// The consumer borrows through a conversion or projection without modifying it.
    BorrowProjectedValue,
};

/// Describes one operation in the checked path from an expression to abstract storage.
enum class AbstractStorageAccessPathKind
{
    /// A property or subscript whose accessor is selected when its use is known.
    DirectStorage,
    /// An operation whose result can preserve a storage-sensitive transformation of its operand.
    Projection,
    /// An operation that always consumes its operand in one fixed way.
    FixedOperandUse,
    /// A concrete value temporarily opened from an existential value.
    OpenedExistential,
    /// Two accesses evaluated in source order, one of which supplies the expression's storage.
    Sequence,
    /// A completed assignment whose result is the destination l-value.
    AssignmentResult,
};

/// Identifies which access in a sequence supplies the enclosing expression's storage.
enum class AbstractStorageSequenceResult
{
    FirstAccess,
    SecondAccess,
};

/// Describes how a checked projection maps a write to its abstract-storage operand.
enum class AbstractStorageProjectionMode
{
    /// The projection can update a complete value and write that value back.
    ///
    /// Fields, fixed swizzles, and a retained element of an existing swizzle use this mode. They do
    /// not require the underlying property or subscript to provide an address.
    ReadModifyWrite,
    /// Applying a write requires the operand's direct address.
    ///
    /// This mode is used when the projection cannot be represented in the deferred complete
    /// value that getter/setter writeback operates on.
    RequireAddressForWrite,
};

/// Represents how `checkAbstractStorageUse()` maps a requested use through one checked expression
/// operation.
///
/// Consider `object.property.xy`. A `DirectStorage` node represents the selected property and its
/// receiver. A `Projection` node represents read-modify-write through the complete property value
/// before a setter writes that value back. `checkAbstractStorageUse()` follows these nodes to select
/// and validate the required accessor calls.
///
/// `AbstractStorageUse` represents the consumer's requested operation, such as reading a value or
/// requiring an address. `AbstractStorageAccessPath` maps that request through checked expression
/// operations. When the path reaches `DirectStorage`, `getAbstractStorageAccessPlan()` selects the
/// concrete accessor calls represented by an `AbstractStorageAccessPlan`, and
/// `_checkAbstractStorageAccessPlan()` validates their receivers.
/// `AbstractStorageProjectionMode` indicates whether a write can flow through one projection by
/// updating a complete value or requires a direct address. The checked `IndexExpr` stores the
/// index-specific information used during lowering; that information is not part of this semantic
/// path.
///
/// The `make*` functions omit semantically empty paths and reuse a base path for identity
/// operations. When a new node is needed, those functions allocate it in `astBuilder`'s arena. An
/// `Expr::abstractStorageAccess` value may refer only to paths owned by the expression's builder or
/// one of its ancestors, so every path remains valid for the expression's lifetime. The
/// kind-specific payload is available only through accessors that assert the corresponding
/// `getKind()` result.
class AbstractStorageAccessPath
{
public:
    /// Records the selected property or subscript at the root of an access path.
    ///
    /// The receiver, specialized declaration reference, and source location are retained until
    /// `checkAbstractStorageUse()` supplies an `AbstractStorageUse` and selects an access plan.
    /// `getAbstractStorageAccessPlan()` derives accessor-specific parameter information from that
    /// declaration reference rather than caching a second classification.
    static AbstractStorageAccess makeDirectStorage(
        ASTBuilder* astBuilder,
        Expr* receiver,
        DeclRef<ContainerDecl> storageDeclRef,
        SourceLoc location);

    /// Records how one checked projection transforms a use of its operand.
    ///
    /// A field or fixed swizzle normally supports read-modify-write through its complete operand.
    /// A projection that cannot be represented in a complete value requires a direct address.
    /// A null base remains null without allocating a semantically invisible identity node.
    static AbstractStorageAccess makeProjection(
        ASTBuilder* astBuilder,
        AbstractStorageAccess baseAccess,
        AbstractStorageProjectionMode mode);

    /// Records an operation whose operand use is independent of the result's use.
    ///
    /// For example, writing through `*pointer` still reads `pointer`; it does not write the pointer
    /// expression itself. The enclosing use is therefore replaced by `operandUse` before following
    /// `operandAccess`. A null operand remains null.
    static AbstractStorageAccess makeFixedOperandUse(
        ASTBuilder* astBuilder,
        AbstractStorageAccess operandAccess,
        AbstractStorageUse operandUse);

    /// Records a concrete value temporarily opened from `originalExpression`.
    ///
    /// `openExistential()` records the initial read on the stabilizing `LetExpr`. Lowering's
    /// `assign()` repackages and writes a later mutation back to the original existential value,
    /// while `checkAbstractStorageUse()` rejects an address request because the opened value has no
    /// stable address in the original expression.
    static AbstractStorageAccess makeOpenedExistential(
        ASTBuilder* astBuilder,
        Expr* originalExpression);

    /// Records two accesses evaluated in source order and identifies the storage result.
    ///
    /// `_setLocalBindingStorageAccess()` records a `LetExpr` initializer first and its body second,
    /// with the body as the result governed by the enclosing use.
    /// `SemanticsVisitor::CheckSimpleSubscriptExpr()` records an indexed l-value's projected base
    /// first and its index second, with the base as the storage result.
    /// `checkAbstractStorageUse()` consumes the non-result access as `ReadValue`. An absent access
    /// is removed while preserving those uses.
    static AbstractStorageAccess makeSequence(
        ASTBuilder* astBuilder,
        AbstractStorageAccess firstAccess,
        AbstractStorageAccess secondAccess,
        AbstractStorageSequenceResult result);

    /// Records the l-value produced by a completed assignment.
    ///
    /// Consuming the result applies the requested use to `destinationAccess`. Discarding the result
    /// performs no additional access because the assignment has already completed its write. A
    /// null destination remains null because ordinary variables need no accessor validation.
    static AbstractStorageAccess makeAssignmentResult(
        ASTBuilder* astBuilder,
        AbstractStorageAccess destinationAccess);

    AbstractStorageAccessPathKind getKind() const { return m_kind; }

    Expr* getStorageReceiver() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::DirectStorage);
        return m_expression;
    }

    DeclRef<ContainerDecl> getStorageDeclRef() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::DirectStorage);
        return m_storageDeclRef;
    }

    SourceLoc getStorageLocation() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::DirectStorage);
        return m_location;
    }

    AbstractStorageAccess getOperandAccess() const
    {
        switch (m_kind)
        {
        case AbstractStorageAccessPathKind::Projection:
        case AbstractStorageAccessPathKind::FixedOperandUse:
        case AbstractStorageAccessPathKind::AssignmentResult:
            break;
        default:
            SLANG_UNEXPECTED("abstract-storage access has no operand");
        }
        return m_firstAccess;
    }

    AbstractStorageProjectionMode getProjectionMode() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::Projection);
        return m_projectionMode;
    }

    AbstractStorageUse getFixedOperandUse() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::FixedOperandUse);
        return m_fixedOperandUse;
    }

    Expr* getOpenedExistentialOriginalExpression() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::OpenedExistential);
        return m_expression;
    }

    AbstractStorageAccess getFirstAccess() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::Sequence);
        return m_firstAccess;
    }

    AbstractStorageAccess getSecondAccess() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::Sequence);
        return m_secondAccess;
    }

    AbstractStorageSequenceResult getSequenceResult() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPathKind::Sequence);
        return m_sequenceResult;
    }

private:
    static AbstractStorageAccessPath* _allocate(ASTBuilder* astBuilder);

    AbstractStorageAccessPathKind m_kind = AbstractStorageAccessPathKind::DirectStorage;
    AbstractStorageAccess m_firstAccess = nullptr;
    AbstractStorageAccess m_secondAccess = nullptr;
    Expr* m_expression = nullptr;
    DeclRef<ContainerDecl> m_storageDeclRef;
    SourceLoc m_location;
    AbstractStorageProjectionMode m_projectionMode =
        AbstractStorageProjectionMode::ReadModifyWrite;
    AbstractStorageUse m_fixedOperandUse = AbstractStorageUse::ReadValue;
    AbstractStorageSequenceResult m_sequenceResult =
        AbstractStorageSequenceResult::SecondAccess;
};

/// Returns how an argument consumes abstract storage for `mode`.
AbstractStorageUse getAbstractStorageUseForParameter(ParamPassingMode mode);

/// Returns how a setter's implicit `newValue` parameter consumes the assigned expression.
///
/// A noncopyable `BorrowIn` value cannot be materialized and then moved into the destination, so
/// that case requires the assigned expression's existing address. Copyable and writable inputs
/// follow the same rules as ordinary parameters.
AbstractStorageUse getAbstractStorageUseForSetterNewValue(ParamInfo const& paramInfo);

/// Returns how invoking `accessorDeclRef` consumes its receiver.
///
/// A mutating `ref` accessor may return an address into its receiver, so that receiver must have
/// stable storage; copying it into a temporary and writing it back before the address is used would
/// leave the returned address referring to the temporary.
AbstractStorageUse getAbstractStorageUseForAccessorReceiver(
    DeclRef<AccessorDecl> accessorDeclRef,
    ParamPassingMode mode);

/// Returns how a reversible implicit-cast operand is used for a writable parameter.
AbstractStorageUse getAbstractStorageUseForImplicitCastOperand(ParamPassingMode mode);

/// Returns how forming an explicit reference consumes its operand expression.
///
/// A writable reference aliases the operand's address; a read-only reference materializes its
/// value.
AbstractStorageUse getAbstractStorageUseForExplicitReference(QualType referenceType);

/// Describes the accessor calls needed to perform one abstract-storage use.
enum class AbstractStorageAccessPlanKind
{
    Invalid,
    ReadValue,
    WriteValue,
    UseAddress,
    ReadThenWriteBack,
};

/// Describes why no accessor plan can implement an abstract-storage use.
enum class AbstractStorageAccessFailure
{
    CannotRead,
    CannotWrite,
    CannotProvideAddress,
};

/// Represents the kind-specific accessors selected for one abstract-storage use.
///
/// The factories establish which accessor payloads are valid for each `getKind()` result. The
/// corresponding accessors are available only through getters that assert the expected kind.
class AbstractStorageAccessPlan
{
public:
    static AbstractStorageAccessPlan makeInvalid(AbstractStorageAccessFailure failure)
    {
        return AbstractStorageAccessPlan(failure);
    }

    static AbstractStorageAccessPlan makeReadValue(DeclRef<AccessorDecl> readAccessor)
    {
        SLANG_RELEASE_ASSERT(readAccessor);
        return AbstractStorageAccessPlan(
            AbstractStorageAccessPlanKind::ReadValue,
            readAccessor,
            DeclRef<SetterDecl>(),
            DeclRef<RefAccessorDecl>());
    }

    static AbstractStorageAccessPlan makeWriteValue(DeclRef<SetterDecl> writeAccessor)
    {
        SLANG_RELEASE_ASSERT(writeAccessor);
        return AbstractStorageAccessPlan(
            AbstractStorageAccessPlanKind::WriteValue,
            DeclRef<AccessorDecl>(),
            writeAccessor,
            DeclRef<RefAccessorDecl>());
    }

    static AbstractStorageAccessPlan makeUseAddress(DeclRef<RefAccessorDecl> addressAccessor)
    {
        SLANG_RELEASE_ASSERT(addressAccessor);
        return AbstractStorageAccessPlan(
            AbstractStorageAccessPlanKind::UseAddress,
            DeclRef<AccessorDecl>(),
            DeclRef<SetterDecl>(),
            addressAccessor);
    }

    static AbstractStorageAccessPlan makeReadThenWriteBack(
        DeclRef<AccessorDecl> readAccessor,
        DeclRef<SetterDecl> writeAccessor)
    {
        SLANG_RELEASE_ASSERT(readAccessor);
        SLANG_RELEASE_ASSERT(writeAccessor);
        return AbstractStorageAccessPlan(
            AbstractStorageAccessPlanKind::ReadThenWriteBack,
            readAccessor,
            writeAccessor,
            DeclRef<RefAccessorDecl>());
    }

    AbstractStorageAccessPlanKind getKind() const { return m_kind; }

    AbstractStorageAccessFailure getFailure() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPlanKind::Invalid);
        return m_failure;
    }

    DeclRef<AccessorDecl> getReadAccessor() const
    {
        switch (m_kind)
        {
        case AbstractStorageAccessPlanKind::ReadValue:
        case AbstractStorageAccessPlanKind::ReadThenWriteBack:
            break;
        default:
            SLANG_UNEXPECTED("abstract-storage plan has no read accessor");
        }
        return m_readAccessor;
    }

    DeclRef<SetterDecl> getWriteAccessor() const
    {
        switch (m_kind)
        {
        case AbstractStorageAccessPlanKind::WriteValue:
        case AbstractStorageAccessPlanKind::ReadThenWriteBack:
            break;
        default:
            SLANG_UNEXPECTED("abstract-storage plan has no write accessor");
        }
        return m_writeAccessor;
    }

    DeclRef<RefAccessorDecl> getAddressAccessor() const
    {
        SLANG_RELEASE_ASSERT(m_kind == AbstractStorageAccessPlanKind::UseAddress);
        return m_addressAccessor;
    }

private:
    explicit AbstractStorageAccessPlan(AbstractStorageAccessFailure failure)
        : m_failure(failure)
    {
    }

    AbstractStorageAccessPlan(
        AbstractStorageAccessPlanKind kind,
        DeclRef<AccessorDecl> readAccessor,
        DeclRef<SetterDecl> writeAccessor,
        DeclRef<RefAccessorDecl> addressAccessor)
        : m_kind(kind)
        , m_readAccessor(readAccessor)
        , m_writeAccessor(writeAccessor)
        , m_addressAccessor(addressAccessor)
    {
    }

    AbstractStorageAccessPlanKind m_kind = AbstractStorageAccessPlanKind::Invalid;
    DeclRef<AccessorDecl> m_readAccessor;
    DeclRef<SetterDecl> m_writeAccessor;
    DeclRef<RefAccessorDecl> m_addressAccessor;
    AbstractStorageAccessFailure m_failure = AbstractStorageAccessFailure::CannotRead;
};

/// Chooses the accessor calls that implement `use` for `storageDeclRef`.
AbstractStorageAccessPlan getAbstractStorageAccessPlan(
    ASTBuilder* astBuilder,
    DeclRef<ContainerDecl> storageDeclRef,
    AbstractStorageUse use);

/// Returns how an enclosing use consumes the abstract storage behind one of its projections.
AbstractStorageUse getProjectedAbstractStorageUse(AbstractStorageUse use);

/// Returns the single implicit `newValue` parameter owned by `setterDeclRef`.
ParamInfo getSetterNewValueParamInfo(
    ASTBuilder* astBuilder,
    DeclRef<SetterDecl> setterDeclRef);

/// Collects declarations reached by `SemanticsDeclReferenceVisitor` from `node`.
///
/// In particular, that traversal ignores unresolved overload candidates and non-evaluated
/// static-member bases.
void collectReferencedDecls(SemanticsVisitor* context, NodeBase* node, HashSet<Decl*>& outDecls);

void registerAssociatedMethods(SemanticsVisitor* context, DeclRef<Decl> declRef);
} // namespace Slang
