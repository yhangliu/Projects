/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/
var express = require('express');
var app = express();
var path = require('path');
// var fs = require('fs');
var csv = require("csv-parse");
const fs = require("fs");
var datafile = [];

const SerialPort = require('serialport');
const Readline = SerialPort.parsers.Readline;
const port = new SerialPort('/dev/cu.SLAB_USBtoUART', {baudRate: 115200});
const parser = new Readline();

// delete old datafile.json if existed
fs.unlinkSync('datafile.json');
// set up for writing console data into file
port.pipe(parser);
parser.on('data', console.log);
parser.on('data', data=>{
  var parts = data.split(",");
  console.log("Time: ", parts[0]);
  console.log("Therm: ", parts[1]);
  console.log("Sonic: ", parts[2]);
  console.log("Light: ", parts[3]);

  let thermdata = {
    type: "TH",
    value: parts[1],
    time: parts[0]
  };
  datafile.push(thermdata);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err;
  }); 

  let sonicdata = {
    type: "UL",
    value: parts[2],
    time: parts[0]
  };
  datafile.push(sonicdata);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });

  let lightdata = {
    type: "IR",
    value: parts[3],
    time: parts[0]
  };
  datafile.push(lightdata);
    fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });
});

