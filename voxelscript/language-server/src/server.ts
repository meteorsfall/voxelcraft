import {
    TextDocuments,
    createConnection,
    DiagnosticSeverity
} from 'vscode-languageserver';
import { unlinkSync, createWriteStream, writeFileSync, readFileSync, readdirSync, lstatSync, existsSync, mkdirSync, appendFileSync, symlinkSync, rmdirSync, open } from 'fs';
import * as path from 'path';
import * as childProcess from 'child_process';
import uri_to_path = require('file-uri-to-path');

const ROOT_DIR = path.join(require('os').homedir(), ".voxells");
const DEBUG_LOG = false;

if (!existsSync(ROOT_DIR)) {
    mkdirSync(ROOT_DIR);
    log("Created root path: " + ROOT_DIR);
}

function is_subdir(base:string, child:string):boolean {
    let relative = path.relative(base, child);
    const isSubdir = (relative != "") && !relative.startsWith('..') && !path.isAbsolute(relative);
    return isSubdir;
}

function recursivelyDelete(filePath) {
    //check if directory or file
    let stats = lstatSync(filePath);
      
    //if file unlinkSync
    if (stats.isFile() || stats.isSymbolicLink()) {
     unlinkSync(filePath);
    }
    //if directory, readdir and call recursivelyDelete for each file
    else {
     let files = readdirSync(filePath);
     files.forEach((file) => {
      recursivelyDelete(path.join(filePath, file));
     });
     rmdirSync(filePath);
    }
   }

interface vspackage {
  package_name:string,
  package_path:string,
  options: any
};

function create_generic_diagnostic(msg : string, severity) {
    return {
        severity: severity,
        range: {
            start: {
                line: 0,
                character: 0,
            },
            end: {
                line: 0,
                character: 0,
            }
        },
        message: msg
    }
}

var access = createWriteStream(path.join(ROOT_DIR, 'stderr_log.txt'));
process.stderr.write = access.write.bind(access);

function log(log : string) : void {
    if (DEBUG_LOG) {
        appendFileSync(path.join(ROOT_DIR, 'log.txt'), log + "\n");
    }
}

log("TEST!");

function clear_src() {

}

let open_projects : any = {};

let lock = false;

const PROJECTS_PATH = path.join(ROOT_DIR, 'projects');
const BUILD_PATH = path.join(ROOT_DIR, 'build');

function get_module_name(uri : string) {
    
}

function compile(package_name : string, module_name : string) {
    
    let return_promise = new Promise((resolve, reject) => {
        let promise_returned = false;
        setTimeout(() => {
            if (promise_returned) {
                return;
            }
            log("Timeout reached!");
            let diagnostics = [];
            diagnostics.push(create_generic_diagnostic('Took too long to compile!', DiagnosticSeverity.Error));
            resolve(diagnostics);
            promise_returned = true;
        }, 15000);

        let build_path = path.join(BUILD_PATH, package_name);
        log("Build path: " + build_path + " " + existsSync(build_path));
        if (!existsSync(build_path)) {
            mkdirSync(build_path);
            log("Created build path: " + build_path);
        }

        log("Compiling Module: " + module_name);
        const child_argv = [
            '--cpp-file=' + path.join(build_path, 'main.cpp'),
            '--source=' + path.join(PROJECTS_PATH, package_name),
            '--override=' + path.join(PROJECTS_PATH, package_name, 'open_files'),
            '--module=' + module_name
        ];
        log("voxelc " + child_argv.join(" "));
        let cp = childProcess.spawn("voxelc", child_argv, {
            stdio: ['pipe', 'pipe', 'pipe', 'ipc']
        });
        log("Forked");
        let response = "";
        cp.stdout.on('data', function (data) {
            response += data;
        });
        
        let compiler_stderr = "";
        cp.stderr.on('data', function (data) {
            compiler_stderr += data;
        });
        
        cp.on('close', function (code) {
            let diagnostics = [];

            log("CODE: " + code);
            if (code == 1) {
                log("Failure!");
                log(response);
                log("++++++++++++++++++++++++++");
                if (compiler_stderr) {
                    log("COMPILER STDERR:");
                    log(compiler_stderr);
                    log("++++++++++++++++++++++++++");
                }

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
            } else if (code == 2) {
                // Grab the last line for the error
                let lines = response.split("\n");
                let accumulated_lines = [];
                while(lines.length > 0) {
                    let line = lines.pop();
                    accumulated_lines.push(line);
                    if (line.match(/^Error: /g)) {
                        break;
                    }
                }
                accumulated_lines.reverse();
                let error_message = accumulated_lines.join("\n");
                log(error_message);
                diagnostics.push(create_generic_diagnostic(error_message, DiagnosticSeverity.Error));
            }

            if (promise_returned) {
                log("Wanted to resolve diagnostics, but promise was already returned");
            } else {
                resolve(diagnostics);
                promise_returned = true;
            }
        });
    });

    return return_promise;
}

function get_package_json(pathname : string) : vspackage | null {
    let dir = pathname;
    while(true) {
      let try_package_path = path.join(dir, 'vspackage.json');
      let package_json_data;
      try {
        package_json_data = readFileSync(try_package_path, 'utf8');
      } catch (err) {
        // Here you get the error when the file was not found,
        // but you also get any other error
        if (err.code === 'ENOENT') {
        } else {
          log(try_package_path + ' not found! Error Code: ' + err.code);
        }
      }
  
      if (typeof(package_json_data) == 'string') {
        return {
          package_name: JSON.parse(package_json_data).name,
          package_path: dir,
          options: JSON.parse(package_json_data)
        };
      }
  
      // Move up to parent directory
      let new_dir = path.dirname(dir);
      if (new_dir == dir) {
        return null;
      }
      dir = new_dir;
    }
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
    if (existsSync(PROJECTS_PATH)) {
        recursivelyDelete(PROJECTS_PATH);
    }
    if (existsSync(BUILD_PATH)) {
        recursivelyDelete(BUILD_PATH);
    }
    mkdirSync(BUILD_PATH);
    mkdirSync(PROJECTS_PATH);
    log("Initialized Server");
    workspaceRoot = params.rootPath;
    return {
        capabilities: {
            // Tell the client that the server works in FULL text document sync mode
            textDocumentSync: documents.syncKind
        }
    };
});

function create_cloned_project_space(uri : string) : string | null {
    log(uri);
    let document_path = uri_to_path(uri);
    let package_data = get_package_json(path.dirname(document_path));
    if (package_data == null) {
        return null;
    }
    log(JSON.stringify(package_data));
    let project_path = path.join(PROJECTS_PATH, package_data.package_name);
    if (!existsSync(project_path)) {
        mkdirSync(project_path);
    }
    let open_files_path = path.join(project_path, 'open_files');
    if (!existsSync(open_files_path)) {
        mkdirSync(open_files_path);
    }
    let imported_files_path = path.join(project_path, 'imported_files');
    if (existsSync(imported_files_path)) {
        unlinkSync(imported_files_path);
    }
    symlinkSync(package_data.package_path, imported_files_path);
    writeFileSync(path.join(project_path, 'vspackage.json'), JSON.stringify(package_data.options));
    return package_data.package_name;
}

async function update_document(uri, data) {
    let project_name = create_cloned_project_space(uri);
    if (project_name == null) {
        let diagnostics = [];
        diagnostics.push(create_generic_diagnostic('Error: No vspackage.json found in any parent directory!', DiagnosticSeverity.Error));
        connection.sendDiagnostics({ uri: uri, diagnostics: diagnostics });
        return;
    }
    let file_name = path.basename(uri);
    let module_name = file_name.split('.')[0];

    let project_path = path.join(PROJECTS_PATH, project_name);
    writeFileSync(path.join(project_path, 'open_files', file_name), data);
    if (!(project_name in open_projects)) {
        open_projects[project_name] = {};
    }
    open_projects[project_name][module_name] = {
        uri: uri
    };
    log("COMPILING: " + module_name);
    let diagnostics = <any> await compile(project_name, module_name);
    connection.sendDiagnostics({ uri: uri, diagnostics: diagnostics });
    
    log("PROJECT!: " + JSON.stringify(open_projects[project_name]));
    for(let other_module_name in open_projects[project_name]) {
        if (other_module_name != module_name) {
            log("COMPILING OTHER: " + other_module_name);
            let diagnostics = <any> await compile(project_name, other_module_name);
            log("Send error about " + other_module_name + " to " + open_projects[project_name][other_module_name].uri);
            connection.sendDiagnostics({ uri: open_projects[project_name][other_module_name].uri, diagnostics: diagnostics });
        }
    }
    
    log("Unlocked!");
    lock = false;
}

const TYPING_DELAY = 1000;

let pending = [];
let pending_data = {};
setInterval(() => {
    if (lock) {
        return;
    }
    if (pending.length > 0 && Date.now() - pending_data[pending[pending.length - 1]].time > TYPING_DELAY) {
        let uri = pending.pop();
        let data = pending_data[uri];
        delete pending_data[uri];
        log("Grabbing uri from pending: " + uri);
        lock = true;
        update_document(data.uri, data.text);
    }
}, 50);

documents.onDidChangeContent(async (change) => {
    
    let docpath = uri_to_path(change.document.uri);
    if (is_subdir(PROJECTS_PATH, docpath)) {
        let diagnostics = [create_generic_diagnostic("Voxelscript file found inside of the internal projects directory", DiagnosticSeverity.Hint)];
        connection.sendDiagnostics({ uri: change.document.uri, diagnostics: diagnostics });
        return;
    }

    let uri = change.document.uri;
    if (uri in pending_data) {
        pending.splice(pending.indexOf(uri), 1);
    }
    pending.push(uri);
    pending_data[uri] = {
        uri: uri,
        text: change.document.getText(),
        time: Date.now()
    };
    log("Set pending: " + uri);
    log(JSON.stringify(pending));
});

// Listen on the connection
connection.listen();
