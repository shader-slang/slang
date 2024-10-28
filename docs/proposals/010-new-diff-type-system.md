# SP #010: New Differentiable Type System

## Problem
Our current `IDifferentiable` system has some flaws. It works fine for value types, since we can assume that every input gets a corresponding output or 'return' value. It works poorly for buffer/pointer types, since we don't 'return' a buffer, but simply want the getters/setters to be differentiable, and the resulting type to have a second buffer/pointer for the differential data.

Here's a demonstrative example with our current codebase when we use value types (like `float`)
```csharp
[Differentiable]
float add(float a, float b)
{
	return a + b;
}

// Synthesized derivative:
[Differentiable]
void s_bwd_add(DifferentialPair<float> dpa, DifferentialPair<float> dpb, float.Differential d_out)
{
	// A backward derivative method is currently responsible for 'setting' the differential values.
	dpa = DifferentialPair<float>(dpa.p, d_out);
	dpb = DifferentialPair<float>(dpb.p, d_out);
}
```

Unfortunately, this makes little sense if we decide to use buffer or pointer types:
```csharp
struct DiffPtr<T> : IDifferentiable
{
	StructuredBuffer<T> bufferRef;
	uint64 offset;
	
	[Differentiable] T get() { ... }
	[Differentiable] void set(T t) { ... }
	/* 
	  Problem 1:
	  We use custom derivatives for get() and set() to backprop and 
	  read gradients. If DiffPtr<T> is differentiable, then get() and 
	  set() need to operate on the *pair* type and not this struct type.
	  There is no proper way to do this currently.
	*/
};

[Differentiable]
void add(DiffPtr<float> a, DiffPtr<float> b, DiffPtr<float> output)
{
	output.set(a.get() + b.get());
}

// Synthesized derivative:
[Differentiable]
void s_bwd_add(
	inout DifferentialPair<DiffPtr<float>> a,
	inout DifferentialPair<DiffPtr<float>> b,
	inout DifferentialPair<DiffPtr<float>> output)
{
	/* 
	   Problem 2:
	   
	   Current backward mode semantics require that the method assume that the differentials
	   a.d and b.d are empty/zero, and it is the backward method's job to populate the result.
	   
	   It doesn't make sense to 'set' the differential part since it is a buffer ref.
	   Rather, we want the user to provide the differential pointer, and use custom derivatives of 
	   the getters/setters to propagate derivatives.
	   
	   This also means methods like dzero(), dadd() and dmul() make no sense 
	   in the context of pointer types. They cannot be initialized within a derivative method.
	*/
}

```

## Workarounds
At the moment the primary workaround is to use a **non-differentiable buffer type** with differentiable methods, and always initialize the object with two pointers for both the primal and differential buffers. This is how our `DiffTensorView<T>` object works. 
Unfortunately, this is a rather hacky workaround with several drawbacks: 
1.  `DiffTensorView<T>` does not conform to `IDifferentiable`, but is used for derivatives. This makes our type system less useful as checks for `is_subtype` from applications using reflection need workarounds to account for corner cases like these.
2.  `DiffTensorView<T>` always has two buffer pointers even when used in non-differentiable methods. This is extra data in the struct, and potentially extra tensor allocations (we explicitly handle this case in `slangtorch` by leaving the diff part uninitialized if a primal method is invoked)
3.  Higher-order derivatives don't work well with this workaround. Differentiating a method twice needs a set of 4 pointers, but we need to account for this ahead of time by using new types like `DiffDiffTensorView` that worsens the problem of carrying around extra data where its not required.


## Solution

We'll need to make the following 4 additions/changes:
### 1. `[deriv_method]` function decorator.
Intended for easy definition of custom derivatives for struct methods. It has the following properties:
1. Accesses to `this` within `[deriv_method]`  are differential pairs.
2. Methods decorated with `[deriv_method]` cannot be called as regular methods (they can still be explicitly invoked with `bwd_diff(obj.method)`), and do not show up in the auto-complete list.

See the next section for example uses of `[deriv_method]`.

### 2. Split `IDifferentiable` interface: `IDifferentiableValueType` and `IDifferentiablePtrType`
This approach moves away from "type-driven" derivative semantics and towards more "function-driven" derivative semantics.
We no longer have a `dadd` , `dzero`, `dmul` etc.. we use default initialization instead of `dzero` and the backward derivative of the `use` method for `dadd`

Further, `IDifferentiablePtrType` types don't have any of these properties. They do not need a way to 'add', and it is especially important that there is no default initializer. We never want the compiler to be able to create a new object of  `IDifferentiablePtrType` since we want to get the user-provided pointers.

Additionally, we can use `IDifferentiableValueType` as the current `IDifferentiable` for backwards compatibility (it should just work in 95% of cases, since no one really defines dadd/dzero/dmul explicitly anyway) 

Here's the new set of base interfaces:
```csharp
interface __IDifferentiableBase { } // Helper type for our implementation.
interface IDifferentiableValueType : __IDifferentiableBase
{
	associatedtype Differential : IDifferentiableValueType & IDefaultInitializable;
	[Differentiable] This use(); // auto-synthesized
}

interface IDifferentiablePtrType : __IDifferentiableBase
{
	associatedtype Differential : IDifferentiablePtrType;
}

```

Some extras in the core module allow us to constrain the diffpair type for things like `IArithmetic`
```csharp
// --- CORE MODULE EXTRAS ---

interface ISelfDifferentiableValueType : IDifferentiableValueType
{
	// Force arithmetic types to be a differential pair of the same two types.
	// Make it simple to define derivatives of arithmetic operations.
	//
	associatedtype Differential : This;
}

extension IFloat : ISelfDifferentiableValueType
{ }

extension float
{
	// trivial auto-synthesis (maybe we even prevent the user from overriding this)
	float use() { return this; } 
	
	// trivial auto-synthesis (maybe we even prevent the user from overriding this).
	[ForwardDerivativeOf(use)]
	[deriv_method] void use_fwd() { return this; } 
	
	// auto-synthesized if necessary by invoking the use_bwd for all fields.
	// we need to provide implementation for 'leaf' types.
	[BackwardDerivativeOf(use)]
	[deriv_method] [mutating] void use_bwd(float d) { this.d += d; } 
}

// The new system lets us define differentiable pointers easily.
// IDifferentiablePtrType'd values are simply treated as references, so they can be freely
// duplicated without requiring a `use()` for correctness.
//
struct DPtr<T : IDifferentiableValueType> : IDifferentiablePtrType
{
	typealias Differential = DPtr<T.Differential>;
	
	Buffer<T> buffer;
	uint64 offset;
	
	[BackwardDerivative(get_bwd)]
	[BackwardDerivative(get_fwd)]
	T get() { return this.buffer[offset]; }
	
	[deriv_method] DifferentialPair<T> get_fwd()
	{ 
		return diffPair(this.p.buffer[offset], this.d().buffer[offset]);
	}
	
	[deriv_method] void get_bwd(Differential d)
	{
		return this.d.InterlockedAdd(offset, d);
	}
	
	DPtr<T> operator+(uint o) { return DPtr<T>{buffer, offset + o}; }
}

// Or we can define a fancier differentiable pointer that does a hashgrid
struct DHashGridPtr<T : IDifferentiableValueType, let N: int> : IDifferentiablePtrType
{ 
	typealias Differential = DPtr<T.Differential>;
	
	Buffer<T> buffer;
	uint64 offset;
	
	[BackwardDerivative(get_bwd)]
	[BackwardDerivative(get_fwd)]
	T get() { return this.buffer[offset]; }
	
	[deriv_method] DifferentialPair<T> get_fwd()
	{ 
		return diffPair(this.p().buffer[offset], this.d().buffer[offset]);
	}
	
	[deriv_method] void get_bwd(Differential d)
	{
		return this.d().InterlockedAdd(offset * N + hash(get_thread_id()), d);
	}
}
```

### 3. Every time we 'reuse' an object that conforms to `IDifferentiableValueType`, we split it with `use()` , and we use `__init__()` where necessary to initialize an accumulator.
Example:
```csharp
float f(float a)
{
	add(a, a);
}
float add(float a, float b)
{
	return a + b;
}

// Synthesized derivatives
void add_bwd(inout DiffPair<float> dpa, inout DiffPair<float> dpb, float d_out)
{
	 dpa = diffPair(dpa.p, d_out);
	 dpb = diffPair(dpb.p, d_out);
}

// Preprocessed-f (before derivative generation)
float f_with_use_expansion(float a)
{
	DiffPair<float> a_extra = a.use();
	return add(a, a_extra);
}

// After fwd-mode:
DiffPair<float> f_fwd(DiffPair<float> dpa)
{
	DiffPair<float> dpa_extra = dpa.use_fwd();
	return add_fwd(a, a_extra_fwd);
}


// bwd-mode:
void f_bwd(inout DiffPair<float> dpa, float d_out)
{
	// fwd-pass

	// split
	DiffPair<float> dpa_extra = dpa.use_fwd();
	// -------
	
	// bwd-pass
	dpa_extra_bwd = DiffPair<float>(dpa_extra.p, float.Differential::__init__());
	add_bwd(dpa, dpa_extra, d_out);
	
	// merge
	dpa.use_bwd(dpa_extra);
}
```

### 4. Objects that conform to `IDifferentiablePtrType` are used without splitting. They are simply not 'transposed' at all, because there is nothing to transpose. The fwd-mode pair is used as is.
Here's the same example above, but with the `DPtr` type defined above.

```csharp
void f(DPtr<float> a, DPtr<float> output)
{
	add(a, a, output);
}

void add(DPtr<float> a, DPtr<float> b, DPtr<float> output)
{
	output.set(a.get() + b.get());
}

// Synthesized derivatives 
// (note: no inout req'd for IDifferentiablePtrType)
// important difference is that `ptr` types don't get transposed, only 
// methods on the objects are.
// they DO NOT have a default initializer (the user must supply the differential part)
void add_bwd(
	DifferentialPair<DPtr<float>> dpa,
	DifferentialPair<DPtr<float>> dpb, 
	DifferentialPair<DPtr<float>> output)
{
	// forward pass.
	var a_p = dpa.p.get();
	var b_p = dpb.p.get();
	// ----
	
	// backward pass.
	float.Differential d_val = DPtr<float>::set_bwd(output); // set_bwd works on the entire pair.
	DifferentialPair<float> a_get_bwd = diffPair(a_p, float.Differential::__init__());
	DifferentialPair<float> b_get_bwd = diffPair(b_p, float.Differential::__init__());
	operator_float_add_bwd(a_get_result_bwd, b_get_result_bwd, d_val);
	DPtr<float>::get_bwd(dpa);
	DPtr<float>::get_bwd(dpb);
}
```
