#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

#define PRINT_DEBUG_MESSAGES
//#define WiFi_CONNECTION_UPV
#define REST_SERVER_THINGSPEAK

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "SaforNet_FXKDA";
  const char WiFiPSK[] = "5YG8P5N4";
#endif

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="5J9621RKR9BMMYXY"; // Escribe la clave de tu canal ThingSpeak
#endif

#define NUM_FIELDS_TO_SEND 5 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)
const int LED_PIN = 5; // Thing's onboard, green LED

#define power_pin 5  // Pin para alimentar el sensor de salinidad

#define Offset -2.81
#define samplingInterval 20
#define printInterval 800
#define ArrayLength 40 // Número de muestras 


Adafruit_ADS1115 ads;  // Constructor del objeto ads1115 con dirección 0x48

void setup() {
  #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(9600);
  #endif  // Inicializamos la comunicación serie (UART) a 9600 baudios
  ads.begin();
  ads.setGain(GAIN_ONE);  // Ajustamos la ganancia (2/3x)

  connectWiFi();

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif
}

void connectWiFi() {
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

void HTTPPost(String fieldData[], int numFields){

    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

void HTTPGet(String fieldData[], int numFields){
  
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}

// Función para calcular el promedio de las muestras
float averageSample(int TempArrayLength, int16_t *samples) {
  long sum = 0;
  for (int i = 0; i < TempArrayLength; i++) {
    sum += samples[i];
  }
  return static_cast<float>(sum) / TempArrayLength;
}

int midehumedad(int channel) {
  // Leemos el valor del ADC del canal de humedad especificado (CHANNEL_HUMIDITY) a través del ADS1115
  int16_t sensorValue = ads.readADC_SingleEnded(channel);

  // Mapeamos el valor leído para obtener el valor de humedad (ajusta estos valores según tus necesidades)
  int humidityValue = map(sensorValue, 15520, 3140, 0, 100);  // Corregido el rango de mapeo

  Serial.println(sensorValue, DEC);

  return humidityValue;
}

float midetemperatura(int channel) {
  int TempArrayLength2 = 10;  // Número de muestras a promediar (ajusta según tus necesidades)
  int16_t adcSamples[TempArrayLength2];

  // Realiza las lecturas y almacena las muestras en el arreglo adcSamples
  for (int i = 0; i < TempArrayLength2; i++) {
    adcSamples[i] = ads.readADC_SingleEnded(channel);
    delay(100);  // Puedes ajustar este valor según sea necesario
  }

  // Calcula el valor promedio de las muestras utilizando la función averageSample
  float averageVoltage = averageSample(TempArrayLength2, adcSamples);
  float volt = (averageVoltage * 4.096) / 33000;

  // Calcula la temperatura utilizando la fórmula con el valor promedio
  float Temp = (volt - 0.79) / 0.035;

  return Temp;
}

float midePH(int channel) {

  int phArray[ArrayLength]; // almacena las muestras 
  int phArrayIndex = 0;

  int16_t adc0 = ads.readADC_SingleEnded(channel);
  phArray[phArrayIndex++] = adc0;

  if (phArrayIndex == ArrayLength) phArrayIndex = 0;

    // Convertimos la lectura a voltaje
  float voltage = adc0 * 0.1875 / 1000.0; // 0.1875 mv por valor en el ADC, 1000 para convertir a voltios
  float valorPH = 3.5 * voltage + Offset;

  return valorPH;

}

float measureSalinity() {
  int16_t adcSalinity;
  digitalWrite(power_pin, HIGH);
  delay(100);
  adcSalinity = analogRead(A0);
  digitalWrite(power_pin, LOW);
  delay(100);

  float Salinidad = map(adcSalinity, 76, 89, 0, 25);
  

  //Serial.println(adcSalinity, DEC);

  return Salinidad;
}

int measureLuminosity(int channel) {

  const int umbralOscuridad = 45; 
  const int umbralSombra = 55;
  const int umbralLuzAmbiente = 92;
  const int umbralLuzDelMovil = 700;
  const int umbralFoco = 5000;

  int lectura = ads.readADC_SingleEnded(channel);

  //Serial.print("Valor leído: ");
  //Serial.println(lectura);
  
  // Compara la lectura con los umbrales y muestra el nivel de luz
  if (lectura < umbralOscuridad) {
    Serial.println("Nivel de luz: Oscuridad");
  } else if (lectura < umbralSombra) {
    Serial.println("Nivel de luz: Sombra");
  } else if (lectura < umbralLuzAmbiente) {
    Serial.println("Nivel de luz: Luz ambiente");
  } else if (lectura < umbralLuzDelMovil){
    Serial.println("Nivel de luz: Luz del movil");
  } else if (lectura > umbralFoco){
    Serial.println("Nivel de luz: Nivel Foco");
  }
  
  delay(15000);

  return lectura;

}

void loop() {

  int humidityValue = midehumedad(2);  // Corregido el nombre de la variable
  float temperatureValue = midetemperatura(1);  // Corregido el nombre de la variable
  float PHValue = midePH(0);
  float salinityValue = measureSalinity();
  int LuminosityValue = measureLuminosity(3);

  // Imprime el valor leído en el monitor serial
  //Serial.print("Humedad: ");
 // Serial.println(humidityValue);  // DEC para mostrar la salida en decimal

  // Imprime la temperatura calculada
  //Serial.print("Temperatura: ");
  //Serial.println(temperatureValue);  // Corregida la variable y ajuste de la temperatura

  //Serial.print("PH: ");
  //Serial.println(PHValue);  // DEC para mostrar la salida en decimal

  //Serial.print("Salinidad: ");
  //Serial.println(salinityValue);

  //measureLuminosity(3);

  //delay(1000);

  String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    data[ 1 ] = humidityValue; //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.println( data[ 1 ] );
    #endif

    data[ 2 ] = temperatureValue; //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Temperatura = " );
        Serial.println( data[ 2 ] );
    #endif

    data[ 3 ] = salinityValue; //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.println( data[ 3 ] );
    #endif

    data[ 4 ] = PHValue; //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "PH = " );
        Serial.println( data[ 4 ] );
    #endif

    data[ 5 ] = LuminosityValue; //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Luminosidad = " );
        Serial.println( data[ 5 ] );
    #endif

    HTTPGet( data, NUM_FIELDS_TO_SEND );

    delay(1000); 
}
