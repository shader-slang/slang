// ir-specialize-resources.cpp
#include "ir-specialize-resources.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang
{

struct ResourceParameterSpecializationContext
{
    // This type implements a pass to specialize functions
    // with resource parameters to ensure that they are
    // legal for a given target.
    //
    // We start with member variables to stand in for
    // the parameters that were passed to the top-level
    // `specializeResourceParameters` function.
    //
    CompileRequest* compileRequest;
    TargetRequest*  targetRequest;
    IRModule*       module;

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
        while( workList.Count() )
        {
            auto call = workList.Last();
            workList.RemoveLast();

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
            workList.Add(call);
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
        // `func` has any parameters that are need specialization.
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

            // If the given parameer doesn't need specialization,
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

    // Of course, now we need to back fill the predicates that
    // the above function used to evaluate prameters and arguments.

    bool doesParamNeedSpecialization(IRParam* param)
    {
        // Whether or not a parameter needs specialization is really
        // a function of its type:
        //
        IRType* type = param->getDataType();

        // What's more, if a parameter of type `T` would need
        // specialization, then it seems clear that a parameter
        // of type "array of `T`" would also need specialization.
        // We will "unwrap" any outer arrays from the parameter
        // type before moving on, since they won't affect
        // our decision.
        //
        type = unwrapArray(type);

        // On all of our (current) targets, a function that
        // takes a `ConstantBuffer<T>` parameter requires
        // specialization. Surprisingly this includes DXIL
        // because dxc apparently does not treat `ConstantBuffer<T>`
        // as a first-class type.
        //
        if(as<IRUniformParameterGroupType>(type))
            return true;

        // For GL/Vulkan targets, we also need to specialize
        // any parameters that use structured or byte-addressed
        // buffers.
        //
        if( isKhronosTarget(targetRequest) )
        {
            if(as<IRHLSLStructuredBufferTypeBase>(type))
                return true;
            if(as<IRByteAddressBufferTypeBase>(type))
                return true;
        }

        // For now, we will not treat any other parameters as
        // needing specialization, even if they use resource
        // types like `Texure2D`, because these are allowed
        // as function parameters in both HLSL and GLSL.
        //
        // TODO: Eventually, if we start generating SPIR-V
        // directly rather than through glslang, we will need
        // to specialize *all* resource-type parameters
        // to follow the restrictions in the spec.
        //
        // TODO: We may want to perform more aggressive
        // specialization in general, especially insofar
        // as it could simplify away functions with resource-type
        // outputs.

        return false;
    }

    bool isArgSuitableForSpecialization(IRInst* inArg)
    {
        // Determining if an argument is suitable for
        // specialization a callee function requires
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
            if( arg->op == kIROp_getElement )
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
            // obersvation of what simple shaders do in
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
    // that means we end up having two different passes that walk
    // over the parameters/arguments of the call (in addition to
    // the ones we had above for determining if we can/should
    // specialize in the first place.
    //
    // The first of the two passes determines information
    // relevant to the call site, comprising both the arguments
    // that will be passed to the specialized function as
    // well as a "key" to identify the specialized function
    // that is required.
    //
    // The key type is similar to that used for generic specialization
    // elsewhere in the IR code. It might be worth pulling this
    // notion out somewhere more centralized, but we are dealing
    // with the code duplication for now.
    //
    struct Key
    {
        // The structure of a specialization key will be a list
        // of instruction starting with the function to be specialized,
        // and then having one or more entries for each parameter
        // that is being specialized to indicate the value to which
        // it is being specialized (e.g. the global shader parameter).
        //
        List<IRInst*> vals;

        // In order to use this type as a `Dictionary` key we
        // need it to support equality and hashing, but the
        // implementaitons are straightforward.
        //
        // TODO: honestly we might consider having `GetHashCode`
        // and `operator==` defined for `List<T>`.

        bool operator==(Key const& other) const
        {
            auto valCount = vals.Count();
            if(valCount != other.vals.Count()) return false;
            for( UInt ii = 0; ii < valCount; ++ii )
            {
                if(vals[ii] != other.vals[ii]) return false;
            }
            return true;
        }

        UInt GetHashCode() const
        {
            auto valCount = vals.Count();
            UInt hash = Slang::GetHashCode(valCount);
            for( UInt ii = 0; ii < valCount; ++ii )
            {
                hash = combineHash(hash, Slang::GetHashCode(vals[ii]));
            }
            return hash;
        }
    };

    // As indicated above, the information we collect about a call
    // site consists of the key for the specialized function we
    // will call, and a list of the arguments that will be passed
    // to the call.
    //
    struct CallSpecializationInfo
    {
        Key key;
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
        // That means the calle should be a known function
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
        // that we genereated (for another call site)
        // that is suitable to this call site.
        //
        IRFunc* newFunc = nullptr;
        if( !specializedFuncs.TryGetValue(callInfo.key, newFunc) )
        {
            // If we didn't find a pre-existing specialized
            // function, then we will go ahead and create one.
            //
            // We start by gathering the infromation from the call
            // site that is relevant to generating a specialized
            // callee function, and that we avoided doing earlier
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
            callInfo.newArgs.Count(),
            callInfo.newArgs.Buffer());

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
    // * What instructions are needed in the body of the callee to
    //   synthesize the value that will stand in for P?
    // * What information, if any, needs to be used to distinguish
    //   this specialized callee from others that might be generated for F?
    //
    // An easy case is when P is a parameter that doesn't need
    // specialization. In tha case:
    //
    // * The existing argument A shold be used as an argument in
    //   the specialized call.
    // * A clone P' of the existing parameter P shold be used as a
    //   parameter of the specialized callee.
    // * No additional instructions are needed in the body of
    //   the callee; the cloned parameter P' should stand in for P.
    // * No inforamtion should be added to the specialization key
    //   based on this P/A.
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
    //   shader parameter.
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
    // case of arbitrarily many levels of indexing, which can be
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
        callInfo.key.vals.Add(oldFunc);

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
            ioInfo.newArgs.Add(oldArg);
        }
        else
        {
            // If specialization is needed, we need
            // to inspect the argument value. This
            // is handled with a different function
            // because it needs to recurse in some case.
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
            // as an argument a the new call site (the
            // global parameter will get specialized into
            // the callee), but we *do* need to make sure
            // that our key for identifying the specialized
            // callee reflects that we are specializing
            // to the chosen parameter.
            //
            ioInfo.key.vals.Add(oldGlobalParam);
        }
        else if( oldArg->op == kIROp_getElement )
        {
            // This is the case where we `oldArg` is
            // in the form `oldBase[oldIndex]`
            //
            auto oldBase = oldArg->getOperand(0);
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
            ioInfo.newArgs.Add(oldIndex);
        }
        else
        {
            // If we fail to match one of th cases above
            // then a precondition was violated in that
            // `isArgSuitableForSpecialization` is allowing
            // a case that this routine is not covering.
            //
            SLANG_UNEXPECTED("mising case in 'getCallInfoForArg'");
            UNREACHABLE_RETURN(nullptr);
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
            funcInfo.replacementsForOldParameters.Add(newVal);
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
            ioInfo.newParams.Add(newParam);

            // The new parameter will be used as the replacement
            // for the old one in the specialized function.
            //
            return newParam;
        }
        else
        {
            // If the parameter requires specialization, then it
            // is time to look at the strucrure of the argument.
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
        else if( oldArg->op == kIROp_getElement )
        {
            // This is the case where the argument is
            // in the form `oldBase[oldIndex]`.
            //
            auto oldBase = oldArg->getOperand(0);
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
            // which means creating a parameter to recive its value,
            // which will also stand in for `oldIndex` in
            // the body of the specialized callee.
            //
            auto builder = getBuilder();
            auto newIndex = builder->createParam(oldIndex->getFullType());
            ioInfo.newParams.Add(newIndex);

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
            // add it to our gatehred list of instructions
            // that should be inserted into the body of
            // the specialized callee.
            //
            ioInfo.newBodyInsts.Add(newVal);

            return newVal;
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

    // Now that we've covered how all the relevant information
    // gets gathered, we can turn our attention to the
    // meat of actually generating a specialized version
    // of a function.
    //
    // For the most part, this is just a matter of *cloning*
    // the original function, while keeping around a mapping
    // from original values/instructions to their replacements.
    //
    // Because we might perform specialization many times,
    // it will get is own nested context type.
    //
    struct CloneContext
    {
        // When cloning, we need an IR builder to use for
        // making new instructions.
        //
        IRBuilder* builder;

        // We also need a mapping from old instruction to their
        // new equivalents, which will serve double duty:
        //
        // * Before we start cloning, this will be used to
        //   register the mapping from things that are to be
        //   replaced entirely (like function parameters to
        //   be specialized away) to their replacements (like
        //   a global shader parameter).
        //
        // * During the process of cloning, this will be
        //   updated as we clone instructions so that when
        //   an instruction later in the function refers to
        //   something from earlier, we can look up the
        //   replacement.
        //
        Dictionary<IRInst*, IRInst*> mapOldValToNew;

        // Whenever we need to look up an operand value
        // during the cloning process we'll use `cloneOperand`,
        // which mostly just uses `mapOldValToNew`.
        //
        IRInst* cloneOperand(IRInst* oldOperand)
        {
            IRInst* newOperand = nullptr;
            if(mapOldValToNew.TryGetValue(oldOperand, newOperand))
                return newOperand;

            // The one wrinkle here, and the place where
            // this cloning logic differs from some other
            // IR cloning implementations we have lying around,
            // is that when we *don't* find an instruction in
            // our map, we automatically assume it is not
            // something we need to be clone, so that the old
            // value is fine to use as-is.
            //
            // Note that this puts an ordering constraint on
            // our work: if we are going to clone some instruction
            // A, then we had better clone it *before* anything
            // that uses A as an operand.
            //
            return oldOperand;
        }

        // The SSA property and the way we have structured
        // our "phi nodes" (block parameters) means that
        // just going through the children of a function,
        // and then the children of a block will generally
        // do the Right Thing and always visit an instruction
        // before its uses.
        //
        // The big exception to this is that branch instructions
        // can refer to blocks later in the same function.
        //
        // We work around this sort of problem in a fairly
        // general fashion, by splitting the cloning of
        // an instruction into two steps.
        //
        // The first step is just to clone the instruction
        // and its direct operands, but not any decorations
        // or children.
        //
        IRInst* cloneInstAndOperands(IRInst* oldInst)
        {
            // In order to clone an instruction we first
            // need to map its operands over to their
            // new values.
            //
            List<IRInst*> newOperands;
            UInt operandCount = oldInst->getOperandCount();
            for(UInt ii = 0; ii < operandCount; ++ii)
            {
                auto oldOperand = oldInst->getOperand(ii);
                auto newOperand = cloneOperand(oldOperand);
                newOperands.Add(newOperand);
            }

            // Now we can just tell the IR builder to
            // go and create an instruction directly
            //
            // Note: this logic would not handle any instructions
            // with special-case data attached, but that only
            // applies to `IRConstant`s at this point, and those
            // should only appear at the global scope rather than
            // in function bodies.
            //
            SLANG_ASSERT(!as<IRConstant>(oldInst));
            auto newInst = builder->emitIntrinsicInst(
                oldInst->getFullType(),
                oldInst->op,
                newOperands.Count(),
                newOperands.Buffer());

            return newInst;
        }

        // The second phase of cloning an instruction is to clone
        // its decorations and children. This step only needs to
        // be performed on those instructions that *have* decorations
        // and/or children.
        //
        // The complexity of this step comes from the fact that it
        // needs to sequence the two phases of cloning for any
        // child instructions. We will do this by performing the
        // first phase of cloning, and building up a list of
        // children that require the second phase of processing.
        // Each entry in that list will be a pair of an old instruction
        // and its new clone.
        //
        struct OldNewPair
        {
            IRInst* oldInst;
            IRInst* newInst;
        };
        void cloneInstDecorationsAndChildren(IRInst* oldInst, IRInst* newInst)
        {
            List<OldNewPair> pairs;
            for( auto oldChild : oldInst->getDecorationsAndChildren() )
            {
                // A a very subtle special case, if one of the children
                // of our `oldInst` already has a registered replacement,
                // then we don't want to clone it (not least because
                // the `Dictionary::Add` method would give us an error).
                //
                // This arises for entries in `mapOldValToNew` that were
                // seeded before cloning begain (e.g., the function
                // parameters that are to be replaced).
                //
                if(mapOldValToNew.ContainsKey(oldChild))
                    continue;

                // Because we are re-using the same IR builder in
                // multiple places, we need to make sure to set
                // its insertion location before creating the
                // child instruction.
                //
                builder->setInsertInto(newInst);

                // Now we can perform the first phase of cloning
                // on the child, and register it in our map from
                // old to new values.
                //
                auto newChild = cloneInstAndOperands(oldChild);
                mapOldValToNew.Add(oldChild, newChild);

                // If an only if the old child had decorations
                // or children, we will register it into our
                // list for processing in the second phase.
                //
                if( oldChild->getFirstDecorationOrChild() )
                {
                    OldNewPair pair;
                    pair.oldInst = oldChild;
                    pair.newInst = newChild;
                    pairs.Add(pair);
                }
            }

            // Once we have done first-phase processing for
            // all child instructions, we scan through those
            // in the list that required second-phase processing,
            // and clone their decorations and/or children recursively.
            //
            for( auto pair : pairs )
            {
                auto oldChild = pair.oldInst;
                auto newChild = pair.newInst;

                cloneInstDecorationsAndChildren(oldChild, newChild);
            }
        }
    };

    // With all of that machinery out of the way,
    // we are now prepared to walk through the process of
    // specializing a given calee function based on
    // the information we have gathered.
    //
    IRFunc* generateSpecializedFunc(
        IRFunc*                         oldFunc,
        FuncSpecializationInfo const&   funcInfo)
    {
        // We start by setting up our context for cloning
        // the blocks and instructions in the old function.
        //
        auto builder = getBuilder();
        CloneContext cloneContext;
        cloneContext.builder = builder;

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
            cloneContext.mapOldValToNew.Add(oldParam, newVal);
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
            paramTypes.Add(param->getFullType());
        }
        IRType* funcType = builder->getFuncType(
            paramTypes.Count(),
            paramTypes.Buffer(),
            oldFunc->getResultType());

        IRFunc* newFunc = builder->createFunc();
        newFunc->setFullType(funcType);

        // The above step has accomplished the "first phase"
        // of cloning the function (since `IRFunc`s have no
        // operands).
        //
        // We can now call into our `CloneContext` to perform
        // the second phase of cloning, which will recursively
        // clone any nested decorations, blocks, and instructions.
        //
        cloneContext.cloneInstDecorationsAndChildren(oldFunc, newFunc);

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
        // gathering, and insert each before `newfirstOrdinary`,
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
void specializeResourceParameters(
    CompileRequest* compileRequest,
    TargetRequest*  targetRequest,
    IRModule*       module)
{
    ResourceParameterSpecializationContext context;
    context.compileRequest = compileRequest;
    context.targetRequest = targetRequest;
    context.module = module;

    context.processModule();
}

} // namesapce Slang
