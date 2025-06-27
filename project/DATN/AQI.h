const uint16_t AQIPoint[] = {0, 50, 100, 150, 200, 300, 400, 500}; //8
const uint16_t BP_O3[] = {0, 160, 200, 300, 400, 800, 1000, 1200};      //O3 1h
const uint32_t BP_CO[] = {0, 10000, 30000, 45000, 60000, 90000, 120000, 150000};
const uint16_t BP_NO2[] = {0, 100, 200, 700, 1200, 2350, 3100, 3850};
const uint16_t BP_PM10[] = {0, 50, 150, 250, 350, 420, 500, 600};
const uint16_t BP_PM25[] = {0, 25, 50, 80, 150, 250, 350, 500};

uint16_t AQIformula(uint16_t AQI_Iup, uint16_t AQI_Idown, uint32_t AQI_BPup, uint32_t AQI_BPdown, float concentration)
{
  uint16_t Iup = AQI_Iup;
  uint16_t Idown = AQI_Idown;
  uint32_t BPup = AQI_BPup;
  uint32_t BPdown = AQI_BPdown;
  float _con = concentration;
  float AQI_IndexRaw = ((float)(Iup - Idown)/(BPup - BPdown))*(_con - BPdown) + Idown;
  return (uint16_t) ceil(AQI_IndexRaw);
}
/*
 * Get AQI Ozone
 */
uint16_t AQI_O3(float concentrationO3)
{
  float _conO3 = concentrationO3;
  uint8_t index = 0, _i = 0;
  for(_i = 7; _i >=1; _i--)
  {
    if(_conO3 >= BP_O3[_i])
    {
      index = _i;
      break;
    }
  }
  if(index == 7) return 500;
  else return AQIformula(AQIPoint[index+1], AQIPoint[index], BP_O3[index+1], BP_O3[index], _conO3);
}
/*
 * Get AQI Carbon monoxide
 */
uint16_t AQI_CO(float concentrationCO)
{
  float _conCO = concentrationCO;
  uint8_t index = 0, _i = 0;
  for(_i = 7; _i >=1; _i--)
  {
    if(_conCO >= BP_CO[_i])
    {
      index = _i;
      break;
    }
  }
  if(index == 7) return 500;
  else return AQIformula(AQIPoint[index+1], AQIPoint[index], BP_CO[index+1], BP_CO[index], _conCO);
}
/*
 * Get AQI Nitrogen dioxide
 */
uint16_t AQI_NO2(float concentrationNO2)
{
  float _conNO2 = concentrationNO2;
  uint8_t index = 0, _i = 0;
  for(_i = 7; _i >=1; _i--)
  {
    if(_conNO2 >= BP_NO2[_i])
    {
      index = _i;
      break;
    }
  }
  if(index == 7) return 500;
  else return AQIformula(AQIPoint[index+1], AQIPoint[index], BP_NO2[index+1], BP_NO2[index], _conNO2);
}

uint16_t AQI_PM25(float cpm25[12])
{
  uint8_t index = 0, _i = 0;
  float max25;
  float min25;
  float _w  = 0, w = 0;
  float Nowcast = 0, NowcastU = 0, NowcastD = 0;
  if(((cpm25[0]==0)&&(cpm25[1]==0)) || ((cpm25[0]==0)&&(cpm25[2]==0)) || ((cpm25[1]==0)&&(cpm25[2]==0))) 
  {
    index = 10;
  }
  else
  {
    min25 = 10000;
    for(_i=0; _i<=11; _i++)
    {
      if(cpm25[_i] == 0) continue;
      max25 = max(max25, cpm25[_i]);
      min25 = min(min25, cpm25[_i]);
    }
    float _w = min25/max25;
    if(_w <= 0.5)
    {
      w = 0.5;
      for(_i=0; _i<=11; _i++)
      {
        Nowcast += pow(0.5,_i+1)*cpm25[_i];
      }
    }
    else 
    {
      w = _w;
      for(_i=0; _i<=11; _i++)
      {
        if(cpm25[_i] == 0) continue;
        NowcastU += pow(w,_i)*cpm25[_i];
        NowcastD += pow(w,_i);     
      }
      Nowcast = NowcastU/NowcastD;
    }
    for(_i = 7; _i >=0; _i--)
    {
      if(Nowcast >= BP_PM25[_i])
      {
        index = _i;
        break;
      }
    }
  }
  if(index == 10) return 0;
  else
  {
    if(index == 7) return 500;
    else return AQIformula(AQIPoint[index+1], AQIPoint[index], BP_PM25[index+1], BP_PM25[index], Nowcast);
  }

}
uint16_t AQI_PM10(float cpm10[12])
{
  uint8_t index = 0, _i = 0;
  float max10;
  float min10;
  float _w  =0, w = 0;
  float Nowcast = 0,NowcastU = 0,NowcastD = 0;
  if(((cpm10[0]==0)&&(cpm10[1]==0)) || ((cpm10[0]==0)&&(cpm10[2]==0)) || ((cpm10[1]==0)&&(cpm10[2]==0))) 
  {
    index = 10;
  }
  else
  {
    min10 = 10000;
    for(_i=0; _i<=11; _i++)
    {
      if(cpm10[_i] == 0) continue;
      max10 = max(max10, cpm10[_i]);
      min10 = min(min10, cpm10[_i]);
    }
    float _w = min10/max10;
    if(_w <= 0.5)
    {
      w = 0.5;
      for(_i=0; _i<=11; _i++)
      {
        Nowcast += pow(0.5,_i+1)*cpm10[_i];
      }
    }
    else 
    {
      w = _w;
      for(_i=0; _i<=11; _i++)
      {
        if(cpm10[_i] == 0) continue;
        NowcastU += pow(w,_i)*cpm10[_i];
        NowcastD += pow(w,_i);     
      }
      Nowcast = NowcastU/NowcastD;
    }
    for(_i = 7; _i >=0; _i--)
    {
      if(Nowcast >= BP_PM10[_i])
      {
        index = _i;
        break;
      }
    }
  }
  if(index == 10) return 0;
  else
  {
    if(index == 7) return 500;
    else return AQIformula(AQIPoint[index+1], AQIPoint[index], BP_PM10[index+1], BP_PM10[index], Nowcast);
  }
}
