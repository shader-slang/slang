# IR Global Value Deduplication

Types, constants and certain operations on constants are considered "global value" in the Slang IR. Some other insts like `Specialize()` and `Ptr(x)` are considered as "hoistable" insts, in that they will be defined at the outer most scope where their operands are available. For example, `Ptr(int)` will always be defined at global scope(as direct children of `IRModuleInst`) because its only operand, `int`, is defined at global scope. However if we have `Ptr(T)` where `T` is a generic parameter, then this `Ptr(T)` inst will be always be defined in the block of the generic. Global and hoistable values are always deduplicated and we can always assume two hoistable values with different pointer addresses are distinct values.

The `IRBuilder` class is responsible for ensuring the uniqueness of global/hoistable values. If you call any `IRBuilder` methods that creates a new hoistable instruction, e.g.  `IRBuilder::createIntrinsicInst`, `IRBuilder::emitXXX` or `IRBuilder::getType`, `IRBuilder` will check if an equivalent value already exists, and if so it returns the existing inst instead of creating a new one.

The trickier part here is to always maintain the uniqueness when we modify the IR. When we update the operand of an inst from a non-hoistable-value to a hoistable-value, we may need to hoist `inst` itself as a result. For example, considered the following code:
```
%1 = IntType
%p = Ptr(%1)
%2 = func {
   %x = ...;
   %3 = Ptr(%x);
   %4 = ArrayType(%3);
   %5 = Var (type: %4);
   ...
}
```

Now consider the scenario where we need to replace the operand in `Ptr(x)` to `int` (where `x` is some non-constant value), we will get a `Ptr(int)` which is now a global value and should be deduplicated:
```
%1 = IntType
%p = Ptr(%1)
%2 = func {
   %x = ...;
   //%3 now becomes %p.
   %4 = ArrayType(%p);
   %5 = Var (type: %4);
   ...
}
```
Note this code is now breaking the invariant that hoistable insts are always defined at the top-most scope, because `%4` becomes is no longer dependent on any local insts in the function, and should be hoisted to the global scope after replacing `%3` with `%p`. This means that we need to continue to perform hoisting of `%4`, to result this final code:
```
%1 = IntType
%p = Ptr(%1)
%4 = ArrayType(%p); // hoisted to global scope
%2 = func {
   %x = ...;
   %5 = Var (type: %4);
   ...
}
```

As illustrated above, because we need to maintain the invariants of global/hoistable values, replacing an operand of an inst can have wide-spread effect on the IR.

To help ensure these invariants, we introduce the `IRBuilder.replaceOperand(inst, operandIndex, newOperand)` method to perform all the cascading modifications after replacing an operand. However the `IRInst.setOperand(idx, newOperand)` will not perform the cascading modifications, and using `setOperand` to modify the operand of a hoistable inst will trigger a runtime assertion error.

Similarly, `inst->replaceUsesWith` will also perform any cascading modifications to ensure the uniqueness of hoistable values. Because of this, we need to be particularly careful when using a loop to iterate the IR linked list or def-use linked list and call `replaceUsesWith` or `replaceOperand` inside the loop.

Consider the following code:

```
IRInst* nextInst = nullptr;
for (auto inst = func->getFirstChild(); inst; inst = nextInst)
{
     nextInst = inst->getNextInst(); // save a copy of nestInst
     // ...
     inst->replaceUsesWith(someNewInst); // Warning: this may be unsafe, because nextInst could been moved to parent->parent!
}
```

Now imagine this code is running on the `func` defined above, imagine we are now at `inst == %3` and we want to replace `inst` with `Ptr(int)`. Before calling `replaceUsesWith`, we have stored `inst->nextInst` to `nextInst`, so `nextInst` is now `%4`(the array type). Now after we call `replaceUsesWith`, `%4` is hoisted to global scope, so in the next iteration, we will start to process `%4` and follow its `next` pointer to `%2` and we will be processing `func` instead of continue walking the child list!

Because of this, we should never be calling `replaceOperand` or `replaceUsesWith` when we are walking the IR linked list. If we want to do so, we must create a temporary workList and add all the insts to the work list before we make any modifications. The same can be said to the def-use linked list. There is `traverseUses` and `traverseUsers` utility functions defined in `slang-ir.h` to help with walking the def-use list safely.

Another detail to keep in mind is that  any local references to an inst may become out-of-date after a call to `replaceOperand` or `replaceUsesWith`. Consider the following code:
```
IRBuilder builder;
auto x = builder.emitXXX(); // x is some non-hoistable value.
auto ptr = builder.getPtrType(x);  // create ptr(x).
x->replaceUsesWith(intType); // this renders `ptr` obsolete!!
auto var = builder.emitVar(ptr); // use the obsolete inst to create another inst.
```
In this example, calling `replaceUsesWith` will cause `ptr` to represent `Ptr(int)`, which may already exist in the global scope. After this call, all uses of `ptr` should be replaced with the global `Ptr(int)` inst instead. `IRBuilder` has provided the mechanism to track all the insts that are removed due to deduplication, and map those removed but not yet deleted inst to the existing inst. When using `ptr` to create a new inst, `IRBuilder` will first check if `ptr` should map to some existing hoistable inst in the global deduplication map and replace it if possible. This means that after the call to `builder.emitVar`, `var->type` is not equal to to `ptr`.

## Best Practices

In summary, the best practices when modifying the IR is:
- Never call `replaceUsesWith` or `replaceOperand` when walking raw linked lists in the IR. Always create a work list and iterate on the work list instead.
- Never assume any local references to an `inst` is up-to-date after a call to `replaceUsesWith` or `replaceOperand`. It is OK to continue using them as operands/types to create a new inst, but do not assume the created inst will reference the same inst passed in as argument.
