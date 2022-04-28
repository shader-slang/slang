// slang-ir-liveness.h
#ifndef SLANG_IR_LIVENESS_H
#define SLANG_IR_LIVENESS_H

namespace Slang
{

struct IRModule;

/* Motivation for tracking liveness.

At a first approximation liveness means variable is `in scope`. The underlying issue might be described as

```HLSL

struct SomeStruct
{
	int value;
	int large[100];
};

int someFunction()
{
	int result = 0;

	for (int i = 0; i < ...; ++i)
	{
		doSomething();

	    SomeStruct s;
		s.value = ...;

		doSomethingElse(s);

		result += s.value;
	}

	return result;
}
```

A compiler might hoist `s` outside of the loop, looking something more like...

```
int someFunction()
{
	SomeStruct s;
	int result = 0;

	for (int i = 0; i < ...; ++i)
	{
		doSomething();

		s.value = ...;

		result += doSomethingElse(s);
	}

	return result;
}
```

The problem is that now `s` is in scope over the loop, and there is potential for values from one interation 
to be used in the next iteration. This isn't a problem in the original version because it is 'obvious' that 
a new `s` is constructed each iteration. The key observation being that when doSomething is executing, `s` doesn't exist,
and so doesn't need to take any register space.

Why hoist? Some compilers define structures via `alloca`s, and these allocas can only be placed at the start of the function.

This would be one level of `liveness`. 

Another observation could be around field liveness. s has no `__init` and isn't initialized in any way. s.value does set some
state, but `large` is untouched. So in a sense s.value holds *all* of the state of s at that point, and only s.value 
would need to be stored to reconstruct s (the rest could be undefined). 

Is this more nuanced information useful to a downstream compilation? Maybe, but the downstream compiler could perform all the same 
analysis. All it's really missing is knowing when there is a `fresh version` of s.

How does this apply to undefined values?

```
int someFunction()
{
	int result = 0;
	int v;				/// v's value is undefined

	for (int i = 0; i < ...; ++i)
	{
		doSomething();

		SomeStruct s;
		s.value = v;

		result += doSomethingElse(s);
	}

	return result;
}
```

In this somewhat silly example, s.value is set to an undefined value. At one level you could say that s is *all* in an undefined state, 
and therefore s is stateless. That's not quite right though because although v is undefined, it should probably be the same value 
every loop. 

Like before though, this may not matter too much in practice because a downstream compiler can see this behavior, and handle appropriately.

Another way a compiler could `see` that it has a `fresh copy` within the loop, would be for all it's state to be set.

```
int someFunction()
{
	SomeStruct s;
	
	int result = 0;
	
	for (int i = 0; i < ...; ++i)
	{
		doSomething();

		// (Note the syntax here is not Slang/HLSL, it's just meant to mean 'initialize s')
		s = SomeStruct{};

		s.value = v;

		result += doSomethingElse(s);
	}

	return result;
}
```

Here because of the initialization of *all* of `s`, a downstream compiler can infer that during `doSomething` it doesn't have to potentially store the contents
of `s` because it will be wiped out after the function. 

All of this gets more confusing around branches. But again that is something a downstream compiler can track if it has a way of knowing when a variable is in scope. 
Similarly calling into a function could return a struct that contains fields which aren't set - this is something a downstream compiler could determine when 
fully specialized.

From the Slang stores where s comes into scope. 

Questions?

Presumably a variable in a prior block is potentially available if it's defined in a block that dominates the node where it is defined. Determining where scope ends
is therefore a question of examining that tree. It would seem it would have to add a live end at the last read acess or it or some subpart of it (after all a write is invisible if it's never read). 
If there are branches doesn't that imply (unless there is some kind of merge point), that there can be multiple ends depending on the unique paths possible?

How does scoping work with phi nodes? 

By making phi node variables not passed by parameter, we can see where they are in effect `live`, when they are assigned to. 

Can parameters to a dominating block be seen by subsequent blocks?

*/

/// Adds LiveStart and LiveEnd instructions to demark the start and end of the liveness of a variable.
void addLivenessTrackingToModule(IRModule* module);

}

#endif // SLANG_IR_LIVENESS_H