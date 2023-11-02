//——————————————————————————————————————————————————————————————————————————————
//   MEGA1 as domain1 getaway
//——————————————————————————————————————————————————————————————————————————————
#include <SPI.h>
#include <ACAN2515.h>
#include<Ascon128.h>

static const byte MCP2515_SCK = 52 ; // SCK input of MCP2515
static const byte MCP2515_SI  = 51 ; // SI input of MCP2515
static const byte MCP2515_SO  = 50 ; // SO output of MCP2515

static const byte MCP2515_CS  = 53 ; // CS input of MCP2515
static const byte MCP2515_INT = 2 ; // INT output of MCP2515
ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;
static const uint32_t QUARTZ_FREQUENCY = 16UL  * 1000UL *1000UL ; // 16 MHz

bool same_mac=true;
unsigned int timecnt=micros();
//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————
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
  settings.mRequestedMode = ACAN2515Settings::NormalMode ; 
  const ACAN2515Mask rxm0 = extended2515Mask (0x1c1800e0);
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (0x8200121 ), receive0},
    { extended2515Filter (0x4280141), receive0},
  }; 
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0, filters, 2);
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

//——————————————————————————————————————————————————————————————————————————————
//   DECRYPT(domain2 key), CHECK MAC
//——————————————————————————————————————————————————————————————————————————————
void ascon_dec(CANMessage *frame,CANMessage *frame_cipher,CANMessage *frame_mac){
  uint8_t key[] = {0x21,0x21,0x22,0x23,0x28,0x29,0x2a,0x2b,0x20,0x21,0x22,0x23,0x28,0x29,0x2a,0x2b};
  uint8_t IV[] = {0x23,0x28,0x29,0x2a,0x2c,0x20,0x21,0x22,0x23,0x28,0x29,0x2a,0x2b,0x22,0x21,0x22};

  //  decry_begin  
  timecnt = micros();
  Ascon128 te;
  te.clear();
  te.setKey(key,16); //around 100us
  te.setIV(IV,16);
  te.decrypt(frame->data,frame_cipher->data,frame_cipher->len);
  timecnt = micros()-timecnt;
  Serial.print("decry_timecnt: ");
  Serial.println(timecnt);

  //auth_check_begin
  timecnt = micros();
  same_mac=te.checkTag(frame_mac->data,frame->len);
  timecnt = micros()-timecnt;
  Serial.print("mac_check_timecnt: ");
  Serial.println(timecnt);
}  

//——————————————————————————————————————————————————————————————————————————————
//   ENCRYPT(domain1 key), MAC
//——————————————————————————————————————————————————————————————————————————————
void ascon_enc(CANMessage *frame,CANMessage *frame_cipher,CANMessage *frame_mac){
  uint8_t key[] = {0x11,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b,0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b};
  uint8_t IV[] = {0x13,0x18,0x19,0x1a,0x1c,0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b,0x12,0x11,0x12};
  
  //  encry_begin  
  timecnt = micros();
  Ascon128 te;
  te.clear();
  te.setKey(key,16); //around 100us
  te.setIV(IV,16);
    
  te.encrypt(frame_cipher->data,frame->data,frame->len);
  timecnt = micros()-timecnt;
  Serial.print("encry_timecnt: ");
  Serial.println(timecnt);

   //  auth_begin
  timecnt = micros();
  te.addAuthData(frame->data,frame->len);
  te.computeTag(frame_mac->data,frame->len);
  timecnt = micros()-timecnt;
  Serial.print("mac_timecnt: ");
  Serial.println(timecnt);
} 

void loop(){
   CANMessage frame,frame_mac,frame_cipher_or_rtr;
   if(can.available()){
    timecnt = micros();
    can.receive(frame_cipher_or_rtr);
    can.receive(frame_mac);
    //------------------------------------------------Receive RTR----------------------------------------------
    if(frame_cipher_or_rtr.rtr==true){
      Serial.print("receive RTR.id:");
      Serial.println(frame_cipher_or_rtr.id,HEX);
      if((frame_cipher_or_rtr.id&0X1c1800e0)==0X8000020){
        frame_cipher_or_rtr.id = 0x8280121;
        frame_mac.id = frame_cipher_or_rtr.id;
        //----------------------------------send_RTR-------------------------------------      
        const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
        const bool ok_mac = can.tryToSend (frame_mac) ;
        if(ok_cipher_or_rtr){
          Serial.print("to getaway list RTR.id:");
          Serial.println(frame_cipher_or_rtr.id,HEX);
          timecnt = micros()-timecnt;
          Serial.print("reply to getaway list_timecnt: ");
          Serial.println(timecnt);
          Serial.println ("-----------------Mega1 reply RTR to getaway list succeed---------------") ;
        }else{
          Serial.println ("-----------------Mega1 reply RTR to getaway list failure---------------") ;
        }
      }
      else if((frame_cipher_or_rtr.id&0X1c1800e0)==0x4080040){
        frame_cipher_or_rtr.id = 0X4300141;
        frame_mac.id = frame_cipher_or_rtr.id;
        //----------------------------------send_RTR-------------------------------------
        const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
        const bool ok_mac = can.tryToSend (frame_mac) ;
        if(ok_cipher_or_rtr){
          Serial.print("to domain RTR.id:");
          Serial.println(frame_cipher_or_rtr.id,HEX);
          timecnt = micros()-timecnt;
          Serial.print("reply to own domain_timecnt: ");
          Serial.println(timecnt);
          Serial.println ("-----------------Mega1 reply RTR to own domain succeed---------------") ;
        }else{
          Serial.println ("-----------------Mega1 reply RTR to own domain failure---------------") ;
        }
      }
    }
    //------------------------------------------------Receive DATA----------------------------------------------
    else{
       Serial.print("receive DATA.id:");
       Serial.println(frame_cipher_or_rtr.id,HEX);
       if((frame_cipher_or_rtr.id&0X1c1800e0)==0x4080040){
        ascon_dec(&frame,&frame_cipher_or_rtr,&frame_mac); 
        if(same_mac){
          frame.id = 0x8200121;
          frame.len = 8;
          frame.ext = true ;
          frame_cipher_or_rtr.id = frame.id;
          frame_cipher_or_rtr.len = frame.len;
          frame_cipher_or_rtr.ext = true;
          frame_mac.ext = true;
          frame_mac.id = frame.id;
          frame_mac.len = frame.len;
          ascon_enc(&frame,&frame_cipher_or_rtr,&frame_mac);
          //----------------------------------send-------------------------------------      
          const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
          //Serial.println(ok_cipher_or_rtr);
          const bool ok_mac = can.tryToSend (frame_mac) ;  
          //Serial.println(ok_mac);         
          if(ok_cipher_or_rtr&&ok_mac){
            Serial.print("to domain DATA.id:");
            Serial.println(frame.id,HEX);
            timecnt = micros()-timecnt;
            Serial.print("reply to own domain_timecnt: ");
            Serial.println(timecnt);
            Serial.println ("-----------------Mega1 reply DATA to own domain succeed---------------") ;
          }else{
            Serial.println ("-----------------Mega1 reply DATA to own domain failure---------------") ;
          }
        }else{
          Serial.println ("-----------------wrong_mac,discard it---------------"); 
        }
      }
      else if((frame_cipher_or_rtr.id&0x1c1800e0)==0X8000020){
        frame.id=0x8280121;
        frame_cipher_or_rtr.id = frame.id;
        frame_mac.id = frame.id;
        //----------------------------------send-------------------------------------   
        const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
        const bool ok_mac = can.tryToSend (frame_mac) ;
        if(ok_cipher_or_rtr&&ok_mac){
          Serial.print("to getaway list DATA.id:");
          Serial.println(frame.id,HEX);
          timecnt = micros()-timecnt;
          Serial.print("reply to getaway list_timecnt: ");
          Serial.println(timecnt);
          Serial.println ("-----------------Mega1 reply DATA to getaway list succeed---------------") ;
        }else{
        Serial.println ("-----------------Mega1 reply DATA to getaway list fail---------------") ;
        }
      }
    }
   }
   delay(2000);
}
