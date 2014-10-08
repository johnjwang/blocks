/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.
 
 modified 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
 */

#include <Servo.h> 

Servo myservo[4];  // create servo object to control a servo 
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position 

void setup() 
{ 
  myservo[0].attach(9);  // attaches the servo on pin 9 to the servo object 
  myservo[1].attach(10);  // attaches the servo on pin 9 to the servo object 
  myservo[2].attach(11);  // attaches the servo on pin 9 to the servo object 
  myservo[3].attach(12);  // attaches the servo on pin 9 to the servo object 
} 

void loop() 
{ 
  for(pos = 0; pos <= 180; pos += 1) // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    for(int i = 0; i<4; ++i)  
      myservo[i].write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = 180; pos>=0; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    for(int i = 0; i<4; ++i)    
      myservo[i].write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
} 


