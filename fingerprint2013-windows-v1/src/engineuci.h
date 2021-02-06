// EngineUCI.h
//
// Copyright (C) 2008-2013, ir. R.L. Pijl

#ifndef __ENGINEUCI_H
#define __ENGINEUCI_H

#include "engine.h"

class UCIEngine : public Engine
{
public:
	UCIEngine();
	virtual ~UCIEngine();

	virtual bool InitEngine(void);

	virtual bool SetOption(const char* id, const char* value);
	virtual bool SetPosition(const char * fen);

	virtual bool SetSearchDepth(int depth);
	virtual bool SetSearchTime(int seconds);
	virtual bool SetSearchLevel(int moves, int seconds, int inc);

	virtual bool SetTimeRemaining(int milliseconds);
	virtual bool SetOppTimeRemaining (int milliseconds);

	virtual bool Search(int mode, Engine::searchPVFunction, Engine::searchFRFunction, Engine::searchCMFunction, Engine::searchRefFunction, Engine::searchStrFunction, int move=0);
	virtual bool SearchStop(void);

	virtual bool Stop(void);

protected:

	virtual bool ResponseThread(void);

private:

	int searchDepth;
	int searchTime;
	int levelMoves;
	int levelSeconds;
	int levelInc;

	int timeOwnRemaining;
	int timeOppRemaining;

	char fenPosition[128];

};


#endif // __ENGINEUCI_H