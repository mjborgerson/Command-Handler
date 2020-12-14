/*************************************************************
*  C++ code for command processor object
*
************************************************************/
#include "CMDHandler.h"
 


// in this simplified version, all parameters are passed to action
// functions as pointers to the parsed parameter strings
typedef struct dacmd {
  uint8_t cmdch1, cmdch2; // 2-letter commands
  void (*aptr)(void*);  // pointer to action function receives pointer to cmdlinetype
  uint16_t numparam;

} dacmdtype;




dacmdtype cmdtable[MAXCOMMANDS];

// add a string parameter for a command line
// This function is no longer called after a strtok, so that it can
// handle embedded qotes.  token points to first character of the string
// which should be a quote mark
char*  CMDHandler::SetParamString( uint16_t pnum, char *token) {

  strncpy((char *)&cmdsbuffer[pnum][0], (char const *)token,MAXPARAMLEN-1);
  return &cmdsbuffer[pnum][0];
}



void CMDHandler::ClearParams(void) {
	uint16_t i;
	for(i= 0; i<MAXPARAMS; i++)  cmdline.pstr[i]= NULL;
	cmdline.cmdidx = 0;
}

//  check to see if c1 and c2 match a command in the table
// use straight linear search for now. Start with command 1
uint16_t CMDHandler::FindCommand( char ch1, char ch2) {
  int16_t idx = 1;
  if(ch1 == 0) return 0;
  while (cmdtable[idx].cmdch1 != 0) {
    if (ch1 == cmdtable[idx].cmdch1) {
      if (ch2 == cmdtable[idx].cmdch2) return idx;
    }
    idx++;
	if(idx >= MAXCOMMANDS) return 0;
  }
  // no match---return 0
  return 0;
}

// This function parses an input command line and collects the parameter values
uint16_t CMDHandler::ParseCommand(char *line){
  char token[MAXLINELEN], lline[MAXLINELEN];
  char *tokptr = &token[0];
  char *midptr, char1, char2;
  int16_t i, nparam, tlen;
  int16_t idx = 0;
  dbprint = false;
  char *save_ptr;   // needed for strtok_r to save its position in string
  if(*line == 0) return 0;  // ignore empty lines
  strncpy((char *)lline, (char const *)line, MAXLINELEN-1);  // copy up to length-1 chars of input line
  tlen = strlen((char const *)lline);

  ClearParams();  // sets all string pointers to NULL


  if (lline[tlen - 3] == '\n') lline[tlen - 3] = 0; // remove trailing linefeed
  if (lline[tlen - 2] == '\n') lline[tlen - 2] = 0; // remove trailing linefeed
  if (lline[tlen - 3] == '\r') lline[tlen - 3] = 0; // remove trailing CR
  if (lline[tlen - 2] == '\r') lline[tlen - 2] = 0; // remove trailing lCR
  tokptr = (char *)strtok_r((char *)lline, " ,\t\n\r", &save_ptr); //  space , and tab are valid separators
  if(tokptr != NULL) {
	  tlen = strlen((char const *)tokptr);
	  if(tlen == 0){
		//rial.println("Zero length command line");
		cmdline.cmdidx = 0;
		return 0;
	  }
  } else {
	  tlen = 0;
	  cmdline.cmdidx = 0;
	  return 0;
  }
  

  // Get first two characters to match against command table

  char1 = toupper(*tokptr++);
  char2 = toupper(*tokptr++);


  idx = FindCommand(char1, char2);
  if(dbprint)Serial.printf("Found command with index %u\n",idx);
  cmdline.cmdidx = idx;

  // now, if numparams is not 0, find the parameters
  nparam = cmdtable[idx].numparam;
  midptr = NULL;
  for (i = 0; i < nparam; i++) {

    tokptr = (char *)strtok_r((char *)midptr, " ,\t\n\r", &save_ptr);
//	Serial.printf("In ParseCommand tokptr = %p\n",tokptr);
//    delay(5);
	if(tokptr == NULL) break;
    if (strlen((char const *)tokptr) > 0) { // see if token length > 0
		midptr = NULL;
		cmdline.pstr[i] = SetParamString( i, tokptr);
    }   else cmdline.pstr[i]= NULL;
	cmdline.validParams = nparam;
  }// end of for loop
  return idx;

}



void CMDHandler::DoCommand(char *cmdstring) {
uint16_t  cmdidx;
void (*aptr)(void *);   // command function pointer
	ParseCommand(cmdstring);
	cmdidx = cmdline.cmdidx;
	if(dbprint) Serial.printf("Executing command with index %u\n", cmdidx);
	if (cmdidx != 0) {
		aptr = cmdtable[cmdidx].aptr;
		aptr(&cmdline);  // call the action function with cmdline as parameter
	}

}


/*************************************************************************
* Public methods
************************************************************************/


void CMDHandler::AddCommand( void (*cmfptr)(void *), const char *cmstring, uint16_t numparams){
	uint16_t cmdidx;

	cmdidx = numcommands;
	if(cmdidx >= MAXCOMMANDS){
		// no room for further commands
		return;
	}


	cmdtable[cmdidx].aptr = cmfptr;
	cmdtable[cmdidx].cmdch1 = cmstring[0];
	cmdtable[cmdidx].cmdch2 = cmstring[1];
	if(numparams >MAXPARAMS) numparams = MAXPARAMS;
	cmdtable[cmdidx].numparam = numparams;
	numcommands++;
	if(dbprint)Serial.printf("Added command #%u with %u parameters\n",cmdidx, numparams);
}


//  add keyboard input to command string,  execute if LF
void CMDHandler::AddToCommand(char ch) {
  static char cmdstring[100];
  static int16_t stridx = 0;
  dbprint = false;

  if (stridx >= MAXLINELEN-2) stridx = 0; // reset string to avoid crashes
  if ((ch == '\r') ||  (ch == '\n')) { // if CR or  LF, end string and send
    cmdstring[stridx] = 0;  // terminate the string
    stridx = 0;   // reset for next string
	if(dbprint)Serial.printf("Processing command line: <0x%02X>\n", cmdstring[0]);
	if(cmdstring[0] != 0){
		DoCommand(cmdstring);
	} else return;

    // On the teensy with the Arduino terminal, we don't use prompts
  }  else  {
    if (ch == '\b') { //process backspace
      if (stridx > 0) stridx--;
    } else {
      cmdstring[stridx] = ch;
      stridx++;
    }

  }
}

void CMDHandler::SetCMDStream(Stream *sptr){
		iostreamptr = sptr;
	
}
	
	// returns true if a character was received
bool CMDHandler::CheckCommandInput(void){  
char ch;
bool rval;
  rval = false;
  while (iostreamptr->available()){
    ch = iostreamptr->read();
    AddToCommand(ch);
	rval = true;
  }	
  return rval;
}

void CMDHandler::SetDBPrint(bool dbpval){
	dbprint = dbpval;
}
