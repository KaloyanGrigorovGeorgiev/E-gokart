#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN A2
#define DHTTYPE DHT11

const int lcdColumns = 16;
const int lcdRows = 2;
const int AirsensorPin = A0;
const int fanPin = 20;

const float FAN_ON_TEMP  = 60.0; 
const float FAN_OFF_TEMP = 55.0; 

float rho = 1.204;
int offset = 0;
int veloc_mean_size = 50;
int zero_span = 100;
int zero_point = 2830;

bool fanRunning = false;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
  lcd.init();
  lcd.backlight();
  dht.begin();
}

void loop() {
  float adc_avg = 0;
  float veloc = 0.0;

  for (int ii = 0; ii < veloc_mean_size; ii++) {
    adc_avg += analogRead(AirsensorPin) - offset;
  }
  adc_avg /= veloc_mean_size;

  if (adc_avg < 100 || adc_avg > 4000) {
    veloc = 0.0;
  } else if (adc_avg > zero_point - zero_span && adc_avg < zero_point + zero_span) {
    veloc = 0.0;
  } else {
    float pressure = (adc_avg / 4095.0) - (2.5 / 3.3);
    float under_sqrt = (10000.0 * abs(pressure)) / rho;
    if (under_sqrt < 0) {
      veloc = 0.0;
    } else {
      veloc = sqrt(under_sqrt);
      if (adc_avg < zero_point) veloc = 0.0;
    }
  }

  float veloc_kmh = veloc * 3.6;
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    temperature = 0.0;
  }


  if (!fanRunning && temperature >= FAN_ON_TEMP) {
    fanRunning = true;
    digitalWrite(fanPin, HIGH);
  } else if (fanRunning && temperature <= FAN_OFF_TEMP) {
    fanRunning = false;
    digitalWrite(fanPin, LOW);
  }

  // Serial.print("ADC raw: ");
  // Serial.print(adc_avg);
  // Serial.print("  Temp: ");
  // Serial.print(temperature);
  // Serial.print(" C  |  Speed: ");
  // Serial.print(veloc_kmh);
  // Serial.print(" km/h  |  Fan: ");
  // Serial.println(fanRunning ? "ON" : "OFF");

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperature, 1);
  lcd.print("C ");
 

  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(veloc_kmh, 1);
  lcd.print(" km/h");

  delay(2000);
}