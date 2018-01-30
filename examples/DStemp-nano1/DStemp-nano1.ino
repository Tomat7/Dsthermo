/*
   Example for DStemp library on Arduino Nano (c) Tomat7, Jan 2018
   Method check() returns NOTHING!
   only check for temperature from OneWire sensor

   Temp return current temperature OR:
   -99  sensor not found
   -83  sensor was found but conversation not finished within defined timeout
   -77  sensor was found and ready but something going wrong
   -71  sensor was found but CRC error

   Connected == 0 значит датчика нет - no sensor found
   в dsMiilis хранится millis() c момента запроса или крайнего Init()
   от dsMillis отсчитывается msConvTimeout - время потраченное на преобразование (DEBUG!)
*/

#include "DStemp.h"
DSThermometer sensor(5); // на этом пине подключен **один** датчик DS18B20
unsigned long msReinit;

void setup()
{
  Serial.begin(115200);
  delay(500);
  sensor.init(750);   // 750 время в миллисекундах необходимое датчику на преобразование
  Serial.print("Connected ");
  Serial.println(sensor.Connected, DEC);
  msReinit = millis();
}

void loop()
{
  sensor.check();   // основной метод, делает всё:
  // - проверяет наличие датчика
  // - инициализирует его при необходимости
  // - командует датчику начать преобразование температуры
  // - получает результат от датчика

  if ((millis() - msReinit) > 1000)
  {
    Serial.print(sensor.Temp);
    String dsInfo = " | parasite power: " + String(sensor.Parasite, DEC) +
                    " | " + String(sensor.TimeConv, DEC);
    Serial.println(dsInfo);
    msReinit = millis();
  }
}
