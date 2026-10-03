# Contributing

This project welcomes contributions and suggestions. Contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant the rights to use your contribution.

When you submit a pull request, a CLA bot will determine whether you need to sign a CLA. Simply follow the instructions provided.

## Cloning

To get this project run:

```sh
git clone --recurse-submodules https://github.com/shader-slang/slang-vscode-extension
```

## Getting dependencies

First you need to get certain prerequisite files to run the project.

* Fork this repository
* Manually run the `Build Dependencies` workflow from the Actions tab of your fork
* Download the artifacts from the workflow run

This should produce the following files:

* `slang-wasm.js`
* `slang-wasm.d.ts`
* `slang-wasm.worker.js`
* `slang-wasm.worker.d.ts`
* `slang-wasm.node.js`
* `slang-wasm.node.d.ts`
* `spirv-tools.js`
* `spirv-tools.d.ts`
* `spirv-tools.worker.js`
* `spirv-tools.worker.d.ts`
* `spirv-tools.node.js`
* `spirv-tools.node.d.ts`

Move them into the `media` directory.

## Running the Sample

* Run `npm install` in this folder. This installs all necessary npm modules for all builds and builds playground libraries
* Open VS Code on this folder.
* Press Ctrl+Shift+B to compile the client and server.
* Switch to the Debug viewlet.
* Select `Run Web Extension` or `Run Native Extension` from the drop down.
* Run the launch config.

You can also run and debug the extension in a browser

* `npm run chrome`
* Use browser dev tools to set breakpoints

## Updating slang-playground

Use `git submodule update --remote --merge` to update the slang-playground submodule.
