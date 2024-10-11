const username = "admin"
const pass = "admin"

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 3000 });

wss.on('connection', ws => {
  console.log('Client đã kết nối.');

  // Nhận dữ liệu từ ESP32
  ws.on('message', message => {
    console.log(`Dữ liệu nhận từ ESP32: ${message}`);
  });

  // Gửi phản hồi về cho ESP32
  ws.send('Xin chào ESP32!');
});
