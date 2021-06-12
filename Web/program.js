const { time } = require('console');

const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);

const dgram = require('dgram');
const socket = require("socket.io")
const path = require('path');

/******************************************************* */

const server = dgram.createSocket('udp4');
const client = dgram.createSocket('udp4');

var ip = "192.168.0.110"
var port = 2807
var messageHeader = 'ff';

server.on('listening', function () {
  var address = server.address();
  console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

http.listen(80, () => console.log("Webserver has started..."))

server.on('message', function (message, remote) {
  k = Buffer.from(message);
  if (k[0] == 255) {

    //IDEK AT THIS POINT, PLEASE AVERT YOUR EYES FROM THIS HIDEOUS CODE
    glasses["active"] = 1;
    glasses["last time"] = Date.now()
    glasses["failed attempts"] = 0;
    glasses["settings"]["led brightness"] = k[1];
    glasses["settings"]["minimum voltage"] = k[2];
    glasses["settings"]["dev mode"] = k[3];
    glasses["status"]["current battery"] = k[4]
    //********************************************************************
    console.log(k)
    io.sockets.emit("changes", glasses)
  }
  else {
    console.log("Seems suss, boss...")
  }
});

server.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.bind(port, "192.168.0.98");

var glasses = {
  "active": 1,
  "ping time": 5000, //ms
  "last time": 0,
  "failed attempts": 0,
  "settings": { "led brightness": 0, "minimum voltage": 0 },
  "status": { "current battery": 0 }
}

/*structure of packet:
[header packet],
[array select], currently either 0 (setting) [brightness, minimum voltage, dev mode] or 1 (status)
[index], currently only processes for setting
[value] ""
*/

function send(message) {
  if (Date.now() - glasses["last time"] >= glasses["ping time"]) {
    glasses["active"] = 0;
    io.sockets.emit("changes", glasses)
  }

  if (glasses["active"] == 0 || message.length != 3) return //would you give cake to a dead man?

  console.log(message)

  message = Buffer.from(message.reduce(((acc, val) => {
    val = parseInt(val)
    if (val > 255) return

    acc += val.toString(16).padStart(2, "0")
    return acc

  }), messageHeader).toString(16), 'hex');

  //console.log(message)
  client.send(message, port, ip);
}

//****************************************************************************************** */

app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname, '/index.html'));
});

io.on("connection", socket => {
  socket.on("states", () => {
    if (Date.now() - glasses["last time"] >= glasses["ping time"]) {
     glasses["active"] = 0
    io.emit("states", glasses)
    }
  })

  socket.on("command", (msg) => send(msg))
});
