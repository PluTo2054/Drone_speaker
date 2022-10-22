#define IR_RECEIVE_PIN      6 // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN         3
#define TONE_PIN            9
#define APPLICATION_PIN     8
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define IR_TIMING_TEST_PIN  7

#include <IRremote.h>
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>

//--------------------- НАСТРОЙКИ ----------------------
#define CH_NUM 0x60   // номер канала (должен совпадать с передатчиком)
//--------------------- НАСТРОЙКИ ----------------------

//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------
// УРОВЕНЬ МОЩНОСТИ ПЕРЕДАТЧИКА
// На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define SIG_POWER RF24_PA_MIN

// СКОРОСТЬ ОБМЕНА
// На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
// должна быть одинакова на приёмнике и передатчике!
// при самой низкой скорости имеем самую высокую чувствительность и дальность!!
// ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!
#define SIG_SPEED RF24_1MBPS
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 10);   // "создать" модуль на пинах 9 и 10 для НАНО/УНО

#define SPEEKER 8
SoftwareSerial secondarySerial(3, 4); // RX, TX
class Mp3Notify;
typedef DFMiniMp3<SoftwareSerial, Mp3Notify, Mp3ChipMH2024K16SS> DfMp3;
DfMp3 dfmp3(secondarySerial);

#define BUTT_1 0x45
#define BUTT_2 0x46
#define BUTT_3 0x47
#define BUTT_4 0x44
#define BUTT_5 0x40
#define BUTT_6 0x43
#define BUTT_7 0x7
#define BUTT_8 0x15
#define BUTT_9 0x9
#define BUTT_0 0x19
#define BUTT_ZIR 0x16
#define BUTT_REH 0xD
#define BUTT_OK 0x1c
#define BUTT_Up 0x18
#define BUTT_DOUN 0x52
#define BUTT_LEFT 0x8
#define BUTT_RITE 0x5A

uint8_t nom_audio = 0;

int buff_rx[5];
int buff_tx[5];

uint8_t pipeNo;
uint8_t address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

void setup()

{

  /*=====Контакти щоб відключати підсилювач=====*/
  pinMode (SPEEKER, OUTPUT);
  digitalWrite(SPEEKER, 1); //включаємо його
  /*=====Запускаємо спілкування з комп'ютером========*/
  Serial.begin(9600);

  /*====Запуск */
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK, 10);
  Serial.print(F("Ready to receive IR signals at pin "));
  Serial.println(IR_RECEIVE_PIN);

  Serial.println("initializing...");

  dfmp3.begin();//стартуємо mp3 модуль

  /*======Робота з гучністю=====*/
  uint16_t volume = dfmp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  dfmp3.setVolume(24); // виставити гучність 0-24
  /*=====Отримуємо кількість треків на флешці=====*/
  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);

  Serial.println("starting...");


  dfmp3.stop(); //зупинити програш аудіофайлу
  digitalWrite(SPEEKER, 0); //виключити підсилювач

  radioSetup();
}



void loop()
{
  if (IrReceiver.decode()) {

    // Print a short summary of received data
    IrReceiver.printIRResultShort(&Serial);
    switch (IrReceiver.decodedIRData.command) {
      case BUTT_0: {
          Serial.println("Pressed button 0");
          nom_audio = 0;
        } break;
      case BUTT_1: {
          Serial.println("Pressed button 1");
          nom_audio = 1;
        } break;
      case BUTT_2: {
          Serial.println("Pressed button 2");
          nom_audio = 2;
        } break;
      case BUTT_3: {
          Serial.println("Pressed button 3");
          nom_audio = 3;
        } break;
      case BUTT_4: {
          Serial.println("Pressed button 4");
          nom_audio = 4;
        } break;
      case BUTT_5: {
          Serial.println("Pressed button 5");
          nom_audio = 5;
        } break;
      case BUTT_6: {
          Serial.println("Pressed button 6");
          nom_audio = 6;
        } break;
      case BUTT_7: {
          Serial.println("Pressed button 7");
          nom_audio = 7;
        } break;
      case BUTT_8: {
          Serial.println("Pressed button 8");
          nom_audio = 8;
        } break;
      case BUTT_9: {
          Serial.println("Pressed button 9");
          nom_audio = 9;
        } break;
      case BUTT_ZIR: {
          Serial.println("Pressed button *");
        } break;
      case BUTT_REH: {
          Serial.println("Pressed button #");
          dfmp3.stop();
        } break;
      case BUTT_OK: {
          Serial.println(nom_audio);
          digitalWrite(SPEEKER, 1);
          dfmp3.playMp3FolderTrack(nom_audio);

        } break;
      case BUTT_Up: {
          Serial.println("Pressed button up");
        } break;
      case BUTT_DOUN: {
          Serial.println("Pressed button doun");
        } break;
      case BUTT_LEFT: {
          Serial.println("Pressed button left");
        } break;
      case BUTT_RITE: {
          Serial.println("Pressed button rite");
        } break;
    }
    IrReceiver.resume();
  }
  if (radio.available(&pipeNo)){
    radio.read(buff_rx, sizeof(buff_rx));
    buff_tx[0] = 30;
    radio.writeAckPayload(pipeNo, &buff_tx, sizeof(buff_tx));

  }
}

void radioSetup() {             // настройка радио
  radio.begin();                // активировать модуль
  radio.setAutoAck(1);          // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);      // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();     // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     // размер пакета, байт
  radio.openReadingPipe(1, address[0]); // хотим слушать трубу 0
  radio.setChannel(CH_NUM);     // выбираем канал (в котором нет шумов!)
  radio.setPALevel(SIG_POWER);  // уровень мощности передатчика
  radio.setDataRate(SIG_SPEED); // скорость обмена
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  radio.powerUp();         // начать работу
  radio.startListening();  // начинаем слушать эфир, мы приёмный модуль
}
