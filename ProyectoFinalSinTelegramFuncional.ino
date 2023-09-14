//Librerías
        
#include <Wire.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include "time.h"

//Pantalla Display en pixeles
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

//Pines
#define PIN_BOTON_1 35 
#define PIN_BOTON_2 34 
bool estadoBoton1 = false; 
bool estadoBoton2 = false; 
const int ledPin = 26; 
bool ledState = LOW; 

int horaDeRegado = 9;
int minutosDeRegado = 38;
int intervalo = 30000;
unsigned long TiempoUltimoCambio = 0;

//Máquina de Estados
#define PANTALLA_INICIAL 1 
#define LED_ESPERA 2   
#define LIMPIAR_1 3
#define CAMBIAR_HORA 4
#define LIMPIAR_2 5
#define CAMBIAR_TIEMPO 6
#define LIMPIAR_3 7


//Estado inicial de la máquina
int estado = 1;  
int estado2 = 1;
//Inicializar rtc
int gmt;
struct tm timeinfo;
ESP32Time rtc;

//Reloj
long unsigned int timestamp; 
const char *ntpServer = "south-america.pool.ntp.org";
long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;



//Constructores y variables globales
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Funciones
void pedir_lahora(void);
void setup_rtc_ntp(void);

//Variables
bool contadorLed = false;

//HTML

Preferences preferences;
DNSServer dnsServer;
AsyncWebServer server(80);

String ssid;
String password;
String Token_tel;
String Id_tel;

char ap_ssid[30] = "abc_mirko";
char ap_password[30] = "mirko1793";

char ap_token[50];

char ap_Id_tel[20];

bool is_setup_done = false;
bool valid_ssid_received = false;
bool valid_password_received = false;
bool wifi_timeout = false;

void setup() 
{
  
  Serial.begin(115200);
  u8g2.begin();
  pinMode(PIN_BOTON_1, INPUT_PULLUP);    
  pinMode(PIN_BOTON_2, INPUT_PULLUP);  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  Serial.println("Conectandose a Wi-Fi...");

  //Llamo a las funciones
  ap_wifisetup(); 
  setup_rtc_ntp();

  }

void loop() 
{
    char stringhoras[7];  
    char stringminutos[7];
    char stringtiempo[7];
    delay (1000);
    
  switch(estado)
  {
    //Pantalla que muestra hora y temperatura
    case PANTALLA_INICIAL: 
    {
    Serial.println("PANTALLA INICIAL"); 
    delay(2000);
    pedir_lahora();
    
    u8g2.clearBuffer();  
    u8g2.setFont(u8g_font_5x7); 

    u8g2.drawStr(0,30,"Hora : ");
    u8g2.drawStr(0,15,"Tiempo: ");
    sprintf (stringhoras, "%d" , horaDeRegado); 
    u8g2.setCursor(50, 30);
    u8g2.print(stringhoras);
    u8g2.drawStr(55,30,":");
    u8g2.setCursor(60, 30);
    sprintf (stringminutos, "%d" , minutosDeRegado); 
    u8g2.print(stringminutos);
    sprintf (stringtiempo, "%d" , intervalo); 
    u8g2.setCursor(80, 15);
    u8g2.print(stringtiempo);

    u8g2.sendBuffer();  

    if(horaDeRegado == timeinfo.tm_hour && minutosDeRegado == timeinfo.tm_min)
    {
      if(contadorLed == false){
        ledState = HIGH;
      digitalWrite(ledPin, ledState);
      Serial.println("LED_ON");
      TiempoUltimoCambio = millis();
      contadorLed = true;
      estadoBoton1 = false;
      estado = LED_ESPERA;
      }
      else{
        ledState = LOW;
        digitalWrite(ledPin, ledState);
      }
    }
      else{
        contadorLed = false;
      }
   
      if(digitalRead(PIN_BOTON_1) == LOW && digitalRead(PIN_BOTON_2) == LOW)
      {
        estado = LIMPIAR_1;
      }
      
      }

    break;
    
    case LED_ESPERA:
    {  
      Serial.println("LED_ESPERA");

       
       if (millis() - TiempoUltimoCambio >= intervalo)  
  {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      Serial.println("LED_OFF");
     
      estado = PANTALLA_INICIAL;
  } 
      if(digitalRead(PIN_BOTON_1) == LOW)
      {
        estadoBoton1 = true;
        Serial.println("BOTON_PRESIONADO");
      }
      if(digitalRead(PIN_BOTON_1) == HIGH && estadoBoton1 == true)
      {
        estadoBoton1 = false;
        Serial.println("BOTON_SUELTO");
        ledState = LOW;
        digitalWrite(ledPin, ledState);
        Serial.println("LED_OFF");
        contadorLed = true;

        estado = PANTALLA_INICIAL;
      }
  }
break;

    //Espera a que se suelten los botones para pasar a la siguiente pantalla
    case LIMPIAR_1:
    { 
      Serial.println("PRIMER ESPERA"); 
      if(digitalRead(PIN_BOTON_1) == HIGH && digitalRead(PIN_BOTON_2) == HIGH)
      {
        estado = CAMBIAR_HORA;
      }
    }
    break;

    //Pantalla que permite cambiar la hora
    case CAMBIAR_HORA: 
    {

    Serial.println("CAMBIAR LA HORA");
    Serial.println(horaDeRegado);  
    Serial.println(minutosDeRegado);
    
    u8g2.clearBuffer();  
    u8g2.setFont(u8g_font_5x7); 

    u8g2.drawStr(0,30,"Hora : ");
    sprintf (stringhoras, "%d" , horaDeRegado); 
    u8g2.setCursor(50, 30);
    u8g2.print(stringhoras);
    u8g2.drawStr(55,30,":");
    u8g2.setCursor(60, 30);
    sprintf (stringminutos, "%d" , minutosDeRegado); 
    u8g2.print(stringminutos);
   

    u8g2.sendBuffer();       
      
      if(digitalRead(PIN_BOTON_1) == LOW) 
      {
        estadoBoton1 = true;
        Serial.println("BOTON_PRESIONADO");
      }
     
      if(digitalRead(PIN_BOTON_1) == HIGH && estadoBoton1 == true) 
      {
        estadoBoton1 = false;
        Serial.println("BOTON_SUELTO");
        horaDeRegado = horaDeRegado + 1;
        if(horaDeRegado == 24){
          horaDeRegado = 0;
        }
        Serial.println(horaDeRegado);
        
      }    

      if(digitalRead(PIN_BOTON_2) == LOW)
      {
        estadoBoton2 = true;
        Serial.println("BOTON_PRESIONADO");
      }
      if(digitalRead(PIN_BOTON_2) == HIGH && estadoBoton2 == true)
      {
        estadoBoton2 = false;
        Serial.println("BOTON_SUELTO");
        minutosDeRegado = minutosDeRegado + 1;
        if(minutosDeRegado == 60){
          minutosDeRegado = 0;
        }
        Serial.println(minutosDeRegado);
      }    
      
      if(digitalRead(PIN_BOTON_1) == LOW && digitalRead(PIN_BOTON_2) == LOW) 
      {
        estado = LIMPIAR_2;
      }
    }
    break;

    //Espera a que se suelten los botones para volver a la pantalla inicial
    case LIMPIAR_2: 
    {
      Serial.println("SEGUNDA ESPERA");
      if(digitalRead(PIN_BOTON_1) == HIGH && digitalRead(PIN_BOTON_2) == HIGH) 
      {
        estado = CAMBIAR_TIEMPO;
             
      }
    }
    break;
  
  case CAMBIAR_TIEMPO: 
    {

    Serial.println("CAMBIAR EL TIEMPO"); 
    Serial.println(intervalo);
    
    u8g2.clearBuffer();  
    u8g2.setFont(u8g_font_5x7); 

    u8g2.drawStr(0,15,"TIEMPO: ");
    sprintf (stringtiempo, "%d" , intervalo);
    u8g2.setCursor(80, 15); 
    u8g2.print(stringtiempo);

    u8g2.sendBuffer();       
      
      if(digitalRead(PIN_BOTON_1) == LOW) 
      {
        estadoBoton1 = true;
        Serial.println("BOTON_PRESIONADO");

      }
      if(digitalRead(PIN_BOTON_1) == HIGH && estadoBoton1 == true) 
      {
        estadoBoton1 = false;
        Serial.println("BOTON_SUELTO");
        intervalo = intervalo + 30000;

      }    

      if(digitalRead(PIN_BOTON_2) == LOW)
      {
        estadoBoton2 = true;
        Serial.println("BOTON_PRESIONADO");
      }
      if(digitalRead(PIN_BOTON_2) == HIGH && estadoBoton2 == true)
      {
        estadoBoton2 = false;
        Serial.println("BOTON_SUELTO");
        intervalo = intervalo - 30000;
        if(intervalo < 0){
          intervalo = 0;
        }
        Serial.println(intervalo);
      }    
      
      if(digitalRead(PIN_BOTON_1) == LOW && digitalRead(PIN_BOTON_2) == LOW) 
      {
        estado = LIMPIAR_3;
      }
    }
    break;

    //Espera a que se suelten los botones para volver a la pantalla inicial
    case LIMPIAR_3: 
    {
      Serial.println("SEGUNDA ESPERA");
      if(digitalRead(PIN_BOTON_1) == HIGH && digitalRead(PIN_BOTON_2) == HIGH) 
      {
        estado = PANTALLA_INICIAL;
             
      }
    }
    break;   
}
  }
  

//Funciones Tiempo
void setup_rtc_ntp(void)
{
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  timestamp = time(NULL);
  rtc.setTime(timestamp + gmtOffset_sec);
}


void pedir_lahora(void)
{
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Veo la hora del rtc interno ");
    timestamp = rtc.getEpoch() - gmtOffset_sec;
    timeinfo = rtc.getTimeStruct();
  }

  else
  {
    Serial.print("NTP Time:");
    timestamp = time(NULL);
  }

  return;
}

//**********************************************************//
//* wifi ap                                **//
//**********************************************************//
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Captive Portal Demo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>Captive Portal Demo</h3>
  <br><br>
  <form action="/get">
    <br>
    SSID: <input type="text" name="ssid">
    <br>
    Password: <input type="text" name="password">
    <br>
    Token_tel: <input type="text" name="Token_tel">
    <br>
    Id_tel: <input type="number" name="Id_tel">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";
void StartCaptivePortal(void);
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html);
  }
};

void setupServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    request->send_P(200, "text/html", index_html);
    Serial.println("Client Connected"); });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String inputMessage;
    String inputParam;

    if (request->hasParam("ssid")) {
      inputMessage = request->getParam("ssid")->value();
      inputParam = "ssid";
      ssid = inputMessage;
      Serial.println(inputMessage);
      valid_ssid_received = true;
    }

    if (request->hasParam("password")) {
      inputMessage = request->getParam("password")->value();
      inputParam = "password";
      password = inputMessage;
      Serial.println(inputMessage);
      valid_password_received = true;
    }
    
    
    if (request->hasParam("Token_tel")) {
      inputMessage = request->getParam("Token_tel")->value();
      inputParam = "Token_tel";
      Token_tel = inputMessage;
      Serial.println(inputMessage);
      valid_password_received = true;
    }

    if (request->hasParam("Id_tel")) {
      inputMessage = request->getParam("Id_tel")->value();
      inputParam = "Id_tel";
      Id_tel = inputMessage;
      Serial.println(inputMessage);
      valid_password_received = true;
    }
    
    request->send(200, "text/html", "The values entered by you have been successfully sent to the device. It will now attempt WiFi connection"); });
}

void WiFiSoftAPSetup()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP("WIFI-GRUPO4");///puede agregarle el nombre del grupo
  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());
}

void WiFiStationSetup(String rec_ssid, String rec_password, String rec_Id_tel, String rec_Token_tel)
{
  wifi_timeout = false;
  WiFi.mode(WIFI_STA);
  
  rec_ssid.toCharArray(ap_ssid, rec_ssid.length() + 1);
  rec_password.toCharArray(ap_password, rec_password.length() + 1);
  rec_Token_tel.toCharArray(ap_token, rec_Token_tel.length() + 1);
  rec_Id_tel.toCharArray(ap_Id_tel, rec_Id_tel.length() + 1);

  WiFi.begin(ap_ssid, ap_password);

  uint32_t t1 = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(2000);
    Serial.print(F("."));
    if (millis() - t1 > 50000) // 50 seconds elapsed connecting to WiFi
    {
      // Serial.println();
      Serial.println(F("Timeout connecting to WiFi. The SSID and Password seem incorrect."));
      valid_ssid_received = false;
      valid_password_received = false;
      is_setup_done = false;
      preferences.putBool("is_setup_done", is_setup_done);

      StartCaptivePortal();
      wifi_timeout = true;
      break;
    }
  }
  if (!wifi_timeout)
  {
    is_setup_done = true;
    Serial.print(F("WiFi connected to: "));
    Serial.println(rec_ssid);
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    preferences.putBool("is_setup_done", is_setup_done);
    preferences.putString("rec_ssid", rec_ssid);
    preferences.putString("rec_password", rec_password);
    preferences.putString("rec_Token_tel", rec_Token_tel);
    preferences.putString("rec_Id_tel", rec_Id_tel);
  }
}

void StartCaptivePortal()
{
  Serial.println(F("Setting up AP Mode"));
  WiFiSoftAPSetup();
  Serial.println(F("Setting up Async WebServer"));
  setupServer();
  Serial.println(F("Starting DNS Server"));
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
  server.begin();
  dnsServer.processNextRequest();
}

void ap_wifisetup()
{
  Serial.println();
  preferences.begin("my-pref", false);
  //preferences.clear();//////////////////////////////////linea de codigo para borrar la eeprom
  is_setup_done = preferences.getBool("is_setup_done", false);
  ssid = preferences.getString("rec_ssid", "Sample_SSID");
  ssid.toCharArray(ap_ssid, ssid.length() + 1);

  password = preferences.getString("rec_password", "abcdefgh");
  password.toCharArray(ap_password, password.length() + 1);

  Token_tel = preferences.getString("rec_Token_tel", "abcdefgh");
  Token_tel.toCharArray(ap_token, Token_tel.length() + 1);

  Id_tel = preferences.getString("rec_Id_tel", "abcdefgh");
  Id_tel.toCharArray(ap_Id_tel, Id_tel.length() + 1);
  if (!is_setup_done)
  {
    StartCaptivePortal();
  }
  else
  {
    Serial.print(F("Saved SSID is "));
    Serial.println(ssid);
    Serial.print(F("Saved Password is "));
    Serial.println(password);
    Serial.print(F("Saved token is "));
    Serial.println(Token_tel);
    Serial.print(F("Saved id is "));
    Serial.println(Id_tel);

    WiFiStationSetup(ssid, password, Id_tel, Token_tel);
  }

  while (!is_setup_done)
  {
    dnsServer.processNextRequest();
    delay(10);
    if (valid_ssid_received && valid_password_received)
    {
      Serial.println(F("Attempting WiFi Connection!"));
      WiFiStationSetup(ssid, password, Id_tel, Token_tel);
    }
  }
}
