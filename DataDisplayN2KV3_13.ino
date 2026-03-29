/*

NMEA Datadisplay using ESP32-S3 Touch 7B from Waveshare

Manfred Radmacher, 2026

*/

// Define CAN pins (default: GPIO 16 for TX, GPIO 4 for RX)
#define ESP32_TWAI_CAN_TX_PIN GPIO_NUM_20
#define ESP32_TWAI_CAN_RX_PIN GPIO_NUM_19

/***********************************************************************/
#include <Arduino.h>
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include "N2kDeviceList.h"
#include "HandleN2kData.h"

#include <rgb_lcd_port.h>  // Header for Waveshare RGB LCD driver
#include "gui_paint.h"     // Header for graphical drawing functions
#include "image.h"         // Header for image resource: startup image
#include <gt911.h>         // Header for touch screen operations (GT911)
#include "DrawData.h"

#define ROTATE ROTATE_0  //rotate = 0, 90, 180, 270

UBYTE *BlackImage;

static esp_lcd_panel_handle_t panel_handle = NULL;  // Declare a handle for the LCD panel
static esp_lcd_touch_handle_t tp_handle = NULL;     // Declare a handle for the touch panel

// Global NMEA2000 instance
extern tNMEA2000 &NMEA2000;

tN2kDeviceList *pN2kDeviceList;

tHandleN2kData tHandleN2kData(&NMEA2000);

// Set the information for other bus devices, which messages we support
const unsigned long TransmitMessages[] PROGMEM = { 130310L, 130311L, 130312L,  // Engine dynamic
                                                   0 };
const unsigned long ReceiveMessages[] PROGMEM = { /*126992L,*/  // System time
                                                  127250L,      // Heading
                                                  127258L,      // Magnetic variation
                                                  128259UL,     // Boat speed
                                                  128267UL,     // Depth
                                                  129025UL,     // Position
                                                  129026L,      // COG and SOG
                                                  129029L,      // GNSS
                                                  130306L,      // Wind
                                                  128275UL,     // Log
                                                  127245UL,     // Rudder

                                                  129540UL,  // sats in view
                                                  0 };

//*****************************************************************************
void PrintUlongList(const char *prefix, const unsigned long *List) {
  uint8_t i;
  if (List != 0) {
    Serial.print(prefix);
    for (i = 0; List[i] != 0; i++) {
      if (i > 0) Serial.print(", ");
      Serial.print(List[i]);
    }
    Serial.println();
  }
}

//*****************************************************************************
void PrintText(const char *Text, bool AddLineFeed = true) {
  if (Text != 0) Serial.print(Text);
  if (AddLineFeed) Serial.println();
}

//############################################################################

//*****************************************************************************
void PrintDevice(const tNMEA2000::tDevice *pDevice) {
  if (pDevice == 0) return;

  Serial.println("----------------------------------------------------------------------");
  Serial.println(pDevice->GetModelID());
  Serial.print("  Source: ");
  Serial.println(pDevice->GetSource());
  Serial.print("  Manufacturer code:        ");
  Serial.println(pDevice->GetManufacturerCode());
  Serial.print("  Unique number:            ");
  Serial.println(pDevice->GetUniqueNumber());
  Serial.print("  Software version:         ");
  Serial.println(pDevice->GetSwCode());
  Serial.print("  Model version:            ");
  Serial.println(pDevice->GetModelVersion());
  Serial.print("  Manufacturer Information: ");
  PrintText(pDevice->GetManufacturerInformation());
  Serial.print("  Installation description1: ");
  PrintText(pDevice->GetInstallationDescription1());
  Serial.print("  Installation description2: ");
  PrintText(pDevice->GetInstallationDescription2());
  PrintUlongList("  Transmit PGNs :", pDevice->GetTransmitPGNs());
  PrintUlongList("  Receive PGNs  :", pDevice->GetReceivePGNs());
  Serial.println();
}

//############################################################################

#define START_DELAY_IN_S 8
//*****************************************************************************
int ListDevices(bool force = false) {
  static bool StartDelayDone = false;
  static int StartDelayCount = 0;
  static unsigned long NextStartDelay = 0;
  int HowManyDevices = 0;

  Serial.printf("ListDevices %d %d %d\n", force, StartDelayDone, StartDelayCount);

  if (!StartDelayDone) {  // We let system first collect data to avoid printing all changes
    if (millis() > NextStartDelay) {
      if (StartDelayCount == 0) {
        Serial.print("Reading device information from NMEA bus ");
        NextStartDelay = millis();
      }
      Serial.print(".");
      NextStartDelay += 1000;
      StartDelayCount++;
      if (StartDelayCount > START_DELAY_IN_S) {
        StartDelayDone = true;
        Serial.println();
      }
    }
    return (0);
  }
  if (!force && !pN2kDeviceList->ReadResetIsListUpdated()) return (0);

  Serial.println();
  Serial.println("**********************************************************************");
  for (uint8_t i = 0; i < N2kMaxBusDevices; i++) PrintDevice(pN2kDeviceList->FindDeviceBySource(i));
  HowManyDevices = pN2kDeviceList->Count();
  return (HowManyDevices);
}

//############################################################################

void setup() {
  // Start Serial
  Serial.begin(115200);  // Initialize serial communication at 11520 0 baud rate
  delay(10);
  Serial.println("Starting Setup");

  DEV_I2C_Port I2Cport = DEV_I2C_Init();
  IO_EXTENSION_Init();
  delay(10);
  IO_EXTENSION_Output(IO_EXTENSION_IO_5, 1);  // Select communication interface: 0 for USB, 1 for CAN
  Serial.println("IO extension output set to can");

  // start screen setup
  // Initialize the Waveshare ESP32-S3 RGB LCD hardware
  panel_handle = waveshare_esp32_s3_rgb_lcd_init();
  // Initialize the Waveshare ESP32-S3 RGB LCD
  Serial.println("Setup after: lcd_init");

  // Turn on the LCD backlight
  wavesahre_rgb_lcd_bl_on();
  Serial.println("Setup after: lcd_bl_on");

  InitBackLightControl();

  UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2;  // Each pixel takes 2 bytes in RGB565
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)          // Allocate memory
  {
    printf("Failed to apply for black memory...\r\n");
    exit(0);  // Exit the program if memory allocation fails
  }
  // Create a new image canvas and set its background color to white
  Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

  // Set the canvas scale
  Paint_SetScale(65);  // e:\Project\ESP32-S3-Touch-LCD-7B\Code\ESP32-S3-Touch-LCD-7B-Demo\PlatformIO\09_DISPLAY_BMP\lib\image\image.h
  Paint_SetRotate(ROTATE_0);

  // Clear the canvas and fill it with a white background
  Paint_Clear(WHITE);
  // Draw an image resource gImage_picture at coordinates (0,0) with size 800x480
  //       Paint_DrawImage(gImage_FatuIva, 0, 0, 1024, 600);
  Paint_DrawImage(gImage_FatuIva, 0, 0, 1024, 600);
  // Update the screen with the new image (BlackImage is the framebuffer being drawn to)
  wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display to show the updated image
                                          // end screen setup

  // setup touch
  tp_handle = touch_gt911_init(I2Cport);
  Serial.println("Setup after: gt911 touch init");
  // end setup touch

  // start NMEA 2000 setup
  Serial.println("starting NMEA 2000 setup");

  // Initialize the CAN communication interface
  NMEA2000.SetN2kCANReceiveFrameBufSize(150);
  NMEA2000.SetN2kCANMsgBufSize(8);
  // Set Product information
  NMEA2000.SetProductInformation("00000003",                 // Manufacturer's Model serial code
                                 100,                        // Manufacturer's product code
                                 "N2k bus device analyzer",  // Manufacturer's Model ID
                                 "1.0.0.10 (2017-07-29)",    // Manufacturer's Software version code
                                 "1.0.0.0 (2017-07-12)"      // Manufacturer's Model version
  );

  // Set device information
  NMEA2000.SetDeviceInformation(2,    // Unique number. Use e.g. Serial number.
                                130,  // Device function=Display. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                120,  // Device class=Display. See codes on  https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046  // Just choosen free from code list on https://web.archive.org/web/20190529161431/http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
  );
  // Uncomment 3 rows below to see, what device will send to bus

  NMEA2000.SetForwardStream(0);                   //&Serial);
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text);  // Show in clear text. Leave uncommented for default Actisense format.
  NMEA2000.EnableForward(false);
 // NMEA2000.SetForwardOwnMessages();

  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, 50);

  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.ExtendReceiveMessages(ReceiveMessages);
  NMEA2000.AttachMsgHandler(&tHandleN2kData);  // NMEA 2000 -> NMEA 0183 conversion

  pN2kDeviceList = new tN2kDeviceList(&NMEA2000);
  NMEA2000.Open();
  ListDevices();
  Serial.println("\nfinished NMEA 2000 setup");
  // end NMEA 2000 setup

  //
  Paint_DrawString(20, 520, "NMEA Data Display Manfred Radmacher ", &FreeSansBold20pt7b, YELLOW, LIGHTGRAY, TRANSPARENT);
  Paint_DrawString(20, 570, "Version 0.2 Feb 23, 2026", &FreeSansBold20pt7b, YELLOW, LIGHTGRAY, TRANSPARENT);
  wavesahre_rgb_lcd_display(BlackImage);
 
  delay(500);

  InitPage(true);
//  task_monitor();
  Serial.println("setup finished");
}

//############################################################################

void loop() {

  NMEA2000.ParseMessages();

  static unsigned long OldMS = 0;
  static int HowManyDevices = 0;
  static unsigned int DoInitPage = 1;

    if (CheckTouchStatus() == true) {
      InitPage(true);
      DoInitPage = 1;
    }
 
    DisplayPage(DoInitPage);

    DoInitPage = 0;
    if ((millis() - OldMS) > 10000) {
      if (HowManyDevices == 0)
        HowManyDevices = ListDevices();
//      PrintAllData();
      OldMS = millis();
    }
}
