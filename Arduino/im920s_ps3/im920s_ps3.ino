/*
内容：IM920sのPS3データ送信側プログラム
作成者：segfau-yama
githubリポジトリ：https://github.com/segfau-yama/im920s_arduino
連絡先(discord)：@.yamayama
参考：https://github.com/RoboconHirose/Robocon2019_teamB_code/blob/master/Robocon2019-B_slave/main.cpp

ペアリング前提であるためIM920sの接続方法についてはリポジトリのREADMEを参照

*/

#include <PS3USB.h>
#include <SoftwareSerial.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#define CHANNEL 16  // IM920sチャンネル番号(1~29), 受信モジュールと合わせる
#define NODE 2  // 受信モジュールのノード番号(0001~FFEF)

USB Usb;
PS3USB PS3(&Usb);
SoftwareSerial IM920sSerial(
    A4, A5);  // 9pinがUSB host Shieldのピンと干渉するためA4,A5を利用

// IM920s送信データ共用体
union Dualshock3Protocol {
  uint8_t data[9];
  struct {
    uint8_t LX;    // LStick X
    uint8_t LY;    // LStick Y
    uint8_t RX;    // RStick X
    uint8_t RY;    // RStick Y
    uint8_t LS;    // L2
    uint8_t RS;    // R2
    uint16_t BTN;  // デジタルボタン割り当て
    uint8_t SUM;   // チェックSUM
  };
};

Dualshock3Protocol IM920s_send;
uint8_t i;
char command[30] = "";
String send_data = "";

// ジョイスティック配列
const uint32_t PS3_Hat[4] = {LeftHatX, LeftHatY, RightHatX, RightHatY};

// デジタルボタン配列
const uint32_t PS3_BTN[15] = {
    TRIANGLE, CIRCLE, CROSS, SQUARE, UP,     RIGHT, DOWN, LEFT,
    L1,       R1,     L3,    R3,     SELECT, START, PS,
};

void setup() {
  IM920sSerial.begin(19200);
  Serial.begin(19200);
  Usb.Init();

  IM920sSerial.println("ENWR");
  delay(100);
  IM920sSerial.println("ECIO");  // 文字列通信モードに変更
}

void loop() {
  Usb.Task();

  // ジョイスティック取得
  for (i = 0; i < 4; i++) {
    IM920s_send.data[i] = PS3.getAnalogHat(PS3_Hat[i]);
  }

  // アナログボタン取得
  IM920s_send.LS = PS3.getAnalogButton(L2);
  IM920s_send.RS = PS3.getAnalogButton(R2);

  // デジタルボタン取得：1bitごとに15個のボタンデータを格納
  // IM920s_send.data[7]: ボタン上位bit
  // IM920s_send.data[8]: ボタン下位bit
  bitWrite(IM920s_send.BTN, 16, 0);  // 16bit目はボタン未割り当て
  for (i = 0; i < 15; i++) {
    if (PS3.getButtonPress(PS3_BTN[i])) {
      bitWrite(IM920s_send.BTN, i, 1);
    }
  }

  // チェックSUM作成
  for (i = 0; i < 7; i++) {
    IM920s_send.SUM += IM920s_send.data[i];
  }

  if (PS3.PS3Connected) {
    // 送信データ列
    for (auto data : IM920s_send.data) {
      char convert_data[2];
      sprintf(convert_data, "%02X", data);
      send_data = send_data + convert_data;
    }
    // IM920s_send.dataを2桁16進数形式で送信
    sprintf(command, "TXDA %s",
            send_data.c_str());  // ノード番号(NODE)にデータ送信
    IM920sSerial.println(command);
    Serial.println(command);
    IM920s_send.BTN = 0;  // ボタンデータ初期化
    send_data = "";
  }
  delay(100);
}
