/* Heltec Automation Receive communication test example
 *
 * Function:
 * 1. Receive the same frequency band lora signal program
 *  
 * Description:
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
 * */

#include "LoRaWan_APP.h"
#include "Arduino.h"

// #include <LoRa.h> // my inclusion

// OLED DISPLAY: Just for tests
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "HT_DisplayUi.h"

#define RF_FREQUENCY 915000000  // Hz

#define TX_OUTPUT_POWER 20  // dBm

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

// Counter to display messages
int cnt = 0;

static RadioEvents_t RadioEvents;

#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);  // addr , freq , i2c group , resolution , rst
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c group , resolution , rst
#endif


int16_t txNumber;

int16_t rssi, rxSize;

bool lora_idle = true;

void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  txNumber = 0;
  rssi = 0;

  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  // Start OLED display
  display.init();
  display.clear();
  display.display();

  display.setContrast(255);
  Serial.begin(115200);
  // pinMode(36, OUTPUT);
  // digitalWrite(36,HIGH);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Initialize...");
  display.drawString(0, 0, "MANGROVESCOPE");
  display.drawString(0, 10, "RX Node");
  display.display();
  delay(5000);
  display.clear();
}



void loop() {


  // rssi = LoRa.rssi;
  // snr = Lora.snr;
  if (lora_idle) {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
    // display.drawString(0, 25, "Waiting for LoRa package...");
    // display.display();
    // delay(500);
    // display.clear();
   
  }

  Radio.IrqProcess();
  
  
  
}


// ------- need to pass display as an argument here, then call and send to OLED

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  
  rssi = rssi;
  rxSize = size;
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';
  Radio.Sleep();
  // display.drawString(30, 0, "Cachorro chupa pau");
  // display.display();
  Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n", rxpacket, rssi, rxSize);
  display.drawString(0, 0, "RX NODE #1");
  display.drawString(0, 20, "RSSI = ");
  display.drawString(40, 20, String(rssi));
  // delay(500);
  // display.clear();
  // display.drawString(0, 30, "SNR = ");
  // display.drawString(40, 30, String(snr));
 
// //  display in  two lines 
  if (cnt==1){
    display.drawString(0, 50, rxpacket);
    cnt = 0;
    display.display();
    display.clear();
    // delay(1000);
  lora_idle = true;
  }
  else{
    display.drawString(0, 40, rxpacket);
    display.display();
    cnt = 1;
  }
  
  // display.drawString(4, 50, payload);
  // --------------------------------------------
  // delay(1000);
  // lora_idle = true;
  // display.display();
  // memcpy(rxpacket, payload, size);
  // rxpacket[size] = '\0';
  // Radio.Sleep();
  // display.drawString(0, 40, rxpacket);
  //------------------------------------
  
  // display.display();
  
  // delay(1000);
  lora_idle = true;
  // display.clear();
  
}