//greenhouse temp control 1, display-button-relay

//essential external libraries (maybe):
extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

//essential libraries:
#include <TM1637Display.h>

//define relay control pins:
#define RCt 3
#define RHt 5
#define RSo 6
#define RSc 9

// define pins for displays:
#define CLK 2
#define DIO 4
#define CLK1 7
#define DIO1 8

//pin 0 is reserved for recieving serial data from another arduino, but doesn't require definition
//pin 1 is, like pin 0, a serial pin, and so is not used
//pins A0-A3 are used for one switch input and three button inputs, but are not defined here

//define displays:
TM1637Display display1(CLK, DIO);
TM1637Display display2(CLK1, DIO1);

//whether displays should update:
int du1 = 0;
int du2 = 0;

//temp variables:

//temp read in variable:
int temp = 0;
//adjusted temp variable for display and relay control reference:
int adjustedTemp = 0;
//variable for seeing if temp has changed
int tempAdVar = 0;
//variable indicating how much the temp has been adjusted:
int tempAdjust = 0;
//temp adjust variable for display update:
int ta = 0;
//variable to indicate whether temp has been adjusted negatively, for display update:
int negative = 0;

//temp variables for display2 and relay controls are pre-set in case of power loss,
//but can be fine-tuned via button controls

//Ctmp, temperature at which cooling fan(s) should turn on:
int Ctmp = 80;
//Htmp, temperature at which heater(s) should turn on:
int Htmp = 40;
//So, temperature at which the greenhouse shutter(s) should open:
int So = 70;
//Sc, tempurature at which the greenhouse shutter(s) should close:
int Sc = 50;

//pre-set temps can be whatever you want them to be.

//There's no storage for settings, so if you ever need to reset to defaults, just unplug the contoller,
//and plug it back in again.


//variable determining whether a button press should register:
int click = 1;

//digit container variables for displays:

//display1:
int a = 0;
int b = 0;
int c = 0;

//display2:
int w = 0;
int x = 0;

int y = 0;
int z = 0;

//display and settings mode, cycles from 1 to 5:
//1 by default
int mode = 1;

//set display 2 on or off:
//off by default
int d2on = 0;
//variable for display 2 switch voltage:
int d2v = 0;

//relay trigger variables:
//whether relays are currently on
int CtRo = 0;
int HtRo = 0;
int SoRo = 0;
int ScRo = 0;
//whether relays should update:
int rUp = 0;

//button control pin read variables:
int SV0 = 0;
int SV1 = 0;
int SV2 = 0;
int SV3 = 0;

//variable for whether controls are switched on:
float ctOn = 0;
//mode-swap button variable:
float swap = 0;
//increment button variable:
float increment = 0;
//decrement button variable:
float decrement = 0;

//empty variables for display data:
uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
uint8_t data1[] = { 0x00, 0x00, 0x00, 0x00 };

//a function to prevent incrementing higher than 99 or decrementing lower than 32
//updated function to prevent accidental conflicts
//updated again because I did a half-assed job the first time
int limit(int which, int in, int alt){

  if (in >= 99){
    in = 99;
  }
  if (in <= 32){
    in = 32;
  }

  switch(which){
    case 1:
    //keep value 1 below value 2
      if (in >= alt) {
        in = alt - 1;
      }
      return in;
      break;
    case 2:
    //keep value 1 above value 2
      if (in <= alt) {
        in = alt + 1;
      }
      return in;
      break;
    case 3:
      return in;
      break;
  }
}

//limit temp adjust to +- 50
int tLimit(int in){
  if (in >= 50){
        in = 50;
  }
  if (in <= -50){
    in = -50;
  }
  return in;
}


void setup() {
  Serial.begin(9600); //temporary for troubleshooting (maybe)
  delay(1000); //time for everthing to power on
  //display brightness range is 0-7
  display1.setBrightness(2);
  display2.setBrightness(2);

  //update display on start, to ensure displays activate on start or after power off event
  du1 = 1;
  du2 = 1;
};


 
void loop() {
  //diagnostics
  //-------------------------------------------
  //Serial.println(temp);

  /*
  if (ctOn >= 1.5){
    Serial.println(ctOn);
  }
  */
  /*
    if (swap >= 1.5){
    Serial.println(swap);
  }
    if (increment >= 1.5){
    Serial.println(increment);
  }
    if (decrement >= 1.5){
    Serial.println(decrement);
  */

  //--------------------------------------

  //update relay triggers if neccesary
  //gets rUp value from previous loop
  if (rUp == 1) {
    //check if relay should be on
    if (adjustedTemp > Ctmp) {
      //turn off opposing relay if on
      if (HtRo != 0){
        analogWrite(RHt, 0);
        HtRo = 0;
      }
      //check if relay is not on
      if (CtRo != 1) {
        //turn relay on
        analogWrite(RCt, 255);
        //set variable to indicate that relay is on
        CtRo = 1;
      }
    }
    //if relay should be off, check if relay is not off
    //2 degree hysteresis for cooling/heating
    else if (adjustedTemp < Ctmp - 2) {
      if (CtRo != 0) {
        //turn relay off
        analogWrite(RCt, 0);
        //set variable to indicate that relay is off
        CtRo = 0;
      }
    }
    //same for the rest of these

    if (adjustedTemp < Htmp) {
      if (CtRo != 0) {
        analogWrite(RCt, 0);
        CtRo = 0;
      }
      if (HtRo != 1) {
        analogWrite(RHt, 255);
        HtRo = 1;
      }
    }
    else if (adjustedTemp > Htmp + 2) {
      if (HtRo != 0) {
        analogWrite(RHt, 0);
        HtRo = 0;
      }
    }

    if (adjustedTemp > So) {
      if (ScRo != 0) {
        analogWrite(RSc, 0);
        ScRo = 0;
      }
      if (SoRo != 1) {
        analogWrite(RSo, 255);
        SoRo = 1;
      }
    }
    //actually shutters don't need to turn off
    /*
    //10 degree hysterisis for shutters
    else if (adjustedTemp < So - 10) {
      if (SoRo != 0) {
        analogWrite(RSo, 0);
        SoRo = 0;
      }
    }
    */

    if (adjustedTemp < Sc) {
      if (SoRo != 0) {
        analogWrite(RSo, 0);
        SoRo = 0;
      }
      if (ScRo != 1) {
        analogWrite(RSc, 255);
          ScRo = 1;
      }
    }
    /*
    else if (adjustedTemp > Sc + 10) {
      if (ScRo != 0) {
        analogWrite(RSc, 0);
        ScRo = 0;
      }
    }
    */
    //relays are updated, reset relay update var to 0
    rUp = 0;
  }

  //get temp from serial connection (another arduino reading temperature)
  if (Serial.available() > 0) {
    int t = Serial.read();
    //check if temperature has changed since last loop
    if (temp != t){
      //if new temp, update temp container var, update display and relays
	  //display and relay update moved to adjustedTemp checking
      temp = t;
    }
  }

  //update temp
  adjustedTemp = temp + tempAdjust;
  //check if (adjusted) temp has changed this loop
  if (tempAdVar != adjustedTemp){
    tempAdVar = adjustedTemp;
    //update display, relays
    du1 = 1;
    rUp = 1;
  }

  //display1 controls: -------------------------------------------------------------------------------

  //update method for display1
  if (du1 == 1){
    //using a new var so as not to alter the temp storage var here
    int t = adjustedTemp;
    if (t >= 0){
      //break down integer into individual digits to send to display

      //getting the "first" (last) digit from the integer is easy, using "% 10"
      c = t % 10;

      //convert int to string for accuracy, as bitshift math, or even initialization, can't handle leading zeros.
      //this is apparently not a bug but due to some obscure, archaic convention called "octals".
      //I don't think my code should generate any leading zeros, but I'm leaving it this way just in case.
      String tocull = String(t);
      //And then convert again into char
      char* cull = tocull.c_str();

      //remove last character from string
      cull[strlen(cull) - 1] = '\0';

      //use atoi to convert string into int,
      //use % 10 to get last digit of new int
      b = atoi(cull) % 10;

      //and again
      cull[strlen(cull) - 1] = '\0';
      a = atoi(cull);

      //only maximum 3-digit integers are expected here
    }
  }

  //send display data to display1
  if (du1 == 1){
    //if it's below 100 degrees, don't display a zero
    if (a == 0){
      data[0] = 0x00;
    }
    else{
      data[0] = display1.encodeDigit(a);
    }
    //I don't do this for 10 because I don't expect it to get below 10 degrees
    data[1] = display1.encodeDigit(b);
    data[2] = display1.encodeDigit(c);
    //display an F
    data[3] = 0b01110001;
      display1.setSegments(data);
      //display1 is updated, reset display1 update variable to 0
      du1 = 0;
  }


  //display2 controls: -------------------------------------------------------------------------------

  //monitor switch to turn on temp settings (display2)
  SV0 = analogRead(A0);
  ctOn = SV0 * (5.0 / 1023.0);
  //check if switch voltage has changed since last loop
  if(d2v != ctOn){
    //if so, update voltage variable
    d2v = ctOn;
    //set display 2 on or off depending on voltage
    if (ctOn > 3){
    d2on = 1;
    }
    else if (ctOn < 1.5){
    d2on = 0;
    mode = 1;
    }
    //update display
    du2 = 1;
  }


  //monitor button controls for temp settings (display2)
  //only runs if temp settings (display2) is switched on
  if (d2on == 1){

    //button one, cycle through modes
    SV1 = analogRead(A1);
    swap = SV1 * (5.0 / 1023.0);

    if ((swap > 3) && (click == 1) ){
      mode++;
      if (mode >= 6){
        mode = 1;
      }
      //disable buttons
      click = 0;
      ///update display
      du2 = 1;
    }

    
    //local relay update variable
    int rUpL = 0;

    //button 2, increment temperature of mode given
    SV2 = analogRead(A2);
    increment = SV2 * (5.0 / 1023.0);

    if ((increment > 3) && (click == 1) ){
      switch(mode){
        case 1:
          Ctmp++;
          Ctmp = limit(3, Ctmp, 0);
          rUpL= 1;
          break;
        case 2:
          Htmp++;
          Htmp = limit(1, Htmp, Sc);
          rUpL= 1;
          break;
        case 3:
          So++;
          So = limit(1, So, Ctmp);
          rUpL= 1;
          break;
        case 4:
          Sc++;
          Sc = limit(1, Sc, So);
          rUpL= 1;
          break;
        case 5:
          tempAdjust++;
          tempAdjust = tLimit(tempAdjust);
          break;
      }
      //disable buttons
      click = 0;
      //update display
      du2 = 1;
    }

    //button 3, decrement temperature of mode given
    SV3 = analogRead(A3);
    decrement = SV3 * (5.0 / 1023.0);

    if ((decrement > 3) && (click == 1) ){
      switch(mode){
        case 1:
          Ctmp--;
          Ctmp = limit(2, Ctmp, So);
          rUpL= 1;
          break;
        case 2:
          Htmp--;
          Htmp = limit(3, Htmp, 0);
          rUpL= 1;
          break;
        case 3:
          So--;
          So = limit(2, So, Sc);
          rUpL= 1;
          break;
        case 4:
          Sc--;
          Sc = limit(2, Sc, Htmp);
          rUpL= 1;
          break;
        case 5:
          tempAdjust--;
          tempAdjust = tLimit(tempAdjust);
          //update display 1
          du1 = 1;
          break;
      }
      //disable buttons
      click = 0;
      //update display
      du2 = 1;
    }

    //update relays if control temp settings changed
    if (rUpL != 0){
      rUp = 1;
      rUpL = 0;
    }
    //(tempAdjust is calculated each loop with temp so relays will update then for tempadjust)

    //require button release before allowing another press, for accuracy
    if ((swap < 1.5) && (increment < 1.5) && (decrement < 1.5)){
      //enable buttons
      click = 1;
    }
  }

  //update method for display2
  if (du2 == 1){
    int t;
    switch(mode){
      case 1:
        t = Ctmp;
        break;
      case 2:
        t = Htmp;
        break;
      case 3:
        t = So;
        break;
      case 4:
        t = Sc;
        break;
      case 5:
        t = tempAdjust;
        ta = 1;
        break;
    }
    //check if t is negative
    if (ta != 0){
      if (t < 0){
        negative = 1;
        int unsin = t * -1;
        t = unsin;
      }
    } 

    //same method as dislpay1
    z = t % 10;

    String tocull = String(t);
    char* cull = tocull.c_str();

    cull[strlen(cull) - 1] = '\0';
    y = atoi(cull);

    //only maximum 2-digit integers are expected here
  }

  //send display data to display2
  //check if display should be on
  if (d2on == 1){
    //check if display2 needs an update
    if (du2 == 1){
      //check display mode
      switch(mode){
        case 1:
          //print C° for cooling temp
          w = 0b00111001;
          x = 0b11100011;
          break;
        case 2:
          //print H° for heating temp
          w = 0b01110110;
          x = 0b11100011;
          break;
        case 3:
        //print So for shutter open temp
          w = 0b01101101;
          x = 0b11011100;
          break;
        case 4:
          //print Sc for shutter close temp 
          w = 0b01101101;
          x = 0b11011000;
          break;
        case 5:
          w = 0b01110111;
          if (negative == 1) {
            x = 0b01000000;
          }
          else {
            x = 0b00000000;
          }
          ta = 0;
          negative = 0;
      }
      //print mode
      data1[0] = w;
      data1[1] = x;
      //print temp setting
      if (y == 0){
      data1[2] = 0b00000000;
      }
      else{
      data1[2] = display2.encodeDigit(y);
      }
      data1[3] = display2.encodeDigit(z);
    }
  }
  else{
    //turn display off if it should be off
    data1[0] = 0b00000000;
    data1[1] = 0b00000000;
    data1[2] = 0b00000000;
    data1[3] = 0b00000000;
  }
  //update display if it needs an update
  if (du2 == 1){
    display2.setSegments(data1);
    //display2 is updated, reset update variable to 0
    du2 = 0;
  }
};