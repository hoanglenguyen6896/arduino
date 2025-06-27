#include <SoftwareSerial.h>
#include "SDS.h"
#include "AQI.h"
#include "mass.h"
#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>

Adafruit_BME280 bme;
Adafruit_ADS1115 ads;
SoftwareSerial my_sds(6, 7);
SoftwareSerial sim(12, 11);
String APIKey = "54CQ7YPGU61BP9KM";
String MainKey = "7V5LRUVUAU5S7C4C";

float o3_con, no2_con, co_con, pm25_con, pm10_con, temp_con, hum_con;
float o3_total, no2_total, co_total, pm25_total, pm10_total, temp_total, hum_total;
float o3_avr, no2_avr, co_avr, pm25_avr, pm10_avr, temp_avr, hum_avr;
float EEpm25[12], EEpm10[12];
uint16_t day, month, year, hour, minute, sec;
uint16_t AQImax, Ozone, Particulate25, Pparticulate10, Carbon, Nitrogen, Temperature, Humidity;
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
  //Check and set Module Sim 808 Status
  //10 - C_PW  9 - PWK   8 - ST
  pinMode(10, OUTPUT);
  pinMode(8, INPUT);
  //Serial.println(digitalRead(8));
  pinMode(9, OUTPUT);
  //digitalWrite(9, LOW);

  Serial.println(millis());
  
  sim.listen();
  SIMpowerOn();
  SIMstart();
  SIMinit();
  SIMgprsOn();
  SIMinternetInit();
  delay(5000);
  sim.listen();
  SIMgetDateTimeOnline(&day, &month, &year, &hour, &minute, &sec);
  Serial.println((String) hour+"h"+minute+"m"+sec+"s"+ " " +day+ "/" +month+"/"+year);
}

void loop()
{
  Serial.println(millis());
  
  sim.listen();
  SIMgetDateTimeOnline(&day, &month, &year, &hour, &minute, &sec);
  Serial.println((String) hour+"h"+minute+"m"+sec+"s"+ " " +day+ "/" +month+"/"+year);
  //SIMpowerOff();
  uint16_t executeTime1 = 59 - minute;
  uint16_t executeTime2 = 60 - sec;
  uint16_t TotalTime = 60*executeTime1 + executeTime2;
  uint16_t estimateTimes = floor (TotalTime/240);
  Serial.println(executeTime1);
  Serial.println(executeTime2);
  Serial.println(TotalTime);
  Serial.println(estimateTimes);
  delay(5000);
  Serial.println(millis());
}

//Return mass of pollution
void execute(float *o3, float *co, float *no2, float *pm25C, float *pm10C, float *temC, float *humC)
{
  uint32_t i = 0, j = 0;
  boolean wakeflag = false, getDataflag = false, adcflag = false;

  float rTem, rPres, rHum;

  float pm25, pm10, pm25Sum, pm10Sum, pm25avr, pm10avr;

  uint16_t o3adc, coadc, no2adc;

  float o3_ugperm3, co_ugperm3, no2_ugperm3, tempTt, humTt;
  float o3avr, coavr, no2avr, temavr, humavr;
  unsigned long timeMillis, sdsMillis, adcMillis, adcMillisP;
  wakeflag = false; getDataflag = false; adcflag = false;
  o3_ugperm3 = 0;
  co_ugperm3 = 0;
  no2_ugperm3 = 0;
  tempTt = 0;
  humTt = 0;
  pm25Sum = 0; pm10Sum = 0;
  timeMillis = millis();
  adcMillis = millis();
  while (millis() - timeMillis <= 240000)
  {
    if (millis() - adcMillis >= 1000)
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
      tempTt += rTem;
      humTt += rHum;
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
  o3avr = (float) o3_ugperm3 / i;  coavr = (float) co_ugperm3 / i;  no2avr = (float) no2_ugperm3 / i;
  temavr = tempTt / i; humavr = humTt / i;
  pm25avr = (float) pm25Sum / j;  pm10avr = (float) pm10Sum / j;
  
  *o3 = o3avr; *co = coavr; *no2 = no2avr;
  *pm25C = pm25avr; *pm10C = pm10avr;
  *temC = temavr; *humC = humavr;
  SDSsetSleep();
  Serial.println((String) "i: " + i + " j: " + j);
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
  while (my_sds.available())
  {
    byte c = my_sds.read();
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
  String textttt="";
  digitalWrite(9, LOW);
  if(digitalRead(8) == 0)
  {
    Serial.println("Starting... ... ...");
    digitalWrite(9, HIGH);
    delay(1000);
    delay(1000);
    digitalWrite(9, LOW);
    while(sim.available() <= 0)
    {
    }
    delay(10000);
    while(sim.available() > 0)
    {
      char c = sim.read();
      if(c > 0)
      {
        textttt +=c;
      }
    }
    Serial.println(textttt);
  }
  else Serial.println("Started");
  
}
void SIMpowerOff()
{
  digitalWrite(10,HIGH);
}
void SIMpowerOn()
{
  digitalWrite(10,LOW);
}
void SIMinit()
{
  String text1;
  sim.listen();
  sim.print("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("SAPBR GPRS");
  text1="";
  delay(1000);
  
  sim.print("AT+SAPBR=3,1,\"APN\",\"v-internet\"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("SAPBR APN");
  text1="";
  delay(1000);

  sim.print("AT+SAPBR=3,1,\"USER\",\" \"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("SAPBR USER");
  text1="";
  delay(1000);

  sim.print("AT+SAPBR=3,1,\"PWD\",\" \"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("SAPBR PWD");
  text1="";
  delay(1000);
}

void SIMgprsOn()
{
  String text1;
  sim.print("AT+SAPBR=1,1\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("SAPBR Start GPRS");
  text1="";
  delay(1000);
}

void SIMgprsOff()
{
  String text1;
  sim.print("AT+SAPBR=0,1\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("SAPBR Start GPRS");
  text1="";
  delay(1000);
}

void SIMinternetInit()
{
  String text1;
  sim.print("AT+HTTPINIT\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("HTTP INIT");
  text1="";
  delay(1000);
}

void SIMsend(int AQI, int AQIO3, int AQIPM25, 
              int AQIPM10, int AQICO, int AQINO2, 
              float Temm, float Humm)
{
  String text1;
  sim.print("AT+HTTPPARA=\"CID\",1\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("HTTP CID");
  text1="";
  delay(1000);

  sim.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=");
  sim.print(APIKey);
  sim.print("&field1="); sim.print(AQI);
  sim.print("&field2="); sim.print(AQIO3);
  sim.print("&field3="); sim.print(AQIPM25);
  sim.print("&field4="); sim.print(AQIPM10);
  sim.print("&field5="); sim.print(AQICO);
  sim.print("&field6="); sim.print(AQINO2);
  sim.print("&field7="); sim.print(Temm);
  sim.print("&field8="); sim.print(Humm);
  sim.print("\"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 0)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("HTTP SENT");
  text1="";
  delay(1000);

  sim.print("AT+HTTPACTION=0\r\n");
  delay(5000);
  while(sim.available()>0)
  {
    char c = sim.read();
    if(c >= 32)
    {
      text1 +=c;
    }
  }
  
  Serial.println(text1);
  Serial.println("HTTP Action");
  text1="";
  delay(1000);
}



void SIMgetDateTimeOnline(uint16_t *dday, uint16_t *dmonth, uint16_t *dyear, 
                          uint16_t *hhour, uint16_t *hmin, uint16_t *hsec)
{
  String text="";
  sim.print("AT+HTTPPARA=\"CID\",1\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
  }

  sim.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=");
  sim.print(APIKey);
  sim.print("&field1=400");
  sim.print("\"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
  }
  delay(3000);
  
  sim.print("AT+HTTPACTION=0\r\n");
  delay(5000);
  while(sim.available()>0)
  {
    char c = sim.read();
  }
  
  sim.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/channels/928487/feeds/last.json?api_key=54CQ7YPGU61BP9KM\"\r\n");
  delay(1000);
  while(sim.available()>0)
  {
    char c = sim.read();
  }
  delay(3000);
  sim.print("AT+HTTPACTION=0\r\n");
  delay(5000);
  while(sim.available()>0)
  {
    char c = sim.read();
  }
  
  boolean newData = true;
  sim.print("AT+HTTPREAD=0,64\r\n");
  //delay(1000);
  while(sim.available()>0 || newData == true)
  {
    char c = sim.read();
    if(c-48 >= 0 && c-48 <=9)
    {
      text +=c;
    }
    if (c == 44) newData = false;
  }
  Serial.println(text);
 
  *dyear = text.charAt(4)*10 - 480 + text.charAt(5) - 48;
  *dmonth = text.charAt(6)*10 - 480 + text.charAt(7) - 48;
  *dday = text.charAt(8)*10 - 480 + text.charAt(9) - 48;
  uint16_t _hour = text.charAt(10)*10 - 480 + text.charAt(11) - 48;
  *hmin = text.charAt(12)*10 - 480 + text.charAt(13) - 48;
  *hsec = text.charAt(14)*10 - 480 + text.charAt(15) - 48;
  uint16_t _hhour = _hour + 7;
  if(_hhour <=23) *hhour = _hhour;
  else *hhour = _hhour - 24;
}
