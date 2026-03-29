/*****************************************************************************
 * | File      	 :   DrawData.h
 * | Author      :   Manfred Radmacher
 * | Function    :   
 * | Info        :
 *                   
 *----------------
 * |This version :   V0.1
 * | Date        :   2026-01-11
 * | Info        :   Basic version
 *
 ******************************************************************************/
#ifndef __DrawData_H
#define __DrawData_H

void SwitchPage();
void SimulateData();
void PrintAllData();
bool CheckTouchStatus();
void DisplayField( int x, int y, double value, char *Title, char *FormatString, uint16_t TextColor, uint16_t BackGroundColor, int DoInitPage);
void DrawFooter();
void DisplayPage(int DoInitPage);
void InitPage(bool DrawLines);
void InitBackLightControl();

void ParseNMEA0183Package(char *packet);

#include <N2kMessages.h>

#define MaxSatellites 30
extern tSatelliteInfo AllSatelliteData[MaxSatellites];

typedef struct  {
  uint16_t GNSSDate, SystemDate, UTCDate;
  int16_t UTCOffset;
  double GNSSTime, SystemTime, UTCTime;  

  double  HeadingMagnetic, HeadingTrue, Deviation, Variation,  
          SOG,COG,STW,
          AWS,TWS,MaxAws,MaxTws,AWA,TWA,AWD,TWD,
          TripLog,Log,RudderPosition,WaterTemp,
          DepthBelowTransducer, DepthBelowSurface, DepthOffset,
          HDOP, PDOP, GPSFIX, GNSStype, GNSSmethod,
          Latitude, Longitude, Altitude, Pitch, Roll,
          AirPressure, AirTemperature,
          EngBattery, HouseBattery, 
          EngTemp, EngineCoolantTemp, 
          FuelSize, FuelLevel, RPM,
          YawAngle, PitchAngle, RollAngle;
  unsigned char nSatellites;
         } BoatDataStruct;

extern BoatDataStruct TheBoatData;

#endif