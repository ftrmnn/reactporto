#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

char auth[] = "rxSRihcZBhrvvL-lxkUF6a4Bdhha5agv"; // Masukkan token Blynk Anda
char ssid[] = "3344"; // Masukkan nama jaringan WiFi Anda
char pass[] = "123454321"; // Masukkan kata sandi WiFi Anda

Servo servo;
int servoPin = D3; // Pin motor servo
const int trigPin = D1; // Pin Trig sensor ultrasonik
const int echoPin = D2; // Pin Echo sensor ultrasonik
const int ledPinServo = D4; // Pin LED fisik untuk motor servo
const int ledPinDistance = D5; // Pin LED fisik untuk sensor ultrasonik
const int ledPinFull = D6; // Pin LED fisik untuk jarak dalam kisaran 100% hingga 20%

WidgetLED led(V1); // LED di aplikasi Blynk untuk menampilkan status motor servo
WidgetLED distanceLed(V2); // LED di aplikasi Blynk untuk menampilkan status pengukuran jarak
WidgetLED servoLed(V3); // LED di aplikasi Blynk untuk menampilkan status motor servo

bool servoState = false; // Status awal servo (mati)
unsigned long servoStartTime = 0; // Waktu saat servo dinyalakan
unsigned long servoDuration = 0; // Durasi hidup servo
unsigned long servoAutoOffDelay = 500; // Waktu penundaan untuk mematikan servo secara otomatis (dalam milidetik)

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "iot.smk2-yk.sch.id", 8080);
  servo.attach(servoPin);
  
  // Menetapkan posisi awal servo
  servo.write(0); // Mengatur posisi awal servo ke 0 derajat
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPinServo, OUTPUT);
  pinMode(ledPinDistance, OUTPUT);
  pinMode(ledPinFull, OUTPUT);
  
  led.off(); // Matikan LED di Blynk pada awalnya
  distanceLed.off(); // Matikan LED di Blynk pada awalnya
  servoLed.off(); // Matikan LED di Blynk pada awalnya
}

void loop() {
  Blynk.run();
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  int distancePercent = map(distance, 0, 26, 100, 0); // Mengubah jarak menjadi persentase

  Blynk.virtualWrite(V4, distancePercent); // Mengirim data persentase jarak ke widget di Blynk
  
 // Cek apakah servo dalam keadaan hidup atau mati
  if (servoState) {
    if (millis() - servoStartTime > 500) { // Check if 3 seconds have passed
      servoState = false; // Turn off the servo
      led.off(); // Matikan LED di Blynk untuk menunjukkan bahwa servo mati
      servoLed.off(); // Matikan LED di Blynk untuk menunjukkan bahwa servo mati
      digitalWrite(ledPinServo, LOW); // Matikan LED fisik untuk motor servo
      servo.write(0); // Matikan servo
    } else {
      servo.write(60); // Jika servo hidup, putar servo ke posisi tengah (90 derajat)
      digitalWrite(ledPinServo, HIGH); // Nyalakan LED fisik untuk motor servo
    }
  } else {
    digitalWrite(ledPinServo, LOW); // Matikan LED fisik untuk motor servo
  }
  
  // Menguji jika jarak kurang dari 10% dan menyalakan LED fisik untuk sensor ultrasonik
  if (distancePercent < 10) {
    digitalWrite(ledPinDistance, HIGH); // Nyalakan LED fisik untuk sensor ultrasonik
    delay(500); // Tunda selama 500ms
    digitalWrite(ledPinDistance, LOW); // Matikan LED fisik untuk sensor ultrasonik
    delay(500); // Tunda selama 500ms
  } else {
    digitalWrite(ledPinDistance, LOW); // Matikan LED fisik untuk sensor ultrasonik
  }

  // Menguji jika jarak dalam kisaran 100% hingga 10% dan menyalakan LED fisik untuk kisaran tersebut
  if (distancePercent >= 10 && distancePercent <= 100) {
    digitalWrite(ledPinFull, HIGH); // Nyalakan LED fisik untuk jarak dalam kisaran 100% hingga 20%
  } else {
    digitalWrite(ledPinFull, LOW); // Matikan LED fisik untuk jarak dalam kisaran 100% hingga 10%
  }
}

BLYNK_WRITE(V1) { // Mendeteksi perubahan pada tombol di Blynk untuk mengontrol motor servo
  int pinValue = param.asInt(); // Mendapatkan nilai tombol (1 = aktif, 0 = tidak aktif)
  
  if (pinValue == HIGH) { // Jika tombol ditekan
    servoState = true; // Aktifkan servo
    servoStartTime = millis(); // Catat waktu saat servo dinyalakan
    led.on(); // Nyalakan LED di Blynk untuk menunjukkan bahwa servo aktif
    servoLed.on(); // Nyalakan LED di Blynk untuk menunjukkan bahwa servo aktif
  } else { // Jika tombol dilepas
    servoState = false; // Matikan servo
    led.off(); // Matikan LED di Blynk untuk menunjukkan bahwa servo mati
    servoLed.off(); // Matikan LED di Blynk untuk menunjukkan bahwa servo mati
  }
}

BLYNK_WRITE(V5) { // Mendeteksi perubahan pada timer di Blynk
  int timerValue = param.asInt(); // Mendapatkan nilai timer (1 = aktif, 0 = tidak aktif)
  
  if (timerValue == HIGH) { // Jika timer diaktifkan
    // Atur durasi hidup servo (dalam milidetik)
    servoDuration = 500; // Durasi 1 detik
    // Mulai servo
    servoState = true;
    servoStartTime = millis(); // Catat waktu mulai servo
    servo.write(60); // Putar servo ke posisi tengah
    // Nyalakan LED servo di Blynk
    led.on();
    servoLed.on();
  } else { // Jika timer dimatikan
    // Matikan servo
    servoState = false;
    servo.write(0);
    // Matikan LED servo di Blynk
    led.off();
    servoLed.off();
  }
}
