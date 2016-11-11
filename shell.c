void parse( char* );
int compareStr( char*, char* );

int main()
{
	char command[80];
	while(1)
	{
		interrupt( 0x21, 0, "SHELL: \0", 0, 0 );
		interrupt( 0x21, 1, command, 0, 0 );
		interrupt( 0x21, 0, "\n\r", 0, 0 );
		parse( command );
	}

	while(1);
}

void parse( char* commandBuffer )
{
	char command[80];
	char arg[80];
	char buffer[512];
	int i = 0;
	int j = 0;

	while( commandBuffer[i] != ' ' )
	{
		command[i] = commandBuffer[i];
		i++;
	}
	command[i] = 0;
	i++;
	while( commandBuffer[i] != 0 )
	{
		arg[j] = commandBuffer[i];
		i++;
		j++;
	}
	arg[i] = 0;

	if( !compareStr( command, "type\0" ) )
	{
		interrupt( 0x21, 3, arg, buffer, 0 );
		interrupt( 0x21, 0, buffer, 0, 0 );
	}
	else if( !compareStr( command, "execute\0" ) )
	{
		interrupt( 0x21, 4, arg, 0x2000, 0 );
	}
	else if( !compareStr( command, "delete\0" ) )
	{
		interrupt( 0x21, 7, arg, 0 , 0 );
	}
	else
	{
		interrupt( 0x21, 0, "Bad command", 0, 0 );
	}
	return;
}

int compareStr( char* a, char* b )
{
	int i = 0;
	while( a[i] == b[i] )
	{
		if( a[i] == '\0' )
		{
			return 0;
		}
		i++;
	}
	return a[i] - b[i];
}
