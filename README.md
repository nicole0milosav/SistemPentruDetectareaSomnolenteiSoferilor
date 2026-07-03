# SistemPentruDetectareaSomnolenteiSoferilor

I. Aplicații necesare
Arduino IDE cu suport placă ESP8266 instalat în Arduino IDE 

II. Compilare cod
1. Deschizi fișierul `licentaVariantaFinala.ino` din Arduino IDE
2. Editezi în cod `ssid` și `password` cu datele rețelei tale WiFi
3. Tools → Board → alegi placa ta (ex: "NodeMCU 1.0 (ESP-12E Module)")

III. Rulare
1. După încărcare, deschizi Serial Monitor (baud 115200) ca să vezi adresa IP primită de ESP8266
2. Deschizi fișierul `site(3).html` în browser
3. Introduci user-ul și parola (modificabi-le din fișierul html, setate 'username@gmail.com' cu parola '123')
4. Introduci acea adresă IP în câmpul de pe pagină 
5. Dashboard-ul începe să afișeze automat  numărul de alerte
