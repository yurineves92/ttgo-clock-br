#include <WiFi.h>  // Arduino IDE - board: ESP32 DEV Module
#include <WebServer.h>
#include <TFT_eSPI.h>     // https://github.com/Bodmer/TFT_eSPI
#include <Preferences.h>  // no need to install this library

WebServer server(80);
Preferences flash;
TFT_eSPI tft = TFT_eSPI();  // User_Setup: Setup25_TTGO_T_Display.h
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite leftSp = TFT_eSprite(&tft);
TFT_eSprite rightS = TFT_eSprite(&tft);

const char* weekdays[] = {"Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado"};
const char* location[] = {"Brasil", "Nova York", "Alemanha", "Japao"}; // Deve ter o mesmo número de itens que os fusos
const char* timeZone[] = {
    "BRT3",                           // Horário de Brasília
    "EST5EDT,M3.2.0,M11.1.0",         // Nova York
    "CET-1CEST,M3.5.0,M10.5.0/3",     // Alemanha
    "JST-9"                           // Japão
};
const char* startTxt[] = {"Conectando", "ao WiFi", "WiFi OK", "Conectado"};
const char* noConnec[] = {
    "WiFi: Sem conexao.",
    "Conecte-se ao ponto de acesso",
    "'Zeus', abra o navegador",
    "address 127.0.0.1",
    "para login e senha."
};

bool freshStart = true;
String webText, buttons, ssid, pasw;
uint8_t count;

void setup() {
  flash.begin("my-clock", true);       // read from flash, true = read only
  count = flash.getInt("counter", 0);  // retrieve the last set time zone - default to first in the array [0]
  flash.end();
  count = count % (sizeof(timeZone) / sizeof(timeZone[0]));  // modulo (# of elements in array) = prevent errors
  pinMode(35, INPUT_PULLUP);                                 // button "switch time zones"
  pinMode(0, INPUT_PULLUP);
  tft.init(), tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(240, 135);  // sprite for faster rendering
  show_Logo_WiFi();               // this is not really necessary but it's nicer.
  connect_to_WiFi();
  showConnected();
  configTzTime(timeZone[count], "pool.ntp.org");  // set the time zone
}

void loop() {
  displayTime();
  if (!digitalRead(0)) displayOnOff();     // flash button
  if (!digitalRead(35)) switchTimeZone();  // button opposite flash button
}

void show_Logo_WiFi() {
  sprite.fillSprite(sprite.color565(100, 100, 100));
  for (int i = 0; i < 8192; i++) {  // create surface with fine texture
    byte j = random(100) + 50;
    sprite.drawPixel(random(240), random(135), sprite.color565(j, j, j));  // random grayscale
  }
  sprite.setTextColor(TFT_YELLOW);
  sprite.drawCentreString(startTxt[0], 120, 76, 4), sprite.drawCentreString(startTxt[1], 120, 100, 4);
  sprite.fillCircle(120, 56, 6, TFT_BLUE);
  for (byte i = 0; i < 3; i++) sprite.drawSmoothArc(120, 54, 40 - i * 11, 35 - i * 11, 128, 232, TFT_BLUE, TFT_BLUE, 1);
  sprite.pushSprite(0, 0);
}

void showConnected() {
  for (byte i = 0; i < 3; i++) sprite.drawSmoothArc(120, 54, 40 - i * 11, 35 - i * 11, 128, 232, TFT_GREEN, TFT_GREEN, 1);
  sprite.fillCircle(120, 56, 6, TFT_GREEN);
  sprite.pushSprite(0, 0);
  leftSp.createSprite(128, 66);  // sprite for new text to be scrolled up
  leftSp.fillSprite(sprite.color565(100, 100, 100));
  for (int i = 0; i < 3072; i++) {
    byte j = random(100) + 50;
    leftSp.drawPixel(random(128), random(66), sprite.color565(j, j, j));  // random grayscale
  }
  leftSp.setTextColor(TFT_GREEN);
  leftSp.drawCentreString(startTxt[2], 64, 5, 4);
  leftSp.setTextColor(TFT_YELLOW);
  leftSp.drawCentreString(startTxt[3], 64, 30, 4);
  for (int i = 1024; i > 560; i--) leftSp.pushSprite(56, i / 8);  // high values to slow down the animation
  leftSp.pushToSprite(&sprite, 56, 70);
  leftSp.deleteSprite();
}

void show_Message_No_Connection() {  // message on display when there is no WiFi connection
  tft.fillScreen(TFT_NAVY);          // also blue color for the leftmost 9 pixels - sprite.pushSprite(9, 0) below
  tft.setTextColor(TFT_YELLOW);
  for (uint8_t i = 0; i < 5; i++) tft.drawCentreString(noConnec[i], 120, i * 27, 4);
}

void displayTime() {
  struct tm tInfo;       // https://cplusplus.com/reference/ctime/tm/
  getLocalTime(&tInfo);  // SNTP update every 3 hours (default ESP32) since we did not set an interval
  if (freshStart) splitScreen(true);
  sprite.setTextColor(TFT_CYAN, TFT_BLACK);
  sprite.fillSprite(TFT_BLACK);
  sprite.setFreeFont(&FreeSansBold18pt7b);
  sprite.setTextSize(2);
  sprite.setCursor(13, 56);
  sprite.printf("%02d:%02d", tInfo.tm_hour, tInfo.tm_min);
  sprite.setTextSize(1);
  sprite.drawFastHLine(0, 72, 240, WiFi.isConnected() ? TFT_GREEN : TFT_RED);
  sprite.setCursor(187, 58);
  sprite.setFreeFont(&FreeSansBold18pt7b);
  sprite.printf("%02d", tInfo.tm_sec);
  sprite.setTextColor(TFT_YELLOW);
  sprite.drawCentreString(weekdays[tInfo.tm_wday], 120, 80, 1);  // weekday
  sprite.setTextColor(TFT_RED);
  sprite.setFreeFont(&FreeSans12pt7b);
  sprite.setCursor(62, 134);
  sprite.printf("%02d-%02d-%04d", tInfo.tm_mday, 1 + tInfo.tm_mon, 1900 + tInfo.tm_year);  // date
  if (freshStart) splitScreen(false);
  sprite.pushSprite(0, 0);
}

void splitScreen(bool split) {  // split (true) or merge (false) sprite horizontally
  leftSp.createSprite(120, 135), rightS.createSprite(120, 135);
  if (!split) freshStart = false;
  for (byte ver = 0; ver < 135; ver++) {  // divide the sprite into 2 pieces & write data to two smaller sprites
    for (byte hor = 0; hor < 120; hor++) leftSp.drawPixel(hor, ver, sprite.readPixel(hor, ver));
    for (byte hor = 120; hor < 240; hor++) rightS.drawPixel(hor - 120, ver, sprite.readPixel(hor, ver));
  }
  if (split) leftSp.drawFastVLine(119, 0, 135, TFT_BLACK), rightS.drawFastVLine(0, 0, 135, TFT_BLACK);
  for (byte hor = 0; hor < 120; hor++) {  // move both sprites to the left & right outer edges
    if (split) leftSp.pushSprite(0 - hor, 0), rightS.pushSprite(hor + 120, 0);
    else leftSp.pushSprite(hor - 120, 0), rightS.pushSprite(240 - hor, 0);
  }
  leftSp.deleteSprite(), rightS.deleteSprite();
}

void switchTimeZone() {
  leftSp.createSprite(240, 28), leftSp.setFreeFont(&FreeSans12pt7b);
  leftSp.fillSprite(TFT_BLACK);
  leftSp.setTextColor(TFT_RED), leftSp.drawCentreString(location[count], 120, 0, 1);
  for (int tel = 0; tel < 240; tel++) leftSp.pushSprite(tel, 110);  // we need an animation that uses up time = UI debouncing
  count = (count + 1) % (sizeof(timeZone) / sizeof(timeZone[0]));   // increase counter modulo (number of elements in string array)
  configTzTime(timeZone[count], "pool.ntp.org");
  leftSp.fillSprite(TFT_BLACK);
  leftSp.drawCentreString(location[count], 120, 0, 1);
  for (int tel = -960; tel < 1; tel++) leftSp.pushSprite(tel / 4, 114);
  leftSp.deleteSprite();
}

void displayOnOff() {
  byte savedZone;
  flash.begin("my-clock");                                 // read from flash memory
  savedZone = flash.getInt("counter", 0);                  // retrieve the last set time zone - default to first in the array [0]
  if (savedZone != count) flash.putInt("counter", count);  // only write the time zone to flash memory when it was changed
  flash.end();                                             // to prevent chip wear from excessive writing
  digitalWrite(TFT_BL, !digitalRead(TFT_BL)), delay(300);
}

void connect_to_WiFi() {  // connect to WiFi, if not successful: start web server
  WiFi.mode(WIFI_MODE_STA);
  flash.begin("login_data", true);  // true = read only
  ssid = flash.getString("ssid", "REDE");
  pasw = flash.getString("pasw", "123456");
  flash.end();
  WiFi.begin(ssid.c_str(), pasw.c_str());
  for (uint8_t i = 0; i < 50; ++i) {  // we try for about 8 seconds to connect
    if (WiFi.isConnected()) {
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      return;  // jumps out of this function when WiFi connection succeeds
    }
    delay(160);
  }
  show_Message_No_Connection();  // script lands on this line only when the connection fails
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {  // html to put found networks on buttons on web page
    buttons += "\n<button onclick='scrollNaar(this.id)' id='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</button><br>";
  }
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP("Lily", "", 1);
  server.on("/", server_Root);
  server.on("/setting", server_Setting);
  server.begin();
  for (;;) server.handleClient();  // infinite loop until the WiFi credentials are inserted
}

void server_Root() {
  webText = "<!DOCTYPE HTML>\n<html lang='en'>\n<head><title>Setup</title>\n<meta name='viewport' ";
  webText += "content='width=device-width, initial-scale=1.0'>";
  webText += "\n<style>\np {\n  font-family: Arial, Helvetica, sans-serif;\n  font-size: 18px;\n  margin: 0;\n  text-align: ";
  webText += "center;\n}\n\nbutton, input[type=submit] {\n  width: 250px;\n  border-radius: 5px;\n  color: White;\n  padding:";
  webText += " 4px 4px;\n  margin-top: 16px;\n  margin: 0 auto;\n  display:block;\n  font-size: 18px;\n  font-weight: 600;";
  webText += "\n  background: DodgerBlue;\n}\n\ninput {\n  width: 250px;\n  font-size: 18px;\n  font-weight: 600;\n}";
  webText += "\n</style>\n</head>\n<body><p style='font-family:arial; ";
  webText += "font-size:240%;'>WiFi setup\n</p><p style='font-family:arial; font-size:160%;'>\n<br>";
  webText += "Networks found:<br> Click on item to select or<br>Enter your network data<br> in the boxes below:</p><br>";
  webText += buttons;
  webText += "\n<form method='get' action='setting'>\n<p><b>\nSSID: <br>\n<input id='ssid' name='ssid'>";
  webText += "<br>PASW: </b><br>\n<input type='password' name='pass'><br><br>\n<input type='submit' value='Save'>";
  webText += "\n</p>\n</form>\n<script>\nfunction scrollNaar(tekst) {\n  document.getElementById('ssid')";
  webText += ".value = tekst;\n  window.scrollTo(0, document.body.scrollHeight);\n}\n</script>\n</body>\n</html>";
  server.send(200, "\ntext/html", webText);
}

void server_Setting() {
  webText = "<!DOCTYPE HTML>\n<html lang='en'>\n<head><title>Setup</title>\n<meta name='viewport' ";
  webText += "content='width=device-width, initial-scale=1.0'>\n<style>\n* {\n  font-family: Arial, Helvetica";
  webText += ", sans-serif;\n  font-size: 45px;\n  font-weight: 600;\n  margin: 0;\n  text-align: center;\n}";
  webText += "\n\n@keyframes op_en_neer {\n  0%   {height:  0px;}\n  50%  {height:  40px;}\n  100% {height:  0px;}\n}";
  webText += "\n\n.opneer {\n  margin: auto;\n  text-align: center;\n  animation: op_en_neer 2s infinite;\n}";
  webText += "\n</style>\n</head>\n<body>\n<div class=\"opneer\"></div>\nESP will reboot<br>Close this window";
  webText += "\n</body>\n</html>";
  String myssid = server.arg("ssid");  // we want to store this in flash memory
  String mypasw = server.arg("pass");
  server.send(200, "\ntext/html", webText);
  delay(500);
  if (myssid.length() > 0 && mypasw.length() > 0) {
    flash.begin("login_data", false);
    flash.putString("ssid", myssid);
    flash.putString("pasw", mypasw);
    flash.end();
    ESP.restart();
  }
}