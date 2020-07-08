#include "../../MarlinCore.h"
#include "../../module/temperature.h"
#include "../../sd/cardreader.h"
#include "../../module/printcounter.h"
#include "../../libs/numtostr.h"
#include "../../HAL/shared/eeprom_api.h"
#include "../../lcd/extui/ui_api.h"
#pragma once

#if ENABLED(NEXTION)
  #include "Nextion_lcd.h"
  #include "Nextion_gfx.h"
  #include "library/Nextion.h"
#endif

	enum NexPage_enum {
			EPageStatus 				= 1,
			EPageSD 						= 2,
			EPageHeating 				= 3,
			EPageMaintain 			= 4,
			EPageSetup 					= 5,
			EPageMove 					= 6,
			EPageSpeed 					= 7,
			EPageFilament 			= 11,
			EPageBedlevel 			= 12,
			EPageSelect 				= 14,
			EPageYesno 					= 15,
			EPageFlow 					= 21,
			EPageKill 					= 30,
			EPageScreenSaver 		= 34,
			EPageBedlevelAuto 	= 35,
	};


  /**
   *******************************************************************
   * NEX lista stron uzytych
   *******************************************************************
   */
  NexObject Pstart        = NexObject(0,  0,  "start");
  NexObject PagePrinter   = NexObject(1,  0,  "stat");

	NexObject PageHeatup		= NexObject(3, 0,	"heatup"); // ** nie trza?
	NexObject PageOptions		= NexObject(4, 0,	"maintain");
  NexObject PageSetup     = NexObject(5,  0,  "setup");

  NexObject Pfilament     = NexObject(11, 0, "filament");
	NexObject Pprobe        = NexObject(12, 0,  "bedlevel"); // nie ma w 8bit?

	NexObject PageSelect    = NexObject(14, 0,  "select");
  NexObject PageYesNo     = NexObject(15, 0,  "yesno");

	NexObject Paccel				= NexObject(18, 0, "accelpage"); // ** nie trzeba?
	NexObject PageKill			= NexObject(30, 0, "kill");
	NexObject Psav 					= NexObject(34, 0, "wyga");
	// 
	// == 9


 // do zaaplikowania
	#if ENABLED(NEX_SCREENSAVER)
	NexObject Psav 					= NexObject(34, 0, "wyga");
	#endif
	#if ENABLED(NEXTION_SEMIAUTO_BED_LEVEL)
	NexObject Pprobe        = NexObject(EPageBedlevel, 0,  "bedlevel");
	#endif
	#if ENABLED(NEXTION_AUTO_BED_LEVEL)
	NexObject Palevel				= NexObject(EPageBedlevelAuto, 0, "ABL");
	#endif

  /**
   *******************************************************************
   * NEX komponenty strona: start
   *******************************************************************
   */
  //NexObject NexVersion     = NexObject(0, 7,  "t1"); // out? nieuzywane
  NexObject splashText        = NexObject(0, 5, "t0");

  /**
   *******************************************************************
   * NEX komponenty strona: status/printer 1
   *******************************************************************
   */
  NexObject LcdX						= NexObject(1,  4,  "vx");
  NexObject LcdY						= NexObject(1,  5,  "vy");
  NexObject LcdZ						= NexObject(1,  6,  "vz");
  NexObject Hotend00				= NexObject(1,  7,  "he00");
  NexObject Hotend01				= NexObject(1,  8,  "he01");
  NexObject Bed0						= NexObject(1, 9,  "bed0");
	NexObject Bed1						= NexObject(1, 10, "bed1");
  NexObject SD							= NexObject(1, 11,  "sd");
  NexObject PrinterFanspeed = NexObject(1, 12,  "fs");
  NexObject VSpeed					= NexObject(1, 13,  "vs");
  NexObject Language				= NexObject(1, 14,  "lang");
  NexObject NStop						= NexObject(1, 18,  "p1");
  NexObject NPlay						= NexObject(1, 19,  "p2");
  NexObject LcdStatus				= NexObject(1, 20,  "t0");
  NexObject LcdTimeElapsed	= NexObject(1, 21,  "t_e");
	NexObject LcdTimeRemain		= NexObject(1, 54,  "t_r");
  NexObject progressbar			= NexObject(1, 22,  "j0");
  NexObject Wavetemp				= NexObject(1, 23,  "s0");
	NexObject percentdone			= NexObject(1, 43,	"t4");
	NexObject NexFilename			= NexObject(1, 55,	"t2");
	//
	// == 18
	// == 29


  /**
   *******************************************************************
   * NEX komponenty strona: SDCard 2
   *******************************************************************
   */
  NexObject sdscrollbar = NexObject(2,   1, "h0");
  NexObject sdrow0      = NexObject(2,   2, "t0");
  NexObject sdrow1      = NexObject(2,   3, "t1");
  NexObject sdrow2      = NexObject(2,   4, "t2");
  NexObject sdrow3      = NexObject(2,   5, "t3");
  NexObject sdrow4      = NexObject(2,   6, "t4");
  NexObject sdrow5      = NexObject(2,   7, "t5");
  NexObject Folder0     = NexObject(2,  18, "p0");
  NexObject Folder1     = NexObject(2,  19, "p1");
  NexObject Folder2     = NexObject(2,  20, "p2");
  NexObject Folder3     = NexObject(2,  21, "p3");
  NexObject Folder4     = NexObject(2,  22, "p4");
  NexObject Folder5     = NexObject(2,  23, "p5");
  NexObject Folderup    = NexObject(2,  24, "p6");
  NexObject sdfolder    = NexObject(2,	 8, "t6");
  NexObject ScrollUp    = NexObject(2,  9, "p7");
  NexObject ScrollDown  = NexObject(2,  10, "p8");
#if ENABLED(NEXTION_SD_LONG_NAMES)
	NexObject file0				= NexObject(2, 11, "n1");
	NexObject file1				= NexObject(2, 12, "n2");
	NexObject file2				= NexObject(2, 13, "n3");
	NexObject file3				= NexObject(2, 14, "n4");
	NexObject file4				= NexObject(2, 15, "n5");
	NexObject file5				= NexObject(2, 16, "n6");
#endif
	// 
	// == 25
	// == 54

	/**
	*******************************************************************
	* NEX komponenty strona: HEATUP 3
	*******************************************************************
	*/
	NexObject heatupenter		= NexObject(3, 6, "m3");
	NexObject temphe				= NexObject(3, 7, "temphe");
	NexObject tempbe				= NexObject(3, 8, "tempbe");
	//NexObject heatbedenter	= NexObject(3, 12, "m4");
	//NexObject hotendenter		= NexObject(3, 13, "m5");
	NexObject chillenter		= NexObject(3, 12, "m5");
	// 
	// == 6
	// == 98


  /**
   *******************************************************************
   * NEX komponenty strona: MOVE 6
   *******************************************************************
   */
  NexObject XYHome      = NexObject(6,   2, "p4");
  NexObject XYUp        = NexObject(6,   3, "p5");
  NexObject XYRight     = NexObject(6,   4, "p6");
  NexObject XYDown      = NexObject(6,   5, "p7");
  NexObject XYLeft      = NexObject(6,   6, "p8");
  NexObject ZHome       = NexObject(6,   7, "p9");
  NexObject ZUp         = NexObject(6,   8, "p10");
  NexObject ZDown       = NexObject(6,   9, "p11");
  NexObject movecmd     = NexObject(6,  11, "vacmd");
  NexObject LedCoord5   = NexObject(6,  12, "t0");
  NexObject MotorOff    = NexObject(6,  17, "p0");
  NexObject Extrude     = NexObject(6,  19, "p12");	
  NexObject Retract     = NexObject(6,  20, "p14");
	// 
	// == 13
	// == 67

	/**
	*******************************************************************
	* NEX komponenty strona: SPEED 7
	*******************************************************************
	*/
	NexObject speedsetbtn	= NexObject(7, 9, "m0");
	NexObject SpeedNex		= NexObject(7, 7, "vspeed");
	// 
	// == 2

	/**
	*******************************************************************
	* NEX komponenty strona:: FAN SPEED 8
	*******************************************************************
	*/
	NexObject FanSpeedNex			= NexObject(8, 7, "vfan");
	NexObject FanSetBtn				= NexObject(8, 9, "m1");
	// 
	// == 3

  /**
   *******************************************************************
   * NEX komponenty strona: GCode 10
   *******************************************************************
   */
  NexObject Tgcode      = NexObject(10,   1, "tgcode");
  NexObject Send        = NexObject(10,  25, "bsend");
	// 
	// == 2

  /**
   *******************************************************************
   * NEX komponenty strona: Probe BEDLEVEL 12
   *******************************************************************
   */
  NexObject ProbeUp     = NexObject(12, 1,  "p0");
  NexObject ProbeSend   = NexObject(12, 2,  "p1");
  NexObject ProbeDown   = NexObject(12, 3,  "p2");
  //NexObject ProbeMsg    = NexObject(14, 4,  "t0");
  NexObject ProbeZ      = NexObject(12, 5,  "t1");
	// 
	// == 4

	/**
	*******************************************************************
	* NEX komponenty strona:: BABYSTEP SCREEN 13
	*******************************************************************
	*/
	NexObject ZbabyUp			= NexObject(13, 1, "m0");
	NexObject ZbabyDown		= NexObject(13, 2, "m1");
	NexObject ZbabyBack_Save = NexObject(13, 3, "m2");
	// 
	// == 3

  /**
   *******************************************************************
   * NEX komponenty strona:: Select 14
   *******************************************************************
   */
  NexObject LcdRiga1    = NexObject(14,  2, "t0");
  NexObject LcdRiga2    = NexObject(14,  3, "t1");
  NexObject LcdRiga3    = NexObject(14,  4, "t2");
  NexObject LcdRiga4    = NexObject(14,  5, "t3");
  NexObject LcdUp       = NexObject(14,  14, "p4");
  NexObject LcdSend     = NexObject(14,  13, "p1");
  NexObject LcdDown     = NexObject(14,  15, "p5");
  NexObject LcdMin      = NexObject(14,  6, "min");
  NexObject LcdMax      = NexObject(14, 7, "max");
  NexObject LcdPos      = NexObject(14, 8, "pos");
	// 
	// == 10

  /**
   *******************************************************************
   * NEX komponenty strona: Yesno 15
   *******************************************************************
   */
  NexObject Vyes        = NexObject(15, 2,  "yn0");
  NexObject Riga0       = NexObject(15, 4,  "tl0");
  NexObject Riga1       = NexObject(15, 5,  "tl1");
  NexObject Riga2       = NexObject(15, 6,  "tl2");
  NexObject Riga3       = NexObject(15, 7,  "tl3");
  NexObject Yes         = NexObject(15, 8,  "p1");
  NexObject No          = NexObject(15, 9,  "p2");
	// 
	// == 7


	/**
	*******************************************************************
	* NEX komponenty strona:: STAT SCREEN 16
	*******************************************************************
	*/
	#if ENABLED(NEX_STAT_PAGE)
		NexObject statin			= NexObject(5, 1, "m2"); //przycisk z innej strony -> setup
		NexObject Sprints			= NexObject(16, 2, "t0");
		NexObject Scompl			= NexObject(16, 3, "t1");
		NexObject Spanic			= NexObject(16, 4, "t2");
		NexObject Stimetotal	= NexObject(16, 5, "t3");
		NexObject Stimelong		= NexObject(16, 6, "t4");
		NexObject Sfilament		= NexObject(16, 7, "t5");

		NexObject Sfirmware		= NexObject(16, 9, "t6");
		NexObject Skompil			= NexObject(16, 10, "t7");
		NexObject Sleveling		= NexObject(16, 11, "t8");
		NexObject Svlcs				= NexObject(16, 12, "t9");
		NexObject Sfilsensor	= NexObject(16, 13, "t10");
	#endif
	// 
	// == 12

	/**
	*******************************************************************
	* NEX komponenty strona:: SERVICE PAGE 17
	*******************************************************************
	*/
	//NexObject SvJerk				= NexObject(17, 4, "m2"); //wejscie w jerk -> przekazuje zmienne float na nuber nextion (brak dziesietnych)
	NexObject SvSteps				= NexObject(17, 5, "m3");	//wejscie w steps -> przekazuje zmienne float na nuber nextion (brak dziesietnych)
	// 

	/**
	*******************************************************************
	* NEX komponenty strona:: ACCEL SCREEN 18
	*******************************************************************
	*/
#if ENABLED(NEX_ACC_PAGE)
	NexObject accelin		= NexObject(5, 3, "m5");	// setaccelpagePopCallback -> przekazuje zmienne float do strony z akceleracja
	NexObject Awork			= NexObject(18, 22, "a0");
	NexObject Aretr			= NexObject(18, 23, "a1");
	NexObject Atravel		= NexObject(18, 24, "a2");
	NexObject Amaxx			= NexObject(18, 25, "a3");
	NexObject Amaxy			= NexObject(18, 26, "a4");
	NexObject Amaxz			= NexObject(18, 27, "a5");
	NexObject Amaxe			= NexObject(18, 28, "a6");
	NexObject Asend			= NexObject(18, 33, "p12"); 
	NexObject Asave			= NexObject(18, 30, "p10");	// setaccelsavebtnPopCallback -> wywo�uje settings.save();
	NexObject Aload			= NexObject(18, 31, "p11"); // setaccelloadbtnPopCallback	-> wywo�uje settings.load();
#endif
	// 
	// == 11

	/**
	*******************************************************************
	* NEX komponenty strona:: FLOWPAGE 21
	*******************************************************************
	*/
	NexObject vFlowNex					= NexObject(21, 7, "vflow");
	NexObject SetFlowBtn				= NexObject(21, 9, "m0");
	NexObject FlowPageFrom			= NexObject(21, 10, "flowfrom");
	// 
	// == 3

	/**
	*******************************************************************
	* NEX komponenty strona: KILL SCREEN 30
	*******************************************************************
	*/
	NexObject Kmsg				= NexObject(30, 2, "tkmsg");
	// == 129
		/**
	*******************************************************************
	* NEX komponenty strona: SCREEN SAVER 34
	*******************************************************************
	*/
	NexObject SStxt				= NexObject(34, 2, "g0");
	NexObject SSprog			= NexObject(34, 3, "j0");
	NexObject SSval				= NexObject(34, 8, "SStime");


	// 132*13 = 1716 bajt�w

  NexObject *nex_listen_list[] =
  {
    // Page 2 touch listen
    &NPlay,

    // Page 3 touch listen
    &sdscrollbar, &ScrollUp, &ScrollDown, &sdrow0, &sdrow1, &sdrow2,
    &sdrow3, &sdrow4, &sdrow5, &Folderup,// &sd_mount, &sd_dismount,

    // Page 4 touch listen setup
		#if ENABLED(NEX_STAT_PAGE)
				&statin, 
		#endif

		#if ENABLED(NEX_ACC_PAGE)
				&accelin,
				// Page 23 tacz listen
				&Asend, &Asave, &Aload,
		#endif

    // Page 5 touch listen
    &MotorOff, &XYHome, &XYUp, &XYRight, &XYDown, &XYLeft,
    &ZHome, &ZUp, &ZDown,
    &Extrude, &Retract,

		&speedsetbtn,

    // Page 7 touch listen
    &Send,

    // Page 11 touch listen
    &Yes, &No,

    // Page 12 touch listen
    //&FilLoad, &FilUnload, &FilExtr,

    // Page 13 touch listen
    &LcdSend,

    // Page 14 touch listen
    &ProbeUp, &ProbeDown, &ProbeSend,

		// Page 15 tacz listen
		&heatupenter, &chillenter,

		// Page 18 tacz listen
		&FanSetBtn,

		//Page 22 service
		&SvSteps,

		// Page 28 babystep
		&ZbabyUp, &ZbabyDown, &ZbabyBack_Save,

		// Page 31 Flow
		&SetFlowBtn,

		// page 34 screensaver
		&Psav,

    NULL
  };

  NexObject *lcd_row_list[] =
  {
    &LcdRiga1,
    &LcdRiga2,
    &LcdRiga3,
    &LcdRiga4,
    NULL
  };

  NexObject *heater_list0[] =
  {
		&Hotend00,
		&Bed0,
    NULL
  };

  NexObject *heater_list1[] =
  {
		&Hotend01,
		&Bed1,
    NULL
  };

  NexObject *row_list[] =
  {
    &sdrow0,
    &sdrow1,
    &sdrow2,
    &sdrow3,
    &sdrow4,
    &sdrow5,
    NULL
  };

  NexObject *folder_list[] =
  {
    &Folder0,
    &Folder1,
    &Folder2,
    &Folder3,
    &Folder4,
    &Folder5,
    NULL
  };

	#if ENABLED(NEXTION_SD_LONG_NAMES)
		NexObject *file_list83[] =
		{
			&file0,
			&file1,
			&file2,
			&file3,
			&file4,
			&file5,
			NULL
		};
	#endif
