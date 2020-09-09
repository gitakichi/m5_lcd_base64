#include <M5StickC.h>               // M5StickCの読み込み
#include <WiFi.h>
#include <PubSubClient.h>
#include "secret.h"//WIFIとMQTTのパスワード等を記載

const uint16_t imgWidth = 60;//#include "data.h"
const uint16_t imgHeight = 49;
// クライアントIDをランダム生成するための文字列
static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void reConnect();
int base64_to_6bit();
int base64Decode_ppm();

int flag = 0;
unsigned short fixed[3000];
byte *received;//byte received[15000];
int received_len;

void setup() {
  //Serial.println("boot");
  M5.begin();
  M5.Axp.ScreenBreath(12);          // 7-12で明るさ設定
  M5.Lcd.setRotation(3);            // 0-3で画面の向き
  M5.Lcd.setSwapBytes(true);        // スワップON(色がおかしい場合には変更する)


  Serial.print("WIFI Connecting ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" CONNECTED");

  mqttClient.setServer(server, 1883);
  mqttClient.setBufferSize(15000);
  /*
    base64Decode_ppm(b64_str, fixed);
    M5.Lcd.startWrite();// 描画開始(明示的に宣言すると早くなる)
    M5.Lcd.pushImage(0, 0, imgWidth, imgHeight, fixed);
    M5.Lcd.endWrite();
  */
}

void loop() {
  reConnect();
  mqttClient.loop();

  if (flag == 1) {
    flag = 0;
    base64Decode_ppm(received, fixed, received_len);
    M5.Lcd.startWrite();// 描画開始(明示的に宣言すると早くなる)
    M5.Lcd.pushImage(0, 0, imgWidth, imgHeight, fixed);
    M5.Lcd.endWrite();
  }
}

void reConnect() { // 接続が切れた際に再接続する
  char clientID[10];

  while (!mqttClient.connected()) {// 接続まで繰り返す
    Serial.print("MQTT Connecting ");
    // Generate ClientID
    for (int i = 0; i < 8; i++) {
      clientID[i] = alphanum[random((sizeof(alphanum)))];
    }

    // MQTT broker に接続する
    if (mqttClient.connect(clientID, username, NULL)) {
      Serial.println(" CONNECTED");
      mqttClient.subscribe(topic);
      mqttClient.setCallback(callback);
    }
    else {//error and retry
      Serial.print(".");
      delay(5000);
    }
  }
}

//HEXダンプとBase64の両方に対応させたい
//http://yamatyuu.net/computer/program/vc2013/base64/index.html
inline int base64_to_6bit(char c) {
  if (c == '=')
    return 0;
  if (c == '/')
    return 63;
  if (c == '+')
    return 62;
  if (c <= '9')
    return (c - '0') + 52;
  if ('a' <= c)
    return (c - 'a') + 26;
  return (c - 'A');
}

int base64Decode_ppm(byte* src, unsigned short *dtc, int src_len) {
  char o0, o1, o2, o3;
  char h0, h1, h2, r_color, g_color, b_color;
  int i = 0 ;
  while (*src != '\0' && i < src_len) {
    //Base64 to HEX
    o0 = base64_to_6bit(*src);
    o1 = base64_to_6bit(*(src + 1));
    o2 = base64_to_6bit(*(src + 2));
    o3 = base64_to_6bit(*(src + 3));

    h0 = (o0 << 2) | ((o1 & 0x30) >> 4);
    h1 = ((o1 & 0xf) << 4) | ((o2 & 0x3c) >> 2);
    h2 = ((o2 & 0x3) << 6) | (o3 & 0x3f);

    //HEX to RGB565
    r_color = h0 >> 3;
    g_color = h1 >> 2;
    b_color = h2 >> 3;
    *dtc = r_color << 11 | g_color << 5 | b_color;

    //address increment
    dtc++;
    src += 4;
    i++;
  }
  return 0;
}

// メッセージを受け取ったらシリアルにプリント
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.println(length);
  received_len = length;
  /*
  for (int i = 0; i < length; i++) { //　メッセージを表示
    //Serial.print((char)payload[i]);
    *(received+i) =*(payload+i);
  }
  //Serial.print("\n");
  */
  received = payload;
  flag = 1;
  
}
