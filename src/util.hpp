typedef unsigned char byte;
typedef unsigned int word;
typedef uint64_t microcode;
//#ifdef _WIN32
//typedef unsigned long long microcode;
//#else
//typedef unsigned  long microcode;
//#endif

class util{
    public:
    static void write_microcode(microcode w);
    static void write_word(word w);
    static void write_byte(byte b);
    static void write_dec(word d);
    static void clear();
};