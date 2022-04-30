#include "dht11.h"
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include <Servo.h>
#include "SCoop.h"

#define DHT11PIN 8
#define RGB_RED_pin 11
#define RGB_GREEN_pin 10
#define RGB_BLUE_pin 9
#define buzzer_pin 2
#define button_pin 4
#define body_pin 7
#define smoke_Ao_pin A0
#define smoke_Do_pin 12
#define servo_pin 5

#define temperature_threshold 24
#define smoke_threshold 200

Servo myservo; // 定义舵机对象，最多八个
int pos = 0;   // 定义舵机转动位置

dht11 DHT11;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

int flag = 0;
int safevalue = 0;
unsigned int smokeValue = 0;
int hadopen = 0;

int Tep_max = 0, Smk_max = 0;
int Tep_min = 100, Smk_min = 150;
char *door = "关";
char *people = "否";
char *people_in_lab = "no";

void setColor(int red, int green, int blue)
{
    analogWrite(RGB_RED_pin, red);
    analogWrite(RGB_GREEN_pin, green);
    analogWrite(RGB_BLUE_pin, blue);
}

void smoke()
{
    smokeValue = analogRead(smoke_Ao_pin);
    //Serial.print("smokeSensor Value = ");
    //Serial.println(smokeValue); //输出气体浓度模拟值
    if (digitalRead(smoke_Do_pin) == LOW)
    {
        //Serial.println("smoke Alarm!!!"); //低电平警报
    }
    else
    {
        //Serial.println("smoke normal!!!");
    }
}

void LCD0(int j)
{
    lcd.setCursor(0, 0);
    lcd.print("initialize still");

    lcd.setCursor(0, 1);
    lcd.print("needs: ");
    lcd.print(j);
    lcd.print(" s");
    lcd.print("       ");
}

void LCD1()
{
    lcd.setCursor(0, 0);
    lcd.print("Tep:");
    lcd.print((int)DHT11.temperature);
    lcd.print("'C");
    lcd.print(" ");
    lcd.print("Smk:");
    lcd.print(smokeValue);
    lcd.print("  ");
}

void LCD2()
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    lcd.print("man in lab: ");
    lcd.print(people_in_lab);
    lcd.print("  ");
}

void judge()
{
    if ((int)DHT11.temperature > temperature_threshold || smokeValue > smoke_threshold)
    {
        safevalue = 1;
    }
    else if ((int)DHT11.temperature <= temperature_threshold && smokeValue <= smoke_threshold)
    {
        safevalue = 0;
    }
}

void closedoor()
{
    // 0到180旋转舵机
    for (pos = 0; pos < 110; pos += 1)
    {
        myservo.write(pos);
        sleep(5);
    }
    sleep(300);
    door = "开";
}

void opendoor()
{
    // 180到0旋转舵机
    for (pos = 110; pos >= 1; pos -= 1)
    {
        myservo.write(pos);
        sleep(5);
    }
    sleep(300);
    door = "关";
}

int findmax(int m, int n)
{
    if (m >= n)
    {
        return m;
    }
    else
    {
        return n;
    }
}

int findmin(int m, int n)
{
    if (m <= n)
    {
        return m;
    }
    else
    {
        return n;
    }
}

void rasp_arduino()
{
    if (Serial.available()) //判断串口缓存区有没有数据
    {
        if ('s' == Serial.read()) //有数据就用read来读取并判断是不是s
        {
            Serial.print("最大温度：");
            Serial.println(Tep_max);
            Serial.print("最小温度：");
            Serial.println(Tep_min);
            Serial.print("最大烟雾浓度模拟值：");
            Serial.println(Smk_max);
            Serial.print("最小烟雾浓度模拟值：");
            Serial.println(Smk_min);
            Serial.print("实验室是否有人：");
            Serial.println(people);
            Serial.print("实验室门开关情况：");
            Serial.println(door);
        } //是的话就向串口打印这串字符
    }
}

void rasp_arduino1()
{
    Serial.print("最大温度：");
    Serial.println(Tep_max);
    Serial.print("最小温度：");
    Serial.println(Tep_min);
    Serial.print("最大烟雾浓度模拟值：");
    Serial.println(Smk_max);
    Serial.print("最小烟雾浓度模拟值：");
    Serial.println(Smk_min);
    Serial.print("实验室是否有人：");
    Serial.println(people);
    Serial.print("实验室门开关情况：");
    Serial.println(door);
}

//完整定义
defineTask(TaskTest);
void TaskTest::setup()
{
    pinMode(RGB_RED_pin, OUTPUT);
    pinMode(RGB_GREEN_pin, OUTPUT);
    pinMode(RGB_BLUE_pin, OUTPUT);

    pinMode(buzzer_pin, OUTPUT);
    pinMode(button_pin, INPUT);
    pinMode(body_pin, INPUT);

    pinMode(smoke_Do_pin, INPUT);

    pinMode(DHT11PIN, INPUT);
    lcd.init(); // initialize the lcd

    lcd.backlight();

    myservo.attach(servo_pin); // 设置舵机控制针脚

    //opendoor();
    for (int i = 60; i >= 1; i--) //延迟1分钟越过初始化不稳定状态
    {
        LCD0(i);
        delay(1000);
    }
}

void TaskTest::loop()
{
    //Serial.println("start");
    DHT11.read(DHT11PIN);
    smoke();
    LCD1();

    Tep_max = findmax(Tep_max, (int)DHT11.temperature);
    Smk_max = findmax(Smk_max, (int)smokeValue);
    Tep_min = findmin(Tep_min, (int)DHT11.temperature);
    Smk_min = findmin(Smk_min, (int)smokeValue);

    /*Serial.print("温度：");
    Serial.print((float)DHT11.temperature);
    Serial.println("'C");
    Serial.print("湿度：");
    Serial.print((float)DHT11.humidity);
    Serial.println("%");*/

    if (digitalRead(body_pin) == HIGH)
    {
        people_in_lab = "yes";
        LCD2();
        //Serial.println("There is someone in the lab！！！");
        if (hadopen == 0)
        {
            opendoor();
        }
        people = "是";
        hadopen = 1;
    }
    else
    {
        people_in_lab = "no";
        LCD2();
        //Serial.println("No one in the lab...");
        if (hadopen == 1)
        {
            closedoor(); //无人关门
        }
        people = "否";
        hadopen = 0;
    }
    //sleep(5000);

    //closedoor(); //最后使用记得注释

    if (digitalRead(button_pin) == LOW)
    {
        flag = 1;
        if (safevalue == 1)
        {
            //Serial.println("The button to clear the alarm has been pressed！！！");
        }
    }
    else
    {
        flag = 0;
    }

    judge();

    if (safevalue == 0)
    {
        //Serial.println("G led !");
        setColor(255, 0, 255);
        digitalWrite(buzzer_pin, LOW);
    }
    else if (safevalue == 1 && flag == 0)
    {
        //Serial.println("R led !");
        setColor(0, 255, 255);
        digitalWrite(buzzer_pin, HIGH);
    }
    else if (safevalue == 1 && flag == 1)
    {
        //Serial.println("B led !");
        setColor(255, 255, 0);
        digitalWrite(buzzer_pin, LOW);
        sleep(10000);
    }

    //Serial.println("end");
    //Serial.println(" ");
    sleep(300);
}

//快速定义
defineTaskLoop(TaskTest1)
{
    rasp_arduino();
}

void setup()
{
    Serial.begin(115200);
    mySCoop.start();
}

void loop()
{
    yield();
}
