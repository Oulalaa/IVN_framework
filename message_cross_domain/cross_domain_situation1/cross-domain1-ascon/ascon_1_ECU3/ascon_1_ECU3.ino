//——————————————————————————————————————————————————————————————————————————————
//  ECU3 communication with ECU1 in domain1
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
static const uint32_t QUARTZ_FREQUENCY = 16UL  * 1000UL * 1000UL ; // 16 MHz

bool same_mac=false;
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
//  SerialUSB.begin (38400) ;
//--- Wait for serial (blink led at 10 Hz during waiting)
  while (!Serial) {
    delay (50) ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  }
//--- Begin SPI
  SPI.begin () ;
  ACAN2515Settings settings (QUARTZ_FREQUENCY, 125UL * 1000UL) ; // CAN bit rate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode ; 
  const ACAN2515Mask rxm0 = extended2515Mask (0x1ff800ff);
  const ACAN2515AcceptanceFilter filters [] = {

    {extended2515Filter (0x4500121), receive0},//0x4500121
    {extended2515Filter (0x4200122), receive0},//0x4200122

  }; 
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0, filters, 2);
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}
//——————————————————————————————————————————————————————————————————————————————
//   ENCRYPT, MAC
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
//——————————————————————————————————————————————————————————————————————————————
//   DECRYPT, CHECK MAC
//——————————————————————————————————————————————————————————————————————————————
void ascon_dec(CANMessage *frame,CANMessage *frame_cipher,CANMessage *frame_mac){
  uint8_t key[] = {0x11,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b,0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b};
  uint8_t IV[] = {0x13,0x18,0x19,0x1a,0x1c,0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b,0x12,0x11,0x12};
 
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


void loop(){
   CANMessage frame,frame_mac, frame_cipher_or_rtr;
      frame.id = 0X4300122 ;
      frame_cipher_or_rtr.id = frame.id;
      frame_mac.id = frame.id;
      
      frame.ext = true ;
      frame_cipher_or_rtr.ext = frame.ext;
      frame_mac.ext = frame.ext;
      
      frame.rtr = true;
      frame_mac.rtr = frame.rtr;
      frame_cipher_or_rtr.rtr = frame.rtr;

    //----------------------------------send_rtr-------------------------------------   
    timecnt = micros();
    const bool ok_rtr = can.tryToSend (frame_cipher_or_rtr) ;  
    const bool ok_mac = can.tryToSend (frame_mac) ;
    if(ok_rtr&&ok_mac){
      Serial.print ("Send_frame_rtr:") ;
      Serial.println(frame_cipher_or_rtr.rtr);
      Serial.print ("send RTR frame ID :") ;
      Serial.print(frame.id,HEX);
      Serial.println();
      timecnt = micros()-timecnt;
      Serial.print("send RTR_timecnt: ");
      Serial.println(timecnt);      
      Serial.println ("-----------------Send RTR success---------------") ;  
    }else{
      Serial.println ("-----------------Send RTR failure---------------") ;
    }
    
   //----------------------------------receive------------------------------------- 
   if(can.available()){
     timecnt = micros();
     can.receive(frame_cipher_or_rtr);
     can.receive(frame_mac);
    //------------------------------------------------Receive RTR----------------------------------------------
    if(frame_cipher_or_rtr.rtr==true){
      frame_cipher_or_rtr.id=0X4400121;
      frame_mac.id = frame_cipher_or_rtr.id;
      frame_cipher_or_rtr.len = 8;
      frame_mac.len = 8;
      frame_cipher_or_rtr.rtr=false;
      frame_mac.rtr=false;
      frame.data[0]=0X31;
      frame.data[1]=0X31;
      frame.data[2]=0X31;
      frame.data[3]=0X31;
      frame.data[4]=0X31;
      frame.data[5]=0X31;
      frame.data[6]=0X31;
      frame.data[7]=0X31;
      Serial.println ("Receive RTR from uno1") ;
      Serial.println ("-----------------Receive RTR success---------------") ;
      ascon_enc(&frame,&frame_cipher_or_rtr,&frame_mac);
      //----------------------------------send_cipher-------------------------------------   
      const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
      const bool ok_mac = can.tryToSend (frame_mac) ;
      if(ok_cipher_or_rtr&&ok_mac){
        Serial.print ("Send DATA frame ID :") ;
        Serial.println(frame_cipher_or_rtr.id,HEX);
        timecnt = micros()-timecnt;
        Serial.print("receive RTR and send DATA_timecnt: ");
        Serial.println(timecnt);
        Serial.println ("-----------------Send DATA success---------------") ;
      }else{
        Serial.println ("-----------------Send DATA failure---------------") ;
      }
    }
    //------------------------------------------------Receive DATA----------------------------------------------
    else{
      Serial.println ("Receive DATA frame ") ;
      Serial.print ("frame_id: ") ;
      Serial.print(frame_cipher_or_rtr.id,HEX);
      ascon_dec(&frame,&frame_cipher_or_rtr,&frame_mac);
      if(same_mac){
        timecnt = micros()-timecnt;
        Serial.print("receive DATA_timecnt: ");
        Serial.println(timecnt);
        Serial.println ("-----------------same_mac,Receive DATA success---------------") ;
      }else{
      Serial.println("-----------------wrong_mac,discard it---------------");
      } 
    }
  }
//  else{
//     Serial.println ("-----------------Receive faliure---------------") ;
//  }
   delay(1000);
}
