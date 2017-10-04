/*
    Copyright (C) 2015, 2017 Nicolas Sauzede <nsauzede@laposte.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIG 10
#define MAXNAME 20

typedef struct sig_t {
	char name[MAXNAME], title[MAXNAME];
	int value;
} _sig_t;

int nsig = 0;
struct sig_t sigs[MAXSIG];

int nsamp = 0;

int curtime = 0;
int lastime = -1;

int bin2int( char *s)
{
	int i, val = 0;
	int n = strlen( s);
	
	for (i = 0; i < n; i++)
	{
		val += (s[i] - '0') << (n - i - 1);
	}
	
	return val;
}

int main( int argc, char *argv[])
{
	FILE *in;
	char *fname = NULL;

	if (argc > 1)
	{
		fname = argv[1];
	}
	
	if (!fname)
	{
		printf( "Usage : %s filename.vcd\n", argv[0]);
		exit( -1);
	}
	in = fopen( fname, "rt");
	if (!in)
	{
		perror( "reading input file\n");
		exit( -1);
	}
	while (!feof( in))
	{
		static int state = 0;
		int i;
		char buf[1024], token[1024];
		
		fgets( buf, sizeof( buf), in);
		if (sscanf( buf, "%s", token) != 1)
		{
			if (state >= 2)
			{
				if (lastime != curtime)
				{
					lastime = curtime;
				printf( "time %6d:\t", curtime);
				for (i = 0; i < nsig; i++)
				{
					printf( "%02X ", sigs[i].value);
				}
				printf( "\n");
				}
			}
			
			continue;
		}
		if (!strcmp( token, "$var"))
		{
			char name[1024], title[1024];
			
			sscanf( buf, "%*s %*s %*s %s %s %*s", name, title);
			if (nsig < MAXSIG) {
				strncpy( sigs[nsig].name, name, sizeof( sigs[nsig].name));
				strncpy( sigs[nsig].title, title, sizeof( sigs[nsig].title));
				printf("added sig %d %s %s\n", nsig, name, title);
				nsig++;
			}
		}
		else if (!strcmp( token, "$dumpvars"))
		{
			state = 2;
		}
		else if (!strcmp( token, "$end"))
		{
			if (state == 2)
			{
				printf( "Found %d sigs :\t", nsig);
				for (i = 0; i < nsig; i++)
				{
//					printf( "%s ", sigs[i].name);
					printf( "%s ", sigs[i].title);
				}
				printf( "\n");
				curtime = 0;
				state = 3;
			}
		}
		else
		{
			if (state >= 2)
			{
				int val;
				char name[1024];
				
				if (token[0] == '#')
				{
					sscanf( token, "%*c%d", &curtime);
				}
				if (token[0] == 'b')
				{
					char v[1024];
					
					sscanf( buf, "b%s %s", v, name);
					val = bin2int( v);
//					printf( "Got name=%s bval=%d token=[%s]\n", name, val, token);
				}
				else
				{
					sscanf( token, "%1d%s", &val, name);
//					printf( "Got name=%s val=%d\n", name, val);
				}
				for (i = 0; i < nsig; i++)
				{
					if (!strcmp( sigs[i].name, name))
						sigs[i].value = val;
				}
			}
		}
	}
	fclose( in);
	
	return 0;
}
