//ECU��α����
//ECU1 ID:00100001   
//ECU2 ID:01000001  
//ECU3 ID:00100010  
//ECU4 ID:01000010  
//��ϢID��00000000001 
//
//ECU1:
//����
void send (){
	switch (gSentFrameCount % 2) {
    case 0://RTR ���� 
	frame.id = ECU2id||00||��ϢID||ECU1id;//X=00   0x8200121
	RTR=1; 
	break;
    case 1://RTR ���� 
	frame.id = ECU3id||10||��ϢID||ECU1id;//0X4500121
	RTR=1;
	break;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id||10||��ϢID||ECU2id), receive0},//����RTR 0x4300141
    {extended2515Filter (ECU1id||10||��ϢID||ECU3id), receive0},//����RTR 0x4300122
    {extended2515Filter (ECU2id||00||��ϢID||ECU1id), receive0},//����DATA 0x8200121
    {extended2515Filter (ECU3id||00||��ϢID||ECU1id), receive0},//����DATA 0x4400121
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//����RTR����װDATA 
	��װDATA����	
	RTR=0;
	frame.id==��8λ��ǰ8λ���� X=00
	/*
	if(id==ECU1id||10||��ϢID||ECU2id)  //0X4300141
	{
	id=ECU2id||00||��ϢID||ECU1id     //0x8200121
	��װDATA����
	RTR=0;
	} 
	if(id==ECU1id||10||��ϢID||ECU3id)  //0X4300122
	{
	id=ECU3id||00||��ϢID||ECU1id    //0X4400121
	��װDATA����
	RTR=0;
	} 
	*/
	send.frame 
	}
	else {
	can.receive();	
	}


//ECU2:
//����
void send (){
	switch (gSentFrameCount % 2) {
    case 0://RTR ���� 
	frame.id = ECU1id||00||��ϢID||ECU2id;//X=00   0x4200141
	RTR=1; 
	break;
    case 1://RTR ���� 
	frame.id = ECU4id||10||��ϢID||ECU2id;//0X8500141
	RTR=1;
	break;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU2id||10||��ϢID||ECU1id), receive0},//����RTR 0x8300121
    {extended2515Filter (ECU2id||10||��ϢID||ECU4id), receive0},//����RTR 0x8300142
    {extended2515Filter (ECU1id||00||��ϢID||ECU2id), receive0},//����DATA 0x4200141
    {extended2515Filter (ECU4id||00||��ϢID||ECU2id), receive0},//����DATA 0x8400141
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//����RTR����װDATA 
	��װDATA����
	RTR=0;	
	frame.id==��8λ��ǰ8λ���� X=00
	/*
	if(id==ECU2id||10||��ϢID||ECU1id)   //0X8300121
	{
	id=ECU1id||00||��ϢID||ECU2id    //0x4200141
	��װDATA����
	RTR=0;
	} 
	if(id==ECU2id||10||��ϢID||ECU4id)   //0X8300142
	{
	id=ECU4id||00||��ϢID||ECU2id   //0X8400141
	��װDATA����
	RTR=0;
	} 
	*/
	send.frame
	}
	else {
	can.receive();	
	}
	
	
//ECU3:
//����
void send (){
    //RTR ���� 
	frame.id = ECU1id||10||��ϢID||ECU3id;//0X4300122
	RTR=1;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU3id||10||��ϢID||ECU1id), receive0},//0x4500121
    {extended2515Filter (ECU1id||00||��ϢID||ECU3id), receive0},//0x4200122
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//����RTR����װDATA 
	��װDATA����	
	frame.id== ECU3id||00||��ϢID||ECU1id  //0X4400121
	can.send ;
	}
	else {
	can.receive();	
	}

	
	
//ECU4:
//����
void send (){
    //RTR ���� 
	frame.id = ECU2id||10||��ϢID||ECU4id;//0X8300142
	RTR=1;
	can.send ;
}
	
 const ACAN2515Mask rxm0 = extended2515Mask (11111111 11 00000000000 11111111);//0x1ff800ff
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU4id||10||��ϢID||ECU2id), receive0},//0x8500141
    {extended2515Filter (ECU2id||00||��ϢID||ECU4id), receive0},//0x8200142
  } ;
void recieve(){
	can.receive();
	if(RTR==1){//����RTR����װDATA 
	��װDATA����	
	frame.id== ECU4id||00||��ϢID||ECU2id  // 0X8400141
	can.send ;
	}
	else {
	can.receive();	
	} 
	
	
	
//gateway1 : 
const ACAN2515Mask rxm1 = extended2515Mask (11100000 11 00000000000 11100000);//0x1c1800e0
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU2id||00||��ϢID||ECU1id), receive0},// RTR/DATA,�����ع㲥 0x8200121 
    {extended2515Filter (ECU1id||01||��ϢID||ECU2id), receive0},// ����ת��DATA,RTR  0X4280141
 
    
void send_gateway(){
	can.receive();
	//������ת�� 
	if��RTR==1��{
			if((frame.id&11100000 11 00000000000 11100000)==01000000||00||00000000000||00100000) //������ת�� RTR   0X1c1800e0   0X8000020
			{
				frame.id=ECU2id||01||��ϢID||ECU1id //0X8280121 
					can.send ;
			}
			if((frame.id&11100000 11 00000000000 11100000)==00100000||01||00000000000||01000000) //������ת�� RTR   0X1c1800e0   0x4080040
			{
			 	frame.id=ECU1id||10||��ϢID||ECU2id	//0X4300141
			 		can.send ;
			}
	} 
	
	if��RTR==0��{
			if((frame.id&11100000 00 00000000000 11100000)==01000000||00||00000000000||00100000){//�����ع㲥DATA    0X1c0000e0   0X8000020
			frame.id=ECU2id||01||��ϢID||ECU1id//0X8280121
			can.send ;	
			}
		    if((frame.id&11100000 11 00000000000 11100000)==00100000||01||00000000000||01000000){// ����ת��DATA     0X1c1800e0   0x4080040
		    frame.id=ECU2id||00||��ϢID||ECU1id //0x8200121
		    can.send ;
			}
	}
	
	
	
	
	
	
//gateway2 : 
const ACAN2515Mask rxm1 = extended2515Mask (11100000 11 00000000000 11100000);//0x1c1800e0
  const ACAN2515AcceptanceFilter filters [] = {
    {extended2515Filter (ECU1id||00||��ϢID||ECU2id), receive0},// RTR/DATA,�����ع㲥  0x4200141 
    {extended2515Filter (ECU2id||01||��ϢID||ECU1id), receive0},// ����ת��DATA,RTR  0X8280121

    
void send_gateway(){
	can.receive();} 
	//������ת�� 
	if��RTR==1��{
			if((frame.id&11100000 11 00000000000 11100000)==00100000||00||00000000000||01000000) //������ת�� RTR    0X1c1800e0   0x4000040
			{
				frame.id=ECU1id||01||��ϢID||ECU2id //0X4280141
					can.send ;
			}
			if((frame.id&11100000 11 00000000000 11100000)==01000000||01||00000000000||00100000) //������ת�� RTR    0X1c1800e0    0x8080020
			{
			 	frame.id=ECU2id||10||��ϢID||ECU1id	//0X8300121
			 		can.send ;
			}
	} 
	
	if��RTR==0��{
			if((frame.id&11100000 00 00000000000 11100000)==00100000||00||00000000000||01000000){//�����ع㲥DATA    0X1c0000e0   0x4000040
			frame.id=ECU1id||01||��ϢID||ECU2id  //0X4280141
			can.send ;	
			}
		    if((frame.id&11100000 11 00000000000 11100000)==01000000||01||00000000000||00100000){// ����ת��DATA     0X1c1800e0   0x8080020
		    frame.id=ECU1id||00||��ϢID||ECU2id // 0x4200141
		    can.send ;
			}
	}
	
	
	
	
	
	
	
	
	
	
	
	
