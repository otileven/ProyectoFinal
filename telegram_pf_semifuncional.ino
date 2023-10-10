
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

int horaAlarma = 13;
int minAlarma = 44;
String text;
//int casillero = 1;

// Initialize Telegram BOT
#define BOTtoken "6085478906:AAGkmo5pVC1NyXsRvGHJQ10DszGOgBZxsws"  // cambiar por token

String CHAT_ID = "-1001851624507"; /// 


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

int estado2 = 0;
const int ledPin = 25; /// pin led 
bool ledState = LOW;

// estados
#define ALARMA 0
#define HORA 1
#define MINUTOS 2
#define CONFIRMAR 3


// funcion que se comunica con telegram
void handleNewMessages(int numNewMessages) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){   ////si el id no corresponde da error . en caso de que no se quiera comprobar el id se debe sacar esta parte 
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    ///fin de verificacion

    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

      if (text == "/Alarma") {  
      estado2 = HORA;
      }
      if(text != "/Alarma"){
        bot.sendMessage(CHAT_ID, "Texto invalido, escriba (/Alarma) para comenzar a programar la alarma", "");
      }
    
  }
}



    //void MaquinaEstados (){
  

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  setup_rtc_ntp();
  
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
  bot.sendMessage(CHAT_ID, "Hola! Para programar una alarma ingrese: (/Alarma)", "");

  
}
void loop() {

  switch(estado2){
      case ALARMA:
      
       if (millis() > lastTimeBotRan + botRequestDelay) {
       int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
      Serial.println("Veo los msj nuevos");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
  
      }
    
       
      if(horaAlarma == timeinfo.tm_hour && minAlarma == timeinfo.tm_min)
    {
        ledState = HIGH;
      digitalWrite(ledPin, ledState);
      Serial.println("LED_ON");
    }
    
      break;

      case HORA:
      bot.sendMessage(CHAT_ID, "Para aumentar la hora en 1, ingrese (Hora mas), para disminuirla en 1, ingrese (Hora menos), para Continuar ingrese (Continuar)", "");
      if(text == "Hora mas") {
        horaAlarma = horaAlarma + 1;
        if(horaAlarma == 24){
          horaAlarma = 0;
        }
        bot.sendMessage(CHAT_ID, String(horaAlarma), ""); 
    }
    if(text == "Hora menos") {
        horaAlarma = horaAlarma - 1;
        if(horaAlarma == 0){
          horaAlarma = 24;
        }
        bot.sendMessage(CHAT_ID, String(horaAlarma), ""); 
    }
    
      if (text != "Hora mas" || text != "Hora menos" || text != "Continuar"){
        bot.sendMessage(CHAT_ID, "Texto inválido, ingrese (Hora mas) para aumentar la hora en 1 u (Hora menos) para disminuir la hora en 1. Para seleccionar los minutos ingrese (Continuar)", "");
      }
      if (text == "Continuar"){
        estado2 = MINUTOS;
      }
 
    
     break;
      
    case MINUTOS:
      bot.sendMessage(CHAT_ID, "Para aumentar los minutos en 5, ingrese (Min mas), para disminuirlos en 5, ingrese (Min menos), para Continuar ingrese (Continuar)", "");
        if(text == "Min mas") {
        minAlarma = minAlarma + 5;
        if(minAlarma == 60){
          horaAlarma = 0;
        }
        bot.sendMessage(CHAT_ID, String(minAlarma), ""); 

    }
    if(text == "Min menos") {
        minAlarma = minAlarma - 5;
        if(minAlarma == 0){
          minAlarma = 60;
        }
       bot.sendMessage(CHAT_ID, String(minAlarma), ""); 
 
    }
    
      if (text != "Min mas" || text != "Min menos" || text != "Continuar"){
        bot.sendMessage(CHAT_ID, "Texto inválido, ingrese (Min mas) para aumentar los minutos en 5 o (Min menos) para disminuir los minutos en 5, para Continuar ingrese (Continuar)", "");
      }

       if (text == "Continuar"){
        estado2 = CONFIRMAR;
      }
      break;

    
     /* case CASILLEROS:
      bot.sendMessage(CHAT_ID, "ingrese (Cas mas) para aumentar en uno el numero de casillero o (Cas menos) para disminuirlo en uno, con un maximo de casillero numero 12", "");
       if(text == "Cas mas") {
        casillero = casillero + 1;
        if(casillero == 12){
          casillero = 0;
        }
        bot.sendMessage(CHAT_ID, casillero, ""); 

    }
    if(text == "Cas menos") {
        casillero = casillero - 1;
        if(casillero == 0){
          casillero = 12;
        }
       bot.sendMessage(CHAT_ID, casillero, ""); 
 
    }
    
      if (text != "Cas. mas" || text != "Cas. menos" || text != "Continuar"){
        bot.sendMessage(CHAT_ID, "Texto inválido, ingrese (Cas mas) para aumentar el numero de casillero o (Cas menos) para disminuirlo, para Continuar ingrese (Continuar)", "");
      }

      if (text == "Continuar"){
        estado2 = CONFIRMAR;
      }*/

     case CONFIRMAR:
     bot.sendMessage(CHAT_ID, String(horaAlarma), "");
     bot.sendMessage(CHAT_ID, String(minAlarma), "");
     bot.sendMessage(CHAT_ID, "Estos son los datos ingresados, ¿Confirmar?", "");
     bot.sendMessage(CHAT_ID, "Para aplicar los valores de la alarma ingrese (confirmar), en caso de querer modificar la hora, ingrese (hora), en caso de querer modificar los minutos, ingrese (min), en caso de querer modificar el numero de casillero, ingrese (casillero)", "");

     if(text == "hora"){
      estado2 = HORA;
     }
     if(text == "min"){
      estado2 = MINUTOS;
     }
     /*if(text == "casillero"){
      estado2 = CASILLERO;
     }*/
     if(text == "confirmar"){
      estado2 = ALARMA;
     }
     if(text != "hora" || text != "min" || text != "casillero" || text != "confirmar"){
       bot.sendMessage(CHAT_ID, "texto invalido, ingrese (confirmar), en caso de querer modificar la hora, ingrese (hora), en caso de querer modificar los minutos, ingrese (min), en caso de querer modificar el numero de casillero, ingrese (casillero) ", "");
     }
    }
    
    /*if (text == "/led_off") {
      bot.sendMessage(CHAT_ID, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(CHAT_ID, "LED is ON", "");
      }
      else{
        bot.sendMessage(CHAT_ID, "LED is OFF", "");
      }
    }*/
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
