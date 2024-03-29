/*
This is the general code for the master. Make sure its radio number is 0.
Currently is just randomly sends our datastructure to a random other radio.
*/

#include <SPI.h>
#include "RF24.h"

byte addresses[][6] = {"1Node","2Node","3Node","4Node", "5Node"};
//This is the master so leave at 0
int radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/


// Used to control whether this node is sending or receiving
bool role = 0;

//What we are sending
struct dataStruct{
  unsigned long _micros;
  float value;
  int myRadio = 0;
}myData;

void setup() {

  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted_HandlingData"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber == 0){
  
    radio.openWritingPipe(addresses[random(1,4)]);
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
    Serial.println("I am radio number 2");
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[3]);
  }
  else if (radioNumber == 4){
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
      if (micros() - started_waiting_at > 2000000 ){          // If waited longer than 200ms, indicate timeout and exit while loop
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
        Serial.print("This is from radio : ");
        Serial.println(myData.myRadio);
        
        //Opens a writing pipe to a random peasant radio.
        if(myData.myRadio == 1){
           radio.openWritingPipe(addresses[2]);
        }
        if(myData.myRadio == 2){
          radio.openWritingPipe(addresses[3]);
        }
        if(myData.myRadio == 3){
          radio.openWritingPipe(addresses[4]);
        }
        if(myData.myRadio == 4){
          radio.openWritingPipe(addresses[1]);
        }
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
     
      radio.stopListening();                               // First, stop listening so we can talk  
      myData.value += 0.01;                                // Increment the float value
      radio.write( &myData, sizeof(myData) );              // Send the final one back.      
      radio.startListening();                              // Now, resume listening so we catch the next packets.     
      Serial.print(F("Sent response "));
      Serial.print(myData._micros);  
      Serial.print(F(" : "));
      Serial.println(myData.value);
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
