
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

//Constructores y variables globales
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Inicializar rtc
int gmt;
struct tm timeinfo;
ESP32Time rtc;

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
const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";


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
  {11, 30, 0},
  {23, 20, 0},
};

LED ledsArray [2]{
  {25, LOW, false},
  {26, LOW, false},
};

int menorTiempo = 1440;
int siguienteAlarma = 0;


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
    char stringhoras[7];  
    char stringminutos[7];
    delay (1000);
      case ALARMA:
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
         Serial.println("Tiempo Restante:" + String(casilleroArray[casillero].tiempoRestante));
         if(casilleroArray[casillero].tiempoRestante <= 0){
          Serial.println("ES NEGATIVO");
          casilleroArray[casillero].tiempoRestante = (casilleroArray[casillero].horaAlarma + 24 - timeinfo.tm_hour)*60 + casilleroArray[casillero].minAlarma - timeinfo.tm_min;
          Serial.println(casilleroArray[casillero].tiempoRestante);
          if(casilleroArray[casillero].tiempoRestante <= menorTiempo){
           Serial.println("AHORA ES POSITIVO");
            siguienteAlarma = casillero;
            menorTiempo = casilleroArray[casillero].tiempoRestante;
            Serial.println(menorTiempo);
         }
         }
         else if(casilleroArray[casillero].tiempoRestante <= menorTiempo){
                    Serial.println("ES POSITIVO");
            siguienteAlarma = casillero;
            menorTiempo = casilleroArray[casillero].tiempoRestante;
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
           bot.sendMessage(CHAT_ID, "Alerta! Pastilla no consumida.\n ", "");
           menorTiempo = 1440;
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
           bot.sendMessage(CHAT_ID, "Pastilla consumida con éxito.\n ", "");
           menorTiempo = 1440;
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
