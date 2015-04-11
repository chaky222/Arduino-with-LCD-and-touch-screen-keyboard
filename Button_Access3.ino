//#include "my.h";
#include <UTFT.h>
#include <UTouch.h>
#include <EEPROM.h>

//PASSWORD PROPERTIES!
char master_password[20]="123123123123123";  // 5 * "123" :) 
bool enable_master_password = true; // if = true, then masster password is active and work, if = false, not work!
int lockOpenTime = 3000, dalay_time_for_error = 500;




extern uint8_t BigFont[];
UTFT        myGLCD(ITDB32S, 38,39,40,41); 
UTouch      myTouch(6,5,4,3,2);
int x_t = 0, y_t = 0, lock = 13;
int count_of_safe_checks = 5,stCurrentLen = 0, address = 0;
int  number_of_session = 0; // 0 - enter password, 1 - Enter old password for change password, 2 - Enter new password first, 3- new second.
char secret_code[20]="1112", tmp_new_secret_code[20]="",stCurrent[20]="";




/*************************
**   Custom functions   **
*************************/



String inttostr(int integers)
{
  char buf[10];
  memset (buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%d", integers);
  return buf;
};

class keyboard
{
       struct buttons
      {
        public:
          int x, y,  width ,height; bool visible;  String caption ;
           buttons(){ visible = true;  }
           void init(int x1,int y1, int width1,int height1,String caption1)
          {
            x = x1; y=y1; width = width1, height = height1;   caption = caption1;          
            button_redraw();
          }         
          void button_redraw ()
          {
                myGLCD.setColor(1, 1, 255);
                myGLCD.fillRoundRect (x, y, x+width, y+height);
                button_draw_normal(); 
                myGLCD.setBackColor(0, 0, 255);
                myGLCD.print(caption,(x+(width - (caption.length()*16))/2),y+(height-16)/2 );
                //button_draw_pressed();
          }
           void button_draw_pressed ()
          {
                myGLCD.setColor(255, 0, 0);
                for (int i1 = 0;i1<3;i1++){
                    myGLCD.drawRoundRect (x+i1, y+i1, x+width-i1, y+height-i1);  
                }
          }       
          void button_draw_normal ()
          {
                myGLCD.setColor(255, 255, 255);
                for (int i1 = 0;i1<3;i1++){
                    myGLCD.drawRoundRect (x+i1, y+i1, x+width-i1, y+height-i1);  
                }  
          } 
          bool button_is_pressed(int pressed_x,int pressed_y)
          {
            //Serial.print("search at pos x =");Serial.print(pressed_x);Serial.print(" y=");Serial.println(pressed_y);
            if ((pressed_x>x)&&(pressed_x<x+width)&&(pressed_y>y)&&(pressed_y<y+height)&&visible)
            {        
                return true;
            } else return false;
          }        
      };
      public:
      buttons keybrd[15];
      int count_of_buttons;
      
      keyboard()
      {
        count_of_buttons = 0;    
      };
      void add_a_new_button(int x1,int y1, int width1,int height1,String caption1)
      {
         keybrd[count_of_buttons].init(x1,y1,width1,height1,caption1);
         count_of_buttons++;
      };
      void redraw()
      {
            myGLCD.clrScr();
            for (int x = 0; x<count_of_buttons;x++)
            {
              if (number_of_session == 1){
                if (x>11)
                {
                    keybrd[x].visible = true;
                    keybrd[x].button_redraw();
                }else{
                    keybrd[x].visible = false;
                }
              }else {
                  if (x<=11){
                    keybrd[x].visible = true;
                    keybrd[x].button_redraw();
                  }else{
                    keybrd[x].visible = false;
                  }              
              }
            }     
      }
      int find_pressed_button(int x_pressed,int y_pressed)
      {                
            for (int i = 0; i<count_of_buttons;i++)
            {
                 if (keybrd[i].button_is_pressed(x_pressed,y_pressed))
                 {        
                    return i;
                 }
            }
            return -1;
      }
};

keyboard My_keyboard;
void draw_regim()
{
  // clear 
  myGLCD.setColor(1, 1, 255);
  myGLCD.fillRect(1, 1, 315, 35);
  myGLCD.setBackColor(0, 0, 255); 
  myGLCD.setColor(255, 255, 255);  
  switch (number_of_session) {
    case 1: myGLCD.print("Access Code Correct", CENTER, 10);  break; 
    case 2:myGLCD.print("Enter NEW Code",       CENTER, 10);  break;
    case 3:myGLCD.print("Retype NEW Code",      CENTER, 10);  break;     
    default: myGLCD.print("Enter Access Code",  CENTER, 10);
  } 
};
void save_new_password(char * new_password)
{
  strcpy(secret_code,new_password); 
  //saving to flash
  char buf[20] ="PASS";
  for (int i = 0;i<5;i++){
     EEPROM.write(address+i,buf[i]);
  } 
  for (int i = 0 ; i< 20 ;i++){
      EEPROM.write(address+i+5, new_password[i]);
  }
};
void load_password()
{
  char buf[20] ="";
  for (int i = 0;i<5;i++){
    buf[i] =  EEPROM.read(address+i);
  } 
  buf[5] = '\0';
 if (strcmp(buf,"PASS") !=0){
    //buf="1";         
  }else
  {
    for (int i1 = 0 ; i1< 20; i1++)
      {
        secret_code[i1] = EEPROM.read(address+5+i1);
      }  
  }

};

void ENTER_PRESSED()
{
   switch (number_of_session){
   case 1 :   break;
     case 2 : // ENTER NEW PASS
           strncpy(tmp_new_secret_code,stCurrent,19);  
           number_of_session = 3;          
       break;
      case 3 : // RETYPE NEW PASS
              if ((strcmp(stCurrent,tmp_new_secret_code) !=0))
              {
                print_big_message(255,0,0,0,0,255,"ERROR IN NEW CODE");
              }else{
                save_new_password(tmp_new_secret_code);
                print_big_message(0,255,0,0,0,255,"Code saved!");  
              }
                number_of_session = 0;      
       break;
                                     default : // ENTER access cod                     
                                        if ((strcmp(stCurrent,secret_code) ==0)||(enable_master_password&&(strcmp(stCurrent,master_password) ==0)) )
                                        {
                                          number_of_session = 1;                                          
                                          print_big_message(0,255,0,0,0,255,"ACCESS GRANTED!");                                                                                       
                                        }else{
                                          print_big_message(255,0,0,0,0,255,"ACCESS DENIED!");
                                         print_big_message(255,0,0,0,0,255,"ACCESS DENIED!");
                                          print_big_message(255,0,0,0,0,255,"ACCESS DENIED!");
                                          print_big_message(255,0,0,0,0,255,"ACCESS DENIED!");  
                                        }
                                     
         }// END OF SWITCH
                                  
  myGLCD.clrScr();
 My_keyboard.redraw();
 draw_regim(); 
 stCurrentLen=0;
 // TROYAN FOR EMPTY STRING HERE!!!! COMMENT IT FOR ACTIVATE BACKDooR!!!
 stCurrent[0]='\0';
  
};

void InitButtons()
{    
    int x=9;
    My_keyboard.add_a_new_button(10+(x*60)- 300*(x/5) ,40+ 60*(x/5),50,50,"0");
      for (x=0; x<9; x++)
      {
        My_keyboard.add_a_new_button(10+(x*60)- 300*(x/5) ,40+ 60*(x/5),50,50,inttostr(x+1));
     } 
    // Draw the lower row of buttons
    My_keyboard.add_a_new_button(10,160,140,50,"Clear");
    My_keyboard.add_a_new_button(160,160,140,50,"Enter");
    My_keyboard.add_a_new_button(10,40,310,70,"Open Door");
    My_keyboard.add_a_new_button(10,120,310,70,"Change Access Code");
    number_of_session =0;
}

void updateStr(int val)
{
  if (stCurrentLen<20)
  {
    stCurrent[stCurrentLen]=inttostr(val)[0];
    stCurrent[stCurrentLen+1]='\0';
    stCurrentLen++;
    myGLCD.setColor(1, 255, 1);
    myGLCD.print(stCurrent, 10, 222);
  }
  else
  {
    stCurrentLen = 0;
    stCurrent[0]='\0';
    print_big_message(255,0,0,0,0,255,"BUFFER FULL!");
    print_big_message(255,0,0,0,0,255,"BUFFER FULL!");
    myGLCD.clrScr();
    My_keyboard.redraw();
    draw_regim();
  }
};

void print_big_message(int cr,int cg,int cb,int crfon,int cgfon, int cbfon, char * str1 )
{
  myGLCD.clrScr();
  myGLCD.setBackColor(crfon, cgfon, cbfon);   
  myGLCD.setColor(cr, cg, cg);  
  myGLCD.print(str1, CENTER, 120);
  myGLCD.setColor(crfon, cgfon, cbfon);
  delay(dalay_time_for_error);
  myGLCD.print(str1, CENTER, 120);
  myGLCD.setBackColor(0, 0, 255); 
  delay(dalay_time_for_error);
   
};






// Draw a red frame while a button is touched
void waitForIt(int x1, int y1, int x2, int y2)
{
  myGLCD.setColor(255, 1, 1);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}

/*************************
**  Required functions  **
*************************/

void setup()
{
// Initial setup
 load_password();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  
  myGLCD.InitLCD();
  myGLCD.clrScr();
  
  My_keyboard = keyboard();
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  InitButtons(); 
  
  My_keyboard.redraw();
  draw_regim();
  stCurrentLen = 0;
  stCurrent[0]='\0';
  digitalWrite(lock, LOW);
  pinMode(lock, OUTPUT);
}

void loop()
{
   if (myTouch.dataAvailable())
    {
      myTouch.read();
      int button_number = My_keyboard.find_pressed_button(myTouch.getX(),myTouch.getY());
      if (button_number>-1)
      {
        My_keyboard.keybrd[button_number].button_draw_pressed();
        while (myTouch.dataAvailable()) myTouch.read();
        My_keyboard.keybrd[button_number].button_draw_normal();
        if (button_number<10){
          updateStr(button_number);
        }
        if (button_number==10){ // CLEAR BUTTON
                stCurrent[0]='\0';
                stCurrentLen=0;
                myGLCD.setColor(0, 0, 255);
                myGLCD.fillRect(1, 215, 319, 239);                                     
        }
         if (button_number==11)
         { // ENTER BUTTON
             ENTER_PRESSED();                            
         }
         if (button_number==12){ // Open door pressed
             number_of_session = 0;
             digitalWrite(lock, HIGH);
             print_big_message(0,255,0,0,0,255,"ACCESS GRANTED!");
             print_big_message(0,255,0,0,0,255,"Welcome Back Jordan!");
             print_big_message(0,255,0,0,0,255,"Welcome Back Jordan!");
             delay(lockOpenTime);                     
             digitalWrite(lock, LOW);   
             myGLCD.clrScr();
             My_keyboard.redraw();
             draw_regim();                           
          }
         if (button_number==13){ // New pass BUTTON
             number_of_session = 2;
             myGLCD.clrScr();
             My_keyboard.redraw();
             draw_regim();
             //myGLCD.setColor(1, 1, 1);
             //myGLCD.fillRect(1, 224, 319, 239);                                     
          }
         
        
      }
    } 

}





