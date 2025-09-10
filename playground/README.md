# Development Instructions

## Setup

Clone the [Slang Playground](https://github.com/shader-slang/slang-playground):

```bash
git clone https://github.com/shader-slang/slang-playground.git
```

### Prerequisites

* Install [Node.js](https://nodejs.org/en/download/)

## Iterate

First you need to get certain prerequisite files to run the project.

* Fork this repository
* Manually run the `Build Dependencies` workflow from the Actions tab of your fork
* Download the artifacts from the workflow run

This should produce the following files:

* `slang-wasm.js`
* `slang-wasm.d.ts`
* `slang-wasm.wasm.gz`
* `spirv-tools.js`
* `spirv-tools.d.ts`
* `spirv-tools.wasm`

Move them into the `src` directory of the slang-playground.

Run `npm install` to install dependencies.

Run `npm run build-engine` to build library files. You will need to do this any time they are updated.

Run `npm run dev` to start the development server. The command will show you the URL where you can access the playground.

Run `npm run build` to build the project. It will create a `dist` directory with the build artifacts. The website should be hostable on any static file server from this directory.

When updating CSS or some transitively included files, the browser may use the out of date file stored in the cache. To prevent this, you can hold Shift key and click Refresh to force the browser to reload all files.

## Locally building dependencies

### Prerequisites

* Install [Docker](https://www.docker.com/get-started/)
* Install [Github CLI](https://cli.github.com/)
* Install [Github Act](https://github.com/nektos/gh-act) as an extension using `gh extension install https://github.com/nektos/gh-act`

### Building

To get the dependencies locally, you can run the following command from the root of the repository:

```bash
gh act -P ubuntu-latest=catthehacker/ubuntu:full-latest -W '.github/workflows/build-dependencies.yml' --artifact-server-path ./out
```

The files will then be available at `out/1/artifact/artifact.zip`.

### Building for deployment

* Run `gh act -P ubuntu-latest=catthehacker/ubuntu:full-latest -j 'build' --artifact-server-path ./out`
* This will create a file at `out/1/artifact/artifact.zip`
* Extracting the zip file will provide a directory from which you can host the website with any web server
  * As an example, you can use vscode's live server extension to host the website by opening `index.html`
