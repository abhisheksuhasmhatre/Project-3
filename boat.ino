//This #include statement was automatically added by the Particle IDE.
#include "Adafruit_SSD1306/Adafruit_SSD1306.h"

//This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

//use hardware SPI
#define OLED_DC     D3
#define OLED_CS     D4
#define OLED_RESET  D5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

//IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN   D2
#define PIXEL_COUNT 64
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

Servo myservo;//create servo object to control a servo
int pos = 0;//variable to store the servo position
int sensorLeft = A1; //input from analog front left proximity sensor.
int sensorRight = A0;//input from analog front right proximity sensor.
int back = A4;//input from analog back proximity sensor.
double sensorOutOne = D1;//output from front left sensor to the photon.
double sensorOutTwo = D0;//output from front right sensor to the photon.
double backout = D6;//output from back sensor to the photon.
int dataValOne = 0;//variable to store sensorOutOne.
int dataValTwo = 0;//variable to store sensorOutTwo.
int backthree = 0;//variable to store backthree.
int  x, minX;//variables for scrolling code.
int status;
int pixel;
int pixelTwo;
int people=0;
char Sensor_Output[128];
int i = 0;

void setup()
{
    Serial.begin(9600);//begin commmand.
    pinMode (sensorLeft, INPUT);//set pin mode to input mode for left sensor.
    pinMode (sensorRight, INPUT);//set pin mode to input mode for right sensor.
    pinMode (back, INPUT);
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    display.begin(SSD1306_SWITCHCAPVCC);
    display.setTextSize(1.6);///text size.
    display.setTextColor(WHITE);//text color.
    display.setTextWrap(false);//turn off text wrapping so we can do scrolling.
    status=0;
    myservo.attach(D0);//attach the servo on the D0 pin to the servo object.
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    Particle.subscribe("Sensor_Output", myHandler);
}

void myHandler(const char *event, const char *data)
{

    int convertToInt = atoi(data);

    i++;

    //this code displays the count of people on the OLED screen with fancy scrolling.
    display.clearDisplay();
    display.setCursor(x/2, 7);
    display.print("Left Right\n");
    display.print(dataValOne);//output value displayed from left sensor.
    display.print("--");
    display.print(dataValTwo);//output value displayed from right sensor.
    display.print("--");
    display.print(backthree);//output value displayed from back sensor.
    display.print("\nPeople In and Out\n");
    display.print(people);//output from counter.
    display.display();
    display.clearDisplay();
    Serial.print(", data: ");
    display_proximity(pixel);//display output from left sensor to neopixel.
    display_proximityTwo(pixelTwo);//display output from Right sensor to neopixel.
    strip.setBrightness(10);//sets brightness to 10.
    strip.show();

    //just to slow down the output - remove if trying to catch an object passing by.
    delay(900);
    strip.clear();
    if (data)
    {
        Serial.println(data);
    }
    else
    {
        Serial.println("NULL");
    }
}

void loop()
{
    dataValOne = analogRead (sensorOutOne);//output analog pin read for front left sensor.
    dataValTwo = analogRead (sensorOutTwo);//output analog pin read for front right sensor.
    backthree = analogRead (backout);//output analog pin read for back sensor.

    //as per state states get the output on the OLED or on web. if Left(1) and right(2) are set then counter increment by one (someone is in).
    //if right(1) and left(2) are set then counter decrement by one (someone got out).
    //if only left or right is set then after a 900 delay the state resets to 0.
    if (getStatus() == 0)
    {
        status = 0;
    }
    else if (getStatus() == 1)
    {
        delay (1000);
        status = 0;
    }
    else if (getStatus() == 2)
    {
        people++;
        status = 0;
    }
    else if (getStatus() == 3)
    {
        delay (1000);
        status = 0;
    }
    else if (getStatus() == 4)
    {
        people--;
        status = 0;
    }

    pixel = dataValOne * 64 / 4000;//get a proportional output from dataValOne w.r.t. 64 neopixels.
    pixelTwo = dataValTwo * 64 / 4000;//get a proportional output from dataValTwo w.r.t. 64 neopixels.
    Serial.println("Left Sensor Right Sensor People Status\n");
    Serial.println(dataValOne);//print output values for left sensor.
    Serial.println("\t");
    Serial.println(dataValTwo);//output analog pin read for right sensor.
    Serial.println("\t");
    Serial.println(people);
    Serial.println("\t");
    Serial.println(getStatus());

    sprintf(Sensor_Output, "%d    %d    %d    %d", (int)dataValOne, (int)dataValTwo, (int)people, (int)status);
    bool success = Particle.publish("Sensor_Output", Sensor_Output);
    if(!success)
        Serial.println("Failed to publish Sensor Output");

    //logic to run the motor in clockwise or anticlockwise direction as per sensor inputs.
    if (backthree <= 2000 && dataValOne >= 2000 && dataValTwo >= 2000)
    {
        myservo.write(180);//rotate clockwise full speed to go back.
    }

     else if (backthree >= 2000 && dataValOne <= 2000 && dataValTwo <= 2000)
    {
        myservo.write(90);//rotate clockwise full speed to go forward.
    }

    else if (backthree <= 2000 && dataValOne <= 2000 && dataValTwo <= 2000)
    {
        myservo.write(0);//rotate anticlockwise full speed to go forward.
    }
}

//to get status of each sensors and their combinations and use that to the counter.
int getStatus()
{
    if(dataValOne <= 3000 && dataValTwo <= 3000)
    {
        status = 0;
    }
    else if(dataValOne >= 3000 && dataValTwo <= 3000)
    {
        status = 1;
    }
    else if(dataValOne >= 3000 && dataValTwo >= 3000)
    {
        status = 2;
    }
    else if(dataValOne <= 3000 && dataValTwo >= 3000)
    {
        status = 3;
    }
    else if(dataValTwo >= 3000 && dataValOne >= 3000)
    {
        status = 4;
    }
    return status;
}
//loop to light up each LED pixel in the neopixel.
void display_proximity(int pixel)
{
  for(int i=0;i<pixel;i++)
  {
      if (dataValOne > 0 && dataValOne <= 1333)
      {
          strip.setPixelColor(i, strip.Color(0, 255, 0));//set color to blue color LED.
      }
      else if (dataValOne > 1333 && dataValOne <= 2666)
      {
          strip.setPixelColor(i, strip.Color(255, 0, 0));//set color to blue color LED.
      }
      else if (dataValOne > 2666 && dataValOne <= 4000)
      {
          strip.setPixelColor(i, strip.Color(0, 0, 255));//set color to blue color LED.
      }
  }
  strip.show();
  //just to slow down the output - remove if trying to catch an object passing by.
  delay(900);
}

//loop to light up each LED pixel in the neopixel.
void display_proximityTwo(int pixelTwo)
{
  for(int i=0;i<pixelTwo;i++)
  {
      if (dataValTwo > 0 && dataValTwo <= 1333)
      {
          strip.setPixelColor(i, strip.Color(0, 0, 255));//set color to blue color LED.
      }
      else if (dataValTwo > 1333 && dataValTwo <= 2666)
      {
          strip.setPixelColor(i, strip.Color(0, 255, 0));//set color to blue color LED.
      }
      else if (dataValTwo > 2666 && dataValTwo <= 4000)
      {
          strip.setPixelColor(i, strip.Color(255, 0, 0));//set color to blue color LED.
      }
  }
  strip.show();
  //just to slow down the output - remove if trying to catch an object passing by.
  delay(900);
}

void clear()
{
     for(int i=0;i<(strip.numPixels());i++)
  {
        strip.setPixelColor(i, strip.Color(0, 0, 0));//set color to blue color LED.
  }
}
