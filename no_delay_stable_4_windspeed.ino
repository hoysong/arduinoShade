#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
int temp;
int downTemp;

/*=================================================================*/
int levelState;

int autois, windis, tempis, lightis;
int toggleAuto = 53;
int toggleWind = 51;
int toggleTemp = 49;
int toggleLight = 47;

int toggleValueTop = 46;
int toggleValueMiddle = 48;
int toggleValueLow = 50;
int toggleValue;

float windSpeed;
float windMs;

int windHigh;
int tempHigh;
int lightHigh;

int relayOne = 8;
int relayTwo = 9;
int relayThree = 10;
int relayFour;

unsigned long t1,t2;
int step = 0;
int stepOne = 0;
int stepTwo = 0;
int opentimerOn = 0;
int closetimerOn = 0;
int isopen = 0;

int windSum;
int windAverage;
int windCounter;
int windAverageLimit = 7;

#define CDS_10 953

int lightOne;
int lightMapOne;
int lightCounterOne;
int lightAverageLimitOne = 7;
int lightAverageOne;
int lightSumOne;
int light_test1;
int test_lightsum;

int lightTwo;
int lightMapTwo;
int lightCounterTwo;
int lightAverageLimitTwo = 7;
int lightAverageTwo;
int lightSumTwo;

int lightTopValue;

unsigned long stateUpTime;

int windTopValue, windLowValue, tempTopValue, tempLowValue, lightHighValue, lightLowValue;


/*=================================================================*/
void setup() {
  pinMode(toggleAuto, INPUT_PULLUP);
  pinMode(toggleWind, INPUT_PULLUP);
  pinMode(toggleTemp, INPUT_PULLUP);
  pinMode(toggleLight, INPUT_PULLUP);


  pinMode(toggleValueTop, INPUT_PULLUP);
  pinMode(toggleValueLow, INPUT_PULLUP);

  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);

  digitalWrite(relayOne, HIGH);
  digitalWrite(relayTwo, HIGH);
  digitalWrite(relayThree, HIGH);

  Serial.begin(9600);
}

/*=================================================================*/
void loop() {
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  downTemp = temp - 6;

  windSpeed = analogRead(A1);

  lightOne = analogRead(A2);
  lightMapOne = map(lightOne, 170, 1050, 0, 100);

  lightTwo = analogRead(A3);
  lightMapTwo = map(lightTwo, 170, 1050, 0, 100);

  windMs = (windSpeed / 1024.0 * 5.0) * 25; //풍속계에서 들어오는 아날로그값을 디지털값으로 수정, 수정값을 풍속
  light_test1 = lightOne + lightTwo;
  test_lightsum = light_test1 / 2;
/*==================== 토글스위치 판단 ====================*/
  if(digitalRead(toggleAuto) == LOW) {
    autois = 1;
  }else {
    autois = 0;
  }

  if(digitalRead(toggleWind) == LOW) {
    windis = 1;
  }else {
    windis = 0;
  }

  if(digitalRead(toggleTemp) == LOW) {
    tempis = 1;
  }else {
    tempis = 0;
  }

  if(digitalRead(toggleLight) == LOW) {
    lightis = 1;
  }else {
    lightis = 0;
  }

  if(digitalRead(toggleValueTop) == LOW) {
    toggleValue = 3;

    windTopValue = 55 + 15;
    windLowValue = 35 + 15;
    tempTopValue = 22 + 2;
    tempLowValue = 20 + 2;
    lightHighValue = 80;
    lightLowValue = 60;

  } else if(digitalRead(toggleValueLow) == LOW) {
    toggleValue = 1;

    windTopValue = 45;
    windLowValue = 30;
    tempTopValue = 22 - 2;
    tempLowValue = 20 - 2;
    lightHighValue = 60 - 10;
    lightLowValue = 40 - 10;

  } else {
    toggleValue = 2;

    windTopValue = 55;
    windLowValue = 35;
    tempTopValue = 22;
    tempLowValue = 20;
    lightHighValue = 60;
    lightLowValue = 40;
  }

/*==================평균값 계산==================*/
  if(windCounter == windAverageLimit) {
    windAverage = round(windSum / windAverageLimit);
    windCounter = 0;
    windSum = 0;
  } else if(windCounter < windAverageLimit) {
    windCounter++;
    windSum = windSum + windSpeed;
  }

  if(lightCounterOne == lightAverageLimitOne) {
    lightAverageOne = round(lightSumOne / lightAverageLimitOne);
    lightCounterOne = 0;
    lightSumOne = 0;
  } else if(lightCounterOne < lightAverageLimitOne) {
    lightCounterOne++;
    lightSumOne = lightSumOne + lightMapOne;
  }

  if(lightCounterTwo == lightAverageLimitTwo) {
    lightAverageTwo = round(lightSumTwo / lightAverageLimitTwo);
    lightCounterTwo = 0;
    lightSumTwo = 0;
  } else if(lightCounterTwo < lightAverageLimitTwo) {
    lightCounterTwo++;
    lightSumTwo = lightSumTwo + lightMapTwo;
  }

/*==================상한값 하한값 판단==================*/
  if(lightAverageOne > lightAverageTwo) {
    lightTopValue = lightAverageOne;
  } else if(lightAverageOne < lightAverageTwo) {
    lightTopValue = lightAverageTwo;
  }

  if(windAverage >= windTopValue){
    windHigh = 1;
  } else if(windAverage <= windLowValue) {
    windHigh = 0;
  }

  if(downTemp >= tempTopValue) {
    tempHigh = 1;
  } else if(downTemp <= tempLowValue) {
    tempHigh = 0;
  }

  if(lightTopValue >= lightHighValue) {
    lightHigh = 1;
  } else if(lightTopValue <= lightLowValue) {
    lightHigh = 0;
  }

  if(autois == 1 && lightis == 1 && lightHigh == 0) {
    digitalWrite(relayThree, LOW);
  } else{
    digitalWrite(relayThree, HIGH);
  }

/*====================== 토글에 따른 센서판단. ======================*/
  if(autois == 1) {
    if(step == 0) {
      if(windis == 1) {
        if(windHigh == 1) {
          closetimerOn = 1;
          levelState = 1;
        } else if(windHigh == 0) {
          if(tempis == 1 && lightis == 1) {
            if(tempHigh == 1 || lightHigh == 1) {
              opentimerOn = 1;
              levelState = 2;
            } else {
              closetimerOn = 1;
              levelState = 3;
            }
          } else if(tempis == 1 && lightis == 0) {
            if(tempHigh == 1) {
              opentimerOn = 1;
              levelState = 4;
            } else {
              closetimerOn = 1;
              levelState = 5;
            }
          } else if(tempis == 0 && lightis == 1) {
            if(lightHigh == 1) {
              opentimerOn = 1;
              levelState = 6;
            } else {
              closetimerOn = 1;
              levelState = 7;
            }
          } else if(tempis == 0 && lightis == 0) {
            if(windHigh == 0) {
              opentimerOn = 1;
              levelState = 8;
            }
          }
        }
      } else if(windis == 0) {
        if(tempis == 1 && lightis == 1) {
          if(tempHigh == 1 || lightHigh == 1) {
            opentimerOn = 1;
            levelState = 9;
          } else {
            closetimerOn = 1;
            levelState = 10;
          }
        } else if(tempis == 1 && lightis == 0) {
          if(tempHigh == 1) {
            opentimerOn = 1;
            levelState = 11;
          } else {
            closetimerOn = 1;
            levelState = 12;
          }
        } else if(tempis == 0 && lightis == 1) {
          if(lightHigh == 1) {
            opentimerOn = 1;
            levelState = 13;
          } else {
            closetimerOn = 1;
            levelState = 14;
          }
        } else if(tempis == 0 && lightis == 0) {
          levelState = 15;
          // nothing
        }
      }
    }
  }

/*======================= open timer=======================*/
  if(opentimerOn == 1 && stepOne == 0){ //스위치가 눌리면 relay on, t1카운터시작
    digitalWrite(relayOne, LOW);
    t1=millis(); // on이후 시간 카운터시작
    stepOne=1;
    step = 1;
  }
  
  if(millis()-t1 >= 250 && stepOne == 1){ //0.25초지나면 off, t2카운터시작
    digitalWrite(relayOne, HIGH);
    t2=millis(); // off이후 시간 카운터시작
    stepOne=2;
  }
  
  if(millis()-t2 >= 1500 && stepOne==2){ //n초지나면 off 유지 종료, 초기단계로
    stepOne=0;
    opentimerOn = 0;
    step = 0;
  }

/*======================= close timer =======================*/
  if(closetimerOn == 1 && stepTwo == 0){ //스위치가 눌리면 relay on, t1카운터시작
    digitalWrite(relayTwo, LOW);
    t1=millis(); // on이후 시간 카운터시작
    stepTwo=1;
    step = 1;
  }
  
  if(millis()-t1 >= 250 && stepTwo ==1){ //0.25초지나면 off, t2카운터시작
    digitalWrite(relayTwo, HIGH);
    t2=millis(); // off이후 시간 카운터시작
    stepTwo=2;
  }
  
  if(millis()-t2 >= 1500 && stepTwo ==2){ //n초지나면 off 유지 종료, 초기단계로
    stepTwo=0;
    closetimerOn = 0;
    step = 0;
  }

/*===================================== state =====================================*/
  if(millis() - stateUpTime >= 1500) {
    stateUpTime = millis();
    state();
  }
}

void state() {
  Serial.println("| Auto | Wind | Temp | Light | Sensor |");
  Serial.println("|  " + String(autois) + "      " + String(windis) + "      " + String(tempis) + "       " + String(lightis) + "       " + String(toggleValue) + "    |");
  Serial.println("Step: " + String(step) + " | " + "level: " + String(levelState));
  Serial.println("Sensor State: " + String(test_lightsum));
  Serial.println("WindSpeed  | " + String(windTopValue) + " " + String(windLowValue) + " " + String(windHigh) + " | " + String(windAverage) + " " + String(windMs , 1));
  Serial.println("Temperture | " + String(tempTopValue) + " " + String(tempLowValue) + " " + String(tempHigh) + " | " + String(downTemp));
  Serial.println("LightValue | " + String(lightHighValue) + " " + String(lightLowValue) + " " + String(lightHigh) + " | " + String(lightAverageOne) + " " + String(lightAverageTwo) + " " + String(lightTopValue));
  Serial.println("=====================================================");
}