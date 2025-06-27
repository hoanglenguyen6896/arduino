#include "DhtApp.h"


DHT dht(DHTPIN, DHTTYPE);

void DhtApp_Init(void)
{
    dht.begin();
}