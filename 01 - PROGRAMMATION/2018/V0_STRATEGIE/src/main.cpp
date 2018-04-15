#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SPI.h>
#include "DFRobotDFPlayerMini.h"

// Points pour chaque action
#define recuperateur 10 // points pour chaque récupérateur au moins vidé d’une balle par l’équipe à qui il appartient
#define chateau 5 //points pour chaque balle de la bonne couleur dans le château d’eau.
#define epuration 10 // points par balle de la couleur adverse dans la station d’épuration
#define deposePanneau 5 //points pour la dépose du panneau devant le château d’eau
#define activePanneau 25 //points pour un panneau alimenté (interrupteur fermé) à la fin du match
#define deposeAbeille 5 //points pour la dépose de l’abeille sur la ruche
#define activeAbeille 50 //points pour une fleur butinée (ballon éclaté)
// Adressage I2C pour les cartes esclaves
#define carteDeplacement 60
#define carteActionneur 80
// Couleur Equipe
#define vert 1
#define orange 0
// Autres
#define tempsMatch 99000
#define SerialPlayer Serial1
// Logo Karibous
#define LOGO_KARIBOUS_width 128
#define LOGO_KARIBOUS_height 33
static unsigned char LOGO_KARIBOUS_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xfc, 0xf9, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x1c, 0xfc, 0xfb, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x1c, 0xfc, 0xfb, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x04, 0x0b, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x1c, 0xfc, 0xf9, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x1c, 0x0c, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xfc, 0x39, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfc, 0xf9, 0xf9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xfc, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0xc0, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xe0, 0xc3, 0x1f,
   0xe0, 0xff, 0xc7, 0xf3, 0xff, 0x81, 0xff, 0x87, 0x0f, 0x38, 0xfc, 0x3f,
   0xfc, 0xf0, 0xc1, 0xff, 0xe1, 0xff, 0xcf, 0xf3, 0xff, 0xc3, 0xff, 0x8f,
   0x0f, 0x38, 0xfe, 0x3f, 0xfc, 0xf8, 0xc0, 0xff, 0xe3, 0xff, 0xdf, 0xf3,
   0xff, 0xe7, 0xff, 0x9f, 0x0f, 0x38, 0xfe, 0x3f, 0xfc, 0xf8, 0xc0, 0xff,
   0xe7, 0xff, 0xdf, 0xf3, 0xff, 0xe7, 0xff, 0x9f, 0x0f, 0x38, 0xff, 0x1f,
   0xfc, 0x7c, 0xc0, 0xff, 0xe7, 0xff, 0xdf, 0xf3, 0xff, 0xe7, 0xff, 0x9f,
   0x0f, 0x38, 0xff, 0x1f, 0xfc, 0x3e, 0xc0, 0xff, 0xe7, 0xff, 0xdf, 0xf3,
   0xff, 0xe7, 0xff, 0x9f, 0x0f, 0x38, 0xff, 0x1f, 0xfc, 0x1f, 0x00, 0xf0,
   0x67, 0x00, 0xde, 0xf3, 0x00, 0xe7, 0x01, 0x9e, 0x0f, 0x38, 0x0f, 0x00,
   0xfc, 0x0f, 0x00, 0x00, 0x67, 0x00, 0xde, 0xf3, 0x00, 0xe3, 0x01, 0x9e,
   0x0f, 0x38, 0x0f, 0x00, 0xfc, 0x0f, 0xe0, 0xff, 0xe7, 0xff, 0xdf, 0xf3,
   0xff, 0xe1, 0x01, 0x9e, 0x0f, 0x38, 0xfe, 0x1f, 0xfc, 0x1f, 0xf0, 0xff,
   0xe7, 0xff, 0xcf, 0xf3, 0xff, 0xe3, 0x01, 0x9e, 0x0f, 0x3c, 0xfc, 0x3f,
   0xfc, 0x1f, 0x70, 0xc0, 0xe7, 0xff, 0xc7, 0x71, 0x80, 0xe7, 0x01, 0x8e,
   0x0f, 0x3c, 0x00, 0x3f, 0xfc, 0x3f, 0x70, 0x00, 0xe7, 0xf9, 0xc1, 0x71,
   0x80, 0xe7, 0x01, 0x8e, 0x0f, 0x1c, 0x00, 0x38, 0x7c, 0x7e, 0xf0, 0x01,
   0xe7, 0xf1, 0xc0, 0xf1, 0xff, 0xe7, 0x01, 0x8e, 0xff, 0x1f, 0x1e, 0x38,
   0x3c, 0x7c, 0xf0, 0xff, 0xe7, 0xf1, 0xc0, 0xf1, 0xff, 0xe7, 0xff, 0x0f,
   0xff, 0x1f, 0xfe, 0x3f, 0x3c, 0xfc, 0xf0, 0xff, 0xe7, 0xe1, 0xc1, 0xf1,
   0xff, 0xc3, 0xff, 0x0f, 0xff, 0x1f, 0xfe, 0x3f, 0x3c, 0xf8, 0xe1, 0xff,
   0xe7, 0xe1, 0xc3, 0xf1, 0xff, 0x81, 0xff, 0x07, 0xfe, 0x0f, 0xfe, 0x1f,
   0x38, 0xf0, 0x03, 0x7f, 0xe7, 0xc1, 0xc7, 0xf1, 0xff, 0x00, 0xf8, 0x03,
   0xf8, 0x07, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


bool equipe = vert;
byte optionNavigation = 0;
int score = 0;
double timeInit=0;
bool statutMp3 = false;
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0,13,11,12,U8X8_PIN_NONE);
DFRobotDFPlayerMini myDFPlayer;

void sendNavigation(byte fonction, int X, int Y, int rot);
void sendNavigation(byte fonction, int rot, int dist);
void majScore(int points, int multiplicateur);
void turnGo(bool recalage,bool ralentit,int turn, int go);
void attente(int temps);
void testDeplacement();
void Homologation();
void recalageInit();
void finMatch();
bool askNavigation();
int majTemps();
// Gestion LCD
void u8g2_prepare();
void u8g2_splash_screen();
void u8g2_menu_pendant_match();
void u8g2_menu_avant_match();
void u8g2_splash_screen_GO();
void draw();

void setup()
{
	Wire.begin();
	Serial.begin(9600);
	SerialPlayer.begin(9600);
	u8g2.begin();
	// Logo des Karibous
  u8g2_splash_screen();
	delay(2000);
	// Initialisation du MP3
	if (!myDFPlayer.begin(SerialPlayer)) statutMp3 = false;
	else statutMp3 = true;
	myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  myDFPlayer.playMp3Folder(1);
	// Menu d'avant Match
  u8g2_menu_avant_match();
	// Gestion tirette

	// Lancement du Match
	timeInit = millis();
	u8g2_splash_screen_GO();
	delay (100);
}

//----------------BOUCLE----------------
void loop()
{
	//testDeplacement();
  Homologation();
}

//----------------TEST DE DEPLACEMENT----------------
void testDeplacement()
{
	sendNavigation(1, 0, 300);
	attente(2000);
	while(askNavigation())
	{
		attente(100);
		Serial.println(askNavigation());
	}
	sendNavigation(1, 0,-300);
	attente(2000);
	while(askNavigation())
	{
		attente(100);
		Serial.println(askNavigation());
	}
}

//----------------STRATEGIE D'HOMOLOGATION----------------
void Homologation()
{
	turnGo(0,false,0,900);
	turnGo(0,true,90,150);
	turnGo(0,false,0,-1000);
	turnGo(0,false,-45,-980);
	turnGo(0,false,-45,-250);
	turnGo(0,true,0,-100);
	turnGo(0,false,0,300);
	// turnGo(0,false,-45,750);
  // turnGo(0,false,135,650); // Pousser les cube
	// turnGo(0,false,0,-480);
	// turnGo(0,false,90,500);
	// turnGo(0,true,0,220); // recalage Tube
	// turnGo(0,false,0,-200);
	// turnGo(0,false,90,800);

  while(1);
}

//----------------DEMANDE L'ETAT DU DEPLACEMENT----------------
bool askNavigation()
{
  bool etatNavigation = true;
  Wire.requestFrom(carteDeplacement, 1);
  char reponseNavigation = Wire.read();
  if (reponseNavigation=='N') etatNavigation = true ;
  else if (reponseNavigation=='O') etatNavigation = false ;
	return etatNavigation;
}

//----------------PROCEDURE DE MAJ DU SCORE----------------
void majScore(int points, int multiplicateur)
{
	score = score + (points*multiplicateur);
}

//----------------MISE A JOUR DU TEMPS DE MATCH----------------
int majTemps()
{
  int tempsRestant = ( tempsMatch - (millis() - timeInit) ) / 1000;
  if ( tempsRestant <= 0 )
  {
    finMatch();
  }
  return tempsRestant;
}

//----------------PROCEDURE D'ATTENTE----------------
void attente(int temps)
{
	int initTemps = millis();
	while( (millis()-initTemps) <= temps)
	{
		// Faire des choses dans la procedure d'attente
		majTemps();
	}
}

//----------------ENVOI UNE COMMANDE TURN GO----------------
void turnGo(bool recalage,bool ralentit,int turn, int go)
{
	bitWrite(optionNavigation,0,equipe);
	bitWrite(optionNavigation,1,recalage);
	bitWrite(optionNavigation,2,ralentit);
	sendNavigation(optionNavigation, turn, go);
	attente(600);
	while(askNavigation())
	{
		attente(100);
		//Serial.println(askNavigation());
	}
}

//----------------ENVOI UNE COMMANDE DE DEPLACEMENT ABSOLU----------------
void sendNavigation(byte fonction, int X, int Y, int rot)
{
	Wire.beginTransmission(carteDeplacement);
	Wire.write(fonction);
	Wire.write(X >> 8);
	Wire.write(X & 255);
	Wire.write(Y >> 8);
	Wire.write(Y & 255);
	Wire.write(rot >> 8);
	Wire.write(rot & 255);
	Wire.endTransmission();
}

//----------------ENVOI UNE COMMANDE DE DEPLACEMENT RELATIF----------------
void sendNavigation(byte fonction, int rot, int dist)
{
	if ( equipe == vert ) rot = -rot ;
	Wire.beginTransmission(carteDeplacement);
	Wire.write(fonction);
	Wire.write(rot >> 8);
	Wire.write(rot & 255);
	Wire.write(dist >> 8);
	Wire.write(dist & 255);
	Wire.endTransmission();
}

//----------------PROCEDURE DE FIN DE MATCH----------------
void finMatch()
{
	// Stopper les moteurs
	sendNavigation(255, 0, 0);
	// Boucle infinie
	while(1)
	{
		// Stopper les moteurs
		sendNavigation(255, 0, 0);
	}
}

//----------------GESTION DES PAGES LCD-------------------
void u8g2_prepare() {
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void u8g2_splash_screen() {
  u8g2.clearBuffer();
  u8g2.drawXBMP( 0, 15, LOGO_KARIBOUS_width, LOGO_KARIBOUS_height, LOGO_KARIBOUS_bits);
  u8g2.sendBuffer();
}

void u8g2_menu_pendant_match() {
  u8g2.setFont(u8g2_font_inr42_mn);
  u8g2.setCursor(8, 9);
  u8g2.print(score);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr( 0, 0, "Score:");
  u8g2.drawStr( 70, 0, "Temps:    sec");
  u8g2.setCursor(95, 0);
  u8g2.print(tempsMatch);
  u8g2.drawStr( 105, 57, "points");
}

void u8g2_menu_avant_match() {
  u8g2.clearBuffer();
  u8g2_prepare();
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr( 0, 0, "Etat MP3:");
    if ( statutMp3 )
    {
      u8g2.drawStr( 40, 0, "OK");
    }
    else
    {
      u8g2.drawStr( 40, 0, "Echec");
    }
  u8g2.sendBuffer();
}

void u8g2_splash_screen_GO() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso58_tr);
  u8g2.drawStr( 18, 2, "GO!");
  u8g2.sendBuffer();
}

void draw() {
  u8g2_prepare();
  u8g2_menu_pendant_match();
}