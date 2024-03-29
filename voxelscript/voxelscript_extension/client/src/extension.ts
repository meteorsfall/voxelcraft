'use strict';
import * as path from 'path';
import {workspace} from 'vscode';
import {LanguageClient, LanguageClientOptions, ServerOptions, Executable, ExecutableOptions} from 'vscode-languageclient';

function activate(context) {
    // The debug options for the server
    let debugArgs = ["--nolazy", "--debug=6004"];
    // Server Executable
    let serverExecutable: Executable = {
        command: "voxells",
        args: [],
        options: {},
    };
    // If the extension is launch in debug mode the debug server options are use
    // Otherwise the run options are used
    let serverOptions: ServerOptions = {
        run: serverExecutable,
        debug: { ...serverExecutable, args: debugArgs },
    };
    // Options to control the language client
    let clientOptions: LanguageClientOptions = {
        // Register the server for plain text documents
        documentSelector: ['voxelscript'],
        synchronize: {
            // Notify the server about file changes to '.clientrc files contain in the workspace
            fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
        }
    };
    // Create the language client and start the client.
    let disposable = new LanguageClient('Language Server Example', serverOptions, clientOptions).start();
    // Push the disposable to the context's subscriptions so that the
    // client can be deactivated on extension deactivation
    context.subscriptions.push(disposable);
}

export {activate};
