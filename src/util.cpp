#include <iostream>
#include "util.hpp"

void util::write_microcode(microcode w) //Dado uma microinstrucao, exibe na tela devidamente espaçado pelas suas partes.
{
    unsigned int v[36];
    for (int i = 35; i >= 0; i--)
    {
        v[i] = (w & 1);
        w = w >> 1;
    }

    for (int i = 0; i < 36; i++)
    {
        std::cout << v[i];
        if (i == 8 || i == 11 || i == 13 || i == 19 || i == 28 || i == 31)
            std::cout << " ";
    }
}

void util::write_word(word w) //Dada uma palavra (valor de 32 bits / 4 bytes), exibe o valor binário correspondente.
{
    unsigned int v[32];
    for (int i = 31; i >= 0; i--)
    {
        v[i] = (w & 1);
        w = w >> 1;
    }

    for (int i = 0; i < 32; i++)
        std::cout << v[i];
}

void util::write_byte(byte b) //Dado um byte (valor de 8 bits), exibe o valor binário correspondente na tela.
{
    unsigned int v[8];
    for (int i = 7; i >= 0; i--)
    {
        v[i] = (b & 1);
        b = b >> 1;
    }

    for (int i = 0; i < 8; i++)
        std::cout << v[i];
}

void util::write_dec(word d) //Dada uma palavra (valor de 32 bits / 4 bytes), exibe o valor decimal correspondente.
{
    std::cout << (int)d << std::endl;
}

void util::clear(){
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
}
