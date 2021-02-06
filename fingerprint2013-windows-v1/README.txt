This is the CSVN FingerPrinting test tool. This tool can be used to submit the fingerprint for the International CSVN Tournament that is held at June 1-2 2013 in Leiden. Questions and/or suggestions can be directed to Richard Pijl by email at: thebaron@telenet.be

The tool and its sourcecode is provided to facilitate submitting an engine fingerprint to the CSVN tournament organisation. Other use of this tool and/or the sourcecode is prohibited.

Prerequisites:
An engine supporting either UCI or Winboard v2 with support for both 'ping' and 'setboard' commands. 
If the UCI engine requires setting of options this can be done by modifying the supplied source code in the file 'main.cpp'. The location where and the method how to modify the code is marked and an example is provided.
Setting options for winboard engines by means of the 'option' command is not yet supported by this tool.

Installation:
Copy the files 'fingerprint.exe' and 'simcsvn1.dos.epd' in the folder where the engine is located.

Execution:
Run 'fingerprint.exe'. The program will run in a console window and will prompt two questions. First the type of engine should be specified. This can be either UCI (U) or Winboard v2 (W). Next the name of the engine executable should be entered. There should be no spaces in the engine name. The result is collected in the file 'fingerprint.epd'. On each run of the tool this file is completely overwritten.
