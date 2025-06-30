String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP8266 Weather Station</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 50px;
      background-color: #f0f0f0;
    }
    button {
      font-size: 20px;
      padding: 15px 30px;
      background-color: #0077cc;
      color: white;
      border: none;
      border-radius: 8px;
      cursor: pointer;
    }
    button:hover {
      background-color: #005fa3;
    }
  </style>
</head>
<body>
  <h1>Ian's Weather Station</h1>
  <p>Click below to manually sync forecast and time</p>
  <form action="/sync" method="POST">
    <button type="submit">🔄 Sync Data </button>
  </form>
</body>
</html>
)rawliteral";