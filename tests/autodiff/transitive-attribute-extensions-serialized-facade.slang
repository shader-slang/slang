//TEST_IGNORE_FILE:

// The facade for `transitive-attribute-extensions-serialized.slang`, which differs from
// `transitive-attribute-extensions-facade.slang` only in importing the precompiled library.
import "transitive-attribute-extensions-serialized-lib";

public struct Container { public Operations operations; }
public struct SampleContainer { public Samples samples; }
public typealias ExposedOperations = Operations;
