//greenhouse temp control 1, temp_sense
//more or less copied from the example file included in the Adafruit_MAX31855.h library

//essential library
#include <Adafruit_MAX31855.h>

//define pins
#define MAXCLK  2
#define MAXCS   3
#define MAXDO   4

//pin 1 is used to send serial to other arduino, but is not defined,
//pin zero is another serial pin and is not used;

//define sensor
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

//neccesary variables
int t;

int tstart = 0;

int ta;
int tb;
int tc;
int td;
int te;
int tf;
int tg;
int th;
int ti;
int tj;

int tcount = 0;

byte temp = 0;
//"byte" doesn't change color like a variable, but it is a variable
//several variable types behave this way in the arduino ide for whatever reason

void setup() {
  Serial.begin(9600);

  while (!Serial) delay(1); // wait for Serial connection
  delay(500);  // wait for MAX chip to stabilize
  //set first loop var to 1
  tstart = 1;
}

void loop() {

  char t1[1] = "5";

   double f = thermocouple.readFahrenheit();
   //this checks if the reading from the sensor is good
   if (isnan(f)) {
    //do nothing
   } else {
    //if the reading is good:
    //cast double "f" to int "t" to round off any numbers less than 1,
    //because I don't need them or have room to display or easily transmit them
       t = f;
   }
   //display reading if first loop
   if (tstart == 1){
      temp = (byte)t;
      Serial.write(temp);
      tstart = 0;
   }

  //then get an average over 10 loops
   switch (tcount){
    case 0:
      ta = t;
      tcount = 1;
      break;
    case 1:
      tb = t;
      tcount = 2;
      break;
    case 2:
      tc = t;
      tcount = 3;
      break;
    case 3:
      td = t;
      tcount = 4;
      break;
    case 4:
      te = t;
      tcount = 5;
      break;
    case 5:
      tf = t;
      tcount = 6;
      break;
    case 6:
      tg = t;
      tcount = 7;
      break;
    case 7:
      th = t;
      tcount = 8;
      break;
    case 8:
      ti = t;
      tcount = 9;
      break;
    case 9:
      tj = t;
      int tk = ta + tb + tc + td + te + tf + tg + th + ti + tj; 
      int ttot = tk/10;
      temp = (byte)ttot;
      Serial.write(temp);
      tcount = 0;
   }


  //a delay is neccesary for temperature sensors to work.
  //this is the primary reason for using two arduinos here,
  //as a delay may make control buttons unresponsive 
  delay(1000);

  //if controls are not being used and only pre-set, non-adjustable temperatures are acceptible,
  //then one arduino could probably be used instead of two
  //if you set analog pins to digitalread/write, maybe
  //otherwise there wouldn't be enough pins I think
  //though in that case you could also use another model of arduino

}
