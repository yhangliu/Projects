/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/
var express = require('express');
var app = express();
var path = require('path');
const fs = require("fs");
var datafile = [];

var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/mydb";
const { DH_CHECK_P_NOT_PRIME } = require('constants');



const dgram = require('dgram');
const { Z_FIXED } = require('zlib');
const server = dgram.createSocket('udp4');
server.bind(1131);




// when message arrives, this gets triggered

server.on('message', (msg, rinfo) => {
  msg = msg.toString();
  console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);
    var parts = msg.split(",");
    myobj = { temp: parts[0], ready: parts[1], time: Date() };
    console.log(myobj);

    MongoClient.connect(url, { useUnifiedTopology: true }, function (err, db) {
        if (err) throw err;
        var dbo = db.db("mydb");

        dbo.collection("votes").insertOne(myobj, function (err, res) {
            if (err) throw err;
            else db.close();
        });
    });

});