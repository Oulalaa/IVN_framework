//ECU的伪代码
//ECU1 ID:00100001   
//ECU2 ID:01000001  
//ECU3 ID:00100010  
//ECU4 ID:01000010  
//消息ID：00000000001 

//ECU1:
//发：
void send (){
	switch (gSentFrameCount % 3) {
    case 0:
	frame.id = ECU2id||00||消息ID||ECU1id;//X=00         0x8200121
	break;
    case 1:
	frame.id = 01100001||00||消息ID||ECU1id;//不是给ECU2的  0xc200121
	break;
	case 2:
	frame.id = ECU1id||00||消息ID||ECU3id;//域内ECU1给ECU3   0x4200122
	break;
	can.send ;
	
}
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU2id||00||消息ID||ECU1id), receive0},//0x8200121
    {extended2515Filter (ECU3id||00||消息ID||ECU1id), receive0},//0x4400121
  } ;
void recieve(){
	can.receive();} 
	
//ECU2:
//发：
void send (){
	switch (gSentFrameCount % 3) {
    case 0:
	frame.id = ECU1id||00||消息ID||ECU2id;//X=00   0x4200141
	break;
    case 1:
	frame.id = 01100001||00||消息ID||ECU2id;//不是给ECU1的 0xc200141
	break;
	case 2:
	frame.id = ECU2id||00||消息ID||ECU4id;//域内ECU2给ECU4  0x8200142
	break;
	can.send ;
}
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id||00||消息ID||ECU2id), receive0},  //0x4200141
    {extended2515Filter (ECU4id||00||消息ID||ECU2id), receive0},  //0x8400141
  } ;
void recieve(){
	can.receive();} 
	
//ECU3:
//发：
void send (){
	frame.id = ECU3id||00||消息ID||ECU1id;//域内ECU3给ECU1 0x4400121
	can.send ;
	
}
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id||00||消息ID||ECU3id), receive0},//0x4200122
  } ;
void recieve(){
	can.receive();} 	
	
	
//ECU4:
//发：
void send (){
	frame.id = ECU4id||00||消息ID||ECU2id;//域内ECU4给ECU2 0x8400141
	can.send ;
	
}
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU2id||00||消息ID||ECU4id), receive0},//0x8200142
  } ;
void recieve(){
	can.receive();} 
	
//gateway1 : 
const ACAN2515Mask rxm0 = extended2515Mask (11100000 11 00000000000 11100000);//0x1c1800e0
  const ACAN2515AcceptanceFilter filters [] = {
  	{extended2515Filter (ECU2id||00||消息ID||ECU1id), receive0},// 前3位不是自己的域，向网关广播 0x8200121 
    {extended2515Filter (ECU1id||01||消息ID||ECU2id), receive0},// 前3位是自己的域，改写 0x4280141
void send_gateway(){
	can.receive();} 
	//向域内转发 
	if（id前三位是自己的域）{
	frame.id=ECU2id||00||消息ID||ECU1id;//0x8200121
	can.send ;
	} 
	//向网关列表转发
	if（id前三位不是自己的域）{
	frame.id=ECU2id||01||消息ID||ECU1id;//0x8280121
	can.send ;
} 
	
//gateway2 : 
const ACAN2515Mask rxm0 = extended2515Mask (11100000 11 00000000000 11100000);//0x1c1800e0
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id||00||消息ID||ECU2id), receive0},// 前3位不是自己的域，向网关广播  0x4200141     
    {extended2515Filter (ECU2id||01||消息ID||ECU1id), receive0},// 前3位是自己的域，改写 0x8280121
void send_gateway(){
	can.receive();} 
	//向域内转发 
	if（id前三位是自己的域）{
	frame.id=ECU1id||00||消息ID||ECU2id;//0x4200141 
	can.send ;
	} 
	//向网关列表转发
	if（id前三位不是自己的域）{
	frame.id=ECU1id||01||消息ID||ECU2id;//0x4280141
	can.send ;
} 
		
	
	
	
	
	
	
	
	
	
	
	
	
