#define R0o3  938.62
#define R0co  58.09
#define R0no2  21.84

#define o3MW 48
#define coMW 28
#define no2MW 46
/*
 * mg/m3 = ppm*(M/22.4)*(273/T)*(p/101.3)
 * (ug/m3) = ppb*(M/22.4)*(273/T)*(p/101.3)
 * M is Molecular Weight
 * T is temperature, Kelvin unit
 * p is pressure, Pascal unit
 */
uint8_t o3ratio30[] = {158, 150, 142, 130, 125, 119, 118, 111, 100, 94, 88};
uint8_t o3ratio60[] = {135, 128, 122, 110, 107, 101, 98, 95, 85, 80, 73};
uint8_t o3ratio85[] = {118, 110, 105, 97, 91, 87, 83, 80, 72, 68, 62};
uint8_t tempText[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 100};
//Convert O3 Adc value to ug/m3
float o3_ugpm3(uint16_t oo3adc, float ttemp, float hhum, float ppres)
{
  float temp = ttemp;
  float hum = hhum;
  float pres = ppres;
  float VRL = oo3adc*0.1875;
  float rsRaw = (((float) 5000/VRL) - 1)*1000;
  float rsraw1;
  float Rratio;
  float ppbO3, ugpm3O3;  //y = 8.9067*x^(2.2864);
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
  Rratio = rsraw1/R0o3;
  if(Rratio <= 1.1) 
  {
    ppbO3=10;
  }
  else if(Rratio >=7.8)
  {
    ppbO3=1000;
  }
  else
  {
    ppbO3 = 8.9067*pow(Rratio,2.2864);
  }
  
  ugpm3O3 = (float) (ppbO3*o3MW*273*pres)/(22.4*293*101300);
  return ugpm3O3;
}
//Convert CO Adc value to ug/m3
float co_ugpm3(uint16_t ccoadc, float ttempC, float hhumC, float ppresC)
{
  float tempC = ttempC;
  float humC = hhumC;
  float presC = ppresC;
  float VRLC = ccoadc*0.1875;
  float rsRawC = (((float) 5000/VRLC) - 1)*47;
  float RratioC;
  float ppbCO, ugpm3CO;  //y = 4.2943x^(-1.175)
  RratioC = rsRawC/R0co;
  if(RratioC>=3.4)
  {
    ppbCO = 1000;
  }
  else if(RratioC<=0.01)
  {
    ppbCO = 1000000;
  }
  else
  {
    ppbCO = 4294.3*pow(RratioC,-1.175);
  }
  ugpm3CO = (float) (ppbCO*coMW*273*presC)/(22.4*298*101300);
  return ugpm3CO;
}
//Convert NO2 Adc value to ug/m3
float no2_ugpm3(uint16_t nno2adc, float ttempN, float hhumN, float ppresN)
{
  float tempN = ttempN;
  float humN = hhumN;
  float presN = ppresN;
  float VRLN = nno2adc*0.1875;
  float rsRawN = (((float) 5000/VRLN) - 1)*22;
  float RratioN;
  float ppbNO2, ugpm3NO2;  //y = 163.94x^0.9908
  RratioN = rsRawN/R0no2;
  if(RratioN<=0.31)
  {
    ppbNO2 = 50;
  }
  else if(RratioN >= 31)
  {
    ppbNO2 = 50000;
  }
  else
  {
    ppbNO2 = 163.94*pow(RratioN,0.9908);
  }
  ugpm3NO2 = (float) (ppbNO2*no2MW*273*presN)/(22.4*298*101300);
  return ugpm3NO2;
}
