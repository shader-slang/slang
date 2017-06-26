//TEST:SIMPLE:-profile vs_5_0

// Confirm that `#import` interacts with preprocessor as expected

// Here is a macro that flows from parent to child file
#define FOO float

// Here we import the child file
#import "import.slang.h"

// Here we use a macro that flows the other way (child->parent)
BAR g( FOO x ) { return f(x); }

// Here we confirm that importing the file again is a no-op
#import "import.slang.h"

void main()
{}
