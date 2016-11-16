var express = require('express');
var SerialPort = require('serialport');

var fs = require('fs');
var bodyParser = require('body-parser');
var app = express();

var port = new SerialPort('COM3', {
    baudRate: 9600,
    dataBits: 8,
    parity: 'none',
    stopBits: 1
});

var setBrightness = function(brightness) {
    var buffer = new Buffer([0, brightness]);
    //first value in a buffer is always the display mode so the AVR knows how to handle the data
    port.write(buffer);
}

port.on('open', function() {
    setBrightness(10);
});

port.on('data', function(data) {
    if(data.toString() == "r") {
        ready = true;
    }
    console.log('serial raw:' + data);
});

port.on('open', function(){
    console.log('succesfully opened port');
});

app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

app.post('/', function(req, res) {
    if(!ready) {
        // uC is not ready to receive new data yet, drop this frame.
        res.end();
        return;
    }
    if(req.body.map) {
        if(req.body.map.win_team != 'none') {
            //first value in a buffer is always the display mode so the AVR knows how to handle the data
            var buffer = new Buffer([2, (req.body.map.win_team == req.body.player.team_name) ? 1 : 0])
            port.write(buffer);
            res.end();
            return;
        }

        if(req.body.hero.stunned) {
            var buffer = new Buffer([3]);
            port.write(buffer);
            res.end();
            return;
        }

        var percentagePerLed = 100/20;
        var healthLeds = Math.round(req.body.hero.health_percent/percentagePerLed);
        var manaLeds = Math.round(req.body.hero.mana_percent/percentagePerLed);
        var ledAmount = healthLeds + manaLeds;
        if(prevLeds == ledAmount) {
            res.end();
            return;
        }
        var buffer = new Buffer([1, healthLeds, 0, manaLeds, 0]);
        port.write(buffer);
        prevLeds = ledAmount;
        ready = false;
        res.end();
    }
})

var ready = true;
var prevLeds = 0;
var server = app.listen(4000, '127.0.0.1');
console.log('listening to localhost:4000');