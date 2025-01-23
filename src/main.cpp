#include <Arduino.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <SPI.h>

#define eprom_hour 0
#define eprom_minutes 1

RTC_DS3231 rtc;
SoftwareSerial mySerial(3,2);  // TX na pin 3, RX na pin 2

String username = "admin";
String password = "password";
int num1 = 0;
int num2 = 0;

void setup()
{
  
  Serial.begin(9600);
  mySerial.begin(9600);
  rtc.begin();
  if(EEPROM.read(eprom_hour)==255){
    EEPROM.write(eprom_hour,10);
  }
    if(EEPROM.read(eprom_minutes)==255){
    EEPROM.write(eprom_minutes,00);
  }

  Serial.println("Inicjalizacja SIM800L...");

  mySerial.println("AT");          // Pierwsze pytanie
  delay(500);
  while (mySerial.available()) {
    mySerial.readString();  // Ignorujemy odpowiedzi, czekamy na kolejne zapytanie
  }

  mySerial.println("AT+CFUN=1");   // Ustaw pełną funkcjonalność
  delay(1000);
  while (mySerial.available()) {
    mySerial.readString();  // Ignorujemy odpowiedzi
  }

  mySerial.println("AT+CMGF=1");   // Ustaw tryb tekstowy SMS
  delay(500);
  while (mySerial.available()) {
    mySerial.readString();  // Ignorujemy odpowiedzi
  }

  mySerial.println("AT+CSCS=\"GSM\"");   // Ustaw kodowanie SMS
  delay(500);
  while (mySerial.available()) {
    mySerial.readString();  // Ignorujemy odpowiedzi
  }

  mySerial.println("AT+CNMI=2,2,0,0,0");   // Powiadomienia o nowych SMS-ach
  delay(500);
  while (mySerial.available()) {
    mySerial.readString();  // Ignorujemy odpowiedzi
  }

  Serial.println("Moduł zainicjalizowany.");
}



void loop()
{


  DateTime now = rtc.now();


  if (mySerial.available()) {


    String response = mySerial.readString(); // Odczyt pełnej odpowiedzi z SIM800L
    Serial.println("Otrzymano wiadomość:");
    Serial.println(response);

    // int hour = response.indexOf();
    // int minutes;
    // int seconds;


    int smsStart = response.indexOf("\r\n") + 2; // Znajdź początek wiadomości
    smsStart = response.indexOf("\r\n", smsStart) + 2; // Przejdź do kolejnej linii (zawiera treść SMS-a)
    int smsEnd = response.lastIndexOf("\r\n");  // Znajdź koniec wiadomości
    String SMS = response.substring(smsStart,smsEnd);

    int hour_id = response.indexOf(",");
    hour_id = response.indexOf(",",hour_id+1);
    hour_id = response.indexOf(",",hour_id+1);
    int minutes_id = response.indexOf(":",hour_id);
    int seconds_id = response.indexOf(":",minutes_id+1);

    String hour_sms = response.substring(hour_id+1,hour_id+3);
    String minutes_sms = response.substring(minutes_id+1,minutes_id+3);
    String seconds_sms = response.substring(seconds_id+1,seconds_id+3);

    Serial.println("UWAGA GODZINA SMS");
    Serial.print(hour_sms);
    Serial.print(":");
    Serial.print(minutes_sms);
    Serial.print(":");
    Serial.println(seconds_sms);

    hour_id = hour_sms.toInt();
    minutes_id = minutes_sms.toInt();
    seconds_id = seconds_sms.toInt();

    if(now.hour() != hour_id || now.minute() != minutes_id || now.second() != seconds_id){
      int id = response.indexOf(",");
      id = response.indexOf(",",id);
      String year = response.substring(id+2,id+4);
      String month = response.substring(id+5,id+7);
      String day = response.substring(id+8,id+10);
      int year_int = year.toInt();
      int month_int = month.toInt();
      int day_int = day.toInt();
      DateTime newTime(year_int,month_int,day_int,hour_id, minutes_id, seconds_id);  //tutaj musisz dodac rok miesiac i dzien dzisiaj mi juz glowa paruje :( zrob to tym substringiem i zmienne w petli zeby od razu sie usuwaly okok??? dzk
      rtc.adjust(newTime);
    }


    int first_end = SMS.indexOf(";");
    int second_end = SMS.indexOf(";",first_end+1);
    if (first_end == -1 || second_end == -1) {
      Serial.println("Błędny format SMS");
      return;
    }



    String username_sms = SMS.substring(0, first_end);
    String password_sms = SMS.substring(first_end+1, second_end);
    String number_sms = SMS.substring(second_end+1, smsEnd);
    int correct_data = number_sms.length();
    if (correct_data != 4) {
      Serial.println("Błędny format godziny, zapisz ją w formacie : GGMM");
      return;
    }


    Serial.print("Login: ");
    Serial.println(username_sms);
    Serial.print("Hasło: ");
    Serial.println(password_sms);
    Serial.print("Liczba: ");
    Serial.println(number_sms);


    Serial.print("Login: ");
    Serial.println(username);
    Serial.print("Hasło: ");
    Serial.println(password);
    Serial.print("Godzina: ");
    Serial.print(EEPROM.read(eprom_hour));
    Serial.print(":");
    Serial.println(EEPROM.read(eprom_minutes));


    if(username_sms == username && password_sms == password){
      String temp = number_sms.substring(0,2);
      num1 = temp.toInt();
      temp = number_sms.substring(2,4);
      num2 = temp.toInt();
      if(num1<0 || num1>24 || num2<0 || num2>60){
        Serial.println("Zła godzina, nie zapisano");
        return; 
      }
      else{
      Serial.println("Sukces! Godzina została zmieniona!");
      Serial.print("Nowa godzina: ");
      EEPROM.update(eprom_hour,num1);
      EEPROM.update(eprom_minutes,num2);
      Serial.print(EEPROM.read(eprom_hour));
      Serial.print(":");
      Serial.println(EEPROM.read(eprom_minutes));
      }
    }
  }

  if (now.hour() < 10) Serial.print("0");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second(), DEC);
}