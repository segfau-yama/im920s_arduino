/*
内容：IM920sのPS3データ送信側プログラム
作成者：segfau-yama
githubリポジトリ：https://github.com/segfau-yama/im920s_arduino
連絡先(discord)：@.yamayama

ペアリング前提であるためIM920sの接続方法についてはリポジトリのREADMEを参照

受信データ形式：aa,bbbb,dd:受信データ<CR><LF>
  aa  ：ダミー（1バイト、00h固定）
  bbbb：ノード番号（2バイト）
  dd  ：受信データの電波強度
  受信データ：Dualshock3Protocolに沿って受信
*/

#include <SoftwareSerial.h>
#define TO_STRING(VariableName) #VariableName
#define CHANNEL 16  // IM920sチャンネル番号(1~29), 受信モジュールと合わせる
#define NODE 1  // 送信モジュールのノード番号(0001~FFEF)

SoftwareSerial IM920sSerial(8, 9);  // IM920s通信用．

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

Dualshock3Protocol IM920s_recv;
int i;
char data;
char command[20] = "";
String recv_data;
char recv_datas[32];
uint8_t recv_SUM = 0;
uint8_t LX_past, LY_past = 0;

void setup() {
  IM920sSerial.begin(19200);
  Serial.begin(19200);

  IM920sSerial.println("ENWR");  // 設定変更許可
  delay(100);
  IM920sSerial.println("DCIO");  // 16進数通信モードに変更

  Serial.println("Start connecting");
}

void loop() {
  // IM920sの受信データが1byte以上か判定
  if (IM920sSerial.available()) {
    data = IM920sSerial.read();
    recv_data = recv_data + data;
    int8_t start = recv_data.indexOf(":");   // :の配列番号を取得
    int8_t end = recv_data.indexOf("\r\n");  // \r\nの配列番号を取得
    if (end >= 0) {
      // DualShock3のデータのみを切り取る
      recv_data = recv_data.substring(start + 1, end);
      recv_data.toCharArray(recv_datas, sizeof(recv_datas) + 1);
      recv_data = "";

      // recv_datasをIM920s_recvに格納
      sscanf(recv_datas, "%hhx,%hhx,%hhx,%hhx,%hhx,%hhx, %hhx, %hhx, %hhx",
             &IM920s_recv.LX, &IM920s_recv.LY, &IM920s_recv.RX, &IM920s_recv.RY,
             &IM920s_recv.LS, &IM920s_recv.RS, &IM920s_recv.BTN.high,
             &IM920s_recv.BTN.low, &IM920s_recv.SUM);
      // チェックSUM
      for (i = 0; i < 7; i++) {
        recv_SUM += IM920s_recv.data[i];
      }
      if (recv_SUM != IM920s_recv.SUM) {
        Serial.println("Check SUM failed!");
      }

      // ジョイスティック例

      uint8_t LX_delta = abs(IM920s_recv.LX - LX_past);
      uint8_t LY_delta = abs(IM920s_recv.LY - LY_past);
      if (LX_delta >= 10 || LY_delta >= 10) {
        sprintf(command, "LX: %d ", IM920s_recv.LX);
        Serial.print(command);
        sprintf(command, "LY: %d", IM920s_recv.LY);
        Serial.println(command);
        delay(50);
      }

      // デジタルボタン例
      if (IM920s_recv.BTN.CIRCLE) {
        Serial.println("CIRCLE");
        delay(50);
      }
      if (IM920s_recv.BTN.LEFT) {
        Serial.println("LEFT");
        delay(50);
      }
      if (IM920s_recv.BTN.L3) {
        Serial.println("L3");
        delay(50);
      }
      if (IM920s_recv.BTN.PS) {
        Serial.println("PS");
        delay(50);
      }
    }
  }
  LX_past = IM920s_recv.LX;
  LY_past = IM920s_recv.LY;
  recv_SUM = 0;
}