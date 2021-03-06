/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION
 * 
 * Dust Sensor for SamYoung DSM501
 *   connect the sensor as follows :
 *        Pin 2 of dust sensor PM1      -> Digital 3 (PMW)
 *	  Pin 3 of dust sensor          -> +5V 
 * 	  Pin 4 of dust sensor PM2.5    -> Digital 6 (PWM) 
 * 	  Pin 5 of dust sensor          -> Ground
 * Datasheet: http://www.samyoungsnc.com/products/3-1%20Specification%20DSM501.pdf
* Contributor: epierre
**/
  

#include <MySensor.h>  
#include <SPI.h>

#define CHILD_ID_DUST_PM10            0
#define CHILD_ID_DUST_PM25            1
#define DUST_SENSOR_DIGITAL_PIN_PM10  3
#define DUST_SENSOR_DIGITAL_PIN_PM25  6

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUSTPM25 =0.0;
float lastDUSTPM25 =0.0;
float valDUSTPM10 =0.0;
float lastDUSTPM10 =0.0;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
long concentrationPM25 = 0;
long concentrationPM10 = 0;
int temp=20; //external temperature, if you can replace this with a DHT11 or better 
float ppmv;

MySensor gw;
MyMessage dustMsgPM10(CHILD_ID_DUST_PM10, V_LEVEL);
MyMessage msgPM10(CHILD_ID_DUST_PM10, V_UNIT_PREFIX);
MyMessage dustMsgPM25(CHILD_ID_DUST_PM25, V_LEVEL);
MyMessage msgPM25(CHILD_ID_DUST_PM25, V_UNIT_PREFIX);

void setup()  
{
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Dust Sensor DSM501", "1.5");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_DUST_PM10, S_DUST);  
  gw.send(msgPM10.set("ppm"));
  gw.present(CHILD_ID_DUST_PM25, S_DUST);  
  gw.send(msgPM25.set("ppm"));
  
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM10,INPUT);
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM25,INPUT);
  
}

void loop()      
{    

  //get PM 2.5 density of particles over 2.5 ??m.
  concentrationPM25=(long)getPM(DUST_SENSOR_DIGITAL_PIN_PM25);
  Serial.print("PM25: ");
  Serial.println(concentrationPM25);
  Serial.print("\n");
  //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
  //0.08205   = Universal gas constant in atm??m3/(kmol??K)
  ppmv=(float)(((concentrationPM25*0.0283168)/100) *  ((0.08205*temp)/0.01))/1000;
  

  if ((concentrationPM25 != lastDUSTPM25)&&(concentrationPM25>0)) {
      gw.send(dustMsgPM25.set(ppmv,3));
      lastDUSTPM25 = ceil(concentrationPM25);
  }
 //get PM 1.0 - density of particles over 1 ??m.
  concentrationPM10=getPM(DUST_SENSOR_DIGITAL_PIN_PM10);
  Serial.print("PM10: ");
  Serial.println(concentrationPM10);
  Serial.print("\n");
  //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
  //0.08205   = Universal gas constant in atm??m3/(kmol??K)
  ppmv=(((concentrationPM10*0.0283168/100) *  (0.08205*temp)/0.01))/1000;
  
  if ((ceil(concentrationPM10) != lastDUSTPM10)&&((long)concentrationPM10>0)) {
      gw.send(dustMsgPM10.set(ppmv,3));
      lastDUSTPM10 = ceil(concentrationPM10);
  }
 
  //sleep to save on radio
  gw.sleep(SLEEP_TIME);
  
}


long getPM(int DUST_SENSOR_DIGITAL_PIN) {

  starttime = millis();

  while (1) {
  
	  duration = pulseIn(DUST_SENSOR_DIGITAL_PIN, LOW);
	  lowpulseoccupancy += duration;
	  endtime = millis();
	  
	  if ((endtime-starttime) > sampletime_ms)
	  {
		ratio = (lowpulseoccupancy-endtime+starttime)/(sampletime_ms*10.0);  // Integer percentage 0=>100
		long concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
		//Serial.print("lowpulseoccupancy:");
		//Serial.print(lowpulseoccupancy);
		//Serial.print("\n");
		//Serial.print("ratio:");
		//Serial.print(ratio);
		//Serial.print("\n");
		//Serial.print("DSM501A:");
		//Serial.println(concentration);
		//Serial.print("\n");
		
		lowpulseoccupancy = 0;
		return(concentration);	  
	  }
  }  
}
