//TEST_IGNORE_FILE:

module "conditional-associated-identity-caller";

import "conditional-associated-identity-module";

public struct Wrapper<T : IIdentityValue> { public T.Value value; }

public T read<T>(Wrapper<Box<T>> box) where Box<T> : IIdentityValue
{
    // The constraint cannot prove IMarker directly, but all possible implementations for
    // this nominal family give Value = Box<T>. No vector-specific member lookup is involved.
    return box.value.payload;
}

int consumeData<D : IData>(D value) { return value.get(); }

public int readBound<T>(Wrapper<Box<T>> box) where Box<T> : IIdentityValue
{
    // Reducing Value to Box<T> must retain the conformance promised by Value : IData.
    return consumeData(box.value);
}
