#include <Arduino.h>
#include <Ticker.h>
#include "Floor0.h"
#include "rtwtypes.h"
#include <SoftwareSerial.h>

static Floor0ModelClass rtObj;
// Instance of model class
SoftwareSerial BusSerial(BUSRX, BUSTX);
char bufferWrite;
char bufferRead;
int bits[8];
char received[3];
char registerAddress;

int button[4] = {0};
void writeToBus(char ra, int btn, boolean send)
{
  if (send)
  {
    char deviceAddress = 0b00000001;
    digitalWrite(BUSCTRL, HIGH);
    BusSerial.write(deviceAddress);
    BusSerial.write(ra);
    BusSerial.write((char)btn);
    digitalWrite(BUSCTRL, LOW);
  }
}

void InterruptSrevicesHandlerRead(char data, int pol)
{
  button[pol] = data;
  switch (pol)
  {
  case 0:
    rtObj.rtY.l1 = data ? true : false;
    digitalWrite(LED1, rtObj.rtY.l1 == true ? HIGH : LOW);
    break;

  case 1:
    rtObj.rtY.l2 = data ? true : false;
    digitalWrite(LED2, rtObj.rtY.l2 == true ? HIGH : LOW);
    break;
  case 2:
    rtObj.rtY.l3 = data ? true : false;
    digitalWrite(LED3, rtObj.rtY.l3 == true ? HIGH : LOW);
    break;
  case 3:
    rtObj.rtY.l4 = data ? true : false;
    digitalWrite(LED4, rtObj.rtY.l4 == true ? HIGH : LOW);
    break;

  default:
    break;
  }
}

void InterruptSrevicesHandler1(boolean sendOrNot)
{
  button[0] = !button[0];
  rtObj.changeButtonState(1);
  delay(2);
  registerAddress = 0b00000001;
  Serial.println("InterruptSrevicesHandler1");
  writeToBus(registerAddress, button[0], sendOrNot);
}
void InterruptSrevicesHandler2(boolean sendOrNot)
{
  button[1] = !button[1];
  rtObj.changeButtonState(2);
  delay(2);
  registerAddress = 0b00000010;
  Serial.println("InterruptSrevicesHandler2");
  writeToBus(registerAddress, button[1], sendOrNot);
}
void InterruptSrevicesHandler3(boolean sendOrNot)
{
  button[2] = !button[2];
  rtObj.changeButtonState(3);
  delay(2);
  registerAddress = 0b00000011;
  Serial.println("InterruptSrevicesHandler3");
  writeToBus(registerAddress, button[2], sendOrNot);
}

void InterruptSrevicesHandler4(boolean sendOrNot)
{
  button[3] = !button[3];
  rtObj.changeButtonState(4);
  delay(2);
  registerAddress = 0b00000100;
  Serial.println("InterruptSrevicesHandler4");
  writeToBus(registerAddress, button[3], sendOrNot);
}

void rt_OneStep(void);
void rt_OneStep(void)
{
  static boolean_T OverrunFlag = false;

  // Disable interrupts here

  // Check for overrun
  if (OverrunFlag)
  {
    rtmSetErrorStatus(rtObj.getRTM(), "Overrun");
    return;
  }

  OverrunFlag = true;

  // Save FPU context here (if necessary)
  // Re-enable timer or interrupt here
  // Set model inputs here

  // Step the model
  rtObj.step();

  // Get model outputs here

  // Indicate task complete
  OverrunFlag = false;

  // Disable interrupts here
  // Restore FPU context here (if necessary)
  // Enable interrupts here
}

void sendStatusToMaster()
{
  int data = 0;
  data += (rtObj.rtY.l1 ? 1 : 0);
  data += (rtObj.rtY.l2 ? 2 : 0);
  data += (rtObj.rtY.l3 ? 4 : 0);
  data += (rtObj.rtY.l4 ? 8 : 0);
  Serial.println(data);
  writeToBus(0b10000000, data, true);
}

Ticker timer1(rt_OneStep, 200);

void setup()
{
  // put your setup code here, to run once:
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(BUSCTRL, OUTPUT);
  digitalWrite(BUSCTRL, LOW);

  Serial.begin(9600);
  rtObj.initialize();
  timer1.start();

  BusSerial.begin(9600);
  // delay(2);
  // Serial.println("Chera?");
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (BusSerial.available() > 0)
  {
    delay(2);
    Serial.println("bufferRead");

    BusSerial.readBytes(received, 3);
    Serial.println(int(received[0]));
    if (received[0] == 2)
    {
      if (received[1] == -128)
      {
        Serial.println("salam");
        sendStatusToMaster();
      }
      else
      {
        Serial.println(int(received[1]));
        switch (received[1])
        {
        case 1:

          InterruptSrevicesHandlerRead(received[2], 0);
          break;
        case 2:
          InterruptSrevicesHandlerRead(received[2], 1);
          break;
        case 3:
          InterruptSrevicesHandlerRead(received[2], 2);
          break;
        case 4:
          InterruptSrevicesHandlerRead(received[2], 3);
          break;

        default:
          break;
        }
      }
    }

    // buffer is your received data...
  }
  else if (digitalRead(BUTTON1))
  {
    InterruptSrevicesHandler1(true);
    while (digitalRead(BUTTON1))
      ;
  }
  else if (digitalRead(BUTTON2))
  {
    InterruptSrevicesHandler2(true);
    while (digitalRead(BUTTON2))
      ;
  }
  else if (digitalRead(BUTTON3))
  {
    InterruptSrevicesHandler3(true);
    while (digitalRead(BUTTON3))
      ;
  }
  else if (digitalRead(BUTTON4))
  {
    InterruptSrevicesHandler4(true);
    while (digitalRead(BUTTON4))
      ;
  }
  delay(2);
  timer1.update();
}