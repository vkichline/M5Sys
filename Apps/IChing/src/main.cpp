#define M5STACK_MPU6886
#include <M5Sys.h>
#include <M5Hexagram.h>

M5Sys       m5sys;
IChing      iching;
M5Hexagram  hex;


void throw_hexagram() {
  VERBOSE("throw_hexagram()\n");
  hex.generate();
  hex.show(true);
}


void select_a_hexagram() {
  VERBOSE("select_a_hexagram()\n");
  ez.header.show("Select a Hexagram");
  ez.buttons.show("up # left # select # Back # down # right");
  ez.canvas.font(&FreeMono9pt7b);
  ez.canvas.lmargin(30);
  ez.canvas.clear();
  int selection = 1;
  while(true) {
    ez.canvas.y(50);
    for(int x = 0; x < 8; x++) {
      for(int y = 1; y <= 8; y++) {
        ez.canvas.printf("%2d ", (x * 8) + y);
      }
      ez.canvas.println();
    }
    M5.Lcd.drawRect(28 + (((selection-1) % 8) * 33), 46 + ((selection-1) / 8) * 18, 25, 20, RED);
    String result = ez.buttons.wait();
    DEBUG("result = %s\n", result.c_str());
    int old_selection = selection;
    if(     0 == result.compareTo("Back")) return;
    else if(0 == result.compareTo("select")) { hex.load_from_id(selection); hex.show(); return;    }
    else if(0 == result.compareTo("left"))   { selection--;    if( 1 > selection) selection  = 64; }
    else if(0 == result.compareTo("right"))  { selection++;    if(65 < selection) selection  =  1; }
    else if(0 == result.compareTo("up"))     { selection -= 8; if( 1 > selection) selection += 64; }
    else if(0 == result.compareTo("down"))   { selection += 8; if(64 < selection) selection -= 64; }
    DEBUG("old_selection = %2d, selection = %2d\n", old_selection, selection);
    if(old_selection != selection) M5.Lcd.drawRect(28 + (((old_selection-1) % 8) * 33), 46 + ((old_selection-1) / 8) * 18, 25, 20, ez.theme->background);
  }
}


void history() {
  VERBOSE("history()\n");
  ez.textBox("History of the I Ching", "The I Ching or Yi Jing, usually translated as Book of Changes or Classic \
of Changes, is an ancient Chinese divination text and the oldest of the Chinese classics. With more than two and a \
half millennia's worth of commentary and interpretation, the I Ching is an influential text read throughout the world, \
providing inspiration to the worlds of religion, philosophy, literature, and art. Originally a divination manual in \
the Western Zhou period, over the course of the Warring States period and early imperial period it was transformed into \
a cosmological text with a series of philosophical commentaries known as the \"Ten Wings\". After becoming part of the \
Five Classics in the 2nd century BC, the I Ching was the subject of scholarly commentary and the basis for divination \
practice for centuries across the Far East, and eventually took on an influential role in Western understanding of Eastern thought.\
\nWikipedia, 2020.",
  true, "up#Done#down", &FreeSans9pt7b);
}


void credits() {
  VERBOSE("credits()\n");
  ez.msgBox("Credits", "The Chinese Classic: I Ching\nText by King Wen, the Duke of Zhou and Confucius, 1000 BC\n\
Translation by Wilhelm/Baynes, 1950\nCode by Van Kichline, in the Year of the Plague", "OK", true, &FreeSans9pt7b);
}


void setup() {
  m5sys.begin("IChing", NETWORK_CONNECTION_NONE);
  iching.begin();
}


void loop() {
  VERBOSE("loop()\n");
  ezMenu m("The I Ching");
  m.txtSmall();
  m.buttons("up # back | Home # select ## down #");
  m.addItem("Cast the Oracle",   throw_hexagram);
  m.addItem("Select a Hexagram", select_a_hexagram);
  //m.addItem("Create a Hexagram");
  m.addItem("History",           history);
  m.addItem("Credits",           credits);
  m.run();                       // exits when back (Home) is pressed
  m5sys.goHome();                // load menu program
}
