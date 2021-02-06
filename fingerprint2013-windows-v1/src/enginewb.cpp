// Enginewb.cpp
//
// Copyright (C) 2008-2013, ir. R.L. Pijl

#include <stdio.h>
#include "enginewb.h"
#include "util.h"

static int pingseq=1;

WBEngine::WBEngine()
{
	myname[0]='\0';
	fping=false;
	fsetboard=false;
	fplayother=false;
	fsan=false;
	fusermove=false;
	ftime=true;
	fdraw=true;
	fsigint=true;
	fsigterm=true;
	freuse=true;
	fanalyze=true;
	for (int i=0; i<16; i++) fvariants[i]=true;
	fcolors=true;
	fics=false;
	fname=true;
	fpause=false;
}

WBEngine::~WBEngine()
{
}

bool
WBEngine::InitEngine(void)
{
	char buf[2048];
	char *s;
	int timeout=2;

	if (!started) {
		errorNumber=ENGINENOTSTARTED;
		return true;
	}

	fprintf(toengine,"xboard\n");
	fgets(buf,2047,fromengine); 
	if (strncmp(buf,"feature done=0", 14)==0) timeout=3600;
	fprintf(toengine,"protover 2\n");

	// TODO: start 2 seconds timer
	do {
		fgets(buf,2047,fromengine); // TODO: make interuptable by timer
		s=strtok(buf," \n\r\t");
		if (!s) continue;
		if (strncmp(s,"feature",7) != 0)
			continue;
		s=strtok(0,"= \n\r\t");
		if (!s) continue;

		if (strncmp(s,"done",4)==0) {
			fprintf(toengine,"accepted done\n");
			s=strtok(0," \n\r\t");
			if (!s) continue;
			if (*s=='0') {
				timeout=3600;
				continue;
			}
			if (*s=='1') break;
			continue;
		}

		if (strncmp(s,"ping",4)==0) {
			fprintf(toengine,"accepted ping\n");
			s=strtok(0," \n\t\r");
			if (!s) continue;
			fping=(*s=='1');
			continue;
		}

		if (strncmp(s,"setboard",8)==0) {
			fprintf(toengine,"accepted setboard\n");
			s=strtok(0," \n\t\r");
			if (!s) continue;
			fsetboard=(*s=='1');
			continue;
		}

		if (strncmp(s,"usermove",8)==0) {
			fprintf(toengine,"accepted usermove\n");
			s=strtok(0," \n\t\r");
			if (!s) continue;
			fusermove=(*s=='1');
			continue;
		}

		if (strncmp(s,"san",3)==0) {
			fprintf(toengine,"accepted san\n");
			s=strtok(0," \n\t\r");
			if (!s) continue;
			fsan=(*s=='1');
			continue;
		}

		// TODO: set remaining features
		fprintf(toengine,"accepted %s\n",s);

	} while (1);

	// Finalize initialization by sending initial commands to setup the engine
	fprintf(toengine,"new\n");
	fprintf(toengine,"post\n");
	fprintf(toengine,"easy\n");
	Synchronize();

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::Synchronize(void)
{
	char buf[2048];

	if (fping) {
		fprintf(toengine,"ping %d\n",pingseq++);
		do {
			fgets(buf,2047,fromengine);
		} while (strncmp(buf,"pong",4));
	}

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::SetPosition(const char * fen)
{
	char buf[2048];

	if (!fen) {
		errorNumber=ENGINENOPOS;
		return true;
	}

	if (fsetboard&&fping) {
		fprintf(toengine,"setboard %s\n", fen);
		fprintf(toengine,"ping %d\n",pingseq++);
		do {
			fgets(buf,2047,fromengine);
			if (strncmp(buf,"Error",5)==0) {
				if (strstr(buf,"setboard")) {
					errorNumber=ENGINEILLPOS;
					return true;
				}
				continue;
			}
		} while (strncmp(buf,"pong",4));

		errorNumber=ENGINEOK;
		return false;
	}

	// TODO: Implementation of 'edit'
	errorNumber=ENGINENOTSUPP;
	return true;
}

bool
WBEngine::SetSearchDepth(int depth)
{
	fprintf(toengine,"sd %d\n",depth);
	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::SetSearchTime(int seconds)
{
	fprintf(toengine,"st %d\n",seconds);
	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::SetSearchLevel(int moves, int seconds, int inc)
{
	if (seconds%60)
		fprintf(toengine,"level %d %d:%d %d\n",moves,seconds/60,seconds%60,inc);
	else
		fprintf(toengine,"level %d %d %d\n",moves,seconds/60,inc);

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::SetTimeRemaining(int milliseconds)
{
	if (ftime) {
		fprintf(toengine,"time %d\n",milliseconds/10);
	}

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::SetOppTimeRemaining (int milliseconds)
{
	if (ftime) {
		fprintf(toengine,"otim %d\n",milliseconds/10);
	}

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::Search(int mode, searchPVFunction pvf, searchFRFunction frf, searchCMFunction cmf, searchRefFunction rf, searchStrFunction sf, int move)
{
    DWORD dwThreadId;
    HANDLE hThread; 

	if (!started) {
		errorNumber=ENGINENOTSTARTED;
		return true;
	}

	if (searching) {
		errorNumber=ENGINEALREADYSEARCH;
		return true;
	}

	// register handler functions.
	if (frf==0) {
		errorNumber=ENGINENOFRF;
		return true;
	}
	finHandler=frf;
	// rest is optional
	pvHandler=pvf;
	cmHandler=cmf;
	refHandler=rf;
	strHandler=sf;

	// create thread with result reading loop
	hThread=CreateThread(NULL, 0, startResponseThread, this, 0, &dwThreadId);
	if (hThread==0) {
		errorNumber=ENGINENORESPTHREAD;
		return true;
	}
	// start search
	searching=true;
	fprintf(toengine,"go\n");

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::ResponseThread(void)
{
	char buf[2048];
	char *s;
	// analyze the engine responses
	do {
		fgets(buf,2047,fromengine);
		//cout << buf << endl;
		s=strtok(buf," \t\n\r");

		if (strncmp(s,"move",4)==0) {
			// played a move, call finHandler
			int move;
			s=strtok(0," \t\n\r");
			move=ParseMove(s);
			if (finHandler) finHandler(move,0);
			break;
		}

		// TODO: include other defined responses that could be sent here

		if (isdigit(*s)) {
			// TODO
			// probably a PV, check if it is
			int depth, score, time, nodes;
			s=strtok(0," \t\n\r"); depth=atoi(s);
			s=strtok(0," \t\n\r"); score=atoi(s);
			s=strtok(0," \t\n\r"); time=atoi(s);
			s=strtok(0," \t\n\r"); nodes=atoi(s);
			s=strtok(0,"\n\r");

			// call pvhandler
			if (pvHandler) pvHandler(0,depth,-1,score,time,nodes,-1,0,s);
			continue;
		}

	} while (1);
	searching=false;
	return false;
}

bool
WBEngine::SearchStop(void)
{
	if (!started) {
		errorNumber=ENGINENOTSTARTED;
		return true;
	}

	if (!searching) {
		errorNumber=ENGINENOSEARCH;
		return true;
	}

	fprintf(toengine,"?\n");

	// response will follow via the registered FRhandler

	errorNumber=ENGINEOK;
	return false;
}

bool
WBEngine::Stop(void)
{
	unsigned int i=0;
	fprintf(toengine,"quit\n");

	return WaitForStop();
}
