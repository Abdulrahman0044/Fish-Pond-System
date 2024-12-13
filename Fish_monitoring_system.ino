#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
//#include <DHT.h>
//#include <AM2302-Sensor.h>
#include <Ds1302.h>
#include <Bonezegei_DHT11.h>
#include <SD.h>
#include <SPI.h>
// #include <MQUnifiedsensor.h>
// #include <MQ2.h>
#include <TimeLib.h>

//#define DHTPIN 9           // DHT22 data pin connected to Arduino digital pin 9
//#define DHTTYPE DHT21     // DHT22 (AM2302) sensor type
#define BUZZER_PIN 7       // Buzzer connected to digital pin 7
#define RELAY_PIN 6        // Relay connected to digital pin 6
#define MQ2_PIN A0         // MQ-2 sensor analog pin connected to Arduino analog pin A0
#define SD_CS_PIN 4       // SD card CS pin connected to Arduino digital pin 10
#define SMOKE_SENSORPIN A1    // Air velocity sensor analog pin connected to Arduino analog pin A1
#define MAXDELAY  3000
#define MINDELAY  1000

hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
hd44780_I2Cexp lcd2(0x27);
Bonezegei_DHT11 dht(9);

const int LCD_COLS = 16;
const int LCD_ROWS = 2;
int clkPin = 2;
int dataPin = A4;
int rstPin = A5;
//DHT dht(DHTPIN, DHTTYPE);

// Create a Ds1307 object
Ds1302 rtc(rstPin, clkPin, dataPin);
File dataFile;
// MQ2 mq2(MQ2_PIN);
// unsigned long previousMillis = 0;
// const long interval = 1000 * 60 * 5; // Log data every 5 minutes
// unsigned long currentMillis = millis();

// float gasValue = 0.0;
float tempDeg = 0.0;       
int hum = 0;
int sensorValue = 0;
const int thresholdValue = 300;
volatile int windCount = 0;
float tempThreshold = 40.0;

#if !defined(SDA) || !defined(SCL)
#if defined(_DTWI0_SDA_PIN) && defined(_DTWI0_SCL_PIN)
#define SDA _DTWI0_SDA_PIN
#define SCL _DTWI0_SCL_PIN
#endif
#endif

const static char* WeekDays[] =
{
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

void setup() {

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(SMOKE_SENSORPIN, INPUT);

  digitalWrite(RELAY_PIN, LOW); // Initially, relay is off
  digitalWrite(BUZZER_PIN, HIGH); // Buzz the buzzer
    delay(MINDELAY);
  digitalWrite(BUZZER_PIN, LOW);
  int status; // Declare the status for the LCD
  Serial.begin(9600);
  lcd.begin(LCD_COLS, LCD_ROWS);
  status = lcd.begin(LCD_COLS, LCD_ROWS);
if(status) // non zero status means it was unsuccesful
{
status = -status; // convert negative status value to positive number

// begin() failed so blink error code using the onboard LED if possible
hd44780::fatalError(status); // does not return
}
// initalization was successful, the backlight should be on now

  lcd.setCursor(0,0);
  lcd.print("Dryer Monitor");
  delay(MAXDELAY);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A Project by:");
  delay(MINDELAY);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Rilwan Aliyu");
  delay(MINDELAY);
  lcd.setCursor(0,1);
  lcd.print("SPS/20/MPY/00007");
  delay(MAXDELAY);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Supervised by: ");
  delay(MINDELAY);
  lcd.setCursor(0,1);
  lcd.print("Prof. Galadanci");
  delay(MAXDELAY);
  lcd.clear();

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  } else{
    Serial.print("SD card initiated!");
  }

  // initialize the RTC
    rtc.init();

    // test if clock is halted and set a date-time to start it
    if (rtc.isHalted())
    {
        Serial.println("RTC is halted. Setting time...");

        Ds1302::DateTime dt = {
            .year = 17,
            .month = Ds1302::MONTH_OCT,
            .day = 3,
            .hour = 4,
            .minute = 51,
            .second = 30,
            .dow = Ds1302::DOW_TUE
        };

        rtc.setDateTime(&dt);
    }
  Serial.println("DS1307RTC Read Test");
  Serial.println("-------------------");

  dht.begin();
 // mq2.begin(); 
}
/*
// The function below takes an integer as input (representing seconds) and prints it on the LCD as two digit
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
*/
void loop() {
  Serial.print("Sensors reading started!");
  // get the current time
    Ds1302::DateTime now;
    rtc.getDateTime(&now);
  digitalWrite(RELAY_PIN, LOW); // turn on the relay
  // Getting Temperature and Humidity Sensor's data
    dht.getData();                    // get All data from DHT11
    tempDeg = dht.getTemperature();      // return temperature in celsius
    hum = dht.getHumidity();               // return humidity
/* Uncomment this if you will use Serial Communication
     Serial.print("Temperature: ");
     Serial.print(tempDeg);
     Serial.println("Humidity: ");
     Serial.print(hum);
     delay(2000);  //delay atleast 2 seconds for DHT11 to read tha data

    */
// Display temperature and humidity on LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(tempDeg);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(hum);
    lcd.print(" %");
     // Check temperature threshold
 if (tempDeg >= 40.0) {
    digitalWrite(BUZZER_PIN, HIGH); // Buzz the buzzer
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH);    // Turn off the system
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
    digitalWrite(RELAY_PIN, LOW); // Turn on the system
  }


// Read the air velocity sensor's value
  windCount = analogRead(MQ2_PIN);
  float windSpeed = (windCount / 5.0) * 1.2;
  Serial.print(" Wind Speed: ");
  Serial.print(windSpeed);
  Serial.print(" m/s");

  //Reading the smoke sensor's data
  sensorValue = analogRead(SMOKE_SENSORPIN); // Read analog value from the smoke sensor
  Serial.print("Sensor Value is ");
  Serial.print(sensorValue);

  if (sensorValue > thresholdValue) {
    Serial.print("Smoke detected!");
    //lcd.setCursor(0,0);
    //lcd.print("Smoke Detected!");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1200);
    digitalWrite(BUZZER_PIN, LOW);
    //lcd.clear();
  } else {
    Serial.println("No smoke detected");
  }


  static uint8_t last_second = 0;
    if (last_second != now.second)
    {
        last_second = now.second;

        Serial.print("20");
        Serial.print(now.year);    // 00-99
        Serial.print('-');
        if (now.month < 10) Serial.print('0');
        Serial.print(now.month);   // 01-12
        Serial.print('-');
        if (now.day < 10) Serial.print('0');
        Serial.print(now.day);     // 01-31
        Serial.print(' ');
        Serial.print(WeekDays[now.dow - 1]); // 1-7
        Serial.print(' ');
        if (now.hour < 10) Serial.print('0');
        Serial.print(now.hour);    // 00-23
        Serial.print(':');
        if (now.minute < 10) Serial.print('0');
        Serial.print(now.minute);  // 00-59
        Serial.print(':');
        if (now.second < 10) Serial.print('0');
        Serial.print(now.second);  // 00-59
        Serial.println();
    }
    // Log data into the SD card
    dataFile = SD.open("data.txt", FILE_WRITE);
    if (dataFile) {
      Serial.print("Writing Data to the SD Card");
      dataFile.print(now.year);
      dataFile.print('/');
      dataFile.print(now.month);
      dataFile.print('/');
      dataFile.print(now.day);
      dataFile.print(' ');
      dataFile.print(now.hour);
      dataFile.print(':');
      dataFile.print(now.minute);
      dataFile.print(':');
      dataFile.print(now.second);
      dataFile.print(',');
      dataFile.print(tempDeg);
      dataFile.print(',');
      dataFile.print(hum);
      dataFile.print(',');
      dataFile.print(sensorValue);
      dataFile.print(',');
      dataFile.println(windSpeed);
      dataFile.close();
    } else {
      Serial.println("Error opening data file");
    }

  delay(10000);
}
  