//Sangini Shah (sms591)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y86emul.h"

//Global variables
unsigned int pc; //program counter
int startOfInstructions; //address of .text
int endOfInstructions; //last address of instructions list
unsigned long imageSize; //y86 image size
unsigned char * y86image; //y86 image
int ZF = 0, SF = 0, OF = 0; //flags
unsigned long registers[8] = {0, 0, 0, 0, 0, 0, 0, 0}; /*eax 0, ecx 1, edx 2, ebx 3, esp 4, ebp 5, esi 6, edi 7*/

void main(int argc, char * argv[])
{
	if(strcmp(argv[1], "-h") == 0)
	{
		printf("Usage: y86emul <y86 file>\nExecutes the specified y86 machine instructions.\n");
		exit(1);
	}

	FILE * y86file = fopen(argv[1], "r");
	if(y86file == NULL)
	{
		fprintf(stderr, "ERROR: File not found. Please check file name.\n");
		exit(1);
	}
	
	char line[1000000];
	fgets(line, sizeof(line)/sizeof(char), y86file);
	char * tokenized = strtok(line, "\t\"");
	if(strcmp(tokenized, ".size") != 0)
	{
		fprintf(stderr, "ERROR: Incorrect input file. No size directive found.\n");
		exit(1);
	}
	else
	{	
		sizeDir(tokenized);
	} 
	
	int textFlag = 0; //used to check if .text directive has already been checked
	memset(tokenized, 0, sizeof(tokenized)/sizeof(char));
	while(fgets(line, sizeof(line)/sizeof(char), y86file))
	{
		tokenized = strtok(line, "\t\"");
		if(strcmp(tokenized, ".string") == 0)
		{
			stringDir(tokenized);
		}
		else if(strcmp(tokenized, ".long") == 0)
		{
			longDir(tokenized);
		}
		else if(strcmp(tokenized, ".bss") == 0)
		{
			bssDir(tokenized);
		}
		else if(strcmp(tokenized, ".byte") == 0)
		{
			byteDir(tokenized);
		}
		else if(strcmp(tokenized, ".text") == 0)
		{
			if(textFlag == 0)
			{
				textFlag == 1;
				textDir(tokenized);
			}
			else
			{
				fprintf(stderr, "ERROR: More than one .text directive. Invalid file\n");
				exit(1);
			}
		}
		memset(tokenized, 0, sizeof(tokenized)/sizeof(char));
	}
	
	fetchDecodeExecute();
	free(y86image);
}

//Checks for the string directive and allocates the size of the y86 image if it is of appropriate size
void sizeDir(char * tok)
{	
	tok = strtok(NULL, "\t\"");
	imageSize = hextodecUnsigned(tok);
	
	y86image = (char *) calloc(1, imageSize + 1);
}

//Places the specified string at its destination on the y86 image
void stringDir(char * tok)
{
	tok = strtok(NULL, "\t\"");
	unsigned long address = hextodecUnsigned(tok);
	tok = strtok(NULL, "\t\"");
	int i;
	for(i = 0; i < strlen(tok); i++)
	{
		y86image[address + i] = tok[i];
	}
}

//Places the specified long at its destination on the y86 image
void longDir(char * tok)
{
	tok = strtok(NULL, "\t\"");
	unsigned long address = hextodecUnsigned(tok);
	tok = strtok(NULL, "\t\"");
	*((int *) &y86image[address]) = atoi(tok);
}

//Allocates a chunk of memory that is of specified size
void bssDir(char * tok)
{
	tok = strtok(NULL, "\t\"");
	unsigned long address = hextodecUnsigned(tok);
	tok = strtok(NULL, "\t\"");
	int size = atoi(tok);
	
	int i;
	for(i = 0; i <= size; i++)
	{
		y86image[address + i] = (long) NULL;
	}
}

//Places the specified byte at its destination of the y86 image
void byteDir(char * tok)
{
	tok = strtok(NULL, "\t\"");
	unsigned long address = hextodecUnsigned(tok);
	tok = strtok(NULL, "\t\"");
	char byte[3];
	byte[0] = tok[0];
	byte[1] = tok[1];
	byte[2] = '\0';
	unsigned long byteValue = hextodecUnsigned(byte);
	y86image[address] = (unsigned char)byteValue;
}

//Places the y86 instructions onto the image and sets the pc to the specified address
void textDir(char * tok)
{
	tok = strtok(NULL, "\t\"");
	unsigned long address = hextodecUnsigned(tok);
	pc = address;
	startOfInstructions = pc;
	tok = strtok(NULL, "\t\"");
	int i;
	char onebyte[3];
	for(i = 0; i < strlen(tok); i+=2)
	{
		onebyte[0] = tok[i];
		onebyte[1] = tok[i+1];
		onebyte[2] = '\0';
		y86image[address] = hextobyte(onebyte);
		address++;
	}
	endOfInstructions = address;
}

void fetchDecodeExecute()
{
	while(pc < endOfInstructions)
	{
		unsigned int instruction = (unsigned int) y86image[pc];
		if(pc < startOfInstructions)
		{
			exit(1);
		}
		
		switch (instruction)
		{
			case 0x00: pc++; break;
			case 0x10: halt(); pc++; break;
			case 0x20: rrmovl(); pc += 2; break;
			case 0x30: irmovl(); pc += 6; break;
			case 0x40: rmmovl(); pc += 6; break;
			case 0x50: mrmovl(); pc += 6; break;
			case 0x60: addl(); pc += 2; break;
			case 0x61: subl(); pc += 2; break;
			case 0x62: andl(); pc += 2; break;
			case 0x63: xorl(); pc += 2; break;
			case 0x64: mull(); pc += 2; break;
			case 0x70: jmp(); break;
			case 0x71: jle(); break;
			case 0x72: jl(); break;
			case 0x73: je(); break;
			case 0x74: jne(); break;
			case 0x75: jge(); break;
			case 0x76: jg(); break;
			case 0x80: call(); break;
			case 0x90: ret(); break;
			case 0xa0: pushl(); pc += 2; break;
			case 0xb0: popl(); pc += 2; break;
			case 0xc0: readb(); pc += 6; break;
			case 0xc1: readl(); pc += 6; break;
			case 0xd0: writeb(); pc += 6; break;
			case 0xd1: writel(); pc += 6; break;
			default: fprintf(stderr, "Execution stopped at pc = 0x%x. STATUS: INS\n" , pc); exit(1); break;
		}
	}
}

void halt()
{
	printf("\nExecution stopped at pc = 0x%x. STATUS: HLT\n", pc);
	exit(1);
}

void rrmovl()
{
	unsigned int nextByte = y86image[pc + 1];
	char * reg = getByteChars(nextByte);
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		registers[reg[1] - '0'] = registers[reg[0] - '0'];
	}
}

void irmovl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	if(reg[0] != 'f' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long immediate = getImmediateValue(pc + 2);
		registers[reg[1] - '0'] = immediate;
	}
}

void rmmovl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{	
		long displacement = getImmediateValue(pc + 2);
		*((int *) &y86image[displacement + registers[reg[1] -'0']]) = registers[reg[0] - '0']; 
	}
}

void mrmovl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{	
		long displacement = getImmediateValue(pc + 2);
		registers[reg[0] - '0'] = *((int *) &y86image[displacement + registers[reg[1] - '0']]);
	}
}

void addl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	OF = 0;
	SF = 0;
	ZF = 0;
	
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long long sum = registers[reg[1] - '0'] + registers[reg[0] - '0'];
		if((registers[reg[0]] > 0 && registers[reg[1]] > 0 && sum < 0) || 
		   (registers[reg[0]] < 0 && registers[reg[1]] < 0 && sum > 0) ||
		   (sum > (power(2, 31) - 1)) || (sum < (-1 * power(2,31))))
		{
			OF = 1;
			printf("Overflow Occurred at Instruction 0x%x\n", pc);
		}
		
		if(sum == 0)
		{
			ZF = 1;
		}
		else if(sum < 0)
		{
			SF = 1;
		}
		
		registers[reg[1] - '0'] = (long) sum;
	}
}

void subl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	ZF = 0;
	SF = 0;
	OF = 0;
	
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long long difference = registers[reg[1] - '0'] - registers[reg[0] - '0'];
		if((registers[reg[0]] < 0 && registers[reg[1]] > 0 && difference < 0) || 
		   (registers[reg[0]] > 0 && registers[reg[1]] < 0 && difference > 0) ||
		   (difference > power(2, 31) - 1) || (difference < (-1 * power(2,31))))
		{
			OF = 1;
			printf("Overflow Occurred at Instruction 0x%x\n", pc);
		}
		
		if(difference == 0)
		{
			ZF = 1;
		}
		else if(difference < 0)
		{
			SF = 1;
		}
		registers[reg[1] - '0'] = (long) difference;
	}
}

void andl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	ZF = 0;
	SF = 0;
	
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long and = registers[reg[1] - '0'] & registers[reg[0] -'0'];
		if(and == 0)
		{
			ZF = 1;
		}
		else if (and < 0)
		{
			SF = 1;
		}
		
		registers[reg[1] - '0'] = and;
	}	
}

void xorl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	ZF = 0;
	SF = 0;
	
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long xor = registers[reg[1] - '0'] ^ registers[reg[0] -'0'];
		if(xor == 0)
		{
			ZF = 1;
		}
		else if (xor < 0)
		{
			SF = 1;
		}
		
		registers[reg[1] - '0'] = xor;
	}
}

void mull()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	ZF = 0;
	SF = 0;
	OF = 0;
	
	if(reg[0] >= 'a' || reg[1] >= 'a')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long long product = registers[reg[0] - '0'] * registers[reg[1] - '0'];
		if((registers[reg[0]] > 0 && registers[reg[1]] > 0 && product < 0) ||
		   (registers[reg[0]] < 0 && registers[reg[1]] < 0 && product > 0) ||
		   (registers[reg[0]] < 0 && registers[reg[1]] > 0 && product > 0) ||
		   (registers[reg[0]] > 0 && registers[reg[1]] < 0 && product > 0) ||
		   product > power(2, 32) - 1 || product < (-1 * power(2, 31)))
		{
			OF = 1;
			printf("Overflow Occurred at Instruction 0x%x\n", pc);
		}
		
		if(product == 0)
		{
			ZF = 1;
		}
		else if(product < 0)
		{
			SF = 1;
		}
		
		registers[reg[1] - '0'] = (long) product;
	}
}

void jmp()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		pc = jmpAddr;
	}
}

void jle()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		if((SF ^ OF) | ZF)
			pc = jmpAddr;
		else
			pc += 5;
	}
}

void jl()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		if(SF ^ OF)
			pc = jmpAddr;
		else
			pc += 5;
	}
}

void je()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		if(ZF == 1)
			pc = jmpAddr;
		else
			pc += 5;
	}
}

void jne()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	//printf("Jump address: %x\n", jmpAddr);
	
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		if(ZF == 0)
			pc = jmpAddr;
		else
			pc += 5;
	}
}

void jge()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		if((SF^OF) == 0)
			pc = jmpAddr;
		else
			pc += 5;
	}
}

void jg()
{
	unsigned long jmpAddr = getAddressValue(pc + 1);
	if(jmpAddr < startOfInstructions || jmpAddr > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		if(~(SF ^ OF) & ~ZF)
			pc = jmpAddr;
		else
			pc += 5;
	}
}

void call()
{
	unsigned long address = getAddressValue(pc + 1);
	pc += 5;
	if(address < startOfInstructions || address > endOfInstructions)
	{
		printf("Incorrect instructions address.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		registers[4] -= 4;
		*((int *) &y86image[registers[4]]) = pc;
		pc = address;
	}
}

void ret()
{
	pc = *((int*)&y86image[registers[4]]);
	registers[4] += 4;
}

void pushl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	if(reg[0] >= 'a' || reg[1] != 'f')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		registers[4] -= 4;
		*((int*) &y86image[registers[4]]) = registers[reg[0] - '0'];
	}
}

void popl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	if(reg[0] >= 'a' || reg[1] != 'f')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		registers[reg[0] - '0'] = *((int *) &y86image[registers[4]]);
		registers[4] += 4;
	}
}

void readb()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	ZF = 0;
	
	if(reg[0] >= 'a' || reg[1] != 'f')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long immediate = getImmediateValue(pc + 2);
		char byte[3];
		byte[2] = '\0';
		if(scanf("%hhx", byte) != EOF)
		{
			long address = registers[reg[0] - '0'] + immediate;
			if(address < endOfInstructions)
			{
				printf("ERROR: Address coincides with data.\nExecution stopped at pc = 0x%x. STATUS: ADR\n");
				exit(1);
			}
			else
			{
				if(byte[0] > 'f' || byte[1] > 'f')
				{
					printf("ERROR: Incorrect input. Input must be a hex value from 00 - ff\n");
				}
				else
				{
					y86image[address] = hextodecUnsigned(byte);
					if(y86image[address] == 0)
					{
						ZF = 1;
					}
				}
			}
		}
		else
		{
			ZF = 1;
		}
	}
}

void readl()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	ZF = 0;
	
	if(reg[0] >= 'a' || reg[1] != 'f')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long immediate = getImmediateValue(pc + 2);
		unsigned long int word[32];
		word[31] = '\0';
		if(scanf("%ld", word) != EOF)
		{
			*((unsigned long int *) (&y86image[registers[reg[0] - '0'] + immediate])) = *word;
			if(*((unsigned long int *) (&y86image[registers[reg[0] - '0'] + immediate])) == 0 )
			{
				ZF = 1;
			}
		}
		else
		{
			ZF = 1;
		}
	}
}

void writeb()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	
	if(reg[0] >= 'a' || reg[1] != 'f')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long immediate = getImmediateValue(pc + 2);
		unsigned char byte = (unsigned char) y86image[registers[reg[0] - '0'] + immediate];
		printf("%c", byte);
	}
}

void writel()
{
	unsigned int registerBytes = y86image[pc + 1];
	char * reg = getByteChars(registerBytes);
	
	if(reg[0] >= 'a' || reg[1] != 'f')
	{
		printf("Incorrect Register Identifier.\nExecution stopped at pc = 0x%x. STATUS: ADR\n", pc);
		exit(1);
	}
	else
	{
		long immediate = getImmediateValue(pc + 2);
		unsigned long int word = *((int *) &y86image[registers[reg[0] - '0'] + immediate]);
		printf("%ld", word);
	}
}

//Converts hex string to an unsigned integer which will then be converted to a char
//and stored into the y86image
unsigned int hextobyte(char * byte)
{
	int i;
	unsigned int sum = 0;
	for(i = strlen(byte) - 1; i >= 0; i--)
	{
		char digit = byte[i];
		if(digit > '9')
		{
			sum += (digit - 'a' + 10) * power(16, strlen(byte) - (i+1)) ;
		}
		else
		{
			sum += (digit - '0') * power(16, strlen(byte) - (i+1));
		}
	}
	return sum;
}

//Converts the 32 bit address from little endian to the proper value
unsigned long getAddressValue(unsigned long tempPC)
{
	char bigEndian[9];
	
	int i;
	for(i = 0; i < 9; i++)
	{
		bigEndian[i] = '\0';
	}
	
	strcat(bigEndian, getByteChars(y86image[tempPC + 3]));
	strcat(bigEndian, getByteChars(y86image[tempPC + 2]));
	strcat(bigEndian, getByteChars(y86image[tempPC + 1]));
	strcat(bigEndian, getByteChars(y86image[tempPC]));
	return hextodecUnsigned(bigEndian);
}

//Converts the 32 bit value/displacement from little endian to the proper value 
long getImmediateValue(unsigned long tempPC)
{
	char bigEndian[9];
	
	int i;
	for(i = 0; i < 9; i++)
	{
		bigEndian[i] = '\0';
	}
	
	strcat(bigEndian, getByteChars(y86image[tempPC + 3]));
	strcat(bigEndian, getByteChars(y86image[tempPC + 2]));
	strcat(bigEndian, getByteChars(y86image[tempPC + 1]));
	strcat(bigEndian, getByteChars(y86image[tempPC]));
	return hextodecSigned(bigEndian);
}

//Converts a byte in unsigned int form to a char string that holds the chars
char * getByteChars(unsigned int byte)
{
	char hex[16]={"0123456789abcdef"};
	char * byteChar = calloc(1,3);
	
	int i = 1;
	while(byte)
	{
		byteChar[i] = hex[byte % 16];
		byte = byte/16;
		i--;
	}
	
	if(byteChar[0] == '\0')
	{
		byteChar[0] = '0';
	}
	
	if(byteChar[1] == '\0')
	{
		byteChar[1] = '0';
	}
	
	return byteChar;
}

//Converts hex string to a signed decimal long
long hextodecSigned(char * num)
{
	//convert to binary first to check for two's complement
	char* bDigits[16] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};
	
	int i, isNeg = 0;
	char* binary = (char*) calloc(1,strlen(num) * 4 + 1);
	for(i = 0; i < strlen(num); i++)
	{
		char digit = num[i];
		if(digit > '9')
		{
			strcat(binary, bDigits[digit - 'a' + 10]) ;
		}
		else
		{
			strcat(binary, bDigits[digit - '0']);
		}
	}
	
	if(binary[0] == '1')
	{
		twoscomplement(binary);
		isNeg = 1;
	}
	
	//convert from binary to decimal
	int j;
	long sum = 0;
	for(j = strlen(binary) - 1; j >= 0; j--)
	{
		sum += (binary[j] - '0') * power(2, strlen(binary) - (j+1));
	}		
	
	if(isNeg == 1)
		return -1 * sum;
	else
		return sum;
}

//Converts hex string to an unsigned decimal long
unsigned long hextodecUnsigned(char * num)
{
	//printf("NUM: %s\n", num);
	int j;
	unsigned long sum = 0;
	for(j = strlen(num) - 1; j >= 0; j--)
	{
		char digit = num[j];
		if(digit > '9')
		{
			sum += (digit - 'a' + 10) * power(16, strlen(num) - (j+1)) ;
		}
		else
		{
			sum += (digit - '0') * power(16, strlen(num) - (j+1));
		}
	}	
	
	return sum;
}

//Converts the number in twoscomplement notation to regular form 
void twoscomplement(char * number)
{
	//Subtract one from the number
	int i;
	for(i = strlen(number) - 1; i >= 0; i--)
	{
		if(number[i] == '1')
		{
			number[i] = '0';
			break;
		}
		else if(number[i] == '0')
		{
			number[i] = '1';
		}
		else
		{
			printf("NaN\n");
			exit(1);
		}
	}
	
	//Invert all the digits
	int x;
	for(x = 0; x < strlen(number); x++)
	{
		if(number[x] == '0')
		{
			number[x] = '1';
		}
		else if(number[x] == '1')
		{
			number[x] = '0';
		}
		else
		{
			printf("NaN\n");
			exit(1);
		}
	}
}

//Performs the math power function (num ^ exponent)
double power(int num, int exponent)
{
	if(exponent == 0)
	{
		return 1.0;
	}
	if(exponent > 0) //positive exponent
	{
		if(exponent == 1)
		{
			return num;
		}

		int i;
		double number = num;
		for(i = 2; i <= exponent; i++)
		{
			number *= num;
		}
		return number;
	}
	else //negative exponent
	{
		exponent = exponent * -1;
		
		if(exponent == 1)
		{
			return 1.0/num;
		}
		
		int i;
		double denominator = (double) num;
		for(i = 2; i <= exponent; i++)
		{
			denominator *= num;
		}
		double number = 1/denominator;
		return number;
	}
}
