#define GYRO_ROLL_ADD 0
#define GYRO_PITCH_ADD 500
#define FLEX1_ADD 1000
#define FLEX2_ADD 5000

int MOUSE_DPS_SCALE = 1;

// USART 관련 레지스터 주소
volatile unsigned char* USART_PORT_DIR_Reg_Addr = 0x400;
volatile unsigned char* USART_PORTC_Reg_DIR = 0x440;
volatile unsigned char* USART_RX_DATAL_Addr = 0x820;
volatile unsigned char* USART_RX_DATAH_Addr = 0x821;
volatile unsigned char* USART_TX_DATAL_Addr = 0x822;
volatile unsigned char* USART_STATUS_Addr = 0x824;
volatile unsigned char* USART_ADC_CTRLA_Addr_Addr = 0x825;
volatile unsigned char* USART_ADC_CTRLB_Addr_Addr = 0x826;
volatile unsigned char* USART_ADC_CTRLC_Addr_Addr = 0x827;
volatile unsigned char* USART_BAUD_L = 0x828;
volatile unsigned char* USART_BAUD_H = 0x829;

// ADC base addr = 0x600
volatile unsigned char* ADC_CTRLA_Addr = 0x600;
volatile unsigned char* ADC_CTRLB_Addr = 0x601;
volatile unsigned char* ADC_CTRLC_Addr = 0x602;
volatile unsigned char* ADC_CTRLD_Addr = 0x603;
volatile unsigned char* ADC_CTRLE_Addr = 0x604;
volatile unsigned char* ADC_MUXPOS = 0x606;
volatile unsigned char* ADC_COMMAND = 0x608;
volatile unsigned char* ADC_EVCTRL = 0x609;
volatile unsigned char* ADC_RES_L = 0x610;
volatile unsigned char* ADC_RES_H = 0x611;
int16_t ADC_result; // ADC result
int16_t ADC_result2; // ADC result2

// I2C base address = 0x08a0
volatile unsigned char* GYRO = 0x68; // MPU-6050 i2c address

// MPU-6050 내부 레지스터 주소
volatile unsigned char* ACCEL_XOUT_H = 0x3b;
volatile unsigned char* ACCEL_XOUT_L = 0x3c;
volatile unsigned char* ACCEL_YOUT_H = 0x3d;
volatile unsigned char* ACCEL_YOUT_L = 0x3e;
volatile unsigned char* ACCEL_ZOUT_H = 0x3f;
volatile unsigned char* ACCEL_ZOUT_L = 0x40;
volatile unsigned char* PWR_MGMT_1 = 0x6b;

// TWI 관련 레지스터 주소
volatile unsigned char* I2C_MCTRLA_Addr = 0x08a3;
volatile unsigned char* I2C_MCTRLB_Addr = 0x08a4;
volatile unsigned char* I2C_MSTATUS = 0x08a5;

volatile unsigned char* I2C_MBAUD = 0x08a6;
volatile unsigned char* I2C_MADDR = 0x08a7;
volatile unsigned char* I2C_MDATA = 0x08a8;


// TWI result
uint8_t AcX_l, AcX_h, AcY_l, AcY_h, AcZ_l, AcZ_h, Tmp_l, Tmp_h, GyX_l, GyX_h, GyY_l, GyY_h, GyZ_l, GyZ_h;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; 

// 가속도 센서 각도
double angleAcX,angleAcY,angleAcZ;
double angleGyX,angleGyY,angleGyZ;
double angleFiX,angleFiY,angleFiZ=0;

// 각도 계산 상수
const double RADIAN_TO_DEGREES= 180/3.14159;
const double DEGREE_PER_SECOND= 32767/250;
//const double ALPHA= 1/(1+0.04); // 상보필터 가중치

const double ALPHA= 0.8; // 상보필터 가중치

unsigned long now=0;
unsigned long past=0;
double dt=0;

double baseAcX,baseAcY,baseAcZ;
double baseGyX,baseGyY,baseGyZ;

/////////////////////// FUNCTION PROTOTYPES /////////////////////////

/////////////////////////////////////////////////////////////////////
void mydelay(){
  volatile int i = 0;
  for(i = 0; i < 10000; i++){
    
  }
}


void getDT(){ // dt 계산
  now=millis();
  dt=(now-past)/1000.0;
  past=now;
}

void calibrateSensor(){
  double sumAcX=0,sumAcY=0,sumAcZ=0;
  double sumGyX=0,sumGyY=0,sumGyZ=0;
  
  gyro_read();
  
  for(int i=0; i<10; i++){
    gyro_read();
    sumAcX+=AcX;
    sumAcY+=AcY;
    sumAcZ+=AcZ;
    sumGyX+=GyX;
    sumGyY+=GyY;
    sumGyZ+=GyZ;
    delay(100);
  }
  
  baseAcX=sumAcX/10; 
  baseAcY=sumAcY/10;
  baseAcZ=sumAcZ/10;
  
  baseGyX=sumGyX/10;
  baseGyY=sumGyY/10;
  baseGyZ=sumGyZ/10;
  
}

void c_filter(){
  gyro_read();
  getDT(); // dT 계산

  angleGyY+=((GyY-baseGyY)/DEGREE_PER_SECOND)*dt; // Gyro ROLL
  angleGyX+=((GyX-baseGyX)/DEGREE_PER_SECOND)*dt; // Gyro PITCH
  
  angleAcY = atan(-AcX/sqrt(pow(AcY,2)+pow(AcZ,2))); // Accel ROLL
  angleAcY = angleAcY * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
  double angleTmpY = angleFiY + (angleGyY * dt);

  angleFiY = ALPHA * angleTmpY + (1.0-ALPHA) * angleAcY; // Filtered PITCH
  
  myTransmit(0x2C); // 구별을 위한 '_'전송
 
  myTransmit_str(DEC_TO_CHAR(angleFiY + GYRO_PITCH_ADD)); // PITCH 각도 + 500
  
  angleAcX = atan(AcY/sqrt(pow(AcX,2)+pow(AcZ,2))); // Accel PITCH
  angleAcX = angleAcX * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
  double angleTmpX = angleFiX + (angleGyX * dt);
  
  angleFiX = ALPHA * angleTmpX + (1.0-ALPHA) * angleAcX; // Filtered ROLL
  myTransmit(0x2C); // 구별을 위한 ','전송
   
  myTransmit_str(DEC_TO_CHAR(angleFiX)); // ROLL 각도  

 
}

void calGyro(){ // GYRO PITCH ROLL 계산(DEGREE)
  gyro_read();
  getDT();
    
  angleGyY+=((GyY-baseGyY)/DEGREE_PER_SECOND)*dt;
  angleGyX+=((GyX-baseGyX)/DEGREE_PER_SECOND)*dt;
    
  angleAcY = atan(-AcX/sqrt(pow(AcY,2)+pow(AcZ,2)));
  angleAcY = angleAcY * RADIAN_TO_DEGREES; // RADIAN -> DEGREE

  int int_angleAcY = angleAcY; // TYPE CASTING(double -> int)
  myTransmit(0x2C); // 구별을 위한 '_'전송
  myTransmit_str(DEC_TO_CHAR(int_angleAcY + GYRO_PITCH_ADD)); // PITCH 각도 + 500
    
  myTransmit(0x2C); // 구별을 위한 '_'전송
  angleAcX = atan(AcY/sqrt(pow(AcX,2)+pow(AcZ,2)));
  angleAcX = angleAcX * RADIAN_TO_DEGREES; // RADIAN -> DEGREE

  int int_angleAcX = angleAcX; // TYPE CASTING(double -> int)
  myTransmit_str(DEC_TO_CHAR(int_angleAcX));  // ROLL 각도 + 0
}


void usart_init(){
  //(*USART_BAUD_L) = 0xa;
  //(*USART_BAUD_H) = 0x1a;
  (*USART_BAUD_L) = 0x05;
  (*USART_BAUD_H) = 0xd;
  (*USART_ADC_CTRLB_Addr_Addr) = 0xC0; // Receiver & Transmitter Enable
  (*USART_ADC_CTRLA_Addr_Addr) = 0x02; // RS-485 Mode
}

void adc_init(){
  (*ADC_CTRLA_Addr) = 0x3; // 10-bit resolution, Free-Running Mode, ADC Enable
  //(*ADC_CTRLB_Addr) = 0x0; // 0 result accumulated
  //(*ADC_CTRLB_Addr) = 0x1; // 2 result accumulated
  (*ADC_CTRLB_Addr) = 0x4; // 16 result accumulated
  //(*ADC_MUXPOS) = 0x0f; // ADC input pin 15
  //(*ADC_EVCTRL) = 0x1; // Enable Start event
}

void i2c_init(char baud){
  (*I2C_MBAUD) = baud; // Set the MBAUD rate
  (*I2C_MCTRLB_Addr)|=0x8; // Clear TWI state(FLUSH)
  (*I2C_MSTATUS)|=0xc0; // RIF, WIF
}

void i2c_enable(){
  //(*I2C_MCTRLA_Addr)|=0xd3; // SmartMode Enable, Enable TWI Master
  (*I2C_MCTRLA_Addr)|=0x3; // SmartMode Enable, Enable TWI Master (NO RIEN, WIEN!!!)
  (*I2C_MSTATUS)|=0x1; // initially sets the bus state to IDLE
}

char i2c_start(char addr){
  
  if( ((*I2C_MSTATUS) & 0x03) != 0x03 ){ // When Bus State is NOT BUSY
    
    (*I2C_MCTRLB_Addr) &= ~(1<<2); // Send ACK(ACKACT)
    (*I2C_MADDR) = addr; // ADDRESS packet = Slave addr + R/W

    if(addr & 1){ // Read Operation
      while( !((*I2C_MSTATUS) & 0x80) );
    }
    else{ // Write Operation
      while( !((*I2C_MSTATUS) & 0x40) );
    }

    return (*I2C_MSTATUS);
  }
  else{
    return (*I2C_MSTATUS);
  }

}

char i2c_read(char* rdata, char ACKorNACK){
  
  if( ((*I2C_MSTATUS) & 0x3) == 0x02 ){ // When Bus State is OWNER
    
    while( !((*I2C_MSTATUS) & 0x80) );

    if(ACKorNACK){
      (*I2C_MCTRLB_Addr) &= ~(1<<2); // Send ACK(ACKACT)
    }
    else{
      (*I2C_MCTRLB_Addr) |= 1<<2; // Send NACK(ACKACT)
    }

    (*rdata) = (*I2C_MDATA);
    
    return (*I2C_MSTATUS);
  }
  else{
    return (*I2C_MSTATUS);
  }

}

char i2c_write(char wdata){
  
  if( ((*I2C_MSTATUS) & 0x03) == 0x02 ){ // When Bus State is OWNER

    while (!(((*I2C_MSTATUS) & 0x40) | ((*I2C_MSTATUS) & 0x10)));
    
    (*I2C_MDATA) = wdata;
    return (*I2C_MSTATUS);
  }
  else{
    return (*I2C_MSTATUS);
  }
}

void i2c_stop(){
  (*I2C_MCTRLB_Addr) |= 0x03; // STOP ADC_COMMAND
}

void myTransmit(unsigned char data){
   while(1){
     if(  ((*USART_STATUS_Addr) & 0x20)  ){
       break;
     }
   }
   (*USART_TX_DATAL_Addr) = data;
}

void myTransmit_str(unsigned char* data){
  while(*data){
    myTransmit(*data++);
  }
}

static unsigned char save_data[15] = {0};

unsigned char* DEC_TO_CHAR(long data){
  unsigned char i, j, temp;
  
  for(i = 1; i < 10; i++){
    save_data[i] = 0;
  } // init 0

  if(data >= 0){ // 양수의 경우
    save_data[0] = '+'; // 앞에 + 붙여줌
    
    for(i = 1; i < 10; i++){
      save_data[i] = (data % 10) + '0';
      data /= 10;
      if(data == 0){
        break;
      }
    }
  } // if end

  else{ // 음수의 경우
    data *= -1; // 일단 양수로 만들고
    save_data[0] = '-'; // 앞에 - 붙여줌
    for (i = 1; i < 10; i++){
      save_data[i] = (data % 10) + '0';
      data /= 10;
      if(data==0){
        break;
      }
    }
  } // else end
  
  i++;
  for(j=1; j<=(i/2); j++){
    temp = save_data[j];
    save_data[j] = save_data[i - j];
    save_data[i - j] = temp;
  }
  
  return save_data;
}

/*void flex_init(){
  (*ADC_MUXPOS) = 0x0f; // ADC input pin 15
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion

}
}*/
void flex_read(){
  
  (*ADC_MUXPOS) = 0x0e; // ADC input pin 15
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result = (*ADC_RES_L)|(*ADC_RES_H)<<8;
  
  myTransmit(0x2C);
  myTransmit_str(DEC_TO_CHAR(ADC_result+FLEX1_ADD));
}

void flex_read2(){
 (*ADC_MUXPOS) = 0x0d; // ADC input pin 15
 (*ADC_EVCTRL) = 0x1; // Enable Start event

 (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result2 = (*ADC_RES_L)|(*ADC_RES_H)<<8;

  myTransmit(0x2C);
  myTransmit_str(DEC_TO_CHAR(ADC_result2+FLEX2_ADD));
}

void gyro_read(){

  ////////////////////////////////////////////////////////////////////////////
  i2c_start(0xd0); // ADDRESS PACKET = slave address(0x68) + WRITE(0x0) = 0xD0
  i2c_write(0x3b);
  i2c_start(0xd1); // ADDRESS PACKET = slave address(0x68) + READ(0x1) = 0xD1
  ////////////////////////////////////////////////////////////////////////////
  
  i2c_read(&AcX_h,1);
  i2c_read(&AcX_l,1);

  i2c_read(&AcY_h,1);
  i2c_read(&AcY_l,1);

  i2c_read(&AcZ_h,1);
  i2c_read(&AcZ_l,1);
  
  i2c_read(&Tmp_h,1);
  i2c_read(&Tmp_l,1);
  
  i2c_read(&GyX_h,1);
  i2c_read(&GyX_l,1);
  
  i2c_read(&GyY_h,1);
  i2c_read(&GyY_l,1);
  
  i2c_read(&GyZ_h,1);
  i2c_read(&GyZ_l,0); // NACK
  
  i2c_stop(); // Stop

  AcX = (AcX_h<<8)|AcX_l;
  AcY = (AcY_h<<8)|AcY_l;
  AcZ = (AcZ_h<<8)|AcZ_l;

  Tmp = (Tmp_h<<8)|Tmp_l;

  GyX = (GyX_h<<8)|GyX_l;
  GyY = (GyY_h<<8)|GyY_l;
  GyZ = (GyZ_h<<8)|GyZ_l;

  //cal_AcZ(AcZ); // 음양 변환


//  myTransmit(0x0a);
//  myTransmit_str(DEC_TO_CHAR(AcX));
//  myTransmit(0x09);
//  myTransmit_str(DEC_TO_CHAR(AcY));
//  myTransmit(0x09);
//  myTransmit_str(DEC_TO_CHAR(cAcZ));
//  myTransmit(0x09);
//  myTransmit_str(DEC_TO_CHAR(Tmp));
//  myTransmit(0x09);
//  myTransmit_str(DEC_TO_CHAR(GyX));
//  myTransmit(0x09);
//  myTransmit_str(DEC_TO_CHAR(GyY));
//  myTransmit(0x09);
//  myTransmit_str(DEC_TO_CHAR(GyZ));
//  myTransmit(0x0d);
//  myTransmit(0x0a);

//  myTransmit(0x0a);
//  myTransmit_str(AcX);
//  myTransmit(0x09);
//  myTransmit_str(AcY);
//  myTransmit(0x09);
//  myTransmit_str(cAcZ);
//  myTransmit(0x09);
//  myTransmit_str(Tmp);
//  myTransmit(0x09);
//  myTransmit_str(GyX);
//  myTransmit(0x09);
//  myTransmit_str(GyY);
//  myTransmit(0x09);
//  myTransmit_str(GyZ);
//  myTransmit(0x0d);
//  myTransmit(0x0a);
  
}

void flex_check(){
  // 두개의 ADC값을 보기 좋게 출력해줌
  // 휨 센서의 값을 확인해 보기 위함.
  (*ADC_MUXPOS) = 0x0f; // ADC input pin 15
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result = (*ADC_RES_L)|(*ADC_RES_H)<<8;
  
  myTransmit(0x0a);
  myTransmit_str(DEC_TO_CHAR(ADC_result+1000)); // 측정값 + 4000
  myTransmit(0x09);
  
  /////////////////////////////////////////////////////////////////////
  
  (*ADC_MUXPOS) = 0x0e; // ADC input pin 14
  //(*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result2 = (*ADC_RES_L)|(*ADC_RES_H)<<8;

  myTransmit_str(DEC_TO_CHAR(ADC_result2+5000)); // 측정값 + 5000
  myTransmit(0x0d);
  myTransmit(0x0a);
}

void gyro_check(){
//  // 자이로 센서의 ROLL PIRCH 값을 보기좋게 출력해줌. calGyro()
//  gyro_read();
//  getDT();
//    
//  angleGyY+=((GyY-baseGyY)/DEGREE_PER_SECOND)*dt;
//  angleGyX+=((GyX-baseGyX)/DEGREE_PER_SECOND)*dt;
//    
//  angleAcY = atan(-AcX/sqrt(pow(AcY,2)+pow(AcZ,2)));
//  angleAcY = angleAcY * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
//
//  int int_angleAcY = angleAcY; // TYPE CASTING(double -> int)
//  myTransmit(0x0a);
//  myTransmit_str(DEC_TO_CHAR(int_angleAcY)); // PITCH 각도 + 500
//    
//  myTransmit(0x09);
//  angleAcX = atan(AcY/sqrt(pow(AcX,2)+pow(AcZ,2))); // GYRO PITCH
//  angleAcX = angleAcX * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
//
//  int int_angleAcX = angleAcX; // TYPE CASTING(double -> int)
//  myTransmit_str(DEC_TO_CHAR(int_angleAcX));  // ROLL 각도 + 0
//  myTransmit(0x0d);
//  myTransmit(0x0a);

  //////////////////////////////////
  gyro_read();
  getDT(); // dT 계산

  angleGyY+=((GyY-baseGyY)/DEGREE_PER_SECOND)*dt; // Gyro ROLL
  angleGyX+=((GyX-baseGyX)/DEGREE_PER_SECOND)*dt; // Gyro PITCH
  
  angleAcY = atan(-AcX/sqrt(pow(AcY,2)+pow(AcZ,2))); // Accel ROLL
  angleAcY = angleAcY * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
  double angleTmpY = angleFiY + (angleGyY * dt);

  angleFiY = ALPHA * angleTmpY + (1.0-ALPHA) * angleAcY; // Filtered PITCH
  myTransmit(0x0a);
  myTransmit_str(DEC_TO_CHAR(angleFiY + GYRO_PITCH_ADD)); // PITCH 각도 + 500
  
  angleAcX = atan(AcY/sqrt(pow(AcX,2)+pow(AcZ,2))); // Accel PITCH
  angleAcX = angleAcX * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
  double angleTmpX = angleFiX + (angleGyX * dt);
  
  angleFiX = ALPHA * angleTmpX + (1.0-ALPHA) * angleAcX; // Filtered ROLL
  myTransmit(0x09); // 구별을 위한 ','전송
   
  myTransmit_str(DEC_TO_CHAR(angleFiX)); // ROLL 각도  
  myTransmit(0x0d);
  myTransmit(0x0a);

  
}

void full_check(){
  // PITCH + ROLL + ADC1 + ADC2 값을 각각 보기 좋게 출력해줌.
  // 최종 출력값을 확인하기 위함.
  gyro_read();
  getDT();
    
  angleGyY+=((GyY-baseGyY)/DEGREE_PER_SECOND)*dt;
  angleGyX+=((GyX-baseGyX)/DEGREE_PER_SECOND)*dt;
    
  angleAcY = atan(-AcX/sqrt(pow(AcY,2)+pow(AcZ,2)));
  angleAcY = angleAcY * RADIAN_TO_DEGREES; // RADIAN -> DEGREE

  int int_angleAcY = angleAcY; // TYPE CASTING(double -> int)
  myTransmit(0x0a);
  myTransmit_str(DEC_TO_CHAR(int_angleAcY)); // ROLL 각도 + 500
    
  myTransmit(0x09);
  angleAcX = atan(AcY/sqrt(pow(AcX,2)+pow(AcZ,2))); // GYRO PITCH
  angleAcX = angleAcX * RADIAN_TO_DEGREES; // RADIAN -> DEGREE

  int int_angleAcX = angleAcX; // TYPE CASTING(double -> int)
  myTransmit_str(DEC_TO_CHAR(int_angleAcX));  // PITCH 각도 + 0
  myTransmit(0x09);
 
  //////////////////////////////////////////////////////////////////////
  
  (*ADC_MUXPOS) = 0x0f; // ADC input pin 15
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result = (*ADC_RES_L)|(*ADC_RES_H)<<8;
  
  myTransmit_str(DEC_TO_CHAR(ADC_result+1000));
  myTransmit(0x09);
  
  /////////////////////////////////////////////////////////////////////
  
  (*ADC_MUXPOS) = 0x0e; // ADC input pin 14
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result2 = (*ADC_RES_L)|(*ADC_RES_H)<<8;

  myTransmit_str(DEC_TO_CHAR(ADC_result2+5000)); // 측정값 + 5000
  myTransmit(0x0d);
  myTransmit(0x0a);
  
}

void final_check(){
  gyro_read();
  getDT(); // dT 계산

  angleGyY+=((GyY-baseGyY)/DEGREE_PER_SECOND)*dt; // Gyro ROLL
  angleGyX+=((GyX-baseGyX)/DEGREE_PER_SECOND)*dt; // Gyro PITCH
  
  angleAcY = atan(-AcX/sqrt(pow(AcY,2)+pow(AcZ,2))); // Accel ROLL
  angleAcY = angleAcY * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
  double angleTmpY = angleFiY + (angleGyY * dt);

  angleFiY = ALPHA * angleTmpY + (1.0-ALPHA) * angleAcY; // Filtered PITCH
  
  myTransmit(0x0a);
  myTransmit_str(DEC_TO_CHAR(angleFiY + GYRO_PITCH_ADD)); // PITCH 각도 + 500
  
  
  angleAcX = atan(AcY/sqrt(pow(AcX,2)+pow(AcZ,2))); // Accel PITCH
  angleAcX = angleAcX * RADIAN_TO_DEGREES; // RADIAN -> DEGREE
  double angleTmpX = angleFiX + (angleGyX * dt);
  
  angleFiX = ALPHA * angleTmpX + (1.0-ALPHA) * angleAcX; // Filtered ROLL
  
  myTransmit(0x09); 
  myTransmit_str(DEC_TO_CHAR(angleFiX)); // ROLL 각도  


  (*ADC_MUXPOS) = 0x0e; // ADC input pin 15
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result = (*ADC_RES_L)|(*ADC_RES_H)<<8;
  
  myTransmit(0x09);
  myTransmit_str(DEC_TO_CHAR(ADC_result+FLEX1_ADD));

  mydelay();

  (*ADC_MUXPOS) = 0x0d; // ADC input pin 15
  (*ADC_EVCTRL) = 0x1; // Enable Start event
  
  (*ADC_COMMAND) = 0x1; // Start conversion
  ADC_result2 = (*ADC_RES_L)|(*ADC_RES_H)<<8;
  
  myTransmit(0x09);
  myTransmit_str(DEC_TO_CHAR(ADC_result2+FLEX2_ADD));
  myTransmit(0x0d);
  myTransmit(0x0a);

  
}

void setup() {
  // USART setting
  usart_init();
  adc_init();

  //i2c setting
  i2c_init(0x0f); // Fast mode BAUD
  i2c_enable();  

  i2c_start(0xd0);
  i2c_write(0x6b);
  i2c_write(0);
  i2c_stop();
  
}



 
void loop() {
  //gyro_read();
  //calGyro();
  c_filter();
  flex_read();
  mydelay();
  flex_read2();
  //flex_check();
  //gyro_check();
  //full_check();

  //final_check();

}






  
