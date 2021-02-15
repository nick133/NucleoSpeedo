
#include <string.h>
#include "ds18b20.h"
#include "sh1122.h"
#include "printf.h"

#include "main.h"
#include "settings.h"
#include "omgui.h"
#include "screens.h"
#include "bitmaps.h"


#ifdef DEBUG
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#endif

#define ASCII_SHIFT 48

static const omBitmapT *SpeedoFont[10] = {
    &AssetBitmaps.MainSpeedo0,
    &AssetBitmaps.MainSpeedo1,
    &AssetBitmaps.MainSpeedo2,
    &AssetBitmaps.MainSpeedo3,
    &AssetBitmaps.MainSpeedo4,
    &AssetBitmaps.MainSpeedo5,
    &AssetBitmaps.MainSpeedo6,
    &AssetBitmaps.MainSpeedo7,
    &AssetBitmaps.MainSpeedo8,
    &AssetBitmaps.MainSpeedo9
};

static char spdreg_prev[3];
static SensorsDataT prevSensors;

static void ScreenShowCb(omScreenT *);
static void ScreenUpdateCb(omScreenT *);
static void RefreshRpm(void);
static void RefreshSpeed(void);
static void RefreshVolt(void);
static void RefreshAmpere(void);

void MainScreenInit(void)
{
    screenMain.Id = IdScreenMain;
    screenMain.Ui = &oledUi;
    screenMain.ShowCallback = ScreenShowCb;
    screenMain.HideCallback = NULL;
    screenMain.UpdateCallback = ScreenUpdateCb;
}


static void ScreenShowCb(omScreenT *screen)
{
    prevSensors.HallRpm = 0;
    prevSensors.Volt = 0;
    memset(prevSensors.Temperature, 0.0, sizeof(prevSensors.Temperature));

    memset(spdreg_prev, '0', sizeof(spdreg_prev));

    omDrawBitmap(&oledUi, &AssetBitmaps.MainSpeedo0, 15+24, 2, false, false);
}


static void ScreenUpdateCb(omScreenT *screen)
{
    for(uint8_t i; i < DS18B20_Quantity(); i++)
    {
        if(sensors.Temperature[i] != prevSensors.Temperature[i])
        {
//SEGGER_RTT_printf(0, "temp[%u]: %u\n", i, (uint16_t)sensors.Temperature[i]);
            prevSensors.Temperature[i] = sensors.Temperature[i];
        }
    }

    if(sensors.HallRpm != prevSensors.HallRpm)
    {
        RefreshRpm();
        RefreshSpeed();
    }

    if(sensors.Volt != prevSensors.Volt)
    {
        RefreshVolt();
        RefreshAmpere();
    }
}


static void RefreshRpm(void)
{
    uint16_t rpm = (config.HallOnWheel == true)
        ? (int)roundf((float)sensors.HallRpm * config.GearRatio) : sensors.HallRpm;

}


static void RefreshSpeed(void)
{
    char spd[3];
    //SEGGER_RTT_printf(0, "from screen: %u\n", sensors.HallRpm);
    // Hall sensor is on wheel
    float speed = sensors.HallRpm * 60 * config.WheelCirc / 1000000;

    // Hall sensor is on motor's rotor
    if(config.HallOnWheel == false)
        { speed /= config.GearRatio; }

    if(config.SpeedUnits == UnitsMph)
        { speed /= KILOS_PER_MILE; }

    snprintf(spd, sizeof(spd), "%3.0f", speed);

    if(spd[0] != spdreg_prev[0])
    {
        if(spd[0] == '1')
        {
            omDrawBitmap(&oledUi, &AssetBitmaps.MainSpeedo1XX, 0, 2, false, false);
        }
        else
        {
            omDrawRectangleFilled(&oledUi, 0, 2, 14, 25, OLED_GRAY_00, OLED_GRAY_00, false);
        }
    }

    if(spd[1] != spdreg_prev[1] && spd[1] != ' ')
    {
        omDrawBitmap(&oledUi, SpeedoFont[spd[1] - ASCII_SHIFT], 15, 2, false, false);
    }

    if(spd[2] != spdreg_prev[2])
    {
        omDrawBitmap(&oledUi, SpeedoFont[spd[1] - ASCII_SHIFT], 15+24, 2, false, false);
    }

    spdreg_prev[0] = spd[0];
    spdreg_prev[1] = spd[1];
    spdreg_prev[2] = spd[2];
}


static void RefreshVolt(void)
{
    
}


static void RefreshAmpere(void)
{
    
}