 /**
 * Nextion_lcd.h
 *
 * Copyright (c) 2020 
 *
 * Grbl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grbl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Grbl. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NEXTION_LCD_H_
#define _NEXTION_LCD_H_

// For debug Connect
//#define NEXTION_CONNECT_DEBUG
#include "../../MarlinCore.h"
#include "../ultralcd.h"
#include "library/Nextion.h"
#include "HardwareSerial.h"

#if ENABLED(ADVANCED_PAUSE_FEATURE)
	#include "../../core/types.h"
#endif


#if ENABLED(NEXTION)

class NextionLCD 
{
  public:
    void check_periodical_actions();  //Aktualizacja LCD z mniejsza czestotliwosca - 0.4s -> nextion_draw_update();
    void nextion_draw_update();       //Odswieza aktualny ekran -> switch
    void init();                      //Inicjalizacja LCD
    void update();                    //Nextion Update 5ms (odswieza i sprawdza komponenty Nex Pop Push)
    void kill_screen_msg(const char* lcd_msg, PGM_P const component); // Wyswietla ekran kill wraz z komunikatem
    



    void return_after_leveling(bool finish);    //powrot do ekranu statusu po zakonczeniu levelingu //wait for user = false
    void lcd_yesno(const uint8_t val, const char* msg1="", const char* msg2="", const char* msg3="");
    void nextion_babystep_z(bool dir);

    void nex_stop_printing();
    void setpage_Status();
    void menu_action_sdfile(const char* filename);
    void nex_check_sdcard_present();

    void menu_action_sddirectory(const char* filename);
  
    void setrowsdcard(uint32_t number = 0);
    void printrowsd(uint8_t row, const bool folder, const char* filename, const char* longfilename);



    static void sethotPopCallback(void *ptr);

    static void YesNoPopCallback(void *ptr);
    static void setmovePopCallback(void *ptr);
    static void setfanandgoPopCallback(void *ptr);
    static void setgcodePopCallback(void *ptr);
    static void sendPopCallback(void *ptr);
    static void setsetupstatPopCallback(void *ptr);
    static void getaccelPagePopCallback(void *ptr);
    static void setaccelpagePopCallback(void *ptr);
    static void setaccelsavebtnPopCallback(void *ptr);
    static void setaccelloadbtnPopCallback(void *ptr);
    static void setBabystepUpPopCallback(void *ptr);
    static void setBabystepDownPopCallback(void *ptr);
    static void setBabystepEEPROMPopCallback(void *ptr);
    static void setspeedPopCallback(void *ptr);
    static void setflowPopCallback(void *ptr);
    static void motoroffPopCallback(void *ptr);

    #if FAN_COUNT > 0
    static void setfanPopCallback(void *ptr);
    #endif

    #if ENABLED(SDSUPPORT)
    static void sdmountdismountPopCallback(void *ptr);
    static void sdlistPopCallback(void *ptr);
    static void sdfilePopCallback(void *ptr);
    static void sdfolderPopCallback(void *ptr);
    static void sdfolderUpPopCallback(void *ptr);
    static void PlayPausePopCallback(void *ptr);
    static void StopPopCallback(void *ptr);
    static void DFirmwareCallback(void *ptr);

    void setpageSD();
    void UploadNewFirmware();
    #endif

    //bool g29_in_progress = false;
 
    #if ENABLED(PROBE_MANUALLY)
      static void ProbelPopCallBack(void *ptr);
      float lcd_probe_pt(const float &lx, const float &ly);
      #if HAS_LEVELING
        void Nextion_ProbeOn();
        void Nextion_ProbeOff();
      #endif
    #endif

  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    void lcd_advanced_pause_show_message(const PauseMessage message,
                                         const PauseMenuResponse mode = PAUSE_RESPONSE_WAIT_FOR);
  #endif

  #if ENABLED(RFID_MODULE)
    void rfidPopCallback(void *ptr);
    void rfid_setText(const char* message, uint32_t color = 65535);
  #endif

  #if ENABLED(NEXTION_GFX)
    void gfx_origin(const float x, const float y, const float z);
    void gfx_scale(const float scale);
    void gfx_clear(const float x, const float y, const float z, bool force_clear=false);
    void gfx_cursor_to(const float x, const float y, const float z, bool force_cursor=false);
    void gfx_line_to(const float x, const float y, const float z);
    void gfx_plane_to(const float x, const float y, const float z);
  #endif

  #if HAS_CASE_LIGHT
    void setlightPopCallback(void *ptr);
  #endif
    //void filamentPopCallback(void *ptr);
    
    //void lcd_scrollinfo(const char* titolo, const char* message);
  private:
  protected:
};

extern NextionLCD nexlcd;


extern float feedrate_mm_s; //

#endif // ENABLED(NEXTION)

#endif /* _NEXTION_LCD_H_ */
