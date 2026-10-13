//TEST_IGNORE_FILE:

module exported_import_lib;

__exported import slang.numerics;

public T exportedWave<T : IAdditive & ITrigonometricFunctions>(T value)
{
    return sin(value + value);
}
