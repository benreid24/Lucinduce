#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <EEPROM.h>

/**   PROGRAM FLOW:
 *    
 *    1. Wait for user to fall asleep
 *      a. Track max movement over THRESHOLD
 *      b. If no movement exceeds THRESHOLD in SLEEPDELAY minutes then asleep
 *    2. Delay INITDELAY minutes
 *    3. Loop:
 *      a. Wait until max exceeds THRESHOLD or CYCLELEN minutes elapsed
 *      b. Continuously flash LEDs
 *      c. Continue until max stays below THRESHOLD for CYCLEDELYAY minutes or CYCLELEN minutes elapsed
 *      d. Delay PERIOD minutes
 *      
 *    Notes:
 *      1. Max movement is logged once per minute in EEPROM starting at address MAXCYCLES*2+1
 *      2. Predicted sleep time is recorded in EEPROM at address 0
 *      3. Predicted cycle times are recorded in EEPROM starting at address 1 to address MAXCYCLES
 *      4. Predicted cycle end times are recorded in EEPROM starting at address MAXCYCLES+1 to MAXCYCLES*2
 *      
 *    EEPROM Structure:
 *    +---------------+---------------+
 *    |    Address    |     Data      |
 *    +---------------+---------------+
 *    | 0             | Sleep start   |
 *    | 1             | Cycle start 1 |
 *    | ...           | Cycle starts  |
 *    | MAXCYCLES     | Cycle start n |
 *    | MAXCYCLES+1   | Cycle end 1   |
 *    | ...           | Cycle ends    |
 *    | MAXCYCLES*2   | Cycle end n   |
 *    | MAXCYCLES*2+1 | Data start    |
 *    | ...           | Data points   |
 *    | 1023          | End of data   |
 *    +---------------+---------------+
 *
 */

#define MAXCYCLES 8
#define THRESHOLD 50

#define SLEEPDELAY 7
#define INITDELAY 40
#define CYCLELEN 35
#define CYCLEDELAY 10
#define PERIOD 80

#define FLASHLEN 800
#define BRIGHTNESS 90

MPU6050 imu;
int16_t gx(0), gy(0), gz(0);
int offx = 0, offy = 0, offz = 0;
int eepromAddr = MAXCYCLES*2+1;

//data tracking
int samples = 0;
uint8_t mMax = 0;
uint8_t cyclesDetected = 1; //used for saving cycle detection times to EEPROM, we can write up to MAXCYCLES
uint8_t cycleEndsDetected = MAXCYCLES+1; //used for saving cycle end times to EEPROM, we can write up to address MAXYCLES*2
int minutesElapsed = 0;
int lastMaxTime = 0; //set to current time when max>50

//delays in increments of 100 milliseconds. This increment was chosen to easily take samples without having to thread
void centiDelay(int centis)
{
  int tot = 0;
  for (int i = 0; i<centis; ++i)
  {
    imu.getRotation(&gx,&gy,&gz);
    gx = abs(gx-offx)/(150);
    gy = abs(gy-offy)/(150);
    gz = abs(gz-offz)/(150);
    tot = gx+gy+gz;
    if (tot>mMax)
      mMax = tot;

    //user connected
    if (Serial) {
      Serial.println("Serial connected, terminating data acquisition");
      Serial.println("EEPROM Data:");
      for (int i = 0; i<1024; ++i) {
        Serial.println(EEPROM.read(i));
      }
      Serial.println("Data finished");
      waitForever();
    }

    samples++;
    if (samples==600) //1 minute
    {
      samples = 0;
      minutesElapsed++;
      if (mMax>=50)
        lastMaxTime = minutesElapsed;
      
      //log data
      if (eepromAddr<1024)
      {
        EEPROM.write(eepromAddr, mMax);
        Serial.print("Max: ");
        Serial.println(mMax);
        eepromAddr++;
      }
      mMax = 0;
    }
    delay(100);
  }
}

void delayMinutes(int m)
{
  for (int i = 0; i<m; ++i)
    centiDelay(600);
}

void setState(int s)
{
  for (int i = 9; i<=10; ++i)
    analogWrite(i,s);
}

void waitSleep()
{
  while (minutesElapsed-lastMaxTime<=SLEEPDELAY)
    centiDelay(10); //1 second
  EEPROM.write(0,minutesElapsed-SLEEPDELAY/2);
}

void waitCycle()
{
  int sTime = minutesElapsed;

  while (minutesElapsed-sTime<=CYCLELEN && mMax<50)
    centiDelay(1);

  if (cyclesDetected<=MAXCYCLES)
    EEPROM.write(cyclesDetected,minutesElapsed%256); //mod to keep in range
  cyclesDetected++;
}

void flash(int t)
{
  setState(BRIGHTNESS);
  delay(t);
  setState(0);
  delay(t);
}

void waitForever()
{
  while (true)
    flash(2000);
}

void setup() {
  //begin serial for debug
  Serial.begin(9600);

  //set pins for output and to be off
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  setState(0);

  //check jumper
  if (analogRead(1)>1000)
  {
    while (!Serial)
      flash(200);
    Serial.println("Jumper detected, not overwriting EEPROM");
    Serial.println("EEPROM Data:");
    for (int i = 0; i<1024; ++i) {
      Serial.println(EEPROM.read(i));
    }
    Serial.println("Data finished\n");
    waitForever();
  }

  //initialize and zero imu
  setState(BRIGHTNESS); //signal calibration
  Wire.begin();
  imu.initialize();
  delay(2500);
  for (int i = 0; i<6; ++i)
  {
    imu.getRotation(&gx,&gy,&gz);
    offx += gx;
    offy += gy;
    offz += gz;
    delay(100);
  }
  offx /= 6;
  offy /= 6;
  offz /= 6;
  setState(0); //signal calibration done

  //user connected
  if (Serial) {
    Serial.println("Serial connected, terminating data acquisition");
    Serial.println("EEPROM Data:");
    for (int i = 0; i<1024; ++i) {
      Serial.println(EEPROM.read(i));
    }
    Serial.println("Data finished");
    waitForever();
  }

  //clear EEPROM
  for (int i = 0; i<1024; ++i)
    EEPROM.write(i,171);
  
  //wait for the user to fall asleep and enter rem sleep
  waitSleep();
  
  //first delay
  delayMinutes(INITDELAY-SLEEPDELAY/2);
}

void loop() {
  //wait for a cycle to start
  waitCycle();
  
  //flash LED for for up to CYCLELEN minutes or if we go back to deep sleep
  int sTime = minutesElapsed;
  while (minutesElapsed-sTime<=CYCLELEN && minutesElapsed-lastMaxTime<=CYCLEDELAY)
  {
    setState(BRIGHTNESS);
    centiDelay(FLASHLEN/100);
    setState(0);
    centiDelay(FLASHLEN/100);
  }

  //write cycle end time
  if (cycleEndsDetected<=MAXCYCLES*2) {
    EEPROM.write(cycleEndsDetected,minutesElapsed%256); //mod to keep in range
    cycleEndsDetected++;
  }

  //wait for a while until the next cycle
  delayMinutes(PERIOD);
}
