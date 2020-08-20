import {
    TextDocuments,
    createConnection,
    DiagnosticSeverity
} from 'vscode-languageserver';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync, appendFileSync } from 'fs';
import * as path from 'path';
import * as childProcess from 'child_process';

const compiler_path = path.join(__dirname, "voxelscript_compiler", "voxelscript_compiler.js");

function log(log : string) : void {
    appendFileSync(__dirname + '/log.txt', log + "\n");
}

function clear_src() {

}

let lock = false;

const PROJECT_PATH = path.join(__dirname, 'project');
const BUILD_PATH = path.join(__dirname, 'build');

function save_file(filename : string, filedata : string) {
    if (!existsSync(PROJECT_PATH)) {
        mkdirSync(PROJECT_PATH);
    }
    writeFileSync(PROJECT_PATH + '/' + filename, filedata);
}

function compile(module_name : string, send_diagnostics) {
    log("Compiling: " + module_name + " with " + compiler_path);
    const child_argv = [
        '--build-target=' + BUILD_PATH,
        '--source=' + PROJECT_PATH,
        '--module=' + module_name
    ];
    let cp = childProcess.fork(compiler_path, child_argv, {
        stdio: ['pipe', 'pipe', 'pipe', 'ipc']
    });

    let response = "";
    cp.stdout.on('data', function (data) {
        response += data;
    });
    
    cp.on('close', function (code) {
        let diagnostics = [];

        if (code != 0) {
            log("Failure!");
            log(response);
            log("++++++++++++++++++++++++++");

            let error = "Unknown Error";
            let lines = response.replace(/\r/g, ' ').split('\n').reverse();
            let valid = true;
            let lines_and_columns = [0, 0, 0, 0];
            for(let line of lines) {
                let error_regex = /^\s+\\--- (.*)$/g;
                let info_regex = /^>+.*\/[^\/]*:(\d+):(\d+) -> (\d+):(\d+)$/g;
                if (line.match(error_regex)) {
                    error = line.replace(error_regex, '$1');
                    log("Error: " + error);
                } else if (line.match(info_regex)) {
                    lines_and_columns = line.replace(info_regex, '$1 $2 $3 $4').split(' ').map(s => parseInt(s));
                    log("Found: " + lines_and_columns.join(" ~ "));
                    break;
                } else {
                    log("No Match: " + line);
                }
            }
            diagnostics.push({
                severity: DiagnosticSeverity.Error,
                range: {
                    start: {
                        line: lines_and_columns[0] - 1,
                        character: lines_and_columns[1] - 1,
                    },
                    end: {
                        line: lines_and_columns[2] - 1,
                        character: lines_and_columns[3] - 1,
                    }
                },
                message: error
            });
        }

        send_diagnostics(diagnostics);

        lock = false;
    });
}

// Create a connection for the server. The connection uses
// stdin / stdout for message passing
let connection = createConnection(process.stdin, process.stdout);

// Create a simple text document manager. The text document manager
// supports full document sync only
let documents = new TextDocuments();

// Make the text document manager listen on the connection
// for open, change and close text document events
documents.listen(connection);

// After the server has started the client sends an initilize request. The server receives
// in the passed params the rootPath of the workspace plus the client capabilites.
let workspaceRoot;
connection.onInitialize((params) => {
    workspaceRoot = params.rootPath;
    return {
        capabilities: {
            // Tell the client that the server works in FULL text document sync mode
            textDocumentSync: documents.syncKind
        }
    };
});

documents.onDidChangeContent((change) => {
    let doc = change.document.getText();
    let file_name = path.basename(change.document.uri);
    let module_name = file_name.split('.')[0];

    if (lock) {
        log("Compiler is locked!");
        return;
    }
    lock = true;
    save_file(file_name, doc);
    compile(module_name, diagnostics => {
        
        // Send the computed diagnostics to VS Code.
        connection.sendDiagnostics({ uri: change.document.uri, diagnostics: diagnostics });
    });
});

// Listen on the connection
connection.listen();
