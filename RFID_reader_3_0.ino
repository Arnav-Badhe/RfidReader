// Version idk
#include <HttpClient.h>
#include <b64.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "time.h"
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiAP.h>
#include <string.h>

#define WIFI_TIMEOUT_MS 30000

#define debug_pin 13
#define BUZZER_PIN 25
#define LED_BUILT 2
#define RST_PIN 17
#define SS_PIN 5

#define RESET_PIN 13

String SSIDbuff = "";
String Passbuff = "";

const char* ntpServer = "pool.ntp.org";
const int  gmtOffset_sec = 16200;
const int   daylightOffset_sec = 3600;
//const char* serverName = "https://api.schooldiary.me/rfid/CardSwipe";
const char* serverName = "http://192.168.1.65:8080/?Action=apicall";
//const char* serverName = "http://165.22.208.139:8080/?Action=apicall";

uint8_t dash[8]  = {B00000,B00000,B00000,B11111,B00000,B00000,B00000};
uint8_t cornner[8] = {B00000,B00000,B00000,B00111,B00100,B00100,B00100,B00100};
uint8_t cornnerR[8] = {B00000,B00000,B00000,B11100,B00100,B00100,B00100,B00100};
uint8_t standing[8] = {B00100,B00100,B00100,B00100,B00100,B00100,B00100,B00100};
uint8_t BottomL[8] = {B00100,B00100,B00100,B00111,B00000,B00000,B00000,B00000};
uint8_t BottomR[8] = {B00100,B00100,B00100,B11100,B00000,B00000,B00000,B00000};

String wifis = String("");
uint8_t diggre[8] =  {B00100,B01010,B00100,B00000,B00000,B00000,B00000,B00000};
int huh = 0;
int counter = 0;

//initialise vspi with default pins
//SCLK = 18, MISO = 19, MOSI = 23, SS = 5

AsyncWebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4); 
MFRC522 mfrc522(SS_PIN, RST_PIN);

const char* PARAM_INPUT_1 = "SSID";
const char* PARAM_INPUT_2 = "password"; 
hw_timer_t *My_timer = NULL;
// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wifi Networks</title>
    <style>
        body {
            color: white;
            margin: 0;
            background-color: rgb(0, 0, 0);
        }

        .container {
            height: inherit;
            width: inherit;
            display: flex;
            justify-content: center;
            
        }

        .vertical-strip {
            width: 100%;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .search-bar {
            width: 90%;
            margin-top: 20px;
            margin-bottom: 20px;
        }

        .search-bar input {
            width: 100%;
            background-color: rgb(38, 38, 38);
            height: 30px;
            border-radius: 10px;
            border: none;
            padding: 5px;
            color: white;
        }

        .search-bar input:focus {
            border: 2px solid rgb(6, 150, 208) !important;
            outline: none;
        }

        .wifi-wrapper {
            width: 100%;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .wifi {
            background-color: rgb(38, 38, 38);
            padding: 10px;
            width: 90%;
            margin-top: 2px;
            margin-bottom: 2px;
            border-radius: 10px;
        }

        .wifi p {
            margin: 0;
        }

        .hidden-input {
            max-height: 0px;
            transition: max-height 0.5s;
        }

        .hidden-input input, button, label {
            display: none;
        }

        .hidden-input.is-visible {
            max-height: 40px;
            transition: max-height 0.5s;
        }

        .hidden-input.is-visible input {
            display: inline-block;
            background-color: rgb(38, 38, 38);
            border: none;
            border-bottom: 2px solid rgb(6, 150, 208);
            color: white;
            height: 66.66%;
            margin-right: 20px;
        }

        .hidden-input.is-visible label {
            display: inline-block;
        }

        .hidden-input input:focus {
            border: 2px solid rgb(6, 150, 208) !important;
            outline: none;
        }

        .hidden-input.is-visible button {
            display: inline-block;
            background-color: rgb(6, 150, 208);
            color: white;
            height: 80%;
            padding: 8px 16px;
            border-radius: 18px;
            border: none;
        }

        .refresh {
        margin-top: 30px;
        background-color: #FFFFFF;
        border: 1px solid #222222;
        border-radius: 8px;
        box-sizing: border-box;
        color: #222222;
        cursor: pointer;
        display: inline-block;
        font-family: Circular,-apple-system,BlinkMacSystemFont,Roboto,"Helvetica Neue",sans-serif;
        font-size: 16px;
        font-weight: 600;
        line-height: 20px;
        outline: none;
        padding: 13px 23px;
        position: relative;
        text-align: center;
        text-decoration: none;
        touch-action: manipulation;
        transition: box-shadow .2s,-ms-transform .1s,-webkit-transform .1s,transform .1s;
        user-select: none;
        -webkit-user-select: none;
        width: auto;
        }

        .refresh:focus-visible {
        box-shadow: #222222 0 0 0 2px, rgba(255, 255, 255, 0.8) 0 0 0 4px;
        transition: box-shadow .2s;
        }

        .refresh:active {
        background-color: #F7F7F7;
        border-color: #000000;
        transform: scale(.96);
        }

        .refresh:disabled {
        border-color: #DDDDDD;
        color: #DDDDDD;
        cursor: not-allowed;
        opacity: 1;
        }

        .no-wifi {
            display: none;
        }

    </style>
</head>
<body>
    <div class="container">
        <div class="vertical-strip">
            <h2>WiFi List</h2>
            <div class="search-bar">
                <input type="text" oninput="fireSearch(event)" id="searchInput" placeholder="Search Networks">
            </div>
            <div class="wifi-wrapper" id="wifiWrapper">
                <!--Dynamically rendered JS-->
            </div>
            <div class="no-wifi" id="noWifi">
                <hr>
                <p>No WiFi networks found nearby.</p>
                <hr>
            </div>
         
        </div>
    </div>
</body>
<script>
    // Getting the Wrapper element
const wifiWrapper = document.getElementById('wifiWrapper')

// Making the XML HTTP Request
let wifiList = []
const xhr = new XMLHttpRequest()
xhr.onreadystatechange = function() {
    if(this.readyState == 4 && this.status == 200) {
        wifiList = JSON.parse(this.responseText)
    }
}
xhr.open("GET", "availablessids", false)
xhr.send();

// Populating the wrapper element with the list of WiFi Networks
function showWifi(wrapper, list) {
    if (list.length == 0) {
        document.getElementById('noWifi').style.display = 'block'
        return;
    } else {
        document.getElementById('noWifi').style.display = 'none'
    }

    list.forEach((element, index) => {
        let output = '<div class="wifi" onclick="openPasswordInput(event)" data-internal-id='+index+'>' + 
                        '<p data-internal-id='+index+'>'+element+'</p>' + 
                        '<div class="hidden-input" id='+index+'>' + 
                            '<label>Password: </label>' + 
                            '<input type="text" name="password" autocomplete="off"></input>' + 
                            '<button class="connect" onclick="sendConnectionRequest(event)">Connect</button>' + 
                        '</div>' + 
                    '</div>';
        wrapper.innerHTML += output;
    });
}

showWifi(wifiWrapper, wifiList);

function openPasswordInput(e) {
    const id = e.target.getAttribute('data-internal-id')
    if (id != null) {
        if (! document.getElementById(id).classList.contains('is-visible')) {
            Array.from(document.getElementsByClassName('wifi')).forEach((element) => {
                document.getElementById(element.getAttribute('data-internal-id')).classList.remove('is-visible')
            })
            document.getElementById(id).classList.add('is-visible')
        }  else {
            document.getElementById(id).classList.remove('is-visible')
           }
    }
}

function sendConnectionRequest(e) {
    const ssid = wifiList[e.target.parentNode.id]
    const password = e.target.previousElementSibling.value
    const setRequest = new XMLHttpRequest()
    setRequest.onreadystatechange = function() {
        if(this.readyState == 4 && this.status == 200) {
            window.location.replace('/set?ssid='+ssid+'&password='+password)
        }
    }
    setRequest.open("GET", "set?SSID="+ssid+"&password="+password)
    setRequest.send();
}

function fireSearch(e) {
    let searchString = e.target.value.toString().toLowerCase()
    wifiWrapper.innerHTML = ''
    wifiList.forEach((element, index) => {
        if (element.toLowerCase().slice(0, searchString.length) == searchString) {
            let output = '<div class="wifi" onclick="openPasswordInput(event)" data-internal-id='+index+'>' + 
                        '<p data-internal-id='+index+'>'+element+'</p>' + 
                        '<div class="hidden-input" id='+index+'>' + 
                            '<label>Password: </label>&nbsp' + 
                            '<input type="text" name="password" autocomplete="off"></input>' + 
                            '<button class="connect" onclick="sendConnectionRequest(event)">Connect</button>' + 
                        '</div>' + 
                    '</div>';
            wifiWrapper.innerHTML += output;
        }
    })
}

function refresh() {
    xhr.open("GET", "availablessids", false)
    xhr.send();
    wifiWrapper.innerHTML = ''
    document.getElementById('searchInput').value = ''
    showWifi(wifiWrapper, wifiList)
}
</script>
</html>
)rawliteral";

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

int offLen(char str[]){
    return ((20 - strlen(str))/2);
}
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
void getWifiList(){
  String result = String("[\"");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    if(n < 0){
      n = n * -1;
    }
    Serial.print(n);
    Serial.println(" networks found");
     for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          
          result.concat(WiFi.SSID(i));
          result.concat("\",\"");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          delay(10);
        }
        result = result.substring(0,(result.length() - 2));
        result.concat("]");      
        
        Serial.println(result);
  }
  wifis = result;
}
void writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  Serial.print("Writing to Eprom: ");
  Serial.print(strToWrite);
  Serial.println("");
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
  EEPROM.commit();
}
String readStringFromEEPROM(int addrOffset){
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}
void epromDump(){
  Serial.print("1>   ");
  for(int i = 0;i < 512;i++){
    int cal = EEPROM.read(i);
    String pl = String(cal);
    if(pl.length() == 1){
      Serial.print(cal);
      Serial.print("       ");
    }else if(pl.length() == 2){
      Serial.print(cal);
      Serial.print("      ");
    }else if(pl.length() == 3){
      Serial.print(cal);
      Serial.print("     ");
    }
    if(((i + 1) % 10) == 0){
      Serial.println("");
      int index = (((i + 1) / 10)+1);
      Serial.print(index);
      if(index < 10){
       Serial.print(">    ");     
      }else{
       Serial.print(">   ");    
      }
    }
  }
}
void connectToWIFI(int i){
  Serial.print("\nSSID:");
  Serial.print(SSIDbuff.c_str());
  Serial.print("\nPassword:");
  Serial.print(Passbuff.c_str());
  Serial.println("\nConnecting to WiFi");
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin(SSIDbuff.c_str(), Passbuff.c_str());

  unsigned long startAttemptTime = millis();
    int ios = 0;
  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){
    Serial.print(".");
    reconecting();
    delay(500);
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("");
    Serial.println("Connetion Failed lol");
  }else{
    Serial.println("");
    Serial.println("Connetion Success");
    Serial.print("Current Ip --> ");
    Serial.print(WiFi.localIP());
    Serial.println("");
  }
}
void showHomeScreeen() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.write(1);
  for(int i = 0;i<18;i++){
    lcd.setCursor(i+1,0);
    lcd.write(0);
  }
  lcd.setCursor(19,0);
  lcd.write(2);
  lcd.setCursor(0,1);
  lcd.write(3);
  lcd.setCursor(0,2);
  lcd.write(3);
  lcd.setCursor(19,1);
  lcd.write(3);
  lcd.setCursor(19,2);
  lcd.write(3);

  lcd.setCursor(0,3);
  lcd.write(4);
  for(int i = 0;i<18;i++){
    lcd.setCursor(i+1,3);
    lcd.write(0);
  }
  lcd.setCursor(19,3);
  lcd.write(5);
 struct tm timeinfo;
 
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  char time[20];
  strftime(time,20, "%d/%B %A", &timeinfo);
  // Alternate Time Formate: "%I:%M %d %B"
  // timeinfo.tm_mon+1

  lcd.setCursor(offLen(time)+1,1);
  lcd.print(time);
  
  strftime(time,20, "%I:%M", &timeinfo);
  lcd.setCursor(offLen(time),2);
  lcd.print(time);
  Serial.println("Done Updating screen");
}
void systemCheck(){
  Serial.println("Dumping ESP health report");
  
  Serial.print("CPU CLOCK Frq: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" Mhz");
  
  Serial.print("Free Heap (DRAM or IRAM): ");
  Serial.println(ESP.getFreeHeap());
  Serial.println(" bytes");

  Serial.print("Core Temp: ");
  Serial.print((temprature_sens_read() - 32) / 1.8);
  Serial.println(" C");
}
void showStartupScreeen() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Starting..");
  lcd.setCursor(0,1);
  lcd.print("Conecting WIFI..");
  lcd.setCursor(0,2);
  lcd.print("Name:");
  lcd.setCursor(5,2);
  lcd.print(SSIDbuff);
}
void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%I:%M %d %B");
}
void cheak(){
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Reconnecting");
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Reconnecting WIFI");
    connectToWIFI(0);
  }
}
void reconecting(){
  digitalWrite(BUZZER_PIN , HIGH);
  digitalWrite(LED_BUILT, HIGH);
  delay(500);  
  digitalWrite(BUZZER_PIN , LOW);
  digitalWrite(LED_BUILT, LOW);
  delay(500);
  digitalWrite(BUZZER_PIN , HIGH);
  digitalWrite(LED_BUILT, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN , LOW);
  digitalWrite(LED_BUILT, LOW);
  delay(1000);
  digitalWrite(BUZZER_PIN , HIGH);
  digitalWrite(LED_BUILT, HIGH);
  delay(500);  
  digitalWrite(BUZZER_PIN , LOW);
  digitalWrite(LED_BUILT, LOW);
  delay(500);
  digitalWrite(BUZZER_PIN , HIGH);
  digitalWrite(LED_BUILT, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN , LOW);
  digitalWrite(LED_BUILT, LOW);
  delay(1000);
}
void ok(){
  digitalWrite(BUZZER_PIN , HIGH);
  delay(700);  
  digitalWrite(BUZZER_PIN , LOW);
}
void ban(){
  digitalWrite(BUZZER_PIN , HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN , LOW);
  delay(500);
  digitalWrite(BUZZER_PIN , HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN , LOW);
  delay(500);
  digitalWrite(BUZZER_PIN , HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN , LOW);  
  delay(500);
  digitalWrite(BUZZER_PIN , HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN , LOW);
}
void unotherised(){
  digitalWrite(BUZZER_PIN , HIGH);
  delay(1500);
  digitalWrite(BUZZER_PIN , LOW);
  delay(1500);
  digitalWrite(BUZZER_PIN , HIGH);
  delay(1500);
  digitalWrite(BUZZER_PIN , LOW);
}
void IRAM_ATTR onTimer(){
  showHomeScreeen();
}
void setup() {
  EEPROM.begin(512);
  pinMode(BUZZER_PIN,OUTPUT);
  digitalWrite(BUZZER_PIN,HIGH);
  pinMode(LED_BUILT, OUTPUT);
  delay(1000);
  digitalWrite(BUZZER_PIN,LOW);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("System Initializing");
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Initiating protocol: SPI");
  delay(4);
  Serial.println("Initiating Device: MRFRC522");
  mfrc522.PCD_DumpVersionToSerial();    
  Serial.println("Initiating EEPROM with all addresses 0x0 to 0x200");
  
  Serial.println("EEPROM Initialized");

  Serial.println("Reading EEPROM For Wifi Configurations");
  
  Serial.println("Reading from EEPROM done Dumping data -> ");

  //epromDump();
  Serial.println("\n");
  
  SSIDbuff = readStringFromEEPROM(0);
  Passbuff = readStringFromEEPROM(100);

  Serial.print("SSID: ");
  Serial.println(SSIDbuff);
  Serial.print("Password: "); 
  Serial.println(Passbuff);
  
  Serial.println("Initiating Device: LCD");
  Serial.println("Initiating protocol: I2C");
  Serial.println("");
  
  lcd.init();                   
  lcd.backlight();
  systemCheck();
  Serial.println("All System Checked All devices online mabey IDK");
  
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 15000000, true);
  timerAlarmEnable(My_timer); //Just Enable
  
  showStartupScreeen();  
  
  for(int i = 0;i < 2;i++){
    connectToWIFI(0); 
    if(WiFi.status() == WL_CONNECTED){
      break;
    }
    lcd.clear();
    lcd.home();
    lcd.print("faild to connect");
    lcd.setCursor(0,1);
    lcd.print("Reconnecting");
    delay(1000);
  }
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_APSTA);
    getWifiList();
    digitalWrite(LED_BUILT, HIGH); 
    Serial.println("Failed to connect to WIFI Going in Maintenance mode");

    huh = 1;
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Fail to connect WIFI");

    lcd.setCursor(0,1);
    lcd.print("Starting Web Server");

    lcd.setCursor(0,2);
    lcd.print("RFID Reader,admin123");

    lcd.setCursor(0,3);
    lcd.print("IP: ");

    WiFi.softAP("RFID Reader", "admin123");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access Point IP address: ");
    Serial.println(myIP);
    Serial.println();

    lcd.setCursor(3,3);
    lcd.print(myIP);

  systemCheck();
     // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

    server.on("/availablessids", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "application/json", wifis.c_str());
  });
    // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server.on("/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String ssid;
    String pass;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if ((request->hasParam(PARAM_INPUT_1)) && (request->hasParam(PARAM_INPUT_2))) {
      ssid = request->getParam(PARAM_INPUT_1)->value();
      Serial.println(ssid);
      writeStringToEEPROM(0,ssid);
      pass = request->getParam(PARAM_INPUT_2)->value();
      Serial.println(pass);
      writeStringToEEPROM(100,pass);  
      delay(500);
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Changing WIFI TO:");
    
      lcd.setCursor(0,1);
      lcd.print(ssid);
    
      lcd.setCursor(0,2);
      lcd.print(pass); 
    
      delay(3000);
    
      lcd.clear();
      lcd.home();
      lcd.print("Please Restart");
    }
    request->send_P(200, "text/html", "worked please restart the device for the changes to take effect");
  });
  server.onNotFound(notFound);
  server.begin();
  }
   lcd.createChar(0, dash);
  lcd.createChar(1, cornner);  
  lcd.createChar(2, cornnerR); 
  lcd.createChar(3, standing); 
  lcd.createChar(4, BottomL); 
  lcd.createChar(5, BottomR); 
  lcd.createChar(6, diggre); 
  
  pinMode(debug_pin,INPUT_PULLUP);
  digitalWrite(23,LOW);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  Serial.print("Entring Main loop ");
  Serial.println(millis());
  if(huh != 1){
    lcd.clear();
   showHomeScreeen(); 
  }
}

void loop() {
  if(huh != 1){
    return;
  }
 if ( ! mfrc522.PICC_IsNewCardPresent()) {
    counter++;
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    counter++;
    return;
  }
  Serial.print("UID tag :");
  String recived = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     recived.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     recived.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
    Serial.println("");
    recived.trim();
    counter = 1;
    recived.trim();
    recived.replace(" ","");
    digitalWrite(BUZZER_PIN , HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN , LOW);

    lcd.clear();
    lcd.setCursor(5,1);
    lcd.print("Waiting..");
    lcd.setCursor(3,2);
    lcd.print("ID:");
    lcd.print(recived);
    int httpResponseCode = 0;
    
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      struct tm timeinfo;
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      
      if(!getLocalTime(&timeinfo)) {
        Serial.println(F(" Failed to obtain time"));
        return;
      }
      Serial.println(&timeinfo, "%d%m%Y%H%M%S");
      char timeStringBuff[50]; //50 chars should be enough
      strftime(timeStringBuff, sizeof(timeStringBuff),"%d%m%Y%H%M%S", &timeinfo);
      //print like "const char*"
      Serial.println(timeStringBuff);
      String timeString(timeStringBuff);
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);
      // Specify content-type header
      http.addHeader("Content-Type", "application/json");
      String reqest = "{\"transDateTime\":\""+timeString+"\",\"code\":\"7uxF5cv8RGpvgTCemxfmw\",\"rfid\":\""+recived+"\"}";
      //reqest.replace("$time",);
      //reqest.replace("$id",);
      Serial.println(reqest);
      httpResponseCode = http.POST(reqest);    
      
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String  payload = http.getString();
      
     payload.trim();
        int i1 = payload.indexOf(',');
        int i2 = payload.indexOf(',',i1+1);
        int i3 = payload.indexOf(',',i2+1);
        int i4 = payload.indexOf(',',i3+1);
        int i5 = payload.indexOf(',',i4+1);
        int i6 = payload.indexOf(',',i5+1);
        int i7 = payload.indexOf(',',i6+1);

        Serial.println(payload.substring(0, i1));
        Serial.println(payload.substring(i1+1, i2));
        Serial.println(payload.substring(i2+1, i3));
        Serial.println(payload.substring(i3+1, i4));
        Serial.println(payload.substring(i4+1, i5));
        Serial.println(payload.substring(i5+1, i6));
        Serial.println(payload.substring(i6+1, i7));
        Serial.println(payload.substring(i7+1, payload.length()));
        
      // Send HTTP POST request
      if(httpResponseCode == 200 && payload.substring(0, i1) == "0"){
         lcd.clear();
         char buff[20];

         lcd.setCursor(offLen(payload.substring(i2+1, i3)),0);
         lcd.print(payload.substring(i2+1, i3));
         
         lcd.setCursor(offLen(payload.substring(i3+1, i4)),1);
         lcd.print(payload.substring(i3+1, i4));
         
         lcd.setCursor(offLen(payload.substring(i4+1, i5)),2);
         lcd.print(payload.substring(i6+1, i7));                   
         
         
      }else{
         lcd.clear();
         lcd.setCursor(1,1);
         lcd.print("Server Side Error!");
         lcd.setCursor(1,2);
         lcd.print("ResponseCode:");
         lcd.setCursor(12,2);
         lcd.print(httpResponseCode);
      }
      // Free resources
      http.end();
      Serial.println("ended");
    }
    else {
      Serial.println("WiFi Disconnected");
       lcd.clear();
       lcd.setCursor(1,1);
       lcd.print("WiFi Disconnected!");
       lcd.setCursor(1,2);
       lcd.print("Reconecting");
       connectToWIFI(1);
    }
}  
