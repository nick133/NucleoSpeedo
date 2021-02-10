/*
 * omgui.h
 *
 *  OLED MCU GUI Library
 *
 *  Created on: 23 янв. 2021 г.
 *      Author: nick
 */

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


Bool omScreenSelect(omScreenT *screen)
{
  if (omScreenIsActive(screen) == True)
  {
    return False;
  }

  if (screen->Ui->ActiveScreen != NULL && screen->Ui->ActiveScreen->HideCallback != NULL)
  {
    screen->Ui->ActiveScreen->HideCallback(screen);
  }

  if (screen->ShowCallback != NULL)
  {
    screen->ShowCallback(screen);
  }

  for (uint16_t i = 0; i < screen->WidgetsNumOf; i++)
  {
    // array[1] equals to *(array + 1)
    // https://stackoverflow.com/questions/16201607/c-pointer-to-array-of-structs
    screen->Widgets[i].ShowCallback(screen->Widgets + i);
  }

  screen->Ui->ActiveScreen = screen;

  return True;
}


Bool omScreenIsActive(omScreenT *screen)
{
  return (screen->Ui->ActiveScreen == NULL || screen->Ui->ActiveScreen->Id != screen->Id) ? False : True;
}


void omGuiUpdate(omGuiT *ui)
{
  ui->UpdateCallback(ui);
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
void omDrawBitmap(omGuiT *ui, omBitmapT *bitmap, uint32_t x, uint32_t y, Bool update)
{
  uint8_t *data = bitmap->RawData; // (!) IMPORTANT (!) Don't increment original pointer
  uint8_t color1, color2;
  Bool is_color1 = True;

  for (uint8_t yto = y; yto < bitmap->Height + y; yto++)
  {
    for (uint8_t xto = x; xto < bitmap->Width + x; xto++)
    {
        // 81 29
      if (xto == 79 && yto == 28)
        SEGGER_RTT_printf(0, "x=%d, y=%d, byte=0x%x\n", xto, yto, *data);
      if (is_color1)
      {
        color1 = *data >> 4;
        color2 = *data & 0x0f;

      if (xto == 79 && yto == 28)
        SEGGER_RTT_printf(0, "x=0x%x, y=0x%x\n", color1, color2);
        if (!(bitmap->IsAlpha == True && bitmap->AlphaColor == color1))
        {
          omDrawPixel(ui, xto, yto, color1);
        }

        is_color1 = False;
      }
      else
      {
        if (!(bitmap->IsAlpha == True && bitmap->AlphaColor == color2))
        {
          omDrawPixel(ui, xto, yto, color2);
        }

        data++;
        is_color1 = True;
      }
    }
  }

  if (update)
  {
    omGuiUpdate(ui);
  }
}

/* void omAnimationStart(omAnimationT *anim)
{
  omBitmapT *bitmap = anim->Bitmaps; // Don't increment original pointer

  for (uint16_t i = 0; i < anim->FramesNumOf; i++)
  {
    omDrawBitmap(bitmap, anim->PosX, anim->PosY);
    omGuiUpdate(anim->Ui);
    SYS_SLEEP(anim->Interval);

    bitmap++;
  }

  return;
} */
