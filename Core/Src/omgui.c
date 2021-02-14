/*
 * omgui.h
 *
 *  OLED MCU GUI Library
 *
 *  Created on: 23 янв. 2021 г.
 *      Author: nick
 */
#include "stdbool.h"
#include "omgui.h"
#include "main.h"

#ifdef DEBUG
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#endif

#include "sh1122.h"
#include "sh1122_conf.h"


void omGuiInit(omGuiT *ui)
{
    ui->InitCallback(ui); // OLED Driver's Init code goes here
    ui->ActiveScreen=NULL; // Discarded if set, use omScreenSelect()
}


void omGuiDeInit(omGuiT *ui)
{
    ui->DeInitCallback(ui); // OLED Driver's DeInit code goes here
}

void omScreenUpdate(omGuiT *ui)
{
    if(ui->ActiveScreen->UpdateCallback != NULL)
    {
        ui->ActiveScreen->UpdateCallback(ui->ActiveScreen);
    }

    omGuiUpdate(ui);
}


bool omScreenSelect(omScreenT *screen)
{
    if(omScreenIsActive(screen) == true) { return false; }

    if(screen->Ui->ActiveScreen != NULL && screen->Ui->ActiveScreen->HideCallback != NULL)
    {
        screen->Ui->ActiveScreen->HideCallback(screen);
    }

    omGuiClear(screen->Ui);

    if(screen->ShowCallback != NULL)
    {
        screen->ShowCallback(screen);
    }

    screen->Ui->ActiveScreen = screen;

    return true;
}


bool omScreenIsActive(omScreenT *screen)
{
    return (screen->Ui->ActiveScreen == NULL
        || screen->Ui->ActiveScreen->Id != screen->Id) ? false : true;
}


void omGuiUpdate(omGuiT *ui)
{
    ui->UpdateCallback(ui);
}


void omGuiClear(omGuiT *ui)
{
    ui->ClearCallback(ui);
}


void omDrawPixel(omGuiT *ui, uint32_t x, uint32_t y, uint8_t color)
{
    ui->DrawPixelCallback(ui, x, y, color);
}


/* Store bitmap RawData in FLASH memory, not RAM, image buffer is corrupted otherwise.
 * Data array must be declared as global 'const'. Further reading:
 * https://electronics.stackexchange.com/questions/74589/how-to-stock-variables-in-flash-memory
 * https://forum.arduino.cc/index.php?topic=461487.0
 */
void omDrawBitmap(omGuiT *ui, omBitmapT *bitmap, uint32_t x, uint32_t y, bool alpha, bool update)
{
    volatile uint8_t color1, color2;
    uint32_t idx = 0;
    bool is_color1 = true;

    for(uint16_t yto = y; yto < bitmap->Height + y; yto++)
    {
        for(uint16_t xto = x; xto < bitmap->Width + x; xto++)
        {
            if(is_color1)
            {
                color1 = bitmap->RawData[idx] >> 4;
                color2 = bitmap->RawData[idx] & 0x0f;

                if(!alpha)
                {
                    omDrawPixel(ui, xto, yto, color1);
                }

                is_color1 = false;
            }
            else
            {
                if(!alpha)
                {
                    omDrawPixel(ui, xto, yto, color2);
                }

                idx++;
                is_color1 = true;
            }
        }
    }

    if(update) { omGuiUpdate(ui); }
}


void omDrawLine(omGuiT *ui, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t color, bool update)
{
    const int16_t deltaX = abs(x2 - x1);
    const int16_t deltaY = abs(y2 - y1);
    const int16_t signX = x1 < x2 ? 1 : -1;
    const int16_t signY = y1 < y2 ? 1 : -1;

    int16_t error = deltaX - deltaY;

    omDrawPixel(ui, x2, y2, color);

    while(x1 != x2 || y1 != y2)
    {
        omDrawPixel(ui, x1, y1, color);
        const int16_t error2 = error * 2;

        if(error2 > -deltaY)
        {
            error -= deltaY;
            x1 += signX;
        }
        if(error2 < deltaX)
        {
            error += deltaX;
            y1 += signY;
        }
    }

    if(update) { omGuiUpdate(ui); }
}


/* void omAnimationStart(omAnimationT *anim)
{
    omBitmapT *bitmap = anim->Bitmaps; // Don't increment original pointer

    for(uint16_t i = 0; i < anim->FramesNumOf; i++)
    {
        omDrawBitmap(bitmap, anim->PosX, anim->PosY);
        omGuiUpdate(anim->Ui);
        SYS_SLEEP(anim->Interval);

        bitmap++;
    }

    return;
} */
