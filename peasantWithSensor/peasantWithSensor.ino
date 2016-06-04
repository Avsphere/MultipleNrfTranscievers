/*
General Peasant code. Recieves a signal from the master then sends something back.
*/

#include <SPI.h>
#include "RF24.h"

byte addresses[][6] = {"1Node","2Node","3Node","4Node"};
/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
int radioNumber = 3;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/


// Used to control whether this node is sending or receiving
bool role = 0;
bool active = false;

struct dataStruct{
  unsigned long _micros;
  float value;
  int myRadio = 3;
}myData;

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 10;       
 
//the time when the sensor outputs a low impulse
long unsigned int lowIn;        
 
//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 5000; 
 
boolean lockLow = true;
boolean takeLowTime; 
 
int pirPin = 5;    //the digital pin connected to the PIR sensor's output
int ledPin = 4;
int state =0;

void setup() {

  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted_HandlingData"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

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
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber == 0){
  
    radio.openWritingPipe(addresses[random(1,5)]);
    radio.openReadingPipe(1,addresses[0]);

  }
  else if(radioNumber == 1){
   Serial.print("I am radio 1");
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  else if (radioNumber == 2){
    Serial.println("I am radio number 2");
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[2]);
    
  }
  else if (radioNumber == 3){
    Serial.println("I am radio number 3");
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[3]);
  } else if (radioNumber == 4){
     Serial.println("I am radio number 4");
     radio.openWritingPipe(addresses[0]);
     radio.openReadingPipe(1,addresses[4]); 
  }
  
  myData.value = 1.22;
  // Start the radio listening for data
  radio.startListening();
}




void loop() {
  
  
/****************** Ping Out Role ***************************/  
if (role == 1)  {
    
    radio.stopListening();                                    // First, stop listening so we can talk.
    
    
    Serial.println(F("Now sending"));

    myData._micros = micros();
     if (!radio.write( &myData, sizeof(myData) )){
       Serial.println(F("failed"));
     }
        
    radio.startListening();                                    // Now, continue listening
    
    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
    }else{
                                                                // Grab the response, compare, and send to debugging spew
        radio.read( &myData, sizeof(myData) );
        unsigned long time = micros();
        
        // Spew it
        Serial.print(F("Sent "));
        Serial.print(time);
        Serial.print(F(", Got response "));
        Serial.print(myData._micros);
        Serial.print(F(", Round-trip delay "));
        Serial.print(time-myData._micros);
        Serial.print(F(" microseconds Value "));
        Serial.println(myData.value);
    }

    // Try again 1s later
    delay(1000);
  }



/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    
    if( radio.available()){
                                                           // Variable for the received timestamp
      while (radio.available()) {                          // While there is data ready
        radio.read( &myData, sizeof(myData) );             // Get the payload
      }
      
      myData.myRadio = radioNumber;
      radio.stopListening();                               // First, stop listening so we can talk  
      myData.value += 0.01;                                // Increment the float value
      active = true;

      digitalWrite(ledPin, HIGH);                          //Turn on LED                         
      
      while(active){
        state = digitalRead(pirPin);
        delay(20);                                            //debouncing?!
        if(state == 1){
          Serial.println("Motion motion motion!!");
          radio.write( &myData, sizeof(myData) );              // Send the final one back.      
          radio.startListening();                              // Now, resume listening so we catch the next packets.     
          Serial.print(F("Sent response "));
          Serial.print(myData._micros);  
          Serial.print(F(" : "));
          Serial.println(myData.value);
          active = false;
      }
   }

      digitalWrite(ledPin, LOW);
    }
 }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.print(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)
    
   }
  else if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
    }
   
  }


} // Loop
