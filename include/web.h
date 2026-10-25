#pragma once
#include <Arduino.h>
    
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Cấu hình</title>
  <style>
    body { font-family: sans-serif; background: #FEFAF3; color: #7C0E44; text-align: center; padding: 20px; margin: 0; }
    .box { max-width: 340px; margin: auto; background: #FFCED8; padding: 20px; border-radius: 12px; }
    select, input, button { width: 100%; padding: 12px; margin: 8px 0; border-radius: 6px; border: 1px solid #FFF4DE; background: #fff4de; color: #7C0E44; box-sizing: border-box; }
    button { background: #FAACBF; border: none; font-weight: bold; cursor: pointer; }
    .rescan-btn { background: #F6E2E9; font-size: 13px; padding: 8px; margin-top: 0; }
    label { display: block; text-align: left; font-size: 13px; color: #7C0E44; margin-top: 8px; }
  </style>
</head>
<body>
  <div class="box">
    <h3>Cấu hình</h3>
    <select id="wifiList" onchange="selectWiFi()">
      <option value="">Đang quét mạng...</option>
    </select>
    <button class="rescan-btn" onclick="scanWiFi()">Quét lại WiFi</button>

    <input type="text" id="ssid" placeholder="Tên WiFi (SSID)">
    <input type="password" id="pass" placeholder="Mật khẩu">

    <label for="tz">Múi giờ:</label>
    <select id="tz">
      <option value="-12">UTC-12:00</option>
      <option value="-11">UTC-11:00</option>
      <option value="-10">UTC-10:00 (Hawaii)</option>
      <option value="-9">UTC-09:00 (Alaska)</option>
      <option value="-8">UTC-08:00 (Los Angeles, Vancouver)</option>
      <option value="-7">UTC-07:00 (Denver, Calgary)</option>
      <option value="-6">UTC-06:00 (Chicago, Mexico)</option>
      <option value="-5">UTC-05:00 (New York, Toronto)</option>
      <option value="-4">UTC-04:00 (Santiago)</option>
      <option value="-3">UTC-03:00 (Brazil, Argentina)</option>
      <option value="0">UTC+00:00 (London, Lisbon)</option>
      <option value="1">UTC+01:00 (Paris, Berlin, Rome)</option>
      <option value="2">UTC+02:00 (Cairo, Helsinki)</option>
      <option value="3">UTC+03:00 (Moscow, Dubai)</option>
      <option value="5.5">UTC+05:30 (Ấn Độ)</option>
      <option value="7" selected>UTC+07:00 (Việt Nam, Bangkok)</option>
      <option value="8">UTC+08:00 (Đài Loan, Singapore, Bắc Kinh)</option>
      <option value="9">UTC+09:00 (Nhật Bản, Hàn Quốc)</option>
      <option value="9.5">UTC+09:30 (Adelaide)</option>
      <option value="10">UTC+10:00 (Sydney, Melbourne)</option>
      <option value="11">UTC+11:00</option>
      <option value="12">UTC+12:00 (New Zealand)</option>
    </select>

    <button onclick="saveConfig()">Lưu & Kết nối</button>
    
    <p id="status" style="font-size: 13px; color: #7C0E44;"></p>
  </div>

  <script>
    window.onload = scanWiFi;

    function scanWiFi() {
      const list = document.getElementById("wifiList");
      const status = document.getElementById("status");
      list.innerHTML = '<option value="">Đang quét mạng...</option>';
      status.innerText = "Đang quét sóng WiFi xung quanh...";

      fetch('/api/scan')
        .then(res => res.json())
        .then(networks => {
          list.innerHTML = '<option value="">Chọn WiFi</option>';
          if (networks.length === 0) {
            status.innerText = "Không tìm thấy mạng.";
            return;
          }
          networks.forEach(net => {
            const opt = document.createElement("option");
            opt.value = net.ssid;
            opt.innerText = `${net.ssid} (${net.rssi} dBm)`;
            list.appendChild(opt);
          });
          status.innerText = `Đã tìm thấy ${networks.length} mạng WiFi.`;
        })
        .catch(() => {
          status.innerText = "Lỗi quét WiFi!";
        });
    }

    function selectWiFi() {
      const list = document.getElementById("wifiList");
      if (list.value) {
        document.getElementById("ssid").value = list.value;
      }
    }

    function saveConfig() {
      const ssid = document.getElementById("ssid").value;
      const pass = document.getElementById("pass").value;
      const tz = document.getElementById("tz").value;
      if (!ssid) return alert("Vui lòng chọn hoặc nhập SSID!");

      fetch('/api/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}&tz=${encodeURIComponent(tz)}`
      }).then(res => res.text()).then(msg => alert(msg));
    }
  </script>
</body>
</html>
)rawliteral";