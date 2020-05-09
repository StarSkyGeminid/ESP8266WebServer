#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"

const char *ssid = "Isi SSID Wifi Anda";
const char *password = "Isi Passwd Wifi Anda";

ESP8266WebServer server ( 80 );

// Setting Pin DHT dan Type DHT
DHT dht(D2, DHT11);

//Led Default/Bawaan ESP8266 V2
const int led = D0;
  
// Rata-rata pembacaan sensor DHT
float avgH;
float avgT;
float avgHif;

unsigned long lmil = 0;
const char main_page[] = R"=====(
<html>
  <head>
    <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <title>ESP8266 DHT11 Monitor</title>
    <style>
      body { background-color: #D0D0D0; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body>
    <h4 id="time_hw" style="background-color: #008CBA;padding:5px;margin:2px;border-radius:5px;display: inline-block;color:#ffffff">HardWare Uptime : </h4><br>
    <div id="stattemp"></div>
    <div class="w3-light-grey">
      <div id="tempbar" class="w3-green w3-round-large w3-center" style="height:24px;width:0"></div>
    </div>
    <div id="stathum"></div>
    <div class="w3-light-grey">
      <div id="humbar" class="w3-blue w3-round-large w3-center" style="height:24px;width:0"></div>
    </div>
    <div id="statHif"></div>
    <div class="w3-light-grey">
      <div id="hifbar" class="w3-red w3-round-large w3-center" style="height:24px;width:0"></div>
    </div>
    <button id="btnclk" onclick="ledset(1)"  style="margin:5px;margin-left:10px">Led On</button>
    <button id="btnclk" onclick="ledset(0)"  style="margin:5px;margin-left:10px">Led Off</button>
    <p id="ledStatus" style="margin:10px">Led Status : OFF</p><br>
    
    <script>      
       Highcharts.setOptions({
          global:{
            useUTC:false,
            timezone:"Asia/Jakarta",
            timezoneOffset:"+7"
         }
       })
     
      function ledset(code) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ledStatus").innerHTML = xhttp.responseText;
          }
        }
        if (code == 1){
          xhttp.open("GET", "ledOn", true);
        }
        else if (code == 0){
          xhttp.open("GET", "ledOff", true);
        }
        else{
          xhttp.open("GET", "ledOff", true);
        }

        xhttp.send();
      };
        
      var graphHif = new Highcharts.Chart({
        chart:{ renderTo : 'statHif',
                height : "40%"
                },
        title: { text: 'Heat Temperature Index' },
        series: [{
          showInLegend: false,
          data: []
        }],
        plotOptions: {
          line: { animation: false,
            dataLabels: { enabled: true }
          },
          series: { color: '#FF0009' }
        },
        xAxis: { type: 'datetime',
          dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
          title: { text: 'Temperature (Celsius)' }
        },
        credits: { enabled: false }
      });
      
      var graphH = new Highcharts.Chart({
        chart:{ renderTo : 'stathum',
                height: "40%" },
        title: { text: 'DHT11 Humidity' },
        series: [{
          showInLegend: false,
          data: []
        }],
        plotOptions: {
          line: { animation: false,
            dataLabels: { enabled: true }
          },
          series: { color: '#059e8a' }
        },
        xAxis: { type: 'datetime',
          dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
          title: { text: 'Humidity (%)' }
        },
        credits: { enabled: false }
      });

      var graphT = new Highcharts.Chart({
        chart:{ renderTo : 'stattemp' ,
                height: "40%"
                },
        title: { text: 'DHT11 Temperature' },
        series: [{
          showInLegend: false,
          data: []
        }],
        plotOptions: {
          line: { animation: false,
            dataLabels: { enabled: true }
          },
          series: { color: '#10B000' }
        },
        xAxis: { type: 'datetime',
          dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
          title: { text: 'Temperature (Celsius)' }
        },
        credits: { enabled: false }
      });
      
      setInterval(function ( ) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var jsn = JSON.parse(this.responseText);
            var x = (new Date()).getTime();
                
            document.getElementById("time_hw").innerHTML = "HardWare UpTime : " + jsn.UpTime[0] + " : " + jsn.UpTime[1] + " : " + jsn.UpTime[2];
            document.getElementById("tempbar").innerHTML = jsn.Temp + "%";
            document.getElementById("tempbar").style.width = jsn.Temp * 2+ "%";
            document.getElementById("humbar").style.width = jsn.Hum +"%";
            document.getElementById("humbar").innerHTML = jsn.Hum + "%";
            document.getElementById("hifbar").innerHTML = jsn.Hif + "%";
            document.getElementById("hifbar").style.width = jsn.Hif * 2+ "%";


            var a = parseFloat(jsn.Temp);
            if (graphT.series[0].data.length > 40){
              graphT.series[0].addPoint([x, a], true, true, true);
            } else {
              graphT.series[0].addPoint([x, a], true, false, true);
            }
            
            b = parseFloat(jsn.Hum);
            if(graphH.series[0].data.length > 40) {
              graphH.series[0].addPoint([x, b], true, true, true);
            } else {
              graphH.series[0].addPoint([x, b], true, false, true);
            }
            
            var c = parseFloat(jsn.Hif);
            if (graphHif.series[0].data.length > 40){
              graphHif.series[0].addPoint([x, c], true, true, true);
            } else {
              graphHif.series[0].addPoint([x, c], true, false, true);
            }
          }
        };
        xhttp.open("GET", "/data", true);
        xhttp.send();
      }, 5000 ) ;
    </script>
  </body>
</html>
)=====";

void handleRoot() {
	server.send ( 200, "text/html", String(main_page) );
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}

void handleOn(){
  digitalWrite(led, 0);
  server.send ( 200, "text/plain", "Led Status : ON");
}

void handleOff(){
  digitalWrite(led, 1);
  server.send ( 200, "text/plain", "Led Status : OFF");
}

void avgData(){
  unsigned long mil = millis();

  // Pengambilan data setiap 2,1 detik. data yang dikirimkan adalah rata rata dari 2 sampel
  if (mil - lmil > 2100){
    float h1 = dht.readHumidity();
    float t1 = dht.readTemperature();
    float hif1 = dht.computeHeatIndex(t1, h1, false);
    if (mil - lmil > 4200){
      float h2 = dht.readHumidity();
      float t2 = dht.readTemperature();
      float hif2 = dht.computeHeatIndex(t2, h2, false);

      if (!isnan(h1) || !isnan(t1) || !isnan(h2) || !isnan(t2)){
        avgH = (h1 + h2) / 2;
        avgT = (t1 + t2) / 2;
        avgHif = (hif1 + hif2) / 2;
      }
      lmil = mil;
    }
  }
}

void handleJson(){
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String Jsondata = "";

   // Disini saya sengaja tidak menggunakan library JSON, karena saya berfikir itu sangat over power. 
  Jsondata += "{";
  Jsondata += "\"UpTime\":[\"";
  Jsondata += hr;
  Jsondata += "\",\"";
  Jsondata += min % 60;
  Jsondata += "\",\"";
  Jsondata += sec % 60;
  Jsondata += "\"]";
  
  if (!isnan(avgH) || !isnan(avgT)) {
    Jsondata += ",\"Hum\":\"";
    Jsondata += avgH;
    Jsondata += "\",\"Temp\":\"";
    Jsondata += avgT;
    Jsondata += "\", \"Hif\":\"";
    Jsondata += avgHif;
    Jsondata += "\"";
  }
  Jsondata += "}";
  
  server.send(200, "application/json", Jsondata);
}

void setup ( void ) {
	pinMode ( led, OUTPUT );
	digitalWrite ( led, 1 );
	Serial.begin ( 115200 );
  dht.begin();
  
	WiFi.begin ( ssid, password );
	Serial.print ( "\n\nConnecting To " );
  Serial.print (ssid);
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( "." );
	}

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( MDNS.begin ( "esp8266" ) ) {
		Serial.println ( "MDNS responder started" );
	}

	server.on ( "/", handleRoot );
  server.on ( "/ledOn", HTTP_GET, handleOn);
  server.on ( "/ledOff", HTTP_GET, handleOff);
  server.on ( "/data", HTTP_GET, handleJson);
	server.onNotFound ( handleNotFound );
	server.begin();
  Serial.println ( "HTTP server started" );
}

void loop ( void ) {
	server.handleClient();
  avgData();
}
