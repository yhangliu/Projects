/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/
var express = require('express');
var app = express();
var path = require('path');
var fs = require('fs');
const { Router } = require('express');
const bodyParser = require('body-parser');

const dgram = require('dgram');
// const message = Buffer.from('Some bytes');
// const client = dgram.createSocket('udp4');
// server.bind(1131);

var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/";
var db_data = [];


// viewed at http://localhost:1130
app.get('/', function (req, res) {
    res.sendFile(path.join(__dirname + '/index.html'));
});

// request data at http://localhost:8080/data or just "/data"
app.get('/data', function (req, res) {
    let data;
    // because the file itself it updating realtime, sometime read err occur
    // when err occur simply try reading again until success.
    while (true) {
        try {
            MongoClient.connect(url, function (err, db) {
                if (err) throw err;
                var dbo = db.db("mydb");
                dbo.collection("votes").find({}).toArray(function (err, result) {
                    if (err) throw err;
                    db_data = result;
                    //console.log(result);
                    db.close();
                });
            }); 
            data = db_data;
            //console.log(data);
            break;
        }
        catch(err) {
            // console.log(err.message);
            continue;
        }
    }
    // console.log(data);
    res.send(data);  // Send array of data back to requestor
});

app.use(bodyParser.urlencoded({extended: true}));
app.post("/postIntensity", function (req, res) {
    const client = dgram.createSocket('udp4');
    //console.log("at button");
    MongoClient.connect(url, function (err, db) {
        if (err) throw err;
        var dbo = db.db("mydb");
        dbo.collection("votes").remove(function (err, delOK) {
            if (err) throw err;
            if (delOK) console.log("Collection emptied");
            db.close();
        });
    }); 
});

app.listen(1130);