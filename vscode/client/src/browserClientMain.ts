import { ExtensionContext, Uri, commands, window, workspace } from 'vscode';
import * as vscode from 'vscode';
import { LanguageClientOptions } from 'vscode-languageclient';

import { LanguageClient } from 'vscode-languageclient/browser';
import type { CompiledPlayground, CompileRequest, EntrypointsRequest, EntrypointsResult, Result, ServerInitializationOptions, Shader } from 'slang-playground-shared';
import { getSlangFilesWithContents, sharedActivate } from './sharedClient';

let client: LanguageClient;

// this method is called when vs code is activated
export async function activate(context: ExtensionContext) {
	const documentSelector = [{ language: 'slang' }];

	const initializationOptions: ServerInitializationOptions = {
		extensionUri: context.extensionUri.toString(true),
		workspaceUris: vscode.workspace.workspaceFolders.map(folder => folder.uri.fsPath),
		files: await getSlangFilesWithContents(),
	}

	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
		documentSelector,
		synchronize: {},
		initializationOptions,
	};

	client = createWorkerLanguageClient(context, clientOptions);

	await client.start();
	
	sharedActivate(context, {
		compileShader: function (parameter: CompileRequest): Promise<Result<Shader>> {
			return client.sendRequest('slang/compile', parameter);
		},
		compilePlayground: function (parameter: CompileRequest): Promise<Result<CompiledPlayground>> {
			return client.sendRequest('slang/compilePlayground', parameter);
		},
		entrypoints: function (parameter: EntrypointsRequest): Promise<EntrypointsResult> {
			return client.sendRequest('slang/entrypoints', parameter);
		}
	});
}

export async function deactivate(): Promise<void> {
	if (client !== undefined) {
		await client.stop();
	}
}

function createWorkerLanguageClient(context: ExtensionContext, clientOptions: LanguageClientOptions) {
	// Create a worker. The worker main file implements the language server.
	const serverMain = Uri.joinPath(context.extensionUri, 'server/dist/browserServerMain.js');
	const worker = new Worker(serverMain.toString(true));

	// create the language server client to communicate with the server running in the worker
	return new LanguageClient('slangLanguageServer', 'Slang Language Server', clientOptions, worker);
}
