
int CBDA = A0;
int doamdat;
int relay = 16; 
  
#include <DHT.h>
#include <DHT_U.h>        
#include <ESP8266WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
SimpleTimer timer;


//DHT config
#define DHTPIN 14   // what digital pin we're connected to Chân DATA nối với chân D5
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
byte degree[8] = { //phần khai báo mã nhị phân cho màng hình LCD,hiển thị chữ độ C.
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
};

// Wi-Fi Settings
const char* ssid = "So nha 56" ;//wireless network name (SSID)Le Xuan Giang's iPhone futureinternetlab3 
const char* password = "nhanhoa123456";//your Wi-Fi network password  123456789 rflabwifi
char auth[] = " ee2f1b80b74640608a4ed9ac41dcfed6";
char pass[] ="nhanhoa123456";
WiFiClient client;
//Tạo server 
WiFiServer server(80);
// ThingSpeak Settings
const int channelID = 791729; //
String writeAPIKey = "7M6BTCCKCXN2ORTL"; // write API key for your ThingSpeak Channel
const char* server1 = "api.thingspeak.com";
const int postingInterval =  2000; // post data every 2 seconds

void setup() {
        Serial.begin(115200);
        dht.begin();
        
        //Khai bao chan bat may bom
        pinMode(relay, OUTPUT);
       
        //Khai bao chan doc so do am dat
    
        
        //Kiem tra ket noi Wifi
        Serial.print("Connecting");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                delay(100);
              
        }
        Serial.println("\r\nWiFi connected");
        // Khoi dong server
        server.begin();
        Serial.println("Khoi dong Server");

       // In dia chi IP
       Serial.println(WiFi.localIP());

       //Cai dat LCD
                    // initialize the lcd 
       lcd.begin();
      
       // Print a message to the LCD.
       lcd.backlight();
       lcd.setCursor(0,0);
       lcd.print("T:");
       lcd.setCursor(9,0);
       lcd.print("H:");
       lcd.setCursor(0,1);
       lcd.print("Doamdat:");
       lcd.createChar(1, degree);

       //Cai dat Blynk
       Blynk.begin(auth, ssid, pass);

       // Setup a function to be called every second
      timer.setInterval(2000L, DOCCAMBIEN);
      timer.setInterval(10000L,tuoitudong);
      timer.setInterval(2000L,taoweb);
   
}

void DOCCAMBIEN() {
       // Đọc giá trị nhiệt độ C (mặc định)
        float t = dht.readTemperature();
        // Đọc giá trị độ ẩm
        float h = dht.readHumidity();        
        doamdat = giatridoam();
        //kiem tra qua trinh doc
        if (isnan(t) || isnan(h)) {
                Serial.println("Failed to read from DHT sensor!");
                return;
        }
        if (client.connect(server1, 80)) {
               //  Construct API request body
                String postStr = writeAPIKey;
                  postStr += "&field1=";
                  postStr += String(t);
                  postStr += "&field2=";
                  postStr += String(h);
                  postStr += "&field3=";
                  postStr += String(doamdat);
                  postStr += "\r\n\r\n";
              

                client.print("POST /update HTTP/1.1\n");
                client.print("Host: api.thingspeak.com\n");
                client.print("Connection: close\n");
                client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
                client.print("Content-Type: application/x-www-form-urlencoded\n");
                client.print("Content-Length: ");
                client.print(postStr.length());
                client.print("\n\n");
                client.print(postStr);
                client.print("\n\n");
                Serial.print("Temperature: ");
                Serial.print(t);
                Serial.print(" oC Humidity: ");
                Serial.print(h);
                Serial.print(" % Doamdat: ");
                Serial.print(doamdat);
                Serial.println("% send to Thingspeak");
        }
        client.stop();
        //hien thi len LCD
        Serial.println("Waiting…");
    lcd.setCursor(2,0);
    lcd.print(round(t));
    lcd.write(1);
    lcd.print("C");
    lcd.setCursor(11,0);
    lcd.print(round(h));
    lcd.print(" %");
  if(doamdat>=10){
    lcd.setCursor(9,1);
    lcd.print(doamdat);
    lcd.print("%");
  }
  else{
    lcd.setCursor(9,1);
    lcd.print("0");
    lcd.setCursor(10,1);
    lcd.print(doamdat);
    lcd.print("%");
    }
    
    Blynk.virtualWrite(V5, h);
    Blynk.virtualWrite(V6, t);
    Blynk.virtualWrite(V7, doamdat);
   if(30>doamdat){
   Blynk.notify("Cây thiếu nước.Yêu cầu tưới cây");
   } 
  }
 void loop(){
   
    Blynk.run();
    timer.run();
  
 }
 //Doc gia tri do am
int giatridoam(){
   int i = 0;
   int anaValue = 0;
 for (i = 0; i < 10; i++){
    anaValue += analogRead(CBDA); //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   // Đợi đọc giá trị ADC
  }
  anaValue = anaValue / (i);
  anaValue = map(anaValue, 1023, 0, 0, 100); //Ít nước:0%  ==> Nhiều nước 100%
  return anaValue;
}
void taoweb(){
  
  // Kiem tra neu co client ket noi
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Doi client gui data
  Serial.println("Co client moi");
  while(!client.available()){
  delay(1);
  }

  //  Doc do am
  float h = dht.readHumidity();
  // Doc nhiet do o do C
  float t = dht.readTemperature();
  //Doc do am dat
  doamdat=giatridoam();

  // Doc yeu cau tu client
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();//Ham doi doc xong het ki tu

  // Kiem tra yeu cau la gi
  if (req.indexOf("/on") != -1){
    digitalWrite(relay, 1);
  }
  else if (req.indexOf("/off") != -1) {
    digitalWrite(relay, 0);
  }

  client.flush();

  // Chuan bi thong tin phan hoi
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
         s += "<head>";
         s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
         s += "<script src=\"https://code.jquery.com/jquery-2.1.3.min.js\"></script>";
         s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">";
         s += "</head>";
         s += "<div class=\"container\">";
         s += "<h1>       THEO DOI NHIET DO , DO AM VA DIEU KHIEN MAY BOM NUOC    </h1>";
         s += "<div class=\"row\">";
         s += "<div class=\"col-md-2\"><input class=\"btn btn-block btn-lg btn-primary\" type=\"button\" value=\"On\" onclick=\"on()\"></div>";
         s += "<div class=\"col-md-2\"><input class=\"btn btn-block btn-lg btn-danger\" type=\"button\" value=\"Off\" onclick=\"off()\"></div>";
         s += "<div class=\"row\">";
         s += "<div class=\"col-md-3\">Nhiet do: </div><div class=\"col-md-3\">" + String(t) + "</div>";
         s += "<div class=\"col-md-3\">Do am: </div><div class=\"col-md-3\">" + String(h) + "</div>";
         s += "<div class=\"col-md-3\">Do am dat: </div><div class=\"col-md-3\">" + String(doamdat) + "</div>";
         s += "</div></div>";
         s += "<script>function on() {$.get(\"/on\");}</script>";
         s += "<script>function off() {$.get(\"/off\");}</script>";

  // Gui thong tin cho client
  client.print(s);
  delay(1);
  Serial.println("Ngat ket noi Client");

}
void tuoitudong(){

 doamdat=giatridoam();
Serial.print("Do am dat :");


    if(doamdat > 50){
    digitalWrite(relay, LOW);
     
  }else if(doamdat < 30){
    digitalWrite(relay,HIGH );
  
  }else{
     digitalWrite(relay, LOW);
    
    }

  }
