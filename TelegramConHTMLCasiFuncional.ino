
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#include <Wire.h>
#include <ESP32Time.h>
#include <U8g2lib.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include "time.h"

//Pantalla Display en pixeles
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

//Inicializar rtc
int gmt;
struct tm timeinfo;
ESP32Time rtc;

//Constructores y variables globales
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Pantalla Display en pixeles
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

//Reloj
long unsigned int timestamp; 
const char *ntpServer = "south-america.pool.ntp.org";
long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

//Funciones
void pedir_lahora(void);
void setup_rtc_ntp(void);


// credenciales de wifi
//const char* ssid = "ORT-IoT";
//const char* password = "OrtIOTnew22$2";


String text;
//int casillero = 1;

// Initialize Telegram BOT
//#define BOTtoken "6085478906:AAGkmo5pVC1NyXsRvGHJQ10DszGOgBZxsws"  // cambiar por token

//String CHAT_ID = "-1001851624507"; /// 


WiFiClientSecure client;
//UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 2000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

int estado2 = 0;


// estados
#define ALARMA 0
#define CASILLERO 1
#define HORA 2
#define MINUTOS 4
#define CONFIRMAR 5
#define LED_ESPERA 6

bool esperandoRespuesta = false;

#define PIN_BOTON_1 35
bool estadoBoton1 = false; 

bool ledState = LOW; 

unsigned long TiempoUltimoCambio = 0;
int espera = 30000;



typedef struct {
  int horaAlarma;
  int minAlarma;
  int tiempoRestante;
} Alarma;

typedef struct {
  int ledPin;
  bool ledState;
  bool contadorLed;
 
} LED;


int casillero = 0;

Alarma casilleroArray [2]{
  {12, 45, 0},
  {17, 30, 0},
};

LED ledsArray [2]{
  {25, LOW, false},
  {26, LOW, false},
};

int menorTiempo = 1440;
int siguienteAlarma = 0;

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



// funcion que se comunica con telegram
void handleNewMessagesAlarma(int numNewMessagesAlarma) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesAlarma));

  for (int i=0; i<numNewMessagesAlarma; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);


    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    
      Serial.println("ALARMA"); 
      if (text == "/Alarma") { 
        Serial.println("SE DETECTÓ ALARMA"); 
        esperandoRespuesta = false;
        estado2 = CASILLERO;
      }
      if(text != "/Alarma"){
         Serial.println("NO SE DETECTÓ ALARMA"); 
        bot.sendMessage(CHAT_ID, "Texto invalido, escriba (/Alarma) para comenzar a programar la alarma", "");
      }
}
}

// funcion que se comunica con telegram
void handleNewMessagesCasillero(int numNewMessagesCasillero) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesCasillero));

  for (int i=0; i<numNewMessagesCasillero; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    
    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    
      Serial.println("CASILLERO"); 
      if(text == "/Cas mas") {
        Serial.println("CASILLERO MAS"); 
        casillero = casillero + 1;
        if(casillero == 2){
          casillero = 0;
        }
        bot.sendMessage(CHAT_ID, "El casillero actual es: " + String(casillero), ""); 
    }
    else if(text == "/Cas menos") {
        Serial.println("CASILLERO MENOS"); 
        casillero = casillero - 1;
        if(casillero == - 1){
          casillero = 1;
        }
        bot.sendMessage(CHAT_ID, "El casillero actual es: " + String(casillero), ""); 
    }
    else if (text == "/Confirmar cas"){
        Serial.println("CONFIRMAR CASILLERO"); 
        esperandoRespuesta = false;
        estado2 = HORA;
      }
      else if (text == "/Volver"){
        Serial.println("VOLVER"); 
        esperandoRespuesta = false;
        estado2 = CONFIRMAR;
      }
      else{
        Serial.println("TEXTO INVÁLIDO CASILLERO"); 
        bot.sendMessage(CHAT_ID, "Texto inválido \nIngrese (/Cas mas) para aumentar el casillero en 1 o (/Cas menos) para disminuir el casillero en 1. \nPara seleccionar el casillero ingrese (/Confirmar cas). \nSi modificó el casillero y desea confirmar la alarma ingrese (/Volver).", "");
      }
      
 
}
}


// funcion que se comunica con telegram
void handleNewMessagesHora(int numNewMessagesHora) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesHora));

  for (int i=0; i<numNewMessagesHora; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);

    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;


       if(text == "/Hora mas") {
        Serial.println("HORA MAS"); 
        casilleroArray[casillero].horaAlarma  = casilleroArray[casillero].horaAlarma  + 1;
        if(casilleroArray[casillero].horaAlarma  == 24){
          casilleroArray[casillero].horaAlarma  = 0;
        }
        bot.sendMessage(CHAT_ID, "La hora actual es: " + String(casilleroArray[casillero].horaAlarma), ""); 
    }
    else if(text == "/Hora menos") {
        Serial.println("HORA MENOS"); 
        casilleroArray[casillero].horaAlarma  = casilleroArray[casillero].horaAlarma  - 1;
        if(casilleroArray[casillero].horaAlarma  == - 1){
          casilleroArray[casillero].horaAlarma  = 23;
        }
        bot.sendMessage(CHAT_ID, "La hora actual es: " + String(casilleroArray[casillero].horaAlarma), ""); 
    }
    else if (text == "/Confirmar hora"){
        Serial.println("CONFIRMAR HORA"); 
        esperandoRespuesta = false;
        estado2 = MINUTOS;
      }
      else if (text == "/Volver"){
        Serial.println("VOLVER"); 
        esperandoRespuesta = false;
        estado2 = CONFIRMAR;
      }
      else{
        Serial.println("TEXTO INVÁLIDO HORA"); 
        bot.sendMessage(CHAT_ID, "Texto inválido \nIngrese (/Hora mas) para aumentar la hora en 1 u (/Hora menos) para disminuir la hora en 1. \nPara seleccionar los minutos ingrese (/Confirmar hora). \nSi modificó la hora y desea confirmar la alarma ingrese (/Volver).", "");
      }
      
 
  
}
}


// funcion que se comunica con telegram
void handleNewMessagesMin(int numNewMessagesMin) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesMin));

  for (int i=0; i<numNewMessagesMin; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);

    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
 
     if(text == "/Min mas") {
        Serial.println("MIN MAS"); 
        casilleroArray[casillero].minAlarma  = casilleroArray[casillero].minAlarma  + 5;
        if(casilleroArray[casillero].minAlarma  == 60){
          casilleroArray[casillero].minAlarma  = 0;
        }
        bot.sendMessage(CHAT_ID, "Los minutos actuales son: " + String(casilleroArray[casillero].minAlarma), "");

    }
    
    else if(text == "/Min menos") {
        Serial.println("MIN MENOS"); 
        casilleroArray[casillero].minAlarma  = casilleroArray[casillero].minAlarma  - 5;
        if(casilleroArray[casillero].minAlarma  == - 5){
          casilleroArray[casillero].minAlarma  = 55;
        }
        bot.sendMessage(CHAT_ID, "Los minutos actuales son: " + String(casilleroArray[casillero].minAlarma), "");
 
    }
     else if (text == "/Confirmar min"){
        Serial.println("CONFIRMAR MIN");
        esperandoRespuesta = false;
        estado2 = CONFIRMAR;
      }
      else if (text == "/Volver"){
        Serial.println("VOLVER"); 
        esperandoRespuesta = false;
        estado2 = CONFIRMAR;
      }
      else{
        Serial.println("TEXTO INVÁLIDO"); 
        bot.sendMessage(CHAT_ID, "Texto inválido \nIngrese (/Min mas) para aumentar los minutos en 5 o (/Min menos) para disminuir los minutos en 5. \nPara Continuar ingrese (/Confirmar min). \nSi modificó la hora y desea confirmar la alarma ingrese (/Volver).", "");
      }
  
}
}


// funcion que se comunica con telegram
void handleNewMessagesConfirmar(int numNewMessagesConfirmar) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesConfirmar));

  for (int i=0; i<numNewMessagesConfirmar; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);


    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
 
    if(text == "/Hora"){
      Serial.println("HORA");
      esperandoRespuesta = false; 
      estado2 = HORA;
     }
     else if(text == "/Min"){
      Serial.println("MIN"); 
      esperandoRespuesta = false; 
      estado2 = MINUTOS;
     }

     else if(text == "/Confirmar"){
      Serial.println("CONFIRMAR"); 
      esperandoRespuesta = false;
      bot.sendMessage(CHAT_ID, "Alarma establecida, muchas gracias", "");
      estado2 = ALARMA;
     }
     else{
       Serial.println("TEXTO INVÁLIDO"); 
       bot.sendMessage(CHAT_ID, "Texto inválido \nIngrese (/Confirmar) \nEn caso de querer modificar la hora, ingrese (/Hora). \nEn caso de querer modificar los minutos, ingrese (/Min). \nEn caso de querer modificar el numero de casillero, ingrese (/Cas) ", "");
     }
  
}
}  

void setup() {
  Serial.begin(115200);
    u8g2.begin();
  pinMode(PIN_BOTON_1, INPUT_PULLUP);  
  pinMode(ledsArray[0].ledPin, OUTPUT);
  digitalWrite(ledsArray[0].ledPin,ledsArray[0].ledState);
  pinMode(ledsArray[1].ledPin, OUTPUT);
  digitalWrite(ledsArray[1].ledPin,ledsArray[1].ledState);

  setup_rtc_ntp();
  pedir_lahora();
    ap_wifisetup(); 

  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  ///bloquea el programa si no se puede conectar a internet 
  while (WiFi.status() != WL_CONNECTED) {   
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  
}
void loop() {

  switch(estado2){
      case ALARMA:
      char stringhoras[7];  
    char stringminutos[7];
    delay (1000);
       pedir_lahora();
       u8g2.clearBuffer();  
       u8g2.setFont(u8g_font_5x7); 
       u8g2.drawStr(0,15,"Siguiente alarma : ");
       sprintf (stringhoras, "%d" , casilleroArray[siguienteAlarma].horaAlarma); 
       u8g2.setCursor(30, 30);
      u8g2.print(stringhoras);
      u8g2.drawStr(45,30,":");
      u8g2.setCursor(60, 30);
      sprintf (stringminutos, "%d" , casilleroArray[siguienteAlarma].minAlarma); 
      u8g2.print(stringminutos);
      u8g2.sendBuffer();  


      Serial.println("Próxima alarma:" + String(casilleroArray[siguienteAlarma].horaAlarma) + ":" + String(casilleroArray[siguienteAlarma].minAlarma));
       if(esperandoRespuesta == false){
       Serial.println("ALARMA");
       bot.sendMessage(CHAT_ID, "Hola! Para programar una alarma ingrese: (/Alarma)", "");
       esperandoRespuesta = true;
      } 
       if (millis() > lastTimeBotRan + botRequestDelay) {
       Serial.println("FUNCION ALARMA");
         for (int casillero=0; casillero<2; casillero++) {
         casilleroArray[casillero].tiempoRestante = (casilleroArray[casillero].horaAlarma - timeinfo.tm_hour)*60 + casilleroArray[casillero].minAlarma - timeinfo.tm_min;
         Serial.println(casilleroArray[casillero].tiempoRestante);
         if(casilleroArray[casillero].tiempoRestante <= 0){
          Serial.println("ES NEGATIVO");
          casilleroArray[casillero].tiempoRestante = (casilleroArray[casillero].horaAlarma + 24 - timeinfo.tm_hour)*60 + casilleroArray[casillero].minAlarma - timeinfo.tm_min;
          Serial.println(casilleroArray[casillero].tiempoRestante);
          if(casilleroArray[casillero].tiempoRestante < menorTiempo){
           Serial.println("AHORA ES POSITIVO");
            siguienteAlarma = casillero;
            casilleroArray[casillero].tiempoRestante = menorTiempo;
         }
         }
         else if(casilleroArray[casillero].tiempoRestante < menorTiempo){
                    Serial.println("ES POSITIVO");
            siguienteAlarma = casillero;
            casilleroArray[casillero].tiempoRestante = menorTiempo;
         }
         }

        for (int casillero=0; casillero<2; casillero++) {
        Serial.println("FOR");
      if(casilleroArray[casillero].horaAlarma == timeinfo.tm_hour && casilleroArray[casillero].minAlarma  == timeinfo.tm_min)
    {
      if(ledsArray[casillero].contadorLed == false){
        ledsArray[casillero].ledState = HIGH;
        digitalWrite(ledsArray[casillero].ledPin,ledsArray[casillero].ledState);
        Serial.println("LED_ON");
        TiempoUltimoCambio = millis();
      ledsArray[casillero].contadorLed = true;
      estadoBoton1 = false;
      estado2 = LED_ESPERA;
      }
      else{
        ledState = LOW;
        digitalWrite(ledsArray[casillero].ledPin,ledsArray[casillero].ledState);
      }
    }
    else{
        ledsArray[casillero].contadorLed = false;
      }
        }
    
       int numNewMessagesAlarma = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessagesAlarma) {
      Serial.println("Veo los msj nuevos");
      handleNewMessagesAlarma(numNewMessagesAlarma);
      numNewMessagesAlarma = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
  
      }     
   

    
    
      break;

      case CASILLERO:
      if(esperandoRespuesta == false){
      Serial.println("CASILLERO"); 
      bot.sendMessage(CHAT_ID, "Para aumentar el casillero en 1, ingrese (/Cas mas).\nPara disminuirlo en 1, ingrese (/Cas menos).\nPara continuar ingrese (/Confirmar cas).\nSi desea modificar el casillero y confirmar la alarma ingrese (/Volver).\n ", "");
      bot.sendMessage(CHAT_ID, "El casillero actual es: " + String(casillero), "");
      esperandoRespuesta = true;
      }
      if (millis() > lastTimeBotRan + botRequestDelay) {
       Serial.println("FUNCION CASILLERO");
       int numNewMessagesCasillero = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessagesCasillero) {
      Serial.println("Veo los msj nuevos");
      handleNewMessagesCasillero(numNewMessagesCasillero);
      numNewMessagesCasillero = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
  
      }
    
     break;

      case HORA:
      if(esperandoRespuesta == false){
      Serial.println("HORA"); 
      bot.sendMessage(CHAT_ID, "Para aumentar la hora en 1, ingrese (/Hora mas). \nPara disminuirla en 1, ingrese (Hora menos). \nPara Continuar ingrese (/Confirmar hora). \nSi desea modificar la hora y confirmar la alarma ingrese (/Volver).", "");
      bot.sendMessage(CHAT_ID, "La hora actual es: " + String(casilleroArray[casillero].horaAlarma), "");
      esperandoRespuesta = true;
      }
      if (millis() > lastTimeBotRan + botRequestDelay) {
       Serial.println("FUNCION HORA");
       int numNewMessagesHora = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessagesHora) {
      Serial.println("Veo los msj nuevos");
      handleNewMessagesHora(numNewMessagesHora);
      numNewMessagesHora = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
  
      }
    
     break;
      
    case MINUTOS:
      if(esperandoRespuesta == false){
      Serial.println("MINUTOS"); 
      bot.sendMessage(CHAT_ID, "Para aumentar los minutos en 5, ingrese (/Min mas). \nPara disminuirlos en 5, ingrese (/Min menos). \nPara Continuar ingrese (/Confirmar min)", "");
      bot.sendMessage(CHAT_ID, "Los minutos actuales son: " + String(casilleroArray[casillero].minAlarma), "");
      esperandoRespuesta = true;
      }
      if (millis() > lastTimeBotRan + botRequestDelay) {
        Serial.println("FUNCION MIN");
       int numNewMessagesMin = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessagesMin) {
      Serial.println("Veo los msj nuevos");
      handleNewMessagesMin(numNewMessagesMin);
      numNewMessagesMin = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
  
      } 
      break;


     case CONFIRMAR:
     if(esperandoRespuesta == false){
     Serial.println("CONFIRMAR"); 
     bot.sendMessage(CHAT_ID, "La hora de la alarma es: " + String(casilleroArray[casillero].horaAlarma) + ":" + String(casilleroArray[casillero].minAlarma ), "");
     bot.sendMessage(CHAT_ID, "¿Confirmar?", "");
     bot.sendMessage(CHAT_ID, "Para aplicar los valores de la alarma ingrese (/Confirmar). \nEn caso de querer modificar la hora, ingrese (/Hora). \nEn caso de querer modificar los minutos, ingrese (/Min). \nEn caso de querer modificar el numero de casillero, ingrese (/Cas).", "");
      esperandoRespuesta = true;
      }
    
     if (millis() > lastTimeBotRan + botRequestDelay) {
       Serial.println("FUNCION CONFIRMAR");
       int numNewMessagesConfirmar = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessagesConfirmar) {
      Serial.println("Veo los msj nuevos");
      handleNewMessagesConfirmar(numNewMessagesConfirmar);
      numNewMessagesConfirmar = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
  
      }

     break;

     case LED_ESPERA:
    {  
      Serial.println("LED_ESPERA");

       
       if (millis() - TiempoUltimoCambio >= espera)  
  {
    for (int casillero=0; casillero<2; casillero++) {
      ledsArray[casillero].ledState = LOW;
      digitalWrite(ledsArray[casillero].ledPin,ledsArray[casillero].ledState);
      Serial.println("LED_OFF");
     
      estado2 = ALARMA;
  }
    
  }
   
      if(digitalRead(PIN_BOTON_1) == LOW)
      {
        estadoBoton1 = true;
        Serial.println("BOTON_PRESIONADO");
      }
      if(digitalRead(PIN_BOTON_1) == HIGH && estadoBoton1 == true)
      {
       for (int casillero=0; casillero<2; casillero++) {
        estadoBoton1 = false;
        Serial.println("BOTON_SUELTO");
        ledsArray[casillero].ledState = LOW;
        digitalWrite(ledsArray[casillero].ledPin,ledsArray[casillero].ledState);
        Serial.println("LED_OFF");
        ledsArray[casillero].contadorLed = true;

        estado2 = ALARMA;
      }
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
