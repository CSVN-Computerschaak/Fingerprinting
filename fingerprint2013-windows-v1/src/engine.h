// Engine.h
//
// Copyright (C) 2008-2013, ir. R.L. Pijl

#ifndef __ENGINE_H
#define __ENGINE_H

#include <windows.h>

DWORD WINAPI startResponseThread(LPVOID lpParam);

class Engine
{
public:
	Engine();
	virtual ~Engine();

	bool SetWorkingDir(const char* wdir);
	bool SetExecName(const char* exec);
	bool StartEngine(void);

	virtual bool InitEngine(void)=0;
	virtual bool SetOption(const char* id, const char* value);

	virtual bool Synchronize(void);

	virtual bool SetPosition(const char * fen)=0;

	virtual bool SetSearchDepth(int depth)=0;
	virtual bool SetSearchTime(int seconds)=0;
	virtual bool SetSearchLevel(int moves, int seconds, int inc)=0;

	virtual bool SetTimeRemaining(int milliseconds)=0;
	virtual bool SetOppTimeRemaining (int milliseconds)=0;

	typedef bool (*searchPVFunction)(int multi, int depth, int seldepth, int score, int time, int nodes, int tbhits, int hashfull, const char* pv);
	typedef bool (*searchFRFunction)(int bestmove, int pondermove);
	typedef bool (*searchCMFunction)(int currmovenr, int currmove, const char* currline);
	typedef bool (*searchRefFunction)(int refmove, const char* refline);
	typedef bool (*searchStrFunction)(const char* str);

	// Search main function. Output is communicated back via handler functions.
	// If a handler function is provided, the options needed to get output for it
	// is automatically set as well (when supported by the engine).
	// SearchFRFunction is compulsory as is marks the end of the search,
	// rest may be null.

	virtual bool Search(int mode, searchPVFunction, searchFRFunction, searchCMFunction, searchRefFunction, searchStrFunction, int move=0)=0;
	virtual bool SearchStop(void)=0;

	virtual bool Stop(void)=0;
	bool WaitForStop(void);

	int GetError(void);
	const char* GetErrorStr();

	friend DWORD WINAPI startResponseThread(LPVOID lpParam);

	typedef enum {
		ENGINEOK=0,	ENGINEOTHERR, ENGINENODIR, ENGINENOMEM, ENGINENONAME,
		ENGINECMDPIPE, ENGINERESPIPE, ENGINEPIPEIN, ENGINEPIPEOUT, ENGINESTDIN,
		ENGINESTDOUT, ENGINEPROCSTART, ENGINEFDCMDPIPE, ENGINEFDRESPIPE, ENGINENOTERM,
		ENGINECOPYPROT, ENGINENOPOS, ENGINEALREADYSTARTED, ENGINENOTSTARTED, ENGINENOSEARCH,
		ENGINEALREADYSEARCH, ENGINENORESPTHREAD, ENGINENOFRF, ENGINENOTSUPP, ENGINEILLPOS,
		ENGINENOOPT
	} err_t;

	typedef enum {
		searchMove=0,
		searchAnalyse,
		searchPonder
	} mode_t;

	typedef enum {
		searchInfoNone=0,
		searchInfoInformative,
		searchInfoFinal
	} info_t;

protected:

	FILE* toengine, *fromengine;
	int errorNumber;
	bool started;
	bool searching;

	bool optionPonder;
	bool ponderMode;

	bool optionMultiPV;
	int multiPV;
	int multiPVmin;
	int multiPVmax;

	bool optionShowCurrline;
	bool showCurrline;

	bool optionShowRefute;
	bool showRefutation;

	bool optionAnalyzeMode;

	searchPVFunction pvHandler;
	searchFRFunction finHandler;
	searchCMFunction cmHandler;
	searchRefFunction refHandler;
	searchStrFunction strHandler;

	virtual bool ResponseThread(void)=0;

private:

	char* engineWorkingDir;
	char* engineExecName;

	int enginepipe[2];
	int enginerespipe[2];
	intptr_t engineid;

	static const char *errorStrings[100];
	enum { READ=0, WRITE };
};

#endif