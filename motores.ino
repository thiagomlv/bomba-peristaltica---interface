/* 
    AUTOR: Thiago Oliveira

    OBSERVACOES: VERSAO DE TESTE
    ==> No arduino Mega do projeto, a interface serial1 usada eh a Serial11, que tem o RX no pino 19
*/

// --- Bibliotecas Auxiliares ---k
#include <AccelStepper.h>

// --- Definicao de Macros ---
#define     TAMANHO_MENSAGEM_POR_BOMBA     6           // A0000-0-000: A mensagem de cada bomba tem 11 caracteres

// --- Mapeamento de Hardware ---
#define     A_STEP_PIN       60
#define     A_DIR_PIN        61
#define     A_ENABLE_PIN     56                        // nao usado no codigo: por padrao em pull down

#define     B_STEP_PIN       46
#define     B_DIR_PIN        48
#define     B_ENABLE_PIN     62                        // nao usado no codigo: por padrao em pull down


// --- Mapeamento de Logica ---
String mensagem,                                       // armazena a mensagem recebida pela interface serial1
       mensagem_A,                                     // armazena a parte da mensagem para a bomba A
       mensagem_B,                                     // armazena a parte da mensagem para a bomba B
       fluxo;

int indice_separador;                                  // armazena o indice do separador "|" na mensagem recebida 

float fluxo_A = 0.0,                                   // armazena o fluxo atual da bomba A
      fluxo_B = 0.0,                                   // armazena o fluxo atual da bomba B
      fluxo_float = 0.0;                               // armazena o fluxo no formato inteiro temporariamente


// --- Prototipo das funcoes auxiliares ---
void processarParametros(String mensagem, char bomba);
void acionar_bomba(char bomba);
void parar_bomba(char bomba);
void voltar_posicao_inicial(char bomba);
float converterRPMParaPassos(float rpm);
float converterMLParaPassos(float rpm);

// --- Inicializacao da biblioteca auxiliar ---
AccelStepper bomba_A(AccelStepper::DRIVER, A_STEP_PIN, A_DIR_PIN);
AccelStepper bomba_B(AccelStepper::DRIVER, B_STEP_PIN, B_DIR_PIN);


/*
##################################################################################################
Configuracoes iniciais
##################################################################################################
*/

void setup() 
{
    pinMode(A_DIR_PIN,  OUTPUT);                // define o pino de direcao do motor A como OUTUP 
    pinMode(A_STEP_PIN, OUTPUT);                // define o pino de passos do motor A como OUTUP 
    pinMode(B_DIR_PIN,  OUTPUT);                // define o pino de direcao do motor B como OUTUP  
    pinMode(B_STEP_PIN, OUTPUT);                // define o pino de passos do motor A como OUTUP 
        
    Serial1.begin(9600);                         // inicializa a interface de comunicacao serial1 1

    bomba_A.setMaxSpeed(1000);                  // Velocidade máxima (passos/segundo)
    bomba_A.setAcceleration(500);               // Aceleração (passos/segundo²)

    bomba_B.setMaxSpeed(1000);                  // Velocidade máxima (passos/segundo)
    bomba_B.setAcceleration(500);               // Aceleração (passos/segundo²)
}

/*
##################################################################################################
Cotrole do sistema
##################################################################################################
*/

void loop()
{
    if (Serial1.available() > 0) 
    {
        mensagem = Serial1.readStringUntil('\n');       // le a mensagem ate o caractere de nova linha
        mensagem.trim();                               // remove espaços em branco e caracteres de nova linha

        // verifica se a mensagem contem os delimitadores esperados
        if (mensagem.indexOf('|') != -1) 
        {
            // divide a mensagem em duas partes: A e B
            indice_separador = mensagem.indexOf('|');
            mensagem_A = mensagem.substring(0, indice_separador);
            mensagem_B = mensagem.substring(indice_separador);

            // processa a parte A
            if (mensagem_A.length() == TAMANHO_MENSAGEM_POR_BOMBA) processarParametros(mensagem_A, "A");

            // processa a parte B
            if (mensagem_B.length() == TAMANHO_MENSAGEM_POR_BOMBA) processarParametros(mensagem_B, "B");;
        }
    }
}

/*
##################################################################################################
Extrair parametros: fluxo, unidade e seringa
##################################################################################################
*/

/*
* @brief extrai o fluxo, a unidade e o tamanho da seringa da mensagem apenas da bomba selecionada
* @param menssagem: mensagem apenas da bomba selecionada (ex.: A0000-0-000)
* @param bomba: bomba selecionada
*/
void processarParametros(String fluxo, char bomba)
{   
    if (bomba == 'A')
    {
        if (fluxo != "XXXX")                                   // o fluxo eh o codigo de reiniciar "XXXX"?
        {                                                      // nao...
            fluxo_float = fluxo.toFloat();                       // converte o fluxo para inteiro
            if (fluxo_A != fluxo_int)                          // verifica se o fluxo recebido eh igual ao que ja estava
            {                                                  // nao...
                fluxo_A = fluxo_int;                           // atualiza o fluxo da bomba com o novo fluxo (localmente)
            }

            if (fluxo_A == 0) parar_bomba('A');                // para a bomba
            else              acionar_bomba('A');              // aciona a bomba com os parametros devidos
        }
        else
        {
            fluxo_A = 0;                                       // define o fluxo atual como zero
            voltar_posicao_inicial('A');                       // volta a bomba para a posicao inicial
        }
    }

    if (bomba == 'B')
    {
        if (fluxo != "XXXX")                                   // o fluxo eh o codigo de reiniciar "XXXX"?
        {                                                      // nao...
            fluxo_int = fluxo.toInt();                         // converte o fluxo para inteiro
            if (fluxo_B != fluxo_int)                          // verifica se o fluxo recebido eh igual ao que ja estava
            {                                                  // nao...
                fluxo_B = fluxo_int;                           // atualiza o fluxo da bomba com o novo fluxo (localmente)
            }

            if (fluxo_B == 0) parar_bomba('B');                // para a bomba
            else              acionar_bomba('B');              // aciona a bomba com os parametros devidos
        }
        else
        {
            fluxo_B = 0;                                       // define o fluxo atual como zero
            voltar_posicao_inicial('B');                     // volta a bomba para a posicao inicial
        }
    }
}

/*
##################################################################################################
Acionar a bomba com os devidos parametros definidos pelo usuario
##################################################################################################
*/

void acionar_bomba(char bomba)
{
    if (bomba == 'A')
    {
        bomba_A.setSpeed(fluxo_A);                       // define a velocidade deseja em passos/segundo
        bomba_A.runSpeed();                              // aciona essa velocidade, sem considerar aceleracao suave
    } else
    if (bomba == 'B')
    {
        bomba_B.setSpeed(fluxo_B);                       // define a velocidade deseja em passos/segundo
        bomba_B.runSpeed();                              // aciona essa velocidade, sem considerar aceleracao suave
    }
}

/*
##################################################################################################
Para o movimento da bomba
##################################################################################################
*/

void parar_bomba(char bomba)
{
    if (bomba == 'A') bomba_A.stop();                    // para o movimento da bomba A
    if (bomba == 'B') bomba_B.stop();                    // para o movimento da bomba B
}

/*
##################################################################################################
Volta a bomba a posicao inicial
##################################################################################################
*/

void voltar_posicao_inicial(char bomba)
{
    if (bomba == 'A')
    {

        bomba_A.setMaxSpeed(200);         // define a velocidade maxima para 200 passos/segundo
        bomba_A.setAcceleration(0);       // atinge 200pps instantaneamente
        bomba_A.moveTo(0);                // define o alvo de posição como 0
        bomba_A.stop();                   // para o motor
        bomba_A.moveTo(0);                // move para a posicao 0
        bomba_A.run();                    // atualiza o motor para alcançar a posicao 0

        // aguarda ate que o motor alcance a posicao 0
        while (bomba_A.distanceToGo() != 0) 
        {
            bomba_A.run();
        }

        bomba_A.setMaxSpeed(1000);                  // Velocidade máxima (passos/segundo)
        bomba_A.setAcceleration(500);               // Aceleração (passos/segundo²)
    } 
    else if (bomba == 'B')
    {
        bomba_B.setMaxSpeed(200);         // define a velocidade maxima para 200 passos/segundo
        bomba_B.setAcceleration(0);       // atinge 200pps instantaneamente
        bomba_B.moveTo(0);                // define o alvo de posição como 0
        bomba_B.stop();                   // para o motor
        bomba_B.moveTo(0);                // move para a posicao 0
        bomba_B.run();                    // atualiza o motor para alcançar a posicao 0

        // aguarda ate que o motor alcance a posicao 0
        while (bomba_B.distanceToGo() != 0) 
        {
            bomba_B.run();
        }

        bomba_B.setMaxSpeed(1000);                  // Velocidade máxima (passos/segundo)
        bomba_B.setAcceleration(500);               // Aceleração (passos/segundo²)
    }
}
