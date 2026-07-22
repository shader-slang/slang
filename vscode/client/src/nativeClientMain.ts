import * as path from 'path';
import * as vscode from 'vscode';
import { ExtensionContext, workspace } from 'vscode';

import * as fs from 'fs';
import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';
import { Worker } from 'worker_threads';
import { CompiledPlayground, CompileRequest, EntrypointsRequest, EntrypointsResult, Result, ServerInitializationOptions, Shader, WorkerRequest } from 'slang-playground-shared';
import { getSlangdLocation } from './native/slangd';
import { SlangSynthesizedCodeProvider } from './native/synth_doc_provider';
import { getSlangFilesWithContents, sharedActivate } from './sharedClient';

let client: LanguageClient;
let worker: Worker;


function sendDidOpenTextDocument(document: vscode.TextDocument) {
	if (document.languageId !== 'slang') return;
	sendMessageToWorker({
		type: 'DidOpenTextDocument',
		textDocument: {
			uri: document.uri.toString(),
			text: document.getText(),
		}
	});
}


function sendDidChangeTextDocument(event: vscode.TextDocumentChangeEvent) {
	const document = event.document;
	if (document.languageId !== 'slang') return;
	sendMessageToWorker({
		type: 'DidChangeTextDocument',
		textDocument: {
			uri: document.uri.toString(),
		},
		contentChanges: event.contentChanges.map(change => ({
			range: {
				start: {
					character: change.range.start.character,
					line: change.range.start.line,
				},
				end: {
					character: change.range.end.character,
					line: change.range.end.line,
				},
			},
			text: change.text
		}))
	});
}

export async function activate(context: ExtensionContext) {
	const serverModule = getSlangdLocation(context);
	const serverOptions: ServerOptions = {
		run: { command: serverModule, transport: TransportKind.stdio },
		debug: {
			command: serverModule, transport: TransportKind.stdio,
			//	, args: ["--debug"]
		}
	};
	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
		// Register the server for plain text documents
		documentSelector: [{ scheme: 'file', language: 'slang' }],
	};

	// Create the language client and start the client.
	client = new LanguageClient(
		'slangLanguageServer',
		'Slang Language Server',
		serverOptions,
		clientOptions
	);
	// Start the client. This will also launch the server
	client.start();

	let synthCodeProvider = new SlangSynthesizedCodeProvider();
	synthCodeProvider.extensionContext = context;

	context.subscriptions.push(
		workspace.registerTextDocumentContentProvider('slang-synth', synthCodeProvider)
	);

	// Initialize language server options, including the implicit playground.slang file.
	const playgroundUri = vscode.Uri.file(path.join(context.extensionPath, 'external', 'slang-playground', 'engine', 'slang-compilation-engine', 'src', 'slang', 'playground.slang'));
	const playgroundDocument = await vscode.workspace.openTextDocument(playgroundUri);
	const initializationOptions: ServerInitializationOptions = {
		extensionUri: context.extensionUri.toString(true),
		workspaceUris: vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders.map(folder => folder.uri.fsPath) : [],
		files: [... await getSlangFilesWithContents(), {uri: playgroundUri.toString(), content: playgroundDocument.getText() }]
	}
	worker = new Worker(path.join(context.extensionPath, 'server', 'dist', 'nativeServerMain.js'), {
		workerData: initializationOptions
	});
	sendMessageToWorker({ type: 'Initialize', initializationOptions: initializationOptions });

	// Listen for document open/change events
	context.subscriptions.push(
		vscode.workspace.onDidOpenTextDocument(sendDidOpenTextDocument),
		vscode.workspace.onDidChangeTextDocument(sendDidChangeTextDocument)
	);

	sharedActivate(context, {
		compileShader: function (parameter: CompileRequest): Promise<Result<Shader>> {
			sendMessageToWorker({ type: 'slang/compile', ...parameter });
			return new Promise((resolve, reject) => {
				worker.once('message', (result: Result<Shader>) => {
					resolve(result);
				});
			});
		},
		compilePlayground: function (parameter: CompileRequest & { uri: string }): Promise<Result<CompiledPlayground>> {
			sendMessageToWorker({ type: 'slang/compilePlayground', ...parameter });
			return new Promise((resolve, reject) => {
				worker.once('message', (result: Result<CompiledPlayground>) => {
					resolve(result);
				});
			});
		},
		entrypoints: function (parameter: EntrypointsRequest): Promise<EntrypointsResult> {
			sendMessageToWorker({ type: 'slang/entrypoints', ...parameter });
			return new Promise((resolve, reject) => {
				worker.once('message', (result: EntrypointsResult) => {
					resolve(result);
				});
			});
		}
	});
}

export function sendMessageToWorker(message: WorkerRequest) {
	worker.postMessage(message);
}

export function deactivate(): Thenable<void> | undefined {
	if (worker) {
		worker.terminate();
	}
	if (!client) {
		return undefined;
	}
	client.stop();
}