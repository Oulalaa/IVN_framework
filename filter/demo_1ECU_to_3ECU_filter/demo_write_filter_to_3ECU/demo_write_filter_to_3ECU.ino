//——————————————————————————————————————————————————————————————————————————————
//  ACAN2515 Demo in normal mode,WRITE ECU
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
static uint32_t gSentFrameCount = 0 ;

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
//--- Configure ACAN2515
//  Serial.println ("Configure ACAN2515") ;
  ACAN2515Settings settings (QUARTZ_FREQUENCY, 125UL * 1000UL) ; // CAN bit rate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode ; // Select NormalMode
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }) ;
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

void loop () {
CANMessage frame;
  if (gBlinkLedDate < millis ()) {
//    gBlinkLedDate += 2000 ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
    switch (gSentFrameCount % 5) {
    case 0 : // Matches UNO1,3
      frame.id = 0x111A111 ;
     frame.ext = true ;
       frame.len = 4;
       frame.data [0] = gSentFrameCount;
       frame.data [1] = gSentFrameCount ;
      frame.data [2] = gSentFrameCount ;
      frame.data [3] = gSentFrameCount ;
      break ;
     case 1 : // Matches UNO2,3
        frame.id = 0x222B222 ;
       frame.ext = true ;
       frame.len = 4;
       frame.data [0] = gSentFrameCount;
       frame.data [1] = gSentFrameCount ;
      frame.data [2] = gSentFrameCount ;
      frame.data [3] = gSentFrameCount ;
      break ;
    case 2 :  // Matches UNO3
      frame.id = 0x333C333 ;
       frame.ext = true ;
       frame.len = 4;
       frame.data [0] = gSentFrameCount;
       frame.data [1] = gSentFrameCount ;
      frame.data [2] = gSentFrameCount ;
      frame.data [3] = gSentFrameCount ;
      break;
    case 3 :  // Matches UNO1,3
      frame.id = 0x111A11A ;
       frame.ext = true ;
       frame.len = 4;
       frame.data [0] = gSentFrameCount;
       frame.data [1] = gSentFrameCount ;
      frame.data [2] = gSentFrameCount ;
      frame.data [3] = gSentFrameCount ;
      break;
    case 4 :  // Matches UNO2,3
      frame.id = 0x222B22B ;
       frame.ext = true ;
       frame.len = 4;
       frame.data [0] = gSentFrameCount;
       frame.data [1] = gSentFrameCount ;
      frame.data [2] = gSentFrameCount ;
      frame.data [3] = gSentFrameCount ;
      break ;
    }
  }
//----------------------------------send-------------------------------------   
    const bool ok = can.tryToSend (frame) ;
    if (ok) {
      Serial.println();
      Serial.print ("Sent...: ") ;
      Serial.print (gSentFrameCount) ;
      Serial.print ("      Frame ID: ") ;
      Serial.println (frame.id,HEX) ;
      gSentFrameCount += 1 ;
    }else{
      Serial.println ("Send failure") ;
    }
}
