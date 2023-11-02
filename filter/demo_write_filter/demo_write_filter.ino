//——————————————————————————————————————————————————————————————————————————————
//  ACAN2515 Demo in normal mode
//——————————————————————————————————————————————————————————————————————————————

#include <ACAN2515.h>
static const byte MCP2515_SCK = 52 ; // SCK input of MCP2515
static const byte MCP2515_SI  = 51 ; // SI input of MCP2515
static const byte MCP2515_SO  = 50 ; // SO output of MCP2515

static const byte MCP2515_CS  = 53 ; // CS input of MCP2515
static const byte MCP2515_INT = 2 ; // INT output of MCP2515
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
    switch (gSentFrameCount % 3) {
    case 0 : // Matches filter #0
      frame.id = 0x3ad2331 ;
      frame.ext = true ;
        frame.rtr = true ;
    //  frame.data [0] = gSentFrameCount ;
    //  frame.len = 1 ;
      break ;
     case 1 : // Does not match filter #0
        frame.id = 0x11d2331 ;
       frame.ext = true ;
         frame.rtr = true ;
     //        frame.len = 1;
     //  frame.data [0] = gSentFrameCount;

      break ;
    case 2 :  // No data byte, still received
      frame.id = 0xd3d2431 ;
            frame.ext = true ;
              frame.rtr = true ;
    //         frame.len = 1;
    //   frame.data [0] = gSentFrameCount;
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
      Serial.println (frame.rtr) ;
      gSentFrameCount += 1 ;
    }else{
      Serial.println ("Send failure") ;
    }
}
