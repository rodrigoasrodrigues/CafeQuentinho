#include "SoftwareSerial.h"
#define DEBUG true
#define TX 9
#define RX 8
#define BG 7
#define BR 6
#define LG 5
#define LR 4

String ssid ="YOUR_WIFI_NETWORK";
String password="YOUR_WIFI_PASSWORD";
SoftwareSerial esp(RX, TX);
String data = "";
String server = "YOUR_SERVER_IP_OR_HOSTNAME";
String uriOK = "/HAS_COFFEE_METHOD";
String uriBAD = "/NO_COFFEE_METHOD";

byte dat [5];
String temp ,hum;

boolean state = false;


void setup() {


  pinMode(LG, OUTPUT);
  pinMode(LR, OUTPUT);
  pinMode(BG, INPUT);
  pinMode(BR, INPUT);

  state = false;

  esp.begin(115200);
  Serial.begin(115200);

  Serial.println("setup");
  
  delay(1000);
  reset();
  delay(1000);
  
  Serial.println("Trying to connect to wifi");
  
  connectWifi();
  
  Serial.println("setup end");
  
}

void loop() {
  if(state){
    // CAFE QUENTINHO
    digitalWrite(LG,HIGH);
    digitalWrite(LR,LOW);
    if(digitalRead(BR)){
      CafeFriozinho();    
    }
  }
  else{
    // CAFE FRIOZINHO
    digitalWrite(LG,LOW);
    digitalWrite(LR,HIGH);
    if(digitalRead(BG)){
      CafeQuentinho();    
    }
  }

delay(100);
}


void CafeQuentinho(){
Serial.println("Cafe quentinho!");
  state = true;
  httppost();
}

void CafeFriozinho(){
  state = false; 
  httppost();
}

//reset the esp8266 module
void reset() {
  /*if(DEBUG){
  if(esp.available())
    Serial.println("ESP Available");
  else
    Serial.println("ESP Not Available");
  }*/
  sendData("AT+RST\r\n", 2000, DEBUG);
  delay(4000);
  if(esp.find("OK") ) Serial.println("Module Reset");
}

//connect to your wifi network
void connectWifi() {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  if(esp.find("OK")) {
    Serial.println("Connected!");
  }
  else {
    Serial.println("Cannot connect to wifi"); 
    connectWifi();
  }
}


void httppost () {
  String uri = "";
  if(state){
    uri = uriOK;
  }
  else{
    uri = uriBAD;
  }
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  if( esp.find("OK")) {
    Serial.println("TCP connection ready");
  } 
  delay(1000);
  String postRequest =
    "POST " + uri + " HTTP/1.0\r\n" +
    "Host: " + server + "\r\n" +
    //"Accept: *" + "/" + "*\r\n" +
    "Content-Length: " + data.length() + "\r\n" +
    //"Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + data;
  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.
  esp.print(sendCmd);
  esp.println(postRequest.length() );
  delay(500);
  if(esp.find(">")) { 
    Serial.println("Sending.."); 
    esp.print(postRequest);
    if( esp.find("SEND OK")) { 
      Serial.println("Packet sent");  
      while (esp.available()) {
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
      // close the connection
      esp.println("AT+CIPCLOSE");
    }
  }
}

String sendData(String command, const int timeout, boolean debug)
{
  // SENDS AT COMMANDS TO MODULE
  String response = "";
  esp.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp.available())
    {
      // The esp has data so display its output to the serial window
      char c = esp.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}
