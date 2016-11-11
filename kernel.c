void printString(char*);
void readString(char*);
void readSector(char*,int);
void writeSector(char*, int);
void handleInterrupt21(int,int,int,int);
void readFile(char*,char*);
void executeProgram(char*,int);
void deleteFile(char*);
int div(int,int);
int mod(int,int);

int main()
{
	/*   Project B   */
	/*   Part 2:
	char line[80];
	printString("Enter a line: \0");
	readString(line);
	printString(line);*/

	/*   Part 3: 
	char buffer[512];
	readSector(buffer, 30);
	printString(buffer);*/
	
	/*   Part 4:
	makeInterrupt21();
	interrupt(0x21,0,0,0,0);*/

	/*   Part 5:
	char line[80];
	makeInterrupt21();
	interrupt(0x21, 1, line, 0, 0);
	interrupt(0x21, 0, line, 0, 0);*/

	/*   Project C   */
	/*   Part 1:
	char buffer[13312];
	makeInterrupt21();
	interrupt(0x21, 3, "messag\0", buffer, 0);
	interrupt(0x21, 0, buffer, 0, 0);*/

	/*   Part 2:
	makeInterrupt21();
	interrupt(0x21, 4, "tstprg\0", 0x2000, 0);*/

	/*   Part 4:   */
	makeInterrupt21();
	interrupt(0x21, 4, "shell\0", 0x2000, 0);

	while(1);
}

void printString(char* chars)
{
	int i = 0;
	while( chars[i] != 0 )
	{
		interrupt(0x10, 0xe*256+chars[i], 0, 0, 0);
		i++;
	}
}

void readString(char* chars)
{
	int i = 0;
	char inp;
	inp = interrupt(0x16, 0, 0, 0, 0);
	while( inp != 0xd )
	{
		if( inp == 0x8 && i > 0)
		{
			i--;
			chars[i] = ' ';
			interrupt(0x10, 0xe*256+'\b', 0, 0, 0);
			interrupt(0x10, 0xe*256+' ', 0, 0, 0);
			interrupt(0x10, 0xe*256+'\b', 0, 0, 0);
			inp = interrupt(0x16, 0, 0, 0, 0);
		}
		else
		{
			chars[i] = inp;
			interrupt(0x10, 0xe*256+inp, 0, 0, 0);
			inp = interrupt(0x16, 0, 0, 0, 0);
			i++;
		}
	}
	chars[i] = 0xa;
	chars[++i] = 0x0;
	interrupt(0x10, 0xe*256+'\r', 0, 0, 0);
	interrupt(0x10, 0xe*256+'\n', 0, 0, 0);
}

void readSector(char* buffer, int sector)
{	
	int AL = 1;
	int AH = 2;
	int AX = AH * 256 + AL;
	int CL = mod(sector, 18) + 1;
	int CH = div(sector, 36);
	int CX = CH * 256 + CL;
	int DL = 0;
	int DH = mod(div(sector, 18), 2);
	int DX = DH * 256 + DL;

	interrupt(0x13, AX, buffer, CX, DX);
}

void writeSector(char* buffer, int sector)
{	
	int AL = 1;
	int AH = 3;
	int AX = AH * 256 + AL;
	int CL = mod(sector, 18) + 1;
	int CH = div(sector, 36);
	int CX = CH * 256 + CL;
	int DL = 0;
	int DH = mod(div(sector, 18), 2);
	int DX = DH * 256 + DL;

	interrupt(0x13, AX, buffer, CX, DX);
}

void readFile(char* fileName, char* fileBuffer)
{
	char dirBuffer[512];
	int i = 0;
	int j = 0;
	int firstSector = 0;

	/*interrupt(0x21, 2, dirBuffer, 2, 0);*/
	readSector(dirBuffer, 2);

	while( i < 512 )
	{
		while( j < 6)
		{
			if( fileName[j] != dirBuffer[i+j] )
			{
				break;
			}
			if( j == 5 )
			{
				printString("file found\n\r");
				firstSector = i + 6;
			}
			j++;
		}
		i += 32;
		j = 0;
	}

	/*   File not found   */
	if( firstSector == 0 )
	{
		printString("file not found");
		return;
	}

	i = 0;
	while( i < 26 && dirBuffer[firstSector + i] != 0 )
	{
		readSector(fileBuffer + (i * 512), dirBuffer[firstSector + i]);
		i++;
	}
}

void deleteFile(char* fileName)
{
	char mapBuffer[512];
	char dirBuffer[512];
	int i = 0;
	int j = 0;
	int firstSector;

	readSector(mapBuffer, 1);
	readSector(dirBuffer, 2);

	while( i < 512 )
	{
		while( j < 6)
		{
			if( fileName[j] != dirBuffer[i+j] )
			{
				break;
			}
			if( j == 5 )
			{
				printString("file found\n\r");
				firstSector = i + 6;
				dirBuffer[i] = 0x00;
			}
			j++;
		}
		i += 32;
		j = 0;
	}

	/*   File not found   */
	if( firstSector == 0 )
	{
		printString("file not found");
		return;
	}

	i = 0;
	while( i < 26 && dirBuffer[firstSector + i] != 0 )
	{
		mapBuffer[dirBuffer[firstSector + i]] = 0x00;
		i++;
	}

	writeSector(mapBuffer, 1);
	writeSector(dirBuffer, 2);
}

void executeProgram(char* name, int segment)
{
	char buffer[13312];
	int i = 0;
	readFile(name, buffer);

	while( i < 13312 )
	{
		putInMemory(segment, i, buffer[i]);
		i++;
	}

	launchProgram(segment);
}

void terminate()
{
	char shell[6];
	shell[0] = 's';
	shell[1] = 'h';
	shell[2] = 'e';
	shell[3] = 'l';
	shell[4] = 'l';
	shell[5] = '\0';

	executeProgram(shell, 0x2000);
	/*interrupt(0x21, 4, shell, 0x2000, 0);*/
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
	if( AX == 0 )
	{
		printString(BX);
	}
	else if( AX == 1 )
	{
		readString(BX);
	}
	else if( AX == 2 )
	{
		readSector(BX, CX);
	}
	else if( AX == 3 )
	{
		readFile(BX, CX);
	}
	else if( AX == 4 )
	{
		executeProgram(BX, CX);
	}
	else if( AX == 5 )
	{
		terminate();
	}
	else if( AX == 6 )
	{
		writeSector(BX, CX);
	}
	else
	{
		printString("error");
	}
}

int div(int a, int b)
{
	int q = 0;
	while( a > (q * b) )
	{
		q++;
	}
	return --q;
}

int mod(int a, int b)
{
	while( a >= b )
	{
		a = a - b;
	}
	return a;
}
