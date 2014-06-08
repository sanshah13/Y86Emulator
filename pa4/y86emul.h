#ifndef Y86EMUL_H
#define Y86EMUL_H

//Directives
void sizeDir(char * tok);
void stringDir(char * tok);
void longDir(char * tok);
void bssDir(char * tok);
void byteDir(char * tok);
void textDir(char * tok);

//Helper methods
unsigned int hextobyte(char * byte);
long getImmediateValue(unsigned long tempPC);
unsigned long getAddressValue(unsigned long tempPC);
char * getByteChars(unsigned int byte);
long hextodecSigned(char * num);
unsigned long hextodecUnsigned(char * num);
double power(int num, int exponent);
void twoscomplement(char * number);

//Execution methods
void fetchDecodeExecute();
void halt();
void rrmovl();
void irmovl();
void rmmovl();
void mrmovl();
void addl();
void subl();
void andl();
void xorl();
void mull();
void jmp();
void jle();
void jl();
void je();
void jne();
void jge();
void jg();
void call();
void ret();
void pushl();
void popl();
void readb();
void readl();
void writeb();
void writel();

#endif