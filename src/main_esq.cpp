#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <bitset>
#include <stdint.h>

#include "util.hpp"

//using namespace std;
//#define TEST_MODE
#define REVERSE_SHIFT
//definiçes de tipos

//estrutura para guardar uma microinstruçao decodificada
struct decoded_microcode
{
    word nadd;
    byte jam;
    byte sft;
    byte alu;
    word reg_w;
    byte mem;
    byte reg_r;
};

//sinalizador para desligar máquina
bool halt = false;

//memoria principal
#define MEM_SIZE 0xFFFF + 1 //0xFFFF + 0x1; // 64 KBytes = 64 x 1024 Bytes = 65536 (0xFFFF+1) x 1 Byte;
byte memory[MEM_SIZE];      //0x0000 a 0xFFFF (0 a 65535)

//registradores
word mar = 0, mdr = 0, pc = 0, sp = 0, lv = 0, cpp = 0, tos = 0, opc = 0, h = 0;
byte mbr = 0;

//barramentos
word bus_a = 0, bus_b = 0, bus_c = 0, alu_out = 0;

//estado da ALU para salto condicional
byte n = 0, z = 1;

//registradores de microprograma
word mpc = 0;

//memória de microprograma: 512 x 64 bits = 512 x 8 bytes = 4096 bytes = 4 KBytes.
//Cada microinstrução é armazenada em 8 bytes (64 bits), mas apenas os 4,5 bytes (36 bits) de ordem mais baixa são de fato decodificados.
//Os 28 bits restantes em cada posição da memória são ignorados, mas podem ser utilizados para futuras melhorias nas microinstruções para controlar microarquiteturas mais complexas.
microcode microprog[512];

//carrega microprograma
//Escreve um microprograma de controle na memória de controle (array microprog, declarado logo acima)
void load_microprog()
{
    FILE *mp = fopen("microprog.rom", "rb");
    if (mp != NULL)
    {
        fread(microprog, sizeof(microcode), 512, mp);
        fclose(mp);
    }
    else
    {
        std::cout << "O microprograma não foi encontrado, Abortando " << std::endl;
        exit(-1);
    }

    //aula 9
    // [0] 000000000 100 00 110101 000000100 001 0001   //PC ← PC + 1; fetch; GOTO [MBR]
    // [1] 000000010 000 00 110101 000000100 001 0001   //PC ← PC + 1; fetch; GOTO [2]
    // [2] 000000011 000 00 010100 100000000 000 0010   //H ← MBR; GOTO [3]
    // [3] 000000100 000 00 110101 000000100 001 0001   //PC ← PC + 1; fetch; GOTO [4]
    // [4] 000000101 000 00 010100 010000000 000 0010   //OPC ← MBR; GOTO [5]
    // [5] 000000110 000 00 111100 000000010 000 0010   //MDR ← H + MBR; GOTO [6]
    // [6] 000000111 000 00 011000 000100000 000 0001   //CPP <- PC;GOTO[7]
    // [7] 000001000 000 00 110110 000100000 000 0110   //CPP<- CPP-1;GOTO[8];
    // [8] 000001001 000 00 110110 000100000 000 0110   //CPP<- CPP-1;GOTO[9];
    // [9] 000001010 000 00 010100 000000001 000 0110   //MAR<-CPP;GOTO[10];
    //[10] 000001010 000 00 010100 000000100 100 0110   // PC<-CPP;GOTO[10]
    // microprog[0] = 0b000000000100001101010000001000010001;
    // microprog[1] = 0b000000010000001101010000001000010001;
    // microprog[2] = 0b000000011000000101001000000000000010;
    // microprog[3] = 0b000000100000001101010000001000010001;
    // microprog[4] = 0b000000101000000101000100000000000010;
    // microprog[5] = 0b000000110000001111000000000100000010;
    // microprog[6] = 0b000000111000000110000001000000000001;
    // microprog[7] = 0b000001000000001101100001000000000110;
    // microprog[8] = 0b000001001000001101100001000000000110;
    // microprog[9] = 0b000001010000000101000000000010000110;
    // microprog[10] = 0b000001010000000100000000001001000001;
    //Aula 10
    //MAIN

    //microprog[0] = 0b000000000100001101010000001000010001; //PC<-PC+1;fetch;GOTOMBR;

    //OPC=0bOPC+memory[end_word]=0b;
    //microprog[2] = 0b000000011000001101010000001000010001; //PC<-PC+1;fetch;
    //microprog[3] = 0b000000100000000101000000000010100010; //MAR<-MBR;read;
    //microprog[4] = 0b000000101000000101001000000000000000; //H<-MDR;
    //microprog[5] = 0b000000000000001111000100000000001000; //OPC<-OPC+H;GOTOMAIN;

    //memory[end_word]=0bOPC;
    //microprog[6] = 0b000000111000001101010000001000010001; //PC<-PC+1;fetch;
    //microprog[7] = 0b000001000000000101000000000010000010; //MAR<-MBR;
    //microprog[8] = 0b000000000000000101000000000101001000; //MDR<-OPC;write;GOTOMAIN;

    ////gotoendereco_comando_programa;
    //microprog[9] = 0b000001010000001101010000001000010001;  //PC<-PC+1;fetch;
    //microprog[10] = 0b000000000100000101000000001000010010; //PC<-MBR;fetch;GOTOMBR;

    //if OPC=0 gotoendereco_comando_programaelsegotoproxima_linha;
    //microprog[11] = 0b000001100001000101000100000000001000;  //OPC<-OPC;I FALU=0  GOTO268(100001100)								ELSEGOTO12(000001100);
    //microprog[12] = 0b000000000000001101010000001000000001;  //PC<-PC+1;GOTOMAIN;
    //microprog[268] = 0b100001101000001101010000001000010001; //PC<-PC+1;fetch;
    //microprog[269] = 0b000000000100000101000000001000010010; //PC<-MBR;fetch;GOTOMBR;

    //OPC=OPC-memory[end_word]=0b;
    //microprog[13] = 0b000001110000001101010000001000010001; //PC<-PC+1;fetch;
    //microprog[14] = 0b000001111000000101000000000010100010; //MAR<-MBR;read;
    //microprog[15] = 0b000010000000000101001000000000000000; //H<-MDR;
    //microprog[16] = 0b000000000000001111110100000000001000; //OPC<-OPC-H;GOTOMAIN;
}

//carrega programa na memória principal para ser executado pelo emulador.
//programa escrito em linguagem de máquina (binário) direto na memória principal (array memory declarado mais acima).
void load_prog_arg(const char *str)
{
    FILE *p = fopen(str, "r");
    byte data[4];
    if (p != NULL)
    {

        fread(data, sizeof(byte), 4, p);
        int val = (data[0] & 0xff) | (data[1] & 0xff) << 8 | (data[2] & 0xff) << 12 | (data[3] & 0xff) << 16;
        std::cout << val << std::endl;
        byte *program = new byte[val];
        fread(program, sizeof(byte), val, p);
        fclose(p);
        for (int i = 0; i < val; i++)
        {
            if (i < 20)
            {
                memory[i] = program[i];
            }
            else
            {
                memory[1025 + (i - 20)] = program[i];
            }
        }
        delete[] program;
    }

    else
    {
        std::cout << "O programa não foi encontrado, Abortando " << std::endl;
        exit(-1);
    }
}
void load_prog()
{

    std::string s = "program.bin";
    load_prog_arg(s.c_str());
    // memory[1] = 0x73; //init (bytes 2 e 3 são descartados por conveniência de implementação)

    // memory[4] = 0x0006; //(CPP inicia com o valor 0x0006 guardado na palavra 1 – bytes 4 a 7.)

    // word tmp = 0x1001; //LV

    // memcpy(&(memory[8]), &tmp, 4); //(LV inicia com o valor de tmp guardado na palavra 2 – bytes 8 a 11)

    // tmp = 0x0400; //PC

    // memcpy(&(memory[12]), &tmp, 4); //(PC inicia com o valor de tmp guardado na palavra 3 – bytes 12 a 15)

    // tmp = 0x1001 + 32; //SP
    // //SP (Stack Pointer) é o ponteiro para o topo da pilha.
    // //A base da pilha é LV e ela já começa com algumas variáveis empilhadas (dependendo do programa).
    // //Cada variável gasta uma palavra de memória. Por isso a soma de LV com num_of_vars.

    // memcpy(&(memory[16]), &tmp, 4); //(SP inicia com o valor de tmp guardado na palavra 4 – bytes 16 a 19)

    // memory[0x4001]=0x19;
    // memory[0x4002]=0x5;
    // memory[0x4003]=0x19;
    // memory[0x4004]=0x5;
    // memory[0x4005]=0x6;

    //Aula 9
    // memory[1] = 1;
    // memory[2] = 3;
    // memory[3] = 4;
    //implementar!
    //Aula 10
    //c)
    //memory[1] = 0x2;
    //memory[2] = 10;
    //memory[3] = 0x2;
    //memory[4] = 13;
    //memory[5] = 0x6;
    //memory[6] = 13;
    //memory[7] = 0xd;
    //memory[8] = 13;
    //memory[9] = 0x2;
    //memory[10] = 11;
    //memory[11] = 0xd;
    //memory[12] = 12;
    //memory[13] = 0x6;
    //memory[14] = 11;
    //memory[15] = 0xb;
    //memory[16] = 25;
    //memory[17] = 0xd;
    //memory[18] = 11;
    //memory[19] = 0x9;
    //memory[20] = 1;
    //memory[40] = 5;
    //memory[44] = 3;
    //memory[48] = 1;
    //memory[52] = 0;
    //d)
    //dados
    //memory[40] = 16;
    //memory[44] = 2;
    //memory[48] = 1;
    //memory[52] = 1;
    //Instruções
    //memory[1] = 0x2;//ADD[10]
    //memory[2] = 10;
    //memory[3] = 0xD;//SUB[11]
    //memory[4] = 11;
    //memory[5] = 0x6;//MOV[10]
    //memory[6] = 10;
    //memory[7] = 0xb;//JZ 25
    //memory[8] = 25;
    //memory[9] = 0xd;//Sub[10];
    //memory[10] = 10;
    //memory[11] = 0x2;//add[12]
    //memory[12] = 12;
    //memory[13] = 0x2;//add[13]
    //memory[14] = 13;
    //memory[15] = 0x6;//MOV [12]
    //memory[16] = 12;
    //memory[17] = 0xD;//SUB[12]
    //memory[18] = 12;
    //memory[19] = 0x9;//GOTO 1
    //memory[20] = 1;
}

//exibe estado da máquina
void debug(bool clr = true)
{
    if (clr)
        util::clear();

    std::cout << "Microinstrução: ";
    util::write_microcode(microprog[mpc]);

    std::cout << "\n\nMemória principal: \nPilha: \n";
    for (int i = lv * 4; i <= sp * 4; i += 4)
    {
        util::write_byte(memory[i + 3]);
        std::cout << " ";
        util::write_byte(memory[i + 2]);
        std::cout << " ";
        util::write_byte(memory[i + 1]);
        std::cout << " ";
        util::write_byte(memory[i]);
        std::cout << " : ";
        if (i < 10)
            std::cout << " ";
        std::cout << i << " | " << memory[i + 3] << " " << memory[i + 2] << " " << memory[i + 1] << " " << memory[i];
        word w;
        memcpy(&w, &memory[i], 4);
        std::cout << " | " << i / 4 << " : " << w << std::endl;
    }

    std::cout << "\n\nPC: \n";
    for (int i = (pc - 1); i <= pc + 20; i += 4)
    {
        util::write_byte(memory[i + 3]);
        std::cout << " ";
        util::write_byte(memory[i + 2]);
        std::cout << " ";
        util::write_byte(memory[i + 1]);
        std::cout << " ";
        util::write_byte(memory[i]);
        std::cout << " : ";
        if (i < 10)
            std::cout << " ";
        std::cout << i << " | " << memory[i + 3] << " " << memory[i + 2] << " " << memory[i + 1] << " " << memory[i];
        word w;
        memcpy(&w, &memory[i], 4);
        std::cout << " | " << i / 4 << " : " << w << std::endl;
    }

    std::cout << "\nRegistradores - \nMAR: " << mar << " (";
    util::write_word(mar);
    std::cout << ") \nMDR: " << mdr << " (";
    util::write_word(mdr);
    std::cout << ") \nPC : " << pc << " (";
    util::write_word(pc);
    std::cout << ") \nMBR: " << (int)mbr << " (";
    util::write_byte(mbr);
    std::cout << ") \nSP : " << sp << " (";
    util::write_word(sp);
    std::cout << ") \nLV : " << lv << " (";
    util::write_word(lv);
    std::cout << ") \nCPP: " << cpp << " (";
    util::write_word(cpp);
    std::cout << ") \nTOS: " << tos << " (";
    util::write_word(tos);
    std::cout << ") \nOPC: " << opc << " (";
    util::write_word(opc);
    std::cout << ") \nH  : " << h << " (";
    util::write_word(h);
    std::cout << ")" << std::endl;
}

decoded_microcode decode_microcode(microcode code) //Recebe uma microinstrução binária e separa suas partes preenchendo uma estrutura de microinstrucao decodificada, retornando-a.
{
    //36 bits -> microc´digo
    decoded_microcode dec;
    dec.reg_r = code & 0b1111;
    code >>= 4;
    dec.mem = code & 0b111;
    code >>= 3;
    dec.reg_w = code & 0b111111111;
    code >>= 9;
    dec.alu = code & 0b111111;
    code >>= 6;
    dec.sft = code & 0b11;
    code >>= 2;
    dec.jam = code & 0b111;
    code >>= 3;
    dec.nadd = code & 0b111111111;
    code >>= 9;
    //implementar!
    // std::cout << "reg_r:" << (std::bitset<4>)dec.reg_r << std::endl;
    // std::cout << "mem:" << (std::bitset<3>)dec.mem << std::endl;
    // std::cout << "reg_w:" << (std::bitset<9>)dec.reg_w << std::endl;
    // std::cout << "alu:" << (std::bitset<6>)dec.alu << std::endl;
    // std::cout << "sft:" << (std::bitset<2>)dec.sft << std::endl;
    // std::cout << "jam:" << (std::bitset<3>)dec.jam << std::endl;
    // std::cout << "nadd:" << (std::bitset<9>)dec.nadd << std::endl;
    return dec;
}

//alu
//recebe uma operação de alu binária representada em um byte (ignora-se os 2 bits de mais alta ordem, pois a operação é representada em 6 bits)
//e duas palavras (as duas entradas da alu), carregando no barramento alu_out o resultado da respectiva operação aplicada às duas palavras.
#ifndef SIMPLE_ALU
void alu(byte func, word a, word b)
{
#define _f0 5
#define _f1 4
#define _ena 3
#define _enb 2
#define _inva 1
#define _inc 0

    alu_out = 0;

    word operand1 = 0, operand2 = 0;

    if ((func >> _ena) & 0x1)
    {
        operand1 = a;
    }
    if ((func >> _enb) & 0x1)
    {
        operand2 = b;
    }
    if ((func >> _inva) & 0x1)
    {
        operand1 = ~operand1;
    }

    int selFunction = (((func >> _f0) & 0x1) << 1) + ((func >> _f1) & 0x1);

    //std::cout << "Function number=" << selFunction << std::endl;

    switch (selFunction)
    {
    case 0b00:
        alu_out = operand1 & operand2;
        break;
    case 0b01:
        alu_out = operand1 | operand2;
        break;
    case 0b10:
        alu_out = ~operand2;
        break;
    case 0b11:
        alu_out = operand1 + operand2;
        break;
    }
    if ((func & 0x1))
    {
        alu_out++;
    }
    if (alu_out)
    {
        z = 0;
        n = 1;
    }
    else
    {
        z = 1;
        n = 0;
    }
    //std::cout << "alu_Out" << alu_out << std::endl;

    //implementar
}
#else
void alu(byte func, word a, word b)
{
    switch (func)
    {
    case (0b011000):
        alu_out = a;
        break;
    case (0b010100):
        alu_out = b;
        break;
    case (0b011010):
        alu_out = ~a;
        break;
    case (0b101100):
        alu_out = ~b;
        break;
    case (0b111100):
        alu_out = (a + b);
        break;
    case (0b111101):
        alu_out = (a + b + 1);
        break;
    case (0b111001):
        alu_out = a + 1;
        break;
    case (0b110101):
        alu_out = b + 1;
        break;
    case (0b111111):
        alu_out = b - a;
        break;
    case (0b110110):
        alu_out = b - 1;
        break;
    case (0b111011):
        alu_out = a * -1;
        break;
    case (0b001100):
        alu_out = a & b;
        break;
    case (0b011100):
        alu_out = a | b;
        break;
    case (0b010000):
        alu_out = 0;
        break;
    case (0b110001):
        alu_out = 1;
        break;
    case (0b110010):
        alu_out = -1;
        break;
    default:
        std::cout << "UNdefined microcode" << (std::bitset<6>)func << std::endl;
        break;
    }
    if (alu_out == 0)
    {
        z = 1;
        n = 0;
    }
    else
    {
        n = 1;
        z = 0;
    }
}
#endif
//Deslocamento. Recebe a instrução binária de deslocamento representada em um byte (ignora-se os 6 bits de mais alta ordem, pois o deslocador eh controlado por 2 bits apenas)
//e uma palavra (a entrada do deslocador) e coloca o resultado no barramento bus_c.
void shift(byte s, word w)
{ //implementar!
#ifdef REVERSE_SHIFT

    if (s & 0b10)
    {
        w = w << 8;
    }
    if (s & 0b1)
    {
        w = w >> 1;
    }
#else
    if (s & 0b1)
    {
        w = w << 8;
    }
    if (s & 0b10)
    {
        w = w >> 1;
    }
    
#endif
bus_c = w;
}

//Leitura de registradores. Recebe o número do registrador a ser lido (0 = mdr, 1 = pc, 2 = mbr, 3 = mbru, ..., 8 = opc) representado em um byte,
//carregando o barramento bus_b (entrada b da ALU) com o valor do respectivo registrador e o barramento bus_a (entrada A da ALU) com o valor do registrador h.
void read_registers(byte reg_end)
{
#define _mdr 0
#define _pc 1
#define _mbr 2
#define _mbru 3
#define _sp 4
#define _lv 5
#define _cpp 6
#define _tos 7
#define _opc 8

    bus_a = h;
    switch (reg_end)
    {
    case _mdr:
        bus_b = mdr;
        break;
    case _pc:
        //std::cout << "wrote PC to bus_B" << std::endl;
        bus_b = pc;
        break;
    case _mbr:
        bus_b = (int32_t)(signed char)(mbr);
        break;
    case _mbru:
        bus_b = (uint32_t)mbr;
        break;
    case _sp:
        bus_b = sp;
        break;
    case _lv:
        bus_b = lv;
        break;
    case _cpp:
        bus_b = cpp;
        break;
    case _tos:
        bus_b = tos;
        break;
    case _opc:
        bus_b = opc;
        break;
    }
}

//Escrita de registradores. Recebe uma palavra (valor de 32 bits) cujos 9 bits de ordem mais baixa indicam quais dos 9 registradores que
//podem ser escritos receberao o valor que está no barramento bus_c (saída do deslocador).
void write_register(word reg_end)
{
    //de 9 ao 0
    //H, OPC, TOS,CPP, LV, SP,PC,MDR,MAR
    for (int i = 0; i < 9; i++)
    {
        if ((reg_end >> i) & 1)
        {
            switch (i)
            {
            case 0:
                mar = bus_c;
                break;
            case 1:
                mdr = bus_c;
                break;
            case 2:
                pc = bus_c;
                break;
            case 3:
                sp = bus_c;
                break;
            case 4:
                lv = bus_c;
                break;
            case 5:
                cpp = bus_c;
                break;
            case 6:
                tos = bus_c;
                break;
            case 7:
                opc = bus_c;
                break;
            case 8:
                h = bus_c;
                break;
            default:
                break;
            }
        }
    }
    //implementar!
}

//Leitura e escrita de memória. Recebe em um byte o comando de memória codificado nos 3 bits de mais baixa ordem (fetch, read e write, podendo executar qualquer conjunto dessas três operações ao
//mesmo tempo, sempre nessa ordem) e executa a respectiva operação na memória principal.
//fetch: lê um byte da memória principal no endereço constando em PC para o registrador MBR. Endereçamento por byte.
//write e read: escreve e lê uma PALAVRA na memória principal (ou seja, 4 bytes em sequência) no endereço constando em MAR com valor no registrador MDR. Nesse caso, o endereçamento é dado em palavras.
//Mas, como a memoria principal eh um array de bytes, deve-se fazer a conversão do endereçamento de palavra para byte (por exemplo, a palavra com endereço 4 corresponde aos bytes 16, 17, 18 e 19).
//Lembrando que esta é uma máquina "little endian", isto é, os bits menos significativos são os de posições mais baixas.
//No exemplo dado, suponha os bytes:
//16 = 00110011
//17 = 11100011
//18 = 10101010
//19 = 01010101
//Tais bytes correspondem à palavra 01010101101010101110001100110011
void mainmemory_io(byte control)
{
    //fetch
    if (control & 0b1)
    {
        mbr = memory[pc];
    }
    //read
    if (control & 0b10)
    {
        mdr = 0;

        for (int i = 0; i < 4; i++)
        {
            //std::cout << "mem read address" << (int)mar << std::endl;
            mdr |= (memory[(mar * 4) + i]) << (i * 8);
        }

        //Deu erro rapaz
    }
    //write
    if (control & 0b100)
    {
        word val = mdr;
        for (int i = 0; i < 4; i++)
        {
            memory[(mar * 4) + i] = (val & 0xff);
            val >>= 8;
        }
    }
}

//Define próxima microinstrução a ser executada. Recebe o endereço da próxima instrução a ser executada codificado em uma palavra (considera-se, portanto, apenas os 9 bits menos significativos)
//e um modificador (regra de salto) codificado em um byte (considera-se, portanto, apenas os 3 bits menos significativos, ou seja JAMZ (bit 0), JAMN (bit 1) e JMPC (bit 2)), construindo e
//retornando o endereço definitivo (codificado em uma word - desconsidera-se os 21 bits mais significativos (são zerados)) da próxima microinstrução.
word next_address(word next, byte jam)
{
    //implementar!
    bool jmpc = 0 != (jam & 0b100);
    bool jamn = 0 != (jam & 0b10);
    bool jamz = 0 != (jam & 0b1);
    word bit = 0;
    bit = (jamz && z) || (jamn && n);
    next |= bit << 8;
    if (jmpc)
    {
        next |= mbr;
    }
    // if (next > 254)
    //     std::cout << "NEXT" << next << std::endl;
    return next;
}
#ifdef TEST_MODE
void inline alutest()
{
    alu(0b011000, 5, 0);
    std::cout << "Valor de A =" << alu_out << std::endl;
    //tem que sair 5, o valor de A
    alu(0b010100, 5, 4);
    std::cout << "Valor de B =" << alu_out << std::endl;
    //tem que sair 4 , o valor de b
    alu(0b11010, 5, 0);
    std::cout << "~A =" << std::bitset<32>(alu_out) << std::endl;
    std::cout << "~A signed int =" << (int32_t)(alu_out) << std::endl;
    //tem que sair o complemento de 101 binario, o valor de A
    alu(0b101100, 5, 4);
    std::cout << "~B =" << std::bitset<32>(alu_out) << std::endl;
    std::cout << "~B signed int =" << (int32_t)(alu_out) << std::endl;
    //tem que sair o complemento de 2 de 1111, o valor de b
    alu(0b111100, 5, 4);
    std::cout << "Soma =" << alu_out << std::endl;
    //tem que sair o a soma de 5 + 4
    alu(0b111101, 5, 4);
    std::cout << "Soma e Incremento =" << alu_out << std::endl;
    //tem que sair o a soma de 5 + 4 + 1
    alu(0b111001, 5, 4);
    std::cout << "Soma de A+1 =" << alu_out << std::endl;
    //tem que sair o a soma de 5 + 1
    alu(0b110101, 0, 4);
    std::cout << "Soma de B+1 =" << alu_out << std::endl;
    //tem que sair o a soma de 4 + 1
    alu(0b111111, 10, 20);
    std::cout << "B- A=" << (int32_t)alu_out << std::endl;
    //tem que sair o a soma de B-A
    alu(0b110110, 0, 20);
    std::cout << "B-1 =" << (int32_t)alu_out << std::endl;
    //tem que sair o a soma de B-1
    alu(0b111011, 5, 20);
    std::cout << "-A =" << (int32_t)alu_out << std::endl;
    //-A
    alu(0b001100, 5, 5);
    std::cout << "A&B =" << (int32_t)alu_out << std::endl;
    // A & B
    alu(0b011100, 0b10101, 0b01010);
    std::cout << "A|B =" << (std::bitset<5>)alu_out << std::endl;
    // A | B
    alu(0b010000, 0b10101, 0b01010);
    std::cout << "0 =" << (std::bitset<5>)alu_out << std::endl;
    // 0
    alu(0b110001, 0b10101, 0b01010);
    std::cout << "1 =" << (int32_t)alu_out << std::endl;
    //1
    alu(0b110010, 0b10101, 0b01010);
    std::cout << "-1 =" << (int32_t)alu_out << std::endl;
    //-1
}
void inline microprogtest()
{
    load_microprog();
    for (int i = 0; i < 512; i++)
    {
        std::cout << "Microinstrução 0x" << std::hex << i << " :";
        util::write_microcode(microprog[i]);
        std::cout << "" << std::endl;
        decode_microcode(microprog[i]);
    }
}
#endif

int main(int argc, char *argv[])
{
#ifdef TEST_MODE
    alutest();
    microprogtest();
    load_prog_arg(argv[1]); //carrega programa na memória principal a ser executado pelo emulador. Já que não temos entrada e saída, jogamos o programa direto na memória ao executar o emulador.
    load_prog();
    while (true)
    {
    }
    //std::cout << "microcode sizeof:" << sizeof(microcode) << std::endl;
    return -1;
#endif
    load_microprog(); //carrega microprograma de controle
    if (argc > 1)
        load_prog_arg(argv[1]); //carrega programa na memória principal a ser executado pelo emulador. Já que não temos entrada e saída, jogamos o programa direto na memória ao executar o emulador.
    else
    {
        load_prog();
    }
    decoded_microcode decmcode;

    //laço principal de execução do emulador. Cada passo no laço corresponde a um pulso do clock.
    //o debug mostra o estado interno do processador, exibindo valores dos registradores e da memória principal.
    //o getchar serve para controlar a execução de cada pulso pelo clique de uma tecla no teclado, para podermos visualizar a execução passo a passo.
    //Substitua os comentários pelos devidos comandos (na ordem dos comentários) para ter a implementação do laço.
    while (!halt)
    {
        debug();
        //std::cout <<(int) memory[48] << std::endl;
        decmcode = decode_microcode(microprog[mpc]);     //implementar! Pega uma microinstrução no armazenamento de controle no endereço determinado pelo registrador contador de microinstrução e decodifica (gera a estrutura de microinstrução, ou seja, separa suas partes). Cada parte é devidamente passada como parâmetro para as funções que vêm a seguir.
        read_registers(decmcode.reg_r);                  //implementar! Lê registradores
        alu(decmcode.alu, bus_a, bus_b);                 //implementar! Executa alu
        shift(decmcode.sft, alu_out);                    //implementar! Executa deslocamento
        write_register(decmcode.reg_w);                  //implementar! Escreve registradores
        mainmemory_io(decmcode.mem);                     //implementar! Manipula memória principal
        mpc = next_address(decmcode.nadd, decmcode.jam); //implementar! Determina endereço da microinstrução (mpc) a ser executada no próximo pulso de clock
        getchar();
    }

    debug(false);

    return 0;
}
