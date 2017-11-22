// здесь все функции
/*
DSThermometer.cpp - Library to operate with DS18B20
Created by Tomat7, October 2017.
*/
#include "Arduino.h"
#include "DSthermo.h"
#include <OneWire.h>

DSThermometer::DSThermometer(OneWire *ds) //: OneWire {ds18x20(pin)}
{
	_msConvTimeout = CONVERSATIONTIME;
	_ds = ds;
	Parasite = false;
	Connected = false;
	dsMillis = 0;
	/*
	_ds -> reset();	
	_ds -> write(0xCC);
	_ds -> write(0xB4);
	if (_ds->read_bit() == 0) { Parasite = true;
	} else { Parasite = false; }
	*/
	/* а вот это не сработало :(
	_pin = pin;
	OneWire ds18x20(_pin);
	OneWire *_ds = new OneWire(pin);
	*/
}

void DSThermometer::init(uint16_t convtimeout)
{
	//	Serial.print("init.. ");
	_msConvTimeout = convtimeout;
	/* if (_ds->read_bit() == 0) { Parasite = true;
	} else { Parasite = false; } */
	initOW();
	setHiResolution();
#ifdef DEBUG2
	Serial.print(_temperature);
	Serial.print(" -init-stop..");
	Serial.println(millis());
#endif
}
/*
check() returns NOTHING!
only check for temperature changes from OneWire sensor
*** NOT IN THIS VERSION!! -100 - conversation not finished yet but sensor still OK
-99  - sensor not found
-98  - sensor was found but conversation not finished within defined timeout
-92  - sensor was found but CRC error 
-91  - sensor was found but something going wrong

Connected = 0 значит датчика нет - no sensor found
в других случаях в нем хранится millis() c момента запроса
от которого отсчитывается msConvTimeout
*/


void DSThermometer::check()
{
	bool TimeIsOut = ((millis() - dsMillis) > _msConvTimeout);

	if (Connected && (_ds->read_bit() == 1))   // вроде готов отдать данные
	{
		//Serial.print("+");          
		_temperature = askOWtemp();  // но можем ещё получить -11 или -22
		if (_temperature >= 0)
		{
			Temp = _temperature;
			TimeConv = millis() - dsMillis;
			requestOW();           		// вроде всё ОК, значит запрашиваем снова
			return;						// и сваливаем
		} 
		else if (TimeConv != 0)			// типа первый раз косяк с CRC
		{
			TimeConv = 0;				// ставим маячёк на будущее
			//initrequest();			// но оставляем прежнюю температуру!
		}
		else							// значит повторный CRC errorr
		{
			Temp = _temperature;		// сообщаем горькую правду
			//initrequest();
		}
	} 
	else if (Connected && TimeIsOut)   // не готов отдать данные и время истекло
	{
		//Serial.print("**");     
		Temp = -33;					// небыло совсем или оторвали - косяк короче
		TimeConv = 0;
		//initrequest();
	}
	else if (TimeIsOut)				// датчика нет и пора проверить его появление
	{
		//Serial.print("--");                   // выводим в Serial
		Temp = -99;
		TimeConv = 0;
		//initrequest();
	} 
	else { return; }
	
	initOW();                             // и пробуем инициализировать
	if (Connected) requestOW();
	return;
}

float DSThermometer::askOWtemp()
{
	byte present = 0;
	byte bufData[9]; // буфер данных
	float owTemp;

	present = _ds -> reset();
	if (present)
	{
		_ds -> write(0xCC);
		_ds -> write(0xBE);                            // Read Scratchpad
		_ds -> read_bytes(bufData, 9);                 // чтение памяти датчика, 9 байтов
		if (OneWire::crc8(bufData, 8) == bufData[8])  // проверка CRC
		{
			//Serial.print("+");      // типа всё хорошо!
			owTemp = (float) ((int) bufData[0] | (((int) bufData[1]) << 8)) * 0.0625; // ХЗ откуда стащил формулу
		} else
		{
			//Serial.print("*");
			owTemp = -11;           // ошибка CRC, вернем -91
		}
	}
	else
	{
		//Serial.print("-");        // датчик есть и готов, но не отдал температуру, вернем -92,
		owTemp = -22;             // короче, наверное такой косяк тоже может быть, надо разбираться
	}
	return owTemp;
}
/*
void DSThermometer::initrequest()
{
	initOW();                             // и пробуем инициализировать
	if (Connected) requestOW();
	return;
}
*/
void DSThermometer::requestOW()
{
	_ds -> reset();
	_ds -> write(0xCC);
	_ds -> write(0x44, Parasite);
	dsMillis = millis();
#ifdef DEBUG3
	Serial.print("reqOW-stop-");
	Serial.println(millis());
#endif
	return;
}

void DSThermometer::initOW()
{
	byte addr[8];
	//unsigned long msNow = millis();
	_ds -> reset_search();
	Connected = (_ds -> search(addr));
	// --- check for Parasite Power
	_ds -> reset();	
	_ds -> write(0xCC);
	_ds -> write(0xB4);
	if (_ds->read_bit() == 0) 
	{ 
		Parasite = true;
	} else { 
		Parasite = false; 
	}
	//_ds -> reset_search();
	dsMillis = millis();
	//requestOW();
#ifdef DEBUG4
	Serial.print("initOW-stop-");
	Serial.println(millis());
#endif
	return;
}

void DSThermometer::setHiResolution()
{
	// --- Setup 12-bit resolution
	_ds -> reset();
	_ds -> write(0xCC);  // No address - only one DS on line
	_ds -> write(0x4E);  // Write scratchpad command
	_ds -> write(0);     // TL data
	_ds -> write(0);     // TH data
	_ds -> write(0x7F); // Configuration Register (resolution) 7F=12bits 5F=11bits 3F=10bits 1F=9bits
	_ds -> reset();      // This "reset" sequence is mandatory
	_ds -> write(0xCC);
	_ds -> write(0x48, Parasite);  // Copy Scratchpad command
	delay(20); 	// added 20ms delay to allow 10ms long EEPROM write operation (DallasTemperature)
	if (Parasite) delay(10); // 10ms delay
	_ds -> reset();
}


