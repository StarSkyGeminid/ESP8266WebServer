#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"

const char *ssid = "AP.Wireless";
const char *password = "1122345668";

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);
ESP8266WebServer server ( 80 );
DHT dht(D2, DHT11);

const int chipSelect = D4;
const int led = D0;
bool writedata = true;

float avgH;
float avgT;
float avgHif;

unsigned long lmil1 = 0;
unsigned long lmil = 0;
const char main_page[] = R"=====(
<html>
  <head>
    <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <title>ESP8266 DHT11 Monitor</title>
    <style>
      body { background-color: #515151; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body>
    <h4 id="time_hw" style="background-color: #008CBA;padding:5px;margin:2px;border-radius:5px;display: inline-block;color:#ffffff">HardWare Uptime : </h4><br>
    <div id="stattemp"></div>
    <div class="w3-dark-grey">
      <div id="tempbar" class="w3-green w3-round-large w3-center" style="height:24px;width:0"></div>
    </div>
    <div id="stathum"></div>
    <div class="w3-dark-grey">
      <div id="humbar" class="w3-blue w3-round-large w3-center" style="height:24px;width:0"></div>
    </div>
    <div id="statHif"></div>
    <div class="w3-dark-grey">
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
        
      var chartHif = new Highcharts.Chart({
        chart:{ renderTo : 'statHif' ,
                backgroundColor: '#676767',
                polar: true,
                type: 'line',
                height: "40%"
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
      
      var chartH = new Highcharts.Chart({
        chart:{ renderTo : 'stathum',
                height: "40%" ,
                backgroundColor: '#676767',
                polar: true,
                type: 'line'},
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

      var chartT = new Highcharts.Chart({
        chart:{ renderTo : 'stattemp' ,
                backgroundColor: '#676767',
                polar: true,
                type: 'line',
                height: "40%"},
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
            var x = (new Date()).getTime(),
                y = parseFloat(jsn.Hum);
            document.getElementById("time_hw").innerHTML = "HardWare UpTime : " + jsn.UpTime[0] + " : " + jsn.UpTime[1] + " : " + jsn.UpTime[2];
            document.getElementById("tempbar").innerHTML = jsn.Temp + "%";
            document.getElementById("tempbar").style.width = jsn.Temp * 2+ "%";
            document.getElementById("humbar").style.width = jsn.Hum +"%";
            document.getElementById("humbar").innerHTML = jsn.Hum + "%";
            document.getElementById("hifbar").innerHTML = jsn.Hif + "%";
            document.getElementById("hifbar").style.width = jsn.Hif * 2+ "%";
            if(chartH.series[0].data.length > 40) {
              chartH.series[0].addPoint([x, y], true, true, true);
            } else {
              chartH.series[0].addPoint([x, y], true, false, true);
            }

            var b = parseFloat(jsn.Hif);
            if (chartHif.series[0].data.length > 40){
              chartT.series[0].addPoint([x, b], true, true, true);
            } else {
              chartHif.series[0].addPoint([x, b], true, false, true);
            }

            var a = parseFloat(jsn.Temp);
            if (chartT.series[0].data.length > 40){
              chartT.series[0].addPoint([x, a], true, true, true);
            } else {
              chartT.series[0].addPoint([x, a], true, false, true);
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

void UpTime(){
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  char data_time[100];
  snprintf (data_time, 100, "HardWare Uptime : %02d : %02d : %02d",hr, min % 60, sec % 60);
  //hr, min % 60, sec % 60
  server.send (200, "text/plain", String(data_time));
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
  
  if (mil - lmil1 > 2100){
    float h1 = dht.readHumidity();
    float t1 = dht.readTemperature();
    float hif1 = dht.computeHeatIndex(t1, h1, false);
    if (mil - lmil1 > 4100){
      float h2 = dht.readHumidity();
      float t2 = dht.readTemperature();
      float hif2 = dht.computeHeatIndex(t2, h2, false);

      avgH = (h1 + h2) / 2;
      avgT = (t1 + t2) / 2;
      avgHif = (hif1 + hif2) / 2;
      lmil = mil;
    }
  }
}

void handleJson(){
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
 
  //hr, min % 60, sec % 60
  
  String Jsondata = "";
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hif = dht.computeHeatIndex(t, h, false);
    
  Jsondata += "{";
  Jsondata += "\"UpTime\":[\"";
  Jsondata += hr;
  Jsondata += "\",\"";
  Jsondata += min % 60;
  Jsondata += "\",\"";
  Jsondata += sec % 60;
  Jsondata += "\"]";
  
  if (!isnan(h) || !isnan(t)) {
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

void handleLog(){
  writedata = false;
  File myFile = SD.open("datalog.txt");
  String alllog;
  
  if (myFile) {
    while (myFile.available()) {
      alllog += myFile.read();
    }
    myFile.close();
  } else {
    alllog += "Error";
  }
  server.send (200, "text/html", alllog);
  writedata = true;
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
  server.on ( "/uptime", HTTP_GET, UpTime);
  server.on ( "/ledOn", HTTP_GET, handleOn);
  server.on ( "/ledOff", HTTP_GET, handleOff);
  server.on ( "/data", HTTP_GET, handleJson);
  server.on ( "/datalog", HTTP_GET, handleLog);
	server.onNotFound ( handleNotFound );
	server.begin();
  Serial.println ( "HTTP server started" );
  timeClient.begin();
  Serial.println ( "NTP server started" );
}

void loop ( void ) {
	server.handleClient();
  avgData();
  if (millis() - lmil >= 5000){
    if (writedata){
     timeClient.update();
     String dataString = "";

     dataString += timeClient.getFormattedTime();
     dataString += ",";
     dataString += avgH;
     dataString += ",";
     dataString += avgT;
     dataString += ",";
     dataString += avgHif;

    
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    }
    else {
      Serial.println("File Write Error");
      writedata = false;
    }
    lmil = millis();
    }
  }
}
