// slang-ir-insts.h
#ifndef SLANG_IR_INSTS_H_INCLUDED
#define SLANG_IR_INSTS_H_INCLUDED

// This file extends the core definitions in `ir.h`
// with a wider variety of concrete instructions,
// and a "builder" abstraction.
//
// TODO: the builder probably needs its own file.

#include "slang-compiler.h"
#include "slang-ir.h"
#include "slang-syntax.h"
#include "slang-type-layout.h"

namespace Slang {

class Decl;

struct IRDecoration : IRInst
{
    IR_PARENT_ISA(Decoration)

    IRDecoration* getNextDecoration()
    {
        return as<IRDecoration>(getNextInst());
    }
};

// Associates an IR-level decoration with a source declaration
// in the high-level AST, that can be used to extract
// additional information that informs code emission.
struct IRHighLevelDeclDecoration : IRDecoration
{
    enum { kOp = kIROp_HighLevelDeclDecoration };
    IR_LEAF_ISA(HighLevelDeclDecoration)

    IRPtrLit* getDeclOperand() { return cast<IRPtrLit>(getOperand(0)); }
    Decl* getDecl() { return (Decl*) getDeclOperand()->getValue(); }
};

enum IRLoopControl
{
    kIRLoopControl_Unroll,
};

struct IRLoopControlDecoration : IRDecoration
{
    enum { kOp = kIROp_LoopControlDecoration };
    IR_LEAF_ISA(LoopControlDecoration)

    IRConstant* getModeOperand() { return cast<IRConstant>(getOperand(0)); }

    IRLoopControl getMode()
    {
        return IRLoopControl(getModeOperand()->value.intVal);
    }
};


struct IRTargetSpecificDecoration : IRDecoration
{
    IR_PARENT_ISA(TargetSpecificDecoration)

    IRStringLit* getTargetNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getTargetName()
    {
        return getTargetNameOperand()->getStringSlice();
    }
};

struct IRTargetDecoration : IRTargetSpecificDecoration
{
    enum { kOp = kIROp_TargetDecoration };
    IR_LEAF_ISA(TargetDecoration)
};

struct IRTargetIntrinsicDecoration : IRTargetSpecificDecoration
{
    enum { kOp = kIROp_TargetIntrinsicDecoration };
    IR_LEAF_ISA(TargetIntrinsicDecoration)

    IRStringLit* getDefinitionOperand() { return cast<IRStringLit>(getOperand(1)); }

    UnownedStringSlice getDefinition()
    {
        return getDefinitionOperand()->getStringSlice();
    }
};

struct IRGLSLOuterArrayDecoration : IRDecoration
{
    enum { kOp = kIROp_GLSLOuterArrayDecoration };
    IR_LEAF_ISA(GLSLOuterArrayDecoration)

    IRStringLit* getOuterArraynameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getOuterArrayName()
    {
        return getOuterArraynameOperand()->getStringSlice();
    }
};

enum class IRInterpolationMode
{
    Linear,
    NoPerspective,
    NoInterpolation,

    Centroid,
    Sample,
};

struct IRInterpolationModeDecoration : IRDecoration
{
    enum { kOp = kIROp_InterpolationModeDecoration };
    IR_LEAF_ISA(InterpolationModeDecoration)

    IRConstant* getModeOperand() { return cast<IRConstant>(getOperand(0)); }

    IRInterpolationMode getMode()
    {
        return IRInterpolationMode(getModeOperand()->value.intVal);
    }
};

/// A decoration that provides a desired name to be used
/// in conjunction with the given instruction. Back-end
/// code generation may use this to help derive symbol
/// names, emit debug information, etc.
struct IRNameHintDecoration : IRDecoration
{
    enum { kOp = kIROp_NameHintDecoration };
    IR_LEAF_ISA(NameHintDecoration)

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getName()
    {
        return getNameOperand()->getStringSlice();
    }
};

#define IR_SIMPLE_DECORATION(NAME)      \
    struct IR##NAME : IRDecoration      \
    {                                   \
        enum { kOp = kIROp_##NAME };    \
    IR_LEAF_ISA(NAME)                   \
    };                                  \
    /**/

/// A decoration that indicates that a variable represents
/// a vulkan ray payload, and should have a location assigned
/// to it.
IR_SIMPLE_DECORATION(VulkanRayPayloadDecoration)

/// A decoration that indicates that a variable represents
/// a vulkan callable shader payload, and should have a location assigned
/// to it.
IR_SIMPLE_DECORATION(VulkanCallablePayloadDecoration)

/// A decoration that indicates that a variable represents
/// vulkan hit attributes, and should have a location assigned
/// to it.
IR_SIMPLE_DECORATION(VulkanHitAttributesDecoration)

struct IRRequireGLSLVersionDecoration : IRDecoration
{
    enum { kOp = kIROp_RequireGLSLVersionDecoration };
    IR_LEAF_ISA(RequireGLSLVersionDecoration)

    IRConstant* getLanguageVersionOperand() { return cast<IRConstant>(getOperand(0)); }

    Int getLanguageVersion()
    {
        return Int(getLanguageVersionOperand()->value.intVal);
    }
};

struct IRRequireGLSLExtensionDecoration : IRDecoration
{
    enum { kOp = kIROp_RequireGLSLExtensionDecoration };
    IR_LEAF_ISA(RequireGLSLExtensionDecoration)

    IRStringLit* getExtensionNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getExtensionName()
    {
        return getExtensionNameOperand()->getStringSlice();
    }
};

IR_SIMPLE_DECORATION(ReadNoneDecoration)
IR_SIMPLE_DECORATION(EarlyDepthStencilDecoration)
IR_SIMPLE_DECORATION(GloballyCoherentDecoration)
IR_SIMPLE_DECORATION(PreciseDecoration)


struct IROutputControlPointsDecoration : IRDecoration
{
    enum { kOp = kIROp_OutputControlPointsDecoration };
    IR_LEAF_ISA(OutputControlPointsDecoration)

    IRIntLit* getControlPointCount() { return cast<IRIntLit>(getOperand(0)); }
};

struct IROutputTopologyDecoration : IRDecoration
{
    enum { kOp = kIROp_OutputTopologyDecoration };
    IR_LEAF_ISA(OutputTopologyDecoration)

    IRStringLit* getTopology() { return cast<IRStringLit>(getOperand(0)); }
};

struct IRPartitioningDecoration : IRDecoration
{
    enum { kOp = kIROp_PartitioningDecoration };
    IR_LEAF_ISA(PartitioningDecoration)

    IRStringLit* getPartitioning() { return cast<IRStringLit>(getOperand(0)); }
};

struct IRDomainDecoration : IRDecoration
{
    enum { kOp = kIROp_DomainDecoration };
    IR_LEAF_ISA(DomainDecoration)

    IRStringLit* getDomain() { return cast<IRStringLit>(getOperand(0)); }
};

struct IRMaxVertexCountDecoration : IRDecoration
{
    enum { kOp = kIROp_MaxVertexCountDecoration };
    IR_LEAF_ISA(MaxVertexCountDecoration)

    IRIntLit* getCount() { return cast<IRIntLit>(getOperand(0)); }
};

struct IRInstanceDecoration : IRDecoration
{
    enum { kOp = kIROp_InstanceDecoration };
    IR_LEAF_ISA(InstanceDecoration)

    IRIntLit* getCount() { return cast<IRIntLit>(getOperand(0)); }
};

struct IRNumThreadsDecoration : IRDecoration
{
    enum { kOp = kIROp_NumThreadsDecoration };
    IR_LEAF_ISA(NumThreadsDecoration)

    IRIntLit* getX() { return cast<IRIntLit>(getOperand(0)); }
    IRIntLit* getY() { return cast<IRIntLit>(getOperand(1)); }
    IRIntLit* getZ() { return cast<IRIntLit>(getOperand(2)); }
};

struct IREntryPointDecoration : IRDecoration
{
    enum { kOp = kIROp_EntryPointDecoration };
    IR_LEAF_ISA(EntryPointDecoration)

    IRIntLit* getProfileInst() { return cast<IRIntLit>(getOperand(0)); }
    Profile getProfile() { return Profile(Profile::RawVal(GetIntVal(getProfileInst()))); }

    IRStringLit* getName()  { return cast<IRStringLit>(getOperand(1)); }
};

struct IRGeometryInputPrimitiveTypeDecoration: IRDecoration
{
    IR_PARENT_ISA(GeometryInputPrimitiveTypeDecoration)
};

IR_SIMPLE_DECORATION(PointInputPrimitiveTypeDecoration)
IR_SIMPLE_DECORATION(LineInputPrimitiveTypeDecoration)
IR_SIMPLE_DECORATION(TriangleInputPrimitiveTypeDecoration)
IR_SIMPLE_DECORATION(LineAdjInputPrimitiveTypeDecoration)
IR_SIMPLE_DECORATION(TriangleAdjInputPrimitiveTypeDecoration)

    /// This is a bit of a hack. The problem is that when GLSL legalization takes place
    /// the parameters from the entry point are globalized *and* potentially split
    /// So even if we did copy a suitable decoration onto the globalized parameters,
    /// it would potentially output multiple times without extra logic.
    /// Using this decoration we can copy the StreamOut type to the entry point, and then
    /// emit as part of entry point attribute emitting.  
struct IRStreamOutputTypeDecoration : IRDecoration
{
    enum { kOp = kIROp_StreamOutputTypeDecoration };
    IR_LEAF_ISA(StreamOutputTypeDecoration)

    IRHLSLStreamOutputType* getStreamType() { return cast<IRHLSLStreamOutputType>(getOperand(0)); }
};

    /// A decoration that marks a value as having linkage. 
    /// A value with linkage is either exported from its module,
    /// or will have a definition imported from another module.
    /// In either case, it requires a mangled name to use when
    /// matching imports and exports.
struct IRLinkageDecoration : IRDecoration
{
    IR_PARENT_ISA(LinkageDecoration)

    IRStringLit* getMangledNameOperand() { return cast<IRStringLit>(getOperand(0)); }

    UnownedStringSlice getMangledName()
    {
        return getMangledNameOperand()->getStringSlice();
    }
};

struct IRImportDecoration : IRLinkageDecoration
{
    enum { kOp = kIROp_ImportDecoration };
    IR_LEAF_ISA(ImportDecoration)
};

struct IRExportDecoration : IRLinkageDecoration
{
    enum { kOp = kIROp_ExportDecoration };
    IR_LEAF_ISA(ExportDecoration)
};

struct IRFormatDecoration : IRDecoration
{
    enum { kOp = kIROp_FormatDecoration };
    IR_LEAF_ISA(FormatDecoration)

    IRConstant* getFormatOperand() { return cast<IRConstant>(getOperand(0)); }

    ImageFormat getFormat()
    {
        return ImageFormat(getFormatOperand()->value.intVal);
    }
};

// An instruction that specializes another IR value
// (representing a generic) to a particular set of generic arguments 
// (instructions representing types, witness tables, etc.)
struct IRSpecialize : IRInst
{
    // The "base" for the call is the generic to be specialized
    IRUse base;
    IRInst* getBase() { return getOperand(0); }

    // after the generic value come the arguments
    UInt getArgCount() { return getOperandCount() - 1; }
    IRInst* getArg(UInt index) { return getOperand(index + 1); }

    IR_LEAF_ISA(Specialize)
};

// An instruction that looks up the implementation
// of an interface operation identified by `requirementDeclRef`
// in the witness table `witnessTable` which should
// hold the conformance information for a specific type.
struct IRLookupWitnessMethod : IRInst
{
    IRUse witnessTable;
    IRUse requirementKey;

    IRInst* getWitnessTable() { return witnessTable.get(); }
    IRInst* getRequirementKey() { return requirementKey.get(); }
};

struct IRLookupWitnessTable : IRInst
{
    IRUse sourceType;
    IRUse interfaceType;
};

// Layout decorations

    /// A decoration that marks a field key as having been associated
    /// with a particular simple semantic (e.g., `COLOR` or `SV_Position`,
    /// but not a `register` semantic).
    ///
    /// This is currently needed so that we can round-trip HLSL `struct`
    /// types that get used for varying input/output. This is an unfortunate
    /// case where some amount of "layout" information can't just come
    /// in via the `TypeLayout` part of things.
    ///
struct IRSemanticDecoration : public IRDecoration
{
    IR_LEAF_ISA(SemanticDecoration)

    IRStringLit* getSemanticNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getSemanticName() { return getSemanticNameOperand()->getStringSlice(); }

    IRIntLit* getSemanticIndexOperand() { return cast<IRIntLit>(getOperand(1)); }
    int getSemanticIndex() { return int(GetIntVal(getSemanticIndexOperand())); }
};

    /// An attribute that can be attached to another instruction as an operand.
    ///
    /// Attributes serve a similar role to decorations, in that both are ways
    /// to attach additional information to an instruction, where the operand
    /// of the attribute/decoration identifies the purpose of the additional
    /// information.
    ///
    /// The key difference between decorations and attributes is that decorations
    /// are stored as children of an instruction (in terms of the ownership
    /// hierarchy), while attributes are referenced as operands.
    ///
    /// The key benefit of having attributes be operands is that they must
    /// be present at the time an instruction is created, which means that
    /// they can affect the conceptual value/identity of an instruction
    /// in cases where we deduplicate/hash instructions by value.
    ///
struct IRAttr : public IRInst
{
    IR_PARENT_ISA(Attr);
};

    /// An attribute that specifies layout information for a single resource kind.
struct IRLayoutResourceInfoAttr : public IRAttr
{
    IR_PARENT_ISA(LayoutResourceInfoAttr);

    IRIntLit* getResourceKindInst() { return cast<IRIntLit>(getOperand(0)); }
    LayoutResourceKind getResourceKind() { return LayoutResourceKind(GetIntVal(getResourceKindInst())); }
};

    /// An attribute that specifies offset information for a single resource kind.
    ///
    /// This operation can appear as `varOffset(kind, offset)` or
    /// `varOffset(kind, offset, space)`. The latter form is only
    /// used when `space` is non-zero.
    ///
struct IRVarOffsetAttr : public IRLayoutResourceInfoAttr
{
    IR_LEAF_ISA(VarOffsetAttr);

    IRIntLit* getOffsetInst() { return cast<IRIntLit>(getOperand(1)); }
    UInt getOffset() { return UInt(GetIntVal(getOffsetInst())); }

    IRIntLit* getSpaceInst()
    {
        if(getOperandCount() > 2)
            return cast<IRIntLit>(getOperand(2));
        return nullptr;
    }

    UInt getSpace()
    {
        if(auto spaceInst = getSpaceInst())
            return UInt(GetIntVal(spaceInst));
        return 0;
    }
};

    /// An attribute that specifies size information for a single resource kind.
struct IRTypeSizeAttr : public IRLayoutResourceInfoAttr
{
    IR_LEAF_ISA(TypeSizeAttr);

    IRIntLit* getSizeInst() { return cast<IRIntLit>(getOperand(1)); }
    LayoutSize getSize() { return LayoutSize::fromRaw(LayoutSize::RawValue(GetIntVal(getSizeInst()))); }
    size_t getFiniteSize() { return getSize().getFiniteValue(); }
};

// Layout

    /// Base type for instructions that represent layout information.
    ///
    /// Layout instructions are effectively just meta-data constants.
    ///
struct IRLayout : IRInst
{
    IR_PARENT_ISA(Layout)
};

struct IRVarLayout;

    /// An attribute to specify that a layout has another layout attached for "pending" data.
    ///
    /// "Pending" data refers to the parts of a type or variable that
    /// couldn't be laid out until the concrete types for existential
    /// type slots were filled in. The layout of pending data may not
    /// be contiguous with the layout of the original type/variable.
    ///
struct IRPendingLayoutAttr : IRAttr
{
    IR_LEAF_ISA(PendingLayoutAttr);

    IRLayout* getLayout() { return cast<IRLayout>(getOperand(0)); }
};

    /// Layout information for a type.
    ///
    /// The most important thing this instruction provides is the
    /// resource usage (aka "size") of the type for each of the
    /// resource kinds it consumes.
    ///
    /// Subtypes of `IRTypeLayout` will include additional type-specific
    /// operands or attributes. For example, a type layout for a
    /// `struct` type will include offset information for its fields.
    ///
struct IRTypeLayout : IRLayout
{
    IR_PARENT_ISA(TypeLayout);

        /// Find the attribute that stores offset information for `kind`.
        ///
        /// Returns null if no attribute is found, indicating that this
        /// type does not consume any resources of `kind`.
        ///
    IRTypeSizeAttr* findSizeAttr(LayoutResourceKind kind);

        /// Get all the attributes representing size information.
    IROperandList<IRTypeSizeAttr> getSizeAttrs();

        /// Unwrap any layers of array-ness and return the outer-most non-array type.
    IRTypeLayout* unwrapArray();

        /// Get the layout for pending data, if present.
    IRTypeLayout* getPendingDataTypeLayout();

        /// A builder for constructing `IRTypeLayout`s
    struct Builder
    {
            /// Begin building.
            ///
            /// The `irBuilder` will be used to construct the
            /// type layout and any additional instructions required.
            ///
        Builder(IRBuilder* irBuilder);

            /// Add `size` units of resource `kind` to the resource usage of this type.
        void addResourceUsage(
            LayoutResourceKind  kind,
            LayoutSize          size);

            /// Add the resource usage specified by `sizeAttr`.
        void addResourceUsage(IRTypeSizeAttr* sizeAttr);

            /// Add all resource usage from `typeLayout`.
        void addResourceUsageFrom(IRTypeLayout* typeLayout);

            /// Set the (optional) layout for pending data.
        void setPendingTypeLayout(
            IRTypeLayout* typeLayout)
        {
            m_pendingTypeLayout = typeLayout;
        }

            /// Build a type layout according to the information specified so far.
        IRTypeLayout* build();

    protected:
        // The following services are provided so that
        // subtypes of `IRTypeLayout` can provide their
        // own `Builder` subtypes that construct appropriate
        // layouts.

            /// Override to customize the opcode of the generated layout.
        virtual IROp getOp() { return kIROp_TypeLayoutBase; }

            /// Override to add additional operands to the generated layout.
        virtual void addOperandsImpl(List<IRInst*>&) {}

            /// Override to add additional attributes to the generated layout.
        virtual void addAttrsImpl(List<IRInst*>&) {}

            /// Use to access the underlying IR builder.
        IRBuilder* getIRBuilder() { return m_irBuilder; };

    private:
        void addOperands(List<IRInst*>&);
        void addAttrs(List<IRInst*>& ioOperands);

        IRBuilder* m_irBuilder = nullptr;
        IRTypeLayout* m_pendingTypeLayout = nullptr;

        struct ResInfo
        {
            LayoutResourceKind  kind = LayoutResourceKind::None;
            LayoutSize          size = 0;
        };
        ResInfo m_resInfos[SLANG_PARAMETER_CATEGORY_COUNT];
    };
};

    /// Type layout for parameter groups (constant buffers and parameter blocks)
struct IRParameterGroupTypeLayout : IRTypeLayout
{
private:
    typedef IRTypeLayout Super;

public:
    IR_LEAF_ISA(ParameterGroupTypeLayout)

    IRVarLayout* getContainerVarLayout()
    {
        return cast<IRVarLayout>(getOperand(0));
    }

    IRVarLayout* getElementVarLayout()
    {
        return cast<IRVarLayout>(getOperand(1));
    }

    // TODO: There shouldn't be a need for the IR to store an "offset" element type layout,
    // but there are just enough places that currently use that information so that removing
    // it would require some careful refactoring.
    //
    IRTypeLayout* getOffsetElementTypeLayout()
    {
        return cast<IRTypeLayout>(getOperand(2));
    }

        /// Specialized builder for parameter group type layouts.
    struct Builder : Super::Builder
    {
    public:
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {}

        void setContainerVarLayout(IRVarLayout* varLayout)
        {
            m_containerVarLayout = varLayout;
        }

        void setElementVarLayout(IRVarLayout* varLayout)
        {
            m_elementVarLayout = varLayout;
        }

        void setOffsetElementTypeLayout(IRTypeLayout* typeLayout)
        {
            m_offsetElementTypeLayout = typeLayout;
        }

        IRParameterGroupTypeLayout* build();

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_ParameterGroupTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRVarLayout* m_containerVarLayout;
        IRVarLayout* m_elementVarLayout;
        IRTypeLayout* m_offsetElementTypeLayout;
    };
};

    /// Specialized layout information for array types
struct IRArrayTypeLayout : IRTypeLayout
{
    typedef IRTypeLayout Super;

    IR_LEAF_ISA(ArrayTypeLayout)

    IRTypeLayout* getElementTypeLayout()
    {
        return cast<IRTypeLayout>(getOperand(0));
    }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, IRTypeLayout* elementTypeLayout)
            : Super::Builder(irBuilder)
            , m_elementTypeLayout(elementTypeLayout)
        {}

        IRArrayTypeLayout* build()
        {
            return cast<IRArrayTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_ArrayTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRTypeLayout* m_elementTypeLayout;
    };
};

    /// Specialized layout information for stream-output types
struct IRStreamOutputTypeLayout : IRTypeLayout
{
    typedef IRTypeLayout Super;

    IR_LEAF_ISA(StreamOutputTypeLayout)

    IRTypeLayout* getElementTypeLayout()
    {
        return cast<IRTypeLayout>(getOperand(0));
    }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, IRTypeLayout* elementTypeLayout)
            : Super::Builder(irBuilder)
            , m_elementTypeLayout(elementTypeLayout)
        {}

        IRArrayTypeLayout* build()
        {
            return cast<IRArrayTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_StreamOutputTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRTypeLayout* m_elementTypeLayout;
    };
};

    /// Specialized layout information for matrix types
struct IRMatrixTypeLayout : IRTypeLayout
{
    typedef IRTypeLayout Super;

    IR_LEAF_ISA(MatrixTypeLayout)

    MatrixLayoutMode getMode()
    {
        return MatrixLayoutMode(GetIntVal(cast<IRIntLit>(getOperand(0))));
    }

    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, MatrixLayoutMode mode);

        IRMatrixTypeLayout* build()
        {
            return cast<IRMatrixTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_MatrixTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRInst* m_modeInst = nullptr;
    };
};

    /// Attribute that specifies the layout for one field of a structure type.
struct IRStructFieldLayoutAttr : IRAttr
{
    IR_LEAF_ISA(StructFieldLayoutAttr)

    IRStructKey* getFieldKey()
    {
        return cast<IRStructKey>(getOperand(0));
    }

    IRVarLayout* getLayout()
    {
        return cast<IRVarLayout>(getOperand(1));
    }
};

    /// Specialized layout information for structure types.
struct IRStructTypeLayout : IRTypeLayout
{
    IR_LEAF_ISA(StructTypeLayout)

    typedef IRTypeLayout Super;

        /// Get all of the attributes that represent field layouts.
    IROperandList<IRStructFieldLayoutAttr> getFieldLayoutAttrs()
    {
        return findAttrs<IRStructFieldLayoutAttr>();
    }

        /// Get the number of fields for which layout information is stored.
    UInt getFieldCount()
    {
        return getFieldLayoutAttrs().getCount();
    }

        /// Get the layout information for a field by `index`
    IRVarLayout* getFieldLayout(UInt index)
    {
        return getFieldLayoutAttrs()[index]->getLayout();
    }

        /// Specialized builder for structure type layouts.
    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder)
            : Super::Builder(irBuilder)
        {}

        void addField(IRStructKey* key, IRVarLayout* layout)
        {
            FieldInfo info;
            info.key = key;
            info.layout = layout;
            m_fields.add(info);
        }

        IRStructTypeLayout* build()
        {
            return cast<IRStructTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_StructTypeLayout; }
        void addAttrsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        struct FieldInfo
        {
            IRStructKey* key;
            IRVarLayout* layout;
        };

        List<FieldInfo> m_fields;
    };
};

    /// Attribute that represents the layout for one case of a union type
struct IRCaseTypeLayoutAttr : IRAttr
{
    IR_LEAF_ISA(CaseTypeLayoutAttr);

    IRTypeLayout* getTypeLayout()
    {
        return cast<IRTypeLayout>(getOperand(0));
    }
};

    /// Specialized layout information for tagged union types
struct IRTaggedUnionTypeLayout : IRTypeLayout
{
    typedef IRTypeLayout Super;

    IR_LEAF_ISA(TaggedUnionTypeLayout)

        /// Get the (byte) offset of the tagged union's tag (aka "discriminator") field
    LayoutSize getTagOffset()
    {
        return LayoutSize::fromRaw(LayoutSize::RawValue(GetIntVal(cast<IRIntLit>(getOperand(0)))));
    }

        /// Get all the attributes representing layouts for the difference cases
    IROperandList<IRCaseTypeLayoutAttr> getCaseTypeLayoutAttrs()
    {
        return findAttrs<IRCaseTypeLayoutAttr>();
    }

        /// Get the number of cases for which layout information is stored
    UInt getCaseCount()
    {
        return getCaseTypeLayoutAttrs().getCount();
    }

        /// Get the layout information for the case at the given `index`
    IRTypeLayout* getCaseTypeLayout(UInt index)
    {
        return getCaseTypeLayoutAttrs()[index]->getTypeLayout();
    }

        /// Specialized builder for tagged union type layouts
    struct Builder : Super::Builder
    {
        Builder(IRBuilder* irBuilder, LayoutSize tagOffset);

        void addCaseTypeLayout(IRTypeLayout* typeLayout);

        IRTaggedUnionTypeLayout* build()
        {
            return cast<IRTaggedUnionTypeLayout>(Super::Builder::build());
        }

    protected:
        IROp getOp() SLANG_OVERRIDE { return kIROp_TaggedUnionTypeLayout; }
        void addOperandsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;
        void addAttrsImpl(List<IRInst*>& ioOperands) SLANG_OVERRIDE;

        IRInst* m_tagOffset = nullptr;
        List<IRAttr*> m_caseTypeLayoutAttrs;
    };
};

    /// Layout information for an entry point
struct IREntryPointLayout : IRLayout
{
    IR_LEAF_ISA(EntryPointLayout)

        /// Get the layout information for the entry point parameters.
        ///
        /// The parameters layout will either be a structure type layout
        /// with one field per parameter, or a parameter group type
        /// layout wrapping such a structure, if the entry point parameters
        /// needed to be allocated into a constant buffer.
        ///
    IRVarLayout* getParamsLayout()
    {
        return cast<IRVarLayout>(getOperand(0));
    }

        /// Get the layout information for the entry point result.
        ///
        /// This represents the return value of the entry point.
        /// Note that it does *not* represent all of the entry
        /// point outputs, because the parameter list may also
        /// contain `out` or `inout` parameters.
        ///
    IRVarLayout* getResultLayout()
    {
        return cast<IRVarLayout>(getOperand(1));
    }
};

    /// Given an entry-point layout, extract the layout for the parameters struct.
IRStructTypeLayout* getScopeStructLayout(IREntryPointLayout* scopeLayout);

    /// Attribute that associates a variable layout with a known stage.
struct IRStageAttr : IRAttr
{
    IR_LEAF_ISA(StageAttr);

    IRIntLit* getStageOperand() { return cast<IRIntLit>(getOperand(0)); }
    Stage getStage() { return Stage(GetIntVal(getStageOperand())); }
};

    /// Base type for attributes that associate a variable layout with a semantic name and index.
struct IRSemanticAttr : IRAttr
{
    IR_PARENT_ISA(SemanticAttr);

    IRStringLit* getNameOperand() { return cast<IRStringLit>(getOperand(0)); }
    UnownedStringSlice getName() { return getNameOperand()->getStringSlice(); }

    IRIntLit* getIndexOperand() { return cast<IRIntLit>(getOperand(1)); }
    UInt getIndex() { return UInt(GetIntVal(getIndexOperand())); }
};

    /// Attribute that associates a variable with a system-value semantic name and index
struct IRSystemValueSemanticAttr : IRSemanticAttr
{
    IR_LEAF_ISA(SystemValueSemanticAttr);
};

    /// Attribute that associates a variable with a user-defined semantic name and index
struct IRUserSemanticAttr : IRSemanticAttr
{
    IR_LEAF_ISA(UserSemanticAttr);
};

    /// Layout infromation for a single parameter/field
struct IRVarLayout : IRLayout
{
    IR_LEAF_ISA(VarLayout)

        /// Get the type layout information for this variable
    IRTypeLayout* getTypeLayout() { return cast<IRTypeLayout>(getOperand(0)); }

        /// Get all the attributes representing resource-kind-specific offsets
    IROperandList<IRVarOffsetAttr> getOffsetAttrs();

        /// Find the offset information (if present) for the given resource `kind`
    IRVarOffsetAttr* findOffsetAttr(LayoutResourceKind kind);

        /// Does this variable use any resources of the given `kind`?
    bool usesResourceKind(LayoutResourceKind kind);

        /// Get the fixed/known stage that this variable is associated with.
        ///
        /// This will be a specific stage for entry-point parameters, but
        /// will be `Stage::Unknown` for any parameter that is not bound
        /// solely to one entry point.
        ///
    Stage getStage();

        /// Find the system-value semantic attribute for this variable, if any.
    IRSystemValueSemanticAttr* findSystemValueSemanticAttr();

        /// Get the (optional) layout for any "pending" data assocaited with this variable.
    IRVarLayout* getPendingVarLayout();

        /// Builder for construction `IRVarLayout`s in a stateful fashion
    struct Builder
    {
            /// Begin building a variable layout with the given `typeLayout`
            ///
            /// The result layout and any instructions needed along the way
            /// will be allocated with `irBuilder`.
            ///
        Builder(
            IRBuilder*      irBuilder,
            IRTypeLayout*   typeLayout);

            /// Represents resource-kind-specific offset information
        struct ResInfo
        {
            LayoutResourceKind  kind = LayoutResourceKind::None;
            UInt                offset = 0;
            UInt                space = 0;
        };

            /// Has any resource usage/offset been registered for the given resource `kind`?
        bool usesResourceKind(LayoutResourceKind kind);

            /// Either fetch or add a `ResInfo` record for `kind` and return it
        ResInfo* findOrAddResourceInfo(LayoutResourceKind kind);

            /// Set the (optional) variable layout for pending data.
        void setPendingVarLayout(IRVarLayout* varLayout)
        {
            m_pendingVarLayout = varLayout;
        }

            /// Set the (optional) system-valeu semantic for this variable.
        void setSystemValueSemantic(String const& name, UInt index);

            /// Set the (optional) user-defined semantic for this variable.
        void setUserSemantic(String const& name, UInt index);

            /// Set the (optional) known stage for this variable.
        void setStage(Stage stage);

            /// Clone all of the layout information from the `other` layout, except for offsets.
            ///
            /// This is convenience when one wants to build a variable layout "like that other one, but..."
        void cloneEverythingButOffsetsFrom(
            IRVarLayout* other);

            /// Build a variable layout using the current state that has been set.
        IRVarLayout* build();

    private:
        IRBuilder* m_irBuilder;
        IRBuilder* getIRBuilder() { return m_irBuilder; };

        IRTypeLayout* m_typeLayout = nullptr;
        IRVarLayout* m_pendingVarLayout = nullptr;

        IRSystemValueSemanticAttr* m_systemValueSemantic = nullptr;
        IRUserSemanticAttr* m_userSemantic = nullptr;
        IRStageAttr* m_stageAttr = nullptr;

        ResInfo m_resInfos[SLANG_PARAMETER_CATEGORY_COUNT];
    };
};

    /// Associate layout information with an instruction.
    ///
    /// This decoration is used in three main ways:
    ///
    /// * To attach an `IRVarLayout` to an `IRGlobalParam` or entry-point `IRParam` representing a shader parameter
    /// * To attach an `IREntryPointLayout` to an `IRFunc` representing an entry point
    /// * To attach an `IRTaggedUnionTypeLayout` to an `IRTaggedUnionType`
    ///
struct IRLayoutDecoration : IRDecoration
{
    enum { kOp = kIROp_LayoutDecoration };
    IR_LEAF_ISA(LayoutDecoration)

        /// Get the layout that is being attached to the parent instruction
    IRLayout* getLayout() { return cast<IRLayout>(getOperand(0)); }
};

//

struct IRCall : IRInst
{
    IR_LEAF_ISA(Call)

    IRInst* getCallee() { return getOperand(0); }

    UInt getArgCount() { return getOperandCount() - 1; }
    IRUse* getArgs() { return getOperands() + 1; }
    IRInst* getArg(UInt index) { return getOperand(index + 1); }
};

struct IRLoad : IRInst
{
    IRUse ptr;
};

struct IRStore : IRInst
{
    IRUse ptr;
    IRUse val;
};

struct IRFieldExtract : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.get(); }
    IRInst* getField() { return field.get(); }
};

struct IRFieldAddress : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.get(); }
    IRInst* getField() { return field.get(); }
};

// Terminators

struct IRReturn : IRTerminatorInst
{};

struct IRReturnVal : IRReturn
{
    IRUse val;

    IRInst* getVal() { return val.get(); }
};

struct IRReturnVoid : IRReturn
{};

struct IRDiscard : IRTerminatorInst
{};

// Signals that this point in the code should be unreachable.
// We can/should emit a dataflow error if we can ever determine
// that a block ending in one of these can actually be
// executed.
struct IRUnreachable : IRTerminatorInst
{
    IR_PARENT_ISA(Unreachable);
};

struct IRMissingReturn : IRUnreachable
{
    IR_LEAF_ISA(MissingReturn);
};

struct IRBlock;

struct IRUnconditionalBranch : IRTerminatorInst
{
    IRUse block;

    IRBlock* getTargetBlock() { return (IRBlock*)block.get(); }

    UInt getArgCount();
    IRUse* getArgs();
    IRInst* getArg(UInt index);

    IR_PARENT_ISA(UnconditionalBranch);
};

// Special cases of unconditional branch, to handle
// structured control flow:
struct IRBreak : IRUnconditionalBranch {};
struct IRContinue : IRUnconditionalBranch {};

// The start of a loop is a special control-flow
// instruction, that records relevant information
// about the loop structure:
struct IRLoop : IRUnconditionalBranch
{
    // The next block after the loop, which
    // is where we expect control flow to
    // re-converge, and also where a
    // `break` will target.
    IRUse breakBlock;

    // The block where control flow will go
    // on a `continue`.
    IRUse continueBlock;

    IRBlock* getBreakBlock() { return (IRBlock*)breakBlock.get(); }
    IRBlock* getContinueBlock() { return (IRBlock*)continueBlock.get(); }
};

struct IRConditionalBranch : IRTerminatorInst
{
    IR_PARENT_ISA(ConditionalBranch)

    IRUse condition;
    IRUse trueBlock;
    IRUse falseBlock;

    IRInst* getCondition() { return condition.get(); }
    IRBlock* getTrueBlock() { return (IRBlock*)trueBlock.get(); }
    IRBlock* getFalseBlock() { return (IRBlock*)falseBlock.get(); }
};

// A conditional branch that represent the test inside a loop
struct IRLoopTest : IRConditionalBranch
{
};

// A conditional branch that represents a one-sided `if`:
//
//     if( <condition> ) { <trueBlock> }
//     <falseBlock>
struct IRIf : IRConditionalBranch
{
    IRBlock* getAfterBlock() { return getFalseBlock(); }
};

// A conditional branch that represents a two-sided `if`:
//
//     if( <condition> ) { <trueBlock> }
//     else              { <falseBlock> }
//     <afterBlock>
//
struct IRIfElse : IRConditionalBranch
{
    IRUse afterBlock;

    IRBlock* getAfterBlock() { return (IRBlock*)afterBlock.get(); }
};

// A multi-way branch that represents a source-level `switch`
struct IRSwitch : IRTerminatorInst
{
    IR_LEAF_ISA(Switch);

    IRUse condition;
    IRUse breakLabel;
    IRUse defaultLabel;

    IRInst* getCondition() { return condition.get(); }
    IRBlock* getBreakLabel() { return (IRBlock*) breakLabel.get(); }
    IRBlock* getDefaultLabel() { return (IRBlock*) defaultLabel.get(); }

    // remaining args are: caseVal, caseLabel, ...

    UInt getCaseCount() { return (getOperandCount() - 3) / 2; }
    IRInst* getCaseValue(UInt index) { return            getOperand(3 + index*2 + 0); }
    IRBlock* getCaseLabel(UInt index) { return (IRBlock*) getOperand(3 + index*2 + 1); }
};

struct IRSwizzle : IRInst
{
    IRUse base;

    IRInst* getBase() { return base.get(); }
    UInt getElementCount()
    {
        return getOperandCount() - 1;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getOperand(index + 1);
    }
};

struct IRSwizzleSet : IRInst
{
    IRUse base;
    IRUse source;

    IRInst* getBase() { return base.get(); }
    IRInst* getSource() { return source.get(); }
    UInt getElementCount()
    {
        return getOperandCount() - 2;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getOperand(index + 2);
    }
};

struct IRSwizzledStore : IRInst
{
    IRInst* getDest() { return getOperand(0); }
    IRInst* getSource() { return getOperand(1); }
    UInt getElementCount()
    {
        return getOperandCount() - 2;
    }
    IRInst* getElementIndex(UInt index)
    {
        return getOperand(index + 2);
    }

    IR_LEAF_ISA(SwizzledStore)
};


struct IRPatchConstantFuncDecoration : IRDecoration
{
    enum { kOp = kIROp_PatchConstantFuncDecoration };
    IR_LEAF_ISA(PatchConstantFuncDecoration)

    IRInst* getFunc() { return getOperand(0); }
}; 

// An IR `var` instruction conceptually represents
// a stack allocation of some memory.
struct IRVar : IRInst
{
    IRPtrType* getDataType()
    {
        return cast<IRPtrType>(IRInst::getDataType());
    }

    static bool isaImpl(IROp op) { return op == kIROp_Var; }
};

/// @brief A global variable.
///
/// Represents a global variable in the IR.
/// If the variable has an initializer, then
/// it is represented by the code in the basic
/// blocks nested inside this value.
struct IRGlobalVar : IRGlobalValueWithCode
{
    IRPtrType* getDataType()
    {
        return cast<IRPtrType>(IRInst::getDataType());
    }
};

/// @brief A global shader parameter.
///
/// Represents a uniform (as opposed to varying) shader parameter
/// passed at the global scope (entry-point `uniform` parameters
/// are encoded as ordinary function parameters.
///
/// Note that an `IRGlobalParam` directly represents the value of
/// the parameter, unlike an `IRGlobalVar`, which represents the
/// *address* of the value. As a result, global parameters are
/// immutable, and subject to various SSA simplifications that
/// do not work for global variables.
///
struct IRGlobalParam : IRInst
{
    IR_LEAF_ISA(GlobalParam)
};

/// @brief A global constnat.
///
/// Represents a global constant that may have a name and linkage.
/// If it has an operand, then this operand is the value of
/// the constants. If there is no operand, then the instruction
/// represents an "extern" constant that will be defined in another
/// module, and which is thus expected to have linkage.
///
struct IRGlobalConstant : IRInst
{
    IR_LEAF_ISA(GlobalConstant);

    /// Get the value of this global constant, or null if the value is not known.
    IRInst* getValue()
    {
        return getOperandCount() != 0 ? getOperand(0) : nullptr;
    }

};

// An entry in a witness table (see below)
struct IRWitnessTableEntry : IRInst
{
    // The AST-level requirement
    IRUse requirementKey;

    // The IR-level value that satisfies the requirement
    IRUse satisfyingVal;

    IRInst* getRequirementKey() { return getOperand(0); }
    IRInst* getSatisfyingVal()  { return getOperand(1); }

    IR_LEAF_ISA(WitnessTableEntry)
};

// A witness table is a global value that stores
// information about how a type conforms to some
// interface. It basically takes the form of a
// map from the required members of the interface
// to the IR values that satisfy those requirements.
struct IRWitnessTable : IRInst
{
    IRInstList<IRWitnessTableEntry> getEntries()
    {
        return IRInstList<IRWitnessTableEntry>(getChildren());
    }

    IR_LEAF_ISA(WitnessTable)
};

// An instruction that yields an undefined value.
//
// Note that we make this an instruction rather than a value,
// so that we will be able to identify a variable that is
// used when undefined.
struct IRUndefined : IRInst
{
};

// A global-scope generic parameter (a type parameter, a
// constraint parameter, etc.)
struct IRGlobalGenericParam : IRInst
{
    IR_LEAF_ISA(GlobalGenericParam)
};

// An instruction that binds a global generic parameter
// to a particular value.
struct IRBindGlobalGenericParam : IRInst
{
    IRGlobalGenericParam* getParam() { return cast<IRGlobalGenericParam>(getOperand(0)); }
    IRInst* getVal() { return getOperand(1); }

    IR_LEAF_ISA(BindGlobalGenericParam)
};


    /// An instruction that packs a concrete value into an existential-type "box"
struct IRMakeExistential : IRInst
{
    IRInst* getWrappedValue() { return getOperand(0); }
    IRInst* getWitnessTable() { return getOperand(1); }

    IR_LEAF_ISA(MakeExistential)
};

    /// Generalizes `IRMakeExistential` by allowing a type with existential sub-fields to be boxed
struct IRWrapExistential : IRInst
{
    IRInst* getWrappedValue() { return getOperand(0); }

    UInt getSlotOperandCount() { return getOperandCount() - 1; }
    IRInst* getSlotOperand(UInt index) { return getOperand(index + 1); }
    IRUse* getSlotOperands() { return getOperands() + 1; }

    IR_LEAF_ISA(WrapExistential)
};


// Description of an instruction to be used for global value numbering
struct IRInstKey
{
    IRInst* inst;

    int GetHashCode();
};

bool operator==(IRInstKey const& left, IRInstKey const& right);

struct IRConstantKey
{
    IRConstant* inst;

    bool operator==(const IRConstantKey& rhs) const { return inst->equal(rhs.inst); }
    int GetHashCode() const { return inst->getHashCode(); }
};

struct SharedIRBuilder
{
    // The parent compilation session
    Session* session;
    Session* getSession()
    {
        return session;
    }

    // The module that will own all of the IR
    IRModule*       module;

    Dictionary<IRInstKey,       IRInst*>    globalValueNumberingMap;
    Dictionary<IRConstantKey,   IRConstant*>    constantMap;

    // TODO: We probably shouldn't use this in the long run.
    Dictionary<void*,           IRLayout*>        layoutMap;
};

struct IRBuilderSourceLocRAII;

struct IRBuilder
{
    // Shared state for all IR builders working on the same module
    SharedIRBuilder*    sharedBuilder;

    Session* getSession()
    {
        return sharedBuilder->getSession();
    }

    IRModule* getModule() { return sharedBuilder->module; }

    // The current parent being inserted into (this might
    // be the global scope, a function, a block inside
    // a function, etc.)
    IRInst*   insertIntoParent = nullptr;
    //
    // An instruction in the current parent that we should insert before
    IRInst*         insertBeforeInst = nullptr;

    // Get the current basic block we are inserting into (if any)
    IRBlock*                getBlock();

    // Get the current function (or other value with code)
    // that we are inserting into (if any).
    IRGlobalValueWithCode*  getFunc();

    void setInsertInto(IRInst* insertInto);
    void setInsertBefore(IRInst* insertBefore);

    IRBuilderSourceLocRAII* sourceLocInfo = nullptr;

    void addInst(IRInst* inst);

    IRInst* getBoolValue(bool value);
    IRInst* getIntValue(IRType* type, IRIntegerValue value);
    IRInst* getFloatValue(IRType* type, IRFloatingPointValue value);
    IRStringLit* getStringValue(const UnownedStringSlice& slice);
    IRPtrLit* getPtrValue(void* value);

    IRBasicType* getBasicType(BaseType baseType);
    IRBasicType* getVoidType();
    IRBasicType* getBoolType();
    IRBasicType* getIntType();
    IRStringType* getStringType();

    IRBasicBlockType*   getBasicBlockType();
    IRType* getWitnessTableType() { return nullptr; }
    IRType* getKeyType() { return nullptr; }

    IRTypeKind*     getTypeKind();
    IRGenericKind*  getGenericKind();

    IRPtrType*  getPtrType(IRType* valueType);
    IROutType*  getOutType(IRType* valueType);
    IRInOutType*  getInOutType(IRType* valueType);
    IRRefType*  getRefType(IRType* valueType);
    IRPtrTypeBase*  getPtrType(IROp op, IRType* valueType);

    IRArrayTypeBase* getArrayTypeBase(
        IROp    op,
        IRType* elementType,
        IRInst* elementCount);

    IRArrayType* getArrayType(
        IRType* elementType,
        IRInst* elementCount);

    IRUnsizedArrayType* getUnsizedArrayType(
        IRType* elementType);

    IRVectorType* getVectorType(
        IRType* elementType,
        IRInst* elementCount);

    IRMatrixType* getMatrixType(
        IRType* elementType,
        IRInst* rowCount,
        IRInst* columnCount);

    IRFuncType* getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType);

    IRFuncType* getFuncType(
        List<IRType*> const&    paramTypes,
        IRType*                 resultType)
    {
        return getFuncType(paramTypes.getCount(), paramTypes.getBuffer(), resultType);
    }

    IRConstantBufferType* getConstantBufferType(
        IRType* elementType);

    IRConstExprRate* getConstExprRate();
    IRGroupSharedRate* getGroupSharedRate();

    IRRateQualifiedType* getRateQualifiedType(
        IRRate* rate,
        IRType* dataType);

    IRType* getTaggedUnionType(
        UInt            caseCount,
        IRType* const*  caseTypes);

    IRType* getTaggedUnionType(
        List<IRType*> const& caseTypes)
    {
        return getTaggedUnionType(caseTypes.getCount(), caseTypes.getBuffer());
    }

    IRType* getBindExistentialsType(
        IRInst*         baseType,
        UInt            slotArgCount,
        IRInst* const*  slotArgs);

    IRType* getBindExistentialsType(
        IRInst*         baseType,
        UInt            slotArgCount,
        IRUse const*    slotArgs);

    // Set the data type of an instruction, while preserving
    // its rate, if any.
    void setDataType(IRInst* inst, IRType* dataType);

        /// Given an existential value, extract the underlying "real" value
    IRInst* emitExtractExistentialValue(
        IRType* type,
        IRInst* existentialValue);

        /// Given an existential value, extract the underlying "real" type
    IRType* emitExtractExistentialType(
        IRInst* existentialValue);

        /// Given an existential value, extract the witness table showing how the value conforms to the existential type.
    IRInst* emitExtractExistentialWitnessTable(
        IRInst* existentialValue);

    IRInst* emitSpecializeInst(
        IRType*         type,
        IRInst*         genericVal,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitLookupInterfaceMethodInst(
        IRType* type,
        IRInst* witnessTableVal,
        IRInst* interfaceMethodVal);

    IRInst* emitCallInst(
        IRType*         type,
        IRInst*         func,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitCallInst(
        IRType*                 type,
        IRInst*                 func,
        List<IRInst*> const&    args)
    {
        return emitCallInst(type, func, args.getCount(), args.getBuffer());
    }

    IRInst* createIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitIntrinsicInst(
        IRType*         type,
        IROp            op,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitConstructorInst(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeVector(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeVector(
        IRType*                 type,
        List<IRInst*> const&    args)
    {
        return emitMakeVector(type, args.getCount(), args.getBuffer());
    }

    IRInst* emitMakeMatrix(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeArray(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeStruct(
        IRType*         type,
        UInt            argCount,
        IRInst* const* args);

    IRInst* emitMakeStruct(
        IRType*                 type,
        List<IRInst*> const&    args)
    {
        return emitMakeStruct(type, args.getCount(), args.getBuffer());
    }

    IRInst* emitMakeExistential(
        IRType* type,
        IRInst* value,
        IRInst* witnessTable);

    IRInst* emitWrapExistential(
        IRType*         type,
        IRInst*         value,
        UInt            slotArgCount,
        IRInst* const*  slotArgs);

    IRInst* emitWrapExistential(
        IRType*         type,
        IRInst*         value,
        UInt            slotArgCount,
        IRUse const*    slotArgs)
    {
        List<IRInst*> slotArgVals;
        for(UInt ii = 0; ii < slotArgCount; ++ii)
            slotArgVals.add(slotArgs[ii].get());

        return emitWrapExistential(type, value, slotArgCount, slotArgVals.getBuffer());
    }

    IRUndefined* emitUndefined(IRType* type);

    IRInst* findOrAddInst(
         IRType*                 type,
         IROp                    op,
         UInt                    operandListCount,
         UInt const*             listOperandCounts,
         IRInst* const* const*   listOperands);

    IRInst* findOrEmitHoistableInst(
        IRType*                 type,
        IROp                    op,
        UInt                    operandListCount,
        UInt const*             listOperandCounts,
        IRInst* const* const*   listOperands);
    IRInst* findOrEmitHoistableInst(
        IRType*         type,
        IROp            op,
        UInt            operandCount,
        IRInst* const*  operands);
    IRInst* findOrEmitHoistableInst(
        IRType*         type,
        IROp            op,
        IRInst*         operand,
        UInt            operandCount,
        IRInst* const*  operands);

    IRModule* createModule();

    IRFunc* createFunc();
    IRGlobalVar* createGlobalVar(
        IRType* valueType);
    IRGlobalParam* createGlobalParam(
        IRType* valueType);
    IRWitnessTable* createWitnessTable();
    IRWitnessTableEntry* createWitnessTableEntry(
        IRWitnessTable* witnessTable,
        IRInst*        requirementKey,
        IRInst*        satisfyingVal);

    // Create an initially empty `struct` type.
    IRStructType*   createStructType();

    // Create an empty `interface` type.
    IRInterfaceType* createInterfaceType();

    // Create a global "key" to use for indexing into a `struct` type.
    IRStructKey*    createStructKey();

    // Create a field nested in a struct type, declaring that
    // the specified field key maps to a field with the specified type.
    IRStructField*  createStructField(
        IRStructType*   structType,
        IRStructKey*    fieldKey,
        IRType*         fieldType);

    IRGeneric* createGeneric();
    IRGeneric* emitGeneric();

    // Low-level operation for creating a type.
    IRType* getType(
        IROp            op,
        UInt            operandCount,
        IRInst* const*  operands);
    IRType* getType(
        IROp            op);
    IRType* getType(
        IROp            op,
        IRInst*         operand0);

        /// Create an empty basic block.
        ///
        /// The created block will not be inserted into the current
        /// function; call `insertBlock()` to attach the block
        /// at an appropriate point.
        ///
    IRBlock* createBlock();

        /// Insert a block into the current function.
        ///
        /// This attaches the given `block` to the current function,
        /// and makes it the current block for
        /// new instructions that get emitted.
        ///
    void insertBlock(IRBlock* block);

        /// Emit a new block into the current function.
        ///
        /// This function is equivalent to using `createBlock()`
        /// and then `insertBlock()`.
        ///
    IRBlock* emitBlock();

    

    IRParam* createParam(
        IRType* type);
    IRParam* emitParam(
        IRType* type);

    IRVar* emitVar(
        IRType* type);

    IRInst* emitLoad(
        IRType* type,
        IRInst* ptr);

    IRInst* emitLoad(
        IRInst*    ptr);

    IRInst* emitStore(
        IRInst*    dstPtr,
        IRInst*    srcVal);

    IRInst* emitFieldExtract(
        IRType*         type,
        IRInst*        base,
        IRInst*        field);

    IRInst* emitFieldAddress(
        IRType*         type,
        IRInst*        basePtr,
        IRInst*        field);

    IRInst* emitElementExtract(
        IRType*     type,
        IRInst*    base,
        IRInst*    index);

    IRInst* emitElementAddress(
        IRType*     type,
        IRInst*    basePtr,
        IRInst*    index);

    IRInst* emitSwizzle(
        IRType*         type,
        IRInst*        base,
        UInt            elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzle(
        IRType*         type,
        IRInst*        base,
        UInt            elementCount,
        UInt const*     elementIndices);

    IRInst* emitSwizzleSet(
        IRType*         type,
        IRInst*        base,
        IRInst*        source,
        UInt            elementCount,
        IRInst* const* elementIndices);

    IRInst* emitSwizzleSet(
        IRType*         type,
        IRInst*        base,
        IRInst*        source,
        UInt            elementCount,
        UInt const*     elementIndices);

    IRInst* emitSwizzledStore(
        IRInst*         dest,
        IRInst*         source,
        UInt            elementCount,
        IRInst* const*  elementIndices);

    IRInst* emitSwizzledStore(
        IRInst*         dest,
        IRInst*         source,
        UInt            elementCount,
        UInt const*     elementIndices);



    IRInst* emitReturn(
        IRInst*    val);

    IRInst* emitReturn();

    IRInst* emitDiscard();

    IRInst* emitUnreachable();
    IRInst* emitMissingReturn();

    IRInst* emitBranch(
        IRBlock*    block);

    IRInst* emitBreak(
        IRBlock*    target);

    IRInst* emitContinue(
        IRBlock*    target);

    IRInst* emitLoop(
        IRBlock*    target,
        IRBlock*    breakBlock,
        IRBlock*    continueBlock);

    IRInst* emitBranch(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock);

    IRInst* emitIf(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    afterBlock);

    IRInst* emitIfElse(
        IRInst*    val,
        IRBlock*    trueBlock,
        IRBlock*    falseBlock,
        IRBlock*    afterBlock);

    IRInst* emitLoopTest(
        IRInst*    val,
        IRBlock*    bodyBlock,
        IRBlock*    breakBlock);

    IRInst* emitSwitch(
        IRInst*        val,
        IRBlock*        breakLabel,
        IRBlock*        defaultLabel,
        UInt            caseArgCount,
        IRInst* const* caseArgs);

    IRGlobalGenericParam* emitGlobalGenericParam(
        IRType* type);

    IRGlobalGenericParam* emitGlobalGenericTypeParam()
    {
        return emitGlobalGenericParam(getTypeKind());
    }

    IRGlobalGenericParam* emitGlobalGenericWitnessTableParam()
    {
        return emitGlobalGenericParam(getWitnessTableType());
    }

    IRBindGlobalGenericParam* emitBindGlobalGenericParam(
        IRInst* param,
        IRInst* val);

    IRDecoration* addBindExistentialSlotsDecoration(
        IRInst*         value,
        UInt            argCount,
        IRInst* const*  args);

    IRInst* emitExtractTaggedUnionTag(
        IRInst* val);

    IRInst* emitExtractTaggedUnionPayload(
        IRType* type,
        IRInst* val,
        IRInst* tag);

    IRInst* emitBitCast(
        IRType* type,
        IRInst* val);

    IRGlobalConstant* emitGlobalConstant(
        IRType* type);

    IRGlobalConstant* emitGlobalConstant(
        IRType* type,
        IRInst* val);

    //
    // Decorations
    //

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* const* operands, Int operandCount);

    IRDecoration* addDecoration(IRInst* value, IROp op)
    {
        return addDecoration(value, op, (IRInst* const*) nullptr, 0);
    }

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* operand)
    {
        return addDecoration(value, op, &operand, 1);
    }

    IRDecoration* addDecoration(IRInst* value, IROp op, IRInst* operand0, IRInst* operand1)
    {
        IRInst* operands[] = { operand0, operand1 };
        return addDecoration(value, op, operands, SLANG_COUNT_OF(operands));
    }

    template <typename T>
    T* addRefObjectToFree(T* ptr)
    {
        getModule()->getObjectScopeManager()->addMaybeNull(ptr);
        return ptr;
    }

    template<typename T>
    void addSimpleDecoration(IRInst* value)
    {
        addDecoration(value, IROp(T::kOp), (IRInst* const*) nullptr, 0);
    }

    void addHighLevelDeclDecoration(IRInst* value, Decl* decl);

//    void addLayoutDecoration(IRInst* value, Layout* layout);
    void addLayoutDecoration(IRInst* value, IRLayout* layout);

//    IRLayout* getLayout(Layout* astLayout);

    IRTypeSizeAttr* getTypeSizeAttr(
        LayoutResourceKind kind,
        LayoutSize size);
    IRVarOffsetAttr* getVarOffsetAttr(
        LayoutResourceKind  kind,
        UInt                offset,
        UInt                space = 0);
    IRPendingLayoutAttr* getPendingLayoutAttr(
        IRLayout* pendingLayout);
    IRStructFieldLayoutAttr* getFieldLayoutAttr(
        IRStructKey*    key,
        IRVarLayout*    layout);
    IRCaseTypeLayoutAttr* getCaseTypeLayoutAttr(
        IRTypeLayout*   layout);

    IRSemanticAttr* getSemanticAttr(
        IROp            op,
        String const&   name,
        UInt            index);
    IRSystemValueSemanticAttr* getSystemValueSemanticAttr(
        String const&   name,
        UInt            index)
    {
        return cast<IRSystemValueSemanticAttr>(getSemanticAttr(
            kIROp_SystemValueSemanticAttr,
            name,
            index));
    }
    IRUserSemanticAttr* getUserSemanticAttr(
        String const&   name,
        UInt            index)
    {
        return cast<IRUserSemanticAttr>(getSemanticAttr(
            kIROp_UserSemanticAttr,
            name,
            index));
    }

    IRStageAttr* getStageAttr(Stage stage);

    IRTypeLayout* getTypeLayout(IROp op, List<IRInst*> const& operands);
    IRVarLayout* getVarLayout(List<IRInst*> const& operands);
    IREntryPointLayout* getEntryPointLayout(
        IRVarLayout* paramsLayout,
        IRVarLayout* resultLayout);


    void addNameHintDecoration(IRInst* value, IRStringLit* name)
    {
        addDecoration(value, kIROp_NameHintDecoration, name);
    }

    void addNameHintDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addNameHintDecoration(value, getStringValue(text));
    }

    void addGLSLOuterArrayDecoration(IRInst* value, UnownedStringSlice const& text)
    {
        addDecoration(value, kIROp_GLSLOuterArrayDecoration, getStringValue(text));
    }

    void addInterpolationModeDecoration(IRInst* value, IRInterpolationMode mode)
    {
        addDecoration(value, kIROp_InterpolationModeDecoration, getIntValue(getIntType(), IRIntegerValue(mode)));
    }

    void addLoopControlDecoration(IRInst* value, IRLoopControl mode)
    {
        addDecoration(value, kIROp_LoopControlDecoration, getIntValue(getIntType(), IRIntegerValue(mode)));
    }

    void addSemanticDecoration(IRInst* value, UnownedStringSlice const& text, int index = 0)
    {
        addDecoration(value, kIROp_SemanticDecoration, getStringValue(text), getIntValue(getIntType(), index));
    }

    void addTargetIntrinsicDecoration(IRInst* value, UnownedStringSlice const& target, UnownedStringSlice const& definition)
    {
        addDecoration(value, kIROp_TargetIntrinsicDecoration, getStringValue(target), getStringValue(definition));
    }

    void addTargetDecoration(IRInst* value, UnownedStringSlice const& target)
    {
        addDecoration(value, kIROp_TargetDecoration, getStringValue(target));
    }

    void addRequireGLSLExtensionDecoration(IRInst* value, UnownedStringSlice const& extensionName)
    {
        addDecoration(value, kIROp_RequireGLSLExtensionDecoration, getStringValue(extensionName));
    }

    void addRequireGLSLVersionDecoration(IRInst* value, Int version)
    {
        addDecoration(value, kIROp_RequireGLSLVersionDecoration, getIntValue(getIntType(), IRIntegerValue(version)));
    }

    void addPatchConstantFuncDecoration(IRInst* value, IRInst* patchConstantFunc)
    {
        addDecoration(value, kIROp_PatchConstantFuncDecoration, patchConstantFunc);
    }

    void addImportDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ImportDecoration, getStringValue(mangledName));
    }

    void addExportDecoration(IRInst* value, UnownedStringSlice const& mangledName)
    {
        addDecoration(value, kIROp_ExportDecoration, getStringValue(mangledName));
    }

    void addEntryPointDecoration(IRInst* value, Profile profile, UnownedStringSlice const& name)
    {
        addDecoration(value, kIROp_EntryPointDecoration, getIntValue(getIntType(), profile.raw), getStringValue(name));
    }

    void addKeepAliveDecoration(IRInst* value)
    {
        addDecoration(value, kIROp_KeepAliveDecoration);
    }

        /// Add a decoration that indicates that the given `inst` depends on the given `dependency`.
        ///
        /// This decoration can be used to ensure that a value that an instruction
        /// implicitly depends on cannot be eliminated so long as the instruction
        /// itself is kept alive.
        ///
    void addDependsOnDecoration(IRInst* inst, IRInst* dependency)
    {
        addDecoration(inst, kIROp_DependsOnDecoration, dependency);
    }

    void addFormatDecoration(IRInst* inst, ImageFormat format)
    {
        addFormatDecoration(inst, getIntValue(getIntType(), IRIntegerValue(format)));
    }

    void addFormatDecoration(IRInst* inst, IRInst* format)
    {
        addDecoration(inst, kIROp_FormatDecoration, format);
    }
};

void addHoistableInst(
    IRBuilder*  builder,
    IRInst*     inst);

// Helper to establish the source location that will be used
// by an IRBuilder.
struct IRBuilderSourceLocRAII
{
    IRBuilder*  builder;
    SourceLoc   sourceLoc;
    IRBuilderSourceLocRAII* next;

    IRBuilderSourceLocRAII(
        IRBuilder*  builder,
        SourceLoc   sourceLoc)
        : builder(builder)
        , sourceLoc(sourceLoc)
        , next(nullptr)
    {
        next = builder->sourceLocInfo;
        builder->sourceLocInfo = this;
    }

    ~IRBuilderSourceLocRAII()
    {
        SLANG_ASSERT(builder->sourceLocInfo == this);
        builder->sourceLocInfo = next;
    }
};

//

void markConstExpr(
    IRBuilder*  builder,
    IRInst*     irValue);

//

IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(
        IRInst*        val,
        String const&   targetName);

}

#endif
