//——————————————————————————————————————————————————————————————————————————————
//  
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
//-------------------------------------authentication-------------------------------------------------

  uint8_t gk[] = {0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71,0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1};
  uint8_t gk_IV[] = {0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71};
  CANMessage frame_E_T1,frame_MAC1,frame_cgk,frame_T2_MAC2;
  bool first = true;
  while(1)
  {
    while(!can.available());
    if(can.available()){
      can.receive(frame_E_T1);
      while(!can.available());
      can.receive(frame_MAC1);
      Serial.print("\nReceive E||T1 :");
      for(int i=0;i<frame_E_T1.len;i++)
      {
        Serial.print(frame_E_T1.data[i],HEX);
        Serial.print(" ");
      }
       Serial.println();

      Serial.print("frame_MAC1 :");
      for(int i=0;i<frame_MAC1.len;i++)
      {
        Serial.print(frame_MAC1.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println();
      uint8_t ID_E_T1[10];
      uint8_t tag_E_T1[8];
      uint8_t G_ID = 0X20;   // 00100000
      uint8_t ECU_ID = 0x21; // 00100001
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
      ascon_checkTag_plaintext(ID_E_T1,10,frame_MAC1.data,frame_MAC1.len,gk,16,gk_IV,16);
      Serial.print("check MAC1: ");
      Serial.println(same_mac);
      unsigned int T1= (frame_E_T1.data[6] << 8) + frame_E_T1.data[7];
      unsigned int T2= 2;
      Serial.print("T1:");
      Serial.println(T1);
      Serial.print("T2:");
      Serial.println(T2);
      frame_cgk.len=6;
      frame_cgk.ext=true;
      frame_cgk.id=frame_E_T1.id;
      frame_E_T1.len=6;
      ascon_dec_frame(&frame_cgk,&frame_E_T1,gk,16,gk_IV,16);
      Serial.print("cgk :");
      for(int i=0;i<frame_cgk.len;i++)
      {
        Serial.print(frame_cgk.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println();
    
      uint8_t cgk[6];
      for(int i=0;i<frame_cgk.len;i++)
      {
        cgk[i]=frame_cgk.data[i];
      }
      uint8_t cgk_IV[6]={0x11,0x66,0x22,0x55,0x33,0x44};
      Serial.print("cgk :");
      for(int i=0;i<6;i++)
      {
        Serial.print(cgk[i],HEX);
        Serial.print(" ");
      }
      Serial.println();
// test step 6 failed
//    if(first)
//    {
//      same_mac= false;
//      first=false;
//    }
      if(T2>T1 && same_mac)
      {
        Serial.println("check T1 and MAC1 success");
      
        frame_T2_MAC2.len=8;
        frame_T2_MAC2.ext = true;
        frame_T2_MAC2.id = 0x4200000;
      
        uint8_t IDG_IDECU_T2[4];
        uint8_t tag_IDG_IDECU_T2[6];
        IDG_IDECU_T2[0]=G_ID;
        IDG_IDECU_T2[1]=ECU_ID;
        IDG_IDECU_T2[2]=T2 >> 8;
        IDG_IDECU_T2[3]=T2 % 256;
        ascon_mac_plaintext(IDG_IDECU_T2,4,tag_IDG_IDECU_T2,6,cgk,6,cgk_IV,6);
        frame_T2_MAC2.data[0]=T2 >> 8;
        frame_T2_MAC2.data[1]=T2 % 256;
        frame_T2_MAC2.data[2]=tag_IDG_IDECU_T2[0];
        frame_T2_MAC2.data[3]=tag_IDG_IDECU_T2[1];
        frame_T2_MAC2.data[4]=tag_IDG_IDECU_T2[2];
        frame_T2_MAC2.data[5]=tag_IDG_IDECU_T2[3];
        frame_T2_MAC2.data[6]=tag_IDG_IDECU_T2[4];
        frame_T2_MAC2.data[7]=tag_IDG_IDECU_T2[5];
        Serial.print("frame_T2_MAC2 :");
        for(int i=0;i<frame_T2_MAC2.len;i++)
        {
          Serial.print(frame_T2_MAC2.data[i],HEX);
          Serial.print(" ");
        }
        Serial.println();
        const bool ok_frame_T2_MAC2=can.tryToSend(frame_T2_MAC2);

        if(ok_frame_T2_MAC2)
        {
          Serial.print ("Send frame_T2_MAC2 ID :") ;
          Serial.print(frame_T2_MAC2.id,HEX);
          Serial.println();
      
          Serial.println ("-----------------Send T2||MAC2 success---------------") ;  

          Serial.print("Send frame_T2_MAC2:");
          for(int i=0;i<frame_T2_MAC2.len;i++)
          {
            Serial.print(frame_T2_MAC2.data[i],HEX);
            Serial.print(" ");
          }
          Serial.println();


          while(!can.available());
          if(can.available())
          {
            int check_sum=0;      //    错误 没加0
            CANMessage frame_success_or_failed;
            can.receive(frame_success_or_failed);
            Serial.println("Receve frame_success_or_failed");
            Serial.print("Receice : ");
            for(int i=0;i<frame_success_or_failed.len;i++)
            {
              check_sum += frame_success_or_failed.data[i];
              Serial.print(frame_success_or_failed.data[i]);
              Serial.print(" ");
            }
            Serial.println();
            if(check_sum == 0 )
            {
              Serial.println("authentication passed");
              break;
            }
            else
            {
              uint8_t IDG_IDECU_Fail_T3[8];
              uint8_t tag_IDG_IDECU_Fail_T3[6];
              IDG_IDECU_Fail_T3[0]=G_ID;
              IDG_IDECU_Fail_T3[1]=ECU_ID;
              IDG_IDECU_Fail_T3[2]=0xff;
              IDG_IDECU_Fail_T3[3]=0xff;
              IDG_IDECU_Fail_T3[4]=0xff;
              IDG_IDECU_Fail_T3[5]=0xff;
              IDG_IDECU_Fail_T3[6]=frame_success_or_failed.data[0];
              IDG_IDECU_Fail_T3[7]=frame_success_or_failed.data[1];
              frame_success_or_failed.data[0]=frame_success_or_failed.data[2];
              frame_success_or_failed.data[1]=frame_success_or_failed.data[3];
              frame_success_or_failed.data[2]=frame_success_or_failed.data[4];
              frame_success_or_failed.data[3]=frame_success_or_failed.data[5];
              frame_success_or_failed.data[4]=frame_success_or_failed.data[6];
              frame_success_or_failed.data[5]=frame_success_or_failed.data[7];
              frame_success_or_failed.len=6;
              ascon_checkTag_plaintext(IDG_IDECU_Fail_T3,8,frame_success_or_failed.data,frame_success_or_failed.len,gk,16,gk_IV,16);
              if(same_mac)
              {
                Serial.println("authentication failed");
                Serial.println("check failed frame success   will back to step 3");
                continue;
              }
              else
              {
                Serial.println("authentication failed");
                delay(1000000);
              }
            }
          }
        }
        else
        {
          Serial.println ("-----------------Send T2||MAC2 failure---------------") ;
        }   
      }
      else
      {
        Serial.println("check T1 or MAC1 failed     will back to step 3");
        CANMessage frame_T2_MAC2_failed;
        frame_T2_MAC2_failed.len=8;
        frame_T2_MAC2_failed.ext = true;
        frame_T2_MAC2_failed.id = 0x4200000;   
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
        ascon_mac_plaintext(IDG_IDECU_Fail_T2,8,tag_IDG_IDECU_Fail_T2,6,gk,16,gk_IV,16);
        frame_T2_MAC2_failed.data[0]=T2 >> 8;
        frame_T2_MAC2_failed.data[1]=T2 % 256;
        frame_T2_MAC2_failed.data[2]=tag_IDG_IDECU_Fail_T2[0];
        frame_T2_MAC2_failed.data[3]=tag_IDG_IDECU_Fail_T2[1];
        frame_T2_MAC2_failed.data[4]=tag_IDG_IDECU_Fail_T2[2];
        frame_T2_MAC2_failed.data[5]=tag_IDG_IDECU_Fail_T2[3];
        frame_T2_MAC2_failed.data[6]=tag_IDG_IDECU_Fail_T2[4];
        frame_T2_MAC2_failed.data[7]=tag_IDG_IDECU_Fail_T2[5];
        Serial.print("frame_T2_MAC2_failed :");
        for(int i=0;i<frame_T2_MAC2_failed.len;i++)
        {
          Serial.print(frame_T2_MAC2_failed.data[i],HEX);
          Serial.print(" ");
        }
        Serial.println();
        const bool ok_frame_T2_MAC2_failed=can.tryToSend(frame_T2_MAC2_failed);
        if(ok_frame_T2_MAC2_failed)
        {
          Serial.print ("Send frame_T2_MAC2_failed ID :") ;
          Serial.print(frame_T2_MAC2_failed.id,HEX);
          Serial.println();
          Serial.println ("-----------------send frame_T2_MAC2_failed success---------------") ;  
          Serial.print("Send frame_T2_MAC2_failed:");
          for(int i=0;i<frame_T2_MAC2_failed.len;i++)
          {
            Serial.print(frame_T2_MAC2_failed.data[i],HEX);
            Serial.print(" ");
          }
          Serial.println();
          Serial.println("back to step 3 ");
          continue;
        }
        else
        {
          Serial.println ("-------------send frame_T2_MAC2_failed failed---------------") ;
        }
      }
    }
    else
    {
      Serial.println ("Waiting for authentication ") ;
    }
  }

//-------------------------------------authentication-----------------------------------------------
}

int count=0;

void loop(){
  Serial.print("loop :");
  Serial.println(++count);
   delay(2000);
// delay(50000);
}
