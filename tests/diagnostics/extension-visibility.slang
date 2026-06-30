// extension-visibility.slang

// Confirm that visibility of `extensions` is
// correctly scoped via `import`.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

import extension_visibility_a;

// Note: not importing b:
// import extension_visibility_b;

import extension_visibility_c;

int shouldntWork(MyThing thing)
{
	return helper(thing);
}

// `MyThing`'s conformance to `IThing` is provided by `extension_visibility_b`,
// which is intentionally not imported here, so specialization of `helper`
// reports the focused non-conformance diagnostic.
// CHECK: type argument 'MyThing' does not conform to the required interface 'IThing'