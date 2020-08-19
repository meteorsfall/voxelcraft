import {
    TextDocuments,
    createConnection,
    DiagnosticSeverity
} from 'vscode-languageserver';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync, appendFileSync } from 'fs';

function log(log : string) : void {
    appendFileSync(__dirname + '/log.txt', log);
}

function clear_src() {

}

const PROJECT_DIR = 'project';

function save_file(filename, filedata) {
    let project_path = __dirname + '/' + PROJECT_DIR;
    if (!existsSync(project_path)) {
        mkdirSync(project_path);
    }
    writeFileSync(project_path + '/' + filename, filedata);
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
    let diagnostics = [];
    let doc = change.document.getText();
    log(change.document.uri);

    let loc = doc.indexOf('bad');

    if (loc >= 0) {
        diagnostics.push({
            severity: DiagnosticSeverity.Error,
            range: {
                start: {
                    line: 0,
                    character: 0,
                },
                end: {
                    line: 0,
                    character: 3,
                }
            },
            message: 'Bad String!!'
        });
    }
        
    // Send the computed diagnostics to VS Code.
    connection.sendDiagnostics({ uri: change.document.uri, diagnostics: diagnostics });
});
// Listen on the connection
connection.listen();
