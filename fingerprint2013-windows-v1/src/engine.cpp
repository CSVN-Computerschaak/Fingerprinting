// Engine.cpp
//
// Copyright (C) 2008-2013, ir. R.L. Pijl

#include <stdio.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include "engine.h"

const char *
Engine::errorStrings[100] = {
	"Ok", // 0
	"Other error",
	"Could not change the working directory",
	"Could not allocate more memory",
	"Engine name not specified",
	"Creating Engine command pipe failed", // 5
	"Creating Engine result pipe failed",
	"Duplicating pipe to input failed",
	"Duplicating pipe to output failed",
	"Could not restore stdin",
	"Could not restore stdout", // 10
	"Could not start the engine process",
	"Opening filestream for engine command pipe failed",
	"Opening filestream for engine result pipe failed",
	"Engine process did not terminate",
	"Copyprotection error", // 15
	"Position is not specified",
	"Engine has already started",
	"Engine has not yet started",
	"Engine is not searching",
	"Engine is already searching", // 20
	"Could not start a thread for collecting the engine responses",
	"No Final Response handler specified in call to Search",
	"Requested feature is not supported in this engine",
	"Illegal Position",
	"Engine does not allow setting options" // 25
};

Engine::Engine()
{
	engineWorkingDir=0;
	engineExecName=0;
	toengine=0;
	fromengine=0;
	engineid=0;
	started=false;
	searching=false;
	optionPonder=false;
	ponderMode=false;
	optionMultiPV=false;
	multiPV=1;
	multiPVmin=1;
	multiPVmax=1;
	optionShowCurrline=false;
	showCurrline=false;
	optionShowRefute=false;
	showRefutation=false;
	optionAnalyzeMode=false;
	pvHandler=0;
	finHandler=0;
	cmHandler=0;
	refHandler=0;
	strHandler=0;
}

Engine::~Engine()
{
	if (engineWorkingDir)
		delete engineWorkingDir;
	if (engineExecName)
		delete engineExecName;
	if (toengine) {
		fclose(toengine);
		toengine=0;
	}
	if (fromengine) {
		fclose(fromengine);
		fromengine=0;
	}
}

bool 
Engine::SetWorkingDir(const char* wdir)
{
	if (engineWorkingDir) {
		delete engineWorkingDir;
		engineWorkingDir=0;
	}
	engineWorkingDir=new char[strlen(wdir)+1];
	if (!engineWorkingDir) {
		errorNumber=ENGINENOMEM;
		return true;
	}

	strcpy(engineWorkingDir,wdir);
	errorNumber=ENGINEOK;
	return false;
}

bool
Engine::SetExecName(const char* exec)
{
	if (engineExecName) {
		delete engineExecName;
		engineExecName=0;
	}
	engineExecName=new char[strlen(exec)+1];
	if (!engineExecName) {
		errorNumber=ENGINENOMEM;
		return true;
	}

	strcpy(engineExecName,exec);
	errorNumber=ENGINEOK;
	return false;
}

bool
Engine::StartEngine(void)
{
	int fdStdOut, fdStdIn;
	char *args[16];
	char buf[1024];
	char *s;
	int i=0;

	if (started) {
		errorNumber=ENGINEALREADYSTARTED;
		return true;
	}
	if (engineWorkingDir)
		if (_chdir(engineWorkingDir)) {
			errorNumber=ENGINENODIR;
			return true;
		}
	if (!engineExecName) {
		errorNumber=ENGINENONAME;
		return true;
	}

	strcpy (buf,engineExecName);
	s=strtok(buf," \n\r\t");
	do {
		args[i++]=strdup(s);
		s=strtok(0," \t\n\r");
	} while (s);
	args[i]=0;

	// Change directory to the engine directory
	// Prepare I/O pipes
	if (_pipe(enginepipe,4096,O_TEXT|O_NOINHERIT)==-1) {
		errorNumber=ENGINECMDPIPE;
		return true;
	}
	if (_pipe(enginerespipe,4096,O_TEXT|O_NOINHERIT)==-1) {
		errorNumber=ENGINERESPIPE;
		return true;
	}

	// Duplicate std file descriptors (next line will close original)
	fdStdOut = _dup(_fileno(stdout));
	fdStdIn = _dup(_fileno(stdin));

	// Duplicate write end of pipe to stdout file descriptor
	if(_dup2(enginerespipe[WRITE], _fileno(stdout)) != 0) {
		errorNumber=ENGINEPIPEOUT;
		return true;
	}
	if(_dup2(enginepipe[READ], _fileno(stdin)) != 0) {
		errorNumber=ENGINEPIPEIN;
		return true;
	}

	// Close original write end of pipe
	_close(enginerespipe[WRITE]);
	_close(enginepipe[READ]);

	// Start the engine
	engineid=_spawnv(P_NOWAIT, args[0], args );

	// Duplicate copy of original stdout back into stdout
	if(_dup2(fdStdOut, _fileno(stdout)) != 0) {
		errorNumber=ENGINESTDOUT;
		return true;
	}
	if(_dup2(fdStdIn, _fileno(stdin)) != 0) {
		errorNumber=ENGINESTDIN;
		return true;
	}

	// Close duplicate copy of original stdout
	_close(fdStdOut);
	_close(fdStdIn);

	if (engineid<=0) {
		errorNumber=ENGINEPROCSTART;
		return true;
	}

	// Connect I/O pipes properly
	toengine=_fdopen(enginepipe[WRITE],"w");
	if (!toengine) {
		errorNumber=ENGINEFDCMDPIPE;
		// TODO: Stop the engine process
		return true;
	}
	fromengine=_fdopen(enginerespipe[READ],"r");
	if (!fromengine) {
		errorNumber=ENGINEFDRESPIPE;
		// TODO: Stop the engine process
		return true;
	}

	setbuf(toengine,0);
	started=true;

	return InitEngine();
}

bool
Engine::SetOption(const char* id, const char* value)
{
	errorNumber=ENGINENOOPT;
	return true;
}

bool
Engine::Synchronize(void)
{
	return false;
}

bool
Engine::WaitForStop(void)
{
	char buf[2048];
	int i=0;

	fclose(toengine);
	toengine=0;
	do {
		i++;
		if (i==0) break;
		fgets(buf,2047,fromengine);
	} while (!feof(fromengine));
	if (i==0) {
		// TODO kill engine
		errorNumber=ENGINENOTERM;
		return true;
	}
	engineid=0;
	fromengine=0;
	started=false;

	errorNumber=ENGINEOK;
	return false;
}

int 
Engine::GetError(void)
{
	return errorNumber;
}

const char*
Engine::GetErrorStr()
{
	if (errorNumber>=0 && errorNumber<100)
		return errorStrings[errorNumber];
	else
		return errorStrings[1];
}

DWORD WINAPI startResponseThread(LPVOID lpParam)
{
	// this function is called when creating a response thread
    Engine* engine;

    engine = (Engine*)lpParam;

	if (engine->ResponseThread()) {
		return 1;
	}
	return 0;
}
