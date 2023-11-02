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

//-------------------------------------authentication-----------------------------------------------
  //delay(random(2000));
// 原始共享密钥gk gk_IV cgk_IV
  uint8_t gk[] = {0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71,0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1};
  uint8_t gk_IV[] = {0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71};
  uint8_t cgk_IV[6]={0x11,0x66,0x22,0x55,0x33,0x44};
//随机数R 48bits
  uint8_t R0[6]={};
  for(int i=0;i<6;i++)
  {
    R0[i]=random(256);
  }
  Serial.print("R :");
  for(int i=0;i<6;i++)
  {
    Serial.print(R0[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
//将随机数R0填充到帧中，
  CANMessage frame_R0,frame_cgk,frame_E_T1,frame_MAC1;
  frame_R0.len=6;
  frame_R0.ext = true;
  frame_R0.id= 0x4200020;
  frame_R0.data[0]=R0[0];
  frame_R0.data[1]=R0[1];
  frame_R0.data[2]=R0[2];
  frame_R0.data[3]=R0[3];
  frame_R0.data[4]=R0[4];
  frame_R0.data[5]=R0[5];
  frame_cgk.len=frame_R0.len;
  frame_cgk.ext = frame_R0.ext;
  frame_cgk.id=frame_R0.id;
  Serial.print("R0 :");
  for(int i=0;i<frame_R0.len;i++)
  {
    Serial.print(frame_R0.data[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
//利用MAC，计算cgk 48bits
  ascon_mac_frame(&frame_R0,&frame_cgk,gk,16,gk_IV,16,6);
  //ascon_enc_plaintext(&frame_R0,&frame_cgk,gk,16,gk_IV,16);
  Serial.print("cgk :");
  for(int i=0;i<frame_cgk.len;i++){
    Serial.print(frame_cgk.data[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
//存储到cgk密钥中 48字节
  uint8_t cgk[6];
  for(int i=0;i<frame_cgk.len;i++)
    {
      cgk[i]=frame_cgk.data[i];
    }
//  R0[0]=0x01;
//  R0[1]=0x02;
//  R0[2]=0x03;
//  R0[3]=0x04;
//  R0[4]=0x05;
//  R0[5]=0x06;
  bool first = true;
  while(1)
  {
//step 3  生成T1
    unsigned int T1= 1;
    Serial.print("\nT1 :");
    Serial.println(T1);
//加密cgk 附上T1
    frame_E_T1.len=8;
    frame_E_T1.ext = true;
    frame_E_T1.id=frame_R0.id;
    ascon_enc_frame(&frame_cgk,&frame_E_T1,gk,16,gk_IV,16);
    frame_E_T1.data[6]=T1 >> 8;
    frame_E_T1.data[7]=T1%256;
    Serial.print("E||T1 :");
    for(int i=0;i<frame_E_T1.len;i++){
      Serial.print(frame_E_T1.data[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
//计算MAC
    uint8_t G_ID = 0X20;   // 00100000
    uint8_t ECU_ID = 0x21; // 00100001
  
    uint8_t ID_E_T1[10];
    uint8_t tag_E_T1[8];
//  uint8_t tag_new[8];
    ID_E_T1[0]=G_ID;
    ID_E_T1[1]=ECU_ID;
    ID_E_T1[2]=frame_E_T1.data[0];
    ID_E_T1[3]=frame_E_T1.data[1];
    ID_E_T1[4]=frame_E_T1.data[2];
    ID_E_T1[5]=frame_E_T1.data[3];
    ID_E_T1[6]=frame_E_T1.data[4];
    ID_E_T1[7]=frame_E_T1.data[5];
    ID_E_T1[8]=frame_E_T1.data[6];
    ID_E_T1[9]=frame_E_T1.data[7];
  
    ascon_mac_plaintext(ID_E_T1,10,tag_E_T1,8,gk,16,gk_IV,16);
    Serial.print("ID_G||ID_ECU||E||T1 :");
    for(int i=0;i<10;i++){
      Serial.print(ID_E_T1[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
  
    Serial.print("ID_G||ID_ECU||E||T1 tag :");
    for(int i=0;i<8;i++){
      Serial.print(tag_E_T1[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
  
    frame_MAC1.len=8;
    frame_MAC1.ext= true;
    frame_MAC1.id=frame_R0.id;
    for(int i=0;i<frame_MAC1.len;i++) {frame_MAC1.data[i]=tag_E_T1[i];}
    const bool ok_frame_E_T1=can.tryToSend(frame_E_T1);
    const bool ok_frame_MAC1=can.tryToSend(frame_MAC1);
  
    if(ok_frame_E_T1&&ok_frame_MAC1)
    {
//      Serial.print ("Send_frame_rtr:") ;
//      Serial.println(frame_cipher_or_rtr.rtr);
      Serial.print ("Send E||T1||MAC1 ID :") ;
      Serial.print(frame_E_T1.id,HEX);
      Serial.println();
      
      Serial.println ("-----------------Send E||T1||MAC1 success---------------") ;  

      Serial.print("Send frame_E_T1:");
      for(int i=0;i<frame_E_T1.len;i++)
      {
        Serial.print(frame_E_T1.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println();
      Serial.print("Send frame_MAC1:");
      for(int i=0;i<frame_MAC1.len;i++)
      {
        Serial.print(frame_MAC1.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println();

      bool over_time=false;
      unsigned int T0_0=millis();
        
      CANMessage frame_T2_MAC2,frame_MAC2;
      while(!can.available())
      {
        unsigned int T0_1=millis();
        if(T0_1 - T0_0 > 5000)
        {
          over_time=true;
          break;
        }
      }
      if(over_time) { continue;}
        

        //delay(1000);
      if(can.available())
      {
        can.receive(frame_T2_MAC2);
        Serial.print("Receice frame_T2_MAC2:");
        for(int i=0;i<frame_T2_MAC2.len;i++)
        {
          Serial.print(frame_T2_MAC2.data[i],HEX);
          Serial.print(" ");
        }
        Serial.println();
          
        unsigned int T2;
        unsigned int T3= 3;
        T2 = (frame_T2_MAC2.data[0] << 8)+(frame_T2_MAC2.data[1]);
        Serial.print("T2:");
        Serial.println(T2);
        Serial.print("T3:");
        Serial.println(T3);

        bool T3_T2 = (T3 > T2);

        uint8_t IDG_IDECU_T2[4];
        uint8_t tag_IDG_IDECU_T2[6];
        IDG_IDECU_T2[0]=G_ID;
        IDG_IDECU_T2[1]=ECU_ID;
        IDG_IDECU_T2[2]=T2 >> 8;
        IDG_IDECU_T2[3]=T2 % 256;
        frame_MAC2.len=6;
        frame_MAC2.ext = frame_T2_MAC2.ext;
        frame_MAC2.id= frame_MAC2.id;
        frame_MAC2.data[0]=frame_T2_MAC2.data[2];
        frame_MAC2.data[1]=frame_T2_MAC2.data[3];
        frame_MAC2.data[2]=frame_T2_MAC2.data[4];
        frame_MAC2.data[3]=frame_T2_MAC2.data[5];
        frame_MAC2.data[4]=frame_T2_MAC2.data[6];
        frame_MAC2.data[5]=frame_T2_MAC2.data[7];
          //same_mac = false;
          
//test step 8 failed
//if(first){
//  first=false;
//  frame_MAC2.data[4]=0x00;
//  frame_MAC2.data[5]=0x00;
//}
        ascon_checkTag_plaintext(IDG_IDECU_T2,4,frame_MAC2.data,frame_MAC2.len,cgk,6,cgk_IV,6);

        if(T3_T2 && same_mac)
        {
          Serial.println("check T2 and MAC2 success"); 
          CANMessage frame_success;
          frame_success.ext = true ;
          frame_success.id = 0x4200020;
          frame_success.len = 8;
          for(int i=0;i<frame_success.len;i++)
          {
            frame_success.data[i]=0x00;
          }
          const bool ok_frame_success=can.tryToSend(frame_success);
          if(ok_frame_success)
          {
            Serial.println("send frame_success success");
            Serial.println("authentication passed");
            break;
          }
          else
          {
            Serial.println("send frame_success failed");
          } 
        }
        else
        {
//        continue;
          uint8_t IDG_IDECU_Fail_T2[8];
          uint8_t tag_IDG_IDECU_Fail_T2[6];
          IDG_IDECU_Fail_T2[0]=G_ID;
          IDG_IDECU_Fail_T2[1]=ECU_ID;
          IDG_IDECU_Fail_T2[2]=0xff;
          IDG_IDECU_Fail_T2[3]=0xff;
          IDG_IDECU_Fail_T2[4]=0xff;
          IDG_IDECU_Fail_T2[5]=0xff;
          IDG_IDECU_Fail_T2[6]=T2 >> 8;
          IDG_IDECU_Fail_T2[7]=T2 % 256;
          ascon_checkTag_plaintext(IDG_IDECU_Fail_T2,8,frame_MAC2.data,frame_MAC2.len,gk,16,gk_IV,16);
          if(same_mac)
          {
            Serial.println("check failed frame success   will back to step 3");
            continue;
          }
          else
          {
            Serial.println("check T2 or MAC2 failed");
            CANMessage frame_T3_MAC3_failed;
            frame_T3_MAC3_failed.len=8;
            frame_T3_MAC3_failed.ext = true;
            frame_T3_MAC3_failed.id = 0x4200020;
            uint8_t IDG_IDECU_Fail_T3[8];
            uint8_t tag_IDG_IDECU_Fail_T3[6];
            IDG_IDECU_Fail_T3[0]=G_ID;
            IDG_IDECU_Fail_T3[1]=ECU_ID;
            IDG_IDECU_Fail_T3[2]=0xff;
            IDG_IDECU_Fail_T3[3]=0xff;
            IDG_IDECU_Fail_T3[4]=0xff;
            IDG_IDECU_Fail_T3[5]=0xff;
            IDG_IDECU_Fail_T3[6]=T3 >> 8;
            IDG_IDECU_Fail_T3[7]=T3 % 256;
            ascon_mac_plaintext(IDG_IDECU_Fail_T3,8,tag_IDG_IDECU_Fail_T3,6,gk,16,gk_IV,16);
            frame_T3_MAC3_failed.data[0]=T3 >> 8;
            frame_T3_MAC3_failed.data[1]=T3 % 256;
            frame_T3_MAC3_failed.data[2]=tag_IDG_IDECU_Fail_T3[0];
            frame_T3_MAC3_failed.data[3]=tag_IDG_IDECU_Fail_T3[1];
            frame_T3_MAC3_failed.data[4]=tag_IDG_IDECU_Fail_T3[2];
            frame_T3_MAC3_failed.data[5]=tag_IDG_IDECU_Fail_T3[3];
            frame_T3_MAC3_failed.data[6]=tag_IDG_IDECU_Fail_T3[4];
            frame_T3_MAC3_failed.data[7]=tag_IDG_IDECU_Fail_T3[5];
            Serial.print("frame_T3_MAC3_failed :");
            for(int i=0;i<frame_T3_MAC3_failed.len;i++){
              Serial.print(frame_T3_MAC3_failed.data[i],HEX);
              Serial.print(" ");
            }
            Serial.println();
            const bool ok_frame_T3_MAC3_failed=can.tryToSend(frame_T3_MAC3_failed);
            if(ok_frame_T3_MAC3_failed)
            {
              Serial.print ("Send frame_T3_MAC3_failed ID :") ;
              Serial.print(frame_T3_MAC3_failed.id,HEX);
              Serial.println();
      
              Serial.println ("-----------------send frame_T3_MAC3_failed success---------------") ;  

              Serial.print("Send frame_T3_MAC3_failed:");
              for(int i=0;i<frame_T3_MAC3_failed.len;i++)
              {
                Serial.print(frame_T3_MAC3_failed.data[i],HEX);
                Serial.print(" ");
              }
              Serial.println();

              Serial.println("back to step 3 ");
              continue;
            }
            else
            {
              Serial.println ("-------------send frame_T3_MAC3_failed failed---------------") ;
            }
          }
        }
      }
      else
      {
        Serial.println ("Waiting for Check T2|| MAC2") ;
      }
    }
    else
    {
      Serial.println ("-----------------Send E||T1||MAC1 failure---------------") ;
    }
  }
//-------------------------------------authentication-----------------------------------------------
}
int count=0;
void loop(){
  Serial.print("loop :");
  Serial.println(++count);
     delay(2000);
//   delay(50000);
}
