// Engineuci.cpp
//
// Copyright (C) 2008-2013, ir. R.L. Pijl

#include <stdio.h>

#include "engineuci.h"
#include "util.h"

UCIEngine::UCIEngine()
{
	searchDepth=-1;
	searchTime=-1;
	levelMoves=40;
	levelSeconds=300;
	levelInc=0;

	timeOwnRemaining=300000;
	timeOppRemaining=300000;

	strcpy(fenPosition,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w kqKQ -");
}

UCIEngine::~UCIEngine()
{
}

bool
UCIEngine::InitEngine(void)
{
	char buf[2048];

	if (!started) {
		errorNumber=ENGINENOTSTARTED;
		return true;
	}

	fprintf(toengine,"uci\n");
	do {
		fgets(buf,2047,fromengine);
		if (strncmp(buf,"copyprotection error",20)==0) {
			errorNumber=ENGINECOPYPROT;
			return true;
		}
		if (strncmp(buf,"id",2)==0) {
			char *s;
			s=strtok(buf," \t\n\r");
			s=strtok(0," \t\n\r");
			if (strncmp(s,"name",4)==0) {
				s=strtok(0," \t\n\r");
				// engine name is now in s
				// TODO: do something with it
				continue;
			}
			if (strncmp(s,"author",6)==0) {
				s=strtok(0," \t\n\r");
				// author name is now in s
				// TODO: do something with it
				continue;
			}
			// protocol error, ignore
			continue;
		}
		if (strncmp(buf,"option", 6)==0) {
			// Option sent.
			char *s;
			//printf(buf);
			s=strtok(buf," \t\n\r");
			s=strtok(0," \t\n\r");

			// ponder, check
			if (strncmp(s,"ponder",6)==0) {
				// engine is obvious able to ponder.
				optionPonder=true;
				s=strtok(0," \t\n\r"); // type
				s=strtok(0," \t\n\r"); // check
				s=strtok(0," \t\n\r"); // default
				s=strtok(0," \t\n\r");
				if (strncmp(s,"true",4)==0)
					ponderMode=true;
				else
					ponderMode=false;
				continue;
			}

			// multipv, spin
			if (strncmp(s,"multipv",7)==0) {
				// engine is capable of multipv.
				optionMultiPV=true;
				s=strtok(0," \t\n\r"); // type
				s=strtok(0," \t\n\r"); // spin
				while(s=strtok(0," \t\n\r")) {
					if (strncmp(s,"min",3)) {
						multiPVmin=atoi(strtok(0,"\t\n\r"));
						continue;
					}
					if (strncmp(s,"max",3)) {
						multiPVmax=atoi(strtok(0,"\t\n\r"));
						continue;
					}
					if (strncmp(s,"default",7)) {
						multiPV=atoi(strtok(0,"\t\n\r"));
						continue;
					}
				}
				continue;
			}

			// UCI_ShowCurrLine, check
			if (strncmp(s,"UCI_ShowCurrLine",16)==0) {
				// engine is able to show the current line.
				optionShowCurrline=true;
				continue;
			}

			// UCI_ShowRefutations, check
			if (strncmp(s,"UCI_ShowRefutations",19)==0) {
				// engine is able to show refutations.
				optionShowRefute=true;
				continue;
			}

			// UCI_AnalyseMode, check
			if (strncmp(s,"UCI_AnalyseMode",15)==0) {
				// engine wants to be told about analysis mode.
				optionAnalyzeMode=true;
				continue;
			}

			continue;
		}
	} while (strncmp(buf,"uciok", 5)!=0);

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetOption(const char* id, const char* value)
{
	fprintf(toengine,"setoption name %s", id);
	if (value) fprintf(toengine," value %s",value);
	fprintf(toengine,"\n");
	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetPosition(const char * fen)
{
	if (!fen) {
		errorNumber=ENGINENOPOS;
		return true;
	}
	strcpy(fenPosition,fen);

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetSearchDepth(int depth)
{
	levelMoves=-1;
	searchDepth=depth;

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetSearchTime(int seconds)
{
	levelMoves=-1;
	searchTime=seconds;

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetSearchLevel(int moves, int seconds, int inc)
{
	searchDepth=-1;
	searchTime=-1;
	levelMoves=moves;
	levelSeconds=seconds;
	levelInc=inc;

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetTimeRemaining(int milliseconds)
{
	timeOwnRemaining=milliseconds;

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::SetOppTimeRemaining (int milliseconds)
{
	timeOppRemaining=milliseconds;

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::Search(int mode, searchPVFunction pvf, searchFRFunction frf, searchCMFunction cmf, searchRefFunction rf, searchStrFunction sf, int move)
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
	fprintf(toengine,"position fen %s\n",fenPosition);
	if (searchTime>0)
		fprintf(toengine,"go movetime %d000\n",searchTime);
	else
		return true;

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::ResponseThread(void)
{
	char buf[2048];
	char *s;
	// analyze the engine responses
	do {
		fgets(buf,2047,fromengine);
		//cout << buf << endl;
		s=strtok(buf," \t\n\r");
		if (!s) continue;
		if (strncmp(s,"bestmove",8)==0) {
			int move, pmove=0;
			// send final report and exit
			s=strtok(0," \n\r\t");
			move=ParseMove(s);
			s=strtok(0," \n\r\t");
			if (s) {
				s=strtok(0," \n\r\t");
				pmove=ParseMove(s);
			}
			if (finHandler(move,pmove)) {
				searching=false;
				return true;
			}
			searching=false;
			return false;
		}
		if (strncmp(s, "info", 4)==0) {
			int depth=-1, seldepth=-1, multi=0, score=0, time=-1, nodes=-1, tbhits=-1, hashfull=0;
			while (s=strtok(0," \n\r\t")) {
				if (strncmp(s,"multi",5)==0) {
					s=strtok(0," \n\r\t");
					multi=atoi(s);
					continue;
				}
				if (strncmp(s,"depth",5)==0) {
					s=strtok(0," \n\r\t");
					depth=atoi(s);
					continue;
				}
				if (strncmp(s,"seldepth",8)==0) {
					s=strtok(0," \n\r\t");
					seldepth=atoi(s);
					continue;
				}
				if (strncmp(s,"score",5)==0) {
					s=strtok(0," \n\r\t"); // type of score
					if (strncmp(s,"cp",2)==0) {
						s=strtok(0," \n\r\t"); // type of score
						score=atoi(s);
						continue;
					}
					if (strncmp(s,"mate",4)==0) {
						s=strtok(0," \n\r\t"); // type of score
						score=atoi(s);
						if (score>0)
							score=100000-score;
						if (score<0)
							score=-100000-score;
						continue;
					}
					continue;
				}
				if (strncmp(s,"time",4)==0) {
					s=strtok(0," \n\r\t");
					time=atoi(s);
					continue;
				}
				if (strncmp(s,"nodes",5)==0) {
					s=strtok(0," \n\r\t");
					nodes=atoi(s);
					continue;
				}
				if (strncmp(s,"tbhits",6)==0) {
					s=strtok(0," \n\r\t");
					tbhits=atoi(s);
					continue;
				}
				if (strncmp(s,"hashfull",8)==0) {
					s=strtok(0," \n\r\t");
					hashfull=atoi(s);
					continue;
				}
				if (strncmp(s,"pv",2)==0) {
					s=strtok(0,"\n\r");
					if (pvHandler) pvHandler(multi, depth,seldepth,score,time,nodes,tbhits,hashfull,s);
					break;
				}
				// unknown, just continue;
			}
		}
		
	} while (1);
	return false;
}

bool
UCIEngine::SearchStop(void)
{
	if (!started) {
		errorNumber=ENGINENOTSTARTED;
		return true;
	}

	if (!searching) {
		errorNumber=ENGINENOSEARCH;
		return true;
	}

	fprintf(toengine,"stop\n");

	// response will follow via the registered FRhandler

	errorNumber=ENGINEOK;
	return false;
}

bool
UCIEngine::Stop(void)
{
	unsigned int i=0;
	fprintf(toengine,"quit\n");

	return WaitForStop();
}
