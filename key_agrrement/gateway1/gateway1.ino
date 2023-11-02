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

// 原始共享密钥gk gk_IV cgk_IV
  uint8_t sk1[] = {0xf1,0xe1,0xd1,0xc1,0xb1,0xa1,0x91,0x81,0x71,0x61,0x51,0x41,0x31,0x21,0x11,0x01};
  uint8_t sk1_IV[] = {0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71};
  uint8_t sk2[]= {0xf2,0xe2,0xd2,0xc2,0xb2,0xa2,0x92,0x82,0x72,0x62,0x52,0x42,0x32,0x22,0x12,0x02};
  uint8_t sk2_IV[] ={0x82,0x92,0xA2,0xB2,0xC2,0xD2,0xE2,0xF2,0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72};
  uint8_t cgk1[6];
  uint8_t cgk1_IV[6]={0x11,0x66,0x22,0x55,0x33,0x44};
  uint8_t cgk2[6];
  uint8_t cgk2_IV[6]={0x22,0x44,0x33,0x77,0x55,0x88};


//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————
static void receive0 (const CANMessage & inMessage) {
    Serial.println();
    Serial.println ("Function_Receive0 ") ;
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
  const ACAN2515Mask rxm1 = extended2515Mask (0x1c1800e0);
  const ACAN2515AcceptanceFilter filters [] = {
    { extended2515Filter (0x8200121), receive0},
    { extended2515Filter (0x4280141), receive0},
//------------------------------------------------------------------
    { extended2515Filter (0x4200000), receive0},
//------------------------------------------------------------------
  }; 
  const uint32_t errorCode = can.begin (settings, [] { can.isr () ; }, rxm0,rxm1, filters, 3);
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}


void loop(){
 //------------------------密钥协商--------------------------------------
//随机数seed 32bits
  uint8_t seed[4]={};
  for(int i=0;i<4;i++)
  {
    seed[i]=random(256);
  }
  Serial.println();
  Serial.print("seed :");
  for(int i=0;i<4;i++)
  {
    Serial.print(seed[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
//构造ID_seed
    uint8_t G1_ID = 0X20;   // 00100000
    uint8_t G2_ID = 0X40;   // 01000000
    uint8_t ECU_ID = 0x21; // 00100001
    uint8_t ID_seed[6]={};
    uint8_t MAC1[4];
    ID_seed[0]=ECU_ID;
    ID_seed[1]=G1_ID;
    ID_seed[2]=seed[0];
    ID_seed[3]=seed[1];
    ID_seed[4]=seed[2];
    ID_seed[5]=seed[3];
//计算MAC1
    ascon_mac_plaintext(ID_seed,6,MAC1,4,sk1,16,sk1_IV,16);
//构造seed_MAC1帧
    CANMessage frame_seed_MAC1,frame_none;
    frame_none.len=8;
    frame_none.ext = true;
    frame_none.id= 0x4200020;
    frame_seed_MAC1.len=8;
    frame_seed_MAC1.ext = true;
    frame_seed_MAC1.id= 0x4200020;
    frame_seed_MAC1.data[0]=seed[0];
    frame_seed_MAC1.data[1]=seed[1];
    frame_seed_MAC1.data[2]=seed[2];
    frame_seed_MAC1.data[3]=seed[3];
    frame_seed_MAC1.data[4]=MAC1[0];
    frame_seed_MAC1.data[5]=MAC1[1];
    frame_seed_MAC1.data[6]=MAC1[2];
    frame_seed_MAC1.data[7]=MAC1[3];
     for(int i=0;i<8;i++){
        frame_none.data[i] = frame_seed_MAC1.data[i];
      }
//--------------------------send---------------------------------
    const bool ok_frame_none=can.tryToSend(frame_none);
    const bool ok_frame_seed_MAC1=can.tryToSend(frame_seed_MAC1);
    if(ok_frame_seed_MAC1&&ok_frame_none)
    {    
      Serial.print ("Send seed_MAC1 ID :") ;
      Serial.print(frame_seed_MAC1.id,HEX);
      Serial.println();
      
      Serial.println ("-----------------Send  seed||MAC1 success---------------") ; 
      Serial.print("Send frame_seed_MAC1:");
      for(int i=0;i<frame_seed_MAC1.len;i++)
      {
        Serial.print(frame_seed_MAC1.data[i],HEX);
        Serial.print(frame_none.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println(); 
    }
//计算更新密钥NEW_KEY
      uint8_t key1[16];
      ascon_mac_plaintext(seed,4,key1,16,cgk1,6,cgk1_IV,6);

//-----------------------receive-----------------------------------
     CANMessage frame_MAC2;
     while(!can.available()){
     }
     if(can.available()){
      can.receive(frame_MAC2);
      Serial.print("Receice frame_MAC2:");
        for(int i=0;i<frame_MAC2.len;i++)
        {
          Serial.print(frame_MAC2.data[i],HEX);
          Serial.print(" ");
        }
        Serial.println();
////提取mac2
//    uint8_t MAC2_tag[8];
//    for(int i=0;i<frame_MAC2.len;i++){
//    MAC2_tag[i] = frame_MAC2.data[i];
//        }
//组装ID_new_key_seed
    uint8_t MAC2[22];
      MAC2[0] = ECU_ID;
      MAC2[1] = G1_ID;
     for(int i=0;i<16;i++){
        MAC2[i+2] = key1[i];
      }
      MAC2[18] = seed[0];
      MAC2[19] = seed[1];
      MAC2[20] = seed[2];
      MAC2[21] = seed[3];
//对比MAC2      
     ascon_checkTag_plaintext(MAC2,22,frame_MAC2.data,frame_MAC2.len,sk1,16,sk1_IV,16);
      if(same_mac){
         Serial.println("check MAC2 success"); 
//向网关列表转发
          CANMessage frame_cgk1,frame_getaway_update_cgk,frame_getaway_update_seed_mac;
          frame_cgk1.ext = true ;
          frame_cgk1.id = 0x1c0100e0;
          frame_cgk1.len = 8;
          frame_getaway_update_cgk.ext = true ;
          frame_getaway_update_cgk.id = 0x1c0100e0;
          frame_getaway_update_cgk.len = 8;
          frame_getaway_update_seed_mac.ext = true ;
          frame_getaway_update_seed_mac.id = 0x1c0100e0;
          frame_getaway_update_seed_mac.len = 8;
//计算用SK2加密后的key
          for(int i=0;i<6;i++){
            frame_cgk1.data[i] = cgk1[i];
          }   
          ascon_enc_frame(&frame_cgk1,&frame_getaway_update_cgk,sk2,16,sk2_IV,16);
//构造网关ID_key
          uint8_t ID_key[18];
           ID_key[0] = G1_ID;
           ID_key[1] = G2_ID;
          for(int i=0;i<16;i++){
            ID_key[i+2] = key1[i];
          }
//计算MAC
          uint8_t MAC[4];
          ascon_mac_plaintext(ID_key,18, MAC,4,sk2,16,sk2_IV,16);
//封装seed_mac
          for(int i=0;i<4;i++){
            frame_getaway_update_seed_mac.data[i] = seed[i]; 
            frame_getaway_update_seed_mac.data[i+4] = MAC[i];
          }      
//---------------send mac---------------------------
      const bool ok_frame_getaway_update_cgk=can.tryToSend(frame_getaway_update_cgk);
      const bool ok_frame_getaway_update_seed_mac=can.tryToSend(frame_getaway_update_seed_mac);
            if(ok_frame_getaway_update_cgk&&ok_frame_getaway_update_seed_mac){

            Serial.println ("-----------------Send mac_key to getaway success---------------") ;   
            }
       }
      }     
//---------------------receive---------------------------
  CANMessage frame,frame_mac,frame_cipher_or_rtr;
    if(can.available()){
    can.receive(frame_cipher_or_rtr);
    can.receive(frame_mac);
        uint8_t G1_ID = 0X20;
        uint8_t G2_ID = 0X40;
        ascon_dec_frame(&frame,&frame_cipher_or_rtr,sk1,16,sk1_IV,16);
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
      ascon_mac_plaintext(seed,4,key2,16,cgk2,6,cgk2_IV,6);

//验证mac
      ID_key2[0] = G2_ID;
      ID_key2[1] = G1_ID;
      for(int i=0;i<16;i++){
        ID_key2[i+2] = key2[i];
      }
      ascon_checkTag_plaintext(ID_key2,18,mac,4,sk1,16,sk1_IV,16);
      if(same_mac){
        Serial.println ("网关2密钥已更新") ;
      }
 }
   delay(1000);
}
