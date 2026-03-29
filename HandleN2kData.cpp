/*
  HandleN2kData.cpp

  by Manfred Radmacher, 2026

  inspired by  N2kDataToNMEA0183.cpp

  Copyright (c) 2015-2018 Timo Lappalainen, Kave Oy, www.kave.fi

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
  Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <N2kMessages.h>
#include <NMEA0183Messages.h>
#include <math.h>

#include "HandleN2kData.h"
#include "DrawData.h"

const double radToDeg = 180.0 / M_PI;

//*****************************************************************************
void tHandleN2kData::HandleMsg(const tN2kMsg &N2kMsg) {

  unsigned char SID;
  tN2kMagneticVariation Source;
  tN2kHeadingReference HeadingRef;
  double aux1, aux2, aux3;
  unsigned char AuxChar;
  uint16_t AuxInt;
  int8_t AuxByte;

  tN2kSpeedWaterReferenceType SpeedRef;
  tN2kHeadingReference HeadingReference;

  tN2kGNSStype GNSStype;
  tN2kGNSSmethod GNSSmethod;
  tN2kWindReference WindReference;

  tN2kFluidType FluidType;
  tN2kEngineDiscreteStatus1 EngStatus1;
  tN2kEngineDiscreteStatus2 EngStatus2;

  tN2kTimeSource TimeSource;
  // Serial.printf("Handle N2k %d \n", N2kMsg.PGN);
  switch (N2kMsg.PGN) {
    case 127250UL:
      {
        ParseN2kHeading(N2kMsg, SID, aux1, aux2, aux3, HeadingRef);
        // this needs a sanity check, for whatever reason
        if ((aux1 >= 0) && (aux1 < (2 * PI))) {
          if (HeadingRef == N2khr_true) {
            TheBoatData.HeadingTrue = aux1 * 180.0 / PI;
          } else if (HeadingRef == N2khr_magnetic) {
            TheBoatData.HeadingMagnetic = aux1 * 180.0 / PI;
          }
        }
        break;
      }
    case 127257UL:
      {
        ParseN2kAttitude(N2kMsg, SID, aux1, aux2, aux3);

        // if the sensor only sends some data, we have to check the other entries
        // against overflow
        if ((aux1 > PI) || (aux1 < -PI))
          aux1 = 0;
        if ((aux2 > PI) || (aux2 < -PI))
          aux2 = 0;
        if ((aux3 > PI) || (aux3 < -PI))
          aux3 = 0;

        TheBoatData.YawAngle = aux1;
        TheBoatData.PitchAngle = aux2;
        TheBoatData.RollAngle = aux3;

        TheBoatData.YawAngle *= 180.0 / PI;
        TheBoatData.PitchAngle *= 180.0 / PI;
        TheBoatData.RollAngle *= 180.0 / PI;

        break;
      }
    case 127258UL:
      {
        ParseN2kMagneticVariation(N2kMsg, SID, Source, DaysSince1970, TheBoatData.Variation);
        TheBoatData.Variation *= 180.0 / PI;
        break;
      }
    case 128259UL:
      {
        ParseN2kBoatSpeed(N2kMsg, SID, aux1, aux2, SpeedRef);
        if ((SpeedRef == N2kSWRT_Paddle_wheel) || (SpeedRef == N2kSWRT_Pitot_tube) || (SpeedRef == N2kSWRT_Doppler_log) || (SpeedRef == N2kSWRT_Ultra_Sound) || (SpeedRef == N2kSWRT_Electro_magnetic))
          TheBoatData.STW = aux1 * 1.852;
        break;
      };
    case 128267UL:
      {
        ParseN2kWaterDepth(N2kMsg, SID, TheBoatData.DepthBelowTransducer, TheBoatData.DepthOffset, aux1);
        TheBoatData.DepthBelowSurface = TheBoatData.DepthBelowTransducer + TheBoatData.DepthOffset;
        break;
      }
    case 129025UL:
      {
        ParseN2kPGN129025(N2kMsg, TheBoatData.Latitude, TheBoatData.Longitude);
        break;
      }
    case 129026UL:
      {
        ParseN2kCOGSOGRapid(N2kMsg, SID, HeadingReference, TheBoatData.COG, TheBoatData.SOG);
        TheBoatData.COG *= 180.0 / PI;
        TheBoatData.SOG *= 1.852;
        break;
      }
    case 129029UL:
      {
        ParseN2kGNSS(N2kMsg, SID, TheBoatData.GNSSDate, TheBoatData.GNSSTime,
                     TheBoatData.Latitude, TheBoatData.Longitude, TheBoatData.Altitude,
                     GNSStype, GNSSmethod,
                     TheBoatData.nSatellites, TheBoatData.HDOP, TheBoatData.PDOP, aux1,
                     AuxChar, GNSStype, AuxInt, aux2);
        TheBoatData.GNSStype = GNSStype;
        TheBoatData.GNSSmethod = GNSSmethod;
        break;
      }
    case 130306UL:
      {
        ParseN2kWindSpeed(N2kMsg, SID, aux1, aux2, WindReference);
        if (WindReference == N2kWind_Apparent) {
          TheBoatData.AWS = aux1 * 1.852;
          aux2 *= 180.0 / PI;
          if (aux2 > 180.0)
            aux2 -= 360;
          TheBoatData.AWA = aux2;
        } else if (WindReference == N2kWind_True_water) {
          TheBoatData.TWS = aux1 * 1.852;
          aux2 *= 180.0 / PI;
          if (aux2 > 180.0)
            aux2 -= 360;
          TheBoatData.TWA = aux2;
        }
        break;
      }
    case 130310UL:
      {
        ParseN2kPGN130310(N2kMsg, SID, aux1, aux2, aux3);
        if (aux1 > 0)
          TheBoatData.WaterTemp = aux1 - 273, 15;
        if (aux2 > 0)
          TheBoatData.AirTemperature = aux2 - 273, 15;
        if (aux3 > 0)
          TheBoatData.AirPressure = aux3;
        break;
      }
    case 127488UL:
      {
        ParseN2kEngineParamRapid(N2kMsg, AuxChar, TheBoatData.RPM, aux2, AuxByte);
        break;
      }
    case 127505UL:
      {
        ParseN2kFluidLevel(N2kMsg, AuxChar, FluidType, aux1, aux2);
        if (FluidType == FluidType) {
          TheBoatData.FuelSize = aux2;
          TheBoatData.FuelLevel = aux1 * TheBoatData.FuelSize / 100.0;
        }
        break;
      }
    case 127489UL:
      {
        ParseN2kEngineDynamicParam(N2kMsg, AuxChar, aux3, aux3, aux1, aux2,
                                   aux3, aux3, aux3, aux3,
                                   AuxByte, AuxByte, EngStatus1, EngStatus2);
        if (AuxChar == 1) {
          TheBoatData.EngTemp = aux1 - 273.15;
          TheBoatData.EngBattery = aux2;
        } else if (AuxChar == 0) {
          TheBoatData.EngineCoolantTemp = aux1 - 273.15;
          TheBoatData.HouseBattery = aux2;
        }
        break;
      }

    case 126992UL:
      {
        ParseN2kSystemTime(N2kMsg, SID, TheBoatData.SystemDate, TheBoatData.SystemTime, TimeSource);
        break;
      }
    case 129033UL:
      {
        ParseN2kLocalOffset(N2kMsg, TheBoatData.UTCDate, TheBoatData.UTCTime, TheBoatData.UTCOffset);
        break;
      }
    case 129540UL:
      {
        // not generated by AIS
        uint8_t TheSatellite;
        tSatelliteInfo SatelliteInfo;

        ParseN2kPGN129540(N2kMsg, TheSatellite, SatelliteInfo);
        Serial.printf("Parse nsats %3d, %d, %5.3f %5.3f %5.3f %d\n", TheSatellite, SatelliteInfo.PRN, SatelliteInfo.Elevation,
                      SatelliteInfo.Azimuth, SatelliteInfo.SNR, SatelliteInfo.UsageStatus);
        if (TheSatellite < MaxSatellites) {
          AllSatelliteData[TheSatellite].PRN = SatelliteInfo.PRN;
          AllSatelliteData[TheSatellite].Elevation = SatelliteInfo.Elevation;
          AllSatelliteData[TheSatellite].Azimuth = SatelliteInfo.Azimuth;
          AllSatelliteData[TheSatellite].SNR = SatelliteInfo.SNR;
          AllSatelliteData[TheSatellite].RangeResiduals = SatelliteInfo.RangeResiduals;
          AllSatelliteData[TheSatellite].UsageStatus = SatelliteInfo.UsageStatus;
          Serial.printf("Parse nsats %3d %4d %5.3f %5.3f %5.3f %d\n", TheSatellite, AllSatelliteData[TheSatellite].PRN, AllSatelliteData[TheSatellite].Elevation,
                        AllSatelliteData[TheSatellite].Azimuth, AllSatelliteData[TheSatellite].SNR, AllSatelliteData[TheSatellite].UsageStatus);
        }
      };  // GNSS Satelittes in view

      // sentences not decoded
    case 59392UL: break;   // ISO Acknowledgement
    case 59904UL: break;   // Request for Address Claimed)
    case 60928UL: break;   // ISO address claim
    case 65359UL: break;   // ??
    case 65379UL: break;   // ??
    case 65360UL: break;   // ??
    case 126464UL: break;  // Transmit and Receive PGN List Group Function

    case 126720UL: break;  // Airmar pitch and roll offset
    case 126993UL: break;  // Heartbeat
    case 126996UL: break;  // product info
    case 126998UL: break;  // Configuration information

    case 127508UL: break;  // battery
    case 127245UL: break;  // Rudder
    case 128275UL: break;  // Logge

    case 129038UL: break;  // ??
    case 129039UL: break;  // AIS class B
    case 129041UL: break;  // AIS AtoN
    case 129539UL: break;  // ParseN2kGNSSDOPData not needed here
    case 129794UL: break;  // AIS static data class A
    case 129797UL: break;  // AIS binary broadcast
    case 129809UL: break;  // AIS static data class B part A
    case 129810UL: break;  // AIS static data class B part A
    case 130311UL: break;  // Environmental Parameters - DEPRECATED
    case 130312UL: break;  // Temperature - DEPRECATED
    case 130314UL: break;  // Pressure
    case 130316UL: break;  // Temperature
    case 130577UL: break;  // direction data
    default:
      {
        Serial.printf("HandleMsg UNKNOWN %d %d %d %d\n", N2kMsg.PGN, N2kMsg.DataLen, N2kMsg.Source, N2kMsg.Priority);
        break;
      }
  }
}
