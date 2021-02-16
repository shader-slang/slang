// slang-ir-specialize-function-call.cpp
#include "slang-ir-specialize-function-call.h"

#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct FunctionParameterSpecializationContext
{
    // This type implements a pass to specialize functions
    // with specific types of parameters (identified by
    // `condition`) to ensure that they are
    // legal or optimized for a given target.
    //
    // We start with member variables to stand in for
    // the parameters that were passed to the top-level
    // `specializeFunctionParameters` function.
    //
    BackEndCompileRequest* compileRequest;
    TargetRequest*  targetRequest;
    IRModule*       module;

    // The condition on which parameters to specialize.
    FunctionCallSpecializeCondition* condition;

    // Our general approach will be to think in terms
    // of specializing call sites, which amount to
    // `IRCall` instructions. We will keep a work list
    // of call sites in the program that may be worth
    // considering for specialization.
    //
    List<IRCall*>   workList;

    // Because we may need to generate specialized functions
    // and generate new calls to those functions, we'll
    // need some IR building state to get our work done.
    //
    SharedIRBuilder sharedBuilderStorage;
    IRBuilder       builderStorage;
    IRBuilder* getBuilder() { return &builderStorage; }

    // With the basic state out of the way, let's walk
    // through the overall flow of the pass.
    //
    void processModule()
    {
        // We will start by initializing our IR building state.
        //
        sharedBuilderStorage.module = module;
        sharedBuilderStorage.session = module->getSession();
        builderStorage.sharedBuilder = &sharedBuilderStorage;

        // Next we will populate our initial work list by
        // recursively finding every single call site in the module.
        //
        addCallsToWorkListRec(module->getModuleInst());

        // We will process the work list until it goes dry,
        // treating it like a stack of work items.
        //
        while( workList.getCount() )
        {
            auto call = workList.getLast();
            workList.removeLast();

            // At each call site we first check whether it
            // is something we can (and should) specialize,
            // and if so, do it. The process of specializing
            // a function may introduce new call sites that
            // become candidates for specialization, so
            // our work list may grow along the way.
            //
            if( canSpecializeCall(call) )
            {
                specializeCall(call);
            }
        }
    }

    // Setting up the work list is a simple recursive procedure.
    //
    void addCallsToWorkListRec(IRInst* inst)
    {
        // If we have a call site, then add it to the list.
        //
        if( auto call = as<IRCall>(inst) )
        {
            workList.add(call);
        }

        // Recursively walk through any children, to
        // see if we uncover more call sites.
        //
        for( auto child : inst->getChildren() )
        {
            addCallsToWorkListRec(child);
        }
    }

    // We need a way to decide for a given call site
    // whether we can/must specialize it.
    //
    bool canSpecializeCall(IRCall* call)
    {
        // We can only specialize calls where the callee
        // func can be statically identified, and where
        // the callee is a definition (with body) rather
        // than a declaration. Otherwise there is no
        // way to generate a specialized callee function.
        //
        auto func = as<IRFunc>(call->getCallee());
        if(!func)
            return false;
        if(!func->isDefinition())
            return false;

        // With the basic checks out of the way, there are
        // two conditions we care about:
        //
        // 1. Should we specialize? This amounts to whether
        // `func` has any parameters that need specialization.
        // We will call those "specializable" parameters for
        // lack of a better name.
        //
        // 2. Can we specialize? This amounts to whether the
        // arguments in `call` that correspond to those
        // specializable parameters are "suitable" for use
        // in specialization.
        //
        // We are going to answer both of these queries in
        // a single loop that walks over the parameters of
        // `func` as well as the arguments to `call`.
        //
        // The loop may seem a bit awkward because we are
        // doing a parallel iteration over a linked list
        // (the parameters of `func`) and an array (the
        // arguments of `call`).
        //
        bool anySpecializableParam = false;
        UInt argCounter = 0;
        for( auto param : func->getParams() )
        {
            UInt argIndex = argCounter++;
            SLANG_ASSERT(argIndex < call->getArgCount());
            auto arg = call->getArg(argIndex);

            // If the given parameter doesn't need specialization,
            // then we need to keep looking.
            //
            if(!doesParamNeedSpecialization(param))
                continue;

            // If we have run into a `param` that needs specialization,
            // then our first condition is met.
            //
            anySpecializableParam = true;

            // Now we need to check whether `arg` is actually suitable
            // for specialization (our second condition). If not, we
            // can bail out immediately because our second condition
            // cannot be met.
            //
            if(!isArgSuitableForSpecialization(arg))
                return false;
        }

        // If we exit the loop, then the second condition must have
        // been met (all the arguments for specializable parameters
        // were suitable for specialization), and the result of the
        // query comes down to the first condition.
        //
        return anySpecializableParam;
    }

    // Of course, now we need to back-fill the predicates that
    // the above function used to evaluate prameters and arguments.

    bool doesParamNeedSpecialization(IRParam* param)
    {
        return condition->doesParamNeedSpecialization(param);
    }

    bool isArgSuitableForSpecialization(IRInst* inArg)
    {
        // Determining if an argument is suitable for
        // specializing a callee function requires
        // looking at its (recurisve) structure.
        //
        // Rather than write a recursively procedure
        // here, we will be tail-recursive by using
        // a simple loop.
        //
        IRInst* arg = inArg;
        for(;;)
        {
            // The leaf case we care about is when the
            // argument at the call site is a global
            // shader parameter, because then we can
            // specialize a callee to refer to the same
            // global parameter directly.
            //
            if(as<IRGlobalParam>(arg)) return true;

            // As we will see later, we can also
            // specialize a call when the argument
            // is the result of indexing into an
            // array (`base[index]`) *if* the `base`
            // of the indexing operation is also
            // suitable for specialization.
            //
            if( arg->getOp() == kIROp_getElement || arg->getOp() == kIROp_Load )
            {
                auto base = arg->getOperand(0);

                // We will "recurse" on the base of
                // the indexing operation by continuing
                // our loop with the `base` as our new
                // argument.
                //
                arg = base;
                continue;
            }

            // By default, we will *not* consider an argument
            // suitable for specialization.
            //
            // TODO: There may be other cases that are worth
            // handling here. The current code is based on
            // observation of what simple shaders do in
            // practice.
            //
            return false;
        }
    }

    // Once we'e determined that a given call site can/should
    // be specialized, we need to perform the actual specialization.
    // This is where things are going to get more involved.
    //
    // There are a few different concerns we need to deal with
    // that mean we end up having two different passes that walk
    // over the parameters/arguments of the call (in addition to
    // the ones we had above for determining if we can/should
    // specialize in the first place).
    //
    // The first of the two passes determines information
    // relevant to the call site, comprising both the arguments
    // that will be passed to the specialized function as
    // well as a "key" to identify the specialized function
    // that is required.
    //
    // We will use the key type defined as part of the IR cloning
    // infrastructure, which uses a sequence of `IRInst*`s
    // to hold the state of the key:
    //
    typedef IRSimpleSpecializationKey Key;

    // As indicated above, the information we collect about a call
    // site consists of the key for the specialized function we
    // will call, and a list of the arguments that will be passed
    // to the call.
    //
    struct CallSpecializationInfo
    {
        Key             key;
        List<IRInst*>   newArgs;
    };

    // Once we've collected the information about a call site
    // we can use a dictionary to see if we already created
    // a specialized version of the callee that matches its
    // requirements.
    //
    Dictionary<Key, IRFunc*> specializedFuncs;

    // If the dictionary didn't have a specialized function
    // suitable for a call site, we need a second information-gathering
    // pass to decide what the new parameters of the specialized
    // functions should be, and what instructions the new function
    // must execute in its body to set up the replacements for the
    // old parameters.
    //
    struct FuncSpecializationInfo
    {
        List<IRParam*>  newParams;
        List<IRInst*>   newBodyInsts;
        List<IRInst*>   replacementsForOldParameters;
    };

    // Before diving into how the different passes collect
    // their information, we will dive into the main
    // specialization logic first.
    //
    void specializeCall(IRCall* oldCall)
    {
        // We have an existing call site `oldCall` that
        // we know can and should be specialized.
        //
        // That means the callee should be a known function
        // definition, or else `canSpecializeCall` didn't
        // correctly check the preconditions.
        //
        auto oldFunc = as<IRFunc>(oldCall->getCallee());
        SLANG_ASSERT(oldFunc);
        SLANG_ASSERT(oldFunc->isDefinition());

        // Our first information-gathering pass will
        // compute the key for the specialized function
        // we want to call, and the arguments we will
        // use for that call.
        //
        CallSpecializationInfo callInfo;
        gatherCallInfo(oldCall, oldFunc, callInfo);

        // Once we have gathered information on the call,
        // we can check if we have an existing specialization
        // that we generated before (for another call site)
        // that is suitable to this call site.
        //
        IRFunc* newFunc = nullptr;
        if( !specializedFuncs.TryGetValue(callInfo.key, newFunc) )
        {
            // If we didn't find a pre-existing specialized
            // function, then we will go ahead and create one.
            //
            // We start by gathering the information from the call
            // site that is relevant to generating a specialized
            // callee function, which we avoided doing earlier
            // because it might have been throwaway work.
            //
            FuncSpecializationInfo funcInfo;
            gatherFuncInfo(oldCall, oldFunc, funcInfo);

            // Now we use the gathered information to generate
            // a new callee function based on the original
            // function and the information we gathered.
            //
            newFunc = generateSpecializedFunc(oldFunc, funcInfo);
            specializedFuncs.Add(callInfo.key, newFunc);
        }

        // Once we've other found or generated a specialized function
        // we need to generate a call to it, and then use the new
        // call as a replacement for the old one.
        //
        auto newCall = getBuilder()->emitCallInst(
            oldCall->getFullType(),
            newFunc,
            callInfo.newArgs.getCount(),
            callInfo.newArgs.getBuffer());

        newCall->insertBefore(oldCall);
        oldCall->replaceUsesWith(newCall);
        oldCall->removeAndDeallocate();
    }

    // Before diving into the details on how we gather information
    // and specialize callees, lets stop to think about what we'd
    // like to do in terms of individual parameters and arguments.
    //
    // Suppose we are specializing both a call site C and the callee
    // function F, and we are consisering a particular pair of
    // a parmeter P of F, and an argument A at the call site.
    //
    // The full extent of information we might want to know given
    // P and A is:
    //
    // * What arguments need to be added to the specialized call?
    // * What parameters need to be added to the specialized callee?
    // * What instructions are needed in the body of the specialized
    //   callee to synthesize the value that will stand in for P?
    // * What information, if any, needs to be used to distinguish
    //   this specialized callee from others that might be generated for F?
    //
    // An easy case is when P is a parameter that doesn't need
    // specialization. In that case:
    //
    // * The existing argument A should be used as an argument in
    //   the specialized call.
    // * A clone P' of the existing parameter P should be used as a
    //   parameter of the specialized callee.
    // * No additional instructions are needed in the body of
    //   the callee; the cloned parameter P' should stand in for P.
    // * No information should be added to the specialization key
    //   based on P and A.
    //
    // The more interesting case is when P has a resource type, and
    // A is some global shader parameter G.
    //
    // * No argument should be added at the new call site
    // * No parameter should be added to the specialized callee
    // * No additional instructions are needed in the body of
    //   the callee; the global G should stand in for P.
    // * The global G should be used to distinguish this specialized
    //   callee from those that might be specialized for a different
    //   global shader parameter.
    //
    // As a final example, imagine that P is still a resource type,
    // but A is now an indexing operation into an array: `G[idx]`:
    //
    // * An argument for `idx` should be added at the call site
    // * A parameter `p_idx` with the same type as `idx` should be added
    //   to the specialized callee.
    // * An instruction should be added to the specialized callee
    //   to compute `G[p_idx]` and use that to stand in for P.
    // * The global G should still be used to distinguish this specialized
    //   call site from others.
    //
    // That's a lot of examples, I know, but hopefully it gives a
    // sense of the information we are tracking and how it differs
    // across the various cases. While the example only covered one
    // level of indexing, the actual implementation will handle the
    // case of arbitrarily many levels of indexing, which can mean
    // piping through any number of additional integer parameters
    // to the callee.

    // The information we gather for a call site (before we know
    // whether a specialize calle is needed) is just the new
    // argument list, and the "key" information that distinguishes
    // what specialized callee we want/need.
    //
    void gatherCallInfo(
        IRCall*                 oldCall,
        IRFunc*                 oldFunc,
        CallSpecializationInfo& callInfo)
    {
        // The specialized callee key always needs to include
        // the original function, since different functions
        // will always yield different specializations.
        //
        callInfo.key.vals.add(oldFunc);

        // The rest of the information is gathered by looking
        // at parameter and argument pairs.
        //
        UInt oldArgCounter = 0;
        for( auto oldParam : oldFunc->getParams() )
        {
            UInt oldArgIndex = oldArgCounter++;
            auto oldArg = oldCall->getArg(oldArgIndex);

            getCallInfoForParam(callInfo, oldParam, oldArg);
        }
    }

    void getCallInfoForParam(
        CallSpecializationInfo& ioInfo,
        IRParam*                oldParam,
        IRInst*                 oldArg)
    {
        // We know that the case where a parameter
        // doesn't need specialization is easy.
        //
        if( !doesParamNeedSpecialization(oldParam) )
        {
            // The new call site will use the same argument
            // value as the old one, and we don't need
            // to add any information to distinguish the
            // specialized callee based on this paramter.
            //
            ioInfo.newArgs.add(oldArg);
        }
        else
        {
            // If specialization is needed, we need
            // to inspect the argument value. This
            // is handled with a different function
            // because it needs to recurse in some cases.
            //
            getCallInfoForArg(ioInfo, oldArg);
        }
    }

    void getCallInfoForArg(
        CallSpecializationInfo& ioInfo,
        IRInst*                 oldArg)
    {
        // The base case we care about is when the original
        // argument is a global shader parameter.
        //
        if( auto oldGlobalParam = as<IRGlobalParam>(oldArg) )
        {
            // In this case we don't need to pass anything
            // as an argument at the new call site (the
            // global parameter will get specialized into
            // the callee), but we *do* need to make sure
            // that our key for identifying the specialized
            // callee reflects that we are specializing
            // to the chosen parameter.
            //
            ioInfo.key.vals.add(oldGlobalParam);
        }
        else if( oldArg->getOp() == kIROp_getElement )
        {
            // This is the case where the `oldArg` is
            // in the form `oldBase[oldIndex]`
            //
            auto oldBase  = oldArg->getOperand(0);
            auto oldIndex = oldArg->getOperand(1);

            // Effectively, we act as if `oldBase` and
            // `oldIndex` were passed to the callee separately,
            // so that `oldBase` is an array-of-resouces and
            // `oldIndex` is an ordinary integer argument.
            //
            // We start by recursively setting up whatever
            // `oldBase` needs:
            //
            getCallInfoForArg(ioInfo, oldBase);

            // Then we process `oldIndex` just like we
            // would have an ordinary argument that doesn't
            // involve specialization: add its value to
            // the arguments at the new call site, and
            // don't add anything to the specialization key.
            //
            ioInfo.newArgs.add(oldIndex);
        }
        else if (oldArg->getOp() == kIROp_Load)
        {
            auto oldBase = oldArg->getOperand(0);
            getCallInfoForArg(ioInfo, oldBase);
        }
        else
        {
            // If we fail to match any of the cases above
            // then a precondition was violated in that
            // `isArgSuitableForSpecialization` is allowing
            // a case that this routine is not covering.
            //
            SLANG_UNEXPECTED("mising case in 'getCallInfoForArg'");
        }
    }

    // The remaining information we've discussed is only
    // gathered once we decide we want to generate a
    // specialized function, but it follows much the same flow.
    //
    void gatherFuncInfo(
        IRCall*                 oldCall,
        IRFunc*                 oldFunc,
        FuncSpecializationInfo& funcInfo)
    {
        UInt oldArgCounter = 0;
        for( auto oldParam : oldFunc->getParams() )
        {
            UInt oldArgIndex = oldArgCounter++;
            auto oldArg = oldCall->getArg(oldArgIndex);

            // For each parameter and argument pair we will
            // frame the main task as producing a value that
            // will stand in for the parameter in the specialized
            // function.
            //
            auto newVal = getSpecializedValueForParam(funcInfo, oldParam, oldArg);

            // We will collect the replacement value to use
            // for each of the original parameters in an array.
            //
            funcInfo.replacementsForOldParameters.add(newVal);
        }
    }

    IRInst* getSpecializedValueForParam(
        FuncSpecializationInfo& ioInfo,
        IRParam*                oldParam,
        IRInst*                 oldArg)
    {
        // As always, the easy case is when the parameter of
        // the original function doesn't need specialization.
        //
        if( !doesParamNeedSpecialization(oldParam) )
        {
            // The specialized callee will need a new parameter
            // that fills the same role as the old one, so we
            // create it here.
            //
            auto newParam = getBuilder()->createParam(oldParam->getFullType());
            ioInfo.newParams.add(newParam);

            // The new parameter will be used as the replacement
            // for the old one in the specialized function.
            //
            return newParam;
        }
        else
        {
            // If the parameter requires specialization, then it
            // is time to look at the structure of the argument.
            //
            return getSpecializedValueForArg(ioInfo, oldArg);
        }
    }

    IRInst* getSpecializedValueForArg(
        FuncSpecializationInfo& ioInfo,
        IRInst*                 oldArg)
    {
        // The logic here parallels `gatherCallInfoForArg`,
        // and only differs in what information it is gathering.
        //
        // As before, the base case is when we have a global
        // shader parameter.
        //
        if( auto globalParam = as<IRGlobalParam>(oldArg) )
        {
            // The specialized function will not need any
            // parameter in this case, and the global itself
            // should be used to stand in for the original
            // parameter in the specialized function.
            //
            return globalParam;
        }
        else if( oldArg->getOp() == kIROp_getElement )
        {
            // This is the case where the argument is
            // in the form `oldBase[oldIndex]`.
            //
            auto oldBase  = oldArg->getOperand(0);
            auto oldIndex = oldArg->getOperand(1);

            // In `gatherCallInfoForArg` this case was
            // handled by acting as if `oldBase` and
            // `oldIndex` were being passed as two
            // separate arguments.
            //
            // We'll follow the same structure here,
            // starting by recursively processing `oldBase`
            // to get a value that can stand in for it
            // in the specialized callee.
            //
            auto newBase = getSpecializedValueForArg(ioInfo, oldBase);

            // Next we'll process `oldIndex` as if it
            // was an ordinary argument (not a specialized one),
            // which means creating a parameter to receive its value,
            // which will also stand in for `oldIndex` in
            // the body of the specialized callee.
            //
            auto builder = getBuilder();
            auto newIndex = builder->createParam(oldIndex->getFullType());
            ioInfo.newParams.add(newIndex);

            // Finally, we need to compute a value that
            // can stand in for `oldArg` (which was
            // `oldBase[oldIndex]`) in the body of the
            // specialized callee.
            //
            // Because we have both a `newBase` and a
            // `newIndex` it is natural to construct
            // `newBase[newIndex]` and use that.
            //
            // The only complication is that we need
            // to make sure that our IR builder isn't
            // set to insert newly created instructions
            // anywhere, since the `emit*` functions
            // will try to automatically insert new
            // instructions if an insertion location
            // is set.
            //
            builder->setInsertInto(nullptr);
            auto newVal = builder->emitElementExtract(
                oldArg->getFullType(),
                newBase,
                newIndex);

            // Because our new instruction wasn't
            // actually inserted anywhere, we need to
            // add it to our gathered list of instructions
            // that should be inserted into the body of
            // the specialized callee.
            //
            ioInfo.newBodyInsts.add(newVal);

            return newVal;
        }
        else if (oldArg->getOp() == kIROp_Load)
        {
            return getSpecializedValueForArg(ioInfo, oldArg->getOperand(0));
        }
        else
        {
            // If we don't match one of the above cases,
            // then `isArgSuitableForSpecialization` is
            // letting through cases that this function
            // hasn't been updated to handle.
            //
            SLANG_UNEXPECTED("mising case in 'getSpecializedValueForArg'");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    // With all of that data-gathering code out of the way,
    // we are now prepared to walk through the process of
    // specializing a given callee function based on
    // the information we have gathered.
    //
    IRFunc* generateSpecializedFunc(
        IRFunc*                         oldFunc,
        FuncSpecializationInfo const&   funcInfo)
    {
        // We will make use of the infrastructure for cloning
        // IR code, that is defined in `ir-clone.{h,cpp}`.
        //
        // In order to do the cloning work we need an
        // "environment" that will map old values to
        // their replacements.
        //
        IRCloneEnv cloneEnv;

        // Next we iterate over the parameters of the old
        // function, and register each as being mapped
        // to its replacement in the `funcInfo` that was
        // already gathered.
        //
        UInt paramCounter = 0;
        for( auto oldParam : oldFunc->getParams() )
        {
            UInt paramIndex = paramCounter++;
            auto newVal = funcInfo.replacementsForOldParameters[paramIndex];
            cloneEnv.mapOldValToNew.Add(oldParam, newVal);
        }

        // Next we will create the skeleton of the new
        // specialized function, including its type.
        //
        // To get the type of the new function we will
        // iterate over the collected list of new
        // parameters (which may differ greatly from the
        // parameter list of the original) and extract
        // their types.
        //
        List<IRType*> paramTypes;
        for( auto param : funcInfo.newParams )
        {
            paramTypes.add(param->getFullType());
        }

        auto builder = getBuilder();
        IRType* funcType = builder->getFuncType(
            paramTypes.getCount(),
            paramTypes.getBuffer(),
            oldFunc->getResultType());

        IRFunc* newFunc = builder->createFunc();
        newFunc->setFullType(funcType);

        // The above step has accomplished the "first phase"
        // of cloning the function (since `IRFunc`s have no
        // operands).
        //
        // We can now use the shared IR cloning infrastructure
        // to perform the second phase of cloning, which will recursively
        // clone any nested decorations, blocks, and instructions.
        //
        cloneInstDecorationsAndChildren(
            &cloneEnv,
            builder->sharedBuilder,
            oldFunc,
            newFunc);

        // If we have added an Linkage decoration, we want to remove and destroy it,
        // because the linkage should only be on the original function and
        // not on the "torn off" copies made in this function.
        //
        // It *could* be argued that we don't want to duplicate the decoration instructions
        // to begin with, just to throw them away. That may be true, but it's simpler to just remove
        // them than filter out in cloning.

        {
            auto decorationList = newFunc->getDecorations();

            const auto end = decorationList.end();
            auto cur = decorationList.begin();

            while(cur != end)
            {
                IRDecoration* decoration = *cur;

                // We step before before the test/destroying to ensure cur is not pointing
                // to a potentially destroyed instruction
                ++cur;

                if (as<IRLinkageDecoration>(decoration))
                {
                    decoration->removeAndDeallocate();
                }
            }
        }


        // We are almost done at this point, except that `newFunc`
        // is lacking its parameters, as well as any of the body
        // instructions that we decided were needed during
        // the information-gathering steps.
        //
        // We will insert these instructions into the first block
        // of the function, before its first ordinary instruction.
        // We know that these should exist because we had as
        // a precondition that `oldFunc` was a definition (so it
        // has at least one block), and in valid IR every block
        // has at least one ordinary instruction (its terminator).
        //
        auto newEntryBlock = newFunc->getFirstBlock();
        SLANG_ASSERT(newEntryBlock);
        auto newFirstOrdinary = newEntryBlock->getFirstOrdinaryInst();
        SLANG_ASSERT(newFirstOrdinary);

        // We simply iterate over the list of parameters and then
        // body instructions that were produced in the information
        // gathering step, and insert each before `newFirstOrdinary`,
        // which has the effect or arranging them in the output
        // in the order they are enumerated here.
        //
        for( auto newParam : funcInfo.newParams )
        {
            newParam->insertBefore(newFirstOrdinary);
        }
        for( auto newBodyInst : funcInfo.newBodyInsts )
        {
            newBodyInst->insertBefore(newFirstOrdinary);
        }

        // At this point we've created a new specialized function,
        // and as such it may contain call sites that were not
        // covered when we built our initial work list.
        //
        // Before handing the specialized function back to the
        // caller, we will make sure to recursively add any
        // potentially-specializable call sites to our work list.
        //
        addCallsToWorkListRec(newFunc);

        return newFunc;
    }
};

// The top-level function for invoking the specialization pass
// is straighforward. We set up the context object
// and then defer to it for the real work.
//
void specializeFunctionCalls(
    BackEndCompileRequest* compileRequest,
    TargetRequest*  targetRequest,
    IRModule*       module,
    FunctionCallSpecializeCondition* condition)
{
    FunctionParameterSpecializationContext context;
    context.compileRequest = compileRequest;
    context.targetRequest = targetRequest;
    context.module = module;
    context.condition = condition;

    context.processModule();
}

} // namesapce Slang
