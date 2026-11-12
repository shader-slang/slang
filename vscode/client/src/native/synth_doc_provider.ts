import * as vscode from 'vscode';
import {getBuiltinModuleCode} from './slangd';

export class SlangSynthesizedCodeProvider implements vscode.TextDocumentContentProvider {
    private _onDidChange = new vscode.EventEmitter<vscode.Uri>();
    readonly onDidChange = this._onDidChange.event;
    public extensionContext: vscode.ExtensionContext;

    provideTextDocumentContent(uri: vscode.Uri, token: vscode.CancellationToken): string | Thenable<string> {
        const requestedCodeId = uri.authority;

        return getBuiltinModuleCode(this.extensionContext, requestedCodeId);
    }

    public update(uri: vscode.Uri) {
        this._onDidChange.fire(uri);
    }
}