#include <LiquidCrystal_I2C.h>

#define BOTAO_30 2
#define BOTAO_45 3
#define BOTAO_60 4
#define BOTAO_120 5
#define BOTAO_PARAR 6
#define RELE_FITA 9
#define RELE_MOTOR 10
#define LED_VERMELHO 11
#define LED_VERDE 12
#define BUZZER 13
#define BUZZER_TOM 1800
#define BUZZER_DURACAO_SOM 1500
#define BUZZER_DURACAO_SILENCIO 1000
#define BUZZER_QTD_TOQUES 3
#define TEMPO_PISCA_LED_VERDE 500

LiquidCrystal_I2C lcd(0x3F, 16, 2);
unsigned long tempoCozimento = 0;
unsigned long inicioCozimento = 0;
unsigned long ultimaAtualizacao = 0;
unsigned long ultimoBuzzer = 0;
unsigned long tempoLedVerde = 0;
unsigned long ultimoBotao = 0;
int buzzerCounter = 0;
bool botaoPressionado = false;
bool cozinhando = false;

void setup()
{
    lcd.init();
    lcd.setBacklight(5);
    lcd.print("INICIALIZANDO...");

    pinMode(BOTAO_30, INPUT_PULLUP);
    pinMode(BOTAO_45, INPUT_PULLUP);
    pinMode(BOTAO_60, INPUT_PULLUP);
    pinMode(BOTAO_120, INPUT_PULLUP);
    pinMode(BOTAO_PARAR, INPUT_PULLUP);
    pinMode(RELE_MOTOR, OUTPUT);
    pinMode(RELE_FITA, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);
    digitalWrite(RELE_MOTOR, HIGH);
    digitalWrite(RELE_FITA, HIGH);
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
    lcd.setBacklight(2);
    lcd.clear();
    atualizarLcd();
}

void loop()
{
    buttonHook();
    atualizarEstado();
}

void buttonHook()
{
    bool botao30 = digitalRead(BOTAO_30) == 0;
    bool botao45 = digitalRead(BOTAO_45) == 0;
    bool botao60 = digitalRead(BOTAO_60) == 0;
    bool botao120 = digitalRead(BOTAO_120) == 0;
    bool botaoParar = digitalRead(BOTAO_PARAR) == 0;
    bool algumBotao = botao30 || botao45 || botao60 || botao120 || botaoParar;

    if (botaoPressionado && algumBotao)
        return;

    if (algumBotao)
    {
        botaoPressionado = true;

        if (botaoParar && cozinhando)
        {
            finalizarCozimento();
            return;
        }
        if (botao30 && ((millis() - 500) > ultimoBotao))
            tempoCozimento += 1800000;
        if (botao45 && ((millis() - 500) > ultimoBotao))
            tempoCozimento += 2700000;
        if (botao60 && ((millis() - 500) > ultimoBotao))
            tempoCozimento += 3600000;
        if (botao120 && ((millis() - 500) > ultimoBotao))
            tempoCozimento += 7200000;

        if (cozinhando) {
            iniciarCozimento();
            atualizarLcd();
        }
        else
            iniciarCozimento();

        ultimoBotao = millis();
    }
    else
        botaoPressionado = false;
}

void atualizarEstado()
{
    if (cozinhando && ((millis() - inicioCozimento) > tempoCozimento))
    {
        buzzerCounter = BUZZER_QTD_TOQUES;
        finalizarCozimento();
        return;
    }

    if (!cozinhando && buzzerCounter > 0)
        tocarBuzzer();

    if (cozinhando && (ultimaAtualizacao < (millis() - 1000)))
    {
        atualizarLcd();
        ultimaAtualizacao = millis();
    }
    if (cozinhando && ((millis() - TEMPO_PISCA_LED_VERDE) > tempoLedVerde)) {
      if(digitalRead(LED_VERDE)) digitalWrite(LED_VERDE, LOW);
      else
        digitalWrite(LED_VERDE, HIGH);
      tempoLedVerde = millis();
    }
}

void atualizarLcd()
{
    if (cozinhando)
    {
        char minutosStr[3];
        char segundosStr[3];
        int tempoTotal = (tempoCozimento - (millis() - inicioCozimento)) / 1000;
        int minutos = tempoTotal / 60;
        int segundos = tempoTotal % 60;
        sprintf(minutosStr, "%02d", minutos);
        sprintf(segundosStr, "%02d", segundos);
        lcd.setCursor(5, 1);
        lcd.print(minutosStr);
        lcd.print(":");
        lcd.print(segundosStr);
    }
    else
    {
        lcd.print("   AGUARDANDO");
        lcd.setCursor(0, 1);
        lcd.print("    TRABALHO");
    }
}

void iniciarCozimento()
{
    cozinhando = true;
    inicioCozimento = millis();
    buzzerCounter = 0;
    digitalWrite(RELE_MOTOR, LOW);
    digitalWrite(RELE_FITA, LOW);
    char minutosStr[3];
    int minutos = (tempoCozimento / 1000) / 60;
    sprintf(minutosStr, "%02d", minutos);
    lcd.clear();
    lcd.print("ASSANDO: ");
    lcd.print(minutosStr);
    lcd.print("'");
}

void finalizarCozimento()
{
    cozinhando = false;
    tempoCozimento = 0;
    digitalWrite(RELE_MOTOR, HIGH);
    digitalWrite(RELE_FITA, HIGH);
    digitalWrite(LED_VERDE, LOW);
    lcd.clear();
    atualizarLcd();
}

void tocarBuzzer()
{
    if (ultimoBuzzer < (millis() - (BUZZER_DURACAO_SILENCIO + BUZZER_DURACAO_SOM)))
    {
        ultimoBuzzer = millis();
        tone(BUZZER, BUZZER_TOM, BUZZER_DURACAO_SOM);
        buzzerCounter--;
    }
}