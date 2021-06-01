/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/
var express = require('express');
var app = express();
var path = require('path');
const fs = require("fs");
var datafile = [];

// const SerialPort = require('serialport');
// const Readline = SerialPort.parsers.Readline;
// const port = new SerialPort('/dev/cu.SLAB_USBtoUART', {baudRate: 115200});
// const parser = new Readline();

// delete old datafile.json if existed
try {
  if (fs.existsSync('datafile.json')) {
    fs.unlinkSync('datafile.json');
  }
} catch (err) {
  console.error(err);
}


const dgram = require('dgram');
const { Z_FIXED } = require('zlib');
const server = dgram.createSocket('udp4');
server.bind(1131);

// when message arrives, this gets triggered
server.on('message', (msg, rinfo) => {
  msg = msg.toString();
  console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);
  var parts = msg.split(",");
  console.log("time: ", parts[0])
  console.log("therm: ", parts[1]);
  console.log("x-axis: ", parts[2]);
  console.log("y-axis: ", parts[3]);
  console.log("z-axis: ", parts[4]);
  console.log("roll: ", parts[5]);
  console.log("pitch: ", parts[6]);

  let thermdata = {
    type: "THERM",
    value: parts[1],
    time: parts[0]
  };
  datafile.push(thermdata);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err;
  }); 

  let x_axis = {
    type: "XAXIS",
    value: parts[2],
    time: parts[0]
  };
  datafile.push(x_axis);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });

  let y_axis = {
    type: "YAXIS",
    value: parts[3],
    time: parts[0]
  };
  datafile.push(y_axis);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });

  let z_axis = {
    type: "ZAXIS",
    value: parts[4],
    time: parts[0]
  };
  datafile.push(z_axis);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });

  let roll = {
    type: "ROLL",
    value: parts[5],
    time: parts[0]
  };
  datafile.push(roll);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });

  let pitch = {
    type: "PITCH",
    value: parts[3],
    time: parts[0]
  };
  datafile.push(pitch);
  fs.writeFileSync("datafile.json", JSON.stringify(datafile), err => {  
    if (err) throw err; 
  });

});