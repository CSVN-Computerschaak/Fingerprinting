// main.cpp
// Fingerprinting tester for CSVN tournaments
//
// Copyright (C) 2013, ir. R.L. Pijl

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "engineuci.h"
#include "enginewb.h"
#include "util.h"

volatile bool searching=false;
FILE *fp;
char buf[1024];

bool fHandler(int move, int)
{
	fprintf(fp,"%s bm %s\n",buf,MoveStr(move));
	searching=false;
	return false;
}


int main(int argc, char* argv[])
{
	FILE *epd;

	Engine* engine;

	printf("CSVN Fingerprinting test tool v1.0\n");
	printf("----------------------------------\n\n");
	printf("What type of engine is used? (W/U) : ");
	gets(buf);

	if (toupper(*buf)=='U') {
		engine=new UCIEngine;
	} else {
		engine=new WBEngine;
	}

	printf("What is the name of the engine executable? : ");
	gets(buf);

	if (engine->SetExecName(buf)) {
		fprintf(stderr,"ERROR: %s\n",engine->GetErrorStr());
		exit(1);
	}
	if (engine->SetWorkingDir(".")) {
		fprintf(stderr,"ERROR: %s\n",engine->GetErrorStr());
		exit(1);
	}
	if (engine->StartEngine()) {
		fprintf(stderr,"ERROR: Could not start the engine: %s\n",engine->GetErrorStr());
		exit(1);
	} 

	engine->Synchronize();
	fprintf(stderr,"done.\n");

	// Enter the option setting below here. This is only possible for UCI engines
	// Both option name and value should be a string parameter
	// Example:
	// engine->SetOption("Threads","1");

	epd=fopen("simcsvn1.dos.epd","r");
	if (epd==0) {
		printf("Could not open the epd-file simcsvn1.dos.epd\n");
		exit(1);
	}

	fp=fopen("fingerprint.epd","w");
	if (fp==0) {
		printf("Could not open the epd-file fingerprint.epd\n");
		exit(1);
	}

	int i=1;
	while (fgets(buf,1024,epd)) {
		strtok(buf,"\n\r");
		engine->SetPosition(buf);
		engine->SetSearchTime(1);
		searching=true;
		engine->Search(0, 0, fHandler, 0, 0, 0);
		fprintf(stderr,"\rEngine search: %d/10000 ",i);
		while (searching) {
			fprintf(stderr,"\\\b"); Sleep(250);
			if (!searching) break;
			fprintf(stderr,"|\b"); Sleep(250);
			if (!searching) break;
			fprintf(stderr,"/\b"); Sleep(250);
			if (!searching) break;
			fprintf(stderr,"-\b"); Sleep(250);
		}
		i++;
	}
	fprintf(stderr,"\nDone.\n");
	printf("The result can be found as 'fingerprint.epd'\n");
	fclose(epd);
	fclose(fp);
}