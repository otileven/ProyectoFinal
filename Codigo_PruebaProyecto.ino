//Librerías
        
#include <Wire.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <U8g2lib.h>

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

int horaDeRegado = 10;
int minutosDeRegado = 35;
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

//Inicializar rtc
int gmt;
struct tm timeinfo;
ESP32Time rtc;

//Reloj
long unsigned int timestamp; 
const char *ntpServer = "south-america.pool.ntp.org";
long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

//WiFi
const char* ssid = "ORT-IoT";           
const char* password = "OrtIOTnew22$2";

//Constructores y variables globales
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Funciones
void pedir_lahora(void);
void setup_rtc_ntp(void);
void initWiFi();

//Variables
bool contadorLed = false;

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
  initWiFi(); 
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
      if(digitalRead(PIN_BOTON_2) == LOW)
      {
        estadoBoton2 = true;
        Serial.println("BOTON_PRESIONADO");
      }
      if(digitalRead(PIN_BOTON_2) == HIGH && estadoBoton2 == true)
      {
        estadoBoton2 = false;
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
          horaDeRegado = 0;
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
  



//Función conexión WiFi
void initWiFi() 
{                                         
  WiFi.begin(ssid , password );
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
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
