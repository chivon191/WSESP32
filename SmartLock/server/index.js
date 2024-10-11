const express = require('express');
const bodyParser = require('body-parser');
const WebSocket = require('ws');
const url = require('url'); // Thêm import này để sử dụng url.parse

const app = express();
const port = 3000;
logged = "false";

// Middleware để xử lý dữ liệu từ form
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.json());

// Giả lập tài khoản đăng nhập
const user = "admin";
const pass = "admin";

let clients = {
  control: [],
  esp32: null,
};

// Route để phục vụ trang login
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/HTML/login.html');
});

app.get('/login', (req, res) => {
  res.sendFile(__dirname + '/HTML/login.html');
});

// Xử lý đăng nhập
app.post('/login', (req, res) => {
  const username = req.body.username;
  const password = req.body.password;
  if (username == user && password == pass) {
    logged = "true";
    res.redirect("/control")
  }
    else {
      res.send(`
        <script>
          alert('Tên đăng nhập hoặc mật khẩu không đúng!');
          window.location.href = '/login'; // Điều hướng về trang đăng nhập
        </script>
      `);
    }
});

// Route điều khiển ESP32 (sau khi đã đăng nhập thành công)
app.get('/control', (req, res) => {
  if(logged == 'true') {
    res.sendFile(__dirname + '/HTML/control.html'); // Trang HTML để điều khiển ESP32 sau khi login
  }
  else res.sendFile(__dirname + '/HTML/login.html');
});

// Khởi động WebSocket server để giao tiếp với ESP32
const wss = new WebSocket.Server({ port: 8080 });

wss.on('connection', (ws, req) => {
  const query = url.parse(req.url, true).query; // Phân tích URL để lấy query
  const clientType = query.type;

  console.log(`Client đã kết nối: ${clientType}`);

  // Lưu trữ kết nối client
  if (clientType === 'control') {
    clients.control.push(ws); // Lưu trữ client control
    console.log("Lưu trữ client control");
  } else if (clientType === 'esp32') {
    clients.esp32 = ws; // Lưu trữ client esp32
    console.log("Lưu trữ client esp32");
  }

  // Nhận dữ liệu từ client
  ws.on('message', message => {
    console.log(`Dữ liệu từ client (${clientType}): ${message}`);
    
    // Nếu client control gửi "open", gửi tin nhắn đến client esp32
    if (clientType == 'control' && message == 'open') {
      clients.control.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {  // Kiểm tra xem client có mở không
          client.send('open');
          console.log('Đã gửi "open" đến client control');
        }
      });
        if (clients.esp32) {
            clients.esp32.send('open'); // Gửi "open" đến client esp32
            console.log('Đã gửi "open" đến ESP32');
        } else {
            console.log("Không có ESP32 nào đang kết nối.");
        }
    }

    if (clientType == 'esp32' && message == 'close') {
      clients.control.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {  // Kiểm tra xem client có mở không
          client.send('close');
          console.log('Đã gửi "close" đến client control');
        }
      });
    }
  });

  // Gửi phản hồi ban đầu khi client kết nối
  ws.send(`Xin chào ${clientType}!`);

  // Xử lý ngắt kết nối
  ws.on('close', () => {
    console.log(`${clientType} đã ngắt kết nối.`);
    logged = "false";
  });
});

// Khởi động HTTP server
app.listen(port, () => {
  console.log(`HTTP server đang chạy tại http://192.168.1.6:${port}`);
});
