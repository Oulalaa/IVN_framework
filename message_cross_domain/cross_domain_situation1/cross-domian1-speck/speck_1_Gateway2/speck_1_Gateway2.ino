//——————————————————————————————————————————————————————————————————————————————
//  ACAN2515 Demo in normal mode， cross domain，getaway1
//——————————————————————————————————————————————————————————————————————————————
#include <SPI.h>
extern "C"{
#include <speck.h>
#include <chaskey.h>
#include <mcp2515.h>
}
#include <ACAN2515.h>
static const byte MCP2515_SCK = 52 ; // SCK input of MCP2515
static const byte MCP2515_SI  = 51 ; // SI input of MCP2515
static const byte MCP2515_SO  = 50 ; // SO output of MCP2515

static const byte MCP2515_CS  = 53 ; // CS input of MCP2515
static const byte MCP2515_INT = 2 ; // INT output of MCP2515
ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;
static const uint32_t QUARTZ_FREQUENCY = 16UL  * 1000UL *1000UL ; // 16 MHz
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
  const ACAN2515Mask rxm0 = extended2515Mask (0x1c1800e0);
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (0x8280121), receive0},
    { extended2515Filter (0x4200141), receive0},
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
   uint32_t k[4] = {0x2398E64F, 0x417ACF39,0x833D3433, 0x009F389F }; //Key used
   uint32_t k1[4], k2[4];  //Subkeys used
   uint32_t taglen = 8;    //Chaskey MAC Length
  //————————————mac——————————————---
    chaskey_subkeys(k1, k2, k);
    chaskey(tag, taglen, frame->data, 8, k, k1, k2);   
  //————————————compare_mac——————————————
   same_mac=true;
    for(int i=0;i<frame->len;i++){
      if(frame_mac->data[i]!=tag[i])  same_mac=false;
    }
}
//——————————————————————————————————————————————————————————————————————————————
//   CHASKEY2
//——————————————————————————————————————————————————————————————————————————————
void chaskey2(CANMessage *frame,CANMessage *frame_mac){
   // ——————————————Chaskey8 init——————————————
    uint8_t tag[8];
   uint32_t k[4] = {0x833D3433, 0x009F389F, 0x2398E64F, 0x417ACF39}; //Key used
   uint32_t k1[4], k2[4];  //Subkeys used
   uint32_t taglen = 8;    //Chaskey MAC Length
   //————————————mac——————————————---
   chaskey_subkeys(k1, k2, k);
   chaskey(frame_mac->data, taglen, frame->data, 8, k, k1, k2);
} 

void loop(){
//   CANMessage frame;
//   if(can.available()){
//    can.receive(frame);
//    if(frame.rtr==1){
//      Serial.print("this is a RTR message, its id:");
//      Serial.println(frame.id);
//      if((frame.id&0X1c1800e0)== 0X4000040){
//        frame.id = 0X4280141;
//        const bool ok= can.tryToSend (frame) ;
//        if(ok){
//          Serial.print("after reply RTR_id:");
//          Serial.println(frame.id);
//          Serial.println ("-----------------Mega2 reply RTR to getaway list succeed---------------") ;
//        }else{
//          Serial.println ("-----------------Mega2 reply RTR to getaway list failure---------------") ;
//        }
//      }
//      else if((frame.id&0X1c1800e0)==0x8080020){
//        frame.id = 0X8300121;
//        const bool ok= can.tryToSend (frame) ;
//        if(ok){
//         Serial.print("after reply RTR_id:");
//          Serial.println(frame.id);
//          Serial.println ("-----------------Mega2 reply RTR to own domain succeed---------------") ;
//        }else{
//          Serial.println ("-----------------Mega2 reply RTR to own domain failure---------------") ;
//        }
//      }
//    }
//    else{
//       Serial.print("this is a DATA message, its id:");
//       Serial.println(frame.id);
//       if((frame.id&0x1c1800e0)==0x8080020){
//        frame.id = 0x4200141;
//        const bool ok= can.tryToSend (frame) ;
//        if(ok){
//          Serial.print("after reply DATA_id:");
//          Serial.println(frame.id);
//          Serial.println ("-----------------Mega2 reply DATA to own domain succeed---------------") ;
//        }else{
//          Serial.println ("-----------------Mega2 reply DATA to own domain failure---------------") ;
//        }
//      }
//      else if((frame.id&0x1c1800e0)==0X4000040){
//        frame.id=0x4280141;
//        const bool ok= can.tryToSend (frame) ;
//        if(ok){
//          Serial.print("after reply DATA_id:");
//          Serial.println(frame.id);
//          Serial.println ("-----------------Mega2 reply DATA to getaway list succeed---------------") ;
//        }else{
//          Serial.println ("-----------------Mega2 reply DATA to getaway list failure---------------") ;
//        }
//      }
//     }
//   }
   CANMessage frame,frame_mac,frame_cipher_or_rtr;
   if(can.available()){
    can.receive(frame_cipher_or_rtr);
    can.receive(frame_mac);
    Serial.print ("Send_frame_rtr:") ;
    Serial.println(frame_cipher_or_rtr.rtr);
//------------------------------------------------Receive RTR----------------------------------------------
    if(frame_cipher_or_rtr.rtr==true){
      Serial.print("this is a RTR message, its id:");
      Serial.println(frame_cipher_or_rtr.id,HEX);
      if((frame_cipher_or_rtr.id&0x1c1800e0)==0x4000040){
        frame_cipher_or_rtr.id = 0X4280141;
        frame_mac.id = frame_cipher_or_rtr.id;
        //----------------------------------send_RTR-------------------------------------      
        const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
        const bool ok_mac = can.tryToSend (frame_mac) ;
//         if (ok_cipher_or_rtr) {
//        Serial.println ("Send_frame_cipher success") ;
//      }else{
//        Serial.println ("Send_frame_cipher failure") ;
//      }
//        if (ok_mac) {
//        Serial.println ("Send_mac success") ;
//      }else{
//        Serial.println ("Send_mac failure") ;
//      }
        if(ok_cipher_or_rtr&&ok_mac){
          Serial.print("after reply RTR_id:");
          Serial.println(frame_cipher_or_rtr.id,HEX);
          Serial.println ("-----------------Mega2 reply RTR to getaway list succeed---------------") ;
        }else{
          Serial.println ("-----------------Mega2 reply RTR to getaway list failure---------------") ;
        }
      }
      else if((frame_cipher_or_rtr.id&0X1c1800e0)==0x8080020){
        frame_cipher_or_rtr.id = 0X8300121;
        frame_mac.id = frame_cipher_or_rtr.id;
        //----------------------------------send_RTR-------------------------------------
        const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
        const bool ok_mac = can.tryToSend (frame_mac) ;
        if(ok_cipher_or_rtr&&ok_mac){
          Serial.print("after reply RTR_id:");
          Serial.println(frame_cipher_or_rtr.id,HEX);
          Serial.println ("-----------------Mega2 reply RTR to own domain succeed---------------") ;
        }else{
          Serial.println ("-----------------Mega2 reply RTR to own domain failure---------------") ;
        }
      }
    }
//------------------------------------------------Receive DATA----------------------------------------------
    else{
       Serial.print("this is a DATA message, its id:");
       Serial.println(frame_cipher_or_rtr.id,HEX);
       if((frame_cipher_or_rtr.id&0X1c1800e0)==0x8080020){
        decrypt(&frame,&frame_cipher_or_rtr);
        chaskey1(&frame,&frame_mac);
        if(same_mac){
          Serial.println ("-----------------Mega2 receive success---------------") ;
          chaskey2(&frame,&frame_mac);
          encrypt(&frame,&frame_cipher_or_rtr);
        
        frame.id = 0x4200141;
        frame_cipher_or_rtr.id = frame.id;
        frame_mac.id = frame.id;
      //----------------------------------send_cipher-------------------------------------      
      const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
      if (ok_cipher_or_rtr) {
        Serial.println ("Send_frame_cipher success") ;
      }else{
        Serial.println ("Send_frame_cipher failure") ;
      }
    //----------------------------------send_mac-------------------------------------   
      const bool ok_mac = can.tryToSend (frame_mac) ;
      if (ok_mac) {
        Serial.println ("Send_mac success") ;
      }else{
        Serial.println ("Send_mac failure") ;
      }
      if(ok_cipher_or_rtr&&ok_mac){
        Serial.print("after reply DATA_id.id:");
        Serial.println(frame.id,HEX);
          Serial.println ("-----------------Mega2 reply DATA to own domain succeed---------------") ;
      }else{
        Serial.println ("-----------------Mega2 reply DATA to own domain failure---------------") ;
      }
     }
   }
     else if((frame_cipher_or_rtr.id&0x1c1800e0)==0X4000040){
      frame.id=0x4280141;
      frame_cipher_or_rtr.id = frame.id;
      frame_mac.id = frame.id;


       for(int i=0;i<8;i++)
    {
     Serial.print (frame_cipher_or_rtr.data[i],HEX) ;
      }
      Serial.println();
      
      //----------------------------------send_cipher-------------------------------------   
      const bool ok_cipher_or_rtr = can.tryToSend (frame_cipher_or_rtr) ;
      if (ok_cipher_or_rtr) {
        Serial.println ("Send_frame_cipher success") ;
      }else{
        Serial.println ("Send_frame_cipher failure") ;
      }
    //----------------------------------send_mac-------------------------------------   
      const bool ok_mac = can.tryToSend (frame_mac) ;
      if (ok_mac) {
        Serial.println ("Send_mac success") ;
      }else{
        Serial.println ("Send_mac failure") ;
      }
      if(ok_cipher_or_rtr&&ok_mac){
        Serial.print("after reply DATA_id:");
        Serial.println(frame.id,HEX);
        Serial.println ("-----------------Mega2 reply DATA to getaway list succeed---------------") ;
      }else{
        Serial.println ("-----------------Mega2 reply DATA to getaway list failure---------------") ;
      }
     }
    }
   }
   delay(2000);
}
