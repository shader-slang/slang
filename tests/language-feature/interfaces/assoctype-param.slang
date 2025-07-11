//TEST:INTERPRET(filecheck=CHECK):

public interface IDataTrait {
    public associatedtype InputType;
    public static const int32_t kElementCount;
}

public struct DataTrait0 : IDataTrait {
    public typedef float InputType;
    public static const int32_t kElementCount = 2;
}

public interface IGenericInterface<Ti : IFloat> {
    public Array<Di.InputType, Di.kElementCount> eval<Di : IDataTrait>(const Di.InputType interface_input);
}

public struct GenericImpl<T : IFloat> : IGenericInterface<T>
{
    public Array<Dx.InputType, Dx.kElementCount> eval<Dx : IDataTrait>(
        const Dx.InputType impl_input)
    {
        return makeArrayFromElement<Dx.InputType, Dx.kElementCount>(impl_input);
    }
}

void main()
{
    GenericImpl<float> f;
    let rs = f.eval<DataTrait0>(1.0);
    printf("result is %f\n", rs[0]);
    // CHECK: result is 1.0
}
