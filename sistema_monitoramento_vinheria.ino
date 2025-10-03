// Sistema de Monitoramento de Luminosidade para Vinheria

#include <LiquidCrystal.h>

//Variáveis dos pinos
int redPin = 8; // Pino vermelho para fora dos limites
int yellowPin = 7; //Pino amarelo para estado de alerta
int greenPin = 6; //Pino verde para ambiente OK
int buzzer = 13; //Buzzer para alarme
int pinoLDR = A0; //Pino LDR no pino A0

LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2); 

//Controle do buzzer
unsigned long tempoInicioBuzzer = 0;
bool buzzerAtivo = false;

//Luminosidade
unsigned int lumOK = 40;
unsigned int lumAlerta = 65;

// Variáveis para calcular a média
const int numLeituras = 10;  // Número de leituras que vão ser feitas
int leituras[numLeituras];  // Array para armazenar leituras
int indiceLeitura = 0;
int total = 0;
int media = 0;

//Cacho de Uvas
byte uva[8] = {
  B01110,
  B11111,
  B11111,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110
};

//Folhas
byte folha[8] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00100,
  B00000
};

void setup(){
  //Definindo os pinos como saída ou entrada
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pinoLDR, INPUT);
  
  lcd.begin(16,2);
  
  // Cria os caracteres customizados
  lcd.createChar(0, uva);
  lcd.createChar(1, folha);
  
  //Mensagem de boas-vindas no display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1)); // Folha
  lcd.print(" Vinharia  ");
  lcd.write(byte(1)); // Folha
  
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); // Uva
  lcd.print("  Agnello  ");
  lcd.write(byte(0)); // Uva
  delay(3000); //Mantém a mensagem durante 3 segundos  
  
  lcd.clear(); //Limpar o display
  
  //Define a velocidade do monitor serial
  Serial.begin(9600);

//Inicia as leituras com zero
  for (int i = 0; i < numLeituras; i++) {
    leituras[i] = 0;
  }
}
    
void loop(){
  //Subtrai a leitura mais antiga
  total = total - leituras[indiceLeitura];
  
  //Faz a leitura do LDR
  int valorLDR = analogRead(pinoLDR);
  leituras[indiceLeitura] = valorLDR;
  
  //Adiciona a nova leitura ao total
  total = total + leituras[indiceLeitura];
  
  //Avança para a próxima posição no array
  indiceLeitura = indiceLeitura + 1;
  
  //Se chegou no final do array, volta para o início
  if (indiceLeitura >= numLeituras) {
    indiceLeitura = 0;
  }
  
  //Calcula a média
  media = total / numLeituras;
  
  //Usando map(), fazemos o mapeamento e definimos 0 para 0% e 1023 para 100%
  int luminosidadePercent = map(media, 0, 1023, 0, 100);
  
  //Mostra a luminosidade atual no display
  lcd.setCursor(0, 0);
  lcd.print("Luminosidade:   ");
  lcd.setCursor(0, 1);
  lcd.print(luminosidadePercent);
  lcd.print("%    ");
  
  //Exibe no monitor
  
  Serial.print("Leitura atual: ");
  Serial.print(valorLDR);
  Serial.print(" | Media:   ");
  Serial.print(media);
  Serial.print(" | Luminosidade:   ");
  Serial.print(luminosidadePercent);
  Serial.println("%");
  
  //Verifica quantos % de luminosidade tem e ativa o led correspondente
  verificarCondicoes(luminosidadePercent);
  
  //Controla o buzzer que vai desligar depois de 3 segundos
  controlarBuzzer();
  
  //Espera 1 segundo e faz a leitura de novo
  delay(1000);
}

void verificarCondicoes(int luminosidade) {
  
  apagarTodosLEDs();
  
  if (luminosidade >= 0 && luminosidade <= lumOK){ //entre 0% e 40%
    digitalWrite(greenPin, HIGH);
    lcd.setCursor(7, 1);
    lcd.print(" - OK   ");
    
    Serial.println("Status: OK - Ambiente ideal (0-40%)");
    
  } else if (luminosidade >= 41 && luminosidade <= lumAlerta){ //entre 41% e 65%
    digitalWrite(yellowPin, HIGH);
    lcd.setCursor(7, 1);
    lcd.print(" ALERTA ");
    
   //Ativa o buzzer durante 3 segundos e caso tena passado os 3 segundos, reativa o buzzer
    if (!buzzerAtivo) {
      buzzerAtivo = true;
      tempoInicioBuzzer = millis();
      digitalWrite(buzzer, HIGH);
      Serial.println("Buzzer ativado - Nível de ALERTA");
    }
    
    Serial.println("Status: ALERTA - Luminosidade entre 41-65%");

  } else if (luminosidade >= 66 && luminosidade <= 100) { //entre 66% e 100%
    digitalWrite(redPin, HIGH);
    lcd.setCursor(7, 1);
    lcd.print(" CRITICO ");
    
    if (!buzzerAtivo) {
      buzzerAtivo = true;
      tempoInicioBuzzer = millis();
      digitalWrite(buzzer, HIGH);
      Serial.println("Buzzer ativado - Nível CRÍTICO");
    }
    
    Serial.println("Status: CRÍTICO - Luminosidade entre 66-100%");
  }
}

void controlarBuzzer() { // Desliga o buzzer após 3 segundos e faz ele voltar a tocar
  if (buzzerAtivo && (millis() - tempoInicioBuzzer >= 3000)) {
    digitalWrite(buzzer, LOW);
    buzzerAtivo = false;
    Serial.println("Buzzer desligado - aguardando próximo ciclo");
  }
}

void apagarTodosLEDs() {
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
}
