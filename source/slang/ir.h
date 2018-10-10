// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "../core/basic.h"

#include "source-loc.h"

#include "../core/slang-memory-arena.h"

#include "type-system-shared.h"

namespace Slang {

class   Decl;
class   GenericDecl;
class   FuncType;
class   Layout;
class   Type;
class   Session;
class   Name;
struct  IRFunc;
struct  IRGlobalValueWithCode;
struct  IRInst;
struct  IRModule;

typedef unsigned int IROpFlags;
enum : IROpFlags
{
    kIROpFlags_None = 0,
    kIROpFlag_Parent = 1 << 0,                  ///< This op is a parent op
    kIROpFlag_UseOther = 1 << 1,                ///< If set this op can use 'other bits' to store information
};

/* Bit usage of IROp is a follows

          MainOp | Pseudo | Other
Bit range: 0-7   |    8   | Remaining bits

If an instruction is 'pseudo' (ie shouldn't appear in output IR), then the Pseudo bit is set - and 'Invalid' falls into 
this category as well as all pseudo ops.
For doing range checks (for example for doing isa tests), the value is masked by kIROpMeta_OpMask, such that the Other bits don't interfere.
The other bits can be used for storage for anything that needs to identify as a different 'op' or 'type'. It is currently 
used currently for storing the TextureFlavor of a IRResourceTypeBase derived types for example. 
*/
enum IROp : int32_t
{
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    kIROp_##ID,

#include "ir-inst-defs.h"

    kIROpCount,

    // We use the range 0x100 to 0x1ff set for pseudo/non main codes
    // Instructions that should not appear in valid IR.

    kIROp_Invalid = 0x100,                                      ///< If bit set, then in pseudo/not normal space 
    kIRPseudoOp_First = kIROp_Invalid,

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS) /* empty */
#define PSEUDO_INST(ID) kIRPseudoOp_##ID,

    kIRPseudoOp_LastPlusOne,

#include "ir-inst-defs.h"

#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS) /* empty */
#define INST_RANGE(BASE, FIRST, LAST)       \
    kIROp_First##BASE   = kIROp_##FIRST,    \
    kIROp_Last##BASE    = kIROp_##LAST,

#include "ir-inst-defs.h"
};

/* IROpMeta describe values for layout of IROp, as well as values for accessing aspects of IROp bits. */
enum IROpMeta
{
    kIROpMeta_OtherShift = 9,                                            ///< Number of bits for op/pseudo ops (shift right by this to get the other bits)
    kIROpMeta_PseudoOpMask = (int32_t(1) << kIROpMeta_OtherShift) - 1,   ///< Mask for ops including pseudo ops
    kIROpMeta_OpMask = 0xff,                                        ///< Mask for just ops
    kIrOpMeta_OtherMask = ~kIROpMeta_PseudoOpMask,                  ///< Mask for bits that can be used for other purposes than 'op' ('other' bits)
    kIROpMeta_IsPseudoOp = kIROp_Invalid,                           ///< 'And' with op, if set, the op is a pseudo op
};

// True if op is pseudo (or invalid which is 'pseudo-like' at least in as so far as current behavior)
SLANG_FORCE_INLINE bool isPseudoOp(IROp op) { return (op & kIROpMeta_IsPseudoOp) != 0; }

IROp findIROp(char const* name);

// A logical operation/opcode in the IR
struct IROpInfo
{
    // What is the name/mnemonic for this operation
    char const*     name;

    // How many required arguments are there
    // (not including the mandatory type argument)
    unsigned int    fixedArgCount;

    // Flags to control how we emit additional info
    IROpFlags       flags;
};

// Look up the info for an op
IROpInfo getIROpInfo(IROp op);

// A use of another value/inst within an IR operation
struct IRUse
{
    IRInst* get() { return usedValue; }
    IRInst* getUser() { return user; }

    void init(IRInst* user, IRInst* usedValue);
    void set(IRInst* usedValue);
    void clear();

    // The instruction that is being used
    IRInst* usedValue = nullptr;

    // The instruction that is doing the using.
    IRInst* user = nullptr;

    // The next use of the same value
    IRUse*  nextUse = nullptr;

    // A "link" back to where this use is referenced,
    // so that we can simplify updates.
    IRUse** prevLink = nullptr;

    void debugValidate();
};

enum IRDecorationOp : uint16_t
{
    kIRDecorationOp_HighLevelDecl,
    kIRDecorationOp_Layout,
    kIRDecorationOp_LoopControl,
    kIRDecorationOp_Target,
    kIRDecorationOp_TargetIntrinsic,
    kIRDecorationOp_GLSLOuterArray,
    kIRDecorationOp_Semantic,
    kIRDecorationOp_InterpolationMode,
    kIRDecorationOp_NameHint,

        /**  The _instruction_ is transitory. Such a decoration should NEVER be found on an output instruction a module. 
        Typically used mark an instruction so can be specially handled - say when creating a IRConstant literal, and the payload of 
        needs to be special cased for lookup. */ 
    kIRDecorationOp_Transitory,   
    kIRDecorationOp_VulkanRayPayload,
    kIRDecorationOp_VulkanHitAttributes,
    
    kIRDecorationOp_CountOf          
};

// represents an object allocated in an IR memory arena
struct IRObject
{
};

// A "decoration" that gets applied to an instruction.
// These usually don't affect semantics, but are useful
// for preserving high-level source information.
struct IRDecoration : public IRObject
{
    static IRDecoration make(IRDecorationOp opIn, IRDecoration* nextIn = nullptr)
    {
        IRDecoration dec;
        dec.next = nextIn;
        dec.op = opIn;
        return dec;
    }

    // Next decoration attached to the same instruction
    IRDecoration* next;

    IRDecorationOp op;
};

struct IRBlock;
struct IRParentInst;
struct IRRate;
struct IRType;

// Every value in the IR is an instruction (even things
// like literal values).
//
struct IRInst : public IRObject
{
    // The operation that this value represents
    IROp op;

    // The total number of operands of this instruction.
    //
    // TODO: We shouldn't need to allocate this on
    // all instructions. Instead we should have
    // instructions that need "vararg" support to
    // allocate this field ahead of the `this`
    // pointer.
    uint32_t operandCount = 0;

    UInt getOperandCount()
    {
        return operandCount;
    }

    // Source location information for this value, if any
    SourceLoc sourceLoc;

    // The linked list of decorations attached to this value
    IRDecoration* firstDecoration = nullptr;

    // Look up a decoration in the list of decorations
    IRDecoration* findDecorationImpl(IRDecorationOp op);
    template<typename T>
    T* findDecoration()
    {
        return (T*) findDecorationImpl(IRDecorationOp(T::kDecorationOp));
    }

    // The first use of this value (start of a linked list)
    IRUse*      firstUse = nullptr;


    // The parent of this instruction.
    IRParentInst*   parent;

    IRParentInst* getParent() { return parent; }

    // The next and previous instructions with the same parent
    IRInst*         next;
    IRInst*         prev;

    IRInst* getNextInst() { return next; }
    IRInst* getPrevInst() { return prev; }

    // The type of the result value of this instruction,
    // or `null` to indicate that the instruction has
    // no value.
    IRUse typeUse;

    IRType* getFullType() { return (IRType*) typeUse.get(); }
    void setFullType(IRType* type) { typeUse.init(this, (IRInst*) type); }

    IRRate* getRate();

    IRType* getDataType();

    // After the type, we have data that is specific to
    // the subtype of `IRInst`. In most cases, this is
    // just a series of `IRUse` values representing
    // operands of the instruction.

    IRUse*      getOperands();

    IRInst* getOperand(UInt index)
    {
        return getOperands()[index].get();
    }

    void setOperand(UInt index, IRInst* value)
    {
        getOperands()[index].set(value);
    }


    //

    // Replace all uses of this value with `other`, so
    // that this value will now have no uses.
    void replaceUsesWith(IRInst* other);

    // Insert this instruction into the same basic block
    // as `other`, right before/after it.
    void insertBefore(IRInst* other);
    void insertAfter(IRInst* other);

    // Insert as first/last child of given parent
    void insertAtStart(IRParentInst* parent);
    void insertAtEnd(IRParentInst* parent);

    // Move to the start/end of current parent
    void moveToStart();
    void moveToEnd();

    // Insert before/after the given instruction, in a specific block
    void insertBefore(IRInst* other, IRParentInst* parent);
    void insertAfter(IRInst* other, IRParentInst* parent);

    // Remove this instruction from its parent block,
    // but don't delete it, or replace uses.
    void removeFromParent();

    // Remove this instruction from its parent block,
    // and then destroy it (it had better have no uses!)
    void removeAndDeallocate();

    // Clear out the arguments of this instruction,
    // so that we don't appear on the list of uses
    // for those values.
    void removeArguments();

    /// Does this instruction have any uses?
    bool hasUses() const { return firstUse != nullptr; }

    /// Does this instructiomn have more than one use?
    bool hasMoreThanOneUse() const { return firstUse != nullptr && firstUse->nextUse != nullptr; }

    /// It is possible that this instruction has side effects?
    ///
    /// This is a conservative test, and will return `true` if an exact answer can't be determined.
    bool mightHaveSideEffects();

    // RTTI support
    static bool isaImpl(IROp) { return true; }

    /// Find the module that this instruction is nested under.
    ///
    /// If this instruction is transitively nested inside some IR module,
    /// this function will return it, and will otherwise return `null`.
    IRModule* getModule();
};

// `dynamic_cast` equivalent
template<typename T>
T* as(IRInst* inst, T* /* */ = nullptr)
{
    if (inst && T::isaImpl(inst->op))
        return (T*) inst;
    return nullptr;
}

// `static_cast` equivalent, with debug validation
template<typename T>
T* cast(IRInst* inst, T* /* */ = nullptr)
{
    SLANG_ASSERT(!inst || as<T>(inst));
    return (T*)inst;
}


// A double-linked list of instruction
struct IRInstListBase
{
    IRInstListBase()
    {}

    IRInstListBase(IRInst* first, IRInst* last)
        : first(first)
        , last(last)
    {}



    IRInst* first = nullptr;
    IRInst* last = nullptr;

    IRInst* getFirst() { return first; }
    IRInst* getLast() { return last; }

    struct Iterator
    {
        IRInst* inst;

        Iterator() : inst(nullptr) {}
        Iterator(IRInst* inst) : inst(inst) {}

        void operator++()
        {
            if (inst)
            {
                inst = inst->next;
            }
        }

        IRInst* operator*()
        {
            return inst;
        }

        bool operator!=(Iterator const& i)
        {
            return inst != i.inst;
        }
    };

    Iterator begin() { return Iterator(first); }
    Iterator end() { return Iterator(last ? last->next : nullptr); }
};

// Specialization of `IRInstListBase` for the case where
// we know (or at least expect) all of the instructions
// to be of type `T`
template<typename T>
struct IRInstList : IRInstListBase
{
    IRInstList() {}

    IRInstList(T* first, T* last)
        : IRInstListBase(first, last)
    {}

    explicit IRInstList(IRInstListBase const& list)
        : IRInstListBase(list)
    {}

    T* getFirst() { return (T*) first; }
    T* getLast() { return (T*) last; }

    struct Iterator : public IRInstListBase::Iterator
    {
        Iterator() {}
        Iterator(IRInst* inst) : IRInstListBase::Iterator(inst) {}

        T* operator*()
        {
            return (T*) inst;
        }
    };

    Iterator begin() { return Iterator(first); }
    Iterator end() { return Iterator(last ? last->next : nullptr); }
};

// Types

#define IR_LEAF_ISA(NAME) static bool isaImpl(IROp op) { return (kIROpMeta_PseudoOpMask & op) == kIROp_##NAME; }
#define IR_PARENT_ISA(NAME) static bool isaImpl(IROp opIn) { const int op = (kIROpMeta_PseudoOpMask & opIn); return op >= kIROp_First##NAME && op <= kIROp_Last##NAME; }

#define SIMPLE_IR_TYPE(NAME, BASE) struct IR##NAME : IR##BASE { IR_LEAF_ISA(NAME) };
#define SIMPLE_IR_PARENT_TYPE(NAME, BASE) struct IR##NAME : IR##BASE { IR_PARENT_ISA(NAME) };


// All types in the IR are represented as instructions which conceptually
// execute before run time.
struct IRType : IRInst
{
    IRType* getCanonicalType() { return this; }

    IR_PARENT_ISA(Type)
};

struct IRBasicType : IRType
{
    BaseType getBaseType() { return BaseType(op - kIROp_FirstBasicType); }

    IR_PARENT_ISA(BasicType)
};

struct IRVoidType : IRBasicType
{
    IR_LEAF_ISA(VoidType)
};

struct IRBoolType : IRBasicType
{
    IR_LEAF_ISA(BoolType)
};

SIMPLE_IR_TYPE(StringType, Type)

// Constant Instructions

typedef int64_t IRIntegerValue;
typedef double IRFloatingPointValue;

struct IRConstant : IRInst
{
    struct StringValue
    {   
        uint32_t numChars;           ///< The number of chars
        char chars[1];               ///< Chars added at end. NOTE! Must be last member of struct! 
    };
    struct StringSliceValue
    {
        uint32_t numChars;
        char* chars;
    };

    union ValueUnion
    {
        IRIntegerValue          intVal;         ///< Used for integrals and boolean
        IRFloatingPointValue    floatVal;

        /// Either of these types could be set with kIROp_StringLit. 
        /// Which is used is currently determined with IRDecorationOp - if a kDecorationOp_Transitory is set, then the transitory StringVal is used, else stringVal
        // which relies on chars being held after the struct).
        StringValue             stringVal;
        StringSliceValue        transitoryStringVal;           
    };

        /// Returns a string slice (or empty string if not appropriate)
    UnownedStringSlice getStringSlice() const;

        /// True if constants are equal
    bool equal(IRConstant& rhs);
        /// Get the hash 
    int getHashCode();

    IR_PARENT_ISA(Constant)

    // Must be last member, because data may be held behind
    // NOTE! The total size of IRConstant may not be allocated - only enough space is allocated for the value type held in the union.
    ValueUnion value;
};

struct IRIntLit : IRConstant
{
    IRIntegerValue getValue() { return value.intVal; }

    IR_LEAF_ISA(IntLit);
};

struct IRBoolLit : IRConstant
{
    bool getValue() { return value.intVal != 0; }

    IR_LEAF_ISA(BoolLit);
};


// Get the compile-time constant integer value of an instruction,
// if it has one, and assert-fail otherwise.
IRIntegerValue GetIntVal(IRInst* inst);

struct IRStringLit : IRConstant
{
    
    IR_LEAF_ISA(StringLit);
};

// A instruction that ends a basic block (usually because of control flow)
struct IRTerminatorInst : IRInst
{
    IR_PARENT_ISA(TerminatorInst)
};

// A function parameter is owned by a basic block, and represents
// either an incoming function parameter (in the entry block), or
// a value that flows from one SSA block to another (in a non-entry
// block).
//
// In each case, the basic idea is that a block is a "label with
// arguments."
//
struct IRParam : IRInst
{
    IRParam* getNextParam();
    IRParam* getPrevParam();

    IR_LEAF_ISA(Param)
};

// A "parent" instruction is one that contains other instructions
// as its children. The most common case of a parent instruction
// is a basic block, but there are other cases (e.g., a function
// is in turn a parent for basic blocks).
struct IRParentInst : IRInst
{
    // The instructions stored under this parent
    IRInstListBase children;

    IRInst* getFirstChild() { return children.first; }
    IRInst* getLastChild()  { return children.last;  }
    IRInstListBase getChildren() { return children; }

    void removeAndDeallocateAllChildren();

    IR_PARENT_ISA(ParentInst)
};

// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
struct IRBlock : IRParentInst
{
    // Linked list of the instructions contained in this block
    //
    IRInstListBase getChildren() { return children; }
    IRInst* getFirstInst() { return children.first; }
    IRInst* getLastInst() { return children.last; }

    // In a valid program, every basic block should end with
    // a "terminator" instruction.
    //
    // This function will return the terminator, if it exists,
    // or `null` if there is none.
    IRTerminatorInst* getTerminator() { return as<IRTerminatorInst>(getLastInst()); }

    // We expect that the siblings of a basic block will
    // always be other basic blocks (we don't allow
    // mixing of blocks and other instructions in the
    // same parent).
    IRBlock* getPrevBlock() { return cast<IRBlock>(getPrevInst()); }
    IRBlock* getNextBlock() { return cast<IRBlock>(getNextInst()); }

    // The parameters of a block are represented by `IRParam`
    // instructions at the start of the block. These play
    // the role of function parameters for the entry block
    // of a function, and of phi nodes in other blocks.
    IRParam* getFirstParam() { return as<IRParam>(getFirstInst()); }
    IRParam* getLastParam();
    IRInstList<IRParam> getParams()
    {
        return IRInstList<IRParam>(
            getFirstParam(),
            getLastParam());
    }

    void addParam(IRParam* param);

    // The "ordinary" instructions come after the parameters
    IRInst* getFirstOrdinaryInst();
    IRInst* getLastOrdinaryInst();
    IRInstList<IRInst> getOrdinaryInsts()
    {
        return IRInstList<IRInst>(
            getFirstOrdinaryInst(),
            getLastOrdinaryInst());
    }

    // The parent of a basic block is assumed to be a
    // value with code (e.g., a function, global variable
    // with initializer, etc.).
    IRGlobalValueWithCode* getParent() { return cast<IRGlobalValueWithCode>(IRInst::getParent()); }

    // The predecessor and successor lists of a block are needed
    // when we want to work with the control flow graph (CFG) of
    // a function. Rather than store these explicitly (and thus
    // need to update them when transformations might change the
    // CFG), we compute predecessors and successors in an
    // implicit fashion using the use-def information for a
    // block itself.
    //
    // To a first approximation, the predecessors of a block
    // are the blocks where the IR value of the block is used.
    // Similarly, the successors of a block are all values used
    // by the terminator instruction of the block.
    // The `getPredecessors()` and `getSuccessors()` functions
    // make this more precise.
    //
    struct PredecessorList
    {
        PredecessorList(IRUse* begin) : b(begin) {}
        IRUse* b;

        UInt getCount();
        bool isEmpty();

        struct Iterator
        {
            Iterator(IRUse* use) : use(use) {}

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that)
            {
                return use != that.use;
            }

            IRUse* use;
        };

        Iterator begin() { return Iterator(b); }
        Iterator end()   { return Iterator(nullptr); }
    };

    struct SuccessorList
    {
        SuccessorList(IRUse* begin, IRUse* end, UInt stride = 1) : begin_(begin), end_(end), stride(stride) {}
        IRUse* begin_;
        IRUse* end_;
        UInt stride;

        UInt getCount();

        struct Iterator
        {
            Iterator(IRUse* use, UInt stride) : use(use), stride(stride) {}

            IRBlock* operator*();

            void operator++();

            bool operator!=(Iterator const& that)
            {
                return use != that.use;
            }

            IRUse* use;
            UInt stride;
        };

        Iterator begin() { return Iterator(begin_, stride); }
        Iterator end()   { return Iterator(end_, stride); }
    };

    PredecessorList getPredecessors();
    SuccessorList getSuccessors();

    //

    IR_LEAF_ISA(Block)
};

SIMPLE_IR_TYPE(BasicBlockType, Type)

struct IRResourceTypeBase : IRType
{
    TextureFlavor getFlavor() const
    {
        return TextureFlavor((op >> kIROpMeta_OtherShift) & 0xFFFF);
    }

    TextureFlavor::Shape GetBaseShape() const
    {
        return getFlavor().GetBaseShape();
    }
    bool isMultisample() const { return getFlavor().isMultisample(); }
    bool isArray() const { return getFlavor().isArray(); }
    SlangResourceShape getShape() const { return getFlavor().getShape(); }
    SlangResourceAccess getAccess() const { return getFlavor().getAccess(); }

    IR_PARENT_ISA(ResourceTypeBase);
};

struct IRResourceType : IRResourceTypeBase
{
    IRType* getElementType() { return (IRType*)getOperand(0); }

    IR_PARENT_ISA(ResourceType)
};

struct IRTextureTypeBase : IRResourceType
{
    IR_PARENT_ISA(TextureTypeBase)
};

struct IRTextureType : IRTextureTypeBase
{
    IR_LEAF_ISA(TextureType)
};

struct IRTextureSamplerType : IRTextureTypeBase
{
    IR_LEAF_ISA(TextureSamplerType)
};

struct IRGLSLImageType : IRTextureTypeBase
{
    IR_LEAF_ISA(GLSLImageType)
};

struct IRSamplerStateTypeBase : IRType
{
    IR_PARENT_ISA(SamplerStateTypeBase)
};

SIMPLE_IR_TYPE(SamplerStateType, SamplerStateTypeBase)
SIMPLE_IR_TYPE(SamplerComparisonStateType, SamplerStateTypeBase)

struct IRBuiltinGenericType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }

    IR_PARENT_ISA(BuiltinGenericType)
};

SIMPLE_IR_PARENT_TYPE(PointerLikeType, BuiltinGenericType);
SIMPLE_IR_PARENT_TYPE(HLSLStructuredBufferTypeBase, BuiltinGenericType)
SIMPLE_IR_TYPE(HLSLStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_IR_TYPE(HLSLRWStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_IR_TYPE(HLSLRasterizerOrderedStructuredBufferType, HLSLStructuredBufferTypeBase)

SIMPLE_IR_PARENT_TYPE(UntypedBufferResourceType, Type)
SIMPLE_IR_TYPE(HLSLByteAddressBufferType, UntypedBufferResourceType)
SIMPLE_IR_TYPE(HLSLRWByteAddressBufferType, UntypedBufferResourceType)
SIMPLE_IR_TYPE(HLSLRasterizerOrderedByteAddressBufferType, UntypedBufferResourceType)

SIMPLE_IR_TYPE(HLSLAppendStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_IR_TYPE(HLSLConsumeStructuredBufferType, HLSLStructuredBufferTypeBase)

struct IRHLSLPatchType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }

    IR_PARENT_ISA(HLSLPatchType)
};

SIMPLE_IR_TYPE(HLSLInputPatchType, HLSLPatchType)
SIMPLE_IR_TYPE(HLSLOutputPatchType, HLSLPatchType)

SIMPLE_IR_PARENT_TYPE(HLSLStreamOutputType, BuiltinGenericType)
SIMPLE_IR_TYPE(HLSLPointStreamType, HLSLStreamOutputType)
SIMPLE_IR_TYPE(HLSLLineStreamType, HLSLStreamOutputType)
SIMPLE_IR_TYPE(HLSLTriangleStreamType, HLSLStreamOutputType)

SIMPLE_IR_TYPE(GLSLInputAttachmentType, Type)
SIMPLE_IR_PARENT_TYPE(ParameterGroupType, PointerLikeType)
SIMPLE_IR_PARENT_TYPE(UniformParameterGroupType, ParameterGroupType)
SIMPLE_IR_PARENT_TYPE(VaryingParameterGroupType, ParameterGroupType)
SIMPLE_IR_TYPE(ConstantBufferType, UniformParameterGroupType)
SIMPLE_IR_TYPE(TextureBufferType, UniformParameterGroupType)
SIMPLE_IR_TYPE(GLSLInputParameterGroupType, VaryingParameterGroupType)
SIMPLE_IR_TYPE(GLSLOutputParameterGroupType, VaryingParameterGroupType)
SIMPLE_IR_TYPE(GLSLShaderStorageBufferType, UniformParameterGroupType)
SIMPLE_IR_TYPE(ParameterBlockType, UniformParameterGroupType)

struct IRArrayTypeBase : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }

    // Returns the element count for an `IRArrayType`, and null
    // for an `IRUnsizedArrayType`.
    IRInst* getElementCount();

    IR_PARENT_ISA(ArrayTypeBase)
};

struct IRArrayType: IRArrayTypeBase
{
    IRInst* getElementCount() { return getOperand(1); }

    IR_LEAF_ISA(ArrayType)
};

SIMPLE_IR_TYPE(UnsizedArrayType, ArrayTypeBase)

SIMPLE_IR_PARENT_TYPE(Rate, Type)
SIMPLE_IR_TYPE(ConstExprRate, Rate)
SIMPLE_IR_TYPE(GroupSharedRate, Rate)

struct IRRateQualifiedType : IRType
{
    IRRate* getRate() { return (IRRate*) getOperand(0); }
    IRType* getValueType() { return (IRType*) getOperand(1); }

    IR_LEAF_ISA(RateQualifiedType)
};


// Unlike the AST-level type system where `TypeType` tracks the
// underlying type, the "type of types" in the IR is a simple
// value with no operands, so that all type nodes have the
// same type.
SIMPLE_IR_PARENT_TYPE(Kind, Type);
SIMPLE_IR_TYPE(TypeKind, Kind);

// The kind of any and all generics.
//
// A more complete type system would include "arrow kinds" to
// be able to track the domain and range of generics (e.g.,
// the `vector` generic maps a type and an integer to a type).
// This is only really needed if we ever wanted to support
// "higher-kinded" generics (e.g., a generic that takes another
// generic as a parameter).
//
SIMPLE_IR_TYPE(GenericKind, Kind)

struct IRVectorType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getElementCount() { return getOperand(1); }

    IR_LEAF_ISA(VectorType)
};

struct IRMatrixType : IRType
{
    IRType* getElementType() { return (IRType*)getOperand(0); }
    IRInst* getRowCount() { return getOperand(1); }
    IRInst* getColumnCount() { return getOperand(2); }

    IR_LEAF_ISA(MatrixType)
};

struct IRPtrTypeBase : IRType
{
    IRType* getValueType() { return (IRType*)getOperand(0); }

    IR_PARENT_ISA(PtrTypeBase)
};

struct IRPtrType : IRPtrTypeBase
{
    IR_LEAF_ISA(PtrType)
};

SIMPLE_IR_PARENT_TYPE(OutTypeBase, PtrTypeBase)
SIMPLE_IR_TYPE(OutType, OutTypeBase)
SIMPLE_IR_TYPE(InOutType, OutTypeBase)
SIMPLE_IR_TYPE(RefType, OutTypeBase)

struct IRFuncType : IRType
{
    IRType* getResultType() { return (IRType*) getOperand(0); }
    UInt getParamCount() { return getOperandCount() - 1; }
    IRType* getParamType(UInt index) { return (IRType*)getOperand(1 + index); }

    IR_LEAF_ISA(FuncType)
};

// A "global value" is an instruction that might have
// linkage, so that it can be declared in one module
// and then resolved to a definition in another module.
struct IRGlobalValue : IRParentInst
{
    // The mangled name, for a symbol that should have linkage,
    // or which might have multiple declarations.
    Name* mangledName = nullptr;

#if 0
    // TODO: these all belong on `IRInst`
    void insertBefore(IRGlobalValue* other);
    void insertBefore(IRGlobalValue* other, IRModule* module);
    void insertAtStart(IRModule* module);

    void insertAfter(IRGlobalValue* other);
    void insertAfter(IRGlobalValue* other, IRModule* module);
    void insertAtEnd(IRModule* module);

    void removeFromParent();

    void moveToEnd();
#endif

    IR_PARENT_ISA(GlobalValue)
};

bool isDefinition(
    IRGlobalValue* inVal);


// A structure type is represented as a parent instruction,
// where the child instructions represent the fields of the
// struct.
//
// The space of fields that a given struct type supports
// are defined as its "keys", which are global values
// (that is, they have mangled names that can be used
// for linkage).
//
struct IRStructKey : IRGlobalValue
{
    IR_LEAF_ISA(StructKey)
};
//
// The fields of the struct are then defined as mappings
// from those keys to the associated type (in the case of
// the struct type) or to values (when lookup up a field).
//
// A struct field thus has two operands: the key, and the
// type of the field.
//
struct IRStructField : IRInst
{
    IRStructKey* getKey() { return cast<IRStructKey>(getOperand(0)); }
    IRType* getFieldType() { return cast<IRType>(getOperand(1)); }

    IR_LEAF_ISA(StructField)
};
//
// The struct type is then represented as a parent instruction
// that contains the various fields. Note that a struct does
// *not* contain the keys, because code needs to be able to
// reference the keys from scopes outside of the struct.
//
struct IRStructType : IRGlobalValue
{
    IRInstList<IRStructField> getFields() { return IRInstList<IRStructField>(getChildren()); }

    IR_LEAF_ISA(StructType)
};


/// @brief A global value that potentially holds executable code.
///
struct IRGlobalValueWithCode : IRGlobalValue
{
    // The children of a value with code will be the basic
    // blocks of its definition.
    IRBlock* getFirstBlock() { return cast<IRBlock>(getFirstChild()); }
    IRBlock* getLastBlock() { return cast<IRBlock>(getLastChild()); }
    IRInstList<IRBlock> getBlocks()
    {
        return IRInstList<IRBlock>(getChildren());
    }

    // Add a block to the end of this function.
    void addBlock(IRBlock* block);

    IR_PARENT_ISA(GlobalValueWithCode)
};

// A value that has parameters so that it can conceptually be called.
struct IRGlobalValueWithParams : IRGlobalValueWithCode
{
    // Convenience accessor for the IR parameters,
    // which are actually the parameters of the first
    // block.
    IRParam* getFirstParam();

    IR_PARENT_ISA(GlobalValueWithParams)
};

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRGlobalValueWithParams
{
    // The type of the IR-level function
    IRFuncType* getDataType() { return (IRFuncType*) IRInst::getDataType(); }

    // Convenience accessors for working with the
    // function's type.
    IRType* getResultType();
    UInt getParamCount();
    IRType* getParamType(UInt index);

    IR_LEAF_ISA(Func)
};

// A generic is akin to a function, but is conceptually executed
// before runtime, to specialize the code nested within.
//
// In practice, a generic always holds only a single block, and ends
// with a `return` instruction for the value that the generic yields.
struct IRGeneric : IRGlobalValueWithParams
{
    IR_LEAF_ISA(Generic)
};

// Find the value that is returned from a generic, so that
// a pass can glean information from it.
IRInst* findGenericReturnVal(IRGeneric* generic);

// The IR module itself is represented as an instruction, which
// serves at the root of the tree of all instructions in the module.
struct IRModuleInst : IRParentInst
{
    // Pointer back to the non-instruction object that represents
    // the module, so that we can get back to it in algorithms
    // that need it.
    IRModule* module;

    IRInstListBase getGlobalInsts() { return getChildren(); }

    IR_LEAF_ISA(Module)
};

struct IRModule : RefObject
{
    enum 
    {
        kMemoryArenaBlockSize = 16 * 1024,           ///< Use 16k block size for memory arena
    };

    SLANG_FORCE_INLINE Session* getSession() const { return session; }
    SLANG_FORCE_INLINE IRModuleInst* getModuleInst() const { return moduleInst;  }

    IRInstListBase getGlobalInsts() const { return getModuleInst()->getChildren(); }

    void addRefObjectToFree(RefObject* obj)
    {
        if (obj)
        {
            obj->addReference();
            m_refObjectsToFree.Add(obj);
        }
    }
    
        /// Ctor
    IRModule():
        memoryArena(kMemoryArenaBlockSize)
    {
    }

    ~IRModule()
    { 
        // Release all ref objects
        for (RefObject* ptr: m_refObjectsToFree)
        {
            ptr->releaseReference();
        }
        // Clear any memory too
        m_refObjectsToFree = List<RefObject*>();
    }

    MemoryArena memoryArena;

    // The compilation session in use.
    Session*    session;
    IRModuleInst* moduleInst;

    protected:
    List<RefObject*> m_refObjectsToFree;
};

void printSlangIRAssembly(StringBuilder& builder, IRModule* module);
String getSlangIRAssembly(IRModule* module);

void dumpIR(IRModule* module);
void dumpIR(IRGlobalValue* globalVal);

String dumpIRFunc(IRFunc* func);

IRInst* createEmptyInst(
    IRModule*   module,
    IROp        op,
    int         totalArgCount);

IRInst* createEmptyInstWithSize(
    IRModule*   module,
    IROp        op,
    size_t      totalSizeInBytes);

IRDecoration* createEmptyDecoration(
    IRModule* module, 
    IRDecorationOp op, 
    size_t sizeInBytes);

template <typename T>
T* createEmptyDecoration(IRModule* module)
{
    return static_cast<T*>(createEmptyDecoration(module, IRDecorationOp(T::kDecorationOp), sizeof(T)));
}

}


#endif
