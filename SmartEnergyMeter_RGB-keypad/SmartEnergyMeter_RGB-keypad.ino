/*
  Project "Smart Energy Meter" using current sensor arduino and 4x4 keypad and a display
  To monitor energy consumption and cost and delelop a pre-paid system with notification and digital control of sort...
  Developed by Sakib Ahmed Sumdany.
  Built on the skeleton of the previous version having gsm module.
  November 30, 2016.
  Released into the public domain.
  Dependencies: ACS712 Library by Sakib Ahmed
*/

#include<EEPROM.h>


String bal="";
String creditCode="*55#";  //recharge code
//////////////////////////////////////////

long keyPadTime=40000;


//Used for LCD Screen 
#include <LiquidCrystal.h>

LiquidCrystal lcd(2, A1, A2, A3, A4, A5);


#include <Keypad.h>

char Keys[4][4] = {
    { '1','2','3', 'A' },
    { '4','5','6', 'B' },
    { '7','8','9', 'C' },
    { '*','0','#', 'D' }
  };


byte rowPins[4] = {11,10,9,8}; //connect to the row pinouts of the keypad
byte colPins[4] = { 7, 6,5,4}; //connect to the column pinouts of the keypad

Keypad kpd( makeKeymap(Keys), rowPins, colPins, 4, 4 );


    unsigned long loopCount;
    unsigned long startTime;
    String msg;






/////////

//For current Sensor
#include "ACS712.h"
float error= 0.05;    // initial error of ACS712 sensor
 ACS712 ac1(A0, error);
double current=0L;
double power=0L;
double load=0L;
float balance=0.00f;
float unit =0.00f;
float unitPrice =5.00f;
long lastPrint = millis();
long lastDtl = millis();

///////////////////////////
int relay = 2;             //pin connected to relay module
//////////////////////////////



char key;


////////////////////////////
void setup(){

  pinMode(relay, OUTPUT);

  //Serial.begin(9600);

  reStoreInfo();   //restore data from arduino's Static memory after reboot/ power failure
  printBalance();

  lcd.begin(20,4);


    kpd.setDebounceTime(1); 
    kpd.setHoldTime(15); 

    startTime = millis();
    msg = "Pressed: ";

}

void loop(){
  keyEvent();


//userInteraction();
RechargeButton();

 
if(balance>=0)
{
  currentCalculation();
  storeInfo();
  printDetails();
  printBalance();
  digitalWrite(relay,HIGH);
  }else{
      digitalWrite(relay,LOW);
      lcd.setCursor(0,0);                             //since the lcd library doesnt provide a line/row clearance functionality
      lcd.print("                    ");              // this is to reset/clear a single row/line of the lcd
      lcd.setCursor(0,0);
      lcd.print("No Balance");
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge!!");
      
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      delay(1);
    }
checkLowBalance();
ResetButton();

}

void ResetButton()
{
//if(digitalRead(pushButton)==0)  
//  Serial.println("Reseting");
if(key=='C')
 {
     
 
        
        lcd.clear();
        lcd.print("RESETTING...");
        delay(1500);
        
        lcd.clear();
        lcd.print("Reset Done");
        msg="Pressed: ";
        delay(1000);
        lcd.clear();
        balance=00.00f;
        unit=00.00f;
        for (int i = 0 ; i < EEPROM.length() ; i++) EEPROM.write(i, 0);    //clear all EEPROM
        storeInfo();
        printBalance();
        //alert=true;
    
  }   
}
void RechargeButton(){

    if ( (millis()-startTime)>keyPadTime && msg.length()>0 ) {  //time ot for key pressed
        msg="pressed: ";
        startTime = millis();
    }
          
              lcd.setCursor(0,3);
              lcd.print("                    ");
              lcd.setCursor(0,3);
              lcd.print(msg);

              if(msg.indexOf(creditCode)>0){
                delay(500);
                balance+=1000.00f;
                lcd.clear();
                lcd.print("Recharge successful");
                delay(1000);
                lcd.clear();
                msg="pressed: ";
              }

}

void keyEvent(){
          key=kpd.getKey();
        
          if(key){
            msg+=key;
            //Serial.println(msg);
            }
  
  }


  
void userInteraction()
{    
        
  if (Serial.available()>0)
    {
      char cmd = Serial.read();
      Serial.println(cmd); 
      
      if(cmd=='R') {
        balance=00.00f;
        unit=00.00f;
        for (int i = 0 ; i < EEPROM.length() ; i++) EEPROM.write(i, 0);    //clear all EEPROM
        storeInfo();
        printBalance();
      }            
      if(cmd=='U') {
        balance+=1000.00f;
        printBalance();
        //alert = true;
        
        }
      if(cmd=='G') {
        unit+=10.00f;
        printBalance();
      
      }
   }
}


void checkLowBalance()
{
  if(balance < 20 && balance>0)     //notify customer about low balance
  {
      lcd.setCursor(0,0);
      lcd.print("                   ");
      lcd.setCursor(0,0);
      
      lcd.print("ALART!!LOW BALANCE!");
      delay(1);
  }
}

void printBalance()
{
  lcd.setCursor(0,1);
  lcd.print("                    ");
  lcd.setCursor(0,1);
      

  lcd.print("Balance: ");
  lcd.print(balance,2);
  lcd.print(" TK");

  lcd.setCursor(0,2);
  lcd.print("                    ");
  lcd.setCursor(0,2);

  
  lcd.print("Usage: ");
  lcd.print(unit,5);
  lcd.print(" Units");


  }

void currentCalculation()
{
   current = ac1.getACcurrent();   //power calculations
   power=current*220.00L;
   load =(power/(1000.0L*3600.0L)); // instantanious load
   unit+=load;                      //Cumulated load or power consumption 
   balance -=load*unitPrice;        // balance adjustment  

   
  }

//printing...

void printDetails()
{
     //Serial.print("\t");
     //Serial.print(current);
     //Serial.print(" Amps, ");
      //Serial.print(power);
     //Serial.print(" Watt,   ");
      //Serial.print(unit,5);
     //Serial.println(" KWatt-hour\t");



  lcd.setCursor(0,0);
  lcd.print("                    ");
  lcd.setCursor(0,0);
  lcd.print("Load: ");
  lcd.print(current,3);
  lcd.print("Amps");
     
}


//eeprom functions
void storeInfo()
{
//  EEPROM[100]=unit;
//  EEPROM[0]=balance;
    
  EEPROM.put(0,unit);
  EEPROM.put(100,balance);
  

  
}

void reStoreInfo()
{
//  unit= EEPROM[100];
//  balance= EEPROM[0];
    
  EEPROM.get(0,unit);
  EEPROM.get(100,balance);
  
}








  
