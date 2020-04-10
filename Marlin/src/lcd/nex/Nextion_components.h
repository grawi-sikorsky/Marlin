#include "../../MarlinCore.h"
#include "../../module/temperature.h"
#include "../../sd/cardreader.h"
#include "../../module/printcounter.h"
#include "../../libs/numtostr.h"
//#include "../../HAL/shared/persistent_store_api.h"
#include "../../lcd/extui/ui_api.h"

#if ENABLED(NEXTION)
  #include "Nextion_lcd.h"
  #include "Nextion_gfx.h"
  #include "library/Nextion.h"
#endif

  /**
   *******************************************************************
   * NEX lista stron
   *******************************************************************
   */
  //NexObject Pstart        	= NexObject(0,  0,  "start");
  NexObject PageMenu       		= NexObject(1,  0,  "menu");
  NexObject PagePrinter    		= NexObject(2,  0,  "printer");
  //NexObject Psdcard       	= NexObject(3,  0,  "sdcard");
  NexObject PageSetup        	= NexObject(4,  0,  "setup");
	//NexObject Pmove         	  = NexObject(5,  0,  "move");
  //NexObject Pspeed        	= NexObject(6,  0,  "speed");// lipa
  //NexObject Pgcode        	= NexObject(7,  0,  "gcode");// lipa
  //NexObject Prfid         	= NexObject(8,  0,  "rfid");
  //NexObject Pbrightness   	= NexObject(9,  0,  "brightness");
  //NexObject Pinfo         	= NexObject(10, 0,  "info");
  NexObject PageYesNo        	= NexObject(11, 0,  "yesno");
  NexObject PageFilament	   	= NexObject(12, 0, "filament");
  NexObject PageSelect    		= NexObject(13, 0,  "select");
  NexObject PageProbe  		  	= NexObject(14, 0,  "bedlevel");
	NexObject PageHeatup				= NexObject(15, 0,	"heatup");
	NexObject PageOptions				= NexObject(16, 0,	"maintain");//
  //NexObject Ptime         	= NexObject(17, 0,  "infomove");
  //NexObject Pfanspeedpage 	= NexObject(18, 0,  "fanspeedpage");
	//NexObject Pstats					= NexObject(19, 0,	"statscreen");
	//NexObject Ptsettings			= NexObject(20, 0,  "tempsettings");
	//NexObject Pinfobedlevel 	= NexObject(21, 0, "infobedlevel");
	//NexObject Pservice				= NexObject(22, 0, "servicepage");
	//NexObject Paccel					= NexObject(23, 0, "accelpage");
	//NexObject Pjerk						= NexObject(25, 0, "jerkpage");
	NexObject PageKill					= NexObject(30, 0, "killpage");

   /*******************************************************************
   * NEX komponenty strona: SplashScreen
   *******************************************************************/
  NexObject splashRandomNr    = NexObject(0, 6, "randomnr");
  NexObject splashText        = NexObject(0, 5, "t0");
  NexObject splashTimer	      = NexObject(0,  3,  "tm1");

  /*******************************************************************
   * NEX komponenty strona: menu
   *******************************************************************/
  NexObject Version     = NexObject(1, 6,  "t0"); // out? nieuzywane

	/*******************************************************************
   * NEX komponenty strona: status/printer
   *******************************************************************/
  NexObject LcdX						= NexObject(2,  4,  "vx");
  NexObject LcdY						= NexObject(2,  5,  "vy");
  NexObject LcdZ						= NexObject(2,  6,  "vz");
  NexObject Hotend00				= NexObject(2,  7,  "he00");
  NexObject Hotend01				= NexObject(2,  8,  "he01");
  NexObject Bed0						= NexObject(2, 9,  "bed0");
	NexObject Bed1						= NexObject(2, 10, "bed1");
  NexObject SD							= NexObject(2, 11,  "sd");
  NexObject PrinterFanspeed = NexObject(2, 12,  "fs");
  NexObject VSpeed					= NexObject(2, 13,  "vs");
  NexObject Language				= NexObject(2, 14,  "lang");
  NexObject NStop						= NexObject(2, 18,  "p1");
  NexObject NPlay						= NexObject(2, 19,  "p2");
  NexObject LcdStatus				= NexObject(2, 20,  "t0");
  NexObject LcdTime					= NexObject(2, 21,  "t2");
  NexObject progressbar			= NexObject(2, 22,  "j0");
  NexObject Wavetemp				= NexObject(2, 23,  "s0");
	NexObject percentdone			= NexObject(2, 49,	"t4");

	/*******************************************************************
   * NEX komponenty strona: SDCard
   *******************************************************************/
  NexObject sdscrollbar = NexObject(3,   1, "h0");
  NexObject sdrow0      = NexObject(3,   2, "t0");
  NexObject sdrow1      = NexObject(3,   3, "t1");
  NexObject sdrow2      = NexObject(3,   4, "t2");
  NexObject sdrow3      = NexObject(3,   5, "t3");
  NexObject sdrow4      = NexObject(3,   6, "t4");
  NexObject sdrow5      = NexObject(3,   7, "t5");
  NexObject Folder0     = NexObject(3,  19, "p0");
  NexObject Folder1     = NexObject(3,  20, "p1");
  NexObject Folder2     = NexObject(3,  21, "p2");
  NexObject Folder3     = NexObject(3,  22, "p3");
  NexObject Folder4     = NexObject(3,  23, "p4");
  NexObject Folder5     = NexObject(3,  24, "p5");
  NexObject Folderup    = NexObject(3,  25, "p6");
  NexObject sdfolder    = NexObject(3,	 9, "t6");
  NexObject ScrollUp    = NexObject(3,  10, "p7");
  NexObject ScrollDown  = NexObject(3,  11, "p8");
#if ENABLED(NEXTION_SD_LONG_NAMES)
	NexObject file0				= NexObject(3, 12, "n1");
	NexObject file1				= NexObject(3, 13, "n2");
	NexObject file2				= NexObject(3, 14, "n3");
	NexObject file3				= NexObject(3, 15, "n4");
	NexObject file4				= NexObject(3, 16, "n5");
	NexObject file5				= NexObject(3, 17, "n6");
#endif

   /*******************************************************************
   * NEX komponenty strona: MOVE
   *******************************************************************/
  NexObject XYHome      = NexObject(5,   2, "p4");
  NexObject XYUp        = NexObject(5,   3, "p5");
  NexObject XYRight     = NexObject(5,   4, "p6");
  NexObject XYDown      = NexObject(5,   5, "p7");
  NexObject XYLeft      = NexObject(5,   6, "p8");
  NexObject ZHome       = NexObject(5,   7, "p9");
  NexObject ZUp         = NexObject(5,   8, "p10");
  NexObject ZDown       = NexObject(5,   9, "p11");
  NexObject movecmd     = NexObject(5,  11, "vacmd");
  NexObject LedCoord5   = NexObject(5,  12, "t0");
  NexObject MotorOff    = NexObject(5,  17, "p0");
  NexObject Extrude     = NexObject(5,  19, "p12");	
  NexObject Retract     = NexObject(5,  20, "p14");

	/*******************************************************************
	* NEX komponenty strona: SPEED
	*******************************************************************/
	NexObject speedsetbtn	= NexObject(6, 9, "m0");
	NexObject SpeedNex		= NexObject(6, 7, "vspeed");

   /*******************************************************************
   * NEX komponenty strona: GCode
   *******************************************************************/
  NexObject Tgcode      = NexObject(7,   1, "tgcode");
  NexObject Send        = NexObject(7,  25, "bsend");

   /*******************************************************************
   * NEX komponenty strona: Info
   *******************************************************************/
  //NexObject InfoText    = NexObject(10, 2,  "t0");
  //NexObject ScrollText  = NexObject(10, 3,  "g0");

   /*******************************************************************
   * NEX komponenty strona: YesNo
   *******************************************************************/
  NexObject Vyes        = NexObject(11, 2,  "yn0");
  NexObject Riga0       = NexObject(11, 4,  "tl0");
  NexObject Riga1       = NexObject(11, 5,  "tl1");
  NexObject Riga2       = NexObject(11, 6,  "tl2");
  NexObject Riga3       = NexObject(11, 7,  "tl3");
  NexObject Yes         = NexObject(11, 8,  "p1");
  NexObject No          = NexObject(11, 9,  "p2");

   /*******************************************************************
   * NEX komponenty strona:: Select
   *******************************************************************/
  NexObject LcdRiga1    = NexObject(13,  2, "t0");
  NexObject LcdRiga2    = NexObject(13,  3, "t1");
  NexObject LcdRiga3    = NexObject(13,  4, "t2");
  NexObject LcdRiga4    = NexObject(13,  5, "t3");
  NexObject LcdUp       = NexObject(13,  15, "p4");
  NexObject LcdSend     = NexObject(13,  14, "p1");
  NexObject LcdDown     = NexObject(13,  16, "p5");
  NexObject LcdMin      = NexObject(13,  7, "min");
  NexObject LcdMax      = NexObject(13, 8, "max");
  NexObject LcdPos      = NexObject(13, 9, "pos");

   /*******************************************************************
   * NEX komponenty strona: Probe
   *******************************************************************/
  NexObject ProbeUp     = NexObject(14, 1,  "p0");
  NexObject ProbeSend   = NexObject(14, 2,  "p1");
  NexObject ProbeDown   = NexObject(14, 3,  "p2");
  //NexObject ProbeMsg    = NexObject(14, 4,  "t0");
  NexObject ProbeZ      = NexObject(14, 5,  "t1");

	/*******************************************************************
	* NEX komponenty strona: HEATUP 15
	*******************************************************************/
	NexObject heatupenter		= NexObject(15, 7, "m3");
	NexObject temphe				= NexObject(15, 8, "temphe");
	NexObject tempbe				= NexObject(15, 9, "tempbe");
	NexObject heatbedenter	= NexObject(15, 12, "m4");
	NexObject hotendenter		= NexObject(15, 13, "m5");
	NexObject chillenter		= NexObject(15, 14, "m6");

	/*******************************************************************
	* NEX komponenty strona:: FAN SPEED 18
	*******************************************************************/
	NexObject FanSpeedNex			= NexObject(18, 7, "vfan");
	NexObject FanSetBtn				= NexObject(18, 9, "m1");
	NexObject FanPageIDfrom		= NexObject(18, 10, "fanpagefrom");

	/*******************************************************************
	* NEX komponenty strona:: STAT SCREEN 19
	*******************************************************************/
	#if ENABLED(NEX_STAT_PAGE)
		NexObject statin			= NexObject(4, 1, "m2"); //przycisk z innej strony -> setup
		NexObject Sprints			= NexObject(19, 2, "t0");
		NexObject Scompl			= NexObject(19, 3, "t1");
		NexObject Spanic			= NexObject(19, 4, "t2");
		NexObject Stimetotal	= NexObject(19, 5, "t3");
		NexObject Stimelong		= NexObject(19, 6, "t4");
		NexObject Sfilament		= NexObject(19, 7, "t5");

		NexObject Sfirmware		= NexObject(19, 9, "t6");
		NexObject Skompil			= NexObject(19, 10, "t7");
		NexObject Sleveling		= NexObject(19, 11, "t8");
		NexObject Svlcs				= NexObject(19, 12, "t9");
		NexObject Sfilsensor	= NexObject(19, 13, "t10");
	#endif

	/*******************************************************************
	* NEX komponenty strona:: SERVICE PAGE 22
	*******************************************************************/
	//NexObject SvJerk				= NexObject(22, 4, "m2"); //wejscie w jerk -> przekazuje zmienne float na nuber nextion (brak dziesietnych)
	NexObject SvSteps				= NexObject(22, 5, "m3");	//wejscie w steps -> przekazuje zmienne float na nuber nextion (brak dziesietnych)

	/*******************************************************************
	* NEX komponenty strona:: ACCEL SCREEN 23
	*******************************************************************/
#if ENABLED(NEX_ACC_PAGE)
	NexObject accelin		= NexObject(4, 4, "m5");	// przycisk na stronie ustawienia - przekazuje zmienne float do strony z akceleracja
	NexObject Awork			= NexObject(23, 22, "a0");
	NexObject Aretr			= NexObject(23, 23, "a1");
	NexObject Atravel		= NexObject(23, 24, "a2");
	NexObject Amaxx			= NexObject(23, 25, "a3");
	NexObject Amaxy			= NexObject(23, 26, "a4");
	NexObject Amaxz			= NexObject(23, 27, "a5");
	NexObject Amaxe			= NexObject(23, 28, "a6");
	NexObject Asend			= NexObject(23, 33, "p12"); 
	NexObject Asave			= NexObject(23, 30, "p10");	// setaccelsavebtnPopCallback -> wywo�uje settings.save();
	NexObject Aload			= NexObject(23, 31, "p11"); // setaccelloadbtnPopCallback	-> wywo�uje settings.load();
#endif

	/*******************************************************************
	* NEX komponenty strona:: BABYSTEP SCREEN 28
	*******************************************************************/
	NexObject ZbabyUp			= NexObject(28, 1, "m0");
	NexObject ZbabyDown		= NexObject(28, 2, "m1");
	NexObject ZbabyBack_Save = NexObject(28, 3, "m2");

	/*******************************************************************
	* NEX komponenty strona: KILL SCREEN 30
	*******************************************************************/
	NexObject Kmsg				= NexObject(30, 2, "tkmsg");
	// == 129

	/*******************************************************************
	* NEX komponenty strona:: FLOWPAGE 31
	*******************************************************************/
	NexObject vFlowNex					= NexObject(31, 7, "vflow");
	NexObject SetFlowBtn				= NexObject(31, 9, "m0");
	NexObject FlowPageFrom			= NexObject(31, 10, "flowfrom");
	// == 132
	// 132*13 = 1716 bajt�w


  NexObject *nex_listen_list[] =
  {
    // Page 2 touch listen
    &NPlay,

    // Page 3 touch listen
    &sdscrollbar, &ScrollUp, &ScrollDown, &sdrow0, &sdrow1, &sdrow2,
    &sdrow3, &sdrow4, &sdrow5, &Folderup,

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
    &ZHome, &ZUp, &ZDown, &Extrude, &Retract, &speedsetbtn,

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
		&heatupenter, &heatbedenter, &hotendenter, &chillenter,

		// Page 18 tacz listen
		&FanSetBtn,

		//Page 22 service
		&SvSteps,

		// Page 28 babystep
		&ZbabyUp, &ZbabyDown, &ZbabyBack_Save,

		// Page 31 Flow
		&SetFlowBtn,
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
