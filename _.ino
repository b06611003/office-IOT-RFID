#include <SPI.h>
#include <MFRC522.h>     // 引用程式庫
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;
#define RST_PIN      D2        // 讀卡機的重置腳位
#define SS_PIN       D4        // 晶片選擇腳位

int speakerPin = D0;
int tempo = 290;
int youare = 0;
String arrays[3] = {"16541", "asdf", "gedec"};
int arraysize = 3;
int inorout[10];
MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件
char notebad[] = "n n";
unsigned long beatsbad[] = {1, 1, 1};
int lengthbad = sizeof(notebad);
char notegood[] = "cegC";
unsigned long beatsgood[] = {2, 2, 2, 2};
int lengthgood = sizeof(notegood);
char noteout[] = "Cgec";
unsigned long beatsout[] = {2, 2, 2, 2};
int lengthout = sizeof(noteout);


void setup() {
  Serial.begin(115200);
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Lalalalala", "1234567890");

  SPI.begin();
  mfrc522.PCD_Init();   // 初始化MFRC522讀卡機模組

  for (int o = 0; o < arraysize ; o++) {
    inorout[o] = 0;
  }
}

void loop() {
  // 確認是否有新卡片
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
    byte idSize = mfrc522.uid.size;   // 取得UID的長度
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

    String thisid = "";
    for (byte i = 0; i < idSize - 2; i++) { // 逐一顯示UID碼
      thisid += id[i];
    }
    Serial.print("read this card id is = ");
    Serial.println(thisid);
    int i = 0;
    for (int a = 0; a < arraysize; a++) {

      Serial.print("comparing......");
      if (thisid.equals(arrays[a])) {
        youare = a;
        Serial.print("match in ");
        Serial.println(a);
        goto abc;
      }
      else {
        Serial.println("i++");
        i = i + 1;
      }
    }
    if (i == arraysize) {
      Serial.print("no");
      for (int i = 0; i < lengthbad; i++) {
        if (notebad[i] == ' ') {
          delay(beatsbad[i] * tempo);
        }
        else {
          playNote(speakerPin, notebad[i], beatsbad[i] * tempo);
        }
        delay(tempo / 10);
      }
    }
    else {

abc:
      Serial.println("in abc");
      if (inorout[youare] == 0) {
        Serial.println("good");
        for (int i = 0; i < lengthgood; i++) {
          if (notegood[i] == ' ') {
            delay(beatsgood[i] * tempo);
          }
          else {
            playNote(speakerPin, notegood[i], beatsgood[i] * tempo);
          }
          delay(tempo / 10);
        }
        inorout[youare] = 1;
      }
      else {
        Serial.println("out");
        for (int i = 0; i < lengthout; i++) {
          if (noteout[i] == ' ') {
            delay(beatsout[i] * tempo);
          }
          else {
            playNote(speakerPin, noteout[i], beatsout[i] * tempo);
          }
          delay(tempo / 10);
        }
        inorout[youare] = 0;
      }

      mfrc522.PICC_HaltA();  // 讓卡片進入停止模式
      if ((WiFiMulti.run() == WL_CONNECTED)) {

        WiFiClient client;

        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        String upweb = "http://api.pushingbox.com/pushingbox?devid=vFCE4C7337179079&uid=";
        upweb += thisid;
        if (http.begin(client, upweb)) {  // HTTP


          Serial.print("[HTTP] GET...\n");
          // start connection and send HTTP header
          int httpCode = http.GET();

          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
              String payload = http.getString();
              Serial.println(payload);
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }

          http.end();
        } else {
          Serial.printf("[HTTP} Unable to connect\n");
        }
      }
    }
  }
}
void playNote(int OutputPin, char note, unsigned long duration) {
  char names[] = { 'n', 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 247, 261, 294, 330, 349, 392, 440, 494, 523 };
  for (int i = 0; i < 9; i++) {
    if (names[i] == note) {
      tone(OutputPin, tones[i], duration);
      delay(duration);
      noTone(OutputPin);
    }
  }
}
