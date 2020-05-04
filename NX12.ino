
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

#include <M5StickC.h>
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>

char auth[] = "jZpGVzEJgw4g3SkzJkdpeAgdTGvRWAoW";  //メールで送られるAuth Token

const uint8_t Srv_RF = 26, Srv_RR = 0, Srv_LF = 32, Srv_LR = 33; //GPIO No.
const uint8_t srvCH_RF = 0, srvCH_RR = 1, srvCH_LF = 2, srvCH_LR = 3; //チャンネル
const double PWM_Hz = 50;   //PWM周波数
const uint8_t PWM_level = 16; //PWM 16bit(0～65535)

int mRF = 0, mRR = 0, mLF = 0, mLR = 0;
int sRF = 0, sRR = 0, sLF = 0, sLR = 0;

int pulseMIN = 2295;  //700μsec 50Hz 16bit
int pulseMAX = 7533;  //2300μsec 50Hz 16bit


void setup() {
  Serial.begin(151200);
  m5.begin();
  pinMode(Srv_RF, OUTPUT);
  pinMode(Srv_RR, OUTPUT);
  pinMode(Srv_LF, OUTPUT);
  pinMode(Srv_LR, OUTPUT);

  Blynk.setDeviceName("Blynk");
  Blynk.begin(auth);

  M5.Lcd.print("NX12 TEST");
  
  //モータのPWMのチャンネル、周波数の設定
  ledcSetup(srvCH_RF, PWM_Hz, PWM_level);
  ledcSetup(srvCH_RR, PWM_Hz, PWM_level);
  ledcSetup(srvCH_LF, PWM_Hz, PWM_level);
  ledcSetup(srvCH_LR, PWM_Hz, PWM_level);

  //モータのピンとチャンネルの設定
  ledcAttachPin(Srv_RF, srvCH_RF);
  ledcAttachPin(Srv_RR, srvCH_RR);
  ledcAttachPin(Srv_LF, srvCH_LF);
  ledcAttachPin(Srv_LR, srvCH_LR);
}

//ジョイスティックのデータ受信
BLYNK_WRITE(V0) {
  int x = param[0].asInt();
  int y = param[1].asInt();

//方向制御
  if(abs(y) > 3){   //yが5以上で左右回転
    mRF = y;
    mRR = y;
    mLF = -y;
    mLR = -y;
  }else if(abs(x) > 3){   //xが5以上で前後進
    mRF = x;
    mRR = x;
    mLF = x;
    mLR = x;
  }else{
    mRF = 0;
    mRR = 0;
    mLF = 0;
    mLR = 0;
  }
}

BLYNK_WRITE(V1) {
  int x = param[0].asInt();
  int y = param[1].asInt();

//方向制御
  if(y > 3){
    if(x > 3){
      sRF = y;
      sRR = 0;
      sLF = 0;
      sLR = y;
    }else if(x < -3){
      sRF = 0;
      sRR = -y;
      sLF = -y;
      sLR = 0;
    }else if(abs(x) <= 3){
      sRF = y;
      sRR = -y;
      sLF = -y;
      sLR = y;
    }
  }else if(y < -3){
    if(x > 3){
      sRF = 0;
      sRR = -y;
      sLF = -y;
      sLR = 0;
    }else if(x < -3){
      sRF = y;
      sRR = 0;
      sLF = 0;
      sLR = y;
    }else if(abs(x) <= 3){
      sRF = y;
      sRR = -y;
      sLF = -y;
      sLR = y;
    }
  }else if(abs(x) > 3){
    sRF = x;
    sRR = x;
    sLF = x;
    sLR = x;
  }else{
    sRF = 0;
    sRR = 0;
    sLF = 0;
    sLR = 0;
  }
}


void motor_drive(int motor_RF, int motor_RR, int motor_LF, int motor_LR){
    motor_RF = map(motor_RF, -10, 10, pulseMIN, pulseMAX);
    motor_RR = map(motor_RR, -10, 10, pulseMIN, pulseMAX);
    motor_LF = map(motor_LF, -10, 10, pulseMIN, pulseMAX);
    motor_LR = map(motor_LR, -10, 10, pulseMIN, pulseMAX);
  
    ledcWrite(srvCH_RF, motor_RF);
    ledcWrite(srvCH_RR, motor_RR);
    ledcWrite(srvCH_LF, motor_LF);
    ledcWrite(srvCH_LR, motor_LR);
}

void loop() {
  M5.update();
  Blynk.run();
  
  motor_drive(-(mRF+sRF), -(mRR+sRR), mLF+sLF, mLR+sLR);
  delay(10);
}
