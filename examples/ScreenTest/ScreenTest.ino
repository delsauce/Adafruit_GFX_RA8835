#include <Adafruit_GFX.h>
#include <Adafruit_GFX_RA8835.h>
#include <Parallel.h>

Adafruit_GFX_RA8835 lcd = Adafruit_GFX_RA8835(44);

#define FPS 5
#define RADIUS 20
#define RECT_WIDTH 40
#define RECT_HEIGHT 20

int16_t next, next2, xpos, ypos;
int16_t rectx, recty;

void setup(){
  lcd.begin();
  lcd.setCursor(25,0);
  for(byte i=1; i<=3; i++)
  {
    lcd.setTextSize(i);
    lcd.print(FPS);
    lcd.print(" fps ");
  }
  
  lcd.drawLine(0,0,319,239,1);
  lcd.drawLine(319,0,0,239,1); 
  
  xpos = 30;
  ypos = 120;
  next=-4;
  lcd.drawCircle(xpos,ypos,RADIUS,1);
  
  rectx = 140;
  recty = 190;
  lcd.fillRoundRect(rectx,recty,RECT_WIDTH,RECT_HEIGHT,4,1);
  
  unsigned long start = micros();
  lcd.update();
  unsigned long total = micros() - start;
  
  Serial.begin(9600);
  Serial.print("Screen write time: ");
  Serial.print(total);
  Serial.println("us");
}

void loop(){
  if (millis() % (1000/FPS) == 0)
  {
    // clear the previous circle
    lcd.drawCircle(xpos, ypos, RADIUS, 0);
    lcd.fillRoundRect(rectx,recty,RECT_WIDTH,RECT_HEIGHT,4,0);
    
    if (ypos < 3*RADIUS)
      next = 4;
      
    if (ypos > (lcd.height() - (3*RADIUS)))
      next = -4;
      
    ypos += next;
    rectx += next;
    
    lcd.drawCircle(xpos,ypos,RADIUS,1);
    lcd.fillRoundRect(rectx,recty,RECT_WIDTH,RECT_HEIGHT,4,1);
    
    lcd.update();
  }
}

