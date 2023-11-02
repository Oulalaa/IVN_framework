//——————————————————————————————————————————————————————————————————————————————
//  ACAN2515 Demo in normal mode,RECEIVE ECU 1
//——————————————————————————————————————————————————————————————————————————————

#include <ACAN2515.h>
static const byte MCP2515_CS  = 10 ; // CS input of MCP2515 (adapt to your design) 
static const byte MCP2515_INT =  2 ; // INT output of MCP2515 (adapt to your design)
ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;
static const uint32_t QUARTZ_FREQUENCY = 16UL * 1000UL * 1000UL ; // 16 MHz

//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————
static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;

static void receive0 (const CANMessage & inMessage) {
    Serial.println();
    Serial.println ("Function_Receive0 ") ;
}

void setup () {
//--- Switch on builtin led
  pinMode (LED_BUILTIN, OUTPUT) ;
  digitalWrite (LED_BUILTIN, HIGH) ;
//--- Start serial
  Serial.begin (38400) ;
//--- Wait for serial (blink led at 10 Hz during waiting)
  while (!Serial) {
    delay (50) ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  }
//--- Begin SPI
  SPI.begin () ;
  ACAN2515Settings settings (QUARTZ_FREQUENCY, 125UL * 1000UL) ; // CAN bit rate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode ; // Select loopback mode
  const ACAN2515Mask rxm0 = extended2515Mask (0x1FFFFFF0) ; // For filter #0 and #1
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (0x111A111), receive0},
  } ;
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0, filters, 1) ;
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

void loop () {
CANMessage frame;
  if (gBlinkLedDate < millis ()) {
//   gBlinkLedDate += 100;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  }
//----------------------------------receive-------------------------------------
CANMessage inmsg; 
  if (can.available ()) {
   can.receive(inmsg);
  //can.dispatchReceivedMessage () ;
     Serial.println();
      Serial.print ("Receive1Frame_id: ") ;
     Serial.print(inmsg.id,HEX);
     Serial.print ("    Received1_SentFrameCount: ") ;
     Serial.print (inmsg.data[0]) ; 
     Serial.print (inmsg.data[1]) ; 
     Serial.print (inmsg.data[2]) ; 
     Serial.print (inmsg.data[3]) ; 
    Serial.print ("    Received1: ") ;
    Serial.println (gReceivedFrameCount) ;
    gReceivedFrameCount ++ ;
  }else{
     Serial.print ("Receive faliure ") ;
  }
}
