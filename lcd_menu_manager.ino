/////////////////////////////////kütüphaneler////////////////////// s
#include <Wire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <stdio.h>
#include <DS1302.h>

///pin tanımlamaları
#define lcd_rs_pin           PB9 //LCD
#define lcd_rw_pin           PB1
#define lcd_e_pin            PB8
#define lcd_d4_pin           PB7
#define lcd_d5_pin           PB6
#define lcd_d6_pin           PB5
#define lcd_d7_pin           PB4

#define yukari_tus           PB3  //TUŞLAR
#define asagi_tus            PA15
#define enter_tus            PA10

#define role_yon             PA3  //ROLELER
#define role_enable          PA5
#define role3                PA4 //kazan
#define role4                PA2 //pompa

#define ntc_enable_pin       PB1  //NTC LER
#define vana_sensor_pin      PB0
#define kazan_sensor_pin     PA7
#define dis_hava_sensor_pin  PA6 
#define boyler_sensor_pin    PA7

#define ledPin               PC13
#define ONE_WIRE_BUS         PB13
#define lcd_backlight        PC13
//#define pot                A6

//#define exit_tus      10


LiquidCrystal lcd(lcd_rs_pin, lcd_e_pin, lcd_d4_pin, lcd_d5_pin, lcd_d6_pin, lcd_d7_pin);


#define yukari      digitalRead(yukari_tus)
#define asagi       digitalRead(asagi_tus)
#define enter       digitalRead(enter_tus)
//#define exitt     digitalRead(exit_tus)

#define kazan_on    digitalWrite(role3, LOW);
#define kazan_off   digitalWrite(role3, HIGH);
#define boyler_on   digitalWrite(role4, LOW);  vana_calisma_izin_flag=1;
#define boyler_off  digitalWrite(role4, HIGH); vana_calisma_izin_flag=0; //vana_mod_byte=manuel;


#define derece      "\337C"
//int yukari = 0;
//int asagi = 0;
//int enter = 0;
int exitt = 0;


int localKey = 0;       // buton için
String keyString = "";  // buton_için
int buton_bekle = 0;


#define lamba_yan  lamba_time =0; digitalWrite(lcd_backlight, HIGH);lamba_durum=1; //backlight lambasını yakar
#define lamba_son  digitalWrite(lcd_backlight, LOW); lamba_durum=0;//backlight lambasını söndürürü

#define acik 0
#define otomatik_isi_takip 1
#define kapali 2
#define manuel 3
#define otomatik_isi_egri 4
#define hesapla (isi_egrisi*((-0.8412*dis_hava_sicakligi)+21.133))+((0.0031*dis_hava_sicakligi)+21.171)



////////////////////////////global değişkenler////////////////////
float tempC = 0;
float dis_hava = 0;
float kazan_sicaklik = 0;
float boyler_sicaklik = 0;
float kazan_sicaklik_set = 60;
float boyler_sicaklik_set = 0;

float istenen_deger = 0;



////dış hava hesaplaması
float isi_egrisi = 1.6;
float dis_hava_sicakligi = -10;
float hesaplanan_gidis_suyu_sicakligi = hesapla;

//exel eğri formülü
//j8 = dış hava sıcaklığı
//k8 = eğri değeri
//(K8*((-0,8412*J8)+21,133))+((0,0031*J8)+21,171)




#define pazartesi 0
#define sali      1
#define carsamba  2
#define persembe  3
#define cuma      4
#define cumartesi 5
#define pazar     6

int hangi_gun =    0;  //0 PAZARTESİ 1 SALI 2 ÇARŞAMBA ...
int hangi_program = 0; // 1. program 2. program.. her gün için üç adet saat programı var onu tutuyro


typedef struct
{
  int acilma_saati[3]     =  {  0, 0, 0};
  int acilma_dakikasi[3]  =  {  0, 0, 0};
  int kapanma_saati[3]    =  {  0, 0, 0};
  int kapanma_dakikasi[3] =  {  0, 0, 0};
  int kazan_sicakligi     =  90;
  int egri_egri_secimi    =  0;
  int zaman_programi_aktifmi [3] =   {  0, 0, 0};

} zaman_programi;

zaman_programi zaman_programlari[8];

int  kazan_calisma_zaman_flag = 0;
int  kazan_mod_flag = 0; // mod 1 otomatik_isi_takip, 0 ise manuel olur manuelde kazan zamana gore calismaz
int  vana_mod_byte = 0; // 0 ise açık, 1 otomatik_isi_takip ,2 kapali

int teknisyen_mod_fag = 0;
int vana_calisma_izin_flag = 0;

unsigned long saniye = 0;
unsigned long saniye_set = 120; //motorun tam kapatma suresi
int           saniye_kur = 0;

unsigned long isitma_zamanlayici =       0;       // vana yönetmek için değişkenler
unsigned long isitma_zamanlayici_set =   0;
int           isitma_zamanlayici_kur =   0;

unsigned long istenen_konum =           90;
unsigned long durma_zamanlayici =        0;
unsigned long durma_zamanlayici_set =    0;
int           durma_zamanlayici_kur =    0;
int           vana_durum =               0;
int           guncel_konum =            90;
int           konum_hesapla_flag = 1;


unsigned long show_menu_no =  100;     //menü yönetmek için değişkenler
int           show_menu_carpan = 100;
char          menu_durum = '>';

int buton_gorev = 0; //buton yönetmek için değişkenler
int arttir = 0;
int azalt = 0;

/////////////////////////////////////////////////////////////////////////////////////////777saat

int lamba_time = 0; //bacligt için zaman sayacı
int lamba_durum = 1;


unsigned long previousMillis = 0;  //saniyede bir yapılacak işlemleri için değişkenler
const long interval = 1000;


//PID

float kd = 0;   //pide kontrol için pid çarpanları
float kp = 2;
float ki = 0;
int oturma_bekle = 10;
int oturma_bekle_timer = 0;
int histersiz = 3;

//DS1302 rtc(kCePin, kIoPin, kSclkPin);

void lcd_yenile();
void saniye_gorevleri();
void dur();
void print_Time() ;
void printTemperature();
void zaman_program_goster();
void buton_fonksiyon();



void lcd_menu_manager_setup(){


  lcd.begin(16, 2);
  pinMode(lcd_backlight, OUTPUT);//lcd lamba yak
  digitalWrite(lcd_backlight, HIGH); //lcd lamba yak
  pinMode(lcd_rw_pin, OUTPUT);//lcd lamba yak
  digitalWrite(lcd_rw_pin,LOW); //lcd lamba yak
  // for (int i = 0 ; i < 200 ; i++) {   EEPROM.write(i, 255);} //eeprom temizle


  // if (!sensors.getAddress(boyler, 3)) Serial.println("Unable to find address for Device 3");

  Temperature();//ölçüm al vana
  dis_hava_olcum_al();//ölçüm al dış hava
  kazan_olcum_al();//ölçüm al kazan
  // boyler_olcum_al();//ölçüm al boyler



  /////////////////////////////////////pin modları atyarlamaları////////////////
  pinMode (ntc_enable_pin, OUTPUT);
  pinMode (role3, OUTPUT);
  pinMode (role4, OUTPUT);
  pinMode (role_yon, OUTPUT);
  pinMode (role_enable, OUTPUT);
  pinMode (ledPin, OUTPUT);
  pinMode (13, OUTPUT);

    pinMode (yukari_tus, INPUT);
    pinMode (asagi_tus,  INPUT) ;
    pinMode (enter_tus,  INPUT) ;
    pinMode (yukari_tus, INPUT);
  // pinMode (exit_tus,   INPUT)  ;


  pinMode(vana_sensor_pin,     INPUT_ANALOG);
  pinMode(kazan_sensor_pin,    INPUT_ANALOG);
  pinMode(dis_hava_sensor_pin, INPUT_ANALOG);
  /////////////////////////////////////ilk enerjilendiğinde yapılacaklar //////////////////////
  digitalWrite(lcd_backlight, HIGH);//lamba yak
  Temperature();
  dis_hava_olcum_al();
  kazan_olcum_al();//ölçüm al kazan
  boyler_olcum_al();//ölçüm al boyler
  Serial.println("resetlendi1----------------------------------------");
  delay(100);

   lcd.noBlink();
  lcd.setCursor ( 0, 0 );        // go to the next line
  lcd.clear();

  //eepromdan degerleri al
  //zaman_programi_ayar_kaydet();
 // eeprom_pid_kaydet();
  if (EEPROM.read(1) == 255 )
  {
    zaman_programi_ayar_kaydet();
  }
  kazan_mod_oku();
  zaman_programi_ayar_oku();//eepromdan zaman ayarlarını rem alır.
  istenen_oku();
  vana_mod_oku();
  eeprom_pid_oku();
  egri_oku();
  boyler_sicaklik_set_oku();
  kazan_sicaklik_set_oku();
}
void lcd_menu_manager_loop(){
/*
while(1){ 
 kazan_off;
 kazan_off;
 dur();

boyler_on;
   lcd.setCursor ( 0, 0 );
lcd.print("POMPA CALISTI");
 while(yukari){} 

delay(3000);
boyler_off;
   lcd.setCursor ( 0, 0 );
lcd.print("POMPA DURDU     ");
delay(3000);
 while(yukari){} 
isit();
   lcd.setCursor ( 0, 0 );
lcd.print("VANA ISITMA YONU");
delay(3000);
 while(yukari){} 
sogut();
   lcd.setCursor ( 0, 0 );
lcd.print("VANA SOGUT YONU");
delay(3000);
 while(yukari){} 
dur();
lcd.setCursor ( 0, 0 );
lcd.print("VANA DUR        ");
delay(3000);
 while(yukari){} 


 
kazan_on;
   lcd.setCursor ( 0, 0 );
lcd.print("KAZAN CALISTI   ");
delay(3000);
 while(yukari){} 
kazan_off;
   lcd.setCursor ( 0, 0 );
lcd.print("KAZAN DURDU     ");
delay(3000);
 while(yukari){} 

}
*/
 /*
while(1){
dis_hava_olcum_al();
kazan_olcum_al();
Temperature(); 
      lcd.setCursor ( 0, 0 );
      lcd.print("D.HA KAZAN KARIS");
      lcd.setCursor ( 0, 1 );
      lcd.print(dis_hava);
      lcd.print(" ");
      lcd.print(kazan_sicaklik);
      lcd.print(" ");
      lcd.print(tempC); 
 
   delay(1000);
   Serial.println("");
   Serial.println("");
}

    */
    int buton_sayac = 0;

    dis_hava_sicakligi = dis_hava;

    if ((!yukari) && (!asagi)) {
      teknisyen_mod_fag = 1;
      //Serial.print("teknisyen");
    }

    if (!yukari) {
      if (lamba_durum == 1) {
          
        if (buton_gorev == 0) show_menu_no += show_menu_carpan;
        if (buton_gorev == 1) arttir = 1;
        lcd_yenile();
        while (!yukari) {
          
          buton_sayac++;
          delay(1);
          if (buton_sayac >= 500) {
            buton_sayac = 0;
            break;
          }
        }
      }
      lamba_yan;
    }




    if (!asagi) {
      //Serial.print("asagi");

      if (lamba_durum == 1) {
        if (buton_gorev == 0)show_menu_no -= show_menu_carpan;
        if (buton_gorev == 1) azalt = 1;
        lcd_yenile();
        while (!asagi) {
          buton_sayac++;
          delay(1);
          if (buton_sayac >= 500) {
            buton_sayac = 0;
            break;
          }
        }
      }
      lamba_yan;
    }


    if (!enter) {
      //Serial.print("enter");
 
      if (lamba_durum == 1) {
        buton_gorev = 1;
        lcd.blink();
        menu_durum = ' ';
        show_menu_no++;
        lcd_yenile();
        while (!enter) {
          buton_sayac++;
          delay(1);
          if (buton_sayac >= 500) {
            buton_sayac = 0;
            break;
          }
        }
      }
      lamba_yan;
    
    }


   if ((!enter) && (!asagi)){

      //Serial.print("exit");
         teknisyen_mod_fag = 0;
      if (lamba_durum == 1) {
        show_menu_no = 100; buton_gorev = 0;
        //if(show_menu_no-(show_menu_no%100)==0){buton_gorev=0;}
        lcd.blink();
        menu_durum = '>';
        lcd_yenile();
        while (!enter) {
          buton_sayac++;
          delay(1);
          if (buton_sayac >= 1000) {
            buton_sayac = 0;
            break;
          }
        }
      }
      lamba_yan;
      lcd.begin(16, 2);
    }





    //digitalWrite(13, LOW);
    //delay(70);
    // digitalWrite(13, HIGH);
    // delay(70);


    saniye_gorevleri();
    vana_mod_gorevleri();

    if ((durma_zamanlayici >= durma_zamanlayici_set) && (durma_zamanlayici_kur == 1)) {

      durma_zamanlayici_kur = 0;
      durma_zamanlayici_set = 0;
      guncel_konum = istenen_konum;

      dur();
    }


  }
  

void vana_mod_gorevleri() {

  if (vana_mod_byte == otomatik_isi_takip) {


    if ((tempC > histersiz + istenen_deger) || (tempC < istenen_deger - histersiz))


      if (oturma_bekle_timer == 0) {
        konum_hesapla_flag = 1;  //  sıcaklığın oturmasını bekleyen sure;
        oturma_bekle_timer = oturma_bekle;
      }

    if (konum_hesapla_flag == 1) {
      istenen_konum = pid(istenen_deger, (int)tempC);
      konum_hesapla_flag = 0;
      konum_belirle(istenen_konum);
    }

  }

  if ((vana_mod_byte == otomatik_isi_egri) && (vana_calisma_izin_flag == 1)) {

    if ((tempC > histersiz + hesapla) || (tempC < hesapla - histersiz))


      if (oturma_bekle_timer == 0) {
        konum_hesapla_flag = 1;  //  sıcaklığın oturmasını bekleyen sure;
        oturma_bekle_timer = oturma_bekle;
      }

    if (konum_hesapla_flag == 1) {
      istenen_konum = pid(hesapla, (int)tempC);
      konum_hesapla_flag = 0;
      konum_belirle(istenen_konum);
    }

  }


  if (vana_mod_byte == acik) {
    isit();
  }
  if (vana_mod_byte == kapali) {
    sogut();
  }
  if (vana_mod_byte == manuel) {
    dur();
  }
}


void zaman_programi_kontrol() {
  int gun = 0;                                                               
  int j = 0;
  int i = 0;
  kazan_calisma_zaman_flag = 0;


  for (i = 0; i < 21; i++) { //günleri tara vakti gelen varmı ve aktif mi
    if (((saat * 60) + dakika) >= ((zaman_programlari[gun].acilma_saati[j] * 60) + (zaman_programlari[gun].acilma_dakikasi[j]))
        && ((saat * 60) + dakika) < ((zaman_programlari[gun].kapanma_saati[j] * 60) + (zaman_programlari[gun].kapanma_dakikasi[j])))
    {
      if ((1 == zaman_programlari[gun].zaman_programi_aktifmi[j]) && (h_gun == gun)) {
        Serial.print("{"); Serial.print(i); Serial.print("}"); Serial.print(zaman_programlari[gun].acilma_saati[j] * 60 + zaman_programlari[gun].acilma_dakikasi[j]); Serial.print("<"); Serial.print(saat * 60 + dakika); Serial.print("<"); Serial.print(zaman_programlari[gun].kapanma_saati[j] * 60 + zaman_programlari[gun].kapanma_dakikasi[j]);
        Serial.println("");
        kazan_calisma_zaman_flag = 1;
      }
    }


    //Serial.print("kazan_calisma_zaman_flag = ");  Serial.println(kazan_calisma_zaman_flag);
    /*
      String gunun_numarasi="";
      if(gun==0){gunun_numarasi=" pazartesi ";}
      if(gun==1){gunun_numarasi=" sali      ";}
      if(gun==2){gunun_numarasi=" carsamba  ";}
      if(gun==3){gunun_numarasi=" persembe  ";}
      if(gun==4){gunun_numarasi=" cuma      ";}
      if(gun==5){gunun_numarasi=" cumartesi ";}
      if(gun==6){gunun_numarasi=" pazar     ";}
      Serial.println("");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(zaman_programlari[gun].zaman_programi_aktifmi[j]);Serial.print(" ");Serial.print(gunun_numarasi);Serial.print("");Serial.print(j);Serial.print(" >>> ");
      Serial.print(zaman_programlari[gun].acilma_saati[j]);Serial.print(":");Serial.print(zaman_programlari[gun].acilma_dakikasi[j]);Serial.print(" -- ");
      Serial.print(zaman_programlari[gun].kapanma_saati[j]);Serial.print(":");Serial.print(zaman_programlari[gun].kapanma_dakikasi[j]);
    */

    gun++;
    if (gun == 7) {
      gun = 0;
      j++;
      if (j == 3) {
        j = 0;
      }
    }

  }


  if (((saat * 60) + dakika) >= ((zaman_programlari[5].acilma_saati[2] * 60) + (zaman_programlari[5].acilma_dakikasi[2])) //tüm günler 1
      && ((saat * 60) + dakika) < ((zaman_programlari[5].kapanma_saati[2] * 60) + (zaman_programlari[5].kapanma_dakikasi[2]))) //tüm günler 1
  {
    if (1 == zaman_programlari[5].zaman_programi_aktifmi[2]) {
      kazan_calisma_zaman_flag = 1; //Serial.println("tumgun1 "); //tüm günler1
    }
  }


  if (((saat * 60) + dakika) >= ((zaman_programlari[6].acilma_saati[2] * 60) + (zaman_programlari[6].acilma_dakikasi[2])) //tüm günler 2
      && ((saat * 60) + dakika) < ((zaman_programlari[6].kapanma_saati[2] * 60) + (zaman_programlari[6].kapanma_dakikasi[2]))) //tüm günler 2
  {
    if (1 == zaman_programlari[6].zaman_programi_aktifmi[2]) {
      kazan_calisma_zaman_flag = 1; //Serial.println("tum gun2"); //tüm günler1
    }
  }

}

int haftanin_gunu(int gun, int ay, int yil)
{
  //https://artofmemory.com/blog/how-to-calculate-the-day-of-the-week-4203.html

  int yuz_yil_kodu = 6 ;// hangi yüzyıl 2000 liyıllar
  int yil_son_iki_hane = yil % 100;
  int ay_carpani = 0;
  int haftanin_gunu = 0;
  int ocak_veya_subatmi = 0;

  switch (ay)
  {
    case 12: ay_carpani = 5;
      break;
    case 11: ay_carpani = 3;
      break;
    case 10: ay_carpani = 0;
      break;
    case 9:  ay_carpani = 5;
      break;
    case 8:  ay_carpani = 2;
      break;
    case 7:  ay_carpani = 6;
      break;
    case 6:  ay_carpani = 4;
      break;
    case 5:  ay_carpani = 1;
      break;
    case 4:  ay_carpani = 6;
      break;
    case 3:  ay_carpani = 3;
      break;
    case 2:  ay_carpani = 3;
      ocak_veya_subatmi = -1;
      break;
    case 1:  ay_carpani = 0;
      ocak_veya_subatmi = -1;
      break;
  }

  haftanin_gunu = (((yil_son_iki_hane + (yil_son_iki_hane / 4)) % 7) + ay_carpani + yuz_yil_kodu + gun + ocak_veya_subatmi) % 7;

  haftanin_gunu = haftanin_gunu + 1;
  if (haftanin_gunu > 7)haftanin_gunu = 1;
  return haftanin_gunu;
}
void saat_tarih_ayarla() {
       
            date.year = yil;
            date.month = ay;
            date.day = gun;
            time.hours = saat;
            time.minutes = dakika;
            time.seconds = rtc_saniye;
            
   epochTime = rtc.dateTimeToEpoch(date, time);
   rtc.setTime(epochTime);
   saat_ayarlaniyor_flag=0;
  /*
  time.hour=saat;
  time.minutes=dakika;
  time.seconds=rtc_saniye;
  date.day=gun;
  date.month=ay;
  date.year=yil;
  */
  // Time t(yil, ay, gun, saat, dakika, 0, Time::kSunday);
 // Time t(yil, ay, gun, saat, dakika, 0, haftanin_gunu(gun, ay, yil));

}


void buton_fonksiyonu() {


  switch (show_menu_no) {

    case 101:
      if (arttir) {
        arttir = 0;
        if (kazan_mod_flag == 0) {
          kazan_mod_flag = 1;
        }
        else
        {
          kazan_mod_flag = 0;

        }


      }
      if (azalt) {
        azalt = 0;

        if (kazan_mod_flag == 0) {
          kazan_mod_flag = 1;
        }
        else
        {
          kazan_mod_flag = 0;
        }

      }

      break;
    case 102:
      kazan_mod_yaz();

      break;

    case 201:
      if (arttir) {
        arttir = 0;
        istenen_deger++;
      }
      if (azalt) {
        azalt = 0;
        istenen_deger--;
      }

      break;

    case 301:  // haftanın gunu

      if (arttir) {
        arttir = 0;

        hangi_program++;
        if (hangi_program == 3)hangi_gun++;
        if (hangi_program == 3)hangi_program = 0;

        if (hangi_gun == 7)hangi_gun = 0;
      }
      if (azalt) {
        azalt = 0;

        if (hangi_gun == 0) {
          if (hangi_program == 0) {
            hangi_gun = 7;
          }
        }
        if (hangi_program == 0) {
          hangi_gun--;
          hangi_program = 3;
        }
        hangi_program--;

      }

      break;

    case 302: //haftanın gununun açılma saati
      if (arttir) {
        arttir = 0;
        zaman_programlari[hangi_gun].acilma_saati[hangi_program]++;
        if (zaman_programlari[hangi_gun].acilma_saati[hangi_program] >= 24)zaman_programlari[hangi_gun].acilma_saati[hangi_program] = 0;
      }
      if (azalt) {
        azalt = 0;

        if (zaman_programlari[hangi_gun].acilma_saati[hangi_program] <= 0)zaman_programlari[hangi_gun].acilma_saati[hangi_program] = 24;
        zaman_programlari[hangi_gun].acilma_saati[hangi_program]--;

      }
      break;
    case 303: //haftanın gununun açılma dakikasi
      if (arttir) {
        arttir = 0;
        zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program]++;
        if (zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program] >= 60)zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program] = 0;
      }
      if (azalt) {
        azalt = 0;
        if (zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program] <= 0)zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program] = 60;
        zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program]--;

      }
      break;
    case 304: //haftanın gununun kapanma saati
      if (arttir) {
        arttir = 0;
        zaman_programlari[hangi_gun].kapanma_saati[hangi_program]++;
        if (zaman_programlari[hangi_gun].kapanma_saati[hangi_program] >= 24)zaman_programlari[hangi_gun].kapanma_saati[hangi_program] = 0;
      }
      if (azalt) {
        azalt = 0;
        if (zaman_programlari[hangi_gun].kapanma_saati[hangi_program] <= 0)zaman_programlari[hangi_gun].kapanma_saati[hangi_program] = 24;
        zaman_programlari[hangi_gun].kapanma_saati[hangi_program]--;
      }

      break;
    case 305: //haftanın gununun kapanma dakikasi
      if (arttir) {
        arttir = 0;
        zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program]++;
        if (zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program] >= 60)zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program] = 0;
      }
      if (azalt) {
        azalt = 0;
        if (zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program] <= 0)zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program] = 60;
        zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program]--;
      }
      break;

    case 306: //haftanın gununun kapanma dakikasi
      if (arttir) {
        arttir = 0;

        if (zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] == 1) {
          zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] = 0;
        }
        else
        {
          zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] = 1;
        }


      }
      if (azalt) {
        azalt = 0;
        if (zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] == 1) {
          zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] = 0;
        } else {
          zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] = 1;
        }

      }
      break;


    case 401:
      if (arttir) {
        arttir = 0;
        dakika++;
        if (dakika >= 60)dakika = 0;
      }
      if (azalt) {
        azalt = 0;
        if (dakika <= 0)dakika = 60;
        dakika--;
      }
      break;

    case 402:
      if (arttir) {
        arttir = 0;
        saat++;
        if (saat == 24)saat = 0;
      }
      if (azalt) {
        azalt = 0;
        if (saat == 0)saat = 1;
        saat--;
      }
      break;
    case 403:
      if (arttir) {
        arttir = 0;
        gun++;
        if (gun == 32)gun = 1;
      }
      if (azalt) {
        azalt = 0;
        gun--;
        if (gun == 0)gun = 31;
      }
      break;
    case 404:
      if (arttir) {
        arttir = 0;
        ay++;
        if (ay == 12)ay = 1;
      }
      if (azalt) {
        azalt = 0;
        ay--;
        if (ay == 0)ay = 12;
      }
      break;
    case 405:
      if (arttir) {
        arttir = 0;
        yil++;
        if (yil == 2030)yil = 2000;
      }
      if (azalt) {
        azalt = 0;
        if (yil == 2000)yil = 2030;
        yil--;
      }
      break;
    case 501:
      if (arttir) {
        arttir = 0;
        vana_mod_byte++;
        if (vana_mod_byte >= 5)vana_mod_byte = 0;
      }
      if (azalt) {
        azalt = 0;
        if (vana_mod_byte <= 0)vana_mod_byte = 5;
        vana_mod_byte--;
      }

      break;

    case 601:

      if (arttir) {
        arttir = 0;
        kp++;
        if (kp >= 20)kp = 0;
      }
      if (azalt) {
        azalt = 0;
        if (kp <= 0)kp = 20;
        kp--;
      }

      break;

    case 602:
      if (arttir) {
        arttir = 0;
        ki++;
        if (ki >= 20)ki = 0;
      }
      if (azalt) {
        azalt = 0;
        if (ki <= 0)ki = 20;
        ki--;
      }

      break;

    case 603:
      if (arttir) {
        arttir = 0;
        kd++;
        if (kd >= 20)kd = 0;
      }
      if (azalt) {
        azalt = 0;
        if (kd <= 0)kd = 20;
        kd--;
      }

      break;

    case 604:
      if (arttir) {
        arttir = 0;
        oturma_bekle++;
        if (oturma_bekle >= 200)oturma_bekle = 0;
      }
      if (azalt) {
        azalt = 0;
        if (oturma_bekle <= 0)oturma_bekle = 200;
        oturma_bekle--;
      }

      break;

    case 605:
      if (arttir) {
        arttir = 0;
        histersiz++;
        if (histersiz >= 200)histersiz = 0;
      }
      if (azalt) {
        azalt = 0;
        if (histersiz <= 0)histersiz = 200;
        histersiz--;
      }

      break;
    case 701:
      if (arttir) {
        arttir = 0;
        isi_egrisi = isi_egrisi + 0.1;
        if (isi_egrisi >= 3)isi_egrisi = 0.2;
      }
      if (azalt) {
        azalt = 0;
        if (isi_egrisi <= 0)isi_egrisi = 3;
        isi_egrisi = isi_egrisi - 0.1;
      }

      break;
    case 901:
      if (arttir) {
        arttir = 0;
        kazan_sicaklik_set = kazan_sicaklik_set + 1;
        if (kazan_sicaklik_set >= 90)kazan_sicaklik_set = 1;
      }
      if (azalt) {
        azalt = 0;
        if (kazan_sicaklik_set <= 0)kazan_sicaklik_set = 90;
        kazan_sicaklik_set = kazan_sicaklik_set - 1;
      }
      break;
    case 1001:
      if (arttir) {
        arttir = 0;
        boyler_sicaklik_set = boyler_sicaklik_set + 1;
        if (boyler_sicaklik_set >= 90)boyler_sicaklik_set = 1;
      }
      if (azalt) {
        azalt = 0;
        if (boyler_sicaklik_set <= 0)boyler_sicaklik_set = 90;
        boyler_sicaklik_set = boyler_sicaklik_set - 1;
      }
      break;



  }

}
void lcd_yenile() {
  // Serial.print("BU GUN = ");//SİLLLL
  // Serial.println(gun);//SİLLLL
  buton_fonksiyonu();

  if (show_menu_no >= 1100)show_menu_no = 100;
  if (show_menu_no == 0)show_menu_no = 1000;

if((show_menu_no>400)&&(show_menu_no<407)){saat_ayarlaniyor_flag=1;}
else{
  saat_ayarlaniyor_flag=0;
  }




  switch (show_menu_no) {

    case 100:
      lcd.begin(16, 2);
      lcd.setCursor ( 0, 0 );
      lcd.clear();

      if (teknisyen_mod_fag == 1)
      {
        Serial.println(hesapla);

        if (vana_mod_byte == otomatik_isi_egri){
          lcd.print(hesapla);

        } else {
          lcd.print (istenen_deger);
        }
      } else {
        lcd.print(kazan_sicaklik);
        lcd.print(derece);  //derece işareti
        lcd.setCursor ( 10, 0 );
      }
      if (kazan_mod_flag == 0) {
        lcd.print ("MANUEL");
      }
      if (kazan_mod_flag == 1) {
        lcd.print ("OTOMAT");
      }
      print_Time();
      lcd.setCursor ( 16, 0 );
      break;

    case 101:
      lcd.setCursor ( 10, 0 );

      if (kazan_mod_flag == 0) {
        lcd.print ("MANUEL");
      }
      if (kazan_mod_flag == 1) {
        lcd.print ("OTOMAT");
      }
      lcd.setCursor ( 15, 0 );
      break;
    case 102:
      lcd.setCursor ( 0, 0 );
      lcd.clear();
      lcd.print ("KAYDEDILDI");
      show_menu_no = 100;
      buton_gorev = 0;
      break;

    case 200:
      /*0
        lcd.setCursor ( 0, 0 );
        lcd.print("          ");
        lcd.setCursor ( 0, 0 );
        lcd.print(guncel_konum);
        lcd.print(" ");
        lcd.print(durma_zamanlayici);
        lcd.print(" ");
        lcd.print(durma_zamanlayici_set);
      */

      lcd.setCursor ( 0, 0 );
      lcd.print("KARISIM SICAKLIK");
      printTemperature();
      break;
    case 201:
      lcd.setCursor ( 0, 1 );
      printTemperature();
      break;
    case 202:
      istenen_kaydet();
      lcd.setCursor ( 0, 1 );
      lcd.print("AYAR KAYDEDILDI");
      delay(1000);
      show_menu_no = 100;
      buton_gorev = 0;
      break;

    case 300:

      lcd.setCursor ( 0, 0 );
      lcd.print("ZAMAN  PROGRAMI");
      lcd.print(menu_durum);
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      hangi_gun = 0;
      break;
    case 301:
      lcd.setCursor ( 0, 0 );
      lcd.print("                ");
      lcd.setCursor ( 0, 0 );
      if (hangi_gun == 0)lcd.print("PAZARTESI");
      if (hangi_gun == 1)lcd.print("SALI");
      if (hangi_gun == 2)lcd.print("CARSAMBA");
      if (hangi_gun == 3)lcd.print("PERSEMBE");
      if (hangi_gun == 4)lcd.print("CUMA");
      if (hangi_gun == 5)lcd.print("CUMARTESI");
      if (hangi_gun == 6)lcd.print("PAZAR");

      if ((hangi_program == 2) && (hangi_gun == 5)) {
        lcd.setCursor ( 0, 0 );
        lcd.print("TUM GUNLER    1");
      }
      if ((hangi_program == 2) && (hangi_gun == 6)) {
        lcd.setCursor ( 0, 0 );
        lcd.print("TUM GUNLER    2");
      }

      if (hangi_program == 0) {
        lcd.print(" 1.");
      }
      if (hangi_program == 1)lcd.print(" 2.");
      if (hangi_program == 2)lcd.print(" 3.");
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      lcd.setCursor ( 0, 0 );
      break;
    case 302:
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      lcd.setCursor ( 1, 1 );
      break;
    case 303:
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      lcd.setCursor ( 4, 1 );
      break;
    case 304:
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      lcd.setCursor ( 12, 1 );
      break;
    case 305:
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      lcd.setCursor ( 15, 1 );
      break;
    case 306:
      lcd.setCursor ( 0, 1 );
      zaman_program_goster();
      lcd.setCursor ( 8, 1 );
      break;
    case 307:
      lcd.setCursor ( 0, 1 );
      zaman_programi_ayar_kaydet();
      lcd.print("AYARLAR KAYDEDILDI");
      delay(500);
      show_menu_no = 100;
      buton_gorev = 0;
    case 400:
      //lcd.clear();
      lcd.setCursor ( 0, 0 );
      lcd.print("TARIH SAAT AYAR");
      lcd.print(menu_durum);
      print_Time();
      lcd.setCursor ( 15, 0 );
      break;
    case 401:
      lcd.setCursor ( 15, 1 );
      char buf[17];
      snprintf(buf, sizeof(buf), "%04d.%02d.%02d %02d:%02d", yil, ay, gun, saat, dakika);
      lcd.setCursor ( 0, 1 );
      lcd.print(buf);
      lcd.setCursor ( 15, 1 );
      break;
    case 402:
      lcd.setCursor ( 12, 1 );
      snprintf(buf, sizeof(buf), "%04d.%02d.%02d %02d:%02d", yil, ay, gun, saat, dakika);
      lcd.setCursor ( 0, 1 );
      lcd.print(buf);
      lcd.setCursor ( 12, 1 );
      break;
    case 403:
      lcd.setCursor ( 9, 1 );
      snprintf(buf, sizeof(buf), "%04d.%02d.%02d %02d:%02d", yil, ay, gun, saat, dakika);
      lcd.setCursor ( 0, 1 );
      lcd.print(buf);
      lcd.setCursor ( 9, 1 );
      break;
    case 404:
      lcd.setCursor ( 6, 1 );
      snprintf(buf, sizeof(buf), "%04d.%02d.%02d %02d:%02d", yil, ay, gun, saat, dakika);
      lcd.setCursor ( 0, 1 );
      lcd.print(buf);
      lcd.setCursor ( 6, 1 );
      break;
    case 405:
      lcd.setCursor ( 3, 1 );
      snprintf(buf, sizeof(buf), "%04d.%02d.%02d %02d:%02d", yil, ay, gun, saat, dakika);
      lcd.setCursor ( 0, 1 );
      lcd.print(buf);
      lcd.setCursor ( 3, 1 );
      break;
    case 406:
      lcd.setCursor ( 0, 1 );
      saat_tarih_ayarla();

      lcd.print("AYAR KAYDEDILDI");
      delay(1000);
      show_menu_no = 100;
      buton_gorev = 0;
    case 500:
      if (!teknisyen_mod_fag)
      {
        show_menu_no = 600;
        break;
      }
      lcd.setCursor ( 0, 0 );
      lcd.print("VANA MOD        ");
      lcd.setCursor ( 0, 1 );
      lcd.print("                ");
      lcd.setCursor ( 0, 1 );
      if (vana_mod_byte == 0)
        lcd.print("ACIK     ");
      if (vana_mod_byte == 1)
        lcd.print("OTOMATIK ISI ");
      if (vana_mod_byte == 2)
        lcd.print("KAPALI   ");
      if (vana_mod_byte == 3)
        lcd.print("MANUEL   ");
      if (vana_mod_byte == 4)
        lcd.print("OTOMATIK EGRI ");
      lcd.setCursor ( 0, 16 );
      buton_gorev = 0;

      break;
    case 501:
      lcd.print("                ");
      lcd.setCursor ( 0, 1 );
      if (vana_mod_byte == 0)
        lcd.print("ACIK     ");
      if (vana_mod_byte == 1)
        lcd.print("OTOMATIK ISI ");
      if (vana_mod_byte == 2)
        lcd.print("KAPALI   ");
      if (vana_mod_byte == 3)
        lcd.print("MANUEL   ");
      if (vana_mod_byte == 4)
        lcd.print("OTOMATIK EGRI ");
      break;
    case 502:
      vana_mod_kaydet();

      lcd.print("AYARLAR KAYDEDILDI");
      show_menu_no = 100;
      buton_gorev = 0;
      break;

    case 600:
      if (!teknisyen_mod_fag)
      {
        show_menu_no = 700;
        break;
      }
      lcd.noBlink();
      lcd.setCursor ( 0, 0 );
      lcd.print("P I D T H      ");
      lcd.setCursor ( 0, 1 );

      lcd.print((int)kp);
      lcd.print(" ");
      lcd.print((int)ki);
      lcd.print(" ");
      lcd.print((int)kd);
      lcd.print(" ");
      lcd.print(oturma_bekle);
      lcd.print(" ");
      lcd.print(histersiz);
      lcd.print("     ");
      // delay(1000);
      // show_menu_no=100;
      buton_gorev = 0;
      break;
    case 601:
      lcd.blink();
      lcd.setCursor ( 0, 0 );
      lcd.print("P I D T H       ");
      lcd.setCursor ( 0, 1 );

      lcd.print((int)kp);
      lcd.print(" ");
      lcd.print((int)ki);
      lcd.print(" ");
      lcd.print((int)kd);
      lcd.print(" ");
      lcd.print(oturma_bekle);
      lcd.print(" ");
      lcd.print(histersiz);
      lcd.print(" ");
      lcd.blink();
      lcd.setCursor ( 0, 1 );
      break;

    case 602:
      lcd.blink();
      lcd.setCursor ( 0, 0 );
      lcd.print("P I D T H      ");
      lcd.setCursor ( 0, 1 );

      lcd.print((int)kp);
      lcd.print(" ");
      lcd.print((int)ki);
      lcd.print(" ");
      lcd.print((int)kd);
      lcd.print(" ");
      lcd.print(oturma_bekle);
      lcd.print(" ");
      lcd.print(histersiz);
      lcd.print(" ");
      lcd.blink();
      lcd.setCursor ( 2, 1 );
      break;

    case 603:
      lcd.blink();
      lcd.setCursor ( 0, 0 );
      lcd.print("P I D T H      ");
      lcd.setCursor ( 0, 1 );

      lcd.print((int)kp);
      lcd.print(" ");
      lcd.print((int)ki);
      lcd.print(" ");
      lcd.print((int)kd);
      lcd.print(" ");
      lcd.print(oturma_bekle);
      lcd.print(" ");
      lcd.print(histersiz);
      lcd.print(" ");
      lcd.blink();
      lcd.setCursor ( 4, 1 );
      break;

    case 604:
      lcd.setCursor ( 0, 0 );
      lcd.print("P I D T H       ");
      lcd.setCursor ( 0, 1 );

      lcd.print((int)kp);
      lcd.print(" ");
      lcd.print((int)ki);
      lcd.print(" ");
      lcd.print((int)kd);
      lcd.print(" ");
      lcd.print(oturma_bekle);
      lcd.print(" ");
      lcd.print(histersiz);
      lcd.print(" ");
      lcd.blink();
      lcd.setCursor ( 6, 1 );
      break;
    case 605:
      lcd.setCursor ( 0, 0 );
      lcd.print("P I D T H       ");
      lcd.setCursor ( 0, 1 );

      lcd.print((int)kp);
      lcd.print(" ");
      lcd.print((int)ki);
      lcd.print(" ");
      lcd.print((int)kd);
      lcd.print(" ");
      lcd.print(oturma_bekle);
      lcd.print(" ");
      lcd.print(histersiz);
      lcd.print(" ");
      lcd.blink();
      lcd.setCursor ( 8, 1 );
      break;
    case 606:
      lcd.print("AYARLAR KAYDEDILDI");
      delay(500);
      eeprom_pid_kaydet();
      show_menu_no = 100;
      buton_gorev = 0;
      break;
    case 700:
      lcd.setCursor ( 0, 0 );
      lcd.print("KONFOR SICAKLIGI");
      lcd.setCursor ( 0, 1 );
      lcd.print(isi_egrisi * 14);
      lcd.print(derece);
      lcd.print("         ");
      //lcd.setCursor ( 0, 1 );
      //lcd.print(isi_egrisi);
      break;
    case 701:
      lcd.setCursor ( 0, 0 );
      lcd.print("ISI EGR. ");
      lcd.print(isi_egrisi * 14);
      lcd.print(derece);
      lcd.setCursor ( 0, 1 );
      lcd.print("                ");
      lcd.setCursor ( 0, 1 );
      lcd.print(isi_egrisi);
      break;
    case 702:
      egri_kaydet();
      show_menu_no = 100;
      buton_gorev = 0;
      break;
    case 800:
      lcd.setCursor ( 0, 0 );
      lcd.print("DIS HAVA SICAK. ");
      lcd.setCursor ( 0, 1 );
      lcd.print("                ");
      lcd.setCursor ( 0, 1 );

      lcd.print(dis_hava);
      lcd.print(derece);

      break;

    case 900:
      lcd.setCursor ( 0, 0 );
      lcd.print("KAZAN SICAKLIK. ");
      lcd.setCursor ( 0, 1 );
      lcd.print("                ");
      lcd.setCursor ( 0, 1 );

      lcd.print(kazan_sicaklik);
      lcd.print(derece);
      lcd.print("  ");
      lcd.print(kazan_sicaklik_set);
      lcd.print(derece);
      break;
    case 901:
      lcd.setCursor ( 0, 1 );
      lcd.print("                ");
      lcd.setCursor ( 0, 1 );

      lcd.print(kazan_sicaklik);
      lcd.print(derece);
      lcd.print("  ");
      lcd.print(kazan_sicaklik_set);
      lcd.print(derece);
      lcd.setCursor ( 15, 1 );


      break;
    case 902:
      kazan_sicaklik_set_kaydet();
      lcd.setCursor ( 0, 1 );
      lcd.print("AYAR KAYDEDILDI");
      delay(1000);
      show_menu_no = 100;
      buton_gorev = 0;
      break;
    case 1000:
      show_menu_no = 100;
      buton_gorev = 0;
      //lcd.setCursor ( 0, 0 );
      //lcd.print("BOYLER SICAKLIK. ");
      //lcd.setCursor ( 0, 1 );
      //lcd.print("                ");
      //lcd.setCursor ( 0, 1 );

      //lcd.print(boyler_sicaklik);
      //lcd.print(derece);
      //lcd.print("  ");
      //lcd.print(boyler_sicaklik_set);
      //lcd.print(derece);

      break;
    case 1001:
      //lcd.setCursor ( 0, 1 );
      //lcd.print("                ");
      //lcd.setCursor ( 0, 1 );
      //lcd.print(boyler_sicaklik);
      //lcd.print(derece);
      //lcd.print("  ");
      //lcd.print(boyler_sicaklik_set);
      //lcd.print(derece);
      //lcd.setCursor ( 15, 1 );

      break;
    case 1002:
      //boyler_sicaklik_set_kaydet();
      //lcd.setCursor ( 0, 1 );
      //lcd.print("AYAR KAYDEDILDI");
      // delay(1000);
      show_menu_no = 100;
      buton_gorev = 0;
      break;



  }

}

void zaman_programi_ayar_kaydet() {


  int gun = 0;
  int j = 0;
  int i = 0;

  for (i = 0; i < 120; i += 5) {
    EEPROM.write(i, zaman_programlari[gun].acilma_saati[j]);
    EEPROM.write(i + 1, zaman_programlari[gun].acilma_dakikasi[j]);
    EEPROM.write(i + 2, zaman_programlari[gun].kapanma_saati[j]);
    EEPROM.write(i + 3, zaman_programlari[gun].kapanma_dakikasi[j]);
    EEPROM.write(i + 4, zaman_programlari[gun].zaman_programi_aktifmi[j]);

    gun++;
    if (gun == 8) {
      gun = 0;
      j++;
      if (j == 3) {
        j = 0;
      }
    }

    /*
      for(i=0;i<105;i++){
      Serial.print(i);
      Serial.print("eeprom = ");
      Serial.println(EEPROM.read(i));
      delay(1);
      }
    */
  }
}
void zaman_programi_ayar_oku() {

  int gun = 0;
  int j = 0;
  int i = 0;
  int k = 0;


  for (i = 0; i < 120; i += 5) {

    /*
       delay(1);
       Serial.print("gun = ");
       Serial.print (gun);
       Serial.print(" ");
       Serial.print("pro = ");
       Serial.print(j);
         Serial.print(" ");
       Serial.print("ac saat adr = ");
       Serial.println(i);

          Serial.print("gun = ");
       Serial.print (gun);
       Serial.print(" ");
       Serial.print("pro = ");
       Serial.print(j);
         Serial.print(" ");
       Serial.print("ac dk adr = ");
       Serial.println(i+1);


          Serial.print("gun = ");
       Serial.print (gun);
       Serial.print(" ");
       Serial.print("pro = ");
       Serial.print(j);
         Serial.print(" ");
       Serial.print("kapat saat adr = ");
       Serial.println(i+2);

          Serial.print("gun = ");
       Serial.print (gun);
       Serial.print(" ");
       Serial.print("pro = ");
       Serial.print(j);
         Serial.print(" ");
       Serial.print("kapat dk adr = ");
       Serial.println(i+3);



          Serial.print("gun = ");
       Serial.print (gun);
       Serial.print(" ");
       Serial.print("pro = ");
       Serial.print(j);
       Serial.print(" ");
       Serial.print(" enable adr = ");
       Serial.println(i+4);
    */
    zaman_programlari[gun].acilma_saati[j] = EEPROM.read(i);
    zaman_programlari[gun].acilma_dakikasi[j] = EEPROM.read(i + 1);
    zaman_programlari[gun].kapanma_saati[j] = EEPROM.read(i + 2);
    zaman_programlari[gun].kapanma_dakikasi[j] = EEPROM.read(i + 3);
    zaman_programlari[gun].zaman_programi_aktifmi[j] = EEPROM.read(i + 4);

    gun++;
    if (gun == 8) {
      gun = 0;
      j++;
      if (j == 3) {
        j = 0;
      }
    }
  }
}
void kazan_mod_yaz() {
  if (kazan_mod_flag == 0) //manuelde ise
  {
    vana_mod_byte = 3;
    Serial.println("vana mod 3");
  }
  if (kazan_mod_flag == 1) ////otomaikde ise
  {
    Serial.println("vana mod 4");
    vana_mod_byte = 4;
  }
  vana_mod_kaydet();

  EEPROM.write(121, kazan_mod_flag);
}
void kazan_mod_oku() {

  if (EEPROM.read(121) == 255 )
  {
    kazan_mod_flag = 0;
    EEPROM.write(121, kazan_mod_flag);
  }
  else
  {
    kazan_mod_flag = EEPROM.read(121);
  }

}
void kazan_sicaklik_set_kaydet() {
  int sayi = (int)kazan_sicaklik_set;
  EEPROM.write(132, sayi);
  Serial.print(sayi);
}
void boyler_sicaklik_set_kaydet() {
  int sayi = (int)boyler_sicaklik_set;
  EEPROM.write(133, sayi);
  Serial.print(sayi);

}
void kazan_sicaklik_set_oku() {
  int sayi = kazan_sicaklik_set;
  if (EEPROM.read(132) == 255 ) {
    EEPROM.write(132, sayi);
  } else {
    kazan_sicaklik_set = EEPROM.read(132);

    // kazan_sicaklik_set=(sayi/10);
    // for(int i=0;i<=sayi%10;i++)kazan_sicaklik_set=kazan_sicaklik_set+0.1;
  }
}
void boyler_sicaklik_set_oku() {
  int sayi = (int)boyler_sicaklik_set;
  if (EEPROM.read(133) == 255 ) {
    EEPROM.write(133, sayi);
  } else {
    boyler_sicaklik_set = EEPROM.read(133);

    //  boyler_sicaklik_set=(sayi/10);
    //  for(int i=0;i<=sayi%10;i++)boyler_sicaklik_set=boyler_sicaklik_set+0.1;

  }
}
void egri_kaydet() {
  int sayi = isi_egrisi * 10;
  EEPROM.write(131, sayi);
  Serial.print(sayi);

}
void egri_oku() {
  int sayi = isi_egrisi * 10;
  if (EEPROM.read(131) == 255 ) {
    EEPROM.write(131, sayi);
  } else {
    sayi = EEPROM.read(131);

    isi_egrisi = (sayi / 10);
    for (int i = 0; i <= sayi % 10; i++)isi_egrisi = isi_egrisi + 0.1;


  }
}
void istenen_kaydet() {
  EEPROM.write(130, (int)istenen_deger);
}
void istenen_oku() {
  if (EEPROM.read(130) == 255 ) {
    EEPROM.write(130, (int)istenen_deger);
  } else {
    istenen_deger = EEPROM.read(130);
  }

}
void eeprom_pid_kaydet(){
  EEPROM.write(124, (int)kp);
  EEPROM.write(125, (int)ki);
  EEPROM.write(126, (int)kd);
  EEPROM.write(127, oturma_bekle);
  EEPROM.write(128, histersiz);
}
void eeprom_pid_oku() { 
  if (EEPROM.read(124) == 255 ) {
    EEPROM.write(124, (int)kp);
  } else {
    kp = EEPROM.read(124);
  }
  if (EEPROM.read(125) == 255 ) {
    EEPROM.write(125, (int)ki);
  } else {
    ki = EEPROM.read(125);
  }
  if (EEPROM.read(126) == 255 ) {
    EEPROM.write(126, (int)kd);
  } else {
    kd = EEPROM.read(126);
  }
  if (EEPROM.read(127) == 255 ) {
    EEPROM.write(127, (int)oturma_bekle);
  } else {
    oturma_bekle = EEPROM.read(127);
  }
  if (EEPROM.read(128) == 255 ) {
    EEPROM.write(128, (int)histersiz);
  } else {
    histersiz = EEPROM.read(128);
  }

}
void vana_mod_kaydet() {

  EEPROM.write(123, vana_mod_byte);
}
void vana_mod_oku() {

  if (EEPROM.read(123) == 255 )
  {
    vana_mod_byte = 0;
    EEPROM.write(123, vana_mod_byte);
  }
  else
  {
    vana_mod_byte = EEPROM.read(123);
  }
}


void zaman_program_goster() {
  char aktif = 'X';
  if (zaman_programlari[hangi_gun].zaman_programi_aktifmi[hangi_program] == 1) {
    aktif = '-';
  }
  char bufff[17];
  lcd.setCursor ( 0, 1 );
  snprintf(bufff, sizeof(bufff), "%02d:%02d  %c%c  %02d:%02d", zaman_programlari[hangi_gun].acilma_saati[hangi_program], zaman_programlari[hangi_gun].acilma_dakikasi[hangi_program], aktif, aktif, zaman_programlari[hangi_gun].kapanma_saati[hangi_program], zaman_programlari[hangi_gun].kapanma_dakikasi[hangi_program]);
  lcd.print(bufff);
}
int pid(int hedef_dgr, int dgr) {
  float pidTerm;
  int hata;
  static int son_hata;
  static int toplam_hata;
  hata = hedef_dgr - dgr;
  toplam_hata += hata;
  pidTerm = (hata) * kp + (hata - son_hata) * kd + toplam_hata * ki;
  son_hata = hata;

  return (int(pidTerm));
}/*
String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: h_gun = 6; return   "pazar";
    case Time::kMonday: h_gun = 0; return   "pazartesi";
    case Time::kTuesday: h_gun = 1; return  "sali";
    case Time::kWednesday: h_gun = 2; return  "carsamba";
    case Time::kThursday: h_gun = 3; return  "persembe";
    case Time::kFriday: h_gun = 4; return    "cuma";
    case Time::kSaturday: h_gun = 5; return "cumartesi";
  }
  return "(unknown day)";
}*/
void print_Time() {
  char buf[50];  
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d  %02d/%02d  ",saat, dakika, rtc_saniye  , ay, gun);
  Serial.println(buf);
  lcd.setCursor ( 0, 1 );
  lcd.print(buf);
}
void printTemperature() {

  lcd.setCursor ( 0, 1 );       // go to the next line


  if (teknisyen_mod_fag)
  {

    lcd.print (istenen_deger);
    lcd.print(derece);
    lcd.print ("  ");
  }

  lcd.print (tempC);
  lcd.print(derece);
  lcd.print ("         ");

}
void Temperature() {

  //  int val = analogRead(A0);
  //  val = map(val, 427, 869, 20, 90);
  // tempC = val ;
  //sensors.requestTemperatures();
  //tempC = sensors.getTempC(deviceAddress);


  digitalWrite(ntc_enable_pin, HIGH);
  delay(1);
  int deger = analogRead(vana_sensor_pin);
    deger = map(deger, 0, 4095, 0, 1024);

  
  double sicaklik = Termistor(deger);
  tempC = sicaklik;
    Serial.print(" adc ");
  Serial.print(deger);
    Serial.print(" vana ");
  Serial.println(sicaklik);
  digitalWrite(ntc_enable_pin, LOW);

}
void dis_hava_olcum_al() {
  //float kontrol=0;
  //sensors.requestTemperatures();
  //kontrol = sensors.getTempC(deviceAddress);
  //if(!(kontrol==-127)){dis_hava=kontrol;}


  digitalWrite(ntc_enable_pin, HIGH);
  delay(1);
  int deger = analogRead(dis_hava_sensor_pin);
  deger = map(deger, 0, 4095, 0, 1024);
  double sicaklik = Termistor(deger);
  dis_hava = sicaklik;
  digitalWrite(ntc_enable_pin, LOW);

  Serial.print(" adc ");
  Serial.print(deger);
    Serial.print(" dis hava ");
  Serial.println(dis_hava);
}
void kazan_olcum_al() {
  float kontrol = 0;
  //sensors.requestTemperatures();
  //kontrol = sensors.getTempC(deviceAddress);
  //if(!(kontrol==-127)){kazan_sicaklik=kontrol;}

  digitalWrite(ntc_enable_pin, HIGH);
  delay(1);
  int deger = analogRead(kazan_sensor_pin);
    deger = map(deger, 0, 4095, 0, 1024);
  double sicaklik = Termistor(deger);
  kazan_sicaklik = sicaklik;
  digitalWrite(ntc_enable_pin, LOW);

  
  Serial.print(" adc ");
  Serial.print(deger);
    Serial.print(" kazan ");
  Serial.println(kazan_sicaklik);
  /*

  */
}
void boyler_olcum_al() {
  float kontrol = 0;
  //sensors.requestTemperatures();
  //kontrol = sensors.getTempC(deviceAddress);
  //if(!(kontrol==-127)){boyler_sicaklik=kontrol;}

  digitalWrite(ntc_enable_pin, HIGH);
  delay(1);
  int deger = analogRead(boyler_sensor_pin);

  deger = map(deger, 0, 4095, 0, 1024);
  double sicaklik = Termistor(deger);
  boyler_sicaklik = sicaklik;
  digitalWrite(ntc_enable_pin, LOW);


}
void saniye_gorevleri() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    saniye++;
    if (saniye == 15) {

      lamba_time ++;  //backlight zaman sayması için
      if (lamba_time >= 10) {
        lamba_time = 0;  //backlig
        lamba_son;
      }
      if (lamba_time >= 4) {
        show_menu_no = 100;  //45 saniye sonra menüden çık
        buton_gorev = 0;
      }

      Temperature();
      dis_hava_olcum_al();
      kazan_olcum_al();//ölçüm al kazan
      boyler_olcum_al();//ölçüm al boyler
    }
    if (saniye >= 15)saniye = 0;



    if (oturma_bekle_timer > 0) {
      oturma_bekle_timer--; //vana durduktan sonra beklemesi gereken sure
    }

    lcd_yenile();

    zaman_programi_kontrol();


    if (kazan_mod_flag == 1) {    //kazan otomatikte ise
      if (kazan_calisma_zaman_flag == 1) { //ÇALIŞMA ZAMANI ARALIĞINDA İSE (kazan çalışma zamanı gelmişse)
        boyler_on;
        vana_mod_byte = otomatik_isi_egri;

        kazan_calistir();
      } else {
        boyler_off;
        vana_mod_byte = manuel;
        kazan_off;
      }
    } else {            //kazan manuel modda iken
      boyler_on;
      vana_mod_byte = manuel;
      kazan_calistir();
    }
    // if((kazan_calisma_zaman_flag==1)&&(kazan_mod_flag==1)){


    if ((kazan_calisma_zaman_flag == 1) && (kazan_mod_flag == 1)) {


      // if((kazan_sicaklik>=kazan_sicaklik_set-2)&&(kazan_sicaklik<=kazan_sicaklik_set+2)){

    }

    if (vana_durum == 1)guncel_konum++; //motor ısıtma yönünde dönüyorsa
    if (guncel_konum > 90)guncel_konum = 0;
    if (vana_durum == 2)guncel_konum--; //motor soğutma yönünde dönüyorsa
    if (guncel_konum < 1)guncel_konum = 90;

    if (durma_zamanlayici_kur == 1) durma_zamanlayici++;
    if (durma_zamanlayici_kur == 0) durma_zamanlayici = 0;
    if (durma_zamanlayici == 5000)  durma_zamanlayici = 4999;


  }

}
void kazan_calistir() {
  if (kazan_sicaklik <= kazan_sicaklik_set - 5) {
    //Serial.println("kazan calisiyor ");
    kazan_on;
  }

  if (kazan_sicaklik >= kazan_sicaklik_set + 5) {
    Serial.print("kazan =");
    Serial.print(kazan_sicaklik);
    Serial.println ("derece durduruldu");

    kazan_off;
  }
}
void konum_belirle (int konum) {
  if (konum < 0) {
    sogut();
    durma_zamanlayici_set  = (-1 * konum);
    durma_zamanlayici_kur = 1;
  }

  if (konum > 0) {
    isit();
    durma_zamanlayici_set = konum;
    durma_zamanlayici_kur = 1;
  }
}
void konum (int konum) {

  if (konum < guncel_konum) {
    sogut();
    durma_zamanlayici_set  = guncel_konum - konum;
    durma_zamanlayici_kur = 1;
  }

  if (konum > guncel_konum) {
    isit();
    durma_zamanlayici_set = konum - guncel_konum;
    durma_zamanlayici_kur = 1;
  }


}
void isit() {
  digitalWrite(role_enable, 0);
  digitalWrite(role_yon, 0);
  //lcd.setCursor ( 13, 0 );
  //lcd.print("isitiyor");
  vana_durum = 1;
}
void sogut() {
  //  lcd.setCursor ( 13, 0 );
  // lcd.print("sogutuyor");
  digitalWrite(role_enable, 0);
  digitalWrite(role_yon, 1);
  vana_durum = 2;
}
void dur() {
  vana_durum = 0;
  //  lcd.setCursor ( 13, 0 );
  //lcd.print("duruyor");
  digitalWrite(role_enable, 1);
  digitalWrite(role_yon, 1);
}
double Termistor(int analogOkuma) {

  double sicaklik;
  sicaklik = log(((10240000 / analogOkuma) - 10000));
  sicaklik = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * sicaklik * sicaklik)) * sicaklik);
  sicaklik = sicaklik - 273.15;
  //sicaklik=sicaklik * -1;
  return sicaklik;
}
