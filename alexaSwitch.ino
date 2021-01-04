#include <WiFi.h>
#include "M5Atom.h"
#include "fauxmoESP.h"

const int RelayPin = 32;
const char DEVICE_NAME[] = "mySwitch";

// Wi-Fi
const char* ssid = "";
const char* password = "";

typedef enum {
  NO_EVENT = 0,
  SW_ON,
  SW_OFF
} EVENT;

volatile EVENT event = NO_EVENT;

fauxmoESP fauxmo;

// setup code
void setup() {

  // リレーの初期化
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);

  // M5の初期化
  M5.begin(true, false, true);
  delay(50);
  M5.dis.drawpix(0, 0x000000);

  // WiFiの初期化
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());

  // Alexa用の初期化
  fauxmo.createServer(true);
  fauxmo.setPort(80);
  fauxmo.enable(true);

  // デバイス名を登録
  fauxmo.addDevice(DEVICE_NAME);

  // コールバック関数を登録
  fauxmo.onSetState(AlexaCallback);
}

// コールバック関数 登録したデバイスにAlexaからイベント来ると呼ばれる
void AlexaCallback(unsigned char device_id, const char *device_name, bool state, unsigned char value) {

  // コールバックに応じてイベント更新
  if (strcmp(device_name, DEVICE_NAME) == 0) {
      if(state) {
        event = SW_ON;
      } else {
        event = SW_OFF;
      }
  }

}

void loop() {

  // スイッチの状態
  static bool state = false;

  // 本体のボタン操作
  if (M5.Btn.wasPressed()) {
    if(state) {
      event = SW_OFF;
    } else {
      event = SW_ON;
    }
  }

  // Alexaからのイベント監視
  switch (event) {
    case SW_ON:
      LightControl(true);
      state = true;
      event = NO_EVENT;
      break;
    case SW_OFF:
      LightControl(false);
      state = false;
      event = NO_EVENT;
      break;
    default:
      break;
  }

  fauxmo.handle();
  delay(50);
  M5.update();
}


// コントロールON / OFF
void LightControl(bool value) {
  if(value){
    Serial.println("ON");
    M5.dis.drawpix(0, 0xf00000);
    digitalWrite(RelayPin, HIGH);
  } else {
    Serial.println("OFF");
    M5.dis.drawpix(0, 0x000000);
    digitalWrite(RelayPin, LOW);
  }
}
