#include <rtc_clock.h>
#include <KeyboardController.h>
#include <LiquidCrystal.h>

/***********************************************************************************************************************************************************************************
  Programma per una centralina di irrigazione
  Versione 1.4
  Last update: 21/06/2013
  
  Questa centralina è in grado di gestire 12 elettrovalvole (si possono aumentare facilmente)
  Creata appositamente per Arduino Due
  Funzioni:
  -Impostazione data e ora
  -Impostare tempo delle zone
  -Orario di partenza
  -Tipologia del programma (quali giorni andare o per quanti giorni)
  -Aumento o diminuzione in % di tutte le zone
  -Programma semiautomatico
  -Programma manuale
  
  Relase:
  1.1 Implementazione del display 24x4 
  1.2 Implementazione secondo display 16x2
  1.3 Fixato problema irrigazione semi-automatica
  1.4 Fixate scritte che fuoriescono dal display con una revisione generale
      Aggiunta funzione per visualizzare il tempo di tutte le zone sui display
      Sistemato problema sottomenù 3, non accettava percentuali sotto il 100%, adesso non accetta quelle sopra il 100%
  
  Created by:
  Tollari Mattia
***********************************************************************************************************************************************************************************/

// Initializzo l'interfaccia USB
USBHost usb;

// Inizializzo i pin del display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // 24x4
LiquidCrystal lcd2(22, 23, 24, 25, 26, 27); // 16x2

// Definisco la tastiera collegata tramite USB
KeyboardController keyboard(usb);

//Inizializzo l'ora
RTC_clock rtc_clock(XTAL);

// Inizializzazione delle variabili:
char taspre; // Tasto premuto
int taspreint=0; // Tasto premuto in intero
int ora=0,minuti=0,sec=0,gio=0,mes=0,ann=0;
int giomes[12]={31,28,31,30,31,30,31,31,30,31,30,31}; // Giorni per ogni mese
int numpro=1; // Numero del programma (1-2-3)
int giosi=0,giono=0,giosiapp,gionoapp; // Variabile necessaria per il punto 2
int durzon[12]={0}; // Durata di ciascuna zona in minuti
int per=0; // Percentuale di incremento per punto 3
int numgioset=0; // Numero di giorno della settimana 
int orainpro,mininpro; // Ora e minuti per l'inizio del programma
int apptemirr=0,apptemirr2=0; // Variabile di appoggio utilizzata per determinare quando finire l'irrigazione di una zona
int zonman=0; // Zona che scelgo per il programma manuale
int secapp=0; // Variabile di appoggio per il secondo display
boolean gioirr[8]={LOW}; // Vettore necessario per il punto 2
boolean tastprem=LOW; // Booleana utilizzata per controllare se un tasto è stato premuto
boolean oraimp=LOW; // Booleana utilizzata per controllare se l'ora è stata impostata
boolean oraval=LOW,minval=LOW,gioval=LOW,mesval=LOW,annval=LOW; // Booleane utilizzate per controllare se le ore, i minuti, il giorno, il mese e l'anno sono validi
boolean tipirr=LOW; // Booleana per decidere quale tipologia di irrigazione deve essere fatta (vedi punto 2)
boolean conval=LOW; // Booleana per verificare se inserisco un numero valido nel punto 2
boolean tipper=LOW; // Booleana per decidere se incremento o decremento il valore in percentuale
boolean perval=LOW; // Booleana per verificare se inserisco un numero valido nel punto 3
boolean segnato=LOW; // Booleana per verificare se ho già decrementato il giorno da quelli da irrigare
boolean finito=LOW; // Booleana che verifica se ha finito il tempo di una zona
boolean numgiosetcam=LOW; // Booleana che determina se è già stato cambiato il numero del giorno
boolean irrpart=LOW; //Booleana che determina se è partita l'irrigazione
boolean vismendis=LOW; // Booleana che determina se ho già visualizzato il menù principale del display, la utilizzo perchè il refresh continuo del loop non permetterebbe di visualizzare bene tutto.
boolean zonval=LOW; // Booleana che determina se ho inserito una zona valida nel programma manuale

// Questa funzione rileva il tasto premuto
void keyPressed() 
{
  taspre=keyboard.getKey();
  taspreint=keyboard.getKey()-48;
  tastprem=HIGH;
  //Serial.print("Pressed:  ");
  //Serial.print(keyboard.getKey());
  Serial.println();
}

void inizioirrigazione()
{
  for(int i=0; i<=3; i++)
  {
    if(durzon[i]>0)
    {
      digitalWrite(i+30, HIGH);
      finito=LOW;
      ora=rtc_clock.get_hours();
      minuti=rtc_clock.get_minutes();
      apptemirr=ora*60+minuti+durzon[i];
      taspreint=-1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Inizio");
      lcd.setCursor(0,1);
      lcd.print("elettrovalvola ");
      lcd.print(i+1);
      lcd.setCursor(0,2);
      lcd.print("Premere un tasto per");
      lcd.setCursor(0,3);
      lcd.print("fermare");
      Serial.print("Inizio elettrovalvola ");
      Serial.print(i+1);
      Serial.println();
      Serial.print("Sono le ");
      Serial.print(ora);
      Serial.print(":");
      Serial.print(minuti);
      Serial.println();
      while(!finito && taspreint==-1)
      {
        visoraedata();
        usb.Task();
        ora=rtc_clock.get_hours();
        minuti=rtc_clock.get_minutes();
        apptemirr2=ora*60+minuti;
        if(apptemirr-apptemirr2<=0) finito=HIGH;
      }
      digitalWrite(i+30, LOW);
    }
  }
vismendis=LOW;
}

void   visoraedata()
{
  sec=rtc_clock.get_seconds();
  if(sec!=secapp)
  {
    secapp=sec;
    ora=rtc_clock.get_hours();
    sec=rtc_clock.get_seconds();
    gio=rtc_clock.get_days();
    mes=rtc_clock.get_months();
    ann=rtc_clock.get_years();
    lcd2.clear();
    lcd2.setCursor(0,0);
    if(ora<10)lcd2.print("0");
    lcd2.print(ora);
    lcd2.print(":");
    if(minuti<10)lcd2.print("0");
    lcd2.print(minuti);
    lcd2.setCursor(0,1);
    if(gio<10)lcd2.print("0");
    lcd2.print(gio);
    lcd2.print("/");
    if(mes<10)lcd2.print("0");
    lcd2.print(mes);
    lcd2.print("/");
    lcd2.print(ann);
  }

}

void setup() 
{
  Serial.begin(9600);
  Serial.println("Program started");
  delay(500);
  
  //inizializzo il display
  lcd.begin(24, 4);
  lcd.print("Program started");
  lcd2.begin(16, 2);
  lcd2.print("Program started");
  delay(1000);
  lcd2.clear();
  lcd2.print("Impostare");
  lcd2.setCursor(0,1);
  lcd2.print("ora e data");
  // Imposto le varie zone come uscita con un ciclo for
  for(int i=30;i<=33;i++)
    {
      pinMode(i, OUTPUT);
      digitalWrite(i, LOW);
    }
  
}

void loop() 
{
  // 
  usb.Task();
  
  // Controllo dell'ora
  if(!oraimp)
  {
    taspreint=7;
    tastprem=HIGH;
  }
  
  // Utilizzo un menù per la scelta di cosa fare
  /********************************************************************************************************************************************************************************
  * Scelte:                                                                                                                                                                       *
  * 0-Impostare l'ora di partenza                                                                                                                                                 *
  * 1-Impostare il tempo di irrigazione (0 se non si vuole far andare una zona)                                                                                                   *
  * 2-Impostare quali giorni andare (o quanti si e quanti no)                                                                                                                     *
  * 3-% di aumento o diminuzione totale del tempo delle zone                                                                                                                      *
  * 4-Programma semiautomatico                                                                                                                                                    *    
  * 5-Programma manuale                                                                                                                                                           *
  * 6-Impostare data e ora                                                                                                                                                        *
  ********************************************************************************************************************************************************************************/
  
  if(!vismendis)
  {
    
    // Menu' display
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Menu' principale");
    lcd.setCursor(0,1);
    lcd.print("Digitare un tasto");
    lcd.setCursor(0,2);
    lcd.print("Da 1 a 8");
    lcd.setCursor(0,3);
    vismendis=HIGH;
  }
  
  //Se un tasto è stato premuto
  if(tastprem)
  {
    lcd.print(taspreint);
    vismendis=LOW;
    delay(500);
    switch(taspreint)
    {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
      case 1:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento ora
      Serial.print("Inserire ore: ");
      oraval=LOW;
      while(!oraval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Tempo di partenza");
        lcd.setCursor(0,1);
        lcd.print("Ore: ");
        
        taspreint=-1;
        while(taspreint==-1)
        {
          usb.Task();
        }
        Serial.print(taspreint);
        orainpro=taspreint*10;
        if(orainpro<=20)
        {
          lcd.print(taspreint);
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          Serial.print(taspreint);
          orainpro+=taspreint;
        }
        if(orainpro<=23 && orainpro>=0)oraval=HIGH;
        else 
        {
          Serial.print("Ora inserita non valida, reinserire:");
          lcd.print(taspreint);
          lcd.setCursor(0,2);
          lcd.print("Ora non valida");
          delay(1000);
         }
      }
      lcd.print(taspreint);
      lcd.setCursor(0,2);
      lcd.print("Ora valida");
      Serial.print("Ora inserita valida");
      Serial.print(orainpro);
      delay(1000);
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento minuti
      minval=LOW;
      Serial.println();
      Serial.print("Inserire minuti: ");
      minval=LOW;
      while(!minval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Tempo di partenza");
        lcd.setCursor(0,1);
        lcd.print("Ore: ");
        lcd.print(orainpro);
        lcd.setCursor(0,2);
        lcd.print("Minuti: ");
        taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
        
        Serial.print(taspreint);
        mininpro=taspreint*10;
        if(mininpro<=50)
        {
          lcd.print(taspreint);
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          Serial.print(taspreint);
          mininpro+=taspreint;
        }
        if(mininpro<=59 && mininpro>=0)minval=HIGH;
        else 
        {
          Serial.print("Minuti inseriti non validi, reinserire:");
          lcd.print(taspreint);
          lcd.setCursor(0,3);      
          lcd.print("Minuti non validi");
          delay(1000);
         }
      }
      lcd.print(taspreint);
      lcd.setCursor(0,3);
      lcd.print("Minuti validi");
      Serial.print("Minuti inseriti validi");
      Serial.print(mininpro);
      Serial.println();
      tastprem=LOW;
      delay(1000);
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
      case 2:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      Serial.print("Impostazione del tempo di tutte le zone (max 60 min):");
      Serial.println();
      for(int i=0; i<=3; i++)
        {
          minval=LOW;
          while(!minval)
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Impostazione tempo");
            lcd.setCursor(0,1);
            lcd.print("delle zone:");
            lcd.setCursor(0,2);
            lcd.print("Zona ");
            lcd.print(i+1);
            lcd.print(":");
            Serial.print("Zona ");
            Serial.print(i+1);
            Serial.print(": ");
            taspreint=-1;
            while(taspreint==-1)
            {
                usb.Task();
            }
           Serial.print(taspreint);
           minuti=taspreint*10;
           if(minuti<=60)
           { 
              lcd.print(taspreint);
              taspreint=-1;
              while(taspreint==-1)
                {
                  usb.Task();
              }
              Serial.print(taspreint);
              minuti+=taspreint;
           }
           if(minuti<=60 && minuti>=0)minval=HIGH; 
           else 
           {
             Serial.print("Minuti inseriti non validi, reinserire:");
             lcd.print(taspreint);
             lcd.setCursor(0,3); 
             lcd.print("Minuti non validi");
             delay(1000);
            }
           }
           durzon[i]=minuti;
           Serial.print("Minuti inseriti validi: ");
           lcd.print(taspreint);
           lcd.setCursor(0,3); 
           lcd.print("Minuti validi");
           Serial.print(minuti);
           Serial.println();
           delay(1000);  
        }
      
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
      case 3:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      
      Serial.print("Selezionare il programma:");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Selezionare il");
      lcd.setCursor(0,1);
      lcd.print("programma");
      lcd.setCursor(0,2);
      lcd.print("1 o 2:");
      lcd.setCursor(0,3);
      
      conval=LOW;
      while(!conval)
      {
        taspreint=-1;
        while(taspreint==-1)
        {
          usb.Task();
        }
        if(taspreint-1==0)
        {
          tipirr=LOW;
          conval=HIGH;
        }
        if(taspreint-1==1)
        {
          tipirr=HIGH;
          conval=HIGH;
        }
      }
      lcd.print(taspreint);
      delay(1000);
      // Irrigazione definendo i giorni
      if(!tipirr)
      {
        for(int i=1; i<=7; i++)
        {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Programma 1:");
            lcd.setCursor(0,1);
            lcd.print("Giorno ");
            lcd.print(i);
            lcd.print(" :");
            lcd.setCursor(0,2);
            Serial.print("giorno ");
            Serial.print(i);
            Serial.println();
            conval=LOW;
            while(!conval)
            {
              taspreint=-1;
              while(taspreint==-1)
              {
                usb.Task();
              }
              if(taspreint==0)
              {
                lcd.print("OFF");
                gioirr[i]=LOW;
                conval=HIGH;
                delay(500);
              }
              if(taspreint==1)
              {
                lcd.print("ON");
                gioirr[i]=HIGH;
                conval=HIGH;
                delay(500);
              }
            }
        }
      }
      // Irrigazione definendo quanti si e quanti no
      if(tipirr)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Programma 2:");
        lcd.setCursor(0,1);
        lcd.print("Giorni di");
        lcd.setCursor(0,2);
        lcd.print("irrigazione:");
        lcd.setCursor(0,3);
        // Giorni si
        Serial.print("giorni si: ");
        taspreint=-1;
        while(taspreint==-1)
        {
          usb.Task();
        }
        lcd.print(taspreint);
        giosi=taspreint;
        giosiapp=taspreint;
        Serial.print(giosi);
        Serial.println();
        // Giorni no
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Programma 2:");
        lcd.setCursor(0,1);
        lcd.print("Giorni di");
        lcd.setCursor(0,2);
        lcd.print("inattivita':");
        lcd.setCursor(0,3);
        Serial.print("giorni no: ");
        taspreint=-1;
        while(taspreint==-1)
        {
           usb.Task();
        }
        lcd.print(taspreint);
        giono=taspreint;
        gionoapp=taspreint;
        Serial.print(giono);
        Serial.println();
      }
      tastprem=LOW;
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
      case 4:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      
      Serial.print("Selezionare se incrementare o decrementare:");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Premere");
      lcd.setCursor(0,1);
      lcd.print("1 per decrementare");
      lcd.setCursor(0,2);
      lcd.print("2 per aumentare");
      lcd.setCursor(0,3);

      perval=LOW;
      while(!perval)
      {
        taspreint=-1;
        while(taspreint==-1)
        {
          usb.Task();
        }
        if(taspreint-1==0)
        {
          tipper=LOW;
          perval=HIGH;
        }
        if(taspreint-1==1)
        {
          tipper=HIGH;
          perval=HIGH;
        }
      }
      lcd.print(taspreint);
      delay(500);
      
      perval=LOW;
      while(!perval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Inserire la");
        lcd.setCursor(0,1);
        lcd.print("percentuale");
        lcd.setCursor(0,2);
        taspreint=-1;
        while(taspreint==-1)
        {
          usb.Task();
        }
        Serial.print(taspreint);
        lcd.print(taspreint);
        per=taspreint*100;
        if(per>=100)
        {
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          per+=taspreint*10;
          /////////////////////////
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          per+=taspreint;
        }
        lcd.setCursor(0,3);
        if(per<=100)
        {
          perval=HIGH;
          lcd.print("percentuale valida");
        }
        else 
        {
          Serial.print("Percentuale inserita non valida, reinserire:");
          lcd.print("Percentuale non valida");
        }      
      }
      if(!tipper)
      {
        for(int i=0; i<=11; i++)
        {
          durzon[i]-=durzon[i]*per/100;
        }
      }
      if(tipper)
      {
        for(int i=0; i<=11; i++)
        {
          durzon[i]+=durzon[i]*per/100;
          if(durzon[i]>60)durzon[i]=60;
        }
      }
      lcd.setCursor(0,3);
      lcd.print("Operazione terminata");
      delay(1500);
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
      case 5:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      inizioirrigazione();
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      case 6:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      zonval=LOW;
      while(!zonval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Programma manuale:");
        lcd.setCursor(0,1);
        lcd.print("Digitare la zona");
        lcd.setCursor(0,2);
        lcd.print("da testare:");
        lcd.setCursor(0,3);
        taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
        
        Serial.print(taspreint);
        zonman=taspreint-1;
        if(zonman<=4 && zonman>=0)zonval=HIGH;
        else 
        {
          Serial.print("Zona inserita non valida, reinserire:");
          lcd.print(taspreint);
          lcd.setCursor(0,3);      
          lcd.print("Zona non valida");
          delay(1000);
         }
      }
      
      digitalWrite(zonman+30, HIGH);
      Serial.write("Premere un qualsiasi tasto per arrestare");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Premere un qualsiasi");
      lcd.setCursor(0,1);      
      lcd.print("tasto per arrestare");
      taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
      digitalWrite(zonman+30, LOW);
      
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      case 7:
      Serial.print("case ");
      Serial.print(taspreint);
      Serial.println();
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento ora
      Serial.print("Inserire ore: ");
      oraval=LOW;
      while(!oraval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Inserimento ora");
        lcd.setCursor(0,1);
        lcd.print("Ore: ");
        
        taspreint=-1;
        while(taspreint==-1)
        {
          usb.Task();
        }
        Serial.print(taspreint);
        orainpro=taspreint*10;
        if(orainpro<=20)
        {
          lcd.print(taspreint);
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          Serial.print(taspreint);
          orainpro+=taspreint;
        }
        if(orainpro<=23 && orainpro>=0)oraval=HIGH;
        else 
        {
          Serial.print("Ora inserita non valida, reinserire:");
          lcd.print(taspreint);
          lcd.setCursor(0,2);
          lcd.print("Ora non valida");
          delay(1000);
         }
      }
      ora=orainpro;
      lcd.print(taspreint);
      lcd.setCursor(0,2);
      lcd.print("Ora valida");
      Serial.print("Ora inserita valida");
      Serial.print(orainpro);
      delay(1000);
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento minuti
      minval=LOW;
      Serial.println();
      Serial.print("Inserire minuti: ");
      minval=LOW;
      while(!minval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Inserimento ora");
        lcd.setCursor(0,1);
        lcd.print("Ore: ");
        lcd.print(orainpro);
        lcd.setCursor(0,2);
        lcd.print("Minuti: ");
        taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
        
        Serial.print(taspreint);
        mininpro=taspreint*10;
        if(mininpro<=50)
        {
          lcd.print(taspreint);
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          Serial.print(taspreint);
          mininpro+=taspreint;
        }
        if(mininpro<=59 && mininpro>=0)minval=HIGH;
        else 
        {
          Serial.print("Minuti inseriti non validi, reinserire:");
          lcd.print(taspreint);
          lcd.setCursor(0,3);      
          lcd.print("Minuti non validi");
          delay(1000);
         }
      }
      minuti=mininpro;
      lcd.print(taspreint);
      lcd.setCursor(0,3);
      lcd.print("Minuti validi");
      Serial.print("Minuti inseriti validi");
      Serial.print(mininpro);
      Serial.println();
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento anno
      Serial.println();
      Serial.print("Inserire l'anno: ");
      annval=LOW;
      while(!annval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Inserimento data");
        lcd.setCursor(0,1);
        lcd.print("Anno: ");
        taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
        lcd.print(taspreint);
        Serial.print(taspreint);
        ann=taspreint*1000;
        if(ann>=2000)
        {
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          ann+=taspreint*100;
          /////////////////////////
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          ann+=taspreint*10;
          /////////////////////////
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          ann+=taspreint;
        }
        if(ann>=2000)annval=HIGH;
        else
        {
          Serial.print("Anno inserito non valido, reinserire:");      
          lcd.setCursor(0,2);
          lcd.print("Anno non valido");
          delay(1000);
        }
      }
      Serial.print("Anno inserito valido: ");
      Serial.print(ann);
      lcd.setCursor(0,2);
      lcd.print("Anno valido");
      Serial.println(); 
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento mese
      Serial.println();
      Serial.print("Inserire mese: ");
      mesval=LOW;
      while(!mesval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("__/__/");
        lcd.print(ann);
        lcd.setCursor(0,1);        
        lcd.print("Inserimento data");
        lcd.setCursor(0,2);
        lcd.print("Mese: ");
        taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
        lcd.print(taspreint);
        Serial.print(taspreint);
        mes=taspreint*10;
        if(mes<=10)
        {
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          mes+=taspreint;
        }
        if(mes<=12 && mes>=0)mesval=HIGH;
        else
        {
          Serial.print("Mese inserito non valido, reinserire:");      
          lcd.setCursor(0,3);
          lcd.print("Mese non valido");
          delay(1000);
        }
      }
      Serial.print("Mese inserito valido: ");
      Serial.print(mes);
      lcd.setCursor(0,3);
      lcd.print("Mese valido");
      Serial.println();
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento giorno
      Serial.println();
      Serial.print("Inserire giorno: ");
      gioval=LOW;
      while(!gioval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("__/");
        lcd.print(mes);
        lcd.print("/");
        lcd.print(ann);
        lcd.setCursor(0,1);        
        lcd.print("Inserimento data");
        lcd.setCursor(0,2);
        lcd.print("Giorno: ");
        taspreint=-1;
       while(taspreint==-1)
        {
          usb.Task();
        }
        lcd.print(taspreint);
        Serial.print(taspreint);
        gio=taspreint*10;
        if(gio<=giomes[mes-1])
        {
          taspreint=-1;
          while(taspreint==-1)
          {
            usb.Task();
          }
          lcd.print(taspreint);
          Serial.print(taspreint);
          gio+=taspreint;
        }
        // Controllo se il mese è febbraio
        int appmes; // Variabile di appoggio per il mese
        int appann=ann; // Variabile di appoggio per l'anno
        if(mes==2)
        {
         appann-=2000; // Il 2000 era un anno bisestile, lo utilizzo per calcorare se l'anno inserito è anch'esso bisestile e velocizzare i calcoli
         while(appann>0)
         {
           appann-=4;
         }
         if(appann==0)appmes=29;
         else appmes=28;
        }
        else appmes=giomes[mes-1];
        if(gio<=appmes && gio>=0)gioval=HIGH;
        else
        {
          Serial.print("Giorno inserito non valido, reinserire:"); 
          lcd.setCursor(0,3);
          lcd.print("Giorno non valido");
          delay(1000);
        }
      }
      Serial.print("Giorno inserito valido: ");
      Serial.print(gio);
      lcd.setCursor(0,3);
      lcd.print("Giorno valido");
      Serial.println();
      
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Inserimento giorno della settimana
      Serial.println();
      Serial.print("Inserire numero del giorno della settimana: ");
      gioval=LOW;
      while(!gioval)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Inserimento giorno");
        lcd.setCursor(0,1);
        lcd.print("della settimana");
        lcd.setCursor(0,2);
        lcd.print("Giorno: ");
        taspreint=-1;
       while(taspreint==-1)
       {
         usb.Task();
       }
       lcd.print(taspreint);
       numgioset=taspreint;
       if(numgioset<=7 && numgioset>=0)gioval=HIGH;
       else 
       {
         Serial.print("Giorno inserito non valido, reinserire:");      
         lcd.setCursor(0,3);
         lcd.print("Giorno non valido");
         delay(1000);
       }
      }
      Serial.print("Giorno inserito valido: ");
      Serial.print(numgioset);
      Serial.println();
      lcd.setCursor(0,3);
      lcd.print("Giorno valido");
      
      rtc_clock.set_time(ora,minuti,00);
      rtc_clock.set_date(gio,mes,ann);
      visoraedata();      
      oraimp=HIGH;
      break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
    case 8:
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Visualizzazione");
    lcd.setCursor(0,1);
    lcd.print("tempo");
    lcd.setCursor(0,2);
    lcd.print("delle zone:");
    delay(2000);
    for(int i=0,app; i<4; i++)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Zona ");
      lcd.print(i+1);
      lcd.setCursor(0,1);
      lcd.print("Tempo:");
      lcd.setCursor(0,1);
      app=durzon[i];
      Serial.print(app);
      lcd.print(app);
      lcd.print(" minuti");
      lcd.setCursor(0,2);
      if(i!=3)
      lcd.print("Per la successiva");
      else
      lcd.print("Per uscire");
      lcd.setCursor(0,3);
      lcd.print("premere un tasto");
      taspreint=-1;
      while(taspreint==-1)
      {
        usb.Task();
      }
      //delay(1000);
    }
    break;
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
/*    case 9:
    ora=rtc_clock.get_hours();
    minuti=rtc_clock.get_minutes();
    gio=rtc_clock.get_days();
    mes=rtc_clock.get_months();
    ann=rtc_clock.get_years();
    Serial.println();
    Serial.print("Sono le ");
    Serial.print(ora);
    Serial.print(":");
    Serial.print(minuti);
    Serial.println();
    Serial.print("Del ");
    Serial.print(gio);
    Serial.print("/");
    Serial.print(mes);
    Serial.print("/");
    Serial.print(ann);
    Serial.println();
    break;    
     }*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
  
}
  
  // Visualizzazione ora e data secondo display
  visoraedata();
  // Controllo se deve partire l'irrigazione
  ora=rtc_clock.get_hours();
  minuti=rtc_clock.get_minutes();
  if(ora==0 && minuti==0 && !numgiosetcam)
  {
    numgioset++;
    numgiosetcam=HIGH;
    segnato=LOW;
    irrpart=LOW;
  }
  if(ora==0 && minuti!=0)numgiosetcam=LOW;

  // A giorni prestabiliti
  if(!tipirr)
  {
    if(gioirr[numgioset])
    {
      if(ora==orainpro && minuti==mininpro && !irrpart)
      {
        inizioirrigazione();
        irrpart=HIGH;
      }
    }
  }
  // A giorni alterni
  if(tipirr)
  {
    if(giosiapp>0)
    {      
      if(ora==orainpro && minuti==mininpro)
      {
        inizioirrigazione();
        giosiapp++;
      }
    }
    else if (gionoapp>0)
    {
      if(ora==orainpro && minuti==mininpro && !segnato)
      {
        gionoapp--;
        segnato=HIGH;
      }
    }
    if(giosiapp==0 && gionoapp==0)
    {
      giosiapp=giosi;
      gionoapp=giono;
    }
  }

  
  
  tastprem=LOW;
  }
      
  
  
  
  
  
  
  
  
  

