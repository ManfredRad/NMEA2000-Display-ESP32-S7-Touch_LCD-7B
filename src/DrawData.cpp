/*****************************************************************************
* | File        :   DrawData.c
* | Author      :   Manfred Radmacher
* | Function    :   
* | Info        :
*----------------
* |	This version:   V0.1
* | Date        :   2026-01-11
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/

// ###############################################################
//
// display one datfield
//
// ###############################################################

#include "Arduino.h"
#include "time.h"
#include "rgb_lcd_port.h"  // Header for Waveshare RGB LCD driver
#include "gui_paint.h"     // Header for graphical drawing functions
#include "gt911.h"         // Header for touch screen operations (GT911)

#include "string.h"  // std::string
#include "Fonts.h"

#include "DrawData.h"
extern UBYTE *BlackImage;

// ###############################################################
//
// constants for drawing
//
// ###############################################################

const int HOWMANYPAGES = 6;

const int PAGE1 = 1;
const int PAGE2 = 2;
const int PAGE3 = 3;
const int PAGE4 = 4;
const int PAGE5 = 5;
const int PAGE6 = 6;

int WhichPage = PAGE1;

const int FooterHeight = 90;
const int HOWMANYBUTTONS = HOWMANYPAGES + 2;

const int HowmanyRows = 2;
const int HowManyColumns = 3;

const int TextX0 = 30;
const int TextY0 = 90;
const int TextDeltaY = 120;

#include "pins_arduino.h"
#include "esp32-hal.h"
#include "esp32-hal-gpio.h"
#include "soc/soc_caps.h"
#include "driver/gpio.h"

#define LED_GPIO_PIN GPIO_NUM_6  // GPIO pin connected to the LED

BoatDataStruct TheBoatData = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// #include <N2kMessages.h>
// tSatelliteInfo AllSatelliteData[MaxSatellites];
tSatelliteInfo AllSatelliteData[MaxSatellites];

void SimulateData() {

  TheBoatData.HeadingTrue = TheBoatData.HeadingMagnetic - 2;
  TheBoatData.HeadingMagnetic = TheBoatData.HeadingMagnetic + 3;
  TheBoatData.STW = 5 + random(1000) / 1000;

  TheBoatData.COG = TheBoatData.HeadingTrue + 12;
  TheBoatData.SOG = TheBoatData.STW - 0.5;

  TheBoatData.AWA = -45.0 + random(1000) / 100;
  TheBoatData.AWS = 17.0 + random(1000) / 200;
  TheBoatData.TWA = TheBoatData.AWA - 10;
  TheBoatData.TWS = TheBoatData.AWS + 5;

  TheBoatData.Latitude = -234589;
  TheBoatData.Longitude = -82634;

  TheBoatData.SystemTime = 0;
  TheBoatData.SystemDate = 0;
  TheBoatData.GNSSTime = 0;
  TheBoatData.GNSSDate = 0;
  TheBoatData.nSatellites = 3 + round(random(1000) / 500);
  TheBoatData.HDOP = 7 + round(random(1000) / 200);
  TheBoatData.PDOP = 23 + round(random(1000) / 200);
  TheBoatData.GPSFIX = 1;

  TheBoatData.Variation = 3 + round(random(1000) / 500);


  TheBoatData.WaterTemp = 20 + round(random(1000) / 200);

  TheBoatData.DepthOffset = 0.4;
  TheBoatData.DepthBelowTransducer = 22 + round(random(1000) / 200);                                  // depth below transducer
  TheBoatData.DepthBelowSurface = TheBoatData.DepthBelowTransducer = 22 + round(random(1000) / 200);  // depth below transducer
  +TheBoatData.DepthOffset;                                                                           // depth below surface

  TheBoatData.Pitch = 3 + round(random(1000) / 500);
  TheBoatData.Roll = 5 + round(random(1000) / 500);

  TheBoatData.AirPressure = 1000 + round(random(1000) / 50);
}

void PrintAllData() {

  Serial.println("");
  Serial.println("###################################################################");
  Serial.printf("SOG COG \t%4.2f %6.1f\n", TheBoatData.SOG, TheBoatData.COG);
  Serial.printf("STW  \t\t%4.2f \n", TheBoatData.STW);
  Serial.printf("HDT HDM \t%4.1f %4.1f %4.1f\n", TheBoatData.HeadingTrue, TheBoatData.HeadingMagnetic, TheBoatData.Variation);

  Serial.printf("AWA AWS \t %4.1f %4.2f\n", TheBoatData.AWA, TheBoatData.AWS);
  Serial.printf("TWA TWS \t %4.1f %4.2f\n", TheBoatData.TWA, TheBoatData.TWS);

  Serial.printf("LAT LON \t %8.4f %8.4f\n", TheBoatData.Latitude, TheBoatData.Longitude);
  Serial.printf("nSat HDOP PDOP\t %4d %6.1f  %6.1f\n", TheBoatData.GPSFIX, TheBoatData.nSatellites, TheBoatData.HDOP, TheBoatData.PDOP);
  Serial.printf("GNSS Time Date \t%6.0f %6.0d\n", TheBoatData.GNSSTime, TheBoatData.GNSSDate);
  Serial.printf("System Time Date \t%6.0f %6.0d\n", TheBoatData.SystemTime, TheBoatData.SystemDate);
  Serial.printf("UTC Time Date \t%6.0f %6.0d %d\n", TheBoatData.UTCTime, TheBoatData.UTCDate, TheBoatData.UTCOffset);

  Serial.printf("DBT DBS Offset\t %6.2f %6.2f %6.2f\n", TheBoatData.DepthBelowTransducer, TheBoatData.DepthBelowSurface, TheBoatData.DepthOffset);

  Serial.printf("Pitch Roll \t %5.2f %5.2f\n", TheBoatData.Pitch, TheBoatData.Roll);

  Serial.printf("AirP AirT WaterT  \t %6.2f %5.2f %5.2f\n", TheBoatData.AirPressure, TheBoatData.AirTemperature, TheBoatData.WaterTemp);

  Serial.printf("Battery E & H \t %4.2f %4.2f\n", TheBoatData.EngBattery, TheBoatData.HouseBattery);
  Serial.printf("Temp E & C \t %5.2f %5.2f\n", TheBoatData.EngTemp, TheBoatData.EngineCoolantTemp);
  Serial.printf("Fuel S & L \t %4.2f %4.2f\n", TheBoatData.FuelSize, TheBoatData.FuelLevel);

  Serial.printf("RPM S & L \t %6.2f\n", TheBoatData.RPM);
}

void InitBackLightControl() {
  // Initialize GPIO pin for PWM output
//  pinMode(LED_GPIO_PIN, OUTPUT);

// Set initial LED brightness (0 = full brightness due to active-low)
//  analogWrite(LED_GPIO_PIN, 0);
  IO_EXTENSION_Pwm_Output(0);  // Optional external control for brightness
}

// TheValue should be between 0 and 255
void SetBackLightIntensity(int TheValue) {
  if ( TheValue > 100 )
    TheValue = 100;
  if ( TheValue < 0 ) 
    TheValue = 0;
  TheValue = 100 - TheValue;

//  analogWrite(LED_GPIO_PIN, TheValue);  // Scale 0–100 to 0–255
  IO_EXTENSION_Pwm_Output(TheValue);    // Optional external PWM controller
}

void ParseXDRSentence(char *TheSentence) {

  char *p1;
  char *p2;
  char *p3;
  char *p4;
  char *p5;

  p1 = strstr(TheSentence, ",");  // Transducer Type
  p2 = strstr(p1 + 1, ",");       // data
  p3 = strstr(p2 + 1, ",");       // Units
  p4 = strstr(p3 + 1, ",");       // Name
  p5 = strstr(p4 + 1, ",");       // start of data quadruplet

  int MoreData = 1;
  char *PointerNext;
  float TheValue;

  do {
    // auxstring is the data descriptor
    TheValue = strtof(p2 + 1, &PointerNext);

    // aux1 is the data value
    if (p1[1] == 'P')  // pressure data
    {
      if (strstr(p4 + 1, "Barometer") != NULL)
        TheBoatData.AirPressure = TheValue;
      else {
        Serial.print("Unknown XDR Name of transducer: ");
        Serial.println(TheSentence);
      }
    } else if (p1[1] == 'A') {  // Pressure
      if (strstr(p4 + 1, "Press") != NULL)
        TheBoatData.AirPressure = TheValue;
      else {
        Serial.print("Unknown XDR Name of transducer: ");
        Serial.println(TheSentence);
      }
    } else if (p1[1] == 'E') {  // Fuel Level
      if (strstr(p4 + 1, "Fuel") != NULL)
        TheBoatData.FuelLevel = TheValue;
      else if (strstr(p4 + 1, "Tank") != NULL)
        TheBoatData.FuelSize = TheValue;
      else {
        Serial.print("Unknown XDR Name of transducer: ");
        Serial.println(TheSentence);
      }
    } else if (p1[1] == 'C')  // Temperature
    {
      if (strstr(p4 + 1, "PortEng") != NULL)
        TheBoatData.EngTemp = TheValue;
      else if (strstr(p4 + 1, "StarBEng") != NULL)
        TheBoatData.EngineCoolantTemp = TheValue;
      else {
        Serial.print("Unknown XDR Name of transducer: ");
        Serial.println(TheSentence);
      }
    } else if (p1[1] == 'U')  // Voltage
    {
      if (strstr(p4 + 1, "PortBatt") != NULL)
        TheBoatData.HouseBattery = TheValue;
      else if (strstr(p4 + 1, "StarBBatt") != NULL)
        TheBoatData.EngBattery = TheValue;
      else {
        Serial.print("Unknown XDR Name of transducer: ");
        Serial.println(TheSentence);
      }
    } else {
      Serial.print("Unknown XDR Transducer Type: ");
      Serial.println(TheSentence);
    }

    if (p5 == NULL)
      MoreData = 0;
    else {
      p1 = p5;  // Transducer Type

      if (p1 == NULL)
        MoreData = 0;
      else {
        p2 = strstr(p1 + 1, ",");  // data
        if (p2 == NULL)
          MoreData = 0;
        else {
          p3 = strstr(p2 + 1, ",");  // data
          if (p3 == NULL)
            MoreData = 0;
          else {
            p4 = strstr(p3 + 1, ",");
            if (p4 == NULL)
              MoreData = 0;
            else {
              p5 = strstr(p4 + 1, ",");
            }
          }
        }
      }
    }
  } while (MoreData == 1);
}

void ParseNMEA0183Package(char *packet) {
  char WhichDataType[100];
  char AuxString[100];
  char *PointerNext;

  float aux1;
  float aux2;

  bool b1;
  bool b2;

  strncpy(WhichDataType, packet + 3, 3);
  WhichDataType[3] = '\0';
  strncpy(AuxString, packet + 7, strlen(packet) - 7);
  AuxString[strlen(packet) - 7] = '\0';
  //     Serial.print(packet);

  // Heading true
  if (strcmp(WhichDataType, "DBT") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    aux1 = strtof(PointerNext + 3, &PointerNext);
    TheBoatData.DepthBelowTransducer = aux1;
  } else if (strcmp(WhichDataType, "DPT") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    TheBoatData.DepthBelowTransducer = aux1;
    aux2 = strtof(PointerNext + 1, &PointerNext);
    TheBoatData.DepthOffset = aux2;
  } else if (strcmp(WhichDataType, "DBS") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    aux1 = strtof(PointerNext + 3, &PointerNext);
    TheBoatData.DepthBelowSurface = aux1;
  }

  // position
  else if (strcmp(WhichDataType, "GLL") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    if (PointerNext[1] == 'S')
      aux1 *= -1;
    TheBoatData.Latitude = aux1;

    aux2 = strtof(PointerNext + 3, &PointerNext);
    if (PointerNext[1] == 'E')
      aux2 *= -1;
    TheBoatData.Longitude = aux2;
    aux2 = strtof(PointerNext + 3, &PointerNext);
    TheBoatData.GNSSTime = aux2;
  }

  // position
  else if (strcmp(WhichDataType, "GGA") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    TheBoatData.GNSSTime = aux1;

    aux2 = strtof(PointerNext + 1, &PointerNext);
    if (PointerNext[1] == 'S')
      aux2 *= -1;
    TheBoatData.Latitude = aux2;

    aux2 = strtof(PointerNext + 3, &PointerNext);
    if (PointerNext[1] == 'E')
      aux2 *= -1;
    TheBoatData.Longitude = aux2;

    aux2 = strtof(PointerNext + 3, &PointerNext);

    TheBoatData.GPSFIX = aux2;

    aux2 = strtof(PointerNext + 1, &PointerNext);
    TheBoatData.nSatellites = aux2;

    aux2 = strtof(PointerNext + 1, &PointerNext);
    TheBoatData.HDOP = aux2;
  }

  // Heading
  else if (strcmp(WhichDataType, "HDG") == 0) {
    TheBoatData.HeadingMagnetic = strtof(AuxString, &PointerNext);
  }

  // Heading magnetic
  else if (strcmp(WhichDataType, "HDM") == 0) {
    TheBoatData.HeadingMagnetic = strtof(AuxString, &PointerNext);
  } else if (strcmp(WhichDataType, "HDT") == 0) {
    TheBoatData.HeadingTrue = strtof(AuxString, &PointerNext);
  } else if (strcmp(WhichDataType, "RPM") == 0) {
    aux1 = strtof(AuxString + 4, &PointerNext);
    TheBoatData.RPM = aux1;
  } else if (strcmp(WhichDataType, "MMB") == 0) {
    aux1 = strtof(AuxString + 4, &PointerNext);
    aux1 = strtof(PointerNext + 3, &PointerNext);
    TheBoatData.AirPressure = aux1;
  }
  // wind true or apparent
  else if (strcmp(WhichDataType, "MWV") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    if (PointerNext[1] == 'T')
      b1 = true;
    else
      b1 = false;

    // units are meters ore knots
    aux2 = strtof(PointerNext + 3, &PointerNext);
    if (PointerNext[1] == 'M') {
      aux2 *= 1.852;
    }
    if (aux1 > 180.0)
      aux1 -= 360;

    if (b1) {
      TheBoatData.TWA = aux1;
      TheBoatData.TWS = aux2;
    } else {
      TheBoatData.AWA = aux1;
      TheBoatData.AWS = aux2;
    }
  }

  else if (strcmp(WhichDataType, "RMC") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    TheBoatData.GNSSTime = aux1;

    aux2 = strtof(PointerNext + 3, &PointerNext);  // ddmm.ccc
    aux1 = trunc(aux2 / 100);                      // degrees
    aux2 -= aux1 * 100;                            // just the minutes plus decimal minutes
    TheBoatData.Latitude = aux1;                   // we have degrees already
    aux1 = trunc(aux2 / 100);                      // this is the minutes
    aux2 -= aux1 * 100;                            // this is the decimal minutes
    TheBoatData.Latitude += aux1 / 60;
    TheBoatData.Latitude += aux2 / 60;

    if (PointerNext[1] == 'S')
      TheBoatData.Latitude *= -1;

    aux2 = strtof(PointerNext + 3, &PointerNext);  // dddmm.ccc
    aux1 = trunc(aux2 / 100);                      // degrees
    aux2 -= aux1 * 100;                            // just the minutes plus decimal minutes
    TheBoatData.Longitude = aux1;                  // we have degrees already
    aux1 = trunc(aux2 / 100);                      // this is the minutes
    aux2 -= aux1 * 100;                            // this is the decimal minutes
    TheBoatData.Longitude += aux1 / 60;
    TheBoatData.Longitude += aux2 / 60;

    TheBoatData.SOG = aux2;

    aux2 = strtof(PointerNext + 1, &PointerNext);
    TheBoatData.COG = aux2;

    aux2 = strtof(PointerNext + 1, &PointerNext);
    TheBoatData.GNSSDate = aux2;

    aux2 = strtof(PointerNext + 1, &PointerNext);

    if (PointerNext[1] == 'E')
      aux2 *= -1;
    TheBoatData.Variation = aux2;
  }

  else if (strcmp(WhichDataType, "RPM") == 0) {
    aux1 = strtof(AuxString + 4, &PointerNext);
    TheBoatData.RPM = aux1;
  }

  // water speed and heading
  else if (strcmp(WhichDataType, "VHW") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    TheBoatData.HeadingMagnetic = aux1;

    aux2 = strtof(PointerNext + 3, &PointerNext);
    aux2 = strtof(PointerNext + 3, &PointerNext);
    TheBoatData.STW = aux2;
  } else if (strcmp(WhichDataType, "MTW") == 0) {
    aux1 = strtof(AuxString, &PointerNext);
    TheBoatData.WaterTemp = aux1;
  }

  else if (strcmp(WhichDataType, "XDR") == 0)
    ParseXDRSentence(packet);
  else {
    Serial.print("Unknown: ");
    Serial.print(packet);
  }
}

// ###############################################################
//
// check status of touch screen
//
// returns true if refresh is needed (i.e. Page has changed)
//
// ###############################################################

bool CheckTouchStatus() {

  static float BackLightIntensity = 100;

  touch_gt911_point_t point_data;  // Structure to store touch point data
  point_data = touch_gt911_read_point(ESP_LCD_TOUCH_MAX_POINTS);
  int WhichButton = -1;

  int OldPage = WhichPage;
  for (int i = 0; i < point_data.cnt; i++) {
    if (point_data.y[i] >= EXAMPLE_LCD_V_RES - FooterHeight) {
      WhichButton = point_data.x[i] / (EXAMPLE_LCD_H_RES / HOWMANYBUTTONS);
      if (WhichButton == 0) {
        BackLightIntensity *= 0.9;

        if (BackLightIntensity < 4)
          BackLightIntensity = 4;

        SetBackLightIntensity((int)BackLightIntensity);
      } else if (WhichButton <= HOWMANYPAGES)
        WhichPage = WhichButton;
      else if (WhichButton == (HOWMANYPAGES + 1)) {
        BackLightIntensity *= 1.1;

        if (BackLightIntensity >= 100)
          BackLightIntensity = 100;

        SetBackLightIntensity((int)BackLightIntensity);
      }
    }
  }

  if (OldPage != WhichPage) {
    return (true);
  } else {
    return (false);
  }
}


void Paint_FillRect(int16_t x0, int16_t y0, int16_t xend, int16_t yend, UWORD BackGroundColor) {

  int16_t TheLine;
  int16_t TheColumn;
  for (TheLine = y0; TheLine < yend; TheLine++)
    for (TheColumn = x0; TheColumn < xend; TheColumn++) {
      UDOUBLE Addr = (TheColumn * 2) + (TheLine * Paint.WidthByte * 2);
      Paint.Image[Addr + 1] = 0xff & (BackGroundColor >> 8);
      Paint.Image[Addr] = 0xff & BackGroundColor;
    }
}

// ###############################################################
//
// display Latitude
//
// lat is in degrees with decimals of degrees
// needs to be converted in deg, min and fractions of minutes
//
// ###############################################################

void DisplayLAT(int x, int y, double value, char *Title, char *FormatString, uint16_t TextColor, uint16_t BackGroundColor, int DoInitPage) {

  int x0;
  int y0;
  int DeltaX;
  int DeltaY;

  char OutText[20];
  char ExtraText[5];

  DeltaX = EXAMPLE_LCD_H_RES / HowManyColumns;
  DeltaY = (EXAMPLE_LCD_V_RES - FooterHeight) / HowmanyRows;
  x0 = x * DeltaX;
  y0 = y * DeltaY;

  float TheDegrees;
  float TheMinutes;
  float TheDecimals;

  TheDegrees = trunc(value);
  TheMinutes = trunc((value - TheDegrees) * 60);
  TheDecimals = (value - TheDegrees - TheMinutes / 60) * 60;
  TheMinutes += TheDecimals;

  if (TheDegrees < 0) {
    TheDegrees *= -1;
    value *= -1;
    strcpy(ExtraText, "S");
  } else
    strcpy(ExtraText, "N");

  Paint_FillRect(x0 + 5, y0 + 5, x0 + DeltaX - 12, y0 + DeltaY - 12, BackGroundColor);

  sprintf(OutText, "%02.0f %s", TheDegrees, ExtraText);
  Paint_DrawString(x0 + TextX0, y0 + TextY0, OutText, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);

  sprintf(OutText, "%6.3f", TheMinutes);
  Paint_DrawString(x0 + TextX0, y0 + TextY0 + TextDeltaY, OutText, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);
}

// ###############################################################
//
// display longitude
//
// lat is in degrees with decimals of degrees
// needs to be converted in deg, min and fractions of minutes
//
// ###############################################################

void DisplayLON(int x, int y, double value, char *Title, char *FormatString, uint16_t TextColor, uint16_t BackGroundColor, int DoInitPage) {

  int x0;
  int y0;
  int DeltaX;
  int DeltaY;

  char OutText[20];
  char ExtraText[5];

  DeltaX = EXAMPLE_LCD_H_RES / HowManyColumns;
  DeltaY = (EXAMPLE_LCD_V_RES - FooterHeight) / HowmanyRows;
  x0 = x * DeltaX;
  y0 = y * DeltaY;

  float TheDegrees;
  float TheMinutes;
  float TheDecimals;

  TheDegrees = trunc(value);
  TheMinutes = trunc((value - TheDegrees) * 60);
  TheDecimals = (value - TheDegrees - TheMinutes / 60) * 60;
  TheMinutes += TheDecimals;

  if (TheDegrees < 0) {
    TheDegrees *= -1;
    value *= -1;
    strcpy(ExtraText, "E");
  } else
    strcpy(ExtraText, "W");

  Paint_FillRect(x0 + 5, y0 + 5, x0 + DeltaX - 12, y0 + DeltaY - 12, BackGroundColor);

  // Paint_DrawRectangle(x0, y0, x0 + DeltaX, y0 + DeltaY, BackGroundColor, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  sprintf(OutText, "%03.0f %s", TheDegrees, ExtraText);
  Paint_DrawString(x0 + TextX0, y0 + TextY0, OutText, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);

  sprintf(OutText, "%6.3f", TheMinutes);
  Paint_DrawString(x0 + TextX0, y0 + TextY0 + TextDeltaY, OutText, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);
}

// ###############################################################
//
// display time
//
// ###############################################################

void DisplayTIME(int x, int y, long int TheTime, long int TheDate, uint16_t TextColor, uint16_t BackGroundColor) {

  int x0;
  int y0;
  int DeltaX;
  int DeltaY;

  char OutText[20];
  char ExtraText[5];

  DeltaX = EXAMPLE_LCD_H_RES / HowManyColumns;
  DeltaY = (EXAMPLE_LCD_V_RES - FooterHeight) / HowmanyRows;
  x0 = x * DeltaX;
  y0 = y * DeltaY;

  int TheHour;
  int TheMinute;
  int TheSecond;

  TheHour = trunc(TheTime / 3600);
  TheMinute = trunc((TheTime - 3600 * TheHour) / 60);
  TheSecond = TheTime - 3600 * TheHour - 60 * TheMinute;

  time_t UnixTime;
  UnixTime = TheDate;
  UnixTime *= 3600 * 24;
  UnixTime += TheTime;
  struct tm *TheTimeStruct;

  TheTimeStruct = localtime(&UnixTime);

  Paint_FillRect(x0 + 5, y0 + 5, x0 + DeltaX - 12, y0 + DeltaY - 12, BackGroundColor);

  sprintf(OutText, "%02d:%02d:%02d", TheTimeStruct->tm_hour, TheTimeStruct->tm_min, TheTimeStruct->tm_sec);
  Paint_DrawString(x0 + TextX0, y0 + TextY0, OutText, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);

  sprintf(OutText, "%02d.%02d.%02d", TheTimeStruct->tm_mday, TheTimeStruct->tm_mon, TheTimeStruct->tm_year - 100);
  Paint_DrawString(x0 + TextX0, y0 + TextY0 + TextDeltaY, OutText, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);
}


// ###############################################################
//
// display a numerical value
//
// ###############################################################

void DisplayWindAngle(int x, int y, double value, char *Title, char *FormatString, uint16_t TextColor, uint16_t BackGroundColor, int DoInitPage) {

  int x0;
  int y0;
  int DeltaX;
  int DeltaY;
  static int OldWhichSide = 0;

  char OutText[20];

  DeltaX = EXAMPLE_LCD_H_RES / HowManyColumns;
  DeltaY = (EXAMPLE_LCD_V_RES - FooterHeight) / HowmanyRows;
  x0 = x * DeltaX;
  y0 = y * DeltaY;

  int WhichSide;
  if (value < 0) {
    value *= -1;
    WhichSide = 0;
  } else
    WhichSide = 1;

  sprintf(OutText, FormatString, value);
  if ((OldWhichSide != WhichSide) || (DoInitPage == 1)) {
    OldWhichSide = WhichSide;
    Paint_FillRect(x0 + TextX0, y0 + TextY0 - 60, x0 + DeltaX - 12, y0 + TextY0 + 14, BackGroundColor);

    if (WhichSide == 0) {
      Paint_DrawCircle(x0 + 0.2 * DeltaX, y0 + TextY0 - 0.2 * TextDeltaY, 0.3 * TextDeltaY, RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      Paint_DrawString(x0 + TextX0 + 0.3 * DeltaX, y0 + TextY0, Title, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);
    } else {
      Paint_DrawCircle(x0 + 0.8 * DeltaX, y0 + TextY0 - 0.2 * TextDeltaY, 0.3 * TextDeltaY, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      Paint_DrawString(x0 + TextX0, y0 + TextY0, Title, &FreeSansBold36pt7b, TextColor, BackGroundColor, TRANSPARENT);
    }
  }

  Paint_FillRect(x0 + TextX0, y0 + TextY0 + TextDeltaY - 86, x0 + DeltaX - 12, y0 + TextY0 + TextDeltaY + 1, BackGroundColor);

  if (WhichSide == 0) {
    Paint_DrawString(x0 + TextX0, y0 + TextY0 + TextDeltaY - 3, OutText, &FreeSansBold56pt7b, RED, BackGroundColor, TRANSPARENT);
  } else {
    Paint_DrawString(x0 + TextX0, y0 + TextY0 + TextDeltaY - 3, OutText, &FreeSansBold56pt7b, GREEN, BackGroundColor, TRANSPARENT);
  }
}

// ###############################################################
//
// display a numerical value
//
// ###############################################################

void DisplayField(int x, int y, double value, char *Title, char *FormatString, uint16_t TextColor, uint16_t BackGroundColor, int DoInitPage) {

  int x0;
  int y0;
  int DeltaX;
  int DeltaY;

  char OutText[20];

  DeltaX = EXAMPLE_LCD_H_RES / HowManyColumns;
  DeltaY = (EXAMPLE_LCD_V_RES - FooterHeight) / HowmanyRows;
  x0 = x * DeltaX;
  y0 = y * DeltaY;

  // Paint_DrawRectangle(x0, y0, x0 + DeltaX, y0 + DeltaY, BackGroundColor, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  //Paint_DrawString_EN(x0 + TextX0, y0 + TextY0, Title, &Font48, TextColor, BackGroundColor);
  if (DoInitPage == 1) {
    Paint_DrawString(x0 + TextX0, y0 + TextY0, Title, &FreeSansBold36pt7b, TextColor, BackGroundColor, WITHBACKGROUND);
  }
  sprintf(OutText, FormatString, value);

  Paint_FillRect(x0 + TextX0, y0 + TextY0 + TextDeltaY - 86, x0 + DeltaX - 12, y0 + TextY0 + TextDeltaY + 1, BackGroundColor);

  Paint_DrawString(x0 + TextX0, y0 + TextY0 + TextDeltaY - 3, OutText, &FreeSansBold56pt7b, TextColor, BackGroundColor, TRANSPARENT);
}

// ###############################################################
//
// draw the footer with buttons
//
// ###############################################################

void DrawFooter(bool DrawLines) {
  int x0;
  int y0;
  int DeltaX;
  int DeltaY;

  int ThePage;
  x0 = 0;
  y0 = EXAMPLE_LCD_V_RES - FooterHeight + 1;

  DeltaX = EXAMPLE_LCD_H_RES / HOWMANYBUTTONS;

  // draw the buttons for touch
  // draw left button for brightness

  Paint_DrawRectangle(x0, y0, x0 + DeltaX, y0 + FooterHeight - 1, DARKGRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawCircle(.5 * DeltaX, y0 + 0.5 * FooterHeight, 0.3 * FooterHeight, YELLOW, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);

  uint16_t CircleColor;
  int TheCircle;
  int TheOffset;
  int TheRadius;
  int DistanceCircle;
  int CircleXPos;
  int TheWidth;

  TheRadius = 0.2 * FooterHeight;
  DistanceCircle = 0.2 * TheRadius;

  // if too many circles (pages) we need to back down
  if ((TheRadius * 2 * HOWMANYPAGES + DistanceCircle * (HOWMANYPAGES + 1)) > DeltaX)
    TheRadius = trunc(DeltaX / HOWMANYPAGES / 2.3);

  DistanceCircle = 0.2 * TheRadius;

  for (ThePage = 1; ThePage <= HOWMANYPAGES; ThePage += 1) {
    Paint_DrawRectangle(x0 + ThePage * DeltaX, y0, x0 + (ThePage + 1) * DeltaX - 1, y0 + FooterHeight - 1, GRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(x0 + ThePage * DeltaX + 2, y0 + 4, x0 + (ThePage + 1) * DeltaX - 2, y0 + FooterHeight - 4, DARKCYAN, DOT_PIXEL_5X5, DRAW_FILL_EMPTY);

    if (ThePage == WhichPage)
      CircleColor = LIGHTGRAY;
    else
      CircleColor = BLACK;

    // total width of all circles plus spaces
    TheWidth = ThePage * 2 * TheRadius + (ThePage - 1) * DistanceCircle;

    // offset from edge
    TheOffset = (DeltaX - TheWidth) / 2;

    for (TheCircle = 0; TheCircle < ThePage; TheCircle += 1) {
      CircleXPos = x0 + ThePage * DeltaX + TheOffset + TheRadius + TheCircle * (2 * TheRadius + DistanceCircle);
      if (TheCircle >= 1)
        CircleXPos += DistanceCircle;
      Paint_DrawCircle(CircleXPos, y0 + 0.5 * FooterHeight, TheRadius, CircleColor, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    }
  }

  // draw right button for brightness
  Paint_DrawRectangle((HOWMANYBUTTONS - 1) * DeltaX + 3, y0, x0 + HOWMANYBUTTONS * DeltaX - 1, y0 + FooterHeight - 1, DARKGRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawCircle((HOWMANYBUTTONS - 1 + .5) * DeltaX, y0 + 0.5 * FooterHeight, 0.3 * FooterHeight, YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  // draw outlines for DisplayFields
  int column;
  int row;

  DeltaX = EXAMPLE_LCD_H_RES / HowManyColumns;
  DeltaY = (EXAMPLE_LCD_V_RES - FooterHeight) / HowmanyRows;

  const int HowManyRows = 2;
  const int HowManyColumns = 3;

if (DrawLines ){
  for (row = 0; row < HowManyRows; row++) {
    for (column = 0; column < HowManyColumns; column++) {
      x0 = column * DeltaX;
      y0 = row * DeltaY;
      Paint_DrawRectangle(x0 + 2, y0 + 4, x0 + DeltaX - 2, y0 + DeltaY - 4, BLACK, DOT_PIXEL_5X5, DRAW_FILL_EMPTY);
    }
   }
  }
}

void DisplaySatsInView() {
  static long LastUpdate = 0;

  int CircleRadius = (EXAMPLE_LCD_V_RES - FooterHeight + 1) * 0.9 / 2;
  int CircleY0 = (EXAMPLE_LCD_V_RES - FooterHeight + 1) / 2;
  int CircleX0 = 0.5 * EXAMPLE_LCD_H_RES;

  int x1 = 20;
  int x2 = 0.75 * EXAMPLE_LCD_H_RES;
  int y1 = 60;
  int y2 = EXAMPLE_LCD_V_RES - 50;
  int DeltaY = 70;

  float TheDegrees;
  float TheMinutes;
  float TheDecimals;

  char ExtraText[5];
  char OutText[20];

  if ((millis() - LastUpdate) > 3000) {
    LastUpdate = millis();
    InitPage(false);
    Paint_DrawCircle(CircleX0, CircleY0, CircleRadius, YELLOW, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(CircleX0, CircleY0, CircleRadius * 0.6, YELLOW, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(CircleX0, CircleY0, CircleRadius * 0.3, YELLOW, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);

    // draw circles
    //    LINE_STYLE_SOLID = 0,
    Paint_DrawLine(CircleX0 - CircleRadius, CircleY0, CircleX0 + CircleRadius, CircleY0, YELLOW, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(CircleX0, CircleY0 - CircleRadius, CircleX0, CircleY0 + CircleRadius, YELLOW, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    // draw Lon
    TheDegrees = trunc(TheBoatData.Longitude);
    TheMinutes = trunc((TheBoatData.Longitude - TheDegrees) * 60);
    TheDecimals = (TheBoatData.Longitude - TheDegrees - TheMinutes / 60) * 60;
    TheMinutes += TheDecimals;

    if (TheDegrees < 0) {
      TheDegrees *= -1;
      strcpy(ExtraText, "E");
    } else
      strcpy(ExtraText, "W");

    sprintf(OutText, "%03.0f %s", TheDegrees, ExtraText);
    Paint_DrawString(x1, y1, OutText, &FreeSansBold36pt7b, GOLD, COAL, TRANSPARENT);

    sprintf(OutText, "%6.3f", TheMinutes);
    Paint_DrawString(x1, y1 + DeltaY, OutText, &FreeSansBold36pt7b, GOLD, COAL, TRANSPARENT);

    // draw Lat
    TheDegrees = trunc(TheBoatData.Latitude);
    TheMinutes = trunc((TheBoatData.Latitude - TheDegrees) * 60);
    TheDecimals = (TheBoatData.Latitude - TheDegrees - TheMinutes / 60) * 60;
    TheMinutes += TheDecimals;

    if (TheDegrees < 0) {
      TheDegrees *= -1;
      strcpy(ExtraText, "S");
    } else
      strcpy(ExtraText, "N");

    sprintf(OutText, "%03.0f %s", TheDegrees, ExtraText);
    Paint_DrawString(x2, y1, OutText, &FreeSansBold36pt7b, GOLD, COAL, TRANSPARENT);

    sprintf(OutText, "%6.3f", TheMinutes);
    Paint_DrawString(x2, y1 + DeltaY, OutText, &FreeSansBold36pt7b, GOLD, COAL, TRANSPARENT);
   
    int HowManySatellites = (int) TheBoatData.nSatellites;
    // draw n satelittes
    Paint_DrawString(x1, y2 - 2 * DeltaY, "nSats", &FreeSansBold36pt7b, AQUA, COAL, TRANSPARENT);
    sprintf(OutText, "%3d", HowManySatellites);
    Paint_DrawString(x1, y2 - 1 * DeltaY, OutText, &FreeSansBold36pt7b, AQUA, COAL, TRANSPARENT);

    // draw HDOP/VDOP
    Paint_DrawString(x2 - 30, y2 - 2 * DeltaY, "PDOP/VDOP", &FreeSansBold24pt7b, ORANGERED, COAL, TRANSPARENT);
    sprintf(OutText, "%5.2f %5.2f", TheBoatData.PDOP, TheBoatData.HDOP);
    Paint_DrawString(x2 - 30, y2 - 1 * DeltaY, OutText, &FreeSansBold24pt7b, ORANGERED, COAL, TRANSPARENT);

    // now draw the satlittes
    uint16_t TheColor;

    double SNR;             ///< SignalToNoiseRatio of the satellite
    double RangeResiduals;  ///< Range Residual
    int SatX0, SatY0;
    double TheAngle, TheElevation, SatRadius;

    for (int i = 0; i < HowManySatellites; i++) {
      TheAngle = AllSatelliteData[i].Azimuth;
      TheElevation = AllSatelliteData[i].Elevation;
 // <Angle is between +/- 2 Pi
      TheAngle = rand() % 20001;
      TheAngle *= 2 * PI / 20000;

      TheElevation = rand() % 20001;
      TheElevation *= PI / 2 / 20000;

      SatX0 = CircleX0 + TheElevation / ( PI / 2 ) * CircleRadius * cos(TheAngle);
      SatY0 = CircleY0 + TheElevation / ( PI / 2 ) * CircleRadius * sin(TheAngle);
      SatRadius = 10;  /// 

      switch (AllSatelliteData[i].UsageStatus) {
        case (N2kDD124_NotTracked):
          TheColor = ORANGERED;
          break;
        case (N2kDD124_TrackedButNotUsedInSolution):
          TheColor = LIGHTGRAY;
          break;
        case (N2kDD124_UsedInSolutionWithoutDifferentialCorrections):
          TheColor = GREEN;
          break;
        default:
          TheColor = DARKGRAY;
          break;
      }
      Paint_DrawCircle(SatX0, SatY0, SatRadius, TheColor, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    }
  }
}

void InitPage(bool DrawLines) {
  Paint_Clear(COAL);
  DrawFooter(DrawLines);
}

// ###############################################################
//
// display a page of data
//
// ###############################################################

void DisplayPage(int DoInitPage) {

  if (WhichPage <= HOWMANYPAGES) {
    switch (WhichPage) {
      case PAGE1:  //normal sailing
        DisplayField(0, 0, TheBoatData.STW, "STW", "% 4.2f", AQUA, COAL, DoInitPage);
        DisplayField(1, 0, TheBoatData.SOG, "SOG", "% 3.1f", ORANGERED, COAL, DoInitPage);
        DisplayField(2, 0, TheBoatData.DepthBelowSurface, " DBS", "%4.2f", YELLOW, COAL, DoInitPage);

        DisplayField(0, 1, TheBoatData.HeadingMagnetic, "HDM", "%3.0f", AQUA, COAL, DoInitPage);
        DisplayField(1, 1, TheBoatData.COG, "COG", "%4.0f", ORANGERED, COAL, DoInitPage);
        DisplayWindAngle(2, 1, TheBoatData.AWA, "AWA", " %3.0f", WHITE, COAL, DoInitPage);
        break;

      case PAGE2:  // sailing with wind
        DisplayField(0, 0, TheBoatData.STW, "STW", "%4.1f", AQUA, COAL, DoInitPage);
        DisplayField(1, 0, TheBoatData.AWS, "AWS", "%4.1f", WHITE, COAL, DoInitPage);
        DisplayField(2, 0, TheBoatData.TWS, "TWS", "%4.1f", ORANGERED, COAL, DoInitPage);

        DisplayField(0, 1, TheBoatData.HeadingMagnetic, "HDM", "%3.0f", AQUA, COAL, DoInitPage);
        DisplayWindAngle(1, 1, TheBoatData.AWA, "AWA", "%3.0f", WHITE, COAL, DoInitPage);
        DisplayField(2, 1, TheBoatData.DepthBelowSurface, "DBS", "%4.2f", YELLOW, COAL, DoInitPage);
        break;

      case PAGE3:  // under engine
        DisplayField(0, 0, TheBoatData.SOG, "SOG", "%4.1f", GREEN, COAL, DoInitPage);
        DisplayField(1, 0, TheBoatData.RPM, "RPM", "%4.0f", YELLOW, COAL, DoInitPage);
        DisplayField(2, 0, TheBoatData.EngTemp, "EngT", "%3.1f", ORANGERED, COAL, DoInitPage);

        DisplayField(0, 1, TheBoatData.COG, "COG", "%4.1f", GREEN, COAL, DoInitPage);
        DisplayField(1, 1, TheBoatData.FuelSize * TheBoatData.FuelLevel / 100, "Fuel", "%3.0f l", WHITE, COAL, DoInitPage);
        DisplayField(2, 1, TheBoatData.EngineCoolantTemp, "CoolT", "%3.1f", AQUA, COAL, DoInitPage);
        break;

      case PAGE4:  // GPS check
        DisplayLAT(0, 0, TheBoatData.Latitude, "LAT", "%4.1f", GREEN, COAL, DoInitPage);
        DisplayLON(1, 0, TheBoatData.Longitude, "LON", "%4.1f", GREEN, COAL, DoInitPage);
        DisplayTIME(2, 0, TheBoatData.GNSSTime, TheBoatData.GNSSDate, AQUA, COAL);

        DisplayField(0, 1, TheBoatData.nSatellites, "nSat", "%3.0f", ORANGERED, COAL, DoInitPage);
        DisplayField(1, 1, TheBoatData.PDOP, "PDOP", "%5.2f", ORANGERED, COAL, DoInitPage);
        DisplayField(2, 1, TheBoatData.HDOP, "HDOP", "%5.2f", ORANGERED, COAL, DoInitPage);
        break;
      case PAGE5:  //environment
        DisplayField(0, 0, TheBoatData.YawAngle, "Yaw", "%4.1f", MAGENTA, COAL, DoInitPage);
        DisplayField(1, 0, TheBoatData.PitchAngle, "Pitch", "%4.1f", GBLUE, COAL, DoInitPage);
        DisplayField(2, 0, TheBoatData.RollAngle, "Roll", "%4.1f", CYAN, COAL, DoInitPage);

        DisplayField(0, 1, TheBoatData.WaterTemp, "WaterT", "%4.1f", AQUA, COAL, DoInitPage);
        DisplayField(1, 1, TheBoatData.AirPressure, "P", "%4.0f", YELLOW, COAL, DoInitPage);
        DisplayField(2, 1, TheBoatData.AirTemperature, "Air T", "%4.2f", YELLOW, COAL, DoInitPage);
        break;
      case PAGE6:
        DisplaySatsInView();
        break;
    }
  }
  wavesahre_rgb_lcd_display(BlackImage);
}


void SwitchPage() {
  WhichPage += 1;
  if (WhichPage > HOWMANYPAGES)
    WhichPage = PAGE1;

  InitPage(true);
  DisplayPage(WhichPage);
}
