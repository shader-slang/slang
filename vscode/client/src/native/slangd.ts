import * as fs from 'fs';
import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';

export function getSlangdLocation(context: ExtensionContext): string {
    let slangdLoc = workspace.getConfiguration("slang").get("slangdLocation", "");
    if (slangdLoc === "") slangdLoc = context.asAbsolutePath(
        path.join('server', 'bin', process.platform + '-' + process.arch, 'slangd')
    );
    return fs.existsSync(slangdLoc) ? slangdLoc : 'slangd';
}

export function getBuiltinModuleCode(context: ExtensionContext, moduleName: string): Promise<string>  {
    const slangdLoc = getSlangdLocation(context);
    const { spawn } = require('child_process');
    return new Promise((resolve, reject) => {
        try {
            const process = spawn(slangdLoc, ['--print-builtin-module', moduleName]);
            const chunks: Buffer[] = [];
            let errorOutput = '';

            process.stdout.on('data', (data) => {
                chunks.push(data);
            });

            process.stderr.on('data', (data) => {
                errorOutput += data.toString();
            });

            process.on('close', (code) => {
                if (code === 0) {
                    resolve(Buffer.concat(chunks).toString());
                } else {
                    reject(new Error(`Process exited with code ${code}: ${errorOutput}`));
                }
            });

            process.on('error', (error) => {
                reject(error);
            });
        } catch (error) {
            console.error(`Error fetching builtin module code for ${moduleName}:`, error);
            resolve(`// Error fetching module ${moduleName}`);
        }
    });
}