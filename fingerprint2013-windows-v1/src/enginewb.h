// EngineWB.h
//
// Copyright (C) 2008-2013, ir. R.L. Pijl

#ifndef __ENGINEWB_H
#define __ENGINEWB_H

#include "engine.h"

class WBEngine : public Engine
{
public:
	WBEngine();
	virtual ~WBEngine();

	virtual bool InitEngine(void);

	virtual bool Synchronize(void);

	virtual bool SetPosition(const char * fen);

	virtual bool SetSearchDepth(int depth);
	virtual bool SetSearchTime(int seconds);
	virtual bool SetSearchLevel(int moves, int seconds, int inc);

	virtual bool SetTimeRemaining(int milliseconds);
	virtual bool SetOppTimeRemaining (int milliseconds);

	virtual bool Search(int mode, Engine::searchPVFunction, Engine::searchFRFunction, Engine::searchCMFunction, Engine::searchRefFunction, Engine::searchStrFunction, int move=0);
	virtual bool SearchStop(void);

	virtual bool Stop(void);

	typedef enum {
		vnormal,
		vwildcastle,
		vnocastle,
		vfischerandom,
		vbughouse,
		vcrazyhouse,
		vlosers,
		vsuicide,
		vgiveaway,
		vtwokings,
		vkriegspiel,
		vatomic,
		v3check,
		vunknown
	} variant_t;

protected:

	virtual bool ResponseThread(void);

private:

	char myname[256];
	bool fping;
	bool fsetboard;
	bool fplayother;
	bool fsan;
	bool fusermove;
	bool ftime;
	bool fdraw;
	bool fsigint;
	bool fsigterm;
	bool freuse;
	bool fanalyze;
	bool fvariants[16];
	bool fcolors;
	bool fics;
	bool fname;
	bool fpause;
};


#endif // __ENGINEWB_H