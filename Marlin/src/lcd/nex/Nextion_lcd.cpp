/**
 * Nextion_lcd.cpp
 */
#include "../../MarlinCore.h"
#include "../../module/temperature.h"
#include "../../sd/cardreader.h"
#include "../../module/printcounter.h"
#include "../../libs/numtostr.h"
#include "../../HAL/shared/eeprom_api.h"
#include "../../lcd/extui/ui_api.h"

#if ENABLED(NEXTION_DISPLAY)
	#include "../../module/stepper.h"
	#include "../../feature/bedlevel/mbl/mesh_bed_leveling.h"
	#include "../../module/planner.h"
	#include "../../module/settings.h"
	#include "../../lcd/ultralcd.h"
	//#include "../../lcd/menu/menu_item.h"
#endif

#if ENABLED(SPEAKER)
	#include "../../libs/buzzer.h"
#endif
#if ENABLED(BABYSTEPPING)
	#include "../../feature/babystep.h"
	extern Babystep babystep;
	int _babystep_z_shift = 0;
#endif

#if ENABLED(NEXTION_DISPLAY)
  #include "Nextion_lcd.h"
	#include "Nextion_components.h"
  #include "Nextion_gfx.h"
  #include "library/Nextion.h"

	NextionLCD nexlcd;

  uint8_t     PageID                    = 0,
              lcd_status_message_level  = 0;
  uint16_t    slidermaxval              = 20;
  char        bufferson[70]             = { 0 };
  //const float manual_feedrate_mm_s[]    = MANUAL_FEEDRATE; //bylo w ultralcd, obecnie jest w planner.h 
	millis_t		screen_timeout_millis;

		// Zmienne dluga nazwa na ekranie statusu
	int		nex_file_number[6];
	int 	nex_file_row_clicked;
	char	filename_printing[40];

	#if PIN_EXISTS(SD_DETECT)
		uint8_t lcd_sd_status;
	#endif
	
	// ZMIENNE ZEWNETRZNE MARLINa
	//extern uint8_t progress_printing; // dodane nex
	extern bool nex_filament_runout_sensor_flag;
	extern xyze_pos_t destination;// = { 0.0 };
	extern bool g29_in_progress;// = false;
	extern inline void set_current_to_destination() { COPY(current_position, destination); }
	extern inline void set_destination_to_current() { COPY(destination, current_position); }
	extern void home_all_axes();


  #if ENABLED(SDSUPPORT)
		extern CardReader card;
    // 0 card not present, 1 SD not insert, 2 SD insert, 3 SD printing
    enum SDstatus_enum {NO_SD = 0, SD_NO_INSERT = 1, SD_INSERT = 2, SD_PRINTING = 3, SD_PAUSE = 4 };
    SDstatus_enum SDstatus    = NO_SD;

		#if ENABLED(NEX_UPLOAD)
			NexUpload Firmware(NEXTION_FIRMWARE_FILE, 57600);
		#endif
  #endif

  #if ENABLED(NEXTION_GFX)
    GFX gfx = GFX(1, 1, 1, 1);
  #endif
	
	#if ENABLED(ADVANCED_PAUSE_FEATURE)
		
		bool nex_m600_heatingup = 0;
		// NOWE 2.0
		void lcd_pause_pausing_message()  { nexlcd.nex_enqueue_filament_change(); }
		void lcd_pause_parking_message()  { nexlcd.lcd_advanced_pause_init_message(); }
		void lcd_pause_changing_message() { nexlcd.lcd_advanced_pause_load_message(); }
		void lcd_pause_unload_message()   { nexlcd.lcd_advanced_pause_unload_message(); }
		void lcd_pause_heating_message()  { nexlcd.lcd_advanced_pause_wait_for_nozzles_to_heat(); }
		void lcd_pause_heat_message()     { nexlcd.lcd_advanced_pause_heat_nozzle(); }
		void lcd_pause_insert_message()   { nexlcd.lcd_advanced_pause_insert_message(); }
		void lcd_pause_load_message()     { nexlcd.lcd_advanced_pause_load_message(); }
		void lcd_pause_waiting_message()  { nexlcd.lcd_advanced_pause_wait_for_nozzles_to_heat(); }
		void lcd_pause_resume_message()   { nexlcd.lcd_advanced_pause_resume_message(); }
		void lcd_pause_toocold_menu()			{ nexlcd.lcd_advanced_pause_toocold_menu(); }

		//void lcd_pause_pausing_message()  { _lcd_pause_message(GET_TEXT(MSG_PAUSE_PRINT_INIT));        }
		//void lcd_pause_changing_message() { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_INIT));    }
		//void lcd_pause_unload_message()   { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_UNLOAD));  }
		//void lcd_pause_heating_message()  { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING)); }
		//void lcd_pause_heat_message()     { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_HEAT));    }
		//void lcd_pause_insert_message()   { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_INSERT));  }
		//void lcd_pause_load_message()     { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_LOAD));    }
		//void lcd_pause_waiting_message()  { _lcd_pause_message(GET_TEXT(MSG_ADVANCED_PAUSE_WAITING));  }
		//void lcd_pause_resume_message()   { _lcd_pause_message(GET_TEXT(MSG_FILAMENT_CHANGE_RESUME));  }
	#endif

  
	
  // Function pointer to menu functions.
  typedef void (*screenFunc_t)();


	/**
	 * 	NEX LCD SDCARD STOP
	 *	Nextion stop print button
	 */
  void NextionLCD::nex_stop_printing() {
		card.flag.abort_sd_printing = true; 		// Ta flaga zatrzymuje wydruk w kolejnej wolnej instrukcji idle();
		// 2.0 did_pause_print = false;					// flaga pause_print na false, na wypadek gdyby drukarka byla w stanie pauzy @_@
		//stepper.quick_stop_panic();						// pomocne z panic'a, trzeba to zaserwowac aby mozna bylo ponownie wykonac jakakolwiek komende
		#if ENABLED(PLOSS_SUPPORT)
			//_babystep_z_shift = 0;								// dodane - zeruje babystep po zatrzymaniu wydruku
			persistentStore.writedata((uint32_t*)(EEPROM_PANIC_BABYSTEP_Z), _babystep_z_shift);	// zeruj babystepping w eeprom
		#endif

		SDstatus = SD_INSERT;
		SD.setValue(SDstatus,"stat");				// 

		percentdone.setText("0", "stat");		// zeruj procenty
		progressbar.setValue(0, "stat");			// zeruj progress bar
		ui.set_status_P(GET_TEXT(MSG_PRINT_DONE), -1);	// status bar info
	}

	/**
	 * 	NEX Obsluga klikniecia przycisku  PLAY / PAUSE
	 */
  void NextionLCD::PlayPausePopCallback(void *ptr){
    UNUSED(ptr);
    if (card.isMounted && card.isFileOpen()) {
      if (card.isPrinting) {														//pause
        card.pauseSDPrint();
        print_job_timer.pause();
        #if ENABLED(PARK_HEAD_ON_PAUSE)
        	queue.inject_P(PSTR("M125"));
        #endif
				ui.set_status_P(GET_TEXT(MSG_PRINT_PAUSED), 1);

				SDstatus = SD_PAUSE;
				SD.setValue(SDstatus,"stat");// ustaw nex sdval na pause
      }
      else {																					//resume
				#if ENABLED(PARK_HEAD_ON_PAUSE)
					queue.inject_P(PSTR("M24"));
				#else
				card.startFileprint();
				print_job_timer.start();
				#endif
				ui.set_status_P(GET_TEXT(MSG_RESUME_PRINT), 1);

				SDstatus = SD_PRINTING;
				SD.setValue(SDstatus,"stat");
      }
    }
  }

	/**
	 * 	Ustawia strone statusu przypisujac zerowe wartosci do zmiennych tj. glowica, stol, fan, stan SD
	 *	wywolywana raz w lcdinit()
	 */
  void NextionLCD::setpage_Status(){
    #if HOTENDS > 0
      Hotend00.setValue(0, "stat");
			Hotend01.setValue(0, "stat");
    #endif

    #if HAS_TEMP_BED
      Bed0.setValue(0,"stat");
			Bed1.setValue(0,"stat");
    #endif

		#if FAN_COUNT > 0
			PrinterFanspeed.setValue(0, "stat");
		#endif

		VSpeed.setValue(100, "stat");

    #if ENABLED(SDSUPPORT)
      if (!card.isMounted) card.mount();
      delay(100);
      if (card.isMounted) {
        SDstatus = SD_INSERT;
        card.cdroot();  // Initial boot
      }
      else SDstatus = SD_NO_INSERT;

      SD.setValue(SDstatus, "stat");
    #endif

    #define LANGUAGE_STRING(M) STRINGIFY(M)
    #define NEXTION_LANGUAGE LANGUAGE_STRING(LCD_LANGUAGE)
    Language.setText(NEXTION_LANGUAGE, "stat");
  }

	/**
	 * 	SDSUPPORT
	 */
	#if ENABLED(SDSUPPORT)

		#if ENABLED(NEX_UPLOAD)
			void NextionLCD::UploadNewFirmware() {
				if (IS_SD_INSERTED || card.isMounted) {
					Firmware.startUpload();
					nexSerial.end();
					ui.init();
				}
			}
		#endif

		#if ENABLED(NEXTION_SD_LONG_NAMES)
			//Drukuje linijke na stronie SDCARD
			//1 iterator
			//2 folder
			//3 nazwa 8.3
			//4 nazwa do wyswietlenia
			void NextionLCD::printrowsd(uint8_t row, const bool folder, const char* filename, const char* longfilename) {			
					if (folder) {
						folder_list[row]->SetVisibility(true);
						row_list[row]->attachPop(nexlcd.sdfolderPopCallback, row_list[row]);
					} else if (filename == "") {
						folder_list[row]->SetVisibility(false);
						row_list[row]->detachPop();
					} else {
						folder_list[row]->SetVisibility(false);
						row_list[row]->attachPop(nexlcd.sdfilePopCallback, row_list[row]);
					}
					file_list83[row]->setText(filename);
					row_list[row]->setText(longfilename);
				}
		#else //SHORT 8.3 DOS NAMES
			//1 iterator
			//2 folder
			//3 nazwa 8.3
			void NextionLCD::printrowsd(uint8_t row, const bool folder, const char* filename) {
				if (folder) {
					folder_list[row]->SetVisibility(true);
					row_list[row]->attachPop(nexlcd.sdfolderPopCallback, row_list[row]);
				}
				else if (filename == "") {
					folder_list[row]->SetVisibility(false);
					row_list[row]->detachPop();
				}
				else {
					folder_list[row]->SetVisibility(false);
					row_list[row]->attachPop(sdfilePopCallback, row_list[row]);
				}
				row_list[row]->setText(filename);
			}
		#endif

		//Ustawia liste plikow na stronie SDCARD
    void NextionLCD::setrowsdcard(uint32_t number) {
      uint16_t fileCnt = card.get_num_Files();
      uint32_t i = 0;
      card.getWorkDirName();
			
      if (card.filename[0] != '/') {
        Folderup.SetVisibility(true);
        Folderup.attachPop(nexlcd.sdfolderUpPopCallback);

				if(card.isMounted())
				{
					sdfolder.setText(card.longFilename);
				}else
				{
					sdfolder.setText(GET_TEXT(MSG_MEDIA_WAITING));
				}

      } else {
        Folderup.detachPop();
        Folderup.SetVisibility(false);
        sdfolder.setText("");
      }

      if (fileCnt >= 0) {
        for (uint8_t row = 0; row < 6; row++) {
          i = row + number;
          if (i < fileCnt) {
            #if ENABLED(SDCARD_SORT_ALPHA)
              card.getfilename_sorted(i);
            #else
              card.getfilename(i); // brak sensownego ciala funkcji - linker wali babole
            #endif

						#if ENABLED(NEXTION_SD_LONG_NAMES)
							printrowsd(row, card.isFilenameisDir(), card.filename, card.longFilename);
							nex_file_number[row] = i;
						#else
							printrowsd(row, card.isFilenameisDir(), card.filename);
							nex_file_number[row] = i;
						#endif

          } else {
						#if ENABLED(NEXTION_SD_LONG_NAMES)
							printrowsd(row, false, "", "");
							nex_file_number[row] = NULL; // nie mozna wyzerowac (bo bedzie w wolnych polach wszezie plik nr: 0 czyli jakiś) może tu być potencjalny bug.
						#else
							printrowsd(row, false, "");
							nex_file_number[row] = NULL; // nie mozna wyzerowac (bo bedzie w wolnych polach wszezie plik nr: 0 czyli jakiś) może tu być potencjalny bug.
						#endif
          }
        }
      }
    }

		/**
		 * 	Funkcja z obsluga klikniecia w linijke z nazwa pliku
		 *	Zapisuje do EEPROM sciezke pliku oraz babystep (VLCS)
		*/
    void NextionLCD::menu_action_sdfile(const char* filename) 
		{
			#if ENABLED(PLOSS_SUPPORT) // jezeli VLCS wlaczony
				for (int i = 0; i < 8; i++) {
					eeprom_write_byte((uint8_t*)EEPROM_SD_FILENAME + i, filename[i]);
				}

				uint8_t depth = (uint8_t)card.getWorkDirDepth();
				eeprom_write_byte((uint8_t*)EEPROM_SD_FILE_DIR_DEPTH, depth);

				for (uint8_t i = 0; i < depth; i++) {
					for (int j = 0; j < 8; j++) {
						eeprom_write_byte((uint8_t*)EEPROM_SD_DIRS + j + 8 * i, dir_names[i][j]);
					}
				}
				_babystep_z_shift = 0;																												// zeruj babystep po uruchomieniu wydruku
				//KATT eeprom_update_dword((uint32_t*)(EEPROM_PANIC_BABYSTEP_Z), _babystep_z_shift);	// zeruj babystepping w eeprom
			#endif // jezeli VLCS wlaczone

			SDstatus = SD_PRINTING;
			SD.setValue(SDstatus,"stat"); // ustaw nex sdval na printing

			


			card.selectFileByName(filename);
			card.openAndPrintFile(filename);
			card.startFileprint();
			strncpy(filename_printing, card.longFilename, 40); // card.longFilename
			SERIAL_ECHOPGM("card.longfilename: "); SERIAL_ECHOLN(card.longFilename);
			
      PagePrinter.show();
			sendCommand("ref 0");
    }


		/**
		 *	Funkcja z obsluga klikniecia w linijke z nazwa FOLDERU
		*/
    void NextionLCD::menu_action_sddirectory(const char* filename) {
			#if ENABLED(PLOSS_SUPPORT)
				uint8_t depth = (uint8_t)card.getWorkDirDepth();	// dodane	
				strcpy(dir_names[depth], filename);								// dodane
			#endif	//OCB?

      card.cd(filename);
      setpageSD();
    }

		void NextionLCD::menu_action_function(screenFunc_t func) { (*func)(); }

		/**
		 *	Ustawia strone z karta SD
		*/
    void NextionLCD::setpageSD() {
      uint16_t fileCnt = card.get_num_Files();

      if (fileCnt <= 6)
        slidermaxval = 0;
      else
        slidermaxval  = fileCnt - 6;

			uint16_t hig = 210 - slidermaxval * 10;
      if (hig < 10) hig = 10;
		
      sdscrollbar.Set_cursor_height_hig(hig);	
      sdscrollbar.setMaxval(slidermaxval);
      sdscrollbar.setValue(slidermaxval,"sdcard");

      setrowsdcard();
    }

		/**
		 *	Obsluga slidera / suwaka
		*/
    void NextionLCD::sdlistPopCallback(void *ptr) {
      UNUSED(ptr);
      uint16_t number = slidermaxval - sdscrollbar.getValue();
      nexlcd.setrowsdcard(number);
    }

		// NEXTION: Obsluga klikniecia linijek tekstu z nazwa pliku do druku
    void NextionLCD::sdfilePopCallback(void *ptr) {
      ZERO(bufferson);
			#if ENABLED(NEXTION_SD_LONG_NAMES)
				if (ptr == &sdrow0){
					file0.getText(bufferson, sizeof(bufferson));
					nex_file_row_clicked = 0;}
				else if (ptr == &sdrow1){
					file1.getText(bufferson, sizeof(bufferson));
					nex_file_row_clicked = 1;}
				else if (ptr == &sdrow2){
					file2.getText(bufferson, sizeof(bufferson));
					nex_file_row_clicked = 2;}
				else if (ptr == &sdrow3){
					file3.getText(bufferson, sizeof(bufferson));
					nex_file_row_clicked = 3;}
				else if (ptr == &sdrow4){
					file4.getText(bufferson, sizeof(bufferson));
					nex_file_row_clicked = 4;}
				else if (ptr == &sdrow5){
					file5.getText(bufferson, sizeof(bufferson));
					nex_file_row_clicked = 5;}
			#else
				if (ptr == &sdrow0)
					sdrow0.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow1)
					sdrow1.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow2)
					sdrow2.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow3)
					sdrow3.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow4)
					sdrow4.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow5)
					sdrow5.getText(bufferson, sizeof(bufferson));
			#endif

			buzzer.tone(100, 2300);
      nexlcd.menu_action_sdfile(bufferson);
    }

		// NEXTION: Obsluga klikniecia linijek tekstu z nazwa FOLDERU
    void NextionLCD::sdfolderPopCallback(void *ptr) {
      ZERO(bufferson);
			#if ENABLED(NEXTION_SD_LONG_NAMES)
				if (ptr == &sdrow0)
					file0.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow1)
					file1.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow2)
					file2.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow3)
					file3.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow4)
					file4.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow5)
					file5.getText(bufferson, sizeof(bufferson));
			#else
				if (ptr == &sdrow0)
					sdrow0.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow1)
					sdrow1.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow2)
					sdrow2.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow3)
					sdrow3.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow4)
					sdrow4.getText(bufferson, sizeof(bufferson));
				else if (ptr == &sdrow5)
					sdrow5.getText(bufferson, sizeof(bufferson));
			#endif
      nexlcd.menu_action_sddirectory(bufferson);
			buzzer.tone(10, 500);
    }

		// NEXTION: Obsluga klikniecia przycisku Folder Up
    void NextionLCD::sdfolderUpPopCallback(void *ptr) {
      UNUSED(ptr);
      card.cdup();
      nexlcd.setpageSD();
			buzzer.tone(10, 500);
    }
	#endif 
	/**
	 * 	SDSUPPORT END
	 */


  void NextionLCD::start_menu(const bool encoder=false, const bool push=false) 
	{
    PageSelect.show();
    LcdUp.SetVisibility(encoder);
    LcdDown.SetVisibility(encoder);
    LcdSend.SetVisibility(push);
    lcdDrawUpdate = true;
    lcd_clicked = !push;
	}

	/**
	 * START_SCREEN  Opening code for a screen having only static items.s
	 *               Do simplified scrolling of the entire screen.
	 *
	 * START_MENU    Opening code for a screen with menu items.
	 *               Scroll as-needed to keep the selected line in view.
	 */
	#define WAIT_FOR_CLICK_F(TYPE, ...) \
    if (lcd_clicked){ \
		nexlcd.menu_action_ ## TYPE(__VA_ARGS__); \
    return; }\

	#define WAIT_FOR_CLICK() \
    if (lcd_clicked){ \
			nex_m600_heatingup = 0;\
			PageFilament.show();\
    return; }\

  #define START_SCREEN() \
    nexlcd.start_menu(false, true); \
    do { \
      uint8_t _lcdLineNr = 0; \

  #define START_MENU() \
    nexlcd.start_menu(true, true); \
    uint16_t encoderLine = 1; \
    uint8_t _lcdLineNr = 0; \
    do { \
      _lcdLineNr = 0; \
      encoderLine = LcdPos.getValue(); \
      delay(100)

  #define MENU_ITEM(TYPE, LABEL, ...) \
      if (nexlcd.lcdDrawUpdate) { \
        lcd_row_list[_lcdLineNr]->setText_PGM(PSTR(LABEL)); \
        LcdMax.setValue(_lcdLineNr); \
      } \
      if (nexlcd.lcd_clicked && encoderLine == _lcdLineNr) { \
        nexlcd.menu_action_ ## TYPE(__VA_ARGS__); \
        return; \
      } \
      ++_lcdLineNr

  #define MENU_BACK(LABEL) MENU_ITEM(back, LABEL)

  #define STATIC_ITEM_P(LABEL) \
      if (nexlcd.lcdDrawUpdate) { \
        lcd_row_list[_lcdLineNr]->setText_PGM(LABEL); \
        LcdMin.setValue(_lcdLineNr + 1); \
      } \
      ++_lcdLineNr \

  #define STATIC_ITEM(LABEL) STATIC_ITEM_P(PSTR(LABEL))

  #define END_MENU() \
      idle(); \
      nexlcd.lcdDrawUpdate = false; \
    } while(1)

  #define END_SCREEN() \
      nexlcd.lcdDrawUpdate = false; \
    } while(0)

	 // Portions from STATIC_ITEM...
	#define HOTEND_STATUS_ITEM() do { \
        if (nexlcd.lcdDrawUpdate) { \
          lcd_row_list[_lcdLineNr]->setText(i8tostr3(thermalManager.degHotend)); \
        } \
				nexlcd.lcdDrawUpdate = true; \
				++_lcdLineNr; \
    }while(0)


	// ========================
	// FILAMENT CHANGE M600
	// ========================
  #if ENABLED(ADVANCED_PAUSE_FEATURE)

    static PauseMenuResponse advanced_pause_mode = PAUSE_RESPONSE_WAIT_FOR;

		void NextionLCD::lcd_advanced_pause_toocold_menu() {
			nex_m600_heatingup = 1; // wlacz wyswietlanie temperatury
			//screen_timeout_millis = millis(); // wlaczamy timer
			START_SCREEN();
				STATIC_ITEM(GET_TEXT(MSG_TOO_COLD_FOR_M600_1));
				STATIC_ITEM(GET_TEXT(MSG_TOO_COLD_FOR_M600_2));
				STATIC_ITEM(GET_TEXT(MSG_TOO_COLD_FOR_M600_3));
				STATIC_ITEM(GET_TEXT(MSG_TOO_COLD_FOR_M600_4));
			WAIT_FOR_CLICK();
			END_MENU();
		}

		void NextionLCD::nex_enqueue_filament_change() {
			#if ENABLED(PREVENT_COLD_EXTRUSION)
				if (!DEBUGGING(DRYRUN) && !thermalManager.allow_cold_extrude &&
					thermalManager.degTargetHotend(active_extruder) < thermalManager.extrude_min_temp) {
					buzzer.tone(120, 700); // dodane beeper too cold
					buzzer.tone(120, 000);
					buzzer.tone(120, 700);
					lcd_advanced_pause_toocold_menu();
					return;
				}
			#endif
			PageSelect.show();
			queue.enqueue_now_P("M600 B0");
		}

    void NextionLCD::lcd_advanced_pause_resume_print() {
      pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT;
			ui.return_to_status();
      PagePrinter.show();
    }

    void NextionLCD::lcd_advanced_pause_extrude_more() {
      pause_menu_response = PAUSE_RESPONSE_EXTRUDE_MORE;
    }

    void NextionLCD::lcd_advanced_pause_option_menu() {
      START_MENU();
      STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_OPTION_HEADER));
      MENU_ITEM(function, GET_TEXT(MSG_FILAMENT_CHANGE_OPTION_RESUME), nexlcd.lcd_advanced_pause_resume_print);
      MENU_ITEM(function, GET_TEXT(MSG_FILAMENT_CHANGE_OPTION_PURGE), nexlcd.lcd_advanced_pause_extrude_more);
      END_MENU();
    }

    void NextionLCD::lcd_advanced_pause_init_message() {
      START_SCREEN();
				STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_HEADER));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_INIT_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_INIT_2));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_unload_message() {
      START_SCREEN();
		STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_HEADER));
		STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_UNLOAD_1));
		STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_UNLOAD_2));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_wait_for_nozzles_to_heat() {
      START_SCREEN();
		STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_HEADER));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING_2));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_heat_nozzle() {
      START_SCREEN();
		STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_HEADER));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_HEAT_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_HEAT_2));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_insert_message() {
      START_SCREEN();
		STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_HEADER));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_INSERT_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_INSERT_2));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_INSERT_3));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_load_message() {
      START_SCREEN();
		STATIC_ITEM(GET_TEXT(MSG_NEX_FILAMENT_CHANGE_HEADER));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_LOAD_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_LOAD_2));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_purge_message() {
      START_SCREEN();
			//STATIC_ITEM(MSG_NEX_FILAMENT_CHANGE_HEADER); usuniete bo przy pauzie rowniez bylo wyswietlane
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_EXTRUDE_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_EXTRUDE_2));
      END_SCREEN();
    }

    void NextionLCD::lcd_advanced_pause_resume_message() {
      START_SCREEN();
		//STATIC_ITEM(MSG_NEX_FILAMENT_CHANGE_HEADER); jw.
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_RESUME_1));
      	STATIC_ITEM(GET_TEXT(MSG_FILAMENT_CHANGE_RESUME_2));
      END_SCREEN();
    }

    void lcd_pause_show_message(const PauseMessage message,
																const PauseMode mode/*=PAUSE_MODE_SAME*/,
																const uint8_t extruder/*=active_extruder*/)
			//const PauseMessage message,const PauseMode mode, const uint8_t extruder) 
		{
      //UNUSED(extruder);
      static PauseMessage old_message;
      //advanced_pause_mode = mode;

      if (old_message != message) {
				nex_m600_heatingup = 0;//zmiana jesli wyjdzie poza heatingup ????
				    
        switch (message) {
          case PAUSE_MESSAGE_PARKING:
						SERIAL_ECHOLN("PARKING:");
            nexlcd.lcd_advanced_pause_init_message();
            break;
          case PAUSE_MESSAGE_CHANGING:
						SERIAL_ECHOLN("CHANGING:??");
						nexlcd.lcd_advanced_pause_init_message();
            break;
          case PAUSE_MESSAGE_UNLOAD:
						nexlcd.lcd_advanced_pause_unload_message();
						SERIAL_ECHOLN("UNLOAD:??");
            break;
          case PAUSE_MESSAGE_WAITING:
						SERIAL_ECHOLN("WAITING:??");
            break;
          case PAUSE_MESSAGE_INSERT:
						nexlcd.lcd_advanced_pause_insert_message();
						SERIAL_ECHOLN("INSERT:??");
            break;
          case PAUSE_MESSAGE_LOAD:
						nexlcd.lcd_advanced_pause_load_message();
						SERIAL_ECHOLN("LOAD:??");
            break;
          case PAUSE_MESSAGE_PURGE:
						nexlcd.lcd_advanced_pause_purge_message();
						SERIAL_ECHOLN("PURGE:??");
            break;
          case PAUSE_MESSAGE_RESUME:
						//nex_m600_heatingup = 1;
						nexlcd.lcd_advanced_pause_resume_message();
           	SERIAL_ECHOLN("RESUME:??");
            break;
					case PAUSE_MESSAGE_HEAT:
           	SERIAL_ECHOLN("HEAT:??");
            break;
          case PAUSE_MESSAGE_HEATING:
           	SERIAL_ECHOLN("HEATING:??");
						 nexlcd.lcd_advanced_pause_wait_for_nozzles_to_heat();
            break;
          case PAUSE_MESSAGE_OPTION:
						SERIAL_ECHOLN("OPTION:??");
            pause_menu_response = PAUSE_RESPONSE_WAIT_FOR;
            nexlcd.lcd_advanced_pause_option_menu();
            break;
          case PAUSE_MESSAGE_STATUS:
						
           	SERIAL_ECHOLN("STATUS:??");
            break;
          default:
            PagePrinter.show();
            break;
        }
        old_message = message;
      }
    }

  #endif // ADVANCED_PAUSE_FEATURE
// =============================
// END OF FILAMENT CHANGE M600
// =============================


// ======================
// VLCS LCD SUPPORT
// ======================
#ifdef PLOSS_SUPPORT
		// ODPOWIEDZI
		void lcd_ploss_menu_response_yes() {
			lcd_ploss_menu_response = PLOSS_LCD_RESPONSE_YES;
		}
		void lcd_ploss_menu_response_no() {
			lcd_ploss_menu_response = PLOSS_LCD_RESPONSE_NO;
		}

		//MENU
		void ploss_recovery_menu() {
			START_MENU();
			STATIC_ITEM("Wykryto zanik napiecia");
			STATIC_ITEM("Czy wznowic wydruk?");
			MENU_ITEM(function, "Tak", lcd_ploss_menu_response_yes);
			MENU_ITEM(function, "Nie", lcd_ploss_menu_response_no);
			END_MENU();
		}

		void ploss_recovery_menu_resuming() {
			PageSelect.show();
			screen_timeout_millis = millis(); //wlaczamy screen timeout
			START_SCREEN();
			LcdSend.SetVisibility(false);
			STATIC_ITEM("Wznawianie wydruku");
			STATIC_ITEM("po zaniku zasilania");
			END_SCREEN();
	}

		void ploss_recovery_menu_last_confirm() {
			PageSelect.show();
			screen_timeout_millis = 0; // wylaczamy screen timeout
			START_SCREEN();
			STATIC_ITEM("Usun nadmiar");
			STATIC_ITEM("filamentu i kliknij");
			STATIC_ITEM("aby wznowic wydruk");
			WAIT_FOR_CLICK_F(function, lcd_ploss_menu_response_yes); // return response if clicked
			END_MENU();
		}

		void lcd_ploss_menu_last_info() {
			START_SCREEN();
			STATIC_ITEM("Wznawianie wydruku");
			STATIC_ITEM("");
			END_SCREEN();
			screen_timeout_millis = millis();
		}

		void ploss_recovery_menu_no_resume() {
			screen_timeout_millis = millis();
			START_SCREEN();
			STATIC_ITEM("Uruchamianie");
			STATIC_ITEM("po zaniku zasilania");
			STATIC_ITEM("Bazowanie osi");
			END_SCREEN();
		}

		void lcd_ploss_recovery_menu(const PlossMenuMessage message) {
			switch (message) {
			case PLOSS_LCD_MANUAL_RECOVERY:
				lcd_ploss_menu_response = PLOSS_LCD_RESPONSE_WAIT_FOR_USER;
				ploss_recovery_menu();
				break;
			case PLOSS_LCD_RECOVERY_RESUMING:
				ploss_recovery_menu_resuming();
				break;
			case PLOSS_LCD_MENU_NO_RESUME:
				ploss_recovery_menu_no_resume();
				//KATT enqueue_and_echo_commands_P(PSTR("G28"));
				break;
			case PLOSS_LCD_MENU_LAST_CONFIRM:
				lcd_ploss_menu_response = PLOSS_LCD_RESPONSE_WAIT_FOR_LAST_CONFIRMATION;
				ploss_recovery_menu_last_confirm();
				break;
			case PLOSS_LAST_INFO:
				lcd_ploss_menu_last_info();
				break;
			case PLOSS_LCD_AUTO_RECOVERY:
				break;
			}
		}
#endif //PLOSS
// ======================
// VLCS LCD SUPPORT   ===
// ======================


  #if ENABLED(RFID_MODULE)
    void rfidPopCallback(void *ptr) {
      ZERO(bufferson);

      String temp = "M522 ";
      uint16_t Rfid_read = RfidR.getValue();

      if (ptr == &Rfid0)
        temp += "T0 ";
      else if (ptr == &Rfid1)
        temp += "T1 ";
      else if (ptr == &Rfid2)
        temp += "T2 ";
      else if (ptr == &Rfid3)
        temp += "T3 ";
      else if (ptr == &Rfid4)
        temp += "T4 ";
      else if (ptr == &Rfid5)
        temp += "T5 ";

      if(Rfid_read)
        temp += "R";
      else
        temp += "W";

      temp.toCharArray(bufferson, sizeof(bufferson));
      commands.enqueue_and_echo(bufferson);
    }

    void rfid_setText(const char* message, uint32_t color /* = 65535 */) {
      char Rfid_status_message[25];
      strncpy(Rfid_status_message, message, 30);
      RfidText.Set_font_color_pco(color);
      RfidText.setText(Rfid_status_message);
    }
  #endif

	/**
	 * 	BED LEVELING SUPPORT
	 */
	#if ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
    void NextionLCD::ProbelPopCallBack(void *ptr){
      if (ptr == &ProbeUp || ptr == &ProbeDown) {

				set_destination_to_current();

        if (ptr == &ProbeUp){
          destination[Z_AXIS] += (MESH_EDIT_Z_STEP); }
        else{
          destination[Z_AXIS] -= (MESH_EDIT_Z_STEP); }

        NOLESS(destination[Z_AXIS], -(LCD_PROBE_Z_RANGE) * 0.5);
        NOMORE(destination[Z_AXIS], (LCD_PROBE_Z_RANGE) * 0.5);

				//ZMIANA W OSTATNIM BUGFIXIE manual feedrate mm m -> manual_feedrate_mm_s
        const float old_feedrate = feedrate_mm_s;
        feedrate_mm_s = manual_feedrate_mm_s[Z_AXIS];
        prepare_line_to_destination(); // will call set_current_from_destination()
        feedrate_mm_s = old_feedrate;

        planner.synchronize(); //przesuniecie metody synchronize ze stepper do planner
      }
      
			else if (ptr == &ProbeSend) {
        #if HAS_LEVELING && ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
				//if (g29_in_progress == true) {
					queue.inject_P("G29 S2");
				//}
        #endif
					wait_for_user = false;
      }
    }
	#endif

	#if ENABLED(NEXTION_DISPLAY)
	void NextionLCD::return_after_leveling(bool finish)
	{
		buzzer.tone(100, 659);
		buzzer.tone(100, 698);

		#if ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
		//nex_ss_state = eeprom_read_byte((uint8_t*)EEPROM_NEX_SS_STATE); // przywroc stan SS sprzed poziomowania
			if (finish == true)
			{
				PagePrinter.show();
			}
		
		#elif ENABLED(NEXTION_AUTO_BED_LEVEL)
		  //enqueue_and_echo_commands_P(PSTR("M500"));  // dodane aby zapisywało poziomowanie podczas trwania funkcji
			//enqueue_and_echo_commands_P(PSTR("G28"));  // dodane aby zapisywało poziomowanie podczas trwania funkcji
			if (finish == true)
			{
				PagePrinter.show();
			}
			//home_all_axes();
		#endif
	}

	#endif
		/**
	 * 	BED LEVELING SUPPORT END
	 */

	// Rozgrzewanie głowicy/blatu/chlodzenie
  void NextionLCD::handle_heatingPopCallback(void *ptr){
    //UNUSED(ptr);
		uint16_t	temp_hotend = temphe.getValue(),
							temp_bed = tempbe.getValue();

		if (ptr == &heatupenter || ptr == &chillenter){		// ROZGRZEJ OBA LUB COOLING
			thermalManager.setTargetHotend(temp_hotend, 0);	
			thermalManager.setTargetBed(temp_bed);
		}
    PagePrinter.show();
		buzzer.tone(100,2300);
  }

	// Wentylator chlodzacy wydruki
	void NextionLCD::handle_fanPage_PopCallback(void *ptr) {
		uint8_t vfanbuff;
		UNUSED(ptr);
		ZERO(bufferson);
		vfanbuff = FanSpeedNex.getValue("fanspeedpage");
		thermalManager.fan_speed[0] = vfanbuff;

		PagePrinter.show();
		buzzer.tone(100, 2300);
	}
	
	#if ENABLED(NEX_STAT_PAGE)
		void NextionLCD::setsetupstatPopCallback(void *ptr){
			UNUSED(ptr);
			// PRINTSTATS START
			char buffer[21];
			printStatistics stats = print_job_timer.getStats();

			Sprints.setText(i16tostr3left(stats.totalPrints),"statscreen");        // Print Count: 999
			Scompl.setText(i16tostr3left(stats.finishedPrints), "statscreen");			// Completed  : 666

			#if ENABLED(PLOSS_SUPPORT)
				Spanic.setText(itostr3left(eeprom_read_byte((uint8_t*)EEPROM_PANIC_POWER_FAIL_COUNT)), "statscreen"); // dodane power fail count
			#endif

			duration_t elapsed = stats.printTime;
			elapsed.toString(buffer);
			Stimetotal.setText(buffer, "statscreen");               // Total print Time: 99y 364d 23h 59m 59s

			elapsed = stats.longestPrint;
			elapsed.toString(buffer);
			Stimelong.setText(buffer, "statscreen");								// Longest job time: 99y 364d 23h 59m 59s

			sprintf_P(buffer, PSTR("%ld.%im"), long(stats.filamentUsed / 1000), int16_t(stats.filamentUsed / 100) % 10);
			Sfilament.setText(buffer, "statscreen");								// Extruded total: 125m
			// END OF PRINTSTATS

			// PRINTER INFO START
			Sfirmware.setText_PGM(PSTR(SHORT_BUILD_VERSION), "statscreen");
			Skompil.setText_PGM(PSTR(STRING_DISTRIBUTION_DATE), "statscreen");

			#if ENABLED(NEXTION_BED_SEMIAUTO_LEVEL)
				Sleveling.setText_PGM(GET_TEXT(MSG_MESH_LEVELING), "statscreen");
			#elif ENABLED(NEXTION_AUTO_BED_LEVEL)
				Sleveling.setText_PGM(GET_TEXT(MSG_BILINEAR_LEVELING), "statscreen");
			#endif

			#if ENABLED(PLOSS_SUPPORT)
						Svlcs.setText_PGM(GET_TEXT(MSG_YES), "statscreen");
			#else
						Svlcs.setText_PGM(GET_TEXT(MSG_NO), "statscreen");
			#endif

			#if ENABLED(FILAMENT_RUNOUT_SENSOR)
						if (eeprom_read_byte((uint8_t*)EEPROM_NEX_FILAMENT_SENSOR) == 1)
						{
							Sfilsensor.setText_PGM(GET_TEXT(MSG_YES), "statscreen");
						}
						else
						{
							Sfilsensor.setText_PGM(GET_TEXT(MSG_NO), "statscreen");
						}
			#else
						Sfilsensor.setText_PGM(GET_TEXT(MSG_NO), "statscreen");
			#endif			
			// END OF PRINTER INFO
		}

	#endif

	#if ENABLED(NEX_ACC_PAGE)
		void NextionLCD::setaccelpagePopCallback(void *ptr){
			UNUSED(ptr); 
			Awork.setValue(planner.settings.acceleration, "accelpage"); //va0
			Aretr.setValue(planner.settings.retract_acceleration, "accelpage");	//va1
			Atravel.setValue(planner.settings.travel_acceleration, "accelpage");
			Amaxx.setValue(planner.settings.max_acceleration_mm_per_s2[X_AXIS], "accelpage");
			Amaxy.setValue(planner.settings.max_acceleration_mm_per_s2[Y_AXIS], "accelpage");
			Amaxz.setValue(planner.settings.max_acceleration_mm_per_s2[Z_AXIS], "accelpage");
			Amaxe.setValue(planner.settings.max_acceleration_mm_per_s2[E_AXIS+active_extruder], "accelpage");
		}
		
		void NextionLCD::getaccelPagePopCallback(void *ptr){
			planner.settings.acceleration = Awork.getValue("accelpage");
			planner.settings.retract_acceleration = Aretr.getValue("accelpage");
			planner.settings.travel_acceleration = Atravel.getValue("accelpage");
			planner.settings.max_acceleration_mm_per_s2[X_AXIS] = Amaxx.getValue("accelpage");
			planner.settings.max_acceleration_mm_per_s2[Y_AXIS] = Amaxy.getValue("accelpage");
			planner.settings.max_acceleration_mm_per_s2[Z_AXIS] = Amaxz.getValue("accelpage");
			planner.settings.max_acceleration_mm_per_s2[E_AXIS + active_extruder] = Amaxe.getValue("accelpage");
		}

		void NextionLCD::setaccelsavebtnPopCallback(void *ptr){
			settings.save();
		}

		void NextionLCD::setaccelloadbtnPopCallback(void *ptr){
			settings.load();
		}
	#endif

	#ifdef CLASSIC_JERK
		void NextionLCD::setjerkpagePopCallback(void *ptr)
		{
				UNUSED(ptr);
				Awork.setValue(planner.max_jerk[X_AXIS], "accelpage"); //va0
				Aretr.setValue(planner.max_jerk[Y_AXIS], "accelpage");	//va1
				Atravel.setValue(planner.max_jerk[Z_AXIS],"accelpage"); //va2
				Amaxx.setValue(planner.max_jerk[E_AXIS],"accelpage");	//va3
		}
	#endif

	#ifdef NEXTION_STEP_SETTINGS
		void setstepspagePopCallback(void *ptr)
		{
				UNUSED(ptr);
				Awork.setValue(planner.axis_steps_per_mm[X_AXIS], "accelpage"); //va0
				Aretr.setValue(planner.axis_steps_per_mm[Y_AXIS], "accelpage");	//va1
				Atravel.setValue(planner.axis_steps_per_mm[Z_AXIS], "accelpage");	//va2
				Amaxx.setValue(planner.axis_steps_per_mm[E_AXIS+active_extruder], "accelpage");	//va3
		}
	#endif

	void NextionLCD::setBabystepUpPopCallback(void *ptr){
		nexlcd.nextion_babystep_z(false);
		int data = 0;
		data = _babystep_z_shift;
		persistentStore.write_data(EEPROM_PANIC_BABYSTEP_Z, (uint8_t*)&data, sizeof(data));

		int data_read = 0;
		persistentStore.read_data(EEPROM_PANIC_BABYSTEP_Z, (uint8_t*)&data_read, sizeof(data_read));
	}

	void NextionLCD::setBabystepDownPopCallback(void *ptr){
		nexlcd.nextion_babystep_z(true);
	}

	void NextionLCD::setBabystepEEPROMPopCallback(void *ptr){
		persistentStore.access_start();
		persistentStore.write_data(EEPROM_PANIC_BABYSTEP_Z, (uint8_t*)&_babystep_z_shift, sizeof(_babystep_z_shift));
		persistentStore.access_finish();		
	}

	void NextionLCD::setspeedPopCallback(void *ptr) {
		int vspeedbuff;
		UNUSED(ptr);
		vspeedbuff = (int)SpeedNex.getValue("speed");

		feedrate_percentage = vspeedbuff;
		PagePrinter.show();
	}

	void NextionLCD::setflowPopCallback(void *ptr){
		uint8_t flowfrom;
		int vflowbuff;
		UNUSED(ptr);
		vflowbuff = (int)vFlowNex.getValue("flowpage");
		flowfrom = FlowPageFrom.getValue("flowpage");

		//KATT flow_percentage[0] = vflowbuff;

		if (flowfrom == 0) // wejscie z status
		{
			PagePrinter.show();
		}
		else if (flowfrom == 1) // wejscie z heatup
		{
			PageOptions.show();
		}
	}

  void NextionLCD::setgcodePopCallback(void *ptr) {
    UNUSED(ptr);
    ZERO(bufferson);
		SERIAL_ECHOLNPGM("setgcodepopcallbac");
    Tgcode.getText(bufferson, sizeof(bufferson), "gcode");
    Tgcode.setText("", "gcode");

		if (strcmp(bufferson,"M600") == 0)
		{
			nexlcd.nex_enqueue_filament_change();
			buzzer.tone(100, 2300);
		}
		else if (strcmp(bufferson, "M78 S78") == 0)
		{
			queue.inject_P(bufferson);
			buzzer.tone(100, 2300);
		}
		else if(strcmp(bufferson, "G29 S1") == 0) // musimy wylapac komende z nextiona zanim trafi do parsera
		{																					// inaczej trzeba bedzie miec osobne wsady do NEXa z auto i semi levelingiem
			#if ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
				queue.inject_P(bufferson);
				Pprobe.show();		// pokaz ekran semiauto leveling
			#endif
			
			#if ENABLED(NEXTION_AUTO_BED_LEVEL)
				queue.enqueue_now_P("G28");	// bazowanie przed poziomowaniem
				queue.enqueue_now_P("G29"); // poziomowanie auto

				Palevel.show();										// pokaz ekran auto leveling
			#endif
		}
		else
		{ 
			queue.inject_P(bufferson);
			buzzer.tone(100, 2300);
		}
  }

  void NextionLCD::setmovePopCallback(void *ptr) {
    UNUSED(ptr);
    ZERO(bufferson);
    movecmd.getText(bufferson, sizeof(bufferson), "move");
		SERIAL_ECHOLN(bufferson);
		queue.enqueue_now_P("G91");			//enqueue now zamiast injectP... A pytaj mnie czego.. 
		queue.enqueue_now_P(bufferson);
		queue.enqueue_now_P("G90");
  }

  void NextionLCD::motoroffPopCallback(void *ptr) {
    UNUSED(ptr);
		queue.inject_P("M84");
  }

	// NEXTION: Obsluga klikniecia przycisku SEND na SELECT PAGE
  void NextionLCD::sendPopCallback(void *ptr) {
    UNUSED(ptr);
    nexlcd.lcd_clicked = true;
		wait_for_user = false;

		// dodane aby wyjsc kliknieciem z ostatniego ekranu vlcs / oraz z ekranu too cold for extrude m600
		if (screen_timeout_millis != 0)
		{
			screen_timeout_millis = 0;
			PagePrinter.show();
		}
  }

	// NEXTION: Obsluga klikniecia YES oraz NO
  void NextionLCD::YesNoPopCallback(void *ptr) {
    if (ptr == &Yes) {
      switch(Vyes.getValue()) {
        #if ENABLED(SDSUPPORT)
          case 1: // Stop Print
						PagePrinter.show();
						nexlcd.nex_stop_printing();
						ui.set_status_P(GET_TEXT(MSG_PRINT_ABORTED), -1);	// status bar info
            break;
          case 2: // Upload Firmware
						#if ENABLED(NEX_UPLOAD)
            UploadNewFirmware(); 
						#endif
						break;
        #endif
        #if HAS_SD_RESTART
          case 3: // Restart file
            PagePrinter.show();
            restart.start_job();
            break;
        #endif
        case 4: // Unconditional stop
          PagePrinter.show();
          break;
				case 5: // ustaw czujnik filamentu
					nex_filament_runout_sensor_flag = 1;
					persistentStore.write_data(EEPROM_NEX_FILAMENT_SENSOR, 1);
					PageSetup.show();
					break;
        default: break;
      }
    }
    else {
      switch(Vyes.getValue()) {
        #if ENABLED(SDSUPPORT)
          case 2:
            PageSetup.show(); break;
        #endif
        #if HAS_SD_RESTART
          case 3:
            card.printingHasFinished();
            PagePrinter.show();
            break;
        #endif
					case 5: // ustaw czujnik filamentu
						nex_filament_runout_sensor_flag = 0;
						persistentStore.write_data(EEPROM_NEX_FILAMENT_SENSOR, (uint8_t)0);
						PageSetup.show();
						break;
        default:
          PagePrinter.show(); break;
      }
    }
  }


// =======================
// ==	LCD INIT					==
// =======================
// Connect
void NextionLCD::connect(){
		for (uint8_t i = 0; i < 5; i++) {
		ZERO(bufferson);
		NextionON = nexInit(bufferson);
		if (NextionON) break;
		delay(10);
	}
	Pstart.show(); // show boot screen

	#if ENABLED(MKS_SKR)
		nexlcd.sendRandomSplashMessage();
	#endif

	if (!NextionON) { SERIAL_ECHOLNPGM("Nextion NOT connected.."); buzzer.tone(220, 700); return; }
	else {
		SERIAL_ECHOLNPGM("Nextion connected!");
	}

	#if ENABLED(MKS_SKR)
		nexlcd.sendRandomSplashMessage(); 		// Funkcja ma wysylac randomowa liczbe dla nextiona ktory na jej podstawie wyswietli mesydz
	#endif
}


// ===========================
// == Random Splash Message ==
// ===========================
#if ENABLED(MKS_SKR)
	void NextionLCD::setRandomSeed()
	{
		int r = 0;
		for( int i=2; i<=64; i++)
		{
			r += analogRead(i);
		}
		randomSeed(r);
	}

void NextionLCD::sendRandomSplashMessage(){
	int32_t randtemp = random(1,21);

	switch(randtemp)
	{
		case 1: splashText.setText(MSG_SPLASH_1, STARTPAGE); break;
		case 2: splashText.setText(MSG_SPLASH_2, STARTPAGE); break;
		case 3: splashText.setText(MSG_SPLASH_3, STARTPAGE); break;
		case 4: splashText.setText(MSG_SPLASH_4, STARTPAGE); break;
		case 5: splashText.setText(MSG_SPLASH_5, STARTPAGE); break;
		case 6: splashText.setText(MSG_SPLASH_6, STARTPAGE); break;
		case 7: splashText.setText(MSG_SPLASH_7, STARTPAGE); break;
		case 8: splashText.setText(MSG_SPLASH_8, STARTPAGE); break;
		case 9: splashText.setText(MSG_SPLASH_9, STARTPAGE); break;
		case 10: splashText.setText(MSG_SPLASH_10, STARTPAGE); break;
		case 11: splashText.setText(MSG_SPLASH_11, STARTPAGE); break;
		case 12: splashText.setText(MSG_SPLASH_12, STARTPAGE); break;
		case 13: splashText.setText(MSG_SPLASH_13, STARTPAGE); break;
		case 14: splashText.setText(MSG_SPLASH_14, STARTPAGE); break;
		case 15: splashText.setText(MSG_SPLASH_15, STARTPAGE); break;
		case 16: splashText.setText(MSG_SPLASH_16, STARTPAGE); break;
		case 17: splashText.setText(MSG_SPLASH_17, STARTPAGE); break;
		case 18: splashText.setText(MSG_SPLASH_18, STARTPAGE); break;
		case 19: splashText.setText(MSG_SPLASH_19, STARTPAGE); break;
		case 20: splashText.setText(MSG_SPLASH_20, STARTPAGE); break;
		case 21: splashText.setText(MSG_SPLASH_21, STARTPAGE); break;
		case 22: splashText.setText(MSG_SPLASH_22, STARTPAGE); break;
	}
}
#endif

// =======================
// == SETUP CALLBACKS		==
// =======================
void NextionLCD::setup_callbacks(){
	//
	// NEX USTAWIENIE PRZYCISKOW
	//
	// SDSUPPORT
	#if ENABLED(SDSUPPORT)
		sdscrollbar.attachPop(sdlistPopCallback);
		ScrollUp.attachPop(sdlistPopCallback);
		ScrollDown.attachPop(sdlistPopCallback);
		NPlay.attachPop(PlayPausePopCallback);
	#endif

	// BED LEVEL
	#if ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
		ProbeUp.attachPush(ProbelPopCallBack, &ProbeUp);
		ProbeSend.attachPop(ProbelPopCallBack, &ProbeSend);
		ProbeDown.attachPush(ProbelPopCallBack, &ProbeDown);
	#endif

	// STATS
	#if ENABLED(NEX_STAT_PAGE)
		statin.attachPop(setsetupstatPopCallback); //dodane info o wejsciu w statystyki
	#endif

	// ACCELERATION
	#if ENABLED(NEX_ACC_PAGE)
		accelin.attachPop(setaccelpagePopCallback); //setaccelpagePopCallback
		Asend.attachPop(getaccelPagePopCallback);
		Asave.attachPop(setaccelsavebtnPopCallback);
		Aload.attachPop(setaccelloadbtnPopCallback);
	#endif

	#ifdef NEXTION_STEP_SETTINGS
	//KATT SvSteps.attachPop(setstepspagePopCallback);
	#endif

	// TEMPERATURA
	heatupenter.attachPop(handle_heatingPopCallback, &heatupenter); // obsluga przycisku rozgrzej oba
	chillenter.attachPop(handle_heatingPopCallback, &chillenter); //obsluga przycisku chlodzenie

	FanSetBtn.attachPop(handle_fanPage_PopCallback); //obsluga przycisku fan set
	speedsetbtn.attachPop(setspeedPopCallback); //obsluga przycisku speed set
	SetFlowBtn.attachPop(setflowPopCallback); //obsluga przycisku set flow

	// BABYSTEP
	ZbabyUp.attachPush(setBabystepUpPopCallback);	// obsluga przycisku babystep up
	ZbabyDown.attachPush(setBabystepDownPopCallback); // obsluga przycisku babystep down
	ZbabyBack_Save.attachPop(setBabystepEEPROMPopCallback);
	
	// MOVE PAGE
	XYHome.attachPop(setmovePopCallback);
	XYUp.attachPush(setmovePopCallback);
	XYRight.attachPush(setmovePopCallback);
	XYDown.attachPush(setmovePopCallback);
	XYLeft.attachPush(setmovePopCallback);
	ZHome.attachPop(setmovePopCallback);
	ZUp.attachPush(setmovePopCallback);
	ZDown.attachPush(setmovePopCallback);
	Extrude.attachPush(setmovePopCallback);
	Retract.attachPush(setmovePopCallback);
	MotorOff.attachPop(motoroffPopCallback);

	// GCODE
	Send.attachPop(setgcodePopCallback); //SERIAL_ECHOLN("attachpop-SEND.");

	// YESNO
	Yes.attachPop(YesNoPopCallback, &Yes);
	No.attachPop(YesNoPopCallback, &No);

	// SELECT PAGE
	LcdSend.attachPop(sendPopCallback);
}

// =======================
// == LCD INIT					==
// =======================
void NextionLCD::init(){

	nexlcd.connect();
	if(!NextionON)
	{
	}
	
	nexlcd.setup_callbacks();

	#if ENABLED(FSENSOR_STATE)
		nex_filament_runout_sensor_flag = eeprom_read_byte((uint8_t*)EEPROM_NEX_FILAMENT_SENSOR);
	#endif

	#if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)
		SET_INPUT_PULLUP(SD_DETECT_PIN);
		lcd_sd_status = 2; // UNKNOWN
	#endif

	#if ENABLED(NEXTION_GFX)
		gfx.color_set(NX_AXIS + X_AXIS, 63488);
		gfx.color_set(NX_AXIS + Y_AXIS, 2016);
		gfx.color_set(NX_AXIS + Z_AXIS, 31);
		gfx.color_set(NX_MOVE, 2047);
		gfx.color_set(NX_TOOL, 65535);
		gfx.color_set(NX_LOW, 2047);
		gfx.color_set(NX_HIGH, 63488);
	#endif

	nexlcd.setpage_Status();
	//splashTimer.enable();

	buzzer.tone(100, 2300); // dodane - wejsciowy brzeczyk
	buzzer.tone(100, 2600);
	buzzer.tone(100, 3100);			
	//delay(1000);

	delay(3000);
	PagePrinter.show();
}
// =======================
// == END OF	LCD INIT	==
// =======================

  

	// Wysyla aktualne temperatury do NEX
  static void degtoLCD(const uint8_t h, float temp) {
    NOMORE(temp, 350);//999

    heater_list0[h]->setValue(temp,"stat");

    #if ENABLED(NEXTION_GFX)
      if (!(print_job_counter.isRunning() || card.isPrinting) && !Wavetemp.getObjVis() && show_Wave) {
        Wavetemp.SetVisibility(true);
      }
    #endif
  }

	// Wysyla docelowe temperatury do NEX
  static void targetdegtoLCD(const uint8_t h, const uint16_t temp) {
    heater_list1[h]->setValue(temp);
  }


	// Wysyla koordynaty do do NEX / MOVE PAGE / STATUS / BEDLEVEL
  static void coordtoLCD() {
    const char* valuetemp;
		static xyze_pos_t temppos;

    ZERO(bufferson);
    if (PageID == EPageStatus) {
			// if sprawdza czy nastapila zmiana pozycji aby nie spamowalo po serialu pozycja bez zmian -> todo: przeniesc na 8 bit... OK
			if( current_position[X_AXIS] != temppos[X_AXIS] )
			{
				LcdX.setText(ftostr41sign(LOGICAL_X_POSITION(current_position[X_AXIS])),"stat");
				temppos[X_AXIS] = current_position[X_AXIS];
			}
			if( current_position[Y_AXIS] != temppos[Y_AXIS] )
			{
				LcdY.setText(ftostr41sign(LOGICAL_Y_POSITION(current_position[Y_AXIS])),"stat");
				temppos[Y_AXIS] = current_position[Y_AXIS];
			}
			if( current_position[Z_AXIS] != temppos[Z_AXIS] )
			{
				LcdZ.setText(ftostr41sign(LOGICAL_Z_POSITION(current_position[Z_AXIS])),"stat");
				temppos[Z_AXIS] = current_position[Z_AXIS];
			}
    }
    else if (PageID == EPageMove) {
      if (all_axes_homed) {
				valuetemp = ftostr4sign(LOGICAL_X_POSITION(current_position[X_AXIS]));
				strcat(bufferson, "X");
				strcat(bufferson, valuetemp);
      }
      else
        strcat(bufferson, "?");

      if (all_axes_homed) {
        valuetemp = ftostr4sign(LOGICAL_Y_POSITION(current_position[Y_AXIS]));
        strcat(bufferson, " Y");
        strcat(bufferson, valuetemp);
      }
      else
        strcat(bufferson, " ?");

      if (all_axes_homed) {
        valuetemp = ftostr52sp(FIXFLOAT(LOGICAL_Z_POSITION(current_position[Z_AXIS])));
        strcat(bufferson, " Z");
        strcat(bufferson, valuetemp);
      }
      else
        strcat(bufferson, " ?");

			// aby w move menu nie spamowalo po serialu:
			if(current_position[X_AXIS] != temppos[X_AXIS] || current_position[Y_AXIS] != temppos[Y_AXIS] || current_position[Z_AXIS] != temppos[Z_AXIS] )
			{
      	LedCoord5.setText(bufferson,"move");
				temppos[X_AXIS] = current_position[X_AXIS];
				temppos[Y_AXIS] = current_position[Y_AXIS];
				temppos[Z_AXIS] = current_position[Z_AXIS];
			}
    }
    else if (PageID == EPageBedlevel) {
      ProbeZ.setText(ftostr43sign(FIXFLOAT(LOGICAL_Z_POSITION(current_position[Z_AXIS]))),"bedlevel");
    }
		else if (PageID == EPageBedlevelAuto)
		{
			ProbeZ.setText(ftostr43sign(FIXFLOAT(LOGICAL_Z_POSITION(current_position[Z_AXIS]))),"ABL");
		}
  }

	// odswieza procent druku, progress bar, oraz czas trwania i pozostaly
	// uzywane na ekranie stat
	static void ref_stat_printprogress()
	{
		// wpierw procenty i progress bar
		ZERO(bufferson);
		strcat(bufferson, ui8tostr3rj(card.percentDone()));		// progress printing mozna chyba w calym pliku zamienic na card.percentDone();
		strcat(bufferson, " %");
		percentdone.setText(bufferson, "stat");						// procenty
		progressbar.setValue(card.percentDone(), "stat"); 	// progressbar

		// nastepnie czas trwania druku i pozostaly
		ZERO(bufferson);
		char buffer1[10];
		uint8_t digit;
		duration_t Time = print_job_timer.duration();
		digit = Time.toDigital(buffer1, true);
		strcat(bufferson, "Start: ");
		strcat(bufferson, buffer1);
		Time = (print_job_timer.duration() * (100 - card.percentDone())) / (card.percentDone() + 0.1);
		digit += Time.toDigital(buffer1, true);
		if (digit > 14)
			strcat(bufferson, ", Left: ");
		else
			strcat(bufferson, ", Left: ");
		strcat(bufferson, buffer1);
		LcdTimeElapsed.setText(bufferson,"stat");

		// todo: pasowaloby zeby nie spamowalo gdy pozostaly czas jest taki sam...
	}

	// Sprawdza obecnosc karty SD i montuje/odmontowuje karte na ekranie
	// IS_SD_INSERTED ma odwrocona logike:
	// 1 - brak karty
	// 0 - karta wlozona
	/*
	void NextionLCD::nex_check_sdcard_present()
	{
		#if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)
			const bool sd_status = IS_SD_INSERTED();												// IS_SD_INSERTED zwraca 0 jeśli prawda.
			if (sd_status != lcd_sd_status && lcd_detected())								// sprawdz czy nastapila zmiana? SD DET ->
			{																																// TAK:
				if (!sd_status)		// jesli SD_DETECT == false:
				{
					SERIAL_ECHOLNPGM("sd:false");
					card.mount();																														// inicjalizacja karty
					SDstatus = SD_INSERT;																										// flaga
					SD.setValue(SDstatus, "stat");																					// przekazana do nex
					if (lcd_sd_status != 2) ui.set_status_P(GET_TEXT(MSG_MEDIA_INSERTED));	// MSG
					if (PageID == EPageSD){ setpageSD(); }																	// ustaw strone i przekaz flage do strony status
				}
				else							// jesli SD_DETECT == true:
				{
					SERIAL_ECHOLNPGM("sd:true");
					card.release();																												// odmontuj karte SD
					SDstatus = SD_NO_INSERT;																							// flaga
					SD.setValue(SDstatus, "stat");																				// przekazana do nex
					if (lcd_sd_status != 2) ui.set_status_P(GET_TEXT(MSG_MEDIA_REMOVED));	// MSG
					if (PageID == EPageSD){ setpageSD(); }																// ustaw strone i przekaz flage do strony status
				}
				lcd_sd_status = sd_status;
			} // CALY IF SPRAWDZA STAN SD_DETECT I JEGO ZMIANE: SD jest->init / SD niet->release
		#endif
	}
	void NextionLCD::nex_update_sd_status()
	{
		#if ENABLED(SDSUPPORT)
			if (card.isFileOpen()) {
				if (IS_SD_PRINTING() && SDstatus != SD_PRINTING) {
					SDstatus = SD_PRINTING;
					SD.setValue(SDstatus,"stat");
				}
				else if (!IS_SD_PRINTING() && SDstatus != SD_PAUSE) {
					SDstatus = SD_PAUSE;
					SD.setValue(SDstatus,"stat");
				}
			}
			else if (card.isMounted && SDstatus != SD_INSERT) {
				SDstatus = SD_INSERT;
				SD.setValue(SDstatus,"stat");
			}
			else if (!card.isMounted && SDstatus != SD_NO_INSERT) {
				SDstatus = SD_NO_INSERT;
				SD.setValue(SDstatus,"stat");
			}
		#endif // HAS_SD_SUPPORT
	}*/

// =======================
// == LCD UPDATE				==
// =======================
	void NextionLCD::update(){
		if (!NextionON) return;
    nexLoop(nex_listen_list); // odswieza sie z delayem 5 ms

		//sprawdzamy timeout ekranu
		millis_t timeout_check;
		timeout_check = millis();
		if (timeout_check > screen_timeout_millis + NEX_SCREEN_TIME && screen_timeout_millis != 0)
		{
			PagePrinter.show();
			screen_timeout_millis = 0;
		}
	}

	void NextionLCD::check_periodical_actions(){
		static millis_t cycle_1s = 0;
		const millis_t now = millis();
		
		if (ELAPSED(now, cycle_1s)) {
			#if ENABLED(MKS_GEN)
				cycle_1s = now + 1000UL; // zmianka z 1000UL
			#elif ENABLED(MKS_SKR)
				cycle_1s = now + 200UL; // zmianka z 1000UL
			#endif
			nextion_draw_update();
		}
	}
// ===========================
// == LCD PERIODICAL UPDATE	==
// ===========================
// ODSWIEZANE 0.2s ()
  void NextionLCD::nextion_draw_update() {
    static uint8_t  	PreviousPage = 0,					// strona nex
                    	PreviousfanSpeed = 0,			// dotychczasowa predkosc wentylatora
											Previousflow = 0,					// dotychczasowy flow
											iterTimeLeft = 0,					// licznik - odswieza time left co ktorys raz.
                    	PreviouspercentDone = 0;	// dotychczasowy postep %
		static uint16_t 	Previousfeedrate = 0;			// dotychczasowa predkosc druku

    static float   		PreviousBedTemp = 0,
                    	PreviousTargetBedTemp = 0;
		static float			PreviousHotendTemp = 0,
											PreviousTargetHotendTemp = 0;

    if (!NextionON) return;

    PageID = Nextion_PageID();																		// sprawdz strone
		if(PageID == 100 || PageID == 101)	PageID = PreviousPage;		// jesli na serialu lipa (przyczyna?) to loop do nastepnej proby

		//nex_check_sdcard_present(); // sprawdz obecnosc karty sd, mount/unmount // potencjalnie tutaj jest bug z odswiezajacym sie ekranem SD 

		// timeout screen saver
		#if ENABLED(NEX_SCREENSAVER)
			if(nex_ss_state == true)
			{
				SERIAL_ECHOPGM("PAGEID:");
				SERIAL_ECHOLN(itostr3left(PageID));
				SERIAL_ECHOPGM("PREV:");
				SERIAL_ECHOLN(itostr3left(PreviousPage));

				if(PageID != 100) // jesli nie zwraca szamba z Nextion_PageID() 
				{
					if(PreviousPage != PageID && PreviousPage != 100) // jesli strona sie zmienila i nie jest zwroconym szambem z Nextion_PageID()
					{
						SERIAL_ECHOLNPGM("Prev != PAGEID");
						nex_ss = millis(); // ustaw SS timeout
					}
					else if(PreviousPage == PageID && nex_ss + (nex_ss_timeout*1000) < millis()) // *1000 bo w eepromie sa zapisywane sekundy zamias ms.
					{
						SERIAL_ECHOLNPGM("Prev == PAGEID &&");
						if(PreviousPage != EPageScreenSaver)// lecisz na screen saver
						{
							nex_ss_pagebefore = PreviousPage;// zapisz poprzednia strone do wyswietlenia po wylaczeniu SS
							Psav.show();		// show screen saver
						}
					}
				}
				SERIAL_ECHOPGM("nex_ss:");
				SERIAL_ECHOLN(nex_ss);
				SERIAL_ECHOPGM("millis:");
				SERIAL_ECHOLN(millis());
			}
			else if(nex_ss_state == false)
			{
				SERIAL_ECHOPGM("nex_ss_state:false");
					// nyc
			}
		#endif

    switch(PageID)
		{
      case EPageStatus: //STATUS PAGE
        if (PreviousPage != EPageStatus) // jednorazowo przy wejsciu w strone STAT
				{
					LcdStatus.setText(lcd_status_message);
          #if ENABLED(NEXTION_GFX)
            #if MECH(DELTA)
              gfx_clear(mechanics.delta_print_radius * 2, mechanics.delta_print_radius * 2, mechanics.delta_height);
            #else
              gfx_clear(X_MAX_POS, Y_MAX_POS, Z_MAX_POS);
            #endif
          #endif

					if(SDstatus == SD_PRINTING || SDstatus == SD_PAUSE)
					{
						ref_stat_printprogress(); // odswieza progres bar, procent i czas trwania
						NexFilename.setText(filename_printing);					// nazwa pliku
					}
				} // jednorazowo przy wejsciu end


				// odswiezane stale
				if(SDstatus == SD_PRINTING || SDstatus == SD_PAUSE)
				{
					if (PreviouspercentDone != card.percentDone()) 
					{
						ref_stat_printprogress();
						PreviouspercentDone = card.percentDone();
						iterTimeLeft = 0;
					}
					// odswiezanie time left itp raz na kilka odswiezen ekranu
					iterTimeLeft++;
					
					if(iterTimeLeft > 10)
					{
						ref_stat_printprogress();
						iterTimeLeft = 0;
					}
				}

				//Wentylator
        if (PreviousfanSpeed != thermalManager.fan_speed[0]) {
					PrinterFanspeed.setValue(((float)(thermalManager.fan_speed[0]) / 255) * 100,"stat");
          PreviousfanSpeed = thermalManager.fan_speed[0];
        }
				//feedrate
        if (Previousfeedrate != feedrate_percentage) {
          VSpeed.setValue(feedrate_percentage,"stat");
          Previousfeedrate = feedrate_percentage;
        }
				//flow
				if (Previousflow != planner.flow_percentage[0]) {
					vFlowNex.setValue(planner.flow_percentage[0], "flowpage");
					Previousflow = planner.flow_percentage[0];
				}
        // HOTEND DOCELOWE I TARGET
        if (PreviousHotendTemp != round(thermalManager.degHotend(0))) // porownaj dotychczasowa z obecna
				{
						PreviousHotendTemp = round(thermalManager.degHotend(0));
            degtoLCD(0, PreviousHotendTemp);
        }
        if (PreviousTargetHotendTemp != thermalManager.degTargetHotend(0)) 
				{
			  		PreviousTargetHotendTemp = thermalManager.degTargetHotend(0);
            targetdegtoLCD(0, PreviousTargetHotendTemp);
        }
        // BED DOCELOWE I TARGET
				#if HAS_TEMP_BED
					if (PreviousBedTemp != round(thermalManager.degBed()))
					{
						PreviousBedTemp = round(thermalManager.degBed());
						degtoLCD(1, PreviousBedTemp);
					}
					if (PreviousTargetBedTemp != thermalManager.degTargetBed()){ 
						PreviousTargetBedTemp = thermalManager.degTargetBed();
						targetdegtoLCD(1, PreviousTargetBedTemp);
					}
				#endif
 
        coordtoLCD();

				//nex_update_sd_status();

				#if HAS_SD_RESTART
          if (restart.count && restart.job_phase == RESTART_IDLE) {
            restart.job_phase = RESTART_MAYBE; // Waiting for a response
            lcd_yesno(3, MSG_RESTART_PRINT, "", MSG_USERWAIT);
          }
        #endif 
        break;

			#if ENABLED(SDSUPPORT)
				case EPageSD: // SD CARD LIST
					if (PreviousPage != EPageSD){
						if(SDstatus == SD_PRINTING || SDstatus == SD_PAUSE)
						{
							// cos gdy drukuje NYC
						}
						else
						{
							setpageSD();
						}
					}
				break;
			#endif
			case EPageHeating:
				//nex_update_sd_status();
				break;
			case EPageMaintain:
				//nex_update_sd_status();
				break;
			case EPageSetup:
				//nex_update_sd_status();
				break;
      case EPageMove: // move page
        coordtoLCD();
        break;
      case EPageSpeed: // speed page
        break;
			case EPageFilament:	// 
				// odswiez temp glowicy na ekranie filament [przyciski]
					degtoLCD(0, thermalManager.degHotend(0));
				break;
			case EPageSelect:	// FILAMENT CHANGE PAGE?
				
				if (nex_m600_heatingup == 1) // pokaz temp glowicy podczas nagrzewania m600 na stronie select
				{
					char temptemp[14];
					strlcpy(temptemp, i8tostr3rj(thermalManager.degHotend(0)), 4);
					strcat(temptemp, PSTR(" / "));
					strcat(temptemp, i8tostr3rj(thermalManager.degTargetHotend(0)));
					LcdRiga4.setText(temptemp);
				}
				break;
      case EPageBedlevel:
				//nex_ss = millis(); // ustaw SS timeout
        coordtoLCD();
        break;
			case EPageFlow:
				vFlowNex.setValue(planner.flow_percentage[0], "flowpage");
				break;
			case EPageScreenSaver:
			/*
				if(PreviousPage != ScreenSaver)
				{
					sendRandomSplashMessage();
					SSprog.setValue(progress_printing); // nie wiadomo jak sie zachowa gdy brak druku
				}
				if (PreviouspercentDone != progress_printing) {
					SSprog.setValue(progress_printing); // nie wiadomo jak sie zachowa gdy brak druku
				}*/
				break;
    }
    PreviousPage = PageID;
  }
	
	void NextionLCD::print_status_msg(const char * const lcdmsg){
		strncpy_P(nexlcd.lcd_status_message, lcdmsg, 48);
		if(PageID == EPageStatus) // Jeżeli strona statusu
		{
			LcdStatus.setText(nexlcd.lcd_status_message, "stat");
		}
	}

  void NextionLCD::lcd_yesno(const uint8_t val, const char* msg1, const char* msg2, const char* msg3) {
    Vyes.setValue(val, "yesno");
    PageYesNo.show();
    Riga0.setText(msg1);
    Riga1.setText(msg2);
    Riga3.setText(msg3);
  }

	void NextionLCD::kill_screen_msg(const char* lcd_msg, PGM_P const component)
	{
		PageKill.show();
		Kmsg.setText_PGM(lcd_msg,"kill");
	}

	// dodana obsluga babystep
	#if ENABLED(BABYSTEPPING)
		void NextionLCD::nextion_babystep_z(bool dir) {
			const int16_t babystep_increment = 8;

			if (dir == true)
			{
				babystep.add_steps(Z_AXIS, babystep_increment);
				_babystep_z_shift += babystep_increment;
			}
			else if (dir == false)
			{
					babystep.add_steps(Z_AXIS, -babystep_increment);
				_babystep_z_shift -= babystep_increment;
			}
		}
	#endif

  #if ENABLED(NEXTION_GFX)
    void NextionLCD::gfx_origin(const float x, const float y, const float z) {
      gfx.origin(x, y, z);
    }

    void NextionLCD::gfx_scale(const float scale) {
      gfx.set_scale(scale);
    }

    void NextionLCD::gfx_clear(const float x, const float y, const float z, bool force_clear) {
      if (PageID == EPageStatus && (print_job_counter.isRunning() || card.isPrinting || force_clear)) {
        Wavetemp.SetVisibility(false);
        show_Wave = !force_clear;
        gfx.clear(x, y, z);
      }
    }

    void NextionLCD::gfx_cursor_to(const float x, const float y, const float z, bool force_cursor) {
      if (PageID == EPageStatus && (print_job_counter.isRunning() || card.isPrinting || force_cursor))
        gfx.cursor_to(x, y, z);
    }

    void NextionLCD::gfx_line_to(const float x, const float y, const float z) {
      if (PageID == EPageStatus && (print_job_counter.isRunning() || card.isPrinting)) {
        #if ENABLED(ARDUINO_ARCH_SAM)
          gfx.line_to(NX_TOOL, x, y, z, true);
        #else
          gfx.line_to(NX_TOOL, x, y, z);
        #endif
      }
    }

    void NextionLCD::gfx_plane_to(const float x, const float y, const float z) {
      uint8_t color;
      if (PageID == EPageStatus) {
        if (z < 10) color = NX_LOW;
        else color = NX_HIGH;
        gfx.line_to(color, x, y, z, true);
      }
    }
  #endif

#endif

/*****************************
 * 
 * DEFINICJE METOD MARLINA 
 * 
 * **************************/
#ifndef EXTENSIBLE_UI //chwilowo przejsciowo nex
	// MARLIN INIT
  void MarlinUI::init()		{ nexlcd.init(); }
	// MARLIN UPDATE
  void MarlinUI::update()	{ nexlcd.update(); }


	// Ustawia pasek statusu progmem
	void MarlinUI::set_status_P(PGM_P const message, int8_t level) { //TO SAMO ^
				if (level < 0) level = lcd_status_message_level = 0;
				if (level < lcd_status_message_level || !NextionON) return;
				strncpy_P(lcd_status_message, message, 24);
				lcd_status_message_level = level;
				if (PageID == EPageStatus) LcdStatus.setText(lcd_status_message);
	};
	// Ustawia pasek statusu
	void MarlinUI::set_status(const char* message, bool persist) {
    UNUSED(persist);
    if (lcd_status_message_level > 0 || !NextionON) return;
    strncpy(lcd_status_message, message, 24);
    if (PageID == EPageStatus) LcdStatus.setText(lcd_status_message);
  }
	// Resetuje status na domyslny WELCOME_MSG
	void MarlinUI::reset_status(){ set_status_P(GET_TEXT(WELCOME_MSG),1); }

	void MarlinUI::nex_bedlevel_finish()
	{
		//ui.nex_return_after_leveling(true); //dodane, powrot do status
		PagePrinter.show();
		queue.inject_P("M500");  // dodane aby zapisywało poziomowanie podczas trwania funkcji
		g29_in_progress = false; // dodane po zakonczeniu g29
	}
#endif


/*****************************
 * 
 * DEFINICJE EXTUI MARLINA WSTEP
 * 
 * **************************/
	// EXTUI INIT
#ifdef EXTENSIBLE_UI
	namespace ExtUI {
		//void OnPidTuning(const result_t rst);

		void onStartup()		{ nexlcd.init(); }
		void onIdle()				{ nexlcd.update(); nexlcd.check_periodical_actions();}
		void onPrinterKilled(PGM_P const error, PGM_P const component) { nexlcd.kill_screen_msg(error, component); }
		void onPlayTone(const unsigned int frequency, const unsigned long duration) {}

		// SD CARD OBSLUGA
		void onMediaInserted() {
			SDstatus = SD_INSERT; 						// przekaz flage do strony stat
			SD.setValue(SDstatus, "stat");		// nex SDval
			SERIAL_ECHOLN("onMediaInsert..");

			PageID = Nextion_PageID();
			if (PageID == EPageSD)
			{
				nexlcd.setpageSD();							// ustaw strone i
			}
		};

		void onMediaRemoved() { 
			SDstatus = SD_NO_INSERT; 					// przekaz flage do strony status
			SD.setValue(SDstatus, "stat");		// nex SDval
			SERIAL_ECHOLN("onMediaRemoved..");

			PageID = Nextion_PageID();
			if (PageID == EPageSD)
			{
				nexlcd.setpageSD();							// ustaw strone i 
			}
		};

		void onMediaError(){SERIAL_ECHOLN("onMediaERROR..");};

		// STATUS BAR
		void onStatusChanged(const char * const msg) {
			nexlcd.print_status_msg(msg);
		}

		void onMeshProbingDone(){
			settings.save();
			buzzer.tone(100, 659);
			buzzer.tone(100, 698);

			#if ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
			#elif ENABLED(NEXTION_AUTO_BED_LEVEL)
			#endif

			PagePrinter.show();
		}

		// Szkieletowe z example - do wypelnienia
		void onMeshUpdate(const int8_t xpos, const int8_t ypos, const float zval) {
    // Called when any mesh points are updated
  	}

		void onPrintTimerStarted() {}
		void onPrintTimerPaused() {}
		void onPrintTimerStopped() {}
		void onFilamentRunout(const extruder_t extruder) {}
		void onUserConfirmRequired(const char * const msg) {}

		void onStoreSettings(char *buff) {
			// Called when saving to EEPROM (i.e. M500). If the ExtUI needs
			// permanent data to be stored, it can write up to eeprom_data_size bytes
			// into buff.

			// Example:
			//  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
			//  memcpy(buff, &myDataStruct, sizeof(myDataStruct));
		}

		void onLoadSettings(const char *buff) {
			// Called while loading settings from EEPROM. If the ExtUI
			// needs to retrieve data, it should copy up to eeprom_data_size bytes
			// from buff

			// Example:
			//  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
			//  memcpy(&myDataStruct, buff, sizeof(myDataStruct));
		}

		void onConfigurationStoreWritten(bool success) {
			// Called after the entire EEPROM has been written,
			// whether successful or not.
			BUZZ(70, 2300); // dodane beeper git
			BUZZ(70, 2900); // dodane beeper git
		}

		void onConfigurationStoreRead(bool success) {
			// Called after the entire EEPROM has been read,
			// whether successful or not.
			BUZZ(70, 2300); // dodane beeper git
			BUZZ(70, 2900); // dodane beeper git
		}

		void onFactoryReset(){
			BUZZ(70, 2900); // dodane beeper git	
			BUZZ(70, 2300); // dodane beeper git
		}

		#if ENABLED(POWER_LOSS_RECOVERY)
			void OnPowerLossResume() {
				// Called on resume from power-loss
			}
		#endif

		//#if HAS_PID_HEATING
			void onPidTuning(const result_t rst) {
				// Called for temperature PID tuning result

				SERIAL_ECHOLNPAIR("onPidTuning:",rst);
				switch(rst) {
					case PID_BAD_EXTRUDER_NUM:
						nexlcd.print_status_msg(STR_PID_BAD_EXTRUDER_NUM);
						break;
					case PID_TEMP_TOO_HIGH:
						nexlcd.print_status_msg(STR_PID_TEMP_TOO_HIGH);
						break;
					case PID_TUNING_TIMEOUT:
						nexlcd.print_status_msg(STR_PID_TIMEOUT);
						break;
					case PID_DONE:
						nexlcd.print_status_msg(STR_PID_AUTOTUNE_FINISHED);
						break;
					case PID_START:
						nexlcd.print_status_msg("Rozpoczeto kalibracje PID");
						break;
				}
      	//ScreenHandler.GotoScreen(DGUSLCD_SCREEN_MAIN);
			}
		//#endif

		void printFile(const char *filename);
		void stopPrint();
		void pausePrint();
		void resumePrint();

	};	// namespace ExtUI

	#endif

