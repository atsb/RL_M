/* bill.c */
#include "includes/bill.h"
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/help.h"
#include "includes/io.h"

static int letter1 (int);
static int letter2 (void);
static int letter3 (void);
static int letter4 (void);
static int letter5 (void);
static int letter6 (void);

/*
*  function to create the tax bill for the user
*/
static int
letter1 (int gold)
{
  resetscroll ();
  screen_clear();

  lstandout ("From:");
  lprcat ("  the LRS (Larn Revenue Service)\n");
  lstandout ("\nSubject:");
  lprcat ("  undeclared income\n");

  lprcat ("\n   We heard you survived the caverns of Larn.  Let me be the");
  lprcat
    ("\nfirst to congratulate you on your success.  It is quite a feat.");
  lprcat ("\nIt must also have been very profitable for you.");
  lprcat ("\n\n   The Dungeon Master has informed us that you brought");

  lprintf ("\n%d gold pieces back with you from your journey.  As the", gold);

  lprcat
    ("\ncounty of Larn is in dire need of funds, we have spared no time");
  lprintf ("\nin preparing your tax bill.  You owe %d gold pieces as",
	   gold * TAXRATE);

  lprcat ("\nof this notice, and is due within 5 days.  Failure to pay will");
  lprcat ("\nmean penalties.  Once again, congratulations, We look forward");
  lprcat ("\nto your future successful expeditions.\n");

  retcont ();

  return (1);
}



static int
letter2 (void)
{
  resetscroll ();
  screen_clear();

  lstandout ("From:");
  lprcat ("  His Majesty King Wilfred of Larndom\n");
  lstandout ("\nSubject:");
  lprcat ("  a noble deed\n");

  lprcat ("\n   I have heard of your magnificent feat, and I, King Wilfred,");
  lprcat
    ("\nforthwith declare today to be a national holiday.  Furthermore,");
  lprcat ("\nhence three days, Ye be invited to the castle to receive the");
  lprcat
    ("\nhonour of Knight of the realm.  Upon thy name shall it be written. . .");
  lprcat ("\nBravery and courage be yours.");
  lprcat ("\nMay you live in happiness forevermore . . .\n");

  retcont ();

  return (1);
}



static int
letter3 (void)
{
  resetscroll ();
  screen_clear();

  lstandout ("From:");
  lprcat ("  Count Endelford\n");
  lstandout ("\nSubject:");
  lprcat ("  You Bastard!\n");

  lprcat ("\n   I heard (from sources) of your journey.  Congratulations!");
  lprcat ("\nYou Bastard!  With several attempts I have yet to endure the");
  lprcat (" caves,\nand you, a nobody, makes the journey!  From this time");
  lprcat (" onward, bewarned\nupon our meeting you shall pay the price!\n");

  retcont ();

  return (1);
}



static int
letter4 (void)
{
  resetscroll ();
  screen_clear();

  lstandout ("From:");
  lprcat ("  Mainair, Duke of Larnty\n");
  lstandout ("\nSubject:");
  lprcat ("  High Praise\n");

  lprcat
    ("\n   With a certainty a hero I declare to be amongst us!  A nod of");
  lprcat ("\nfavour I send to thee.  Me thinks Count Endelford this day of");
  lprcat ("\nright breath'eth fire as of dragon of whom ye are slayer.  I");
  lprcat ("\nyearn to behold his anger and jealously.  Should ye choose to");
  lprcat ("\nunleash some of thy wealth upon those who be unfortunate, I,");
  lprcat ("\nDuke Mainair, Shall equal thy gift also.\n");

  retcont ();

  return (1);
}



static int
letter5 (void)
{
  resetscroll ();
  screen_clear();

  lstandout ("From:");
  lprcat ("  St. Mary's Children's Home\n");
  lstandout ("\nSubject:");
  lprcat ("  these poor children\n");

  lprcat ("\n   News of your great conquests has spread to all of Larndom.");
  lprcat ("\nMight I have a moment of a great man's time.  We here at St.");
  lprcat ("\nMary's Children's Home are very poor, and many children are");
  lprcat ("\nstarving.  Disease is widespread and very often fatal without");
  lprcat
    ("\ngood food.  Could you possibly find it in your heart to help us");
  lprcat ("\nin our plight?  Whatever you could give will help much.");
  lprcat ("\n(your gift is tax deductible)\n");

  retcont ();

  return (1);
}



static int
letter6 (void)
{
  resetscroll ();
  screen_clear();

  lstandout ("From:");
  lprcat ("  The National Cancer Society of Larn\n");
  lstandout ("\nSubject:");
  lprcat ("  hope\n");

  lprcat
    ("\nCongratulations on your successful expedition.  We are sure much");
  lprcat
    ("\ncourage and determination were needed on your quest.  There are");
  lprcat ("\nmany though, that could never hope to undertake such a journey");
  lprcat ("\ndue to an enfeebling disease -- cancer.  We at the National");
  lprcat ("\nCancer Society of Larn wish to appeal to your philanthropy in");
  lprcat ("\norder to save many good people -- possibly even yourself a few");
  lprcat
    ("\nyears from now.  Much work needs to be done in researching this");
  lprcat
    ("\ndreaded disease, and you can help today.  Could you please see it");
  lprcat ("\nin your heart to give generously?  Your continued good health");
  lprcat ("\ncan be your everlasting reward.\n");

  retcont ();

  return (1);
}


/* 
* Page the mail to the terminal    - dgk
*/
void
readmail (int gold)
{
  letter1 (gold);
  letter2 ();
  letter3 ();
  letter4 ();
  letter5 ();
  letter6 ();
}
