#include <MsTimer2.h> //定时器库的 头文件
#include <SoftwareSerial.h>
#include <dht11.h>          //引用dht11库文件，使得下面可以调用相关参数
#include <Servo.h> 

#define uchar unsigned char
#define DHT11PIN A3        //定义温湿度针脚号为A3号引脚
#define MQ_2PIN A2         //定义MQ2的引脚为模拟引脚A2
#define Elec_2PIN A1       //定义组副电量的引脚为A1
#define Elec_1PIN A0       //定义组主电量的引脚为A0
#define GsmPIN 4           //定义报警引脚为4号数字引脚
#define Buzz 13
#define SEND_Message   "0011000D91688170387506F70008AA186CB36C605B669662667A80FD63A752365B9E9A8C5BA4FF01" //河池学院智能控制实验室
#define SEND_Message_1 "0011000D91688170387506F70008AA144E007EA78B6662A5FF0C53D173B0706B60C5FF01"//一级警报，发现火情！
#define ATDCall "ATD18078357607;\r\n"

dht11 DHT11;                    //实例化一个对象
Servo myservo;  //创建一个舵机控制对象
SoftwareSerial mySerial(2, 3);   //定义一个软串口 2RX 3TX


int Hum     = 0; //湿度
int Temp    = 0; //温度
int Val     = 0;
int Val_E   = 0;
int Val_E_1 = 0;
int LED     = 7;
int gsm     = 0;
int State   = 0;
int gsm_count = 0;
int flag = 0;
String comdata   = " ";     //串口上位机数据接收缓存区
char AT_CMHF[]   = "AT+CMGF=0";
char AT_CMGS[]   = "AT+CMGS=39";
char AT_CMGS_1[] = "AT+CMGS=35";
//char ATDCall[]   = "ATD18078357607;";


uchar Electricity[5]   = {0xff, 0x60, 0x22, 0x00, 0xff}; //主电量
uchar Electricity_1[5] = {0xff, 0x64, 0x22, 0x00, 0xff}; //副电量
uchar Humidity[5]      = {0xff, 0x61, 0x22, 0x00, 0xff}; //湿度
uchar Temperature[5]   = {0xff, 0x62, 0x22, 0x00, 0xff}; //温度
uchar Concentration[5] = {0xff, 0x63, 0x22, 0x00, 0xff}; //浓度
uchar Pictures[5]      = {0xff, 0x89, 0x06, 0x00, 0xff}; //抓取现场图像

//void Forward();
//void Back();
//void Stop();
//void Gsm_Pdu();
//void Serial_Receiving();


void setup() {
  // put your setup code here, to run once:
    mySerial.begin(115200);
    Serial.begin(9600);
    myservo.attach(11);  // 该舵机由arduino第11脚控制
    pinMode(LED, OUTPUT);
    pinMode(DHT11PIN, OUTPUT);    //定义DHT11D的输出口
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(Elec_2PIN, INPUT);
    pinMode(Elec_1PIN, INPUT);
    pinMode(GsmPIN, INPUT);
    pinMode(Buzz, OUTPUT);
    Init_Data();
    MsTimer2::set(1000, flash); //中断设置函数，每1000ms进入一次中断
    MsTimer2::start();//开始计时

}

void loop() {
  // put your main code here, to run repeatedly:
  //串口从上位机接收数据
  if(flag == 0)
  {
//     Forward();
      Stop(20);

//     Forward_count(13);
//      Stop(1);
//      Turn_right(9);
//      Stop(1);
//      Forward_count(13);
//      Stop(100);
      flag++;
  }
//  else
//   Stop(20);

  
       Serial_Receiving();
  //前进
      while (State == 8)
      {
        Forward();
        Serial_Receiving();
      }
      //后退
      while (State == 1)
      {
        Back();
        Serial_Receiving();
      }
      //停止
      while (State == 2)
      {
        Stop(10);
        Serial_Receiving();
      }

      if(State == 9)
        { 
          Steering();
          State=0;
        }

 if(millis()>60000)
 {
  int Gsm_Vaule = digitalRead(GsmPIN);
  if (Gsm_Vaule == 0)
  {  
    int ST = 0;
    if (gsm_count < 1)
    {
      delay(1);
//       Gsm_Pdu();
      gsm_count++;
    while (1)
        {
//    Serial.print("  GSM   ");
//    Serial.print(Gsm_Vaule);
//    Serial.print("  OVER  ");
          Stop(10);
          if(ST == 0)
          {
          Serial.write(Pictures,5);
          delay(1);
          Steering(); 
          ST =1;
          }
            //刷新传感器数据
          Dht11();
          MQ_2();
          Electricity_Display();
          Electricity_Display_1();    
          
          if (Val>30 && Temp >30)
          {
          GPRS_Call();
// Serial.print("Call llllllllllllllllllllllll");
          }
        }
     
      
    }
  }
}

  if (Val>40 && Temp >30)
  {
    GPRS_Call();
//    Serial.print("Call llllllllllllllllllllllll");
  }

  //刷新传感器数据
  Dht11();
  MQ_2();
  Electricity_Display();
  Electricity_Display_1();





  /***************************测试程序****************************************************************************************/
  //测试软串口接收到的数据
  //      if(mySerial.available()>0)
  //     {
  //       Serial.print((char)mySerial.read());
  //     }

  // 蜂鸣器测试
  //  digitalWrite(Buzz, HIGH);
  //  delay(100);
  //  digitalWrite(Buzz, LOW);

  //  //一级报警
  //  int Gsm_Vaule = digitalRead(GsmPIN);
  ////  int Gsm_Vaule_1 = !Gsm_Vaule;
//    Serial.print("  GSM   ");
//    Serial.print(Gsm_Vaule);
//    Serial.print("  OVER  ");
  ////  Serial.print("  GSM_1   ");
  ////  Serial.print(Gsm_Vaule_1);
  ////  Serial.print("  OVER  ");
  //if(millis()>60000)
  // {  if (Gsm_Vaule == 0)
  //  { //Gsm_Pdu_Yiji();
  ////    Forward();
  //  digitalWrite(Buzz, HIGH);
  //  delay(100);
  //  digitalWrite(Buzz,  LOW);
  //  }
  // }
  //

  /**********2018年7月26日测试电机内容*****************/
  //      if (comdata[3] == 0x07) //判断按键
  //      Back();
  //      else
  //      Forward();

  /*********************************************************************************************************************************/

}

void Serial_Receiving()
{
  while (Serial.available() > 0)
  {
    comdata += char(Serial.read()); //一位一位的读取串口数据，串接到comedata
    delay(2);
  }
  State = comdata[3];
  Serial.print("  State  ");
  Serial.println(State);
   Serial.print("  OVER  ");
  comdata = ""; //数据清空，否则会影响下一次的数据
}

/************************************模块函数******************************************/

/***************************
   初始化
 ***************************/
void Init_Data()
{
  Stop(10);
  myservo.write(30); //初始化摄像头起始位置
  Serial.write(Temperature, 5);
  Serial.write(Humidity, 5);
  Serial.write(Electricity, 5);
  Serial.write(Electricity_1, 5);
  Serial.write(Concentration, 5);
}

/***************************
  串口发送数据到上位机
***************************/
void flash()
{
  Temperature[2] = Temp;   //温度
  Humidity[2] = Hum;
  Concentration[2] = Val;  //浓度
  Electricity[2] = Val_E;
  Electricity_1[2] = Val_E_1;
  delay(10);
  Serial.write(Temperature, 5);   //发送一帧温度数据
  Serial.write(Humidity, 5);      //发送一帧湿度数据
  Serial.write(Electricity, 5);   //发送一帧主电量数据
  Serial.write(Electricity_1, 5); //发送一帧副电量数据
  Serial.write(Concentration, 5); //发送一帧浓度数据
}

/*****************************
   MQ_2烟雾传感器
 *****************************/
void MQ_2()
{
  int val = analogRead(MQ_2PIN); //读取模拟脚的数据
  Val = (val / 10.24);
//    Serial.print("Val=");
//    Serial.println(Val, DEC); //输出十进制
  delay(100);
}

/*****************************
   DHT11温湿度
 *****************************/
void Dht11()
{
  int chk = DHT11.read(DHT11PIN);                 //将读取到的值赋给chk
  Temp = (float)DHT11.temperature;             //将温度值赋值给
  Hum  = (float)DHT11.humidity;                 //将湿度值赋给
//    Serial.print("Tempeature:");                        //打印出Tempeature:
//    Serial.println(Temp);                                     //打印温度结果
  //
  //  Serial.print("Humidity:");                            //打印出Humidity:
  //  Serial.print(Hum);                                     //打印出湿度结果
  //  Serial.println("%");                                  //打印出%
  //  delay(10);                                       //延时一段时间
}

/*****************************
  主电量显示
*****************************/
void Electricity_Display()
{
  float val   = analogRead(Elec_1PIN);
  float val1  = (val * 5.6 / 1023 + 1.2); //将15V电压转化为5V为基准电压
  Val_E = val1 / 5.6 * 100; //电量百分比
  //  Serial.print("模拟量:");
  //  Serial.print(val);
  //  Serial.print("\t");
  //  Serial.print("主电压:");
  //  Serial.print(val1);
  //  Serial.print("\t");
  //  Serial.print("电量百分比:");
  //  Serial.println(Val_E);
  //  delay(1000);
}

/*****************************
  副电量显示
*****************************/
void Electricity_Display_1()
{
  float val_1   = analogRead(A1);
  float val1_1  = (val_1 * 5 / 1023 + 0.8); ////将10V电压转化为5V为基准电压
  Val_E_1 = val1_1 / 5 * 100; //电量百分比
  //  Serial.print(" 模拟量: ");
  //  Serial.print(val_1);
  //  Serial.print("\t");
  //  Serial.print(" 副电压: ");
  //  Serial.print(val1_1);
  //  Serial.print("\t");
  //  Serial.print(" 电量百分比: ");
  //  Serial.println(Val_E_1);
  //  delay(1000);
}

/****************************
   GSM短信模块
 ****************************/
void Gsm_Pdu(void)
{
  mySerial.print("\r");
  delay(1000);
  mySerial.println(AT_CMHF);
  delay(500);
  mySerial.println(AT_CMGS_1);
  delay(1000);
  mySerial.print(SEND_Message_1); //发送内容：河池学院智能控制实验室
  delay(1000);
  mySerial.write(0x1A); //或者 Serial.print("\x01A");
  delay(100);
}

/****************************
   电话呼叫
 ****************************/
void GPRS_Call(void)
{
  mySerial.println(ATDCall);
  delay(2000);
}

/**********************************************
  摄像头转向角度
**********************************************/
void Steering()
{
      myservo.write(0); // 指定舵机转向的角度 
      delay(650);// 等待15ms让舵机到达指定位置 
      myservo.write(30); // 指定舵机转向的角度 
      delay(650);// 等待15ms让舵机到达指定位置 
      myservo.write(60); // 指定舵机转向的角度 
      delay(650);// 等待15ms让舵机到达指定位置 
      myservo.write(90); // 指定舵机转向的角度 
      delay(650);// 等待15ms让舵机到达指定位置 
      myservo.write(120); // 指定舵机转向的角度 
      delay(650);// 等待15ms让舵机到达指定位置 
      myservo.write(150); // 指定舵机转向的角度 
      delay(800);// 等待15ms让舵机到达指定位置 
      myservo.write(180); // 指定舵机转向的角度 
      delay(1000); // 等待15ms让舵机到达指定位置 
      myservo.write(30); // 指定舵机转向的角度 
      delay(650); // 等待15ms让舵机到达指定位置 
}

/***********************************电机驱动**************************/

/*****************************
   前进
 *****************************/
void Forward()
{
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
  delay(100);

}

/*****************************
   后退
 *****************************/
void Back()
{
  //左轮
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);
  //右轮
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
  delay(100);
  //  digitalWrite(9, LOW);
  //  digitalWrite(5, LOW);
  //  digitalWrite(6, HIGH);
  //  digitalWrite(10,HIGH);
  //  delay(300);
  //  digitalWrite(9, LOW);
  //  digitalWrite(5, LOW);
  //  digitalWrite(6, LOW);
  //  digitalWrite(10,LOW);
  //  delay(700);
}

/*****************************
   停止
 *****************************/
void Stop(int s2)
{
  while (s2--) {
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    delay(100);
  }
}
//

void Turn_right(int s)
{
  while (s--) {
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    delay(100);
    //  digitalWrite(9, LOW);
    //  digitalWrite(10, LOW);
    //  digitalWrite(5, LOW);
    //  digitalWrite(6, LOW);
    //  delay(10);

  }
}

void Forward_count(int s1)
{
  while (s1--)
  {
    digitalWrite(9, HIGH);
    digitalWrite(10, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    delay(100);
  }
}









