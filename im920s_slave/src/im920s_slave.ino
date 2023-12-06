/*
内容：IM920sのPS3データ送信側プログラム
作成者：segfau-yama
githubリポジトリ：
連絡先(discord)：@.yamayama

ペアリング前提であるためIM920sの接続方法についてはリポジトリのを参照

受信データ形式：aa,bbbb,dd:受信データ<CR><LF>
  aa  ：ダミー（1バイト、00h固定）
  bbbb：ノード番号（2バイト）
  dd  ：受信データの電波強度
  受信データ：abcde…形式で各バイトをASCII1文字で出力
*/

#include <SoftwareSerial.h>

#define CHANNEL 16  // IM920sチャンネル番号(1~29), 受信モジュールと合わせる
#define NODE 1  // 送信モジュールのノード番号(0001~FFEF)

SoftwareSerial IM920sSerial(8, 9);  // IM920s通信用．

// IM920s受信データ共用体
union Dualshock3Protocol {
  uint8_t data[11];
  struct {
    uint8_t HEAD;  // 先頭Byte
    uint8_t LX;    // LStick X
    uint8_t LY;    // LStick Y
    uint8_t RX;    // RStick X
    uint8_t RY;    // RStick Y
    uint8_t LS;    // L2
    uint8_t RS;    // R2
    uint16_t BTN;  // デジタルボタン割り当て
    uint8_t SUM;   // チェックSUM
    uint8_t TAIL;  // 末尾Byte
  };
};

Dualshock3Protocol IM920s_recv;
int i;
char data;
char *convert_data;
char *command = "";
String receive_data;
int8_t start;
int8_t end;

void setup() {
  IM920sSerial.begin(19200);
  Serial.begin(19200);

  IM920sSerial.println("ENWR");
  delay(100);
  IM920sSerial.println("ECIO");  // 文字列通信モードに変更

  Serial.println("Start connecting");
}

void loop() {
  // IM920sの受信データが1byte以上か判定
  if (IM920sSerial.available()) {
    data = IM920sSerial.read();
    receive_data = receive_data + data;
    start = receive_data.indexOf(": ");
    end = receive_data.indexOf("\r\n");
    if (end >= 0) {
      receive_data = receive_data.substring(
          start + 1, end);  // DualShock3のデータのみ切り取り
      Serial.println(receive_data);
      receive_data = "";
    }
  }
}
