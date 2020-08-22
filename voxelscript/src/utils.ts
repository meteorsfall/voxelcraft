
import * as path from "path";
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import minimist from 'minimist';
import { start } from "repl";

interface file_information {
  filepath:string,
  name:string,
  data:string
};
interface vspackage {
  package_name:string,
  package_path:string,
  options: any
};
interface option_type {
  build_target : string | null,
  source : string,
  desired_module : string | null,
  override : string | null
};

// Check if child directory string is a descendent of the base directory string
function is_subdir(base:string, child:string):boolean {
    let relative = path.relative(base, child);
    const isSubdir = (relative != "") && !relative.startsWith('..') && !path.isAbsolute(relative);
    return isSubdir;
}

// Get all .vs files in any subdirectories of a base folder
let getAllVoxelScriptSubfiles = (baseFolder : string, fileList : file_information[] = []) => {
  // Get folders and files
  let folders:string[] = readdirSync(baseFolder).filter(file => statSync(path.join(baseFolder, file)).isDirectory());
  let files:string[] = readdirSync(baseFolder).filter(file => !statSync(path.join(baseFolder, file)).isDirectory());

  // Loop through all files
  for (let file of files) {
    // Recreate filepath of file
    let file_path = path.join(baseFolder, file);

    // Only add if it's a \w+.vs file
    if (path.basename(file_path).match(/^\w+\.vs$/g)) {
      // Read the file and push to the folder list
      let file_data = readFileSync(file_path, 'utf8');
      fileList.push({
        filepath: file_path,
        name: path.basename(file_path).split('.')[0],
        data: file_data,
      });
    }
  }

  // Recurse into all subfolders, accumulating files in fileList
  folders.forEach(folder => {
    getAllVoxelScriptSubfiles(path.join(baseFolder,folder), fileList);
  });

  // Return the file list
  return fileList;
}

// Find a vspackage.json in any parent directory, specifically return the first one that is found
function get_package_json(pathname : string) : vspackage | null {
  // Start with the given pathname
  let dir = pathname;

  while(true) {
    // Read vspackage.json if it exists
    let try_package_path = path.join(dir, 'vspackage.json');
    let package_json_data = null;
    try {
      package_json_data = readFileSync(try_package_path, 'utf8');
    } catch (err) {
      // If we find an expected error, we should log it
      if (err.code != 'ENOENT') {
        console.log(try_package_path + ' not found! Error Code: ' + err.code);
      }
    }

    // If package_json_data exists, return it
    if (package_json_data) {
      return {
        package_name: JSON.parse(package_json_data).name,
        package_path: dir,
        options: JSON.parse(package_json_data)
      };
    } else {
      // Otherwise, Move up to the parent directory
      let new_dir = path.dirname(dir);
      if (new_dir == dir) {
        return null;
      }
      dir = new_dir;
    }
  }
}

// Print error messages. Example Error message is below:
//
// ~~~ Error parsing module Entity ~
// >>> /home/npip99/programming/voxelcraft/Entity.vs:6:24 -> 6:25
// 99  |     int entity_id();
// 100 |
// 101 |     bool exists() {
// 102 |         return this.en tity_id() != 0;
// 102 |                        ^
// 103 |     }
// 104 |
// 105 |     int get_entity_id() {
//     \--- Expected ".", "[","(", postfix operator, binary operator, "?", or ";", but character "t" found instead.
//
function error_to_string(module_name : string, file_path : string, code : string, err : any) {
  // Create function to go from 1-indexed line and columns, to 0-indexed javascript arrays
  let zero_index = (i : number) => i-1;

  // Padding of lines/columns around the ^'ed error message
  
  // How many lines above and below the ^ to print
  const lines_padding = 3;
  // How many columns left and right of the ^ to print
  const column_padding = 40;
  
  // (Temp variables to hold line & column data, see usage below)
  let start_data;
  let end_data;

  // If there's a missing dependency, then we generate the error message accordingly
  if (err.missing_dependency) {
    start_data = err.missing_dependency.location.start;
    end_data = err.missing_dependency.location.end;
  } else {
  // Otherwise, we just grab the standard one
    start_data = err.location.start;
    end_data = err.location.end;
    if (!start_data.line) {
      let lines = code.split("\n");
      let running_offset = 0;
      for(let i = 1; i <= lines.length; i++) {
        let line = lines[zero_index(i)];
        if (!start_data.line && running_offset + line.length + 1 >= start_data.offset) {
          start_data.line = i;
          start_data.column = start_data.offset - running_offset;
        }
        if (!end_data.line && running_offset + line.length + 1 >= end_data.offset) {
          end_data.line = i;
          end_data.column = end_data.offset - running_offset;
        }
        running_offset += line.length + 1;
      }
    }
  }

  // Get the lines of code in an array
  let lines = code.split('\n');

  // Get the start/end line and column for error messages
  // This will show where to underline in red
  // Note that these are 1-indexed
  let error_top_line = start_data.line;
  let error_left_col = start_data.column;
  let error_bottom_line = end_data.line;
  let error_right_col = end_data.column;

  // Make a window around the error location

  // Calculate the leftmost column and rightmost column to display in the error message window
  let left_col = Math.min(error_left_col, error_right_col) - column_padding;
  let right_col = Math.min(error_right_col, error_left_col) + column_padding;
  if (left_col < 1) left_col = 1;

  // Calculate the topmost line and bottommost line to display in the error message window
  let top_line = error_top_line - lines_padding;
  if (top_line < 1) top_line = 1;
  let bottom_line = error_bottom_line + lines_padding;
  if (bottom_line > lines.length) bottom_line = lines.length;

  // Calculate digits of the longest (largest) line number,
  // as that will be the minimum padding of the rest of the lines
  let max_num_digits = ("" + bottom_line).length;

  // Begin accumulating output
  let output = '';

  // Print the name of the module that had an error, with a tilde band around it
  // ~~~ Error parsing module Entity ~~~
  // ... | ...
  // 104 | ...
  // 105 | ...
  output += '\n';
  let tilde_band = "";
  for(let i = 0; i < max_num_digits; i++) {
    tilde_band += "~";
  }
  output += tilde_band + ' Error parsing module ' + module_name + ' ' + tilde_band;
  output += '\n';

  // Print error filename and row & column error locations
  // >>> /home/npip99/programming/voxelcraft/Entity.vs:6:24 -> 6:25
  // ...
  // 105 | ...
  for(let i = 0; i < max_num_digits; i++) {
    output += ">";
  }
  output += " " + file_path + ":" + error_top_line + ":" + error_left_col + ' -> ' + error_bottom_line + ':' + error_right_col + "\n";

  // Print each line individually
  // 101 |     bool exists() {
  for(let i = top_line; i <= bottom_line; i++) {
    // Grab the line itself
    let line = lines[zero_index(i)];

    // Print line #
    output += i;
    // Pad the line based on number of missing digits versus number of digits on the last line
    for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
      output += " ";
    }

    // Print a pipe to separate line # and the line itself
    output += " | ";

    // Print the line itself, from left column to right column
    for(let column_number = left_col; column_number <= right_col && zero_index(column_number) < line.length; column_number++) {
      // Get character
      let character = line[zero_index(column_number)];

      // Print character, while hiding \r
      if (character == '\r') {
        character = ' ';
      }
      output += character;
    }
    output += '\n';
    
    // Print ^ below any error regions of the line. E.g.
    //
    // 102 |         return this.en tity_id() != 0;
    // 102 |                        ^
    //
    // If i is a line where the error has occured,
    if (error_top_line <= i && i <= error_bottom_line) {
      // Print line # again, and again pad any missing digits
      output += i;
      for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
        output += " ";
      }

      // Print a line of spaces, but with ^ under any of the errored columns from the line above it
      output += " | ";
      for(let column_number = left_col; column_number <= right_col; column_number++) {
        if (zero_index(column_number) >= line.length) {
          break;
        }

        // Print a carrot if the column is an error column of that line
        if (i == error_top_line && i == error_bottom_line) {
          // If this is the only error line, the carrots print between the left and right error column
          output += error_left_col <= column_number && column_number <= error_right_col ? "^" : " ";
        } else if (i == error_top_line) {
          // If it's only the top line, print after the left column (inclusive)
          output += error_left_col <= column_number ? "^" : " ";
        } else if (i == error_bottom_line) {
          // If it's only the bottom line, print before the right column (inclusive)
          output += column_number <= error_right_col ? "^" : " ";
        } else {
          // Always print a carrot if it's on the interior lines of a multi-line error
          output += "^";
        }
      }
      output += "\n";
    }
  }

  // Print line that contains the error message
  // ...
  // 105 | 
  // 106 |     int get_entity_id() {
  //     \--- Expected ".", "[","(", postfix operator, binary operator, "?", or ";", but character "t" found instead.
  //
  for(let j = 0; j < max_num_digits; j++) {
    output += " ";
  }
  output += " \\--- ";
  if (err.missing_dependency) {
    // Print the missing dependency error message
    output += 'Missing Dependency: ' + err.missing_dependency.module_name + " (" + err.message + ")";
    output += "\n";;
  } else if (err.typescript_error) {
      // Print the missing dependency error message
      output += err.message;
      output += "\n";
  } else {
    output += "Expected ";

    // Create list of expected symbols
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
    
    // Remove duplicates
    expected_arr = [...Array.from(new Set(expected_arr))];

    // Put the "or" in "W, X, Y, or Z"
    if (expected_arr.length > 1) {
      expected_arr[expected_arr.length - 1] = "or " + expected_arr[expected_arr.length - 1];
    }

    // Generate rest of the error message
    // ".", "[","(", postfix operator, binary operator, "?", or ";", but character "t" was found instead.
    // ~~~~~
    // ".", "[","(", postfix operator, binary operator, "?", or ";", but end of file was found instead.
    output += expected_arr.join(", ") + ", but ";
    if (err.found) {
      output += "character \"" + err.found + "\" was found instead.\n";
    } else {
      output += "end of file was found instead.\n";
    }
  }

  // Return generated error message
  return output;
}

// Use minimist to parse arguments
function parse_args(args : any[]) : option_type {
  let argv = minimist(args.slice(2));

  let build_target : string | null = "";
  let source : string = "";
  let desired_module : string | null = null;
  let override : string | null = null;

  // Use path.resolve to get the absolute directories of each argument

  if (argv['build-target']) {
    build_target = path.resolve(argv['build-target']);
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

export {is_subdir, getAllVoxelScriptSubfiles, get_package_json, error_to_string, parse_args};
