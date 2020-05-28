#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define step1Pin  8
#define dir1Pin   7
#define en1Pin    6

#define step2Pin  2
#define dir2Pin   3
#define en2Pin    4

#define R1pin     31
#define R2pin     33
#define R3pin     35
#define R4pin     37
#define R5pin     25

#define S1pin     51
#define S2pin     49
#define S3pin     47
#define S4pin     45
#define S5pin     43
#define S6pin     41
#define S7pin     39

#define B1pin     53

#define MOTOR32FAST 0
#define MOTOR32SLOW 1
#define ESPERADAPRENSA 2000
#define ESPERAINICIO 500

enum state_enum {
                  ZERACILEXT,
                  VAIPARAPRENSA,                                                        
                  VAIPARAEXTRATOR,                           
                  EXTRACAO,                            
                  LIMPEZA,                            
                  VAIPARAALIMENTADOR,                           
                  ALIMENTA,                             
                  ZERACILPRE,
                  ESPERAOKPRENSA,
                  PRENSAAPERTA,
                  PRENSASOBE,
                  ESPERAOKEXTRACAO,
                  RETORNAEXTRATOR,
                  PROCURAEXTRATOR
                };

bool msg_flg = 0;
uint8_t botoes = 0;
int flag = 0;
byte eant = 0;
int qtd = -1;
uint8_t state = ZERACILEXT;
int iant=0;
int iant2=0;
int iant3=0;
int iant4=0;
int iant5=0;
int i=0;
int j=0;

void state_machine_run(uint8_t botoes);
void lebotoes();
void nema32para();
void nema32avanca();
void nema32retorna();
void nema17limpa();
void cilindroExtratorAvanca();
void cilindroExtratorRetorna();
void cilindroExtratorPara();
void cilindroPrensaAvanca();
void cilindroPrensaRetorna();
void cilindroPrensaPara();

LiquidCrystal_I2C lcd(0x27, 16, 2); // Criando um LCD de 16x2 no endereço 0x27

void setup()
{
  Serial.begin(9600);                                                                                  //comunicação com o console serial baud rate de 9600

  TCCR3B = (TCCR3B & 0xF8) | 0x04; //divisor do pin2
  TCCR4B = (TCCR4B & 0xF8) | 0x04; //divisor do pin8

  lcd.init();                 // Inicializando o LCD
  lcd.backlight();            // Ligando o BackLight do LCD

  pinMode(53, INPUT);                                 //Botão1
  pinMode(51, INPUT);                                 //Sensor1
  pinMode(49, INPUT);                                 //Sensor3
  pinMode(47, INPUT);                                 //Sensor2
  pinMode(45, INPUT);                                 //Sensor4
  pinMode(43, INPUT);                                 //Sensor5
  pinMode(41, INPUT);                                 //Sensor6
  pinMode(39, INPUT);                                 //Sensor7

  pinMode(31, OUTPUT);                                //RELE1 CIL1 avança
  pinMode(33, OUTPUT);                                //RELE2 CIL1 volta
  pinMode(35, OUTPUT);                                //RELE3 Cil2 avança
  pinMode(37, OUTPUT);                                //RELE4 Cil2 volta
  
  pinMode(25, OUTPUT);                                //RELE 5 MASTER

  pinMode(4, OUTPUT);                                 //NEMA32 PWM STEP
  pinMode(3, OUTPUT);                                 //NEMA32 DIRECTION
  pinMode(2, OUTPUT);                                 //NEMA32 ENABLE

  pinMode(8, OUTPUT);                                 //NEMA17 PWM STEP
  pinMode(7, OUTPUT);                                 //NEMA17 DIRECTION
  pinMode(6, OUTPUT);                                 //NEMA17 ENABLE

  digitalWrite(37, HIGH);
  digitalWrite(35, HIGH);
  digitalWrite(33, HIGH);
  digitalWrite(31, HIGH);
  digitalWrite(25, LOW);
  digitalWrite(4, LOW);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
  digitalWrite(8, LOW);
  digitalWrite(7, LOW);
  digitalWrite(6, LOW);

  msg_flg = 1;
  delay(ESPERAINICIO);
  TCCR4B = (TCCR4B & 0xF8) | 0x03; //divisor do pin8
}

void loop()
{
  leinput();
  state_machine_run();
}

void state_machine_run()
{
  switch (state)
  {
    case ZERACILEXT:
      nema32para();
      leinput();  
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Posicionando");
        lcd.setCursor(0, 1);
        lcd.print("extrator");
        msg_flg = 0;
      }
    if ((botoes & 0b00100000) == 0)
      {
        cilindroExtratorRetorna();
      }
      else if ((botoes & 0b00100000) != 0)
      {
        cilindroExtratorPara();
        state = ZERACILPRE;
        msg_flg = 1;
      }
    break;

    case ZERACILPRE:
      nema32para();
      leinput();
      if (msg_flg == 1)
      {
        lcd.clear();
          lcd.print("Retornando a");
          lcd.setCursor(0, 1);
          lcd.print("prensa");
          lcd.print(qtd);
          msg_flg = 0;
      }
      if ((botoes & 0b00101000) == 0b00101000)
      {
        cilindroPrensaPara();
        state = PROCURAEXTRATOR;
        msg_flg = 1;
      }
      else if ((botoes & 0b00100000) == 0)
      {
        cilindroPrensaPara();
        state = ZERACILEXT;
        msg_flg = 1;
      }
      else
      {
        cilindroPrensaRetorna();  
      }
    break;

    case PROCURAEXTRATOR:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Procurando");
        lcd.setCursor(0, 1);
        lcd.print("Extrator ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      leinput();
      if ((botoes & 0b100) == 0)
      {
        nema32avanca();
      }
      else
      {
        nema32para();
      }
      if (botoes == 0b00101100)
      {
       state = VAIPARAEXTRATOR;
       msg_flg = 1;
      }
    break;
    
    case VAIPARAEXTRATOR:
      cilindroExtratorPara();
      if (msg_flg == 1)
      {
        lcd.clear();
          lcd.print("-> Extracao");
          lcd.setCursor(0, 1);
          lcd.print("Produzidos ");
          lcd.print(qtd);
          msg_flg = 0;
        }
      leinput();
      if ((botoes & 0b10) == 0)
      {
        nema32retorna();
      }
      else
      {
        nema32para();
      }
      if (botoes == 0b00101010)
      {
        nema32para();
        state = EXTRACAO;
        msg_flg = 1;
      }
    break;
    
    case EXTRACAO:
      nema32para();
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Extraindo");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      
      leinput();
      if ((botoes & 0b01000000) == 0)
      {
        cilindroExtratorAvanca();
      }
      else if((botoes & 0b01000000) != 0)
      {
        cilindroExtratorPara();
      }
      if (botoes == 0b01001010)
      {
        cilindroExtratorPara();
        state = LIMPEZA;
        msg_flg = 1;
      }
    break;
      
    case LIMPEZA:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Limpando");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        qtd = qtd + 1;
        lcd.print(qtd);
        msg_flg = 0;
      }
      nema17limpa();
      delay(800);
      leinput();
      if (botoes == 0b01001010)
      {
        state = ESPERAOKEXTRACAO;
        msg_flg = 1;
      }
    break;
    
    case ESPERAOKEXTRACAO:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Aguardando OK");
        lcd.setCursor(0, 1);
        lcd.print("Para Iniciar");
        msg_flg = 0;
      }
      nema32para();
      leinput();
      if (botoes == 0b11001010)
      {
        state = RETORNAEXTRATOR;
        msg_flg = 1;
      }
    break;

    case RETORNAEXTRATOR:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Limpando");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      if ((botoes & 0b00100000) == 0)
      {
        cilindroExtratorRetorna();
      }
      else
      {
        cilindroExtratorPara();
      }
      leinput();
      if (botoes == 0b00101010)
      {
        cilindroExtratorPara();
        j=0;
        //digitalWrite(en1Pin, HIGH);
        delay(100);
        state = VAIPARAALIMENTADOR;
        msg_flg = 1;
      }
    break;
    
    case VAIPARAALIMENTADOR:
      cilindroExtratorPara();
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("-> Alimentador");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      leinput();
      if ((botoes & 0b001) == 0)
      {
        nema32retorna();
      }
      else
      {
        nema32para();
      }
      if (botoes == 0b00101001)
      {
        state = ALIMENTA;
        msg_flg = 1;
      }
    break;

    case ALIMENTA:
      nema32para();
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Alim. copos");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      delay(2000);
      lcd.clear();
      lcd.print("Completo");
      lcd.setCursor(0, 1);
      lcd.print("Produzidos ");
      lcd.print(qtd);
      leinput();
      if (botoes == 0b00101001)
      {
        state = VAIPARAPRENSA;
        msg_flg = 1;
      }
    break;

    case VAIPARAPRENSA:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("<- Prensa");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        //Serial.println("Estado S1");
        msg_flg = 0;
      }
      leinput();
      if ((botoes & 0b100) == 0)
      {
        nema32avanca();
      }
      else
      {
        nema32para();
      }
      if (botoes == 0b00101100)
      {
       state = ESPERAOKPRENSA;
       msg_flg = 1;
      }
    break;

    case ESPERAOKPRENSA:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Aguardando OK");
        lcd.setCursor(0, 1);
        lcd.print("Para Prensa");
        msg_flg = 0;
      }
      nema32para();
      leinput();
      if (botoes == 0b10101100)
      {
        state = PRENSAAPERTA;
        msg_flg = 1;
      }
    break;

    case PRENSAAPERTA:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Descendo prensa");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      nema32para();
      leinput();
      if ((botoes & 0b00010000) == 0)
      {
        cilindroPrensaAvanca();
      }
      else if ((botoes & 0b00010000) != 0)
      {
        cilindroPrensaPara();
      }
      if ((botoes & 0b00010000) != 0)
      {
        cilindroPrensaPara();
        delay(ESPERADAPRENSA);
        state = PRENSASOBE;
        msg_flg = 1;
      }
    break;

    case PRENSASOBE:
      if (msg_flg == 1)
      {
        lcd.clear();
        lcd.print("Subindo prensa");
        lcd.setCursor(0, 1);
        lcd.print("Produzidos ");
        lcd.print(qtd);
        msg_flg = 0;
      }
      nema32para();
      leinput();
      if ((botoes & 0b00001000) == 0)
      {
        cilindroPrensaRetorna();
      }
      else if ((botoes & 0b00001000) != 0)
      {
        cilindroPrensaPara();
      }
      if ((botoes & 0b00001000) != 0)
      {
        cilindroPrensaPara();
        state = VAIPARAEXTRATOR;
        msg_flg = 1;
      }
    break;
    
  }
}

void leinput()
{
  int botoesant = botoes;
  iant5=iant4;
  iant4=iant3;
  iant3=iant2;
  iant2=iant;
  iant=i;
  i =       ((digitalRead(53) << 7) |   //b1
            (digitalRead(39) << 6) |    //s7
            (digitalRead(41) << 5) |    //s6
            (digitalRead(43) << 4) |    //s5
            (digitalRead(45) << 3) |    //s4
            (digitalRead(49) << 2) |    //s3
            (digitalRead(47) << 1) |    //s2
            (digitalRead(51)));         //s1
  if((iant&iant2&iant3&iant4&iant5)==i)
  {
    botoes=i; 
  }
  delay(5);
}

void nema32avanca()
{
  digitalWrite(dir1Pin, LOW);
  digitalWrite(en1Pin, LOW);
  analogWrite(step1Pin, 128);
}

void nema32para()
{
  digitalWrite(en1Pin, LOW);
  analogWrite(step1Pin, 0);
}

void nema32retorna()
{
  digitalWrite(dir1Pin, HIGH);
  digitalWrite(en1Pin, LOW);
  analogWrite(step1Pin, 128);
}

void nema17limpa()
{
  digitalWrite(dir2Pin, HIGH);
  digitalWrite(en2Pin, LOW);
  analogWrite(step2Pin, 128);
  delay(3500);
  digitalWrite(dir2Pin, LOW);
  digitalWrite(en2Pin, LOW);
  analogWrite(step2Pin, 128);
  delay(3000);
  digitalWrite(dir2Pin, HIGH);
  digitalWrite(en2Pin, HIGH);
  analogWrite(step2Pin, 0);
}

void cilindroExtratorAvanca()
{
  digitalWrite(R5pin, HIGH);
  delay(250);
  digitalWrite(R1pin, HIGH);
  digitalWrite(R2pin, HIGH);
  digitalWrite(R3pin, LOW);
  digitalWrite(R4pin, HIGH);
  delayMicroseconds(100);
}

void cilindroExtratorRetorna()
{
  digitalWrite(R5pin, HIGH);
  delay(250);
  digitalWrite(R1pin, HIGH);
  digitalWrite(R2pin, HIGH);
  digitalWrite(R3pin, HIGH);
  digitalWrite(R4pin, LOW);
  delayMicroseconds(100);
}

void cilindroExtratorPara()
{
  digitalWrite(R5pin, LOW);
  delay(250);
  digitalWrite(R1pin, HIGH);
  digitalWrite(R2pin, HIGH);
  digitalWrite(R3pin, HIGH);
  digitalWrite(R4pin, HIGH);
  delayMicroseconds(100);
}

void cilindroPrensaAvanca()
{
  digitalWrite(R5pin, HIGH);
  delay(250);
  digitalWrite(R1pin, LOW);
  digitalWrite(R2pin, HIGH);
  digitalWrite(R3pin, HIGH);
  digitalWrite(R4pin, HIGH);
  delayMicroseconds(100);
}

void cilindroPrensaRetorna()
{
  digitalWrite(R5pin, HIGH);
  delay(250);
  digitalWrite(R1pin, HIGH);
  digitalWrite(R2pin, LOW);
  digitalWrite(R3pin, HIGH);
  digitalWrite(R4pin, HIGH);
  delayMicroseconds(100);
}

void cilindroPrensaPara()
{
  digitalWrite(R5pin, LOW);
  delay(250);
  digitalWrite(R1pin, HIGH);
  digitalWrite(R2pin, HIGH);
  digitalWrite(R3pin, HIGH);
  digitalWrite(R4pin, HIGH);
  delayMicroseconds(100);
}
