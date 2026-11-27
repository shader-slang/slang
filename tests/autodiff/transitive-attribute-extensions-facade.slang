//TEST_IGNORE_FILE:

import "transitive-attribute-extensions-lib";

public struct Container { public Operations operations; }
public struct SampleContainer { public Samples samples; }
public typealias ExposedOperations = Operations;
