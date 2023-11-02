//ECU的伪代码
//ECU1 ID:00100001   
//ECU2 ID:01000001  
//ECU3 ID:00100010  
//ECU4 ID:01000010  
//消息ID12：11000000000 
//消息ID13：10100000000 
//消息ID24：01010000000 
//消息ID34：00110000000
//ECU1:
//发：
void send (){
	switch (gSentFrameCount % 2) {
    case 0://RTR 跨域 
	frame.id = 01011111||00||11000000000||ECU1id;//X=00   0xbe60021 
	RTR=1; 
	break;
    case 1://RTR 域内 
	frame.id = 00111111||10||10100000000||ECU1id;//0X7f50021
	RTR=1;
	break;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (00000000 11 11111111111 11111111);//0x01fffff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id(全0)||10||11000000000||ECU2id), receive0},//跨域RTR 0x0160041
    {extended2515Filter (ECU1id(全0)||10||10100000000||ECU3id), receive0},//域内RTR 0x0150022
    {extended2515Filter (ECU2id(全0)||00||11000000000||ECU1id), receive0},//跨域DATA 0x0060021
    {extended2515Filter (ECU3id(全0)||00||10100000000||ECU1id), receive0},//域内DATA 0x0050021
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//接受RTR，组装DATA 
	组装DATA发送	
	RTR=0;
	frame.id==后8位和前8位互换,后八位改为自己ID X=00
	/*
	if(id==ECU1域id+11111||10||11000000000||ECU2id)  //0X7f60041
	{
	id=ECU2id||00||11000000000||ECU1id     //0x8260021
	组装DATA发送
	RTR=0;
	} 
	if(id==ECU1域id+11111||10||10100000000||ECU3id)  //0X7f50022
	{
	id=ECU3id||00||10100000000||ECU1id    //0X4450021
	组装DATA发送
	RTR=0;
	} 
	*/
	send.frame 
	}
	else {
	can.receive();	
	}

	 
	
//ECU2:
//发：
void send (){
	switch (gSentFrameCount % 2) {
    case 0://RTR 跨域 
	frame.id = 00111111||00||11000000000||ECU2id;//X=00   0x7e60041
	RTR=1; 
	break;
    case 1://RTR 域内 
	frame.id = 01011111||10||01010000000||ECU2id;//0Xbf28041
	RTR=1;
	break;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (00000000 11 1111111111 11111111);//0x01fffff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU2id(全0)||10||11000000000||ECU1id), receive0},//跨域RTR 0x0160021
    {extended2515Filter (ECU2id(全0)||10||01010000000||ECU4id), receive0},//域内RTR 0x0128042
    {extended2515Filter (ECU1id(全0)||00||11000000000||ECU2id), receive0},//跨域DATA 0x0060041
    {extended2515Filter (ECU4id(全0)||00||01010000000||ECU2id), receive0},//域内DATA 0x0028041
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//接受RTR，组装DATA 
	组装DATA发送
	RTR=0;	
	frame.id==后8位和前8位互换,后八位改为自己ID X=00
	/*
	if(id==ECU2域id+11111||10||11000000000||ECU1id)   //0Xbf60021
	{
	id=ECU1id||00||11000000000||ECU2id    //0x4260041
	组装DATA发送
	RTR=0;
	} 
	if(id==ECU2域id+11111||10||01010000000||ECU4id)   //0Xbf28042
	{
	id=ECU4id||00||01010000000||ECU2id   //0X8428041
	组装DATA发送
	RTR=0;
	} 
	*/
	send.frame
	}
	else {
	can.receive();	
	}
	
//ECU3:
//发：
void send (){
    //RTR 域内 
	frame.id = 00111111||10||10100000000||ECU3id;//0X7f50022
	RTR=1;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (0000000 11 11111111111 11111111);//0x01fffff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU3id(全零)||10||10100000000||ECU1id), receive0},//0x0150021
    {extended2515Filter (ECU1id(全零)||00||10100000000||ECU3id), receive0},//0x0050022
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//接受RTR，组装DATA 
	组装DATA发送	
	frame.id== ECU3id||00||10100000000||ECU1id  //0X4450021
	can.send ;
	}
	else {
	can.receive();	
	}

	
	
//ECU4:
//发：
void send (){
    //RTR 域内 
	frame.id = 01011111||10||01010000000||ECU4id;//0Xbf28042
	RTR=1;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (00000000 11 11111111111 11111111);//0x01fffff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU4id(全零)||10||01010000000||ECU2id), receive0},//0x0128041
    {extended2515Filter (ECU2id(全零)||00||01010000000||ECU4id), receive0},//0x0028042
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//接受RTR，组装DATA 
	组装DATA发送	
	frame.id== ECU4id||00||01010000000||ECU2id  // 0X8428041
	can.send ;
	}
	else {
	can.receive();	
	} 
	
	
	
//gateway1 : 
const ACAN2515Mask rxm1 = extended2515Mask (11100000 11 00000000000 11100000);//0x1c1800e0
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU2id||00||消息ID(全零代替)||ECU1id), receive0},// RTR/DATA,向网关广播 0x8200021
    {extended2515Filter (ECU1id||01||消息ID(全零代替)||ECU2id), receive0},// 向域内转播DATA,RTR  0X4280041
 
    
void send_gateway(){
	can.receive();
	//向域内转发 
	if(RTR==1{
			if((frame.id&11100000 11 00000000000 11100000)==01000000||00||00000000000||00100000) //向网关转发 RTR   0X1c1800e0   0X8000020
			{
				frame.id=01011111||01||消息ID(11000000000)||ECU1id //0X bee0021
					can.send ;
			}
			if((frame.id&11100000 11 00000000000 11100000)==00100000||01||00000000000||01000000) //向域内转发 RTR   0X1c1800e0   0x4080040
			{
			 	frame.id=00111111||10||消息ID(11000000000)||ECU2id	//0X7f60041
			 		can.send ;
			}
	} 
	
	if(RTR==0){
			if((frame.id&11100000 00 00000000000 11100000)==01000000||00||00000000000||00100000){//向网关广播DATA    0X1c0000e0   0X8000020
			frame.id=ECU2id||01||消息ID(11000000000)||ECU1id//0X82e0021
			can.send ;	
			}
		    if((frame.id&11100000 11 00000000000 11100000)==00100000||01||00000000000||01000000){// 域内转播DATA     0X1c1800e0   0x4080040
		    frame.id=ECU2id||00||消息ID(11000000000)||ECU1id //0x8260021
		    can.send ;
			}
	}
	
	
	
	
	
	
//gateway2 : 
const ACAN2515Mask rxm1 = extended2515Mask (11100000 11 00000000000 11100000);//0x1c1800e0
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id||00||消息ID(全零代替)||ECU2id), receive0},// RTR/DATA,向网关广播  0x4200041
    {extended2515Filter (ECU2id||01||消息ID(全零代替)||ECU1id), receive0},// 域内转播DATA,RTR  0X8280021

    
void send_gateway(){
	can.receive();} 
	//向域内转发 
	if(RTR==1){
			if((frame.id&11100000 11 00000000000 11100000)==00100000||00||00000000000||01000000) //向网关转发 RTR    0X1c1800e0   0x4000040
			{
				frame.id=00111111||01||消息ID(11000000000)||ECU2id //0X7ee0041
					can.send ;
			}
			if((frame.id&11100000 11 00000000000 11100000)==01000000||01||00000000000||00100000) //向域内转发 RTR    0X1c1800e0    0x8080020
			{
			 	frame.id=01011111||10||消息ID(11000000000)||ECU1id	//0Xbf60021
			 		can.send ;
			}
	} 
	
	if(RTR==0){
			if((frame.id&11100000 00 00000000000 11100000)==00100000||00||00000000000||01000000){//向网关广播DATA    0X1c0000e0   0x4000040
			frame.id=ECU1id||01||消息ID(11000000000)||ECU2id  //0X42e0041
			can.send ;	
			}
		    if((frame.id&11100000 11 00000000000 11100000)==01000000||01||00000000000||00100000){// 域内转播DATA     0X1c1800e0   0x8080020
		    frame.id=ECU1id||00||消息ID(11000000000)||ECU2id // 0x4260041
		    can.send ;
			}
	}
	
	
	
	
	
	
	
	
	
	
	

