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

static const byte MCP2515_SCK = 52 ; // SCK input of MCP2515
static const byte MCP2515_SI  = 51 ; // SI input of MCP2515
static const byte MCP2515_SO  = 50 ; // SO output of MCP2515

static const byte MCP2515_CS  = 53 ; // CS input of MCP2515
static const byte MCP2515_INT = 2 ; // INT output of MCP2515
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
  const ACAN2515Mask rxm1 = extended2515Mask (0x01fffff);
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (0x0160041), receive0},
    {extended2515Filter (0x0150022), receive0},
    {extended2515Filter (0x0060021), receive0},
    {extended2515Filter (0x0050021), receive0},
  }; 
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0,rxm1, filters, 4);
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
    uint8_t speck128_64_key[] = {0x00,0x01,0x02,0x03,0x08,0x09,0x0a,0x0b,0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b};
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
   uint8_t speck128_64_key[] = {0x00,0x01,0x02,0x03,0x08,0x09,0x0a,0x0b,0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b};  
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
   uint32_t k[4] = {0x2398E64F, 0x417ACF39,0x833D3433, 0x009F389F }; //Key used
   uint32_t k1[4], k2[4];  //Subkeys used
   uint32_t taglen = 8;    //Chaskey MAC Length
  //————————————mac——————————————---
   same_mac=true;
    chaskey_subkeys(k1, k2, k);
    chaskey(tag, taglen, frame->data, 8, k, k1, k2);   
  //————————————compare_mac——————————————
    for(int i=0;i<frame->len;i++){
      if(frame_mac->data[i]!=tag[i])  same_mac=false;
    }
}
void chaskey1_1(CANMessage *frame,CANMessage *frame_mac){
  // ——————————————Chaskey8 init——————————————
   uint8_t tag[16];        //Chaskey MAC Result
   uint32_t k[4] = {0x2398E64F, 0x417ACF39,0x833D3433, 0x009F389F }; //Key used
   uint32_t k1[4], k2[4];  //Subkeys used
   uint32_t taglen = 8;    //Chaskey MAC Length
   //————————————mac——————————————---
   chaskey_subkeys(k1, k2, k);
   chaskey(frame_mac->data, taglen, frame->data, 8, k, k1, k2);
}


void loop(){
   CANMessage frame,frame_mac,frame_cipher_or_rtr;
//------------------------------------------------Sent RTR----------------------------------------------
    switch (gSentFrameCount % 2) {
    case 0 : // send RTR kuayu (uno2)
      frame.id = 0xbe60021 ;
      frame_cipher_or_rtr.id = frame.id;
      frame_mac.id = frame.id;
      
      frame.ext = true ;
      frame_cipher_or_rtr.ext = frame.ext;
      frame_mac.ext = frame.ext;
      
      frame.rtr = true;
      frame_mac.rtr = frame.rtr;
      frame_cipher_or_rtr.rtr = frame.rtr;
      break ;
      
     case 1 : // send RTR bu kua yu(uno3)
      frame.id = 0x7f50021 ;
      frame_cipher_or_rtr.id = frame.id;
      frame_mac.id = frame.id;
      
      frame.ext = true ;
      frame_cipher_or_rtr.ext = frame.ext;
      frame_mac.ext = frame.ext;
      
      frame.rtr = true;
      frame_mac.rtr = frame.rtr;
      frame_cipher_or_rtr.rtr = frame.rtr;
       break ;
    }

//----------------------------------send_rtr-------------------------------------   
    const bool ok_rtr = can.tryToSend (frame_cipher_or_rtr) ;
//    if (ok_rtr) {
//      Serial.println ("Send_frame_rtr success") ;
//    }else{
//      Serial.println ("Send_rtr failure") ;
//    }
//----------------------------------send_mac-------------------------------------   
    const bool ok_mac = can.tryToSend (frame_mac) ;
//    if (ok_mac) {
//      Serial.println ("Send_mac_rtr success") ;
//    }else{
//      Serial.println ("Send_mac_rtr failure") ;
//    }
    if(ok_rtr&&ok_mac){
      Serial.print ("Send_frame_rtr:") ;
      Serial.println(frame_cipher_or_rtr.rtr);
      Serial.print ("send RTR frame ID :") ;
      Serial.print(frame.id,HEX);
      Serial.println();
      gSentFrameCount++;
      Serial.println ("-----------------Send success---------------") ;  
    }else{
      Serial.println ("-----------------Send failure---------------") ;
    }
    
   if(can.available()){
    can.receive(frame_cipher_or_rtr);
    can.receive(frame_mac);
//------------------------------------------------Receive RTR----------------------------------------------
    if(frame_cipher_or_rtr.rtr==true){
      if(frame_cipher_or_rtr.id==0x7f60041){
        frame_cipher_or_rtr.id=0x8260021;
        frame_mac.id = frame_cipher_or_rtr.id;
        frame_cipher_or_rtr.len = 8;
        frame_mac.len = 8;
        frame_cipher_or_rtr.rtr=false;
        frame_mac.rtr=false;
        frame.data[0]=0X12;
        frame.data[1]=0X12;
        frame.data[2]=0X12;
        frame.data[3]=0X12;
        frame.data[4]=0X12;
        frame.data[5]=0X12;
        frame.data[6]=0X12;
        frame.data[7]=0X12;
 
        Serial.println ("Receive RTR from uno2") ;
        Serial.println ("-----------------Receive RTR success---------------") ;
      }
      else if(frame_cipher_or_rtr.id==0X7f50022){
        frame_cipher_or_rtr.id=0X4450021;
        frame_mac.id = frame_cipher_or_rtr.id;
        frame_cipher_or_rtr.len = 8;
        frame_mac.len = 8;
        frame_cipher_or_rtr.rtr=false;
        frame_mac.rtr=false;
        frame.data[0]=0X13;
        frame.data[1]=0X13;
        frame.data[2]=0X13;
        frame.data[3]=0X13;
        frame.data[4]=0X13;
        frame.data[5]=0X13;
        frame.data[6]=0X13;
        frame.data[7]=0X13;

        Serial.println ("Receive RTR from uno3") ;
        Serial.println ("-----------------Receive RTR success---------------") ;
      }
      chaskey1_1(&frame,&frame_mac);
      encrypt(&frame,&frame_cipher_or_rtr);
//----------------------------------send_cipher-------------------------------------   
    const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
//    if (ok_cipher_or_rtr) {
//      Serial.println ("Send_frame_data_cipher success") ;
//    }else{
//      Serial.println ("Send_frame_data_cipher failure") ;
//    }
//----------------------------------send_mac-------------------------------------   
    const bool ok_mac = can.tryToSend (frame_mac) ;
//    if (ok_mac) {
//      Serial.println ("Send_data_mac success") ;
//    }else{
//      Serial.println ("Send_data_mac failure") ;
//    }
    if(ok_cipher_or_rtr&&ok_mac){
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
    decrypt(&frame,&frame_cipher_or_rtr);
    chaskey1(&frame,&frame_mac);
    
    Serial.print ("    Received: ") ;
    for(int i=0;i<8;i++)
    {
     Serial.print (frame.data[i],HEX) ;
      }
      Serial.println();
   
    if(same_mac){
      Serial.println("Same mac, the message is correct ");
      Serial.println ("-----------------Receive success---------------") ;
    }else  Serial.println("Different mac, the message is not correct");
    
  }
    }
//else{
//     Serial.println ("-----------------Receive faliure---------------") ;
//  }
   delay(2000);
}
