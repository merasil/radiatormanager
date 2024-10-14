#include <Arduino.h>

/* HTML Code */
const char* serverIndex =
"<form method='GET' action='/config'>"
    "<input type='submit' value='Set Config'>"
"</form>"
"<form method='GET' action='/configerase'>"
    "<input type='submit' value='Erase Config'>"
"</form>"
"<form method='GET' action='/firmware'>"
    "<input type='submit' value='Upload Firmware'>"
"</form>";

const char* serverConfig =
"<pre>"
"<form method='POST' action='/configset'>"
   "SSID:           <input type='text' name='ssid'><br>"
   "PSK:            <input type='text' name='psk'><br>"
   "Hostname:       <input type='text' name='hostname'><br>"
   "Static IP:      <input type='text' name='ip'><br>"
   "Subnet Mask:    <input type='text' name='subnet'><br>"
   "Gateway IP:     <input type='text' name='gw'><br>"
   "DNS IP:         <input type='text' name='dns'><br>"
   "MQTT Server:    <input type='text' name='mqttsrv'><br>"
   "MQTT User:      <input type='text' name='mqttuser'><br>"
   "MQTT Pass:      <input type='text' name='mqttpass'><br><br>"
    "<input type='submit' value='send'>"
"</form>"
"</pre>";

const char* serverConfigSend =
"Command send to Device."
"<form method='GET' action='/'>"
    "<input type='submit' value='Return'>"
"</form>";

const char* serverFirmwareUpload = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/firmwareupdate',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";