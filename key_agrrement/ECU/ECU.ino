//——————————————————————————————————————————————————————————————————————————————
//   ECU1 communication with ECU3 in domain1, ECU2 in domain2
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

static uint32_t gSentFrameCount = 0 ;
bool same_mac=false;

uint8_t sk1[] = {0xf1,0xe1,0xd1,0xc1,0xb1,0xa1,0x91,0x81,0x71,0x61,0x51,0x41,0x31,0x21,0x11,0x01};
uint8_t sk1_IV[] = {0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71};
uint8_t cgk1 = {};
uint8_t cgk1_IV[6]={0x11,0x66,0x22,0x55,0x33,0x44};



unsigned int timecnt=micros();
//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————
static void receive0 (const CANMessage & inMessage) {
    Serial.println();
    Serial.println ("Function_Receive0 ") ;
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


void ascon_enc_frame(CANMessage *frame,CANMessage *frame_cipher,uint8_t *key,int keylen,uint8_t *IV,int IVlen)
{
  //  encry_begin  
  Ascon128 te;
  te.clear();
  te.setKey(key,keylen); //around 100us
  te.setIV(IV,IVlen);
  te.encrypt(frame_cipher->data,frame->data,frame->len);

}
void ascon_mac_frame(CANMessage *frame,CANMessage *frame_mac,uint8_t *key,int keylen,uint8_t *IV,int IVlen,int mac_len)
{
  //  encry_begin  
  Ascon128 te;
  te.clear();
  te.setKey(key,keylen); //around 100us
  te.setIV(IV,IVlen);
   //  auth_begin
  te.addAuthData(frame->data,frame->len);
  te.computeTag(frame_mac->data,mac_len);
}

void ascon_dec_frame(CANMessage *frame,CANMessage *frame_cipher,uint8_t *key,int keylen,uint8_t *IV,int IVlen){

  //  decry_begin  
  Ascon128 te;
  te.clear();
  te.setKey(key,keylen); //around 100us
  te.setIV(IV,IVlen);
  te.decrypt(frame->data,frame_cipher->data,frame_cipher->len);

}  

void ascon_checkTag_frame(CANMessage *frame,CANMessage *frame_mac,uint8_t *key,int keylen,uint8_t *IV,int IVlen){
  //  decry_begin  
  Ascon128 te;
  te.clear();
  te.setKey(key,16); //around 100us
  te.setIV(IV,16);

  same_mac=te.checkTag(frame_mac->data,frame->len);
}  

void ascon_mac_plaintext(uint8_t *plaintext,int plaintext_len,uint8_t *tag ,int tag_len,uint8_t *key,int keylen,uint8_t *IV,int IVlen)
{
  //  encry_begin  
  Ascon128 te;
  te.clear();
  te.setKey(key,keylen); //around 100us
  te.setIV(IV,IVlen);
   //  auth_begin
  te.addAuthData(plaintext,plaintext_len);
  te.computeTag(tag,tag_len);
}
void ascon_checkTag_plaintext(uint8_t *plaintext,int plaintext_len,uint8_t *tag ,int tag_len,uint8_t *key,int keylen,uint8_t *IV,int IVlen){
  //  decry_begin  
  Ascon128 te;
  te.clear();
  te.setKey(key,keylen); //around 100us
  te.setIV(IV,IVlen);
  te.addAuthData(plaintext,plaintext_len);
  same_mac=te.checkTag(tag,tag_len);
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
  const ACAN2515Mask rxm1 = extended2515Mask (0x1ff800ff);
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (0x4300141), receive0},
    {extended2515Filter (0x4300122), receive0},
    {extended2515Filter (0x8200121), receive0},
    {extended2515Filter (0x4400121), receive0},
    {extended2515Filter (0x4200020), receive0},
  }; 
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0,rxm1, filters, 5);
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}


void loop(){
  CANMessage frame,frame_mac,frame_cipher_or_rtr;
   //----------------------------------receive------------------------------------- 
   if(can.available()){
    can.receive(frame_mac);
     while(!can.available());
    can.receive(frame_cipher_or_rtr);
    Serial.print ("frame_mac ID :") ;
    Serial.print(frame_mac.id,HEX);

     for(int i=0;i<frame_mac.len;i++)
      {
        Serial.print(frame_mac.data[i],HEX);
        Serial.print(" ");
      }
    uint8_t G1_ID = 0X20;
    uint8_t ECU_ID = 0x21; 
    uint8_t MAC1[4];
    uint8_t ID_seed[6];
    uint8_t seed[4];
//提取seed,mac1
    if(frame_mac.id == 0x4200020){
      for(int i=0;i<4;i++){
        seed[i] = frame_mac.data[i];
        MAC1[i] = frame_mac.data[i+4];
      }

    Serial.println();
    Serial.print("seed :");
    for(int i=0;i<4;i++)
    {
      Serial.print(seed[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
      
    ID_seed[0] = ECU_ID;
    ID_seed[1] = G1_ID;  
    for(int i=0;i<4;i++){
    ID_seed[i+2] = seed[i];
    }
//对比mac1
      ascon_checkTag_plaintext(ID_seed,6,MAC1,4,sk1,16,sk1_IV,16);
      if(same_mac){
//计算new_key
      uint8_t key1[16];
      ascon_mac_plaintext(seed,4,key1,16,cgk1,6,cgk1_IV,6);
//构建MAC2
      uint8_t ID_KEY_seed[22];
      ID_KEY_seed[0] = ECU_ID;
      ID_KEY_seed[1] = G1_ID;
      for(int i=0;i<16;i++){
        ID_KEY_seed[i+2] = key1[i];
      }
      ID_KEY_seed[18] = seed[0];
      ID_KEY_seed[19] = seed[1];
      ID_KEY_seed[20] = seed[2];
      ID_KEY_seed[21] = seed[3];
//计算MAC2
      uint8_t MAC2[8];
      ascon_mac_plaintext(ID_KEY_seed,22,MAC2,8,sk1,16,sk1_IV,16);
//--------------send--------------------
      CANMessage frame_MAC2;
      frame_MAC2.ext = true ;
      frame_MAC2.id = 0x4200000;
      frame_MAC2.len = 8;
      for(int i=0;i<frame_MAC2.len;i++){
        frame_MAC2.data[i] = MAC2[i];
      }
      const bool ok_frame_MAC2=can.tryToSend(frame_MAC2);
      if(ok_frame_MAC2){
         Serial.println ("-----------------Send mac2 success---------------") ;  
      }      
     }
    }
  }
}
