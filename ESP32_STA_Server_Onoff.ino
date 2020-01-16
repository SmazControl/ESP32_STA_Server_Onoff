#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "phloenlom_2.4GHz"
#define STAPSK  "248248248"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

WebServer server(80);

const String postForms = R"=====(
<html>
  <head>
    <title>ESP8266 Web Server POST handling</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body>
    <h1>OnOff</h1><br>
    22 &nbsp; <input type="button" onclick="onoff(22,1)" value="ON">
       &nbsp; <input type="button" onclick="onoff(22,0)" value="OFF"><br><br>
    19 &nbsp; <input type="button" onclick="onoff(19,1)" value="ON">
       &nbsp; <input type="button" onclick="onoff(19,0)" value="OFF"><br><br>
    23 &nbsp; <input type="button" onclick="onoff(19,1)" value="ON">
       &nbsp; <input type="button" onclick="onoff(19,0)" value="OFF"><br><br>
    18 &nbsp; <input type="button" onclick="onoff(19,1)" value="ON">
       &nbsp; <input type="button" onclick="onoff(19,0)" value="OFF"><br><br>
    <h1>POST plain text to postplain</h1><br>
    <form method="post" enctype="text/plain" action="postplain/">
      <input type='text' name='hello' value='{"hello": "world", "trash": "test"}'><br>
      <input type="submit" value="Submit">
    </form>
    <h1>POST form data to /postform/</h1><br>
    <form method="post" enctype="application/x-www-form-urlencoded" action="/postform/">
      <input type="text" name="hello" value="world"><br>
      <input type="submit" value="Submit">
    </form>
    <script>
      function httpGetAsync(theUrl, callback)
      {
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.onreadystatechange = function() {
          if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
            callback(xmlHttp.responseText);
          } else {
            console.log(xmlHttp.responseText);
          }
        }
        xmlHttp.open("GET", theUrl, true); // true for asynchronous 
        xmlHttp.send(null);
      }
      function callbackPin(rst) {
        console.log(rst)
      }
      function onoff(pin,state) {
        httpGetAsync("http://"+window.location.host+"/onoff?pin"+pin.toString()+"="+state.toString(),callbackPin);
      }
    </script>
  </body>
</html>)=====";

void handleOnoff() {
  String message = "Number of args received:";
  message += server.args();  
  message += "\n";                  
  for (int i = 0; i < server.args(); i++) {
    message += "Arg nº" + (String)i + " –> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "\n";
    if(server.argName(i).substring(0,3)=="pin") {
      String pin = server.argName(i).substring(3);
      Serial.println("Pin "+pin+"="+String(server.arg(i)));
      int ipin = pin.toInt();
      if(ipin==22||ipin==19||ipin==23||ipin==18) {
        if(server.arg(i).toInt()==0) {
          digitalWrite(ipin,LOW);
        } else {
          digitalWrite(ipin,HIGH);
        }
      }
      server.send(200, "text/plain", "Pin "+pin+"="+String(server.arg(i)));  
    } else {
      server.send(200, "text/plain", "Pin not found");        
    }
  }
}


void handleRoot() {
  server.send(200, "text/html", postForms);
}

void handleScan() {
  server.send(200, "text/plain", WiFi.localIP().toString());  
}

void handlePlain() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
    Serial.println(server.arg("plain"));
    const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));
    // Parameters
    const char* username = root["username"]; // "Bret"
    const char* password = root["password"]; // "Sincere@april.biz"
    const char* title = root["title"]; // Thai language test
    // Output to serial monitor
    Serial.print("Username:");
    Serial.println(username);
    Serial.print("Password:"); 
    Serial.println(password);
    Serial.print("Title:"); 
    Serial.println(title);
  }
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(200, "text/plain", message);
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  pinMode(22, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(18, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/onoff", handleOnoff);

  server.on("/postplain/", handlePlain);

  server.on("/postform/", handleForm);

  server.on("/scan", handleScan);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
