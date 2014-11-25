#include <Wire.h>
#include <Adafruit_LSM303.h>
#include <Adafruit_NeoPixel.h>
 
 
#define toneC    1911
#define toneC1    1804
#define toneD    1703
#define toneEb    1607
#define toneE    1517
#define toneF    1432
#define toneF1    1352
#define toneG    1276
#define toneAb    1204
#define toneA    1136
#define toneBb    1073
#define toneB    1012
#define tonec       955
#define tonec1      902
#define toned       851
#define toneeb      803
#define tonee       758
#define tonef       716
#define tonef1      676
#define toneg       638
#define toneab      602
#define tonea       568
#define tonebb      536
#define toneb       506
 
#define tonep       0 
int speaker = 9; 
long vel = 20000;
boolean hasplayed = false;

int melod[] = {tonec, toneG, toneE, toneA, toneB, toneBb, toneA, toneG, tonee, toneg, tonea, tonef, toneg, tonee, tonec, toned, toneB};
int ritmo[] = {18, 18, 18, 12, 12, 6, 12, 8, 8, 8, 12, 6, 12, 12, 6, 6, 6};
int playix =0;
int led = 7;
int accel_x;
Adafruit_LSM303 lsm;
 
void tocar(int tom, long tempo_value) {
  long tempo_gasto = 0;
  while (tempo_gasto < tempo_value) {
    digitalWrite(speaker, HIGH);
    delayMicroseconds(tom / 2);
 
    digitalWrite(speaker, LOW);
    delayMicroseconds(tom/2);	 
    tempo_gasto += tom;
  }
}

void playit()
{
  if (hasplayed == true){ return;}
  for (int i=playix; i<playix+3; i++) {
    
    int tom = melod[i];
    int tempo = ritmo[i];
 
    long tvalue = tempo * vel;
   if (i >= 17) i = 0;
   
    tocar(tom, tvalue);
 
    delayMicroseconds(1000);
    playix = i;
  }
    hasplayed = false;
}
 
 
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, 6, NEO_GRB + NEO_KHZ800);
//Adafruit_LSM303 lsm;
 
// Here is where you can put in your favorite colors that will appear!
// just add new {nnn, nnn, nnn}, lines. They will be picked out randomly
//                                  R   G   B
uint8_t myFavoriteColors[][5] = {{200,   0, 200},   // purple
                                 {0,   117,   255},   // blue 
                                 {200,   10,   10},   // red 
                                 {20,   200,   10},   // green 
                                 {200, 200, 200},   // white
                               };
// don't edit the line below
#define FAVCOLORS sizeof(myFavoriteColors) / 5
 
// mess with this number to adjust TWINklitude :)
// lower number = more sensitive
#define MOVE_THRESHOLD 300
 
void setup() 
{
 
  pinMode(speaker, OUTPUT);
  Serial.begin(9600);
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
    while (1);
  }
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}
 
void loop() 
{
  // Take a reading of accellerometer data
  lsm.read();
  Serial.print("Accel X: "); Serial.print(lsm.accelData.x); Serial.print(" ");
  Serial.print("Y: "); Serial.print(lsm.accelData.y);       Serial.print(" ");
  Serial.print("Z: "); Serial.print(lsm.accelData.z);     Serial.print(" ");
 
  // Get the magnitude (length) of the 3 axis vector
  // http://en.wikipedia.org/wiki/Euclidean_vector#Length
  double storedVector = lsm.accelData.x*lsm.accelData.x;
  storedVector += lsm.accelData.y*lsm.accelData.y;
  storedVector += lsm.accelData.z*lsm.accelData.z;
  storedVector = sqrt(storedVector);
  Serial.print("Len: "); Serial.println(storedVector);
  
  // wait a bit
  delay(100);
  if(0)playit();
  
  // get new data!
  lsm.read();
  double newVector = lsm.accelData.x*lsm.accelData.x;
  newVector += lsm.accelData.y*lsm.accelData.y;
  newVector += lsm.accelData.z*lsm.accelData.z;
  newVector = sqrt(newVector);
  Serial.print("New Len: "); Serial.println(newVector);
  
  // are we moving 
  if (abs(newVector - storedVector) > MOVE_THRESHOLD) {
    Serial.println("Twinkle!");
    //playit();  
    flashRandom(5, 1);  // first number is 'wait' delay, shorter num == shorter twinkle
    flashRandom(5, 3);  // second number is how many neopixels to simultaneously light up
    flashRandom(5, 2);
    hasplayed=false;
  }
}
 
void flashRandom(int wait, uint8_t howmany) {
 
  for(uint16_t i=0; i<howmany; i++) {
    // pick a random favorite color!
    int c = random(FAVCOLORS);
    int red = myFavoriteColors[c][0];
    int green = myFavoriteColors[c][1];
    int blue = myFavoriteColors[c][2]; 
 
    // get a random pixel from the list
    int j = random(strip.numPixels());
    //Serial.print("Lighting up "); Serial.println(j); 
    
    // now we will 'fade' it in 5 steps
    for (int x=0; x < 5; x++) {
      int r = red * (x+1); r /= 5;
      int g = green * (x+1); g /= 5;
      int b = blue * (x+1); b /= 5;
      
      strip.setPixelColor(j, strip.Color(r, g, b));
      strip.show();
      delay(wait);
    }
    // & fade out in 5 steps
    for (int x=5; x >= 0; x--) {
      int r = red * x; r /= 5;
      int g = green * x; g /= 5;
      int b = blue * x; b /= 5;
      
      strip.setPixelColor(j, strip.Color(r, g, b));
      strip.show();
      delay(wait);
    }
  }
  // LEDs will be off when done (they are faded to 0)
}
