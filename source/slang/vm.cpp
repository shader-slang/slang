#include "vm.h"

// Implementation of the Slang bytecode VM
//

#include "bytecode.h"
#include "ir.h"

#include "../../slang.h"

namespace Slang
{

// Representation of a type during VM execution
struct VMTypeImpl
{
    uint32_t op;

    // Size and alignment of instances of this
    // type.
    UInt    size;
    UInt    alignment;
};
struct VMType
{
    VMTypeImpl*     impl;

    UInt getSize() { return impl->size; }
    UInt getAlignment() { return impl->alignment;  }
};

struct VMPtrTypeImpl : VMTypeImpl
{
    VMType base;
    uint32_t addressSpace;
};

struct VMReg
{
    // Type that the register is meant to hold
    VMType  type;

    // offset of the variable inside the frame
    size_t  offset;
};

struct VMConst
{
    // Type of the constant
    VMType type;

    // Operand address to use
    void*   ptr;
};

struct VMFrame;

// Information about a function after it has been
// loaded into the VM.
struct VMFunc
{
    // The parent module that this function belongs to
    VMFrame*    module;
    BCFunc*     bcFunc;

    VMReg*      regs;
    VMConst*    consts;

    size_t      frameSize;
};

struct VMFrame
{
    // The function from which this frame was spawned
    VMFunc*     func;

    // The parent frame on the call stack of the
    // current thread.
    VMFrame*    parent;

    // The instruction pointer within this frame
    BCOp*   ip;

    // Registers are stored after this point.
};

struct VMModule : VMFunc
{
};

UInt decodeUInt(BCOp** ioPtr)
{
    BCOp* ptr = *ioPtr;

    UInt value = *ptr++;
    if( value >= 128 )
    {
        SLANG_UNEXPECTED("deal with this later");
    }

    *ioPtr = ptr;
    return value;
}

Int decodeSInt(BCOp** ioPtr)
{
    UInt uVal = decodeUInt(ioPtr);
    if( uVal & 1 )
    {
        return Int(~(uVal >> 1));
    }
    else
    {
        return Int(uVal >> 1);
    }
}

void* getRegPtrImpl(VMFrame* frame, UInt id)
{
    VMFunc* vmFunc = frame->func;
    VMReg* vmReg = &vmFunc->regs[id];
    size_t offset = vmReg->offset;

    return (void*)((char*)frame + offset);
}

void* getOperandPtrImpl(VMFrame* frame, Int id)
{
    if( id >= 0 )
    {
        // This ID represents a local variable/register
        // of the current call frame, and should
        // be used to index into a table of such values.

        return getRegPtrImpl(frame, id);
    }
    else
    {
        // This ID represents a global value that has
        // been imported into the current func, and
        // should be looked up via indirection into
        // the current module.

        VMFunc* vmFunc = frame->func;
        VMConst* vmConst = &vmFunc->consts[~id];
        return vmConst->ptr;
    }
}

VMType getOperandTypeImpl(VMFrame* frame, Int id)
{
    if( id >= 0 )
    {
        return frame->func->regs[id].type;
    }
    else
    {
        return frame->func->consts[~id].type;
    }
}
struct VMPtrAndType
{
    void*   ptr;
    VMType  type;
};

VMPtrAndType decodeOperandPtrAndType(VMFrame* frame, BCOp** ioIP)
{
    Int id = decodeSInt(ioIP);

    VMPtrAndType ptrAndType;
    ptrAndType.ptr = getOperandPtrImpl(frame, id);
    ptrAndType.type = getOperandTypeImpl(frame, id);
    return ptrAndType;
}

void* decodeOperandPtrImpl(VMFrame* frame, BCOp** ioIP)
{
    Int id = decodeSInt(ioIP);
    return getOperandPtrImpl(frame, id);
}


template<typename T>
T* decodeOperandPtr(VMFrame* frame, BCOp** ioIP)
{
    return (T*)decodeOperandPtrImpl(frame, ioIP);
}


template<typename T>
T& decodeOperand(VMFrame* frame, BCOp** ioIP)
{
    return *decodeOperandPtr<T>(frame, ioIP);
}

VMFunc* loadVMFunc(
    BCFunc*     bcFunc,
    VMFrame*    vmModuleInstance);

struct VMSizeAlign
{
    UInt size;
    UInt align;
};

VMSizeAlign getVMSymbolSize(BCSymbol* symbol)
{
    VMSizeAlign result;
    result.size = sizeof(void*);
    result.align = sizeof(void*);
    switch( symbol->op )
    {
    default:
        SLANG_UNEXPECTED("op");
        break;

    case kIROp_TypeType:
        break;

    case kIROp_Func:
        {
            BCFunc* func = (BCFunc*) symbol;
            result.size = sizeof(VMFunc)
                + func->regCount   * sizeof(VMReg)
                + func->constCount * sizeof(VMConst);
        }
        break;
    }

    return result;
}

VMFunc* loadVMFunc(
    BCFunc*     bcFunc,
    VMFrame*    vmModuleInstance)
{
    UInt regCount = bcFunc->regCount;
    UInt constCount = bcFunc->constCount;
    UInt vmFuncSize = sizeof(VMFunc)
        + regCount * sizeof(VMReg)
        + constCount * sizeof(VMConst);

    VMFunc* vmFunc = (VMFunc*) malloc(vmFuncSize);
    VMReg* vmRegs = (VMReg*) (vmFunc + 1);
    VMConst* vmConsts = (VMConst*) (vmRegs + regCount);

    vmFunc->module = vmModuleInstance;
    vmFunc->bcFunc = bcFunc;
    vmFunc->regs = vmRegs;
    vmFunc->consts = vmConsts;

    UInt offset = sizeof(VMFrame);
    for( UInt rr = 0; rr < regCount; ++rr )
    {
        BCReg* bcReg = &bcFunc->regs[rr];
        auto typeGlobalID = bcReg->typeGlobalID;

        // HACK: when we are loading a module itself, we might
        // not yet know the size for the things it defines
        // (since the module itself might define the type of
        // one of its symbols), so for now we hack it and
        // assume everything at module level is 16 bytes or less.
        //
        // TODO: this also seems like it will cause problems
        // in other contexts (any time the type of a register
        // would depend on an earlier instruction in the same
        // scope) so this needs careful thought.
        VMType vmType = { nullptr };
        UInt regSize = 16;
        UInt regAlign = 8;

        if (vmModuleInstance)
        {
            // We expect the type to come from the outer module, so
            // that we can allocate space for it as we go.
            vmType = *(VMType*)getRegPtrImpl(vmModuleInstance, typeGlobalID);

            regSize = vmType.getSize();
            regAlign = vmType.getAlignment();
        }

        offset = (offset + (regAlign-1)) & ~(regAlign-1);

        size_t regOffset = offset;
        offset += regSize;

        vmRegs[rr].type = vmType;
        vmRegs[rr].offset = regOffset;
    }
    vmFunc->frameSize = offset;

    for( UInt cc = 0; cc < constCount; ++cc )
    {
        auto globalID = bcFunc->consts[cc].globalID;
        auto globalPtr = getRegPtrImpl(vmModuleInstance, globalID);
        vmFunc->consts[cc].ptr = globalPtr;
        vmFunc->consts[cc].type = vmModuleInstance->func->regs[globalID].type;
    }

    return vmFunc;
}

VMFrame* createFrame(VMFunc* vmFunc)
{
    VMFrame* vmFrame = (VMFrame*) malloc(vmFunc->frameSize);
    vmFrame->func = vmFunc;
    vmFrame->ip = vmFunc->bcFunc->blocks[0].code;
    return vmFrame;
}

void dumpVMFrame(VMFrame* vmFrame)
{
    fflush(stderr);

    // We want to walk the VM frame and dump the
    // state of all of its logical registers.
    // For now this is made easier by having
    // no overlapping register assignments...
    VMFunc* vmFunc = vmFrame->func;
    BCFunc* bcFunc = vmFunc->bcFunc;
    UInt regCount = bcFunc->regCount;

    for (UInt rr = 0; rr < regCount; ++rr)
    {
        VMType regType = getOperandTypeImpl(vmFrame, rr);
        void* regData = getRegPtrImpl(vmFrame, rr);

        char const* name = bcFunc->regs[rr].name;

        // Use the type to print the data...

        fprintf(stderr, "0x%p: ", regData);

        fprintf(stderr, "%%%u ", rr);
        if (name)
        {
            fprintf(stderr, "\"%s\" ", name);
        }
        if (regType.impl)
        {
            switch (regType.impl->op)
            {
            case kIROp_TypeType:
                // TODO: we could recursively go and print types...
                fprintf(stderr, ": Type = ???");
                break;

            case kIROp_readWriteStructuredBufferType:
                fprintf(stderr, ": RWStructuredBuffer<???> = ???");
                break;

            case kIROp_structuredBufferType:
                fprintf(stderr, ": StructuredBuffer<???> = ???");
                break;

            case kIROp_BoolType:
                fprintf(stderr, ": Bool = %s", *(bool*)regData ? "true" : "false");
                break;

            case kIROp_Int32Type:
                fprintf(stderr, ": Int32 = %d", *(int32_t*)regData);
                break;

            case kIROp_UInt32Type:
                fprintf(stderr, ": UInt32 = %u", *(uint32_t*)regData);
                break;

            case kIROp_PtrType:
                {
                    fprintf(stderr, ": Ptr<???> = 0x%p", *(void**)regData);
                }
                break;

            default:
                fprintf(stderr, "<unknown>");
                break;
            }
        }
        else
        {
            fprintf(stderr, ": <null>");
        }
        fprintf(stderr, "; // ");
        fprintf(stderr, "%s", getIROpInfo((IROp) bcFunc->regs[rr].op).name);
        fprintf(stderr, "\n");

        // Okay, now we need to use the type
        // stored in the VM register to tell
        // us how to print things.
    }

    IROp op = IROp(*vmFrame->ip);
    IROpInfo opInfo = getIROpInfo(op);
    fprintf(stderr, "NEXT op: %s\n", opInfo.name);

    fflush(stderr);
}

struct VM
{
};

VM* createVM()
{
    VM* vm = new VM();
    return vm;
}

struct VMThread
{
    // The currently executing call frame
    VMFrame*    frame;
};

void resumeThread(
    VMThread*   vmThread);

VMFrame* loadVMModuleInstance(
    VM*         vm,
    void const* bytecode,
    size_t      bytecodeSize)
{
    BCHeader* bcHeader = (BCHeader*) bytecode;

    BCModule* bcModule = bcHeader->module;

    UInt vmModuleSize = sizeof(VMModule) + bcModule->regCount * sizeof(VMReg);

    VMModule* vmModule = (VMModule*) loadVMFunc(bcModule, nullptr);

    // Create a frame to store the loaded symbols, and execute it
    // to initialize them.
    VMFrame* vmModuleInstance = createFrame(vmModule);
    vmModuleInstance->parent = nullptr;
    vmModule->module = vmModuleInstance;


    VMThread thread;
    thread.frame = vmModuleInstance;

    resumeThread(&thread);

    return vmModuleInstance;
}

void* findGlobalSymbolPtr(
    VMFrame*    moduleInstance,
    char const* name)
{
    // Okay, we need to search through the available
    // "registers" looking for one that gives us a
    // name match.
    //
    BCFunc* bcFunc = moduleInstance->func->bcFunc;
    UInt regCount = bcFunc->regCount;
    for (UInt rr = 0; rr < regCount; ++rr)
    {
        BCReg* bcReg = &bcFunc->regs[rr];

        char const* symbolName = bcReg->name;
        if (!symbolName)
            continue;

        if(strcmp(symbolName, name) == 0)
            return getRegPtrImpl(moduleInstance, rr);
    }

    return nullptr;
}

VMThread* createThread(
    VM* vm)
{
    VMThread* thread = new VMThread();
    thread->frame = nullptr;
    return thread;
}

void beginCall(
    VMThread*   vmThread,
    VMFunc*     vmFunc)
{
    VMFrame* vmFrame = createFrame(vmFunc);

    vmFrame->parent = vmThread->frame;
    vmThread->frame = vmFrame;
}

void setArg(
    VMThread*   vmThread,
    UInt        argIndex,
    void const* data,
    size_t      size)
{
    // TODO: need all kinds of validation here...

    void* dest = getRegPtrImpl(vmThread->frame, argIndex);
    memcpy(dest, data, size);
}

void resumeThread(
    VMThread*   vmThread)
{
    auto frame = vmThread->frame;
    auto ip = frame->ip;

    for(;;)
    {
#if 0
        // debugging:
        frame->ip = ip;
        dumpVMFrame(frame);
#endif

        auto op = (IROp) decodeUInt(&ip);
        switch( op )
        {
        case kIROp_TypeType:
            {
                // The type of types
                Int argCount = decodeUInt(&ip);
                void* arg0Ptr = decodeOperandPtr<void>(frame, &ip);
                VMType* destPtr = decodeOperandPtr<VMType>(frame, &ip);

                auto typeImpl = new VMTypeImpl();
                typeImpl->op = op;
                typeImpl->size = sizeof(VMType);
                typeImpl->alignment = alignof(VMType);

                VMType type = { typeImpl };
                *destPtr = type;
            }
            break;

        case kIROp_BlockType:
        case kIROp_Int32Type:
        case kIROp_UInt32Type:
        case kIROp_Float32Type:
        case kIROp_BoolType:
        case kIROp_VoidType:
        case kIROp_FuncType: // TODO: we should in principle handle function types here
            {
                // Case to handle types without arguments.
                UInt argCount = decodeUInt(&ip);
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                }
                VMType* destPtr = decodeOperandPtr<VMType>(frame, &ip);

                auto typeImpl = new VMTypeImpl();
                typeImpl->op = op;

                UInt size = 1;
                UInt align = 0;
                switch (op)
                {
                case kIROp_BlockType:   size = sizeof(void*);       break;
                case kIROp_Int32Type:   size = sizeof(int32_t);     break;
                case kIROp_UInt32Type:  size = sizeof(uint32_t);    break;
                case kIROp_Float32Type: size = sizeof(float);       break;
                case kIROp_BoolType:    size = sizeof(bool);        break;
                case kIROp_VoidType:    size = 0; align = 1;        break;
                default:
                    break;
                }
                if (!align) align = size;
                typeImpl->size = size;
                typeImpl->alignment = align;

                VMType type = { typeImpl };
                *destPtr = type;
            }
            break;

        case kIROp_PtrType:
            {
                // Case to handle types without arguments.
                UInt argCount = decodeUInt(&ip);
                VMType* typeTypePtr = decodeOperandPtr<VMType>(frame, &ip);
                VMType baseType = decodeOperand<VMType>(frame, &ip);
                int32_t addressSpace = decodeOperand<int32_t>(frame, &ip);
                VMType* destPtr = decodeOperandPtr<VMType>(frame, &ip);



                auto typeImpl = new VMPtrTypeImpl();
                typeImpl->op = op;
                typeImpl->size = sizeof(void*);
                typeImpl->alignment = alignof(void*);
                typeImpl->base = baseType;
                typeImpl->addressSpace = addressSpace;

                VMType type = { typeImpl };
                *destPtr = type;
            }
            break;

        case kIROp_readWriteStructuredBufferType:
        case kIROp_structuredBufferType:
            {
                // Case to handle types without arguments.
                UInt argCount = decodeUInt(&ip);
                VMType* typeTypePtr = decodeOperandPtr<VMType>(frame, &ip);
                VMType baseType = decodeOperand<VMType>(frame, &ip);
                VMType* destPtr = decodeOperandPtr<VMType>(frame, &ip);


                // TODO: give these their own representations!
                auto typeImpl = new VMPtrTypeImpl();
                typeImpl->op = op;
                typeImpl->base = baseType;
                typeImpl->size = sizeof(void*);
                typeImpl->alignment = sizeof(void*);

                VMType type = { typeImpl };
                *destPtr = type;
            }
            break;


        case kIROp_IntLit:
            {
                VMType type = decodeOperand<VMType>(frame, &ip);
                UInt uVal = decodeUInt(&ip);
                void* destPtr = decodeOperandPtr<void>(frame, &ip);

                switch( type.impl->op )
                {
                case kIROp_Int32Type:
                    *(int32_t*)destPtr = int32_t(uVal);
                    break;

                case kIROp_UInt32Type:
                    *(uint32_t*)destPtr = uint32_t(uVal);
                    break;

                default:
                    SLANG_UNEXPECTED("integer type");
                    break;
                }

            }
            break;

        case kIROp_FloatLit:
            {
                VMType type = decodeOperand<VMType>(frame, &ip);

                static const UInt size = sizeof(IRFloatingPointValue);
                IRFloatingPointValue value;
                memcpy(&value, ip, size);
                ip += size;
                void* destPtr = decodeOperandPtr<void>(frame, &ip);

                switch( type.impl->op )
                {
                case kIROp_Float32Type:
                    *(float*)destPtr = float(value);
                    break;

                default:
                    SLANG_UNEXPECTED("float type");
                    break;
                }
            }
            break;

        case kIROp_boolConst:
            {
                bool val = (*ip++) != 0;
                bool* destPtr = decodeOperandPtr<bool>(frame, &ip);
                *destPtr = val;
            }
            break;

        case kIROp_Func:
            {
                UInt nestedID = decodeUInt(&ip);
                void* destPtr = decodeOperandPtr<void>(frame, &ip);

                BCSymbol* bcSymbol = frame->func->bcFunc->nestedSymbols[nestedID];
                BCFunc* bcFunc = (BCFunc*)bcSymbol;
                VMFunc* vmFunc = loadVMFunc(bcFunc, frame->func->module);

                *(VMFunc**)destPtr = vmFunc;
            }
            break;

        case kIROp_Var:
            {
                // This instruction represents the `alloca` for a variable of some type.

                UInt argCount = decodeUInt(&ip);
                void* argPtrs[16] = { 0 };
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                    argPtrs[aa] = argPtr;
                }

                void** destPtr = decodeOperandPtr<void*>(frame, &ip);

                // For now this is a bit silly and simple: the
                // storage for the variable we are "allocating"
                // should be right after outer destination, so we can
                // set it pretty easily.

                *destPtr = destPtr + 1;
            }
            break;

        case kIROp_Store:
            {
                // An ordinary memory store
                VMType type = decodeOperand<VMType>(frame, &ip);
                void* dest = decodeOperand<void*>(frame, &ip);
                void* src = decodeOperandPtr<void>(frame, &ip);

                memcpy(dest, src, type.getSize());
            }
            break;

        case kIROp_Load:
            {
                // An ordinary memory store
                VMType type = decodeOperand<VMType>(frame, &ip);
                void* src = decodeOperand<void*>(frame, &ip);
                void* dest = decodeOperandPtr<void>(frame, &ip);

                memcpy(dest, src, type.getSize());
            }
            break;

        case kIROp_BufferLoad:
            {
                UInt argCount = decodeUInt(&ip);
                void* argPtrs[16] = { 0 };
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                    argPtrs[aa] = argPtr;
                }

                void* dest = decodeOperandPtr<void>(frame, &ip);

                VMType type = *(VMType*)argPtrs[0];
                char* bufferData = *(char**)argPtrs[1];
                uint32_t index = *(uint32_t*)argPtrs[2];

                auto size = type.getSize();
                char* elementData = bufferData + index*size;
                memcpy(dest, elementData, size);
            }
            break;

        case kIROp_BufferStore:
            {
                UInt argCount = decodeUInt(&ip);

                VMType* resultTypePtr = decodeOperandPtr<VMType>(frame, &ip);
                char* bufferData = decodeOperand<char*>(frame, &ip);
                uint32_t index = decodeOperand<uint32_t>(frame, &ip);

                auto srcPtrAndType = decodeOperandPtrAndType(frame, &ip);
                void* srcPtr = srcPtrAndType.ptr;
                VMType type = srcPtrAndType.type;

                auto size = type.getSize();
                char* elementData = bufferData + index*size;
                memcpy(elementData, srcPtr, size);
            }
            break;
        case kIROp_Call:
            {
                UInt operandCount = decodeUInt(&ip);
                VMType type  = decodeOperand<VMType>(frame, &ip);
                VMFunc* func = decodeOperand<VMFunc*>(frame, &ip);

                // Okay, we need to create a frame to prepare the call
                VMFrame* newFrame = createFrame(func);
                newFrame->parent = frame;

                // Remaining arguments should populate the
                // first N registers of the callee
                UInt argCount = operandCount - 2;
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                    void* regPtr = getRegPtrImpl(newFrame, aa);

                    VMType regType = func->regs[aa].type;
                    memcpy(regPtr, argPtr, regType.getSize());
                }

                // Note that we do *not* try to read off
                // the destination operand, and instead
                // leave that to be done during the
                // return sequence.
                //

                // Save the IP we were using in teh current function.
                //
                frame->ip = ip;

                // Now switch over to the callee:
                //
                frame = newFrame;
                ip = newFrame->ip;
            }
            break;

        case kIROp_ReturnVoid:
            {
                // Easy case: just jump to the parent frame,
                // without having to worry about operands.
                VMFrame* newFrame = frame->parent;
                vmThread->frame = newFrame;

                // HACK: we need to know when we are done.
                // TODO: We should probably have the bottom
                // of the stack for a thread always point
                // to a special bytecode sequence that
                // forces a `yield` op that can handle
                // the exit from the interpreter, rather
                // than always take a branch here.
                if (!newFrame)
                    return;

                frame = newFrame;
                ip = frame->ip;

            }
            break;

        case kIROp_ReturnVal:
            {
                UInt argCount = decodeUInt(&ip);
                void* typePtr = decodeOperandPtr<void>(frame, &ip);
                void* argPtr = decodeOperandPtr<void>(frame, &ip);

                VMFrame* oldFrame = frame;
                VMFrame* newFrame = frame->parent;
                vmThread->frame = newFrame;

                // Note: see the comments above about
                // this branch.
                if (!newFrame)
                    return;

                frame = newFrame;
                ip = frame->ip;

                auto destPtrAndType = decodeOperandPtrAndType(frame, &ip);
                void* destPtr = destPtrAndType.ptr;
                VMType type = destPtrAndType.type;

                memcpy(destPtr, argPtr, type.getSize());
            }
            break;

        case kIROp_loop:
        case kIROp_unconditionalBranch:
        case kIROp_break:
        case kIROp_continue:
            {
                // For now our encoding is very regular, so we can decode without
                // knowing too much about an instruction...

                UInt argCount = decodeUInt(&ip);
                void* typePtr = decodeOperandPtr<void>(frame, &ip);
                Int destinationBlock = decodeSInt(&ip);
                for( UInt aa = 2; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                }

                // TODO: we need to deal with the case of
                // passing arguments to the block, which
                // means copying between the registers...
                //
                ip = frame->func->bcFunc->blocks[destinationBlock].code;
            }
            break;

        case kIROp_loopTest:
        case kIROp_if:
        case kIROp_ifElse:
        case kIROp_conditionalBranch:
            {
                // For now our encoding is very regular, so we can decode without
                // knowing too much about an instruction...

                UInt argCount = decodeUInt(&ip);
                void* typePtr = decodeOperandPtr<void>(frame, &ip);
                bool* condition = decodeOperandPtr<bool>(frame, &ip);
                Int trueBlockID = decodeSInt(&ip);
                Int falseBlockID = decodeSInt(&ip);
                for( UInt aa = 4; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                }

                Int destinationBlock = *condition ? trueBlockID : falseBlockID;

                // TODO: we need to deal with the case of
                // passing arguments to the block, which
                // means copying between the registers...
                //
                ip = frame->func->bcFunc->blocks[destinationBlock].code;
            }
            break;

        case kIROp_Greater:
            {
                // For now our encoding is very regular, so we can decode without
                // knowing too much about an instruction...

                UInt argCount = decodeUInt(&ip);
                void* argPtrs[16] = { 0 };
                VMType resultType = decodeOperand<VMType>(frame, &ip);
                auto leftOpnd = decodeOperandPtrAndType(frame, &ip);
                auto type = leftOpnd.type;
                auto leftPtr = leftOpnd.ptr;
                void* rightPtr = decodeOperandPtr<void>(frame, &ip);

                bool* destPtr = decodeOperandPtr<bool>(frame, &ip);

                switch (type.impl->op)
                {
                case kIROp_Int32Type:
                    *destPtr = *(int32_t*)leftPtr > *(int32_t*)rightPtr;
                    break;

                default:
                    SLANG_UNEXPECTED("comparison op case");
                    break;
                }
            }
            break;

        case kIROp_Mul:
            {
                UInt argCount = decodeUInt(&ip);
                VMType type = decodeOperand<VMType>(frame, &ip);
                void* leftPtr = decodeOperandPtr<void>(frame, &ip);
                void* rightPtr = decodeOperandPtr<void>(frame, &ip);

                void* destPtr = decodeOperandPtr<void>(frame, &ip);

                switch (type.impl->op)
                {
                case kIROp_Int32Type:
                    *(int32_t*)destPtr = *(int32_t*)leftPtr * *(int32_t*)rightPtr;
                    break;

                default:
                    SLANG_UNEXPECTED("comparison op case");
                    break;
                }
            }
            break;

        case kIROp_Sub:
            {
                UInt argCount = decodeUInt(&ip);
                VMType type = decodeOperand<VMType>(frame, &ip);
                void* leftPtr = decodeOperandPtr<void>(frame, &ip);
                void* rightPtr = decodeOperandPtr<void>(frame, &ip);

                void* destPtr = decodeOperandPtr<void>(frame, &ip);

                switch (type.impl->op)
                {
                case kIROp_Int32Type:
                    *(int32_t*)destPtr = *(int32_t*)leftPtr - *(int32_t*)rightPtr;
                    break;

                default:
                    SLANG_UNEXPECTED("comparison op case");
                    break;
                }
            }
            break;

        default:
            {
                // For now our encoding is very regular, so we can decode without
                // knowing too much about an instruction...

                UInt argCount = decodeUInt(&ip);
                void* argPtrs[16] = { 0 };
                for( UInt aa = 0; aa < argCount; ++aa )
                {
                    void* argPtr = decodeOperandPtr<void>(frame, &ip);
                    argPtrs[aa] = argPtr;
                }

                SLANG_UNEXPECTED("unknown bytecode op");
                return;
            }
            break;
        }
    }

}




SLANG_API void SlangVMThread_resume(
    SlangVMThread*  thread)
{
    Slang::resumeThread(
        (Slang::VMThread*)  thread);
}

} // namespace Slang

SLANG_API SlangVM* SlangVM_create()
{
    return (SlangVM*) Slang::createVM();
}

SLANG_API SlangVMModule* SlangVMModule_load(
    SlangVM*    vm,
    void const* bytecode,
    size_t      bytecodeSize)
{
    return (SlangVMModule*) Slang::loadVMModuleInstance(
        (Slang::VM*) vm,
        bytecode,
        bytecodeSize);
}

SLANG_API void* SlangVMModule_findGlobalSymbolPtr(
    SlangVMModule*  module,
    char const*     name)
{
    return (SlangVMFunc*) Slang::findGlobalSymbolPtr(
        (Slang::VMFrame*) module,
        name);
}

SLANG_API SlangVMThread* SlangVMThread_create(
    SlangVM*    vm)
{
    return (SlangVMThread*)Slang::createThread(
        (Slang::VM*) vm);
}

SLANG_API void SlangVMThread_beginCall(
    SlangVMThread*  thread,
    SlangVMFunc*    func)
{
    Slang::beginCall(
        (Slang::VMThread*)  thread,
        (Slang::VMFunc*)    func);
}

SLANG_API void SlangVMThread_setArg(
    SlangVMThread*  thread,
    SlangUInt       argIndex,
    void const*     data,
    size_t          size)
{
    Slang::setArg(
        (Slang::VMThread*)  thread,
        argIndex,
        data,
        size);
}

SLANG_API void SlangVMThread_resume(
    SlangVMThread*  thread)
{
    Slang::resumeThread(
        (Slang::VMThread*)  thread);
}
