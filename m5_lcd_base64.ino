#include <M5StickC.h>               // M5StickCの読み込み

// イメージデータ
#include "data.h"                   // 画像データの読み込み

int base64_to_6bit();
int base64Decode_ppm();

void setup() {
  //Serial.println("boot");
  M5.begin();
  M5.Axp.ScreenBreath(12);          // 7-12で明るさ設定
  M5.Lcd.setRotation(3);            // 0-3で画面の向き
  M5.Lcd.setSwapBytes(true);        // スワップON(色がおかしい場合には変更する)

  unsigned short fixed[2940];
  base64Decode_ppm(b64_str, fixed);

  M5.Lcd.startWrite();// 描画開始(明示的に宣言すると早くなる)
  M5.Lcd.pushImage(0, 0, imgWidth, imgHeight, fixed);
  M5.Lcd.endWrite();
}

void loop() {

}

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

int base64Decode_ppm(const char* src, unsigned short *dtc) {
  char o0, o1, o2, o3;
  char h0, h1, h2, r_color, g_color, b_color;

  while (*src != '\0') {
    //Base64 to HEX
    o0 = base64_to_6bit(*src);
    o1 = base64_to_6bit(*(src+1));
    o2 = base64_to_6bit(*(src+2));
    o3 = base64_to_6bit(*(src+3));

    h0 = (o0 << 2) | ((o1 & 0x30) >> 4);
    h1 = ((o1 & 0xf) << 4) | ((o2 & 0x3c) >> 2);
    h2 = ((o2 & 0x3) << 6) | o3 & 0x3f;

    //HEX to RGB565
    r_color = h0 >> 3;
    g_color = h1 >> 2;
    b_color = h2 >> 3;
    *dtc = r_color << 11 | g_color << 5 | b_color;

    //address increment
    dtc++;
    src+=4;
  }
  return 0;
}
