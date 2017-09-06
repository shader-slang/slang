// ir.h
#ifndef SLANG_IR_H_INCLUDED
#define SLANG_IR_H_INCLUDED

// This file defines the intermediate representation (IR) used for Slang
// shader code. This is a typed static single assignment (SSA) IR,
// similar in spirit to LLVM (but much simpler).
//

#include "type-layout.h"

// We need the definition of `BaseType` which currently belongs to the AST
#include "syntax.h"

namespace Slang {

struct IRFunc;
struct IRInst;
struct IRModule;
struct IRParentInst;
struct IRType;

typedef unsigned int IROpFlags;
enum : IROpFlags
{
    kIROpFlags_None = 0,

    // This op is a parent op
    kIROpFlag_Parent = 1 << 0,
};

enum IROp : uint16_t
{
    
#define INST(ID, MNEMONIC, ARG_COUNT, FLAGS)  \
    kIROp_##ID,

#include "ir-inst-defs.h"
};

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

// A use of another value/inst within an IR operation
struct IRUse
{
    // The value that is doing the using.
    IRInst* user;

    // The value that is being used
    IRInst* usedValue;

    // The next use of the same value
    IRUse*  nextUse;

    // A "link" back to where this use is referenced,
    // so that we can simplify updates.
    IRUse** prevLink;

    void init(IRInst* user, IRInst* usedValue);
};

enum IRDecorationOp : uint16_t
{
    kIRDecorationOp_HighLevelDecl,
    kIRDecorationOp_Layout,
};

// A "decoration" that gets applied to an instruction.
// These usually don't affect semantics, but are useful
// for preserving high-level source information.
struct IRDecoration
{
    // Next decoration attached to the same instruction
    IRDecoration* next;

    IRDecorationOp op;
};

typedef uint32_t IRInstID;

// In the IR, almost *everything* is an instruction,
// in order to make the representation as uniform as possible.
struct IRInst
{
    // The operation that this value represents
    IROp op;

    // A unique ID to represent the op when printing
    // (or zero to indicate that the value of this
    // op isn't special).
    IRInstID id;

    // The total number of arguments of this instruction
    // (including the type)
    uint32_t argCount;

    // The parent of this instruction.
    // This will often be a basic block, but we
    // allow instructions to nest in more general ways.
    IRParentInst*    parent;

    // The next and previous instructions in the same parent block
    IRInst*     nextInst;
    IRInst*     prevInst;

    // The first use of this value (start of a linked list)
    IRUse*      firstUse;

    // The linked list of decorations attached to this instruction
    IRDecoration* firstDecoration;

    IRDecoration* findDecorationImpl(IRDecorationOp op);

    template<typename T>
    T* findDecoration()
    {
        return (T*) findDecorationImpl(IRDecorationOp(T::kDecorationOp));
    }


    // The type of this value
    IRUse       type;

    IRType* getType() { return (IRType*) type.usedValue; }

    UInt getArgCount()
    {
        return argCount;
    }

    IRUse*      getArgs();

    IRInst* getArg(UInt index)
    {
        return getArgs()[index].usedValue;
    }
};

// This type alias exists because I waffled on the name for a bit.
// All existing uses of `IRValue` should move to `IRInst`
typedef IRInst IRValue;

class Decl;

// Associates an IR-level decoration with a source declaration
// in the high-level AST, that can be used to extract
// additional information that informs code emission.
struct IRHighLevelDeclDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_HighLevelDecl };

    Decl* decl;
};

// Associates an IR-level decoration with a source layout
struct IRLayoutDecoration : IRDecoration
{
    enum { kDecorationOp = kIRDecorationOp_Layout };

    Layout* layout;
};


typedef long long IRIntegerValue;
typedef double IRFloatingPointValue;

struct IRConstant : IRInst
{
    union
    {
        IRIntegerValue          intVal;
        IRFloatingPointValue    floatVal;

        // HACK: allows us to hash the value easily
        void*                   ptrData[2];
    } u;
};

// Representation of a type at the IR level.
// Such a type may not correspond to the high-level-language notion
// of a type as used by the front end.
//
// Note that types are instructions in the IR, so that operations
// may take type operands as easily as values.
struct IRType : IRInst
{
};

struct IRVectorType : IRType
{
    IRUse   elementType;
    IRUse   elementCount;

    IRType* getElementType() { return (IRType*) elementType.usedValue; }
    IRInst* getElementCount() { return elementCount.usedValue; }
};

struct IRFuncType : IRType
{
    IRUse resultType;
    // parameter tyeps are varargs...

    IRType* getResultType() { return (IRType*) resultType.usedValue; }
    UInt getParamCount()
    {
        return getArgCount() - 2;
    }
    IRType* getParamType(UInt index)
    {
        return (IRType*) getArg(2 + index);
    }
};

struct IRPtrType : IRType
{
    IRUse valueType;

    IRType* getValueType() { return (IRType*) valueType.usedValue; }
};

struct IRTextureType : IRType
{
    IRUse flavor;
    IRUse elementType;

    IRIntegerValue getFlavor() { return ((IRConstant*) flavor.usedValue)->u.intVal; }
    IRType* getElementType() { return (IRType*) elementType.usedValue; }
};

struct IRUniformBufferType : IRType
{
    IRUse elementType;
    IRType* getElementType() { return (IRType*) elementType.usedValue; }
};

struct IRConstantBufferType : IRUniformBufferType {};
struct IRTextureBufferType : IRUniformBufferType {};

struct IRCall : IRInst
{
    IRUse func;
};

struct IRLoad : IRInst
{
    IRUse ptr;
};

struct IRStructField;
struct IRFieldExtract : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.usedValue; }
    IRStructField* getField() { return (IRStructField*) field.usedValue; }
};

struct IRFieldAddress : IRInst
{
    IRUse   base;
    IRUse   field;

    IRInst* getBase() { return base.usedValue; }
    IRStructField* getField() { return (IRStructField*) field.usedValue; }
};


// A instruction that ends a basic block (usually because of control flow)
struct IRTerminatorInst : IRInst
{};

struct IRReturn : IRTerminatorInst
{};

struct IRReturnVal : IRReturn
{
    IRUse val;

    IRInst* getVal() { return val.usedValue; }
};

struct IRReturnVoid : IRReturn
{};

// A parent instruction contains a sequence of other instructions
//
struct IRParentInst : IRInst
{
    // The first and last instruction in the container (or NULL in
    // the case that the container is empty).
    //
    IRInst* firstChild;
    IRInst* lastChild;
};

struct IRStructField : IRInst
{
    IRType* getFieldType() { return (IRType*) type.usedValue; }

    IRStructField* getNextField() { return (IRStructField*) nextInst; }
};

struct IRStructDecl : IRParentInst
{
    IRStructField* getFirstField() { return (IRStructField*) firstChild; }
    IRStructField* getLastField() { return (IRStructField*) lastChild; }
};


// A basic block is a parent instruction that adds the constraint
// that all the children need to be "ordinary" instructions (so
// no function declarations, or nested blocks). We also expect
// that the previous/next instruction are always a basic block.
//
struct IRBlock : IRParentInst
{
    // Note that in a valid program, every block must end with
    // a "terminator" instruction, so these should be non-NULL,
    // and `last` should actually be an `IRTerminatorInst`.

    IRBlock* getPrevBlock() { return (IRBlock*) prevInst; }
    IRBlock* getNextBlock() { return (IRBlock*) nextInst; }
};

// A function parameter is represented by an instruction
// in the entry block of a function.
struct IRParam : IRInst
{
    IRParam* getNextParam();
};

struct IRVar : IRInst
{};

// A function is a parent to zero or more blocks of instructions.
//
// A function is itself a value, so that it can be a direct operand of
// an instruction (e.g., a call).
struct IRFunc : IRParentInst
{
    IRFuncType* getType() { return (IRFuncType*) type.usedValue; }

    IRType* getResultType() { return getType()->getResultType(); }
    UInt getParamCount() { return getType()->getParamCount(); }
    IRType* getParamType(UInt index) { return getType()->getParamType(index); }

    IRBlock* getFirstBlock() { return (IRBlock*) firstChild; }
    IRBlock* getLastBlock() { return (IRBlock*) lastChild; }

    IRParam* getFirstParam();
};

// A module is a parent to functions, global variables, types, etc.
struct IRModule : IRParentInst
{
    // The designated entry-point function, if any
    IRFunc*     entryPoint;

    // A special counter used to assign logical ids to instructions in this module.
    IRInstID    idCounter;
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

    int GetHashCode();
};
bool operator==(IRConstantKey const& left, IRConstantKey const& right);

struct SharedIRBuilder
{
    // The module that will own all of the IR
    IRModule*       module;

    Dictionary<IRInstKey,       IRInst*>    globalValueNumberingMap;
    Dictionary<IRConstantKey,   IRConstant*>    constantMap;
};

struct IRBuilder
{
    // Shared state for all IR builders working on the same module
    SharedIRBuilder*    shared;

    IRModule* getModule() { return shared->module; }

    // The parent instruction to add children to.
    IRParentInst*   parentInst;

    void addInst(IRParentInst* parent, IRInst* inst);
    void addInst(IRInst* inst);

    IRType* getBaseType(BaseType flavor);
    IRType* getBoolType();
    IRType* getVectorType(IRType* elementType, IRValue* elementCount);
    IRType* getTypeType();
    IRType* getVoidType();
    IRType* getBlockType();

    IRType* getIntrinsicType(
        IROp op,
        UInt argCount,
        IRValue* const* args);

    IRStructDecl* createStructType();
    IRStructField* createStructField(IRType* fieldType);

    IRType* getFuncType(
        UInt            paramCount,
        IRType* const*  paramTypes,
        IRType*         resultType);

    IRType* getPtrType(
        IRType* valueType);

    IRValue* getBoolValue(bool value);
    IRValue* getIntValue(IRType* type, IRIntegerValue value);
    IRValue* getFloatValue(IRType* type, IRFloatingPointValue value);

    IRInst* emitCallInst(
        IRType*         type,
        IRValue*        func,
        UInt            argCount,
        IRValue* const* args);

    IRInst* emitIntrinsicInst(
        IRType*         type,
        IntrinsicOp     intrinsicOp,
        UInt            argCount,
        IRValue* const* args);

    IRInst* emitConstructorInst(
        IRType*         type,
        UInt            argCount,
        IRValue* const* args);

    IRModule* createModule();

    IRFunc* createFunc();

    IRBlock* createBlock();
    IRBlock* emitBlock();

    IRParam* emitParam(
        IRType* type);

    IRVar* emitVar(
        IRType* type);

    IRInst* emitLoad(
        IRValue*    ptr);

    IRInst* emitFieldExtract(
        IRType*         type,
        IRValue*        base,
        IRStructField*  field);

    IRInst* emitFieldAddress(
        IRType*         type,
        IRValue*        basePtr,
        IRStructField*  field);

    IRInst* emitReturn(
        IRValue*    val);

    IRInst* emitReturn();

    IRDecoration* addDecorationImpl(
        IRInst*         inst,
        UInt            decorationSize,
        IRDecorationOp  op);

    template<typename T>
    T* addDecoration(IRInst* inst, IRDecorationOp op)
    {
        return (T*) addDecorationImpl(inst, sizeof(T), op);
    }

    template<typename T>
    T* addDecoration(IRInst* inst)
    {
        return (T*) addDecorationImpl(inst, sizeof(T), IRDecorationOp(T::kDecorationOp));
    }

    IRHighLevelDeclDecoration* addHighLevelDeclDecoration(IRInst* inst, Decl* decl);
    IRLayoutDecoration* addLayoutDecoration(IRInst* inst, Layout* layout);
};

void dumpIR(IRModule* module);

}

#endif
