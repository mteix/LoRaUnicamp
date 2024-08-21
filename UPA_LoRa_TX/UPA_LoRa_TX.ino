

#include "LoRaWan_APP.h"
#include "Arduino.h"


// OLED DISPLAY: Just for tests
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "HT_DisplayUi.h"


#define RF_FREQUENCY 915000000  // Hz

#define TX_OUTPUT_POWER 5  // dBm

#define LORA_BANDWIDTH 0         // [0: 125 kHz, \
                                 //  1: 250 kHz, \
                                 //  2: 500 kHz, \
                                 //  3: Reserved]
#define LORA_SPREADING_FACTOR 7  // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5, \
                                 //  2: 4/6, \
                                 //  3: 4/7, \
                                 //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false


#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30  // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle = true;

bool write_msg = false;

String msg;
static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

// OLED DISPLAY

#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);  // addr , freq , i2c group , resolution , rst
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c group , resolution , rst
#endif


void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  txNumber = 0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  display.init();
  display.drawString(0, 0, "UPA 2024");
  display.drawString(0, 10, "Eng. de Telecom");
  display.drawString(0, 30, "LoRa TX");
  display.display();
  delay(7000);
  display.clear();
}



void loop() {

  // check if someone wants to send a message

  if (Serial.available() > 0) {
    // read the incoming byte:
    String msg2 = Serial.readString();
    // String msg2;
    sprintf(txpacket, "m = %s", msg2);
    // write_msg = true;
    
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));  //send the package out
    Radio.IrqProcess();
    
    lora_idle = false;
  }

  if (write_msg == false) {
    if (lora_idle == true) {
      delay(1000);


      txNumber += 0.01;
      sprintf(txpacket, "Packet # %0.2f", txNumber);  //start a package

      // Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));
      display.drawString(0, 25, "Sending package # ");
      display.drawString(100, 25, String(txNumber));
      display.drawString(0, 40, "TX Power (dBm): ");
      display.drawString(100, 40, String(TX_OUTPUT_POWER));

      display.display();
      display.clear();
      delay(1000);
      if (Serial.available() > 0) {
        String msg2 = Serial.readString();
        sprintf(txpacket, "%s", msg2);
        display.drawString(0, 25, "Mensagem: ");
        display.drawString(0, 40, String(msg2));
        // display.drawString(0, 40, "TX Power (dBm): ");
        // display.drawString(100, 40, String(TX_OUTPUT_POWER));
      display.display();
      delay(1500);
      display.clear();
      }
    // String msg2;
      
      Radio.Send((uint8_t *)txpacket, strlen(txpacket));  //send the package out
      lora_idle = false;
    }
  }
  Radio.IrqProcess();
}

void OnTxDone(void) {
  // Serial.println("TX done......");
  lora_idle = true;
}

void OnTxTimeout(void) {
  Radio.Sleep();
  Serial.println("TX Timeout......");
  lora_idle = true;
}