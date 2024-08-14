/* Heltec Automation send communication test example
 *
 * Function:
 * 1. Send data from a esp32 device over hardware 
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


/*
	Medidor de batimentos cardiacos 
        Rodrigo Feliciano - http://www.pakequis.com.br
	
	Com o código original: 
        Pulse Sensor Amped 1.4  by Joel Murphy and Yury Gitman   http://www.pulsesensor.com
	https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino/blob/master/README.md  
*/



#include "LoRaWan_APP.h"
#include "Arduino.h"


// OLED DISPLAY: Just for tests
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "HT_DisplayUi.h"


// LoRa Variables
#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             20        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );


//HBS (Heart Beating Sensor) Variables:

//  Variables
int pulsePin = 1;                 // Pulse Sensor purple wire connected to analog pin 1

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.


// OLED DISPLAY 

#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);  // addr , freq , i2c group , resolution , rst
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c group , resolution , rst
#endif


void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);

    interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 

    txNumber=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
   
  display.init();
  display.drawString(0, 0, "MANGROVESCOPE");
  display.drawString(0, 10, "TX Node");
  display.display();
  delay(1000);
  display.clear();
   }

//  Where the Magic Happens
void loop()
{

	if(lora_idle == true)
	{
    if (QS == true)		// A Heartbeat Was Found
	  {     
  		lcd.setCursor(9,0);
  		lcd.print(BPM);	//Mostra mensagem
                  lcd.print(" BPM ");
  		QS = false;     // reset the Quantified Self flag for next time    
      }
 
    delay(20);                             //  take a break
    //delay(1000);
		sprintf(txpacket,"Packet # %0.2f",BPM);  //start a package
   
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
    display.drawString(0, 25, "Sending package # ");
    display.drawString(100, 25, String(BPM));
    display.drawString(0, 40, "TX Power (dBm): ");
    display.drawString(100, 40, String(TX_OUTPUT_POWER));

    display.display();
    display.clear();
    delay(1000);
		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;
	}
  Radio.IrqProcess( );
}

void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}
