// slang-ir-explicit-global-context.cpp
#include "slang-ir-explicit-global-context.h"

#include "slang-ir-insts.h"

namespace Slang
{

// The job of this pass is take global-scope declarations
// that are actually scoped to a single shader thread or
// thread-group, and wrap them up in an explicit "context"
// type that gets passed between functions.

struct IntroduceExplicitGlobalContextPass
{
    IRModule*       m_module = nullptr;
    CodeGenTarget   m_target = CodeGenTarget::Unknown;

    SharedIRBuilder*    m_sharedBuilder         = nullptr;
    IRStructType*       m_contextStructType     = nullptr;
    IRPtrType*          m_contextStructPtrType  = nullptr;

    IRGlobalParam*      m_globalUniformsParam   = nullptr;
    List<IRGlobalVar*>  m_globalVars;
    List<IRFunc*>       m_entryPoints;

    void processModule()
    {
        SharedIRBuilder sharedBuilder(m_module);
        m_sharedBuilder = &sharedBuilder;

        IRBuilder builder(&sharedBuilder);

        // The global context will be represneted by a `struct`
        // type with a name hint of `KernelContext`.
        //
        m_contextStructType = builder.createStructType();
        builder.addNameHintDecoration(m_contextStructType, UnownedTerminatedStringSlice("KernelContext"));

        // The context will usually be passed around by pointer,
        // so we get and cache that pointer type up front.
        //
        m_contextStructPtrType = builder.getPtrType(m_contextStructType);

        // The transformation we will perform will need to affect
        // global variables, global shader parameters, and entry-point
        // function (at the very least), and we start with an explicit
        // pass to collect these entities into explicit lists to simplify
        // looping over them later.
        //
        for( auto inst : m_module->getGlobalInsts() )
        {
            switch( inst->op )
            {
            case kIROp_GlobalVar:
                {
                    // A "global variable" in HLSL (and thus Slang) is actually
                    // a weird kind of thread-local variable, and so it cannot
                    // actually be lowered to a global variable on targets where
                    // globals behave like, well, globals.
                    //
                    auto globalVar = cast<IRGlobalVar>(inst);

                    // One important exception is that CUDA *does* support
                    // global variables with the `__shared__` qualifer, with
                    // semantics that exactly match HLSL/Slang `groupshared`.
                    //
                    // We thus need to skip processing of global variables
                    // that were marked `groupshared`. In our current IR,
                    // this is represented as a variable with the `@GroupShared`
                    // rate on its type.
                    //
                    if( m_target == CodeGenTarget::CUDASource )
                    {
                        if( as<IRGroupSharedRate>(globalVar->getRate()) )
                            continue;
                    }

                    m_globalVars.add(globalVar);
                }
                break;

            case kIROp_GlobalParam:
                {
                    // Global parameters are another HLSL/Slang concept
                    // that doesn't have a parallel in langauges like C/C++.
                    //
                    auto globalParam = cast<IRGlobalParam>(inst);


                    // One detail we need to be careful about is that as a result
                    // of legalizing the varying parameters of kernels, we can end
                    // up with global parameters for varying parameters on CUDA
                    // (e.g., to represent `threadIdx`. We thus skip any global-scope
                    // parameters that are varying instead of uniform.
                    //
                    auto layoutDecor = globalParam->findDecoration<IRLayoutDecoration>();
                    SLANG_ASSERT(layoutDecor);
                    auto layout = as<IRVarLayout>(layoutDecor->getLayout());
                    SLANG_ASSERT(layout);
                    if(isVaryingParameter(layout))
                        continue;

                    // Because of upstream passes, we expect there to be only a
                    // single global uniform parameter (at most).
                    //
                    // Note: If we ever changed out mind about the representation
                    // and wanted to support multiple global parameters, we could
                    // easily generalize this code to work with a list.
                    //
                    SLANG_ASSERT(!m_globalUniformsParam);
                    m_globalUniformsParam = globalParam;
                }
                break;

            case kIROp_Func:
                {
                    // Every entry point function is going to need to be modified,
                    // so that it can explicit create the context that other
                    // operations will use.

                    // We need to filter the IR functions to find only those
                    // that represent entry points.
                    //
                    auto func = cast<IRFunc>(inst);
                    if(!func->findDecoration<IREntryPointDecoration>())
                        continue;

                    m_entryPoints.add(func);
                }
                break;
            }
        }

        // Now that we've capture all the relevant global entities from the IR,
        // we can being to transform them in an appropriate order.
        //
        // The first step will be to create fields in the `KernelContext`
        // type to represent any global parameters or global variables.
        //
        // The keys for the fields that are created will be remembered
        // in a dictionary, so that we can find them later based on
        // the global parameter/variable.
        //
        if( m_globalUniformsParam )
        {
            // For the parameter representing all the global uniform shader
            // parameters, we create a field that exactly matches its type.
            //
            createContextStructField(m_globalUniformsParam, m_globalUniformsParam->getFullType());
        }
        for( auto globalVar : m_globalVars )
        {
            // A `IRGlobalVar` represents a pointer to where the variable is stored,
            // so we need to create a field of the pointed-to type to represent it.
            //
            createContextStructField(globalVar, globalVar->getDataType()->getValueType());
        }

        // Once all the fields have been created, we can process the entry points.
        //
        // Each entry point will create a local `KernelContext` variable and
        // initialize it based on the parameters passed to the entry point.
        //
        // The local variable introduced here will be registered as the representation
        // of the context to be used in the body of the entry point.
        //
        for( auto entryPoint : m_entryPoints )
        {
            createContextForEntryPoint(entryPoint);
        }

        // Now that we've prepared all the entry points, we can make another
        // pass over the global parameters/variables and start to replace
        // their use sites with references to the fields of the context.
        //
        // Wherever a global parameter/variable is being referenced in a function,
        // we will need to find or create a context value for that function
        // to use. The context value for entry points has already been established
        // above, but other functions will have an explicit context parameter
        // added on demand.
        //
        if( m_globalUniformsParam )
        {
            replaceUsesOfGlobalParam(m_globalUniformsParam);
        }
        for( auto globalVar : m_globalVars )
        {
            replaceUsesOfGlobalVar(globalVar);
        }
    }

    // As noted above, we will maintain mappings to record
    // the key for the context field created for a global
    // variable parameter, and to record the context pointer
    // value to use for a function.
    //
    Dictionary<IRInst*, IRStructKey*> m_mapInstToContextFieldKey;
    Dictionary<IRFunc*, IRInst*> m_mapFuncToContextPtr;

    void createContextStructField(IRInst* originalInst, IRType* type)
    {
        // Creating a field in the context struct to represent
        // `originalInst` is straightforward.

        IRBuilder builder(m_sharedBuilder);
        builder.setInsertBefore(m_contextStructType);

        // We create a "key" for the new field, and then a field
        // of the appropraite type.
        //
        auto key = builder.createStructKey();
        auto field = builder.createStructField(m_contextStructType, key, type);

        // If the original instruction had a name hint on it,
        // then we transfer that name hint over to the key,
        // so that the field will have the name of the former
        // global variable/parameter.
        //
        if( auto nameHint = originalInst->findDecoration<IRNameHintDecoration>() )
        {
            nameHint->insertAtStart(key);
        }

        // Any other decorations on the original instruction
        // (e.g., pertaining to layout) need to be transferred
        // over to the field (not the key).
        //
        originalInst->transferDecorationsTo(field);

        // We end by making note of the key that was created
        // for the instruction, so that we can use the key
        // to access the field later.
        //
        m_mapInstToContextFieldKey.Add(originalInst, key);
    }

    void createContextForEntryPoint(IRFunc* entryPointFunc)
    {
        // We can only introduce the explicit context into
        // entry points that have definitions.
        //
        auto firstBlock = entryPointFunc->getFirstBlock();
        if(!firstBlock)
            return;

        IRBuilder builder(m_sharedBuilder);

        // The code we introduce will all be added to the start
        // of the first block of the function.
        //
        auto firstOrdinary = firstBlock->getFirstOrdinaryInst();
        builder.setInsertBefore(firstOrdinary);

        // If there was a global-scope uniform parameter before,
        // then we need to introduce an explicit parameter onto
        // each entry-point function to represent it.
        //
        IRParam* globalUniformsParam = nullptr;
        if( m_globalUniformsParam )
        {
            globalUniformsParam = builder.createParam(m_globalUniformsParam->getFullType());
            if( auto nameHint = m_globalUniformsParam->findDecoration<IRNameHintDecoration>() )
            {
                builder.addNameHintDecoration(globalUniformsParam, nameHint->getNameOperand());
            }

            // The new parameter will be the last one in the
            // parameter list of the entry point.
            //
            globalUniformsParam->insertBefore(firstOrdinary);
        }

        // The `KernelContext` to use inside the entry point
        // will be a local variable declared in the first block.
        //
        auto contextVarPtr = builder.emitVar(m_contextStructType);
        addKernelContextNameHint(contextVarPtr);
        m_mapFuncToContextPtr.Add(entryPointFunc, contextVarPtr);

        // If there is a global-scope uniform parameter, then
        // we need to use our new explicit entry point parameter
        // to inialize the corresponding field of the `KernelContext`
        // before moving on with execution of the kernel body.
        //
        if(m_globalUniformsParam)
        {
            auto fieldKey = m_mapInstToContextFieldKey[m_globalUniformsParam];
            auto fieldType = globalUniformsParam->getFullType();
            auto fieldPtrType = builder.getPtrType(fieldType);

            // We compute the addrress of the field and store the
            // value of the parameter into it.
            //
            auto fieldPtr = builder.emitFieldAddress(fieldPtrType, contextVarPtr, fieldKey);
            builder.emitStore(fieldPtr, globalUniformsParam);
        }

        // Note: at this point the `KernelContext` has additional
        // fields for global variables that do not seem to have
        // been initialized.
        //
        // Instead of making this pass take responsibility for initializing
        // global variables, it is instead expected that clients will
        // run the pass in `slang-ir-explicit-global-init` first,
        // in order to move all initialization of globals into the
        // entry point functions.
    }

    void replaceUsesOfGlobalParam(IRGlobalParam* globalParam)
    {
        IRBuilder builder(m_sharedBuilder);

        // A global shader parameter was mapped to a field
        // in the context structure, so we find the appropriate key.
        //
        auto key = m_mapInstToContextFieldKey[globalParam];

        auto valType = globalParam->getFullType();
        auto ptrType = builder.getPtrType(valType);

        // We then iterate over the uses of the parameter,
        // being careful to defend against the use/def information
        // being changed while we walk it.
        //
        IRUse* nextUse = nullptr;
        for( IRUse* use = globalParam->firstUse; use; use = nextUse )
        {
            nextUse = use->nextUse;

            // At each use site, we need to look up the context
            // pointer that is appropriate for that use.
            //
            auto user = use->getUser();
            auto contextParam = findOrCreateContextPtrForInst(user);
            builder.setInsertBefore(user);

            // The value of the parameter can be produced by
            // taking the address of the corresponding field
            // in the context struct and loading from it.
            //
            auto ptr = builder.emitFieldAddress(ptrType, contextParam, key);
            auto val = builder.emitLoad(valType, ptr);
            use->set(val);
        }
    }

    void replaceUsesOfGlobalVar(IRGlobalVar* globalVar)
    {
        IRBuilder builder(m_sharedBuilder);

        // A global variable was mapped to a field
        // in the context structure, so we find the appropriate key.
        //
        auto key = m_mapInstToContextFieldKey[globalVar];

        auto ptrType = globalVar->getDataType();

        // We then iterate over the uses of the variable,
        // being careful to defend against the use/def information
        // being changed while we walk it.
        //
        IRUse* nextUse = nullptr;
        for( IRUse* use = globalVar->firstUse; use; use = nextUse )
        {
            nextUse = use->nextUse;

            // At each use site, we need to look up the context
            // pointer that is appropriate for that use.
            //
            auto user = use->getUser();
            auto contextParam = findOrCreateContextPtrForInst(user);
            builder.setInsertBefore(user);

            // The address of the variable can be produced by
            // taking the address of the corresponding field
            // in the context struct.
            //
            auto ptr = builder.emitFieldAddress(ptrType, contextParam, key);
            use->set(ptr);
        }
    }

    IRInst* findOrCreateContextPtrForInst(IRInst* inst)
    {
        // When looking up the context pointer to use for
        // an instruction, we need to find the enclosing
        // function and use whatever context pointer it uses.
        //
        for( IRInst* i = inst; i; i = i->getParent() )
        {
            if( auto func = as<IRFunc>(i) )
            {
                return findOrCreateContextPtrForFunc(func);
            }
        }

        // If a non-constant global entity is being referenced by
        // something that is *not* nested under an IR function, then
        // we are in trouble.
        //
        SLANG_UNEXPECTED("no outer func at use site for global");
        UNREACHABLE_RETURN(nullptr);
    }

    IRInst* findOrCreateContextPtrForFunc(IRFunc* func)
    {
        // At this point we are being asked to either find or
        // produce a context pointer for use inside `func`.
        //
        // If we already created such a pointer (perhaps because
        // `func` is an entry point), then we are home free.
        //
        if( auto found = m_mapFuncToContextPtr.TryGetValue(func) )
        {
            return *found;
        }

        // Otherwise, we are going to need to introduce an
        // explicit parameter to `func` to represent the
        // context.
        //
        IRBuilder builder(m_sharedBuilder);

        // We can safely assume that `func` has a body, because
        // otherwise we wouldn't be getting a request for the
        // context pointer value to use in its body.
        //
        auto firstBlock = func->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        // We create a new parameter at the end of the parameter
        // list for `func`, with a type of `KernelContext*`.
        //
        IRParam* contextParam = builder.createParam(m_contextStructPtrType);
        addKernelContextNameHint(contextParam);
        contextParam->insertBefore(firstBlock->getFirstOrdinaryInst());

        // The new parameter can be registerd as the context value
        // to be used for `func` right away.
        //
        // Note: we register the value *before* modifying locations
        // that call `func` to protect against a possible infinite-recursion
        // situation if `func` is recursive along some path.
        //
        m_mapFuncToContextPtr.Add(func, contextParam);

        // Any code that calls `func` now needs to be updated to pass
        // the context parameter.
        //
        // TODO: There is an issue here if `func` might be called
        // dynamically, through something like a witness table.
        //
        List<IRUse*> uses;
        for( auto use = func->firstUse; use; use = use->nextUse )
        {
            // We will only fix up calls to `func`, and ignore
            // other operations that might refer to it.
            //
            // TODO: We need to allow things like decorations that might
            // refer to `func`, but this logic is also going to
            // ignore things like witness tables that refer to `func`,
            // or operations that pass `func` as a function pointer
            // to a higher-order function.
            //
            auto call = as<IRCall>(use->getUser());
            if(!call)
                continue;

            // We are going to construct a new call to `func`
            // that has all of the arguments of the original call...
            //
            UInt originalArgCount = call->getArgCount();
            List<IRInst*> args;
            for( UInt aa = 0; aa < originalArgCount; ++aa )
            {
                args.add(call->getArg(aa));
            }

            // ... plus an additional argument representing
            // the context pointer at the call site (note that
            // this step leads to a potential for recursion in this pass;
            // the maximum depth of the recursion is bounded by the
            // maximum length of a cycle-free path through the call
            // graph of the program).
            //
            args.add(findOrCreateContextPtrForInst(call));

            // The new call will be emitted right before the old one,
            // then used to replace it.
            //
            builder.setInsertBefore(call);
            auto newCall = builder.emitCallInst(call->getFullType(), call->getCallee(), args);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }

        return contextParam;
    }

    // Because we have multiple places where instructions representing
    // the kernel context get introduced, we have factored out a subroutine
    // for setting up the name hint to be used by those instructions.
    //
    void addKernelContextNameHint(IRInst* inst)
    {
        IRBuilder builder(m_sharedBuilder);
        builder.addNameHintDecoration(inst, UnownedTerminatedStringSlice("kernelContext"));
    }
};

    /// Collect global-scope variables/paramters to form an explicit context that gets threaded through
void introduceExplicitGlobalContext(
    IRModule*       module,
    CodeGenTarget   target)
{
    IntroduceExplicitGlobalContextPass pass;
    pass.m_module = module;
    pass.m_target = target;
    pass.processModule();
}

}
