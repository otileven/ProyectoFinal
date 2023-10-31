
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

//Inicializar rtc
int gmt;
struct tm timeinfo;
ESP32Time rtc;

//Reloj
long unsigned int timestamp; 
const char *ntpServer = "south-america.pool.ntp.org";
long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

//Funciones
void pedir_lahora(void);
void setup_rtc_ntp(void);


// credenciales de wifi
const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";


String text;
//int casillero = 1;

// Initialize Telegram BOT
#define BOTtoken "6085478906:AAGkmo5pVC1NyXsRvGHJQ10DszGOgBZxsws"  // cambiar por token

String CHAT_ID = "-1001851624507"; /// 


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 2000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

int estado2 = 0;


// estados
#define ALARMA 0
#define CASILLERO 1
#define HORA 2
#define MINUTOS 4
#define CONFIRMAR 5

bool esperandoRespuesta = false;

  

bool ledState = LOW; 
   

typedef struct {
  int horaAlarma  = 16;
  int minAlarma  = 10;
  const int ledPin = 26;
} Alarma;


int casillero = 0;
Alarma casilleroArray [2];




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
        if(casillero == 10){
          casillero = 0;
        }
        bot.sendMessage(CHAT_ID, String(casillero), ""); 
    }
    else if(text == "/Cas menos") {
        Serial.println("CASILLERO MENOS"); 
        casillero = casillero - 1;
        if(casillero == - 1){
          casillero = 9;
        }
        bot.sendMessage(CHAT_ID, String(casillero), ""); 
    }
    else if (text == "/Confirmar cas"){
        Serial.println("CONFIRMAR CASILLERO"); 
        esperandoRespuesta = false;
        estado2 = HORA;
      }
      else{
        Serial.println("TEXTO INVÁLIDO CASILLERO"); 
        bot.sendMessage(CHAT_ID, "Texto inválido, ingrese (/Cas mas) para aumentar el casillero en 1 o (/Cas menos) para disminuir el casillero en 1. Para seleccionar el casillero ingrese (/Confirmar cas)", "");
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
        casilleroArray[0].horaAlarma  = casilleroArray[0].horaAlarma  + 1;
        if(casilleroArray[0].horaAlarma  == 24){
          casilleroArray[0].horaAlarma  = 0;
        }
        bot.sendMessage(CHAT_ID, String(casilleroArray[0].horaAlarma ), ""); 
    }
    else if(text == "/Hora menos") {
        Serial.println("HORA MENOS"); 
        casilleroArray[0].horaAlarma  = casilleroArray[0].horaAlarma  - 1;
        if(casilleroArray[0].horaAlarma  == - 1){
          casilleroArray[0].horaAlarma  = 23;
        }
        bot.sendMessage(CHAT_ID, String(casilleroArray[0].horaAlarma ), ""); 
    }
    else if (text == "/Confirmar hora"){
        Serial.println("CONFIRMAR HORA"); 
        esperandoRespuesta = false;
        estado2 = MINUTOS;
      }
      else{
        Serial.println("TEXTO INVÁLIDO HORA"); 
        bot.sendMessage(CHAT_ID, "Texto inválido, ingrese (/Hora mas) para aumentar la hora en 1 u (/Hora menos) para disminuir la hora en 1. Para seleccionar los minutos ingrese (/Confirmar hora)", "");
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
        casilleroArray[0].minAlarma  = casilleroArray[0].minAlarma  + 5;
        if(casilleroArray[0].minAlarma  == 60){
          casilleroArray[0].minAlarma  = 0;
        }
        bot.sendMessage(CHAT_ID, String(casilleroArray[0].minAlarma ), ""); 

    }
    
    else if(text == "/Min menos") {
        Serial.println("MIN MENOS"); 
        casilleroArray[0].minAlarma  = casilleroArray[0].minAlarma  - 5;
        if(casilleroArray[0].minAlarma  == - 5){
          casilleroArray[0].minAlarma  = 55;
        }
       bot.sendMessage(CHAT_ID, String(casilleroArray[0].minAlarma ), ""); 
 
    }
     else if (text == "/Confirmar min"){
        Serial.println("CONFIRMAR MIN");
        esperandoRespuesta = false;
        estado2 = CONFIRMAR;
      }
      else{
        Serial.println("TEXTO INVÁLIDO"); 
        bot.sendMessage(CHAT_ID, "Texto inválido, ingrese (/Min mas) para aumentar los minutos en 5 o (/Min menos) para disminuir los minutos en 5, para Continuar ingrese (/Confirmar min)", "");
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
       bot.sendMessage(CHAT_ID, "texto invalido, ingrese (/Confirmar), en caso de querer modificar la hora, ingrese (/Hora), en caso de querer modificar los minutos, ingrese (/Min), en caso de querer modificar el numero de casillero, ingrese (casillero) ", "");
     }
  
}
}  

void setup() {
  Serial.begin(115200);

  pinMode(casilleroArray[0].ledPin, OUTPUT);
  digitalWrite(casilleroArray[0].ledPin, ledState);

  setup_rtc_ntp();
  pedir_lahora();

  
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
       pedir_lahora();
       if(esperandoRespuesta == false){
       Serial.println("ALARMA");
       bot.sendMessage(CHAT_ID, "Hola! Para programar una alarma ingrese: (/Alarma)", "");
       esperandoRespuesta = true;
      } 
       if (millis() > lastTimeBotRan + botRequestDelay) {
       Serial.println("FUNCION ALARMA");
        
      if(casilleroArray[0].horaAlarma == timeinfo.tm_hour && casilleroArray[0].minAlarma  == timeinfo.tm_min)
    {
        ledState = HIGH;
      digitalWrite(casilleroArray[0].ledPin, ledState);
      Serial.println("LED_ON");
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
      bot.sendMessage(CHAT_ID, "Para aumentar el casillero en 1, ingrese (/Cas mas), para disminuirlo en 1, ingrese (Cas menos), para Continuar ingrese (/Confirmar cas)", "");
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
      bot.sendMessage(CHAT_ID, "Para aumentar la hora en 1, ingrese (/Hora mas), para disminuirla en 1, ingrese (Hora menos), para Continuar ingrese (/Confirmar hora)", "");
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
      bot.sendMessage(CHAT_ID, "Para aumentar los minutos en 5, ingrese (/Min mas), para disminuirlos en 5, ingrese (/Min menos), para Continuar ingrese (/Confirmar min)", "");
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
     bot.sendMessage(CHAT_ID, String(casilleroArray[0].horaAlarma ), "");
     bot.sendMessage(CHAT_ID, String(casilleroArray[0].minAlarma ), "");
     bot.sendMessage(CHAT_ID, "Estos son los datos ingresados, ¿Confirmar?", "");
     bot.sendMessage(CHAT_ID, "Para aplicar los valores de la alarma ingrese (/Confirmar), en caso de querer modificar la hora, ingrese (/Hora), en caso de querer modificar los minutos, ingrese (/Min), en caso de querer modificar el numero de casillero, ingrese (casillero)", "");
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
