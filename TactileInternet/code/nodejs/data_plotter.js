/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/
var express = require('express');
var app = express();
var path = require('path');
var fs = require('fs');

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
    console.log(data);
    res.send(data);  // Send array of data back to requestor
});

app.listen(8080);