/*
*
* constants
*
*/

#define VER    14
#define SUB_VERSION  1
#define PATCHLEVEL 4

/* defines below are for use in the termcap mode only */
#define ST_START 1
#define ST_END   2
#define BOLD     3
#define END_BOLD 4
#define CLEAR    5
#define CL_LINE  6
#define T_INIT   7
#define T_END    8
#define CL_DOWN 14
#define CURSOR  15



#define KNOWNOT   0x00
#define HAVESEEN  0x1
#define KNOWHERE  0x2
#define KNOWALL   (HAVESEEN | KNOWHERE)

#define PATHLEN   80

#define LARNHOME ""

#ifndef WIZID
#define WIZID  1000
#endif

#define TRUE 1
#define FALSE 0

#define MAXLEVEL 11		/*  max # levels in the dungeon         */
#define MAXVLEVEL 3		/*  max # of levels in the temple of the luran  */
#define MAXX 67
#define MAXY 17

#define SCORESIZE 10		/* this is the number of people on a scoreboard max */
#define MAXPLEVEL 100		/* maximum player level allowed        */
#define SPNUM 38		/* maximum number of spells in existance   */
#define TIMELIMIT 30000		/* maximum number of moves before the game is called */
#define TAXRATE 1/20		/* tax rate for the LRS */

#define BUFBIG  4096		/* size of the output buffer */
#define MAXIBUF 4096		/* size of the input buffer */
#define LOGNAMESIZE 20		/* max size of the player's name */

#define SAVEFILENAMESIZE 128	/* max size of the savefile path */

#define STRING_BUFFER_SIZE	256



/*
* monster related constants
*/
/* maximum # monsters in the dungeon */
#define MAXMONST 56

/*  defines for the monsters as objects  */
#define BAT 1
#define GNOME 2
#define HOBGOBLIN 3
#define JACKAL 4
#define KOBOLD 5
#define ORC 6
#define SNAKE 7
#define CENTIPEDE 8
#define JACULI 9
#define TROGLODYTE 10
#define ANT 11
#define EYE 12
#define LEPRECHAUN 13
#define NYMPH 14
#define QUASIT 15
#define RUSTMONSTER 16
#define ZOMBIE 17
#define ASSASSINBUG 18
#define BUGBEAR 19
#define HELLHOUND 20
#define ICELIZARD 21
#define CENTAUR 22
#define TROLL 23
#define YETI 24
#define WHITEDRAGON 25
#define ELF 26
#define CUBE 27
#define METAMORPH 28
#define VORTEX 29
#define ZILLER 30
#define VIOLETFUNGI 31
#define WRAITH 32
#define FORVALAKA 33
#define LAMANOBE 34
#define OSEQUIP 35
#define ROTHE 36
#define XORN 37
#define VAMPIRE 38
#define INVISIBLESTALKER 39
#define POLTERGEIST 40
#define DISENCHANTRESS 41
#define SHAMBLINGMOUND 42
#define YELLOWMOLD 43
#define UMBERHULK 44
#define GNOMEKING 45
#define MIMIC 46
#define WATERLORD 47
#define BRONZEDRAGON 48
#define GREENDRAGON 49
#define PURPLEWORM 50
#define XVART 51
#define SPIRITNAGA 52
#define SILVERDRAGON 53
#define PLATINUMDRAGON 54
#define GREENURCHIN 55
#define REDDRAGON 56
#define DEMONLORD 57
#define DEMONPRINCE 64




/*
* defines for the character attribute array   cdesc[]
*/
#define STRENGTH 0		/* characters physical strength not due to objects */
#define INTELLIGENCE 1
#define WISDOM 2
#define CONSTITUTION 3
#define DEXTERITY 4
#define CHARISMA 5
#define HPMAX 6
#define HP 7
#define GOLD 8
#define EXPERIENCE 9
#define LEVEL 10
#define REGEN 11
#define WCLASS 12
#define AC 13
#define BANKACCOUNT 14
#define SPELLMAX 15
#define SPELLS 16
#define ENERGY 17
#define ECOUNTER 18
#define MOREDEFENSES 19
#define WEAR 20
#define PROTECTIONTIME 21
#define WIELD 22
#define AMULET 23
#define REGENCOUNTER 24
#define MOREDAM 25
#define DEXCOUNT 26
#define STRCOUNT 27
#define BLINDCOUNT 28
#define CAVELEVEL 29
#define CONFUSE 30
#define ALTPRO 31
#define HERO 32
#define CHARMCOUNT 33
#define INVISIBILITY 34
#define CANCELLATION 35
#define HASTESELF 36
#define EYEOFLARN 37
#define AGGRAVATE 38
#define GLOBE 39
#define TELEFLAG 40
#define SLAYING 41
#define NEGATESPIRIT 42
#define SCAREMONST 43
#define AWARENESS 44
#define HOLDMONST 45
#define TIMESTOP 46
#define HASTEMONST 47
#define CUBEofUNDEAD 48
#define GIANTSTR 49
#define FIRERESISTANCE 50
#define BESSMANN 51
#define NOTHEFT 52
#define HARDGAME 53
#define CPUTIME 54
#define BYTESIN 55
#define BYTESOUT 56
#define MOVESMADE 57
#define MONSTKILLED 58
#define SPELLSCAST 59
#define LANCEDEATH 60
#define SPIRITPRO 61
#define UNDEADPRO 62
#define SHIELD 63
#define STEALTH 64
#define ITCHING 65
#define LAUGHING 66
#define DRAINSTRENGTH 67
#define CLUMSINESS 68
#define INFEEBLEMENT 69
#define HALFDAM 70
#define SEEINVISIBLE 71
#define FILLROOM 72
#define RANDOMWALK 73
#define SPHCAST 74		/* nz if an active sphere of annihilation */
#define WTW 75			/* walk through walls */
#define STREXTRA 76		/* character strength due to objects or enchantments */
#define TMP 77			/* misc scratch space */
#define LIFEPROT 78		/* life protection counter */


/* nap related */
#define NAPTIME 1000


/*
* object related constants
*/
#define MAXSCROLL 28		/* maximum number of scrolls that are possible */
#define MAXSCROLLNAME	32

#define MAXPOTION 35		/* maximum number of potions that are possible */
#define MAXPOTIONNAME	32

#define MAXOBJ 94		/* the maximum number of objects   n < MAXOBJ */

/*  defines for the objects in the game     */
#define MAXOBJECT  93

#define OALTAR 1
#define OTHRONE 2
#define OORB 3
#define OPIT 4
#define OSTAIRSUP 5
#define OELEVATORUP 6
#define OFOUNTAIN 7
#define OSTATUE 8
#define OTELEPORTER 9
#define OSCHOOL 10
#define OMIRROR 11
#define ODNDSTORE 12
#define OSTAIRSDOWN 13
#define OELEVATORDOWN 14
#define OBANK2 15
#define OBANK 16
#define ODEADFOUNTAIN 17
#define OMAXGOLD 70
#define OGOLDPILE 18
#define OOPENDOOR 19
#define OCLOSEDDOOR 20
#define OWALL 21
#define OTRAPARROW 66
#define OTRAPARROWIV 67

#define OLARNEYE 22

#define OPLATE 23
#define OCHAIN 24
#define OLEATHER 25
#define ORING 60
#define OSTUDLEATHER 61
#define OSPLINT 62
#define OPLATEARMOR 63
#define OSSPLATE 64
#define OSHIELD 68
#define OELVENCHAIN 92


#define OSWORDofSLASHING 26
#define OHAMMER 27
#define OSWORD 28
#define O2SWORD 29
#define OHSWORD 59
#define OSPEAR 30
#define ODAGGER 31
#define OBATTLEAXE 57
#define OLONGSWORD 58
#define OLANCE 65
#define OVORPAL 90
#define OSLAYER 91

#define ORINGOFEXTRA 32
#define OREGENRING 33
#define OPROTRING 34
#define OENERGYRING 35
#define ODEXRING 36
#define OSTRRING 37
#define OCLEVERRING 38
#define ODAMRING 39

#define OBELT 40

#define OSCROLL 41
#define OPOTION 42
#define OBOOK 43
#define OCHEST 44
#define OAMULET 45

#define OORBOFDRAGON 46
#define OSPIRITSCARAB 47
#define OCUBEofUNDEAD 48
#define ONOTHEFT 49

#define ODIAMOND 50
#define ORUBY 51
#define OEMERALD 52
#define OSAPPHIRE 53

#define OENTRANCE 54
#define OVOLDOWN 55
#define OVOLUP 56
#define OHOME 69

#define OKGOLD 71
#define ODGOLD 72
#define OIVDARTRAP 73
#define ODARTRAP 74
#define OTRAPDOOR 75
#define OIVTRAPDOOR 76
#define OTRADEPOST 77
#define OIVTELETRAP 78
#define ODEADTHRONE 79
#define OANNIHILATION 80	/* sphere of annihilation */
#define OTHRONE2 81
#define OLRS 82			/* Larn Revenue Service */
#define OCOOKIE 83
#define OURN 84
#define OBRASSLAMP 85
#define OHANDofFEAR 86		/* hand of fear */
#define OSPHTAILSMAN 87		/* tailsman of the sphere */
#define OWWAND 88		/* wand of wonder */
#define OPSTAFF 89		/* staff of power */
#define OPUDDLE 93		/* puddle object */
/* used up to 93. ~Gibbon */
