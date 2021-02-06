// Util.cpp
// Copyright (C) 2008-2013, ir. R.L. Pijl

#include "util.h"
#include "position.h"

int ParseMove(const char* s)
{
	// lean and mean, no error checking: TODO!
	int m;

	if (strncmp(s,"0000",4)==0) return 0;

	m=0;
	m+=s[0]-'a';
	m+=(s[1]-'1') << 3;
	m+=(s[2]-'a') << 6;
	m+=(s[3]-'1') << 9;
	switch (s[4]) {
		case 'q':
			m+=1<<12;
			break;
		case 'r':
			m+=2<<12;
			break;
		case 'b':
			m+=3<<12;
			break;
		case 'n':
			m+=4<<12;
			break;
		case '\0':
		default:
			break;
	}
	return m;
}

static char pChar[5] = { '\0', 'q', 'r', 'b', 'n' };
const char* MoveStr(int m)
{
	static char buf[6];
	int i=0;

	buf[0]=(m&7)+'a';
	m>>=3;
	buf[1]=(m&7)+'1';
	m>>=3;
	buf[2]=(m&7)+'a';
	m>>=3;
	buf[3]=(m&7)+'1';
	m>>=3;
	buf[4]=pChar[m&7];
	buf[5]='\0';
	return buf;
}

/*
int incheck(const char* fen)
{
	Position p;
	p.SetFEN(fen);
	if (p.Black2Move() && p.WhiteAttacks(p.blackKing)) return 1;
	if (p.White2Move() && p.BlackAttacks(p.whiteKing)) return 1;
	return 0;
}*/