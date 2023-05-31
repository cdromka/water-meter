#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//DueFlashStorage dueFlashStorage;

int X;
int Y;
int B;
float TIME = 0;
float FREQUENCY = 0;
float WATER = 0;
float TOTAL = 0;
float LS = 0;
float V = 140; // tank volume in liters
const int input = D5;
const int inbutton = D7;
bool dirtyFlag = false;
int dirtyCounter = 0;
int dirtyCounterMax = 20; // cycles

struct Configuration {
  float total;
};

Configuration configuration;

void setup()
{
Serial.begin(115200);
EEPROM.begin(512);
pinMode(LED_BUILTIN, OUTPUT);

Serial.println("Hello!");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  // display.clearDisplay();

  // // Draw a single pixel in white
  // display.drawPixel(10, 10, SSD1306_WHITE);

  // // Show the display buffer on the screen. You MUST call display() after
  // // drawing commands to make them visible on screen!
  // display.display();
  pinMode(input,INPUT);
  pinMode(inbutton,INPUT_PULLUP);
  loadconfiguration();
  digitalWrite(LED_BUILTIN, HIGH);   
}

void loop()
{ 
  X = pulseIn(input, HIGH);
  Y = pulseIn(input, LOW);

  B = digitalRead(inbutton);
  if (B == LOW)
    resetconfiguration();
    
  TIME = X + Y;
  FREQUENCY = 1000000/TIME;
//   WATER = FREQUENCY/7.5;
// 21 - by docs 18 by calculations....
  WATER = FREQUENCY/18.25;  
  LS = WATER/60;
  if(FREQUENCY >= 0)
  {
    if(isinf(FREQUENCY))
    {
      printcounters(0.00, TOTAL);
    }
    else
    {
      TOTAL = TOTAL + LS;
      printcounters(WATER, TOTAL);
      if (LS > 0)
      {
        dirtyFlag = true;
        dirtyCounter = 0;
        Serial.println("Configuration is dirty. Restarting countdown before saving.");    
        digitalWrite(LED_BUILTIN, LOW);              
      }        
    }
  }

  if (dirtyFlag)
  {
    dirtyCounter++;
    Serial.print("Configuration counter: ");          
    Serial.print(dirtyCounter);              
    Serial.print(" out of ");
    Serial.println(dirtyCounterMax);                                
  }
  if (dirtyCounter > dirtyCounterMax)
  {
    saveconfiguration(TOTAL);
    dirtyFlag = false;
    dirtyCounter = 0;
    digitalWrite(LED_BUILTIN, HIGH);     
  }
    
  delay(1000);    
}

void printcounters(float v, float t)
{
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
//  display.println(F("Hello, world!"));

  display.print("+ ");
  display.print(V - t);
  display.println(" L");  

 if (v > 0.00)
 {
  display.print("@ ");
  display.print(v);
  display.println("L/M");
 } else
 {
  display.print("- ");
  display.print(t);
  if (dirtyFlag)
    display.println("*L");    
  else
    display.println(" L");  
 }

  display.display();

// debug
  Serial.println(FREQUENCY);
  Serial.print("d:");
  Serial.print(WATER);
  Serial.println("L/M");
  Serial.print("V:");
  Serial.print( TOTAL);
  Serial.println("L");
}

void resetconfiguration()
{
  saveconfiguration(0.0);    
  TOTAL = 0;
  printcounters(WATER, TOTAL);  
}

void loadconfiguration()
{
  EEPROM.get(0, TOTAL);
  Serial.println("Configuration Loaded.");  
}

void saveconfiguration(float t) 
{
  EEPROM.put(0, t);
  EEPROM.commit();
  Serial.println("Configuration Saved.");    
}
