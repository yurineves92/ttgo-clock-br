#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Inicializa a biblioteca, pinos definidos no User_Setup.h

uint32_t targetTime = 0;       // Para timeout de 1 segundo

byte omm = 99;
bool initial = 1;
byte xcolon = 0;
unsigned int colour = 0;

// Array dos meses em inglês
const char *mesesEng[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Array dos meses em português (sem acentuação)
const char *mesesPt[] = {"Janeiro", "Fevereiro", "Marco", "Abril", 
                         "Maio", "Junho", "Julho", "Agosto", 
                         "Setembro", "Outubro", "Novembro", "Dezembro"};

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6);  // Hora, Minutos, Segundos

void setup(void) {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  targetTime = millis() + 1000; 
}

void formatarDataBrasileira() {
  const char *data = __DATE__;
  char mes[4];
  int dia, ano;

  sscanf(data, "%s %d %d", mes, &dia, &ano);

  // Encontrar índice do mês
  int mesIndex = 0;
  for (int i = 0; i < 12; i++) {
    if (strcmp(mes, mesesEng[i]) == 0) {
      mesIndex = i; // Índice do mês
      break;
    }
  }

  // Exibe a data formatada
  char dataBrasileira[30];
  sprintf(dataBrasileira, "%02d de %s de %04d", dia, mesesPt[mesIndex], ano);

  // Exibir no display TFT
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(8, 52);
  tft.print(dataBrasileira); // Exibe a data no formato brasileiro
}

void loop() {
  if (targetTime < millis()) {
    targetTime = millis() + 1000;
    ss++;              
    if (ss == 60) {
      ss = 0;
      omm = mm;
      mm++;            
      if (mm > 59) {
        mm = 0;
        hh++;          
        if (hh > 23) {
          hh = 0;
        }
      }
    }

    if (ss == 0 || initial) {
      initial = 0;

      // Chama a função para formatar e exibir a data
      formatarDataBrasileira();

      tft.setTextColor(TFT_BLUE, TFT_BLACK);
      tft.drawCentreString("Yuri Neves", 40, 65, 2);
    }

    // Atualiza o relógio
    byte xpos = 6;
    byte ypos = 0;
    if (omm != mm) {
      tft.setTextColor(0x39C4, TFT_BLACK);
      tft.drawString("88:88", xpos, ypos, 7);
      tft.setTextColor(0xFBE0); 
      omm = mm;

      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 7);
      xpos += tft.drawNumber(hh, xpos, ypos, 7);
      xcolon = xpos;
      xpos += tft.drawChar(':', xpos, ypos, 7);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 7);
      tft.drawNumber(mm, xpos, ypos, 7);
    }

    if (ss % 2) {
      tft.setTextColor(0x39C4, TFT_BLACK);
      xpos += tft.drawChar(':', xcolon, ypos, 7);
      tft.setTextColor(0xFBE0, TFT_BLACK);
    }
  }
}
