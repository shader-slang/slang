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
            rawVal = (char*)ptr - (char*)this;
        }
        else
        {
            rawVal = 0;
        }
    }

    operator T*() const { return getPtr(); }

    T* getPtr() const
    {
        if(!rawVal) return nullptr;
        return (T*)((char const*)this + rawVal);
    }
};

struct BCType
{
    uint32_t op;
};

struct BCSymbol
{
    // The opcode that was used to define
    // this symbol; used to categorize things
    uint32_t op;

    // The type of the symbol is represent
    // as an index into the global-scope symbol
    // list of the module.
    //
    // Note: This currently precludes having
    // a register with a type that is not
    // statically determined, but that is
    // probably okay.
    uint32_t   typeGlobalID;

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

struct BCConst
{
    // The ID of the symbol in the global
    // scope that we are trying to refer
    // to.
    //
    // TODO: eventually, if we have general
    // nesting, then this might be the
    // entry in the outer scope that
    // is being referenced.
    uint32_t    globalID;
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

    // Data for "nested" symbols (e.g., a function
    // nested inside this function).
    uint32_t                    nestedSymbolCount;
    BCPtr<BCPtr<BCSymbol>>      nestedSymbols;
};

// A module is encoded more or less like a function.
struct BCModule : BCFunc
{
};

struct BCHeader
{
    char                magic[8];
    uint32_t            version;

    // TODO: probably want a section-based file
    // format so that we can add/remove different
    // kinds of data without having to revise
    // the schema here.

    // The bytecode representation of the module
    BCPtr<BCModule>     module;
};



}


#endif
