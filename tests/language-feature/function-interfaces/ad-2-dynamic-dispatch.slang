//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    static float zap(float x) : IBackwardDifferentiable<__func_as_type(This.zap)>;
};

struct FooImpl1 : IFoo
{
    static float zap(float x)
    {
        return 2 * x;
    }
};

struct FooImpl2 : IFoo
{
    static float zap(float x)
    {
        return 4 * x;
    }
};

struct BwdCallableForZapImpl1 : IBwdCallable<__func_as_type(FooImpl1::zap)>
{
    float x;
    float val() { return FooImpl1::zap(x); } // primal value
    void operator()(out float.Differential dx, float.Differential dOut)
    {
        dx = 2 * x * dOut;
    }
};

extension __func_as_type(FooImpl1::zap) : IBackwardDifferentiable<__func_as_type(FooImpl1::zap)>
{
    typealias BwdCallable = BwdCallableForZapImpl1;
    static This.BwdCallable apply_bwd(float x)
    {
        return BwdCallableForZapImpl1(x);
    }
}

struct BwdCallableForZapImpl2 : IBwdCallable<__func_as_type(FooImpl2::zap)>
{
    float x;
    float val() { return FooImpl2::zap(x); }
    void operator()(out float.Differential dx, float.Differential dOut)
    {
        dx = 4 * x * dOut;
    }
};

extension __func_as_type(FooImpl2::zap) : IBackwardDifferentiable<__func_as_type(FooImpl2::zap)>
{
    typealias BwdCallable = BwdCallableForZapImpl2;
    static This.BwdCallable apply_bwd(float x)
    {
        return BwdCallableForZapImpl2(x);
    }
}

IFoo getFoo(uint id)
{
    if (id == 0)
        return FooImpl1();
    else
        return FooImpl2();
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IFoo foo = getFoo(tig % 2);

    // foo.zap --> IBackwardDifferentiable<foo.__table.zap>
    let bwdCallable = foo.zap.apply_bwd(1.f); // primal_ctx (bwdCallable -> intermediate-context struct + call() function which is bwd-prop)
    outputBuffer[tig] = bwdCallable.val(); // primal value
    
    var dx = 0.f;
    bwdCallable(dx, 2.0); // bwd-prop (dresult = 2.0)
    outputBuffer[tig + 1] = dx; // Expect: 4.0

    bwdCallable(dx, 3.0); // bwd-prop 
    outputBuffer[tig + 2] = dx; // Expect: 6.0
}
/*
table_def IFoo
{
    zap_key : (FuncType(float -> float))
    zap_IBwdDiff_key : (WitnessTableType<IBackwardDifferentiable<zap_key>>)

    // synthesized.
    _syn_zap_IBwdDiff_apply_bwd_key : FuncType(float -> zap_IBwdDiff_bwd_callable_key)
    _syn_zap_IBwdDiff_bwd_callable_key : (AssociatedType<I_syn_BwdCallable>)
    _syn_zap_IBwdDiff_bwd_callable_IBwdCallable_key : (WitnessTableType<I_syn_BwdCallable>)
}

table_def I_syn_BwdCallable
{
    // 
    val_key : FuncType(ThisType -> float)
    operator_call_key : FuncType((ThisType, out float, float) -> void)
}

table_def IBackwardDifferentiable<FType>
{
    bwd_callable_key : (AssociatedType<IBwdCallable<FType>>)
    bwd_callable_table_key : (WitnessTableType<IBwdCallable<FType>>)
    apply_bwd_key : (ApplyBwdFuncType(FType))
}

table_def IBwdCallable<FType>
{
    val_key : (ResultFuncType(FType))
    operator_call_key : (BackPropFuncType(FType))
}
IBackwardDifferentiable<key>            ->  
IBackwardDifferentiable<FooImpl1::zap>  
IBackwardDifferentiable<FooImpl2::zap>
// ----- -----
void computeMain(uint tig : SV_GroupIndex)
{
    dyn IFoo foo = getFoo(tig % 2);
    IFoo_zap_BwdCallable ()

    // foo.zap --> IBackwardDifferentiable<foo.__table.zap>
    let table : WitnessTableType<IFoo> = foo._table;
    {
        {
            let zap_func : FuncType(float -> float) = table.zap_key;
            let zap_bwd_diff_table : WitnessTableType<IBackwardDifferentiable<zap_func>> = table.zap_IBwdDiff_key;
            let bwd_callable_type : TypeKind = zap_bwd_diff_table.bwd_callable_key;
            let apply_bwd_func : FuncType(float -> bwd_callable_type) = zap_bwd_diff_table.apply_bwd_key;
            let bwd_callable : bwd_callable_type = apply_bwd_func(1.f);

            branch %next %bwd_callable
        }
        %next: {
            param bwd_callable : bwd_callable_type;
            let bwd_callable_table : WitnessTableType<IBwdCallable<zap_func>> = zap_bwd_diff_table.bwd_callable_table_key;
            let val_func : FuncType(bwd_callable_type -> float) = bwd_callable_table.val_key;
            let val_func_result : float = val_func(bwd_callable);
        }
    }

    // Strategy 1: Key compression: compress the keys... (if there are multiple key lookups, insert them into the "base" table)
    {
        {
            let zap_func: FuncType(float -> float) = table.zap_key;
            let bwd_callable_type: TypeKind = table._syn_zap_IBwdDiff_BwdCallable_key;
            let apply_bwd_func: FuncType(float -> bwd_callable_type) = table._syn_zap_IBwdDiff_apply_bwd_key;
            let bwd_callable: bwd_callable_type = apply_bwd_func(1.f);
            
            extend table_def IFoo
            {
                _syn_zap_IBwdDiff_BwdCallable : AssociatedType<I__syn_IFoo_IBwdDiff_BwdCallable>
                _syn_zap_IBwdDiff_apply_bwd_key : FuncType(float -> AssociatedType(I__syn_IFoo_IBwdDiff_BwdCallable))
            }

            extend global 
            {
                table_def I__syn_IFoo_BwdDiff_BwdCallable
                {}
            }
            
            extend table (FooImpl1 : IFoo)
            {
                _syn_zap_IBwdDiff_BwdCallable : Lookup(Lookup(FooImpl1, zap_IBwdDiff_key), bwd_callable_key) 
                _syn_zap_IBwdDiff_apply_bwd_key : Lookup(Lookup(FooImpl1, zap_IBwdDiff_key), apply_bwd_key)
            }

            extend table (FooImpl2 : IFoo)
            {
                _syn_zap_IBwdDiff_BwdCallable : Lookup(Lookup(FooImpl2, zap_IBwdDiff_key), bwd_callable_key) 
                _syn_zap_IBwdDiff_apply_bwd_key : Lookup(Lookup(FooImpl2, zap_IBwdDiff_key), apply_bwd_key)
            }
            // Extend any other IFoo tables..

            branch %next %bwd_callable
        }

        // This is the same as the above, but with the "base" table
        %next: {
            param bwd_callable : bwd_callable_type;
            let val_func: FuncType(bwd_callable_type -> float) = table._syn_zap_IBwdDiff_BwdCallable_IBwdCallable_val_key;
            let val_func_result: float = val_func(bwd_callable);

            extend table_def IFoo
            {
                _syn_zap_IBwdDiff_BwdCallable_IBwdCallable_val_key : FuncType(AssociatedType(I__syn_IFoo_IBwdDiff_BwdCallable) -> float)
            }

            extend table (FooImpl1 : IFoo)
            {
                _syn_zap_IBwdDiff_BwdCallable_IBwdCallable_val_key : Lookup(
                    Lookup(Lookup(FooImpl1, zap_IBwdDiff_table_key), IBwdDiff_BwdCallable_IBwdCallable_table_key), val_key)
            }

            extend table (FooImpl2 : IFoo)
            {
                _syn_zap_IBwdDiff_BwdCallable_IBwdCallable_val_key : Lookup(
                    Lookup(Lookup(FooImpl2, zap_IBwdDiff_table_key), IBwdDiff_BwdCallable_IBwdCallable_table_key), val_key)
            }
        }
    }

    // Try the same thing on a different version.. 
    {
        let table : WitnessTableType<IFoo> = foo._table;
        {
            {
                let zap_func : FuncType(float -> float) = table.zap_key;
                let zap_bwd_diff_table : WitnessTableType<IBackwardDifferentiable<zap_func>> = table.zap_IBwdDiff_key;
                let bwd_callable_type : TypeKind = zap_bwd_diff_table.bwd_callable_key;
                let apply_bwd_func : FuncType(float -> bwd_callable_type) = zap_bwd_diff_table.apply_bwd_key;
                let bwd_callable : bwd_callable_type = apply_bwd_func(1.f);
                let bwd_callable_box : IBwdCallable<zap_func> = MakeExistential(bwd_callable_type, zap_bwd_diff_table.bwd_callable_table_key, bwd_callable);
                branch %next %bwd_callable_box
            }

            %next: {
                param bwd_callable_box : IBwdCallable<zap_func>;
                let bwd_callable_table : WitnessTableType<IBwdCallable<zap_func>> = bwd_callable_box._table;
                let bwd_callable_type : TypeKind = bwd_callable_box._type;
                let val_func : FuncType(bwd_callable_type -> float) = bwd_callable_table.val_key;
                let val_func_result : float = val_func(bwd_callable_box._value);
            }
        }
    }

    // Use the same approach:
    {
        let table : WitnessTableType<IFoo> = foo._table;
        {
            {
                let zap_func : FuncType(float -> float) = table.zap_key;
                let bwd_callable_type : TypeKind = table.zap_IBwdDiff_key_BwdCallable_key;
                let apply_bwd_func : FuncType(float -> bwd_callable_type) = table.zap_IBwdDiff_key_apply_bwd_key;
                let bwd_callable : bwd_callable_type = apply_bwd_func(1.f);
                let bwd_callable_box : I__syn_IFoo_IBwdDiff_BwdCallable = MakeExistential(bwd_callable_type, table.zap_IBwdDiff_key_BwdCallable_table_key, bwd_callable);

                extend table_def IFoo
                {
                    zap_IBwdDiff_key_BwdCallable_key : AssociatedType<I__syn_IFoo_IBwdDiff_BwdCallable>
                    zap_IBwdDiff_key_BwdCallable_table_key : WitnessTableType<I__syn_IFoo_IBwdDiff_BwdCallable>
                    zap_IBwdDiff_key_apply_bwd_key : FuncType(float -> zap_IBwdDiff_key_BwdCallable_key)
                }

                extend global 
                {
                    table_def I__syn_IFoo_BwdDiff_BwdCallable
                    {}
                }

                extend table (FooImpl1 : IFoo)
                {
                    zap_IBwdDiff_key_BwdCallable : Lookup(Lookup(FooImpl1, zap_IBwdDiff_key), bwd_callable_key) 
                    zap_IBwdDiff_key_BwdCallable_table : table (Lookup(Lookup(FooImpl1, zap_IBwdDiff_key), bwd_callable_key) : I__syn_IFoo_BwdDiff_BwdCallable)
                        { }
                    zap_IBwdDiff_key_apply_bwd_key : Lookup(Lookup(FooImpl1, zap_IBwdDiff_key), apply_bwd_key)
                }

                extend table (FooImpl2 : IFoo)
                {
                    zap_IBwdDiff_key_BwdCallable : Lookup(Lookup(FooImpl2, zap_IBwdDiff_key), bwd_callable_key) 
                    zap_IBwdDiff_key_BwdCallable_table : table (Lookup(Lookup(FooImpl2, zap_IBwdDiff_key), bwd_callable_key) : I__syn_IFoo_BwdDiff_BwdCallable)
                        { }
                    zap_IBwdDiff_key_apply_bwd_key : Lookup(Lookup(FooImpl2, zap_IBwdDiff_key), apply_bwd_key)
                }

                branch %next %bwd_callable_box
            }

            %next: {
                param bwd_callable_box : I__syn_IFoo_BwdDiff_BwdCallable;
                let bwd_callable_table : WitnessTableType<I__syn_IFoo_BwdDiff_BwdCallable> = bwd_callable_box._table;
                let bwd_callable_type : TypeKind = bwd_callable_box._type;
                let val_func : FuncType(bwd_callable_type -> float) = bwd_callable_table.val_key;
                let val_func_result : float = val_func(bwd_callable_box._value);

                extend table_def I_syn_IFoo_BwdDiff_BwdCallable
                {
                    val_key : FuncType(ThisType -> float)
                }

                extend table (Lookup(Lookup(FooImpl1, zap_IBwdDiff_table_key), bwd_callable_key) : I__syn_IFoo_BwdDiff_BwdCallable)
                {
                    val_key : Lookup(Lookup(Lookup(FooImpl1, zap_IBwdDiff_table_key), bwd_callable_table_key), val_key)
                }

                extend table (Lookup(Lookup(FooImpl2, zap_IBwdDiff_table_key), bwd_callable_key) : I__syn_IFoo_BwdDiff_BwdCallable)
                {
                    val_key : Lookup(Lookup(Lookup(FooImpl2, zap_IBwdDiff_table_key), bwd_callable_table_key), val_key)
                }
            }
        }
    }


    //let bwdCallable = foo._table.zap.(zap : IBackwardDifferentiable<zap_key>).apply_bwd(1.f);
    outputBuffer[tig] = bwdCallable.val();
    
    var dx = 0.f;
    bwdCallable(dx, 2.0);
    outputBuffer[tig + 1] = dx; // Expect: 4.0

    bwdCallable(dx, 3.0);
    outputBuffer[tig + 2] = dx; // Expect: 6.0
}
*/