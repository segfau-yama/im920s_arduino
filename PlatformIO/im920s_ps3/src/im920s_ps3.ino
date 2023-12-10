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

// デジタルボタン共用体
union DigitalButtons {
  uint16_t all;
  struct {
    uint8_t high;
    uint8_t low;
  };
  struct {
    unsigned TRIANGLE : 1;
    unsigned CIRCLE : 1;
    unsigned CROSS : 1;
    unsigned SQUARE : 1;
    unsigned UP : 1;
    unsigned RIGHT : 1;
    unsigned DOWN : 1;
    unsigned LEFT : 1;
    unsigned L1 : 1;
    unsigned R1 : 1;
    unsigned L3 : 1;
    unsigned R3 : 1;
    unsigned SELECT : 1;
    unsigned START : 1;
    unsigned PS : 1;
    unsigned NO : 1;
  };
};

// IM920s受信データ共用体
union Dualshock3Protocol {
  uint8_t data[9];
  struct {
    uint8_t LX;          // LStick X
    uint8_t LY;          // LStick Y
    uint8_t RX;          // RStick X
    uint8_t RY;          // RStick Y
    uint8_t LS;          // L2
    uint8_t RS;          // R2
    DigitalButtons BTN;  // デジタルボタン割り当て
    uint8_t SUM;         // チェックSUM
  };
};

Dualshock3Protocol IM920s_send;
uint8_t i;
char command[30] = "";
char send_data[30] = "";

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

  IM920sSerial.println("ENWR");  // 設定変更許可
  delay(100);
  IM920sSerial.println("DCIO");  // 16進数通信モードに変更
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
  bitWrite(IM920s_send.BTN.all, 16, 0);  // 16bit目はボタン未割り当て
  for (i = 0; i < 15; i++) {
    if (PS3.getButtonPress(PS3_BTN[i])) {
      bitWrite(IM920s_send.BTN.all, i, 1);
    }
  }

  // チェックSUM作成
  for (i = 0; i < 7; i++) {
    IM920s_send.SUM += IM920s_send.data[i];
  }

  if (PS3.PS3Connected) {
    // 送信データをsend_dataに格納
    sprintf(send_data, "%02X %02X %02X %02X %02X %02X %02X %02X",
            IM920s_send.LX, IM920s_send.LY, IM920s_send.RX, IM920s_send.RY,
            IM920s_send.LS, IM920s_send.RS, IM920s_send.BTN, IM920s_send.SUM);

    // IM920s_send.dataを2桁16進数形式で送信
    sprintf(command, "TXDA %s", send_data);  // 全体にデータ送信
    IM920sSerial.println(command);
    Serial.println(command);
    IM920s_send.BTN = 0;  // ボタンデータ初期化
    IM920s_send.SUM = 0;  // Check SUM初期化
  }
  delay(50);
}
