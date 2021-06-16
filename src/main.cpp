#pragma region Includes
#pragma region Includes - Arduino
#include <Arduino.h>
#include "pins_arduino.h"
#pragma endregion Includes - Arduino

#pragma region Includes - C / C++
#include <math.h>
#pragma endregion Includes - C / C++

#pragma region Includes - Libraries
#include <ServoCds55.h>
#include <NewPing.h>
#pragma endregion Includes - Libraries
#pragma endregion Includes

#pragma region Pin definitions
#define LPINGPIN 2
#define FPINGPIN 3
#define RPINGPIN 4
#pragma endregion Pin definitions

#pragma region Signals
#define ROGER_THAT "!"
#define ROTATION_COMPLETED "@"
#define REQUEST_FOR_LEFT_ROTATION "#"
#define REQUEST_FOR_RIGHT_ROTATION "$"

#pragma region Signals - Interrupt
#define STOP "%"
#define ROGER_INTERRUPT "^"
#define END_INTERRUPT "*"
#define START "&"
#define BACKWARDS "("
#pragma endregion Signals - Interrupt
#pragma endregion Signals


bool startStopBool = false;
bool leftPriority = false;

bool foundWall = false;

bool turning = false;

double initialValue;

int sideSensorDistance;
int forwardSensorDistance;

ServoCds55 myservo;

NewPing sonar[3] = {
    NewPing(LPINGPIN, LPINGPIN, 400),
    NewPing(FPINGPIN, FPINGPIN, 400),
    NewPing(RPINGPIN, RPINGPIN, 400)};

#pragma region Function definitions
void establishContact();
uint16_t checkPing(int pingOnPin);
void rotate(int left, int right);
void obstacleInTheFront();
void waitForString(String wantedString);
void checkForInterrupt();
void sideRotation();
#pragma endregion Function definitions

void establishContact()
{
  while (Serial.available() <= 0)
  {
    Serial.println('A');
    delay(300);
  }
}

uint16_t checkPing(int pingOnPin)
{
  uint16_t returnDistance = 0;
  if (pingOnPin == FPINGPIN)
  {
    returnDistance = sonar[1].ping_cm();
    if (returnDistance == 0)
    {
      delay(50);
    }
  }
  else if (pingOnPin == RPINGPIN)
  {
    returnDistance = sonar[2].ping_cm();
    if (returnDistance == 0)
    {
      delay(50);
      returnDistance = sonar[2].ping_cm();
    }
  }
  else if (pingOnPin == LPINGPIN)
  {
    returnDistance = sonar[0].ping_cm();
    if (returnDistance == 0)
    {
      delay(50);
      returnDistance = sonar[0].ping_cm();
    }
  }

  return returnDistance;
}

void directRotate(int left, int right)
{
  myservo.rotate(1, left);
  myservo.rotate(2, right * -1);
}

void rotate(int left, int right)
{
  if (leftPriority)
  {
    myservo.rotate(1, left * -1);
    myservo.rotate(2, right);
  }
  else
  {
    myservo.rotate(1, left);
    myservo.rotate(2, right * -1);
  }
}

void obstacleInTheFront()
{
  if (leftPriority)
  {
    Serial.println(REQUEST_FOR_RIGHT_ROTATION);
    waitForString(ROGER_THAT);
    directRotate(50, -50);
  }
  else
  {
    Serial.println(REQUEST_FOR_LEFT_ROTATION);
    waitForString(ROGER_THAT);
    directRotate(-50, 50);
  }
  waitForString(ROTATION_COMPLETED);
  myservo.rotate(1, 0);
  myservo.rotate(2, 0);
}

void backwards()
{
  Serial.println(ROGER_THAT);
  directRotate(-150, 150);
  waitForString(STOP);
  myservo.rotate(1, 0);
  myservo.rotate(2, 0);
  obstacleInTheFront();
  obstacleInTheFront();
}

void waitForString(String wantedString)
{
  String recievedData = Serial.readStringUntil('\n');
  while (recievedData != wantedString)
  {
    recievedData = Serial.readStringUntil('\n');
  }
  if (wantedString != ROGER_THAT)
    Serial.println(ROGER_THAT);
}

void checkForInterrupt()
{
  Serial.setTimeout(500);
  String recievedData = Serial.readStringUntil('\n');
  Serial.setTimeout(1000);
  if (recievedData == STOP)
  {
    myservo.rotate(1, 0);
    myservo.rotate(2, 0);

    Serial.println(ROGER_INTERRUPT);
    recievedData = Serial.readStringUntil('\n');
    while (recievedData != END_INTERRUPT && recievedData != BACKWARDS)
    {
      recievedData = Serial.readStringUntil('\n');
    }

    if (recievedData == BACKWARDS)
    {
      backwards();
      return;
    }

    Serial.println(ROGER_INTERRUPT);
  }
}

void sideRotation()
{
  myservo.rotate(1, 0);
  myservo.rotate(2, 0);
  if (leftPriority)
  {
    Serial.println(REQUEST_FOR_LEFT_ROTATION);
    waitForString(ROGER_THAT);
    directRotate(15, 150);
  }
  else
  {
    Serial.println(REQUEST_FOR_RIGHT_ROTATION);
    waitForString(ROGER_THAT);
    directRotate(150, 15);
  }
  waitForString(ROTATION_COMPLETED);
  myservo.rotate(1, 0);
  myservo.rotate(2, 0);
}

void setup(void)
{
  Serial.begin(115200);
  myservo.begin();
  delay(1000);

  myservo.rotate(1, 0);
  myservo.rotate(2, 0);

  delay(8000);
  waitForString(START);
}

void loop()
{
  checkForInterrupt();

  sideSensorDistance = checkPing(leftPriority ? LPINGPIN : RPINGPIN);
  delay(29);
  forwardSensorDistance = checkPing(FPINGPIN);

  if (sideSensorDistance > 300)
  {
    sideSensorDistance = 0;
  }
  else if (forwardSensorDistance > 300)
  {
    forwardSensorDistance = 0;
  }

  if (forwardSensorDistance < 13)
  {
    // Serial.println("Front rotation");
    if (sideSensorDistance > 13)
    {
      sideRotation();
      return;
    }
    else
    {
      foundWall = true;
      myservo.rotate(1, 0);
      myservo.rotate(2, 0);

      obstacleInTheFront();
      return;
    }
  }
  else if (sideSensorDistance > 17)
  {
    sideRotation();
    return;
  }

  switch (sideSensorDistance)
  {
  case 0 ... 2:
  {
    rotate(45, 150);
    break;
  }
  case 3 ... 8:
  {
    int side = 15 + ((sideSensorDistance - 2) * 5);
    rotate(side, 150);
    foundWall = true;
    break;
  }

  case 10 ... 16:
  {
    int side = 150 - ((sideSensorDistance - 9) * 5);
    rotate(150, side);
    foundWall = true;
    break;
  }

  default:
    rotate(150, 150);
    break;
  }
}
