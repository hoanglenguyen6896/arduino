#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_ADS1015.h>

#include "SDS.h"
#include "AQI.h"
#include "mass.h"

SoftwareSerial my_sds(6, 7);
SoftwareSerial sim(12, 11);

Adafruit_BME280 bme;
Adafruit_ADS1115 ads;

#define APIKey  "54CQ7YPGU61BP9KM"
//#define MainKey  "8BRRHUSU6V4PUKV7"
#define MainKey "7V5LRUVUAU5S7C4C"

float o3_con, no2_con, co_con, pm25_con, pm10_con;
float o3_total, no2_total, co_total, pm25_total, pm10_total;
float o3_avr, no2_avr, co_avr, pm25_avr, pm10_avr;
float tempT, temp_total, temp_avr, humT, hum_total, hum_avr;
float EEpm25[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
                   , EEpm10[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint16_t day, month, year, hour, minute, sec;

uint16_t AQImax, Ozone, Particulate25, Particulate10, Carbon, Nitrogen;
float Temperature, Humidity;
void setup()
{
  //Start Serial
  Serial.begin(9600);
  sim.begin(9600);
  my_sds.begin(9600);
  
  //Start ADS1115 & BME
  ads.begin();
  unsigned status;
  status = bme.begin();

  //Set SDS to sleep
  my_sds.listen();
  delay(1000);
  SDSsetSleep();
  pinMode(10, OUTPUT);
  pinMode(8, INPUT);
  pinMode(9, OUTPUT);
  
  sim.listen();
  SIMpowerOn();
  SIMstart();
  SIMinit();
  SIMgprsOn();
  SIMinternetInit();
  delay(3000);
  SIMgetDateTimeOnline(&day, &month, &year, &hour, &minute, &sec);

  Serial.println((String) day + "/" + month + "/" + year + "\t" + hour + "h" + minute + "m" + sec + "s");
  int k = hour-1;
  int js = hour-1;
  for(int i = 11; i>=0; i--)
  {
    if(k<0) js=23+k;
    else js = k;
    EEPROM.get(js*4,EEpm25[12-i]);
    delay(10);
    EEPROM.get(js*4+200,EEpm10[12-i]);
    delay(10);
    k--;

  }
  delay(1000);
  SIMpowerOff();
}

void loop()
{
  uint8_t executeTime1 = 59 - minute;
  uint8_t executeTime2 = 60 - sec;
  uint16_t TotalTime = 60 * executeTime1 + executeTime2;
  uint8_t estimateTimes = floor (TotalTime / 240);
  o3_total = 0;
  no2_total = 0;
  co_total = 0;
  pm25_total = 0;
  pm10_total = 0;
  temp_total = 0;
  hum_total = 0;
  delay(1000);
  Serial.println((String) "Number of 4m Cycle: " + estimateTimes);
  if (estimateTimes > 0)
  {
    uint8_t es;
    for (es = 1; es <= estimateTimes; es++)
    {
      execute(&o3_con, &co_con, &no2_con, &pm25_con, &pm10_con, &tempT, &humT);
      o3_total += o3_con;
      no2_total += no2_con;
      co_total += co_con;
      pm25_total += pm25_con;
      pm10_total += pm10_con;
      temp_total += tempT;
      hum_total += humT;
      Serial.println((String) "PM2.5 = " + pm25_con + " ug/m3\t" + "PM10 = " + pm10_con + " ug/m3");
      Serial.println((String) "O3 = "+ o3_con + " ug/m3");
      Serial.println((String) "CO = "+ co_con + " ug/m3");
      Serial.println((String) "NO2 = "+ no2_con + " ug/m3");
    }
    o3_avr = o3_total/estimateTimes;
    no2_avr = no2_total/estimateTimes;
    co_avr = co_total/estimateTimes;
    pm25_avr = pm25_total/estimateTimes;
    pm10_avr = pm10_total/estimateTimes;
    temp_avr = temp_total/estimateTimes;
    hum_avr = hum_total/estimateTimes;
  }
  else
  {
    o3_avr = 0; no2_avr = 0;
    co_avr = 0;
    pm25_avr = 0; pm10_avr = 0;
    temp_avr = 0; hum_avr = 0;
  }
  
  EEpm25[0] = pm25_avr;
  EEpm10[0] = pm10_avr;

  Ozone = AQI_O3(o3_avr);
  Particulate25 = AQI_PM25(EEpm25);
  Particulate10 = AQI_PM10(EEpm10);
  Carbon = AQI_CO(co_avr);
  Nitrogen = AQI_NO2(no2_avr);
  Temperature = temp_avr;
  Humidity = hum_avr;
  
  AQImax = Ozone;
  if (Particulate25 > AQImax) AQImax = Particulate25;
  if (Particulate10 > AQImax) AQImax = Particulate10;
  if (Carbon > AQImax) AQImax = Carbon;
  if (Nitrogen > AQImax) AQImax = Nitrogen;
  
  sim.listen();
  SIMpowerOn();
  SIMstart();
  delay(5000);
  SIMinit();
  SIMgprsOn();
  SIMinternetInit();
  delay(5000);
  
  SIMsend( AQImax,  Ozone,  Particulate25, Particulate10,  Carbon,  Nitrogen, Temperature, Humidity);
           
  delay(5000);

  EEPROM.put(hour*4, pm25_avr);
  delay(10);
  EEPROM.put(hour*4+200, pm10_avr);
  delay(10);
  uint16_t _hour = hour;
  uint16_t _min = minute;
  uint16_t _sec = sec;
  if(_hour<=11)
  {
    EEPROM.put((hour+12)*4,(float) 0.00);
    delay(10);
    EEPROM.put((hour+12)*4+200,(float) 0.00);
    delay(10);
  }
  else
  {
    EEPROM.put((hour-12)*4,(float) 0.00);
    delay(10);
    EEPROM.put((hour-12)*4+200,(float) 0.00);
    delay(10);
  }
  
  SIMgetDateTimeOnline(&day, &month, &year, &hour, &minute, &sec);
  delay(5000);
  SIMpowerOff();

  
  if (_hour == hour)
  {
    uint16_t k = abs(59 - minute) * 60 + abs(60 - sec);
    hour++;
    minute = 0;
    sec = 0;
    if(hour == 24) hour = 0;
    delay(k*1000);
  }
  for(int i=11; i>0; i--)
  {
    EEpm25[i] = EEpm25[i-1];
    EEpm10[i] = EEpm10[i-1];
  }
  EEpm10[0] = 0;
  EEpm25[0] = 0;
  
}

//Return mass of pollution
void execute(float *o3, float *co, float *no2, float *pm25C, float *pm10C, float *tempT, float *humid)
{
  uint32_t i = 0, j = 0;
  boolean wakeflag = false, getDataflag = false;

  float rTem, rPres, rHum;

  float pm25, pm10, pm25Sum, pm10Sum, pm25avr, pm10avr;

  uint16_t o3adc, coadc, no2adc;

  float o3_ugperm3, co_ugperm3, no2_ugperm3, temTo, humTo;
  unsigned long timeMillis, sdsMillis, adcMillis;
  wakeflag = false; getDataflag = false;
  o3_ugperm3 = 0;
  co_ugperm3 = 0;
  no2_ugperm3 = 0;
  temTo = 0;
  humTo = 0;
  pm25Sum = 0; 
  pm10Sum = 0;
  timeMillis = millis();
  adcMillis = millis();
  while (millis() - timeMillis <= 240000)
  {
    if (millis() - adcMillis >= 3000)
    {
      adcMillis = millis();
      o3adc = ads.readADC_SingleEnded(0);
      no2adc = ads.readADC_SingleEnded(1);
      coadc = ads.readADC_SingleEnded(2);

      rTem = bme.readTemperature();
      rPres = bme.readPressure();
      rHum = bme.readHumidity();

      o3_ugperm3 += o3_ugpm3(o3adc, rTem, rHum, rPres);
      co_ugperm3 += co_ugpm3(coadc, rTem, rHum, rPres);
      no2_ugperm3 += no2_ugpm3(no2adc, rTem, rHum, rPres);

      temTo += rTem;
      humTo += rHum;
      
      i++;
    }
    if (millis() - timeMillis >= 150000 && wakeflag == false)
    {
      my_sds.listen();
      SDSsetWakeup();
      wakeflag = true;
    }
    if (millis() - timeMillis >= 180000 && getDataflag == false)
    {
      SDSsetQuery();
      getDataflag = true;
      sdsMillis = millis();
    }
    if (getDataflag == true && millis() - sdsMillis >= 3000)
    {
      SDSgetData(&pm25, &pm10);
      sdsMillis = millis();
      pm25Sum += pm25;
      pm10Sum += pm10;
      j++;
    }
  }
  *o3 = (float) o3_ugperm3/i;  
  *co = (float) co_ugperm3/i;  
  *no2 = (float) no2_ugperm3/i;
  *pm25C = (float) pm25Sum/j;  
  *pm10C = (float) pm10Sum/j;
  *tempT = (float) temTo/i;
  *humid = (float) humTo/i;
  SDSsetSleep();
}

/*--------------------------------------------------
   SDS function
  --------------------------------------------------*/
void SDSsetQuery()  //Set query mode
{
  uint8_t i = 0;
  for (i = 0; i <= 18; i++)
  {
    my_sds.write(query[i]);
    delay(10);
  }
  delay(1000);
  i = 0;
  while (my_sds.available())
  {
    respond[i] = my_sds.read();
  }
}

void SDSsetSleep()  //Set sleep
{
  uint8_t i = 0;
  for (i = 0; i <= 18; i++)
  {
    my_sds.write(sleep[i]);
    delay(10);
  }
  delay(1000);
  i = 0;
  while (my_sds.available())
  {
    respond[i] = my_sds.read();
    i++;
  }
}

void SDSsetWakeup()   //Set wake up working
{
  uint8_t i = 0;
  for (i = 0; i <= 18; i++)
  {
    my_sds.write(wakeup[i]);
    delay(0);
  }
  delay(1000);
  i = 0;
  while (my_sds.available())
  {
    respond[i] = my_sds.read();
    i++;
  }
}

void SDSgetData(float *pm25, float *pm10)   //Get data from query mode
{
  uint8_t i = 0;
  for (i = 0; i <= 18; i++)
  {
    my_sds.write(queryData[i]);
    delay(0);
  }
  delay(1000);
  i = 0;
  while (my_sds.available())
  {
    respond[i] = my_sds.read();
    i++;
  }
  delay(1000);
  *pm25 = (float) (respond[2] + respond[3] * 256) / 10;
  *pm10 = (float) (respond[4] + respond[5] * 256) / 10;
}

/*--------------------------------------------------
   SIM function
  --------------------------------------------------*/

void SIMstart()
{
  char c;
  digitalWrite(9, LOW);
  if (digitalRead(8) == 0)
  {
    digitalWrite(9, HIGH);
    delay(1000);
    delay(1000);
    digitalWrite(9, LOW);
    while (sim.available() <= 0)
    {
    }
    delay(10000);
    while (sim.available() > 0)
    {
      c = sim.read();
    }
  }
}
void SIMpowerOff()
{
  digitalWrite(10, HIGH);
}
void SIMpowerOn()
{
  digitalWrite(10, LOW);
}
void SIMinit()
{
  char c;
  sim.listen();
  sim.print("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
  sim.print("AT+SAPBR=3,1,\"APN\",\"v-internet\"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);

  sim.print("AT+SAPBR=3,1,\"USER\",\" \"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);

  sim.print("AT+SAPBR=3,1,\"PWD\",\" \"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
}

void SIMgprsOn()
{
  char c;
  sim.print("AT+SAPBR=1,1\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
}

void SIMgprsOff()
{
  char c;
  sim.print("AT+SAPBR=0,1\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
}

void SIMinternetInit()
{
  char c;
  sim.print("AT+HTTPINIT\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
}

void SIMsend(int AQI, int AQIO3, int AQIPM25, int AQIPM10, int AQICO, int AQINO2, float TEMPER, float HUMI)
{
  char c;
  //String text1;
  sim.print("AT+HTTPPARA=\"CID\",1\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
  sim.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=");
  sim.print(MainKey);
  sim.print("&field1="); sim.print(AQI);
  sim.print("&field2="); sim.print(AQIO3);
  sim.print("&field3="); sim.print(AQIPM25);
  sim.print("&field4="); sim.print(AQIPM10);
  sim.print("&field5="); sim.print(AQICO);
  sim.print("&field6="); sim.print(AQINO2);
  sim.print("&field7="); sim.print(TEMPER);
  sim.print("&field8="); sim.print(HUMI);
  sim.print("\"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
  sim.print("AT+HTTPACTION=0\r\n");
  delay(5000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(1000);
}

void SIMgetDateTimeOnline(uint16_t *dday, uint16_t *dmonth, uint16_t *dyear,
                          uint16_t *hhour, uint16_t *hmin, uint16_t *hsec)
{
  String text = "";
  char c;
  sim.print("AT+HTTPPARA=\"CID\",1\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  sim.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=");
  sim.print(APIKey);
  sim.print("&field1=400");
  sim.print("\"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(3000);
  sim.print("AT+HTTPACTION=0\r\n");
  delay(5000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  sim.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/channels/928487/feeds/last.json?api_key=54CQ7YPGU61BP9KM\"\r\n");
  delay(1000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  delay(3000);
  sim.print("AT+HTTPACTION=0\r\n");
  delay(5000);
  while (sim.available() > 0)
  {
    c = sim.read();
  }
  boolean newData = true;
  sim.print("AT+HTTPREAD=0,64\r\n");
  while (sim.available() > 0 || newData == true)
  {
    c = sim.read();
    if (c - 48 >= 0 && c - 48 <= 9)
    {
      text += c;
    }
    if (c == 44) newData = false;
  }
  *dyear = text.charAt(4) * 10 - 480 + text.charAt(5) - 48;
  *dmonth = text.charAt(6) * 10 - 480 + text.charAt(7) - 48;
  *dday = text.charAt(8) * 10 - 480 + text.charAt(9) - 48;
  *hhour = text.charAt(10) * 10 - 480 + text.charAt(11) - 48;
  *hmin = text.charAt(12) * 10 - 480 + text.charAt(13) - 48;
  *hsec = text.charAt(14) * 10 - 480 + text.charAt(15) - 48;
  //uint16_t _hhour = _hour + 7;
  //if (_hhour <= 23) *hhour = _hhour;
  //else *hhour = _hhour - 24;
}
