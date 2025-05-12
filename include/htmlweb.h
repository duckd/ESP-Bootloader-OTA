
#ifndef HTMLWEB_H
#define HTMLWEB_H

const char* htmlAdminTemplate = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>Admin</title></head>
<body>
  <h1>ESP32 Admin rozhraní</h1>
  <form id="otaForm">
    <input type="file" id="firmware" name="firmware" required>
    <button type="submit">Nahrát OTA</button>
  </form>
  <p>ChipID: %CHIP_ID%</p>
  <p>Verze firmwaru: %BUILD_VERSION%</p>
  <script>
    document.getElementById('otaForm').addEventListener('submit', function(e) {
      e.preventDefault();
      let file = document.getElementById('firmware').files[0];
      let formData = new FormData();
      formData.append('firmware', file);
      let xhr = new XMLHttpRequest();
      xhr.open('POST', '/update', true);
      xhr.send(formData);
    });
  </script>
</body>
</html>
)rawliteral";

#endif
