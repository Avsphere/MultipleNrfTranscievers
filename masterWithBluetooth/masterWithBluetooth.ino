/*
This is the general code for the master. 
*/

#include <SPI.h>
#include "RF24.h"
//All of the nodes that we are using.
byte addresses[][6] = {"1Node","2Node","3Node","4Node", "5Node"};

//MASTER is 0
int radioNumber = 0;
int state = 0;
int flag = 0;

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

  Serial.begin(57600);
  
  radio.begin();
  //Range Amplifiers
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108); //Above most wifi channels
  radio.setPALevel(RF24_PA_HIGH);
  
   //Starts on a random device 1, 2, or 3.
   radio.openWritingPipe(addresses[random(1,4)]);
   radio.openReadingPipe(1,addresses[0]);
  
  myData.value = 0;
  // Start the radio listening for data

}




void loop() {
    if(Serial.available() > 0){
      state = Serial.read();
      flag=0;
    }
    // if the state is 0 the radio will turn off
    if (state == '0') {
        radio.stopListening();
        if(flag == 0){
          Serial.println("Radio: off");
          flag = 1;
        }
    }
    // if the state is 1 the radio will turn on
    else if (state == '1') {
        radio.startListening();
        if(flag == 0){
          Serial.println("Radio: on");
          flag = 1;
        }
    }
    
/****************** Transmission Role ***************************/  
if(state == "1"){
  if (role == 1)  {
      
      radio.stopListening();                                    // First, stop listening so we can talk.  
      myData._micros = micros();
       if (!radio.write( &myData, sizeof(myData) )){
         Serial.println(F("failed")); //auto ack is on
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
            radio.openWritingPipe(addresses[1]);
          }
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
  }


} // Loop
