
import * as path from "path";
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import minimist from 'minimist';

interface folder_information {
    filename:string,
    name:string,
    data:string
};
interface vspackage {
    package_name:string,
    package_path:string,
    options: any
  };
  interface option_type {
    build_target : string,
    source : string,
    desired_module : string | null,
    override : string | null
  };

function is_subdir(base:string, child:string):boolean {
    let relative = path.relative(base, child);
    const isSubdir = (relative != "") && !relative.startsWith('..') && !path.isAbsolute(relative);
    return isSubdir;
}

let getAllSubfiles = (baseFolder : string, folderList : folder_information[] = []) => {
  let folders:string[] = readdirSync(baseFolder).filter(file => statSync(path.join(baseFolder, file)).isDirectory());
  let files:string[] = readdirSync(baseFolder).filter(file => !statSync(path.join(baseFolder, file)).isDirectory());
  for (let file of files) {
    let filename = path.join(baseFolder, file);
    if (path.basename(filename).match(/^\w+\.vs$/g)) {
      let file_data = readFileSync(filename, 'utf8');
      folderList.push({
        filename: filename,
        name: path.basename(filename).split('.')[0],
        data: file_data,
      });
    }
  }
  folders.forEach(folder => {
    getAllSubfiles(path.join(baseFolder,folder), folderList);
  });
  return folderList;
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
          console.log(try_package_path + ' not found! Error Code: ' + err.code);
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


  function error_to_string(module_name : string, file_path : string, code : string, err : any) {
    const width = 80;
    const height = 6;
    
    let start_data;
    let end_data;
    let msg;
  
    if (err.missing_dependency) {
      start_data = err.missing_dependency.location.start;
      end_data = err.missing_dependency.location.end;
      msg = 'Missing Dependency: ' + err.missing_dependency.module_name;
    } else {
      start_data = err.location.start;
      end_data = err.location.end;
      msg = err.message;
    }
  
    let lines = code.split('\n');
  
    let start_line = start_data.line;
    let start_col = start_data.column;
    let end_line = end_data.line;
    let end_col = end_data.column;
  
    let left_col = start_col - width / 2;
    let right_col = end_col + width / 2;
    if (left_col < 1) left_col = 1;
  
    let top_line = start_line - height / 2;
    if (top_line < 1) top_line = 1;
    let bottom_line = end_line + height / 2;
    if (bottom_line > lines.length) bottom_line = lines.length;
  
    let max_num_digits = ("" + bottom_line).length;
  
    let output = '';
    
    // Print error module
    let band = "";
    for(let i = 0; i < max_num_digits; i++) {
      band += "~";
    }
  
    output += '\n';
    output += band + ' Error parsing module ' + module_name + ' ' + band + '\n';
  
    // Print error filename and row/col
    for(let i = 0; i < max_num_digits; i++) {
      output += ">";
    }
    output += " " + file_path + ":" + start_line + ":" + start_col + ' -> ' + end_line + ':' + end_col + "\n";
  
    for(let i = top_line; i <= bottom_line; i++) {
      let line = lines[i-1];
      // Print line #
      output += i;
      for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
        output += " ";
      }
      // Print line
      output += " | ";
      for(let c = left_col; c <= right_col && c - 1 < line.length; c++) {
        let ch = line[c-1];
        if (ch == '\r') {
          ch = ' ';
        }
        output += ch;
      }
      output += '\n';
      // Print ^ below error regions of the line
      if (start_line <= i && i <= end_line) {
        // Print line #
        output += i;
        for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
          output += " ";
        }
        // Print line with ^ under the marked columns
        output += " | ";
        for(let c = left_col; c <= right_col && c < line.length; c++) {
          let should_point = true;
          if (c < start_col && i == start_line) {
            should_point = false;
          }
          // Clip off the end_col itself
          if (end_col <= c && i == end_line) {
            should_point = false;
          }
          if (should_point) {
            output += "^";
          } else {
            output += " ";
          }
        }
        output += "\n";
      }
    }
    for(let j = 0; j < max_num_digits; j++) {
      output += " ";
    }
    output += " \\--- ";
    if (err.missing_dependency) {
      output += msg + " (" + err.message + ")";
      output += "\n";
    } else {
      output += "Expected ";
  
      // Format list of expected symbols
      let expected_arr : string[] = [];
      for(let e of err.expected) {
        if (e.type == 'end') {
          expected_arr.push("end of file");
        } else if (e.text) {
          expected_arr.push("\"" + e.text + "\"");
        } else {
          expected_arr.push(e.description);
        }
      }
      expected_arr = [...Array.from(new Set(expected_arr))];
      if (expected_arr.length > 1) {
        expected_arr[expected_arr.length - 1] = "or " + expected_arr[expected_arr.length - 1];
      }
      output += expected_arr.join(", ") + ", but ";
      if (err.found) {
        output += "character \"" + err.found + "\" found instead.\n";
      } else {
        output += "end of file was reached.\n";
      }
    }
  
    return output;
  }

function parse_args(args : any[]) : option_type {
  let argv = minimist(args.slice(2));

  let build_target : string = "";
  let source : string = "";
  let desired_module : string | null = null;
  let override : string | null = null;

  if (argv['build-target']) {
    build_target = path.resolve(argv['build-target']);
  } else {
    throw new Error("No --build-target given!");
  }

  if (argv['source']) {
    source = path.resolve(argv['source']);
  } else {
    throw new Error("No --source given!");
  }

  if (argv['module']) {
    desired_module = argv['module'];
  }

  if (argv['override']) {
    override = path.resolve(argv['override']);
  }

  return {
    build_target,
    source,
    desired_module,
    override
  };
}

export {is_subdir, getAllSubfiles, get_package_json, error_to_string, parse_args};
