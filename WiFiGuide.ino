/* 	Author: Kacper Turon 
	WiFi source finder 
		using 
	Arduino & WiFi shield */
	
#include <Servo.h>
#include <SPI.h>
#include <WiFi.h>

const int rgbRed = A0, rgbGreen = A2, rgbBlue = A1; //Green when prorgam started, blue if connected to wifi, red if disconnected from wifi
const int greenLed = A5, yellowLed = A4; //Red - obstacle avoidance yellow - while taking RSSI measurements ; green - if the source was found ; blue - when is not connected to WiFi
Servo servoLeft, servoRight;

char ssid[]="PromisedLand"; //The lower dbm the stronger connection //GreenNET
char pass[]="waterycoconut068";


bool ssidFound = false; //Is arduino connected to the WiFi with details above
bool sourceFound = false; //Has arduino finished it task 

long prevRssi = 0; //Previous measurement
long rssi = -100; //The most recent measurement

long sensorDistance = 0; //Distance from the obstacle to ultrasonic sensor/ping))) in cm
int angle = 420; //210 - 45 degrees, 420 - 90 degrees

unsigned long prevMilis,milis; //Timer for checking connection to WiFi every minute

int irValue = 0; //0 - Object below IR led 1 - no object

void setup() {
  Serial.begin(9600); 
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(rgbRed, OUTPUT);
  pinMode(rgbGreen, OUTPUT);
  pinMode(rgbBlue, OUTPUT);

  
  setColor(0,255,0);
  servoLeft.attach(9);
  servoRight.attach(8);
  
  ssidStatus();
  if(ssidFound){
	  scan();
	  goStraight();
	  delay(3000);
	  stayStill();
	  updateRSSI();
  }
  prevMilis = millis(); //Get time in milliseconds from when the program started
}

void loop() {
 
  if(ssidFound){
    
   
    if(!sourceFound){
      stayStill();
      delay(500);
      updateRSSI();
      
       if(rssi>-36&&prevRssi>-45){//Source found
        stayStill();
        detachServos();
        
        sourceFound = true;
        digitalWrite(greenLed, HIGH);
        Serial.print("Source found.");
        Serial.println();
      }
      
      if(prevRssi>rssi){
        turnRight();
        goStraight();
        delay(2000);
        
      }else{
        goStraight(); 
        delay(2000);
      }
     
    }
  }else if(!ssidFound&&!sourceFound){//Signal lost check it again
    ssidStatus();
	if(ssidFound){//If signal was found again rescan the area
		scan();
		goStraight();
		delay(6000);
		stayStill();
		updateRSSI();
	}else{
     stayStill();
	}
  }
  milis = millis();
  if(milis-prevMilis>60000){//Check WiFi every minute
	ssidStatus();
	prevMilis = millis();
  }
}

/*MOVEMENT CONTROLS*/
void turnLeft(){
  servoLeft.writeMicroseconds(1300);
  servoRight.writeMicroseconds(1300);
  delay(angle);
}
void turnRight(){
  servoLeft.writeMicroseconds(1700);
  servoRight.writeMicroseconds(1700);
  delay(angle);
}
void stayStill(){
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500);
}
void goStraight(){
  servoLeft.writeMicroseconds(1550);
  servoRight.writeMicroseconds(1450);
  
}
void detachServos(){
  servoLeft.detach();
  servoRight.detach(); 
}

/*MOVEMENT CONTROLS*/

/*WIFI SHIELD CONTROLS*/
void updateRSSI(){
  digitalWrite(yellowLed, HIGH);
  
  prevRssi = rssi;
  long rssiTemp[5];
  byte available_networks = WiFi.scanNetworks();
  for(int i=0; i<3; i++){
      WiFi.begin(ssid,pass);
      rssiTemp[i] = WiFi.RSSI();
      Serial.print(rssiTemp[i]);
      Serial.println();
      if(rssiTemp[i]<-100||rssiTemp[i]>-10){
        rssiTemp[i] = prevRssi;
      }
  }		
        
  rssi = (rssiTemp[0]+rssiTemp[1]+rssiTemp[2])/3;
      
  if(rssi-prevRssi<-15){
    rssi = prevRssi;
  }
  
  Serial.print("Old: ");
  Serial.print(prevRssi);
  Serial.print(" dBm ");
  Serial.print("New: ");
  Serial.print(rssi);
  Serial.print(" dBm");
  Serial.println();

  digitalWrite(yellowLed, LOW);
}

void scan(){
  int angles[8] = {0,210,420, 630,840, 1060, 1270, 1480};
  int signals[9]; //Keeps RSSI measurements for each angle in angles[]
  int maxSignal,index=0; 
  angle = 0;
  for(int i = 0; i<sizeof(angles)/sizeof(angles[0]);i++){
    turnLeft();
    angle=210;
    stayStill();
    delay(1500);
    updateRSSI();
    signals[i] = rssi;     
  }
  turnLeft();
  stayStill();
  delay(1500);
  
  maxSignal = signals[0];
  for(int i=0;i<sizeof(angles)/sizeof(angles[0]);i++)  
  {
    if (signals[i] > maxSignal) {
       index = i;
       maxSignal = signals[i];
    }
  }
  Serial.print("The largest signal: ");
  Serial.print(maxSignal);
  Serial.print(" angle: ");
  Serial.print(angles[index]);
  Serial.println();
  
 angle = angles[index]; 
 turnLeft();
 angle = 414;
}

void ssidStatus(){
   int status = WL_IDLE_STATUS; 

   status = WiFi.begin(ssid,pass);
   if ( status != WL_CONNECTED) { 
     ssidFound = false;
	 Serial.print("WiFi not found.");
	 Serial.println();
     setColor(255,0,0);
   } 
   else {
     ssidFound = true;
     setColor(0,0,255);
   }
}
/*WIFI SHIELD CONTROLS*/


/*RGB Led*/
void setColor(int red, int green, int blue)
 {
   analogWrite(rgbRed, red);
   analogWrite(rgbGreen, green);
   analogWrite(rgbBlue, blue);
 }
 /*RGB Led*/
 
