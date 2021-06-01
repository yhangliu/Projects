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

// viewed at http://localhost:8080
app.get('/', function (req, res) {
    res.sendFile(path.join(__dirname + '/index.html'));
});

// request data at http://localhost:8080/data or just "/data"
app.get('/data', function (req, res) {
    let data;
    // because the file itself it updating realtime, sometime read err occur
    // when err occur simply try reading again until success.
    while (true) {
        try{
            data = JSON.parse(fs.readFileSync('datafile.json', 'utf-8'));
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
    // payload = (Math.round(req.body['intensity'])/10).toString()
    payload = req.body['intensity'];
    console.log(payload);
    client.send(payload, 1131, '10.0.0.145', (err) => {
        client.close();
    });
});

app.listen(1130);