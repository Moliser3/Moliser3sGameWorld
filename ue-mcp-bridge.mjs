const WS_URL = 'ws://localhost:8001';

let ws;

function connect() {
  ws = new WebSocket(WS_URL);

  ws.onopen = () => {
    process.stderr.write('Connected to UE MCP server\n');
  };

  ws.onmessage = (event) => {
    process.stdout.write(event.data + '\n');
  };

  ws.onclose = () => {
    process.stderr.write('Disconnected from UE MCP server\n');
    process.exit(1);
  };

  ws.onerror = (err) => {
    process.stderr.write('WebSocket error: ' + err.message + '\n');
    process.exit(1);
  };
}

import readline from 'readline';
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stderr,
  terminal: false
});

rl.on('line', (line) => {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(line.trim());
  }
});

connect();
