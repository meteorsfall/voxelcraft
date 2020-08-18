let peg = require("pegjs");
var Tracer = require('pegjs-backtrace');
let fs = require('fs');

let PEGJS_FILE = 'voxelscript.pegjs';
let VS_FILE = 'test.vs';

fs.readFile(PEGJS_FILE, 'utf8', function (err,data) {
  if (err) {
    return console.log(err);
  }

  let parser = peg.generate(data, {cache:true, trace:true});
  //parser = custom_parser;

  fs.readFile(VS_FILE, 'utf8', function (err,vs_data) {
    if (err) {
      return console.log(err);
    }

    let tracer = new Tracer(vs_data, {});
    try {
      let results = parser.parse(vs_data, {tracer:tracer});
      console.log(JSON.stringify(results, null, 4));
    } catch (err) {
      console.log(tracer.getBacktraceString());
      if (!err.hasOwnProperty('location')) throw(err);
      // Slice `text` with a little context before and after the error offset
      console.log('Error: ' + vs_data.slice(err.location.start.offset-10,
         err.location.end.offset+10).replace(/\r/g, '\\r'));
   }
  });
});
