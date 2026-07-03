#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const int MPU = 0x68;
int in1 = 14;
int in2 = 12;
int in3 = 13;
int in4 = 15;
int ledPin = 2;
int buzzerPin = 0;

const char* ssid     = "Orange-ys49-2.4G";
const char* password = "7949WfRe";

ESP8266WebServer server(80);

bool mpuOk = false;  //retine daca mpu a fost gasit la pornire
bool motoreleOprite = false; //Flag care previne apelarea repetată a motorOprit() și tone() la fiecare iterație.
bool ledState = false;
unsigned long previousMillis = 0;

// Contor somnolenta - se incrementeaza la fiecare tranzitie inclinat->drept
int contor = 0;
bool eraInclinat = false;

void motorPornit() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void motorOprit() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}

void setupWiFi() {
  WiFi.begin(ssid, password); //pornește procesul de conectare la rețea
  Serial.print("Conectare WiFi");
  int tries = 0; //contor încercări
  while (WiFi.status() != WL_CONNECTED && tries < 20) { //bucla de asteptare
    delay(500);  //verifica la fiecare 500ms daca s-a conectat, de max 20 de ori, 10 sec in total
    Serial.print(".");
    tries++;
  }
  //wifi.status returneaza starea curenta
  //WL_CONNECTEDconectat ✓
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectat! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi esuat, continua fara retea.");
  }
}

void setupServer() {
  server.on("/ping", HTTP_GET, []() { //definirea rutelor, parametri sunt ruta-ping adresa ip-, metoda-http citire doar- , functie
    server.sendHeader("Access-Control-Allow-Origin", "*");
    //CORS=Cross Origin Resource Sharing, browserul blocheaza implicit cererile catre alte adrese IP decat cea a paginii
    server.send(200, "text/plain", "pong");
    //trimite rasp catre browser , 200-cod HTPP succes-, tip continut, pong -corpul raspunsului, ce primeste browserul
  });

  server.on("/count", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", String(contor));
  });
///pingpongbutonul Test din HTML verifică dacă ESP e online
// count - HTML ul citeste countul la fiecare 500ms
  server.begin(); //porneste serverul
  Serial.println("Server HTTP pornit.");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5);
  Wire.setTimeout(1000);

  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  motorOprit();
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  byte err = Wire.endTransmission(true);                            // ELIBEREAZA MAGISTRALA

  if (err == 0) {
    mpuOk = true;
    Serial.println("MPU6050 gata!");
  } else {
    Serial.println("EROARE: MPU6050 negasit!");
  }

  setupWiFi();
  setupServer();
}

void loop() {
  ESP.wdtFeed();  //sunt viu, nu restarта
  server.handleClient(); // proceseaza cererile HTTP, VERIFICA DACA BROWSERUL A CERUT CEVA

  if (!mpuOk) {
    delay(500);
    return;
  }

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  byte err = Wire.endTransmission(false); // PASTREAZA MAGISTRALA OCUPATA

  if (err != 0) {
    motorOprit();
    noTone(buzzerPin);
    digitalWrite(ledPin, LOW);
    delay(200);
    return;
  }

  Wire.requestFrom(MPU, 6, true);

  if (Wire.available() < 6) { // ESTE O VERIFICARE CA SA VEDEM DACA BUFFERUL ARE DESTUI BYTES
    motorOprit();            // PRACTIC DACA NU ARE DATE SENZOR, NU FAC NIMIC SI INCERC DIN NOU LA URM ITERATIE
    noTone(buzzerPin);       // DE SIGURANTA OPRESTE MOTOARELE, STINGE LED, OPRESTE BUZZER
    digitalWrite(ledPin, LOW);
    delay(200); 
    return; //SARE PESTE RESTUL LOOP-ULUI
  }

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();

  float accelX = ax / 16384.0;
  float accelY = ay / 16384.0;

  float prag = 0.8;
  bool inclinat = (accelX > prag || accelX < -prag || accelY > prag || accelY < -prag);

//eraInclinat reține starea din iterația anterioară, inclinat e starea curentă.
  // Incrementeaza contorul la tranzitia inclinat -> drept (sofer si-a revenit)
  if (eraInclinat && !inclinat) {
    contor++;
    Serial.println("Somnolenta detectata! Contor: " + String(contor));
  }
  eraInclinat = inclinat;

  unsigned long currentMillis = millis();

  if (inclinat) {
    motorPornit();
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
    ledState = false;
    motoreleOprite = false; 
    Serial.println("Motoare: PORNITE");
  } else {
    if (!motoreleOprite) {
      motorOprit();
      tone(buzzerPin, 1000);
      motoreleOprite = true;
      Serial.println("Motoare: OPRITE");
    }
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis; 
      ledState = !ledState; 
      digitalWrite(ledPin, ledState ? HIGH : LOW); 
    } 
  }
//resetează flag-ul ca data viitoare când devine drept, motoarele să se oprească din nou corect
    //se repeta la fiecare 1000ms, numara incontinuu ms de la pornire esp  
//logica comparatiei, masuram mereu diferenta fata de ultima schimbare
//se sc himba state ul led ului  
//se aplica pe pin
//practic fac semnalizarile sa porneasca 1s , sa se opreasca 1s si tot asa
  delay(200);
}