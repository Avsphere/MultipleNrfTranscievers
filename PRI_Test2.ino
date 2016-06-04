//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 10;       
 
//the time when the sensor outputs a low impulse
long unsigned int lowIn;        
 
//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 5000; 
 
boolean lockLow = true;
boolean takeLowTime; 
 
int pirPin = 8;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;
int state =0;
int prevState =0;
 
/////////////////////////////
//SETUP
void setup(){
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);
 
  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
  }
 
////////////////////////////
//LOOP
void loop(){
    state = digitalRead(pirPin);
    delay(20); //debouncing?!
     if(state != prevState){
       Serial.println("Motion motion motion!!");
       delay(500);
  }
  else if (state == prevState){
    Serial.println(" *Silence* ");
    delay(500);
  }
}
