// bytecode.h
#ifndef SLANG_BYTECODE_H_INCLUDED
#define SLANG_BYTECODE_H_INCLUDED

// This file defines a "bytecode" format for storing shader code
// that has been generated via the Slang IR. The bytecode has
// two main goals, that can end up in a bit of conflict:
//
// 1) It is the official serialized form of the Slang IR, and
//    so it is of some importance that constructs in the IR be
//    able to round-trip through the bytecode.
//
// 2) It should support being directly executed/interpreted,
//    so that Slang code can be executed on CPUs when
//    performance isn't critical (or when a JIT just isn't
//    an option).
//

#include "../core/basic.h"

namespace Slang
{
template<typename T>
struct BytecodeGenerationPtr;

// A "pointer" stored in a serialized bytecode file, which
// is represented as a byte offset relative to itself.
//
template<typename T>
struct BCPtr
{
    typedef int32_t RawVal;

    RawVal rawVal;

    BCPtr() : rawVal(0) {}

    BCPtr(T* ptr)
        : rawVal(0)
    {
        *this = ptr;
    }

    BCPtr(BCPtr<T> const& ptr)
        : rawVal(0)
    {
        *this = ptr.getPtr();
    }

    void operator=(BCPtr<T> const& ptr)
    {
        *this = ptr.getPtr();
    }

    void operator=(T* ptr)
    {
        if (ptr)
        {
            rawVal = (RawVal)((char*)ptr - (char*)this);
        }
        else
        {
            rawVal = 0;
        }
    }

    operator T*() const { return getPtr(); }
    T* operator->() const { return getPtr(); }

    T* getPtr() const
    {
        if(!rawVal) return nullptr;
        return (T*)((char const*)this + rawVal);
    }
};

// Representation of a "type-level" value in
// the bytecode fiel. This corresponds to
// the AST-level notion of a `Val`
struct BCVal
{
    // The opcode used to define this value
    uint32_t op;

    // The ID of the type within its module
    uint32_t id;
};

struct BCType : BCVal
{
    // TODO: avoid having to encode this?
    uint32_t argCount;

    // type-specific operands follow

    //

    BCPtr<BCVal>* getArgs() { return (BCPtr<BCVal>*) (this +1); }

    BCVal* getArg(UInt index) { return getArgs()[index]; }
};

struct BCPtrType : BCType
{
    BCPtr<BCType>   valueType;
};

struct BCFuncType : BCType
{
    BCPtr<BCType>   resultType;
    BCPtr<BCType>   paramTypes[1];

    BCType* getResultType() { return resultType; }

    UInt getParamCount() { return argCount - 1; }
    BCType* getParamType(UInt index) { return paramTypes[index]; }
};

struct BCConstant : BCVal
{
    uint32_t        typeID;
    BCPtr<uint8_t>  ptr;
};

struct BCSymbol
{
    // The opcode that was used to define
    // this symbol; used to categorize things
    uint32_t op;

    // The index (in the module's type table)
    // of the type of the symbol:
    uint32_t typeID;

    // The name of this symbol (which might
    // be a mangled name at some point,
    // so it is really only meant to be
    // used for linkage...)
    BCPtr<char> name;
};

typedef uint8_t BCOp;

struct BCReg : BCSymbol
{
    // The index of the variable/register
    // that should be stored immediately
    // preceding this one.
    uint32_t    previousVarIndexPlusOne;
};

enum BCConstFlavor
{
    kBCConstFlavor_GlobalSymbol,
    kBCConstFlavor_Constant,
};

struct BCConst
{
    // The flavor of bytecode constant we
    // are dealing with.
    uint32_t flavor;
    uint32_t id;
};

struct BCBlock
{
    // The start of the bytecode for this block
    BCPtr<BCOp> code;

    // The list of parameters of the block
    uint32_t        paramCount;
    BCPtr<BCReg>    params;
};

struct BCFunc : BCSymbol
{
    // A list of "registers" used to hold
    // intermediate values during execution
    // of this function.
    uint32_t        regCount;
    BCPtr<BCReg>    regs;

    // The basic blocks of the function
    uint32_t        blockCount;
    BCPtr<BCBlock>  blocks;

    // A list of "constants" which are values
    // from the global scope that this function
    // wants to be able to refer to. We could
    // just encode global values directly,
    // but this would make the encoding less dense.
    uint32_t        constCount;
    BCPtr<BCConst>  consts;
};

struct BCModule
{
    // The symbols (functions, global variables, etc.)
    // that have been declared in the module.
    uint32_t                symbolCount;
    BCPtr<BCPtr<BCSymbol>>  symbols;

    // The types that are used by this module, stored
    // in a single array so that they can be conveniently
    // mapped to another representation in one go.
    //
    // Instructions in a bytecode instruction sequence
    // might reference these types by index.
    uint32_t                typeCount;
    BCPtr<BCPtr<BCType>>    types;

    // True compile-time constants go here:
    uint32_t                constantCount;
    BCPtr<BCConstant>       constants;
};

struct BCHeader
{
    char                magic[8];
    uint32_t            version;

    // TODO: probably want a section-based file
    // format so that we can add/remove different
    // kinds of data without having to revise
    // the schema here.

    // TODO: should include AST declaration structure
    // here, which can be used for refleciton, and
    // also loaded to resolve dependencies when
    // compiling other modules.

    // TODO: Include the original entry point requests?

    // Zero or more IR modules, corresponding to
    // the translation units of the original compile
    // request.
    uint32_t                moduleCount;
    BCPtr<BCPtr<BCModule>>  modules;

    // TODO: should enumerate targets here, and
    // include reflection layout info + compiled
    // entry points for each target.
};

class CompileRequest;
void generateBytecodeForCompileRequest(
    CompileRequest* compileReq);

}


#endif
