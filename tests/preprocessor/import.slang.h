// Confirm that `#import` interacts with preprocessor as expected

// We add a guard to ensure that this file isn't imported more than once
#ifdef BAR
#error File imported more than one!
#endif

// Here we use a macro from the parent file
FOO f( FOO y ) { return y; }

// Here is a macro that flows from child to parent
#define BAR float
