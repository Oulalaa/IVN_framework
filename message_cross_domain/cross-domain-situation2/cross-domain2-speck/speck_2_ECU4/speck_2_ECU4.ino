//——————————————————————————————————————————————————————————————————————————————
//  
//——————————————————————————————————————————————————————————————————————————————
#include <SPI.h>
extern "C"{
#include <speck.h>
#include <chaskey.h>
#include <mcp2515.h>
}
#include <ACAN2515.h>
static uint32_t gSentFrameCount = 0 ;
static uint32_t gReceivedFrameCount = 0 ;


static const byte MCP2515_CS  = 10 ; // CS input of MCP2515 (adapt to your design) 
static const byte MCP2515_INT =  2 ; // INT output of MCP2515 (adapt to your design)
ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;
static const uint32_t QUARTZ_FREQUENCY = 16UL  * 1000UL * 1000UL ; // 16 MHz
bool same_mac=true;
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
  const ACAN2515Mask rxm0 = extended2515Mask (0x01fffff);
  const ACAN2515AcceptanceFilter filters [] = {

    {extended2515Filter (0x0128041), receive0},
    {extended2515Filter (0x0028042), receive0},

  }; 
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0, filters, 2);
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}
//——————————————————————————————————————————————————————————————————————————————
//   DECRYPT
//——————————————————————————————————————————————————————————————————————————————
void decrypt(CANMessage *frame,CANMessage *frame_cipher){
    //--------------------speck_init——————————————
    uint8_t my_IV[] = {0x32,0x14,0x76,0x58};
    uint8_t my_counter[] = {0x2F,0x3D,0x5C,0x7B};
    uint32_t result;
    SimSpk_Cipher my_speck_cipher;
    uint8_t speck128_64_key[] = {0x1b,0x1a,0x19,0x18,0x13,0x12,0x11,0x10,0x0b,0x0a,0x09,0x08,0x03,0x02,0x01,0x00};
    //————————————decrypt——————————————
    result = Speck_Init(&my_speck_cipher, cfg_128_64, ECB, speck128_64_key, my_IV, my_counter);
    Speck_Decrypt(my_speck_cipher, frame_cipher->data, frame->data);
}
//——————————————————————————————————————————————————————————————————————————————
//   ENCRYPT
//——————————————————————————————————————————————————————————————————————————————
void encrypt(CANMessage *frame,CANMessage *frame_cipher){
   //--------------------speck_init——————————————
   uint8_t my_IV[] = {0x32,0x14,0x76,0x58};
   uint8_t my_counter[] = {0x2F,0x3D,0x5C,0x7B};
   uint32_t result;
   SimSpk_Cipher my_speck_cipher;
   uint8_t speck128_64_key[] = {0x1b,0x1a,0x19,0x18,0x13,0x12,0x11,0x10,0x0b,0x0a,0x09,0x08,0x03,0x02,0x01,0x00};  
   //————————————encrypt——————————————
   result = Speck_Init(&my_speck_cipher, cfg_128_64, ECB, speck128_64_key, my_IV, my_counter);  
   Speck_Encrypt(my_speck_cipher, frame->data, frame_cipher->data);
}
//——————————————————————————————————————————————————————————————————————————————
//   CHASKEY1
//——————————————————————————————————————————————————————————————————————————————
void chaskey1(CANMessage *frame,CANMessage *frame_mac){
  // ——————————————Chaskey8 init——————————————
   uint8_t tag[16];        //Chaskey MAC Result
   uint32_t k[4] = {0x833D3433, 0x009F389F, 0x2398E64F, 0x417ACF39 }; //Key used
   uint32_t k1[4], k2[4];  //Subkeys used
   uint32_t taglen = 8;    //Chaskey MAC Length
  //————————————mac——————————————---
    chaskey_subkeys(k1, k2, k);
    chaskey(tag, taglen, frame->data, 8, k, k1, k2);   
  //————————————compare_mac——————————————
    for(int i=0;i<frame->len;i++){
      if(frame_mac->data[i]!=tag[i])  same_mac=false;
    }
}


void loop(){
   CANMessage frame,frame_mac,frame_cipher,frame_rtr;
//------------------------------------------------Sent RTR----------------------------------------------
      frame.id = 0Xbf28042;
      frame.ext = true ;
//      frame_cipher.ext = true;
      frame_rtr.ext = true;
      frame_mac.ext = true;
      frame.len = 8;
      frame.rtr = 1;
//      frame_cipher.id = frame.id;
//      frame_cipher.len = frame.len;
//      frame_cipher.rtr = frame.rtr;
      frame_rtr.id = frame.id;
      frame_rtr.len = frame.len;
      frame_rtr.rtr = frame.rtr;
      frame_mac.id = frame.id;
      frame_mac.len = frame.len;
      frame_mac.rtr = frame.rtr;


      frame.data[0]=0X11;
      frame.data[1]=0X22;
      frame.data[2]=0X33;
      frame.data[3]=0X44;
      frame.data[4]=0X55;
      frame.data[5]=0X66;
      frame.data[6]=0X77;
      frame.data[7]=0X88;
    
   chaskey1(&frame,&frame_mac);
   encrypt(&frame,&frame_rtr);


//----------------------------------send_rtr-------------------------------------   
    const bool ok_rtr = can.tryToSend (frame_rtr) ;
    if (ok_rtr) {
      Serial.println ("Send_frame_rtr success") ;
    }else{
      Serial.println ("Send_rtr failure") ;
    }
//----------------------------------send_mac-------------------------------------   
    const bool ok_mac = can.tryToSend (frame_mac) ;
    if (ok_mac) {
      Serial.println ("Send_mac_rtr success") ;
    }else{
      Serial.println ("Send_mac_rtr failure") ;
    }
    if(ok_rtr&&ok_mac){
      //gSentFrameCount += 1 ;
      Serial.print ("send RTR frame ID :") ;
      Serial.print(frame.id,HEX);
      Serial.println();
      gSentFrameCount++;
      Serial.println ("-----------------Send success---------------") ;
      
    }else{
      Serial.println ("-----------------Send failure---------------") ;
    }

//------------------------------------------------Sent RTR----------------------------------------------
CANMessage frame_cipher_or_rtr;
   if(can.available()){
    can.receive(frame_cipher_or_rtr);
    can.receive(frame_mac);
//------------------------------------------------Receive RTR----------------------------------------------
    if(frame_cipher_or_rtr.rtr==1){
        frame_cipher_or_rtr.id=0X8428041;
        frame.data[0]=0X31;
        frame.data[1]=0X31;
        frame.data[2]=0X31;
        frame.data[3]=0X31;
        frame.data[4]=0X31;
        frame.data[5]=0X31;
        frame.data[6]=0X31;
        frame.data[7]=0X31;
        frame_cipher_or_rtr.rtr=0;
        frame_mac.rtr=0;
        Serial.println ("Receive rtr from uno2") ;

      chaskey1(&frame,&frame_mac);
      encrypt(&frame,&frame_cipher_or_rtr);
//----------------------------------send_cipher-------------------------------------   
    const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
    if (ok_cipher_or_rtr) {
      Serial.println ("Send_frame_data_cipher success") ;
    }else{
      Serial.println ("Send_frame_data_cipher failure") ;
    }
//----------------------------------send_mac-------------------------------------   
    const bool ok_mac = can.tryToSend (frame_mac) ;
    if (ok_mac) {
      Serial.println ("Send_data_mac success") ;
    }else{
      Serial.println ("Send_data_mac failure") ;
    }
    if(ok_cipher_or_rtr&&ok_mac){
      //gSentFrameCount += 1 ;
      Serial.print ("Send data frame ID :") ;
      Serial.print(frame_cipher_or_rtr.id,HEX);
      Serial.println();
      Serial.println ("-----------------Send success---------------") ;
      
    }else{
      Serial.println ("-----------------Send failure---------------") ;
    }
    }
//------------------------------------------------Receive DATA----------------------------------------------

    else{
      Serial.println ("Receive DATA frame ") ;
      Serial.print ("frame_id: ") ;
      Serial.print(frame_cipher_or_rtr.id,HEX);
    decrypt(&frame,&frame_cipher);
    chaskey1(&frame,&frame_mac);
    Serial.print ("    Received: ") ;
    for(int i=0;i<8;i++)
    {
     Serial.print (frame.data[i],HEX) ;
      }
      Serial.println();
    if(same_mac){
      Serial.println("Same mac, the message is correct ");
    }else  Serial.println("Different mac, the message is not correct");
    Serial.println ("-----------------Receive success---------------") ;
  }
    
    
  
    }else{
     Serial.println ("-----------------Receive faliure---------------") ;
  }
   delay(1000);
}
