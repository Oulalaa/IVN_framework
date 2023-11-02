#include <SPI.h>
#include <ACAN2515.h>
#include<Ascon128.h>

static const byte MCP2515_SCK = 52 ; // SCK input of MCP2515
static const byte MCP2515_SI  = 51 ; // SI input of MCP2515
static const byte MCP2515_SO  = 50 ; // SO output of MCP2515

static const byte MCP2515_CS  = 53 ; // CS input of MCP2515
static const byte MCP2515_INT = 2 ; // INT output of MCP2515
static ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;
static const uint32_t QUARTZ_FREQUENCY = 16UL  * 1000UL * 1000UL ; // 16 MHz  
uint32_t timecnt = micros();
bool same_mac= false;
// 原始共享密钥gk gk_IV cgk_IV
  uint8_t sk1[] = {0xf1,0xe1,0xd1,0xc1,0xb1,0xa1,0x91,0x81,0x71,0x61,0x51,0x41,0x31,0x21,0x11,0x01};
  uint8_t sk1_IV[] = {0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71};
  uint8_t sk2[]= {0xf2,0xe2,0xd2,0xc2,0xb2,0xa2,0x92,0x82,0x72,0x62,0x52,0x42,0x32,0x22,0x12,0x02};
  uint8_t sk2_IV[] ={0x82,0x92,0xA2,0xB2,0xC2,0xD2,0xE2,0xF2,0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72};
  uint8_t cgk1[6];
  uint8_t cgk1_IV[6]={0x11,0x66,0x22,0x55,0x33,0x44};
  uint8_t cgk2[6];
  uint8_t cgk2_IV[6]={0x22,0x44,0x33,0x77,0x55,0x88};


static void receive0 (const CANMessage & inMessage) {
    Serial.println();
    Serial.println ("Function_Receive0 ") ;
}

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
  te.computeTag(frame_mac->data,8);
  timecnt = micros()-timecnt;
  Serial.print("mac_timecnt: ");
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
  te.setKey(key,16); //around 100us
  te.setIV(IV,16);
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

void setup() {
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
  const ACAN2515Mask rxm1 = extended2515Mask (0x1c1800e0);
  const ACAN2515AcceptanceFilter filters [] = {
  //方案一
    {extended2515Filter (0x4200141), receive0},
    {extended2515Filter (0x8280121), receive0},
  //方案二
    {extended2515Filter (0x4260041), receive0},
    {extended2515Filter (0x1c0100e0), receive0},
  //方案三
//    {extended2515Filter (0x4200141), receive0},
//    {extended2515Filter (0x8280121), receive0},
  }; 
  const uint32_t errorCode = can.begin (settings, []() { can.isr(); }, rxm0,rxm1, filters, 4);
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

void loop() {
//---------------------receive---------------------------
  CANMessage frame,frame_mac,frame_cipher_or_rtr;
    if(can.available()){
      Serial.println("eeeeeeee"); 
    //  can.receive(frame_mac);
    can.receive(frame_cipher_or_rtr);
    while(!can.available());
    can.receive(frame_mac);
    Serial.print(frame_mac.id,HEX);
    Serial.print(frame_cipher_or_rtr.id,HEX);

     Serial.print("MAC:");
      for(int i=0;i<frame_cipher_or_rtr.len;i++)
      {
        Serial.print(frame_cipher_or_rtr.data[i],HEX);
       // Serial.print(frame_none.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println();     
   // can.receive(frame_cipher_or_rtr);
        uint8_t G1_ID = 0X20;
        uint8_t G2_ID = 0X40;
        ascon_dec_frame(&frame,&frame_cipher_or_rtr,sk2,16,sk2_IV,16);
        uint8_t seed[4];
        uint8_t mac[4];
        uint8_t key2[16];
        uint8_t ID_key2[18];
        for(int i=0;i<4;i++){
          seed[i] = frame_mac.data[i];
          mac[i] = frame_mac.data[i+4];
        }
//更新cgk2
      for(int i=0;i<6;i++){
        cgk2[i] = frame.data[i];
      }
//计算新密钥
      ascon_mac_plaintext(seed,4,key2,16,cgk1,6,cgk1_IV,6);
//验证mac
      ID_key2[0] = G1_ID;
      ID_key2[1] = G2_ID;
      for(int i=0;i<16;i++){
        ID_key2[i+2] = key2[i];
      }
      ascon_checkTag_plaintext(ID_key2,18,mac,4,sk2,16,sk2_IV,16);
      if(same_mac){
        Serial.println ("网关2密钥已更新") ;
      }
 }
  //delay(500);
}
