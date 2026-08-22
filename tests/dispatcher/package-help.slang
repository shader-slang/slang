// Verify that both package command names delegate to the installed slang-package tool.
//TEST:DISPATCHER(filecheck=PACKAGE): package help
//TEST:DISPATCHER(filecheck=PKG): pkg help
//TEST:DISPATCHER(filecheck=FLAG): package --help
//TEST:DISPATCHER(filecheck=PKG_FLAG): pkg --help
//TEST:DISPATCHER(filecheck=BAD): package definitely-not-a-command

// PACKAGE: result code = 0
// PACKAGE: Usage: slang-package <command>

// PKG: result code = 0
// PKG: Usage: slang-package <command>

// FLAG: result code = 0
// FLAG: Usage: slang-package <command>

// PKG_FLAG: result code = 0
// PKG_FLAG: Usage: slang-package <command>

// BAD: result code = 1
// BAD: slang-package: error: Invalid command or arguments
