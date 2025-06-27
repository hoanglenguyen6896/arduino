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
float o3_con, no2_con, co_con, pm25_con, pm10_con;
float o3_total, no2_total, co_total, pm25_total, pm10_total;
float o3_avr, no2_avr, co_avr, pm25_avr, pm10_avr;
float tempT, temp_total, temp_avr, humT, hum_total, hum_avr;
Adafruit_BME280 bme;
Adafruit_ADS1115 ads;
int i;
float Ro3tt,Rno2tt,Rcott, Ro3,Rno2,Rco;
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
  SIMpowerOff();
  i=1;
}
void loop()
{
  
  uint16_t o3, no2, co;
  float pres, temp, hum;
  
  o3 = ads.readADC_SingleEnded(0);
  no2 = ads.readADC_SingleEnded(1);
  co = ads.readADC_SingleEnded(2);
  temp = bme.readTemperature();
  hum = bme.readHumidity();
  pres = bme.readPressure();
  
  float VRL = o3*0.1875;
  float rsRaw = (((float) 5000/VRL) - 1)*1000;
  float rsraw1;
  if(hum<50)  //use 30%rh
  {
    for(uint8_t i=0;i<=10;i++)
    {
      if(temp<=tempText[i]+2.5)
      {
        rsraw1 = (float) rsRaw*100/o3ratio30[i];
        break;
      }
    }
  }
  else if(hum<75)
  {
    for(uint8_t i=0;i<=10;i++)
    {
      if(temp<=tempText[i]+2.5)
      {
        rsraw1 = (float) rsRaw*100/o3ratio60[i];
        break;
      }
    }
  }
  else
  {
    for(uint8_t i=0;i<=10;i++)
    {
      if(temp<=tempText[i]+2.5)
      {
        rsraw1 = (float) rsRaw*100/o3ratio85[i];
        break;
      }
    }
  }
  float VRLC = co*0.1875;
  float rsRawC = (((float) 5000/VRLC) - 1)*47;

  float VRLN = no2*0.1875;
  float rsRawN = (((float) 5000/VRLN) - 1)*22;
  //Ro3tt += rsraw1;
  //Rno2tt +=rsRawN;
  //Rcott +=rsRawC;
  Serial.println((String) "Temp = " + temp);
  Serial.println((String) "Hum = " + hum);
  Serial.println((String) "Pres = " + pres);
  Serial.println((String) "RsO3 = " + rsraw1);
  Serial.println(o3_ugpm3(o3,temp,hum,pres));
  Serial.println((String) "RsNO2 = " + rsRawN);
  Serial.println(no2_ugpm3(no2,temp,hum,pres));
  Serial.println((String) "RsCO = " + rsRawC);
  Serial.println(co_ugpm3(co,temp,hum,pres));
  Serial.println();
  delay(3000);
  
  /*if(i == 1200)
  {
    Ro3 = Ro3tt/i; Rno2 = Rno2tt/i; Rco = Rcott/i;
    Serial.println((String) "RsO3 = " + Ro3);
    Serial.println((String) "RsNO2 = " + Rno2);
    Serial.println((String) "RsCO = " + Rco);
    Serial.println();
    Ro3tt=0; Rno2tt=0; Rcott=0;
    i=0;
  }
  i++;*/
}


/*--------------------------------------------------
   SDS function
  --------------------------------------------------*/
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


/*--------------------------------------------------
   SIM function
  --------------------------------------------------*/

void SIMpowerOff()
{
  digitalWrite(10, HIGH);
}
