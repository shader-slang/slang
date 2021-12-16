// slang-ir-wrap-structured-buffers.cpp
#include "slang-ir-wrap-structured-buffers.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

// Suppose a programmer wrote HLSL/Slang code like this:
//
//      StructuredBuffer<float4x4> gBuffer;
//
//      ... gBuffer[index] ...
//      ... gBuffer.Load(index) ...
//
// Further suppose that they specified that row-major
// matrix layout should be used. It might surprise such
// a user that the fxc and dxc compilers both use
// column-major layout for the matrices in `gBuffer`,
// and there is no way to change that fact.
//
// In contrast, we want Slang to respect the matrix layout
// setting given by the user in all cases, so we need to
// find a way to force fxc and dxc to do what we want.
//
// Fortunately, fxc and dxc *do* respect the row-major
// layout setting for code like the following:
//
//      struct Wrapper { float4x4 wrapped; }
//      StructuredBuffer<Wrapper> gBuffer;
//
//      ... gBuffer[index].wrapped ...
//      ... gBuffer.Load(index).wrapped ...
//
// The role of the pass in this file is to transform IR
// for code like the first case into IR for code like the
// second case.
//
// The main thing that makes this pass challenging is
// recognizing calls to `StructuredBuffer<T>` builtins
// so that we can transform them to append the `.wrapped`
// field reference.

// As is typical, we will wrap up our IR pass in a
// "context" structure type.
//
struct WrapStructuredBuffersContext
{
    IRModule* m_module = nullptr;

    // We process a module by processing all its instructions, recursively.
    //
    void processModule()
    {
        processInstRec(m_module->getModuleInst());
    }

    void processInstRec(IRInst* inst)
    {
        processInst(inst);

        for(auto child : inst->getChildren())
            processInstRec(child);
    }

    void processInst(IRInst* inst)
    {
        // We will frame our processing as a search for types
        // of the form `*StructuredBuffer<matrixType>` where
        // `matrixType` is any matrix type.
        //
        // If the instruction we are looking at doesn't have
        // the right form, then we will skip it.
        //
        auto oldStructuredBufferType = as<IRHLSLStructuredBufferTypeBase>(inst);
        if(!oldStructuredBufferType)
            return;
        auto matrixType = as<IRMatrixType>(oldStructuredBufferType->getElementType());
        if(!matrixType)
            return;

        // Having found a `*StructuredBuffer<M>` we will now
        // need an IR builder to help us construct the wrapper code.
        //
        SharedIRBuilder sharedBuilderStorage(m_module);
        auto sharedBuilder = &sharedBuilderStorage;
        IRBuilder builderStorage(sharedBuilder);
        auto builder = &builderStorage;

        // We begin by constructing a structure type that wraps
        // our `matrixType`, into something like:
        //
        //      struct Wrapper { matrixType wrapped; }
        //
        builder->setInsertBefore(oldStructuredBufferType);
        auto wrappedFieldKey = getWrappedFieldKey(builder);
        auto wrapperStruct = getWrapperStruct(builder, matrixType);

        // Now that we have our wrapper struct, we can construct a type
        // that is `*StructuredBuffer<wrapperStruct>` and use it to
        // replace `*StructuredBuffer<matrixType>`
        //
        // Note: we are constructing a *new* type instead of modifying
        // the old one in-place because eventually when we do deduplication
        // of types/constants more consistently it might cause problems
        // to modify a tyep in a way that changes its hash code.
        //
        // TODO: the above statement is still slippery, since we are still
        // replacing one type A with another type B globally, and doing
        // so could affect any type that in turn referenced A...
        //
        auto newStructuredBufferType = builder->getType(oldStructuredBufferType->getOp(), wrapperStruct);
        oldStructuredBufferType->replaceUsesWith(newStructuredBufferType);

        // Any values that used our old structured bufer type
        // now have the new structured buffer type, but that
        // means that operations on them might be getting
        // applied incorrectly.
        //
        // For example, if we had a call like:
        //
        //      float4x4 m = gBuffer.Load(someIndex);
        //
        // the result of the `Load` call is now `wrapperStruct`
        // and we can't assign that to a matrix-type variable.
        //
        // we need to invetigate values of our structured
        // buffer type, and then investigate operations that
        // are using those values, to see if we can find the
        // ones we need to rewrite.
        //
        // We can find values of `newStructuredBufferType` by
        // scanning through its IR uses, since values of that
        // type are using it as a (type) operand.
        //
        for( auto typeUse = newStructuredBufferType->firstUse; typeUse; typeUse = typeUse->nextUse )
        {
            // There might be uses of `newStructuredBufferType` where
            // it isn't being used as the type of a value, so we
            // start by weeding out the ones we don't care about.
            //
            auto valueOfStructuredBufferType = typeUse->getUser();
            if(valueOfStructuredBufferType->getFullType() != newStructuredBufferType)
                continue;

            // Now we have some `valueOfStructuredBufferType`. In our running
            // example, this might be `gBuffer`, which is an `IRGlobalParam`.
            //
            // We don't need to change anything about `gBuffer` itself, since
            // replacing `oldStructuredBufferType` with `newStructuredBufferType`
            // already replaced the type of `gBuffer`.
            //
            // Instead, we want to look for instructions that *use* the buffer,
            // because these could be calls to intrinsic functions like
            // `RWStructuredBuffer.Load`
            //
            for( auto valueUse = valueOfStructuredBufferType->firstUse; valueUse; valueUse = valueUse->nextUse )
            {
                // we are only interested in instructions that are calls,
                // with at least one argument, where the first argument
                // is our `valueOfStructuredBufferType`. These
                // are calls that could potentially be intrinsic
                // operations on `*StructuredBuffer`.
                //
                auto call = as<IRCall>(valueUse->getUser());
                if(!call)
                    continue;
                if(call->getArgCount() == 0)
                    continue;
                if(call->getArg(0) != valueOfStructuredBufferType)
                    continue;

                // At this point we have a candidate `call` instruction,
                // but we need to determine whether it is a call to
                // one of the `*StructuredBuffer` intrinsics that we want
                // to rewrite, or if it is another user-defined function
                // that we should leave along (even if that user-defined
                // function happens to return a matrix).
                //
                // For now we will do this in a somewhat ad-hoc fashion.
                // We know that the `Load` and `operator[]` operations
                // on `*StructuredBuffer` are generic, and unlike user-defined
                // generic functions they will not have been specialized
                // before we get here.
                //
                // We will thus use the fact that the callee of the call
                // is a `specialize` instruction to let us know that it
                // is an intrinsic, and thus should be one of the functions
                // we care about.
                //
                // TODO: Figure out if there is a more robust way to make
                // this check. It is possible that structured buffer
                // access should be modeled with explicit IR opcodes
                // rather than just as builtin functions.
                //
                auto callee = call->getCallee();
                if(!as<IRSpecialize>(callee))
                    continue;

                // At this point it seems likely we have one of the calls
                // we want to rewrite, but there are still intrinsics
                // like `GetDimensions` that we want to leave alone.
                //
                // For now we will look at the return type of the call,
                // where we care about two cases.
                //
                builder->setInsertBefore(call->getNextInst());
                auto oldResultType = call->getDataType();

                // First we care about the case for `Load`, which
                // will return the element type, which would be
                // a matrix type.
                //
                if( as<IRMatrixType>(oldResultType) )
                {
                    // We know that the call to `Load` should now
                    // return our wrapper struct type, so we will
                    // go ahead and modify its type to be correct.
                    //
                    auto newResultType = wrapperStruct;
                    builder->setDataType(call, newResultType);

                    // Next, we need to make sure to extract the
                    // field from the wrapper struct, so that
                    // we get back to a value of the expected
                    // type.
                    //
                    // This logic takes something like:
                    //
                    //      WrapperStruct call = gBuffer.Load(index);
                    //
                    // and follows it with:
                    //
                    //      float4x4 newVal = call.wrapped;
                    //
                    auto newVal = builder->emitFieldExtract(oldResultType, call, wrappedFieldKey);

                    // Any code that used the value of `call` should
                    // now use `newVal` instead...
                    //
                    call->replaceUsesWith(newVal);
                    //
                    // ... except for one important gotcha, which is
                    // that `newVal` itself used `call`, and replacing
                    // `call` with `newVal` results in `newVal` using
                    // itself as one of its operands.
                    //
                    // It is a bit of a kludge, but we fix the situation
                    // by just setting the appropriate operand again.
                    //
                    // TODO: it might be helpful to have a variant
                    // of `replaceUsesWith` that can handle cases like
                    // this.
                    //
                    newVal->setOperand(0, call);
                }
                //
                // The second interesting case is the `ref` accessor
                // in `operator[]` for a `RWStructuredBuffer`, which
                // at the IR level returns a *pointer* to the buffer
                // element type.
                //
                else if(auto oldPtrType = as<IRPtrTypeBase>(oldResultType))
                {
                    auto pointeeType = oldPtrType->getValueType();
                    if( as<IRMatrixType>(pointeeType) )
                    {
                        // At this point we know that the intrinsic
                        // operation returned a pointer to a matrix,
                        // which seems like a good indications that
                        // it is our `operator[]` and it should now
                        // return a pointer to the wrapper struct
                        // instead.
                        //
                        // The logic here is almost identical to the
                        // non-pointer case above, so please refer
                        // there if you want the comments.

                        auto newResultType = builder->getPtrType(oldPtrType->getOp(), wrapperStruct);
                        builder->setDataType(call, newResultType);

                        auto newVal = builder->emitFieldAddress(oldResultType, call, wrappedFieldKey);
                        call->replaceUsesWith(newVal);
                        newVal->setOperand(0, call);
                    }
                }
            }
        }
    }

        /// Get the struture field "key" to use for generated wrappers
    IRStructKey* getWrappedFieldKey(IRBuilder* builder)
    {
        // We will re-use the same field key for all of the
        // wrapper structs we might create, so that all of
        // the new field access operations will use the same
        // field name to make their purpose more clear.
        //
        // TODO: It might be useful to give the field key
        // a name that is indicative of its purpose so
        // that people won't be confused why their code
        // has been transformed and now there is this
        // `._S2` in the middle of their expressions.

        if( !m_wrappedFieldKey )
        {
            m_wrappedFieldKey = builder->createStructKey();
        }
        return m_wrappedFieldKey;
    }

        /// Lazily created and cached field "key" to use for wrapper structs.
    IRStructKey* m_wrappedFieldKey = nullptr;

        /// Get the wrapper struct to use for a particular `matrixType`.
    IRStructType* getWrapperStruct(IRBuilder* builder, IRMatrixType* matrixType)
    {
        // TODO: Because our type de-duplication isn't perfect right now,
        // it is possible that there could be more than one equivalent
        // matrix type, and thus more than one equivalent structured buffer
        // type using that matrix type. As a result, if we just generate
        // one new `struct` per IR structured-buffer type, we could end up
        // with conflicts when a buffer with one IR type gets passed to
        // a function with another (equivalent) IR type.
        //
        // The right fix here is to cache and look up these structure
        // tyeps based on the scalar type (opcode) and row/column counts
        // of the given `matrixType`.
        //
        // For now I will ignore this issue and hope that we can address
        // the more fundamental issue of type deduplication before this
        // choice comes back to bite me.

        auto wrappedFieldKey = getWrappedFieldKey(builder);

        auto wrapperStruct = builder->createStructType();
        builder->createStructField(wrapperStruct, wrappedFieldKey, matrixType);
        return wrapperStruct;
    }
};

void wrapStructuredBuffersOfMatrices(
    IRModule*                           module)
{
    WrapStructuredBuffersContext context;
    context.m_module = module;
    context.processModule();
}

}
