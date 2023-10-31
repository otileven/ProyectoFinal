//Librerías
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
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

//Credenciales de wifi
const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";

// Initialize Telegram BOT
#define BOTtoken "6085478906:AAGkmo5pVC1NyXsRvGHJQ10DszGOgBZxsws"  
String CHAT_ID = "-1001851624507"; 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//Variables
int intervalo = 1000; 
unsigned long lastTimeBotRan=0;
unsigned long now; 
String text;
int estado2 = 0;
bool esperandoRespuesta = false;
//Funcion que se comunica con telegram: Alarma
int casillero = 4;
bool ledState = LOW; 
const int ledPin = 26;

//Estados
#define ALARMA 0
#define CASILLERO 1
#define HORA 2
#define MINUTOS 3
#define CONFIRMAR 4

//Estructura
typedef struct {
  int horaAlarma  = 14;
  int minAlarma  = 5;
} Alarma; 

//Array
Alarma miArray [10];

void handleNewMessagesAlarma(int numNewMessagesAlarma) {
    Serial.println("Mensaje nuevo");
    Serial.println(String(numNewMessagesAlarma));
    
    for (int i=0; i<numNewMessagesAlarma; i++) {
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
    
    Serial.println("ALARMA"); 
      if (text == "/Alarma") { 
        Serial.println("SE DETECTÓ ALARMA"); 
        esperandoRespuesta = false;
        bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
        estado2 = CASILLERO;
      }
      if(text != "/Alarma"){
        Serial.println("NO SE DETECTÓ ALARMA"); 
        bot.sendMessage(CHAT_ID, "Texto invalido, escriba (/Alarma) para comenzar a programar la alarma", "");
      }
}
}

//Funcion que se comunica con telegram: Casilleros
void handleNewMessagesCasillero(int numNewMessagesCasillero) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesCasillero));

  for (int i=0; i<numNewMessagesCasillero; i++) {
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

//Funcion que se comunica con telegram: Hora
void handleNewMessagesHora(int numNewMessagesHora) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesHora));

  for (int i=0; i<numNewMessagesHora; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    if(chat_id != CHAT_ID){   ////si el id no corresponde da error . en caso de que no se quiera comprobar el id se debe sacar esta parte 
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
      }
    ///fin de verificacion

    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

     if(text == "/Hora mas") {
       Serial.println("HORA MAS"); 
       miArray[casillero].horaAlarma  = miArray[casillero].horaAlarma  + 1;
       if(miArray[casillero].horaAlarma  == 24){
       miArray[casillero].horaAlarma  = 0;
        }
        bot.sendMessage(CHAT_ID, String(miArray[casillero].horaAlarma ), ""); 
        }
     else if(text == "/Hora menos") {
        Serial.println("HORA MENOS"); 
        miArray[casillero].horaAlarma  = miArray[casillero].horaAlarma  - 1;
        if(miArray[casillero].horaAlarma  == - 1){
        miArray[casillero].horaAlarma  = 23;
        }
        bot.sendMessage(CHAT_ID, String(miArray[casillero].horaAlarma ), ""); 
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


//Funcion que se comunica con telegram: Minutos
void handleNewMessagesMin(int numNewMessagesMin) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesMin));

  for (int i=0; i<numNewMessagesMin; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    if(chat_id != CHAT_ID){   ////si el id no corresponde da error . en caso de que no se quiera comprobar el id se debe sacar esta parte 
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
      }
    ///fin de verificacion

    // imprime el msj recibido 
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
 
     if(text == "/Min mas") {
        Serial.println("MIN MAS"); 
        miArray[casillero].minAlarma  = miArray[casillero].minAlarma  + 5;
        if(miArray[casillero].minAlarma  == 60){
        miArray[casillero].minAlarma  = 0;
        }
        bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
        }
    
     else if(text == "/Min menos") {
        Serial.println("MIN MENOS"); 
        if(miArray[casillero].minAlarma  == 0){
        miArray[casillero].minAlarma  = 60;
        }
        miArray[casillero].minAlarma  = miArray[casillero].minAlarma  - 5;
       bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
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


//Funcion que se comunica con telegram: Confirmacion
void handleNewMessagesConfirmar(int numNewMessagesConfirmar) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessagesConfirmar));

  for (int i=0; i<numNewMessagesConfirmar; i++) {
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
    else if(text == "/CAS"){
      estado2 = CASILLERO;
     }
    else if(text == "/Confirmar"){
      Serial.println("CONFIRMAR"); 
      esperandoRespuesta = false;
      bot.sendMessage(CHAT_ID, "Alarma establecida, muchas gracias", "");
      estado2 = ALARMA;
     }
     else{
      Serial.println("TEXTO INVÁLIDO"); 
      bot.sendMessage(CHAT_ID, "texto invalido, ingrese (/Confirmar), en caso de querer modificar la hora, ingrese (/Hora), en caso de querer modificar los minutos, ingrese (/Min), en caso de querer modificar el numero de casillero, ingrese (/Cas) ", "");
     }
  
}
}  

void setup() {
  Serial.begin(115200);

  //LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  //Funciones
  setup_rtc_ntp();
  pedir_lahora();
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  //Bloquea el programa si no se puede conectar a internet 
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
       bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
       Serial.println("ALARMA");
       Serial.println(miArray[casillero].minAlarma); 
       Serial.println("mil"); 
      
       if(esperandoRespuesta == false){
       Serial.println("ALARMA");
       bot.sendMessage(CHAT_ID, "Hola! Para programar una alarma ingrese: (/Alarma)", "");
       esperandoRespuesta = true;
        } 


        now =millis(); 
       if ( now -lastTimeBotRan >  intervalo) {
       Serial.println("FUNCION ALARMA");
       for (casillero = 0; casillero < 10; casillero++) {
        Serial.println("FOR");
        bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
            if(miArray[casillero].horaAlarma == timeinfo.tm_hour && miArray[casillero].minAlarma == timeinfo.tm_min)
              {
              ledState = HIGH;
              digitalWrite(ledPin, ledState);
              Serial.println("LED_ON");
              }
        }
       
       int numNewMessagesAlarma = bot.getUpdates(bot.last_message_received + 1);
       while(numNewMessagesAlarma) {
       Serial.println("Veo los msj nuevos");
       handleNewMessagesAlarma(numNewMessagesAlarma);
       numNewMessagesAlarma = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = now;
      }     

      break;

      case CASILLERO:
    
      //bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
      int test = 5;
      bot.sendMessage(CHAT_ID, String(test),""); 

      Serial.println(miArray[casillero].minAlarma); 
      
      if(esperandoRespuesta == false){
        Serial.println("CASILLERO"); 
        bot.sendMessage(CHAT_ID, "Para aumentar el casillero en 1, ingrese (/Cas mas), para disminuirlo en 1, ingrese (Cas menos), para Continuar ingrese (/Confirmar cas)", "");
        esperandoRespuesta = true;
      }
       now = millis(); 
       if ( now -lastTimeBotRan >  intervalo) {
       Serial.println("FUNCION CASILLERO");
       int numNewMessagesCasillero = bot.getUpdates(bot.last_message_received + 1);
       while(numNewMessagesCasillero) {
       Serial.println("Veo los msj nuevos");
       handleNewMessagesCasillero(numNewMessagesCasillero);
       numNewMessagesCasillero = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = now;
      }
    
     break;

     case HORA:
     bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
     Serial.println(miArray[casillero].minAlarma); 
     
    if(esperandoRespuesta == false){
      Serial.println("HORA"); 
      bot.sendMessage(CHAT_ID, "Para aumentar la hora en 1, ingrese (/Hora mas), para disminuirla en 1, ingrese (Hora menos), para Continuar ingrese (/Confirmar hora)", "");
      esperandoRespuesta = true;
      }
     
    now = millis(); 
       if ( now -lastTimeBotRan >  intervalo) {
      Serial.println("FUNCION HORA");
      int numNewMessagesHora = bot.getUpdates(bot.last_message_received + 1);
      while(numNewMessagesHora) {
      Serial.println("Veo los msj nuevos");
      handleNewMessagesHora(numNewMessagesHora);
      numNewMessagesHora = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = now;
  
      }
    
     break;
      
     case MINUTOS:
      bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), ""); 
      Serial.println(miArray[casillero].minAlarma); 
      
      if(esperandoRespuesta == false){
        Serial.println("MINUTOS"); 
        bot.sendMessage(CHAT_ID, "Para aumentar los minutos en 5, ingrese (/Min mas), para disminuirlos en 5, ingrese (/Min menos), para Continuar ingrese (/Confirmar min)", "");
        esperandoRespuesta = true;
      }
      
      now = millis(); 
       if ( now -lastTimeBotRan >  intervalo) {
        Serial.println("FUNCION MIN");
        int numNewMessagesMin = bot.getUpdates(bot.last_message_received + 1);
        while(numNewMessagesMin) {
        Serial.println("Veo los msj nuevos");
        handleNewMessagesMin(numNewMessagesMin);
        numNewMessagesMin = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = now;
  
      } 
      break;

     case CONFIRMAR:
     Serial.println(miArray[casillero].minAlarma); 
     
     if(esperandoRespuesta == false){
       Serial.println("CONFIRMAR"); 
       bot.sendMessage(CHAT_ID, String(miArray[casillero].horaAlarma ), "");
       bot.sendMessage(CHAT_ID, String(miArray[casillero].minAlarma ), "");
       bot.sendMessage(CHAT_ID, "Estos son los datos ingresados, ¿Confirmar?", "");
       bot.sendMessage(CHAT_ID, "Para aplicar los valores de la alarma ingrese (/Confirmar), en caso de querer modificar la hora, ingrese (/Hora), en caso de querer modificar los minutos, ingrese (/Min), en caso de querer modificar el numero de casillero, ingrese (casillero)", "");
       esperandoRespuesta = true;
     }
    
     now = millis(); 
       if ( now -lastTimeBotRan >  intervalo) {
        Serial.println("FUNCION CONFIRMAR");
        int numNewMessagesConfirmar = bot.getUpdates(bot.last_message_received + 1);
        while(numNewMessagesConfirmar) {
        Serial.println("Veo los msj nuevos");
        handleNewMessagesConfirmar(numNewMessagesConfirmar);
        numNewMessagesConfirmar = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = now;
  
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
