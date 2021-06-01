const dgram = require('dgram');
const message = Buffer.from('0');
const client = dgram.createSocket('udp4');
client.send(message, 1130, '10.0.0.145', (err) => {
  client.close();
});