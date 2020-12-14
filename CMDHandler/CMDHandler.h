/*************************************************************
*  Header for command processor object
*
************************************************************/

#ifndef  CMDHANDLER_H
#define CMDHANDLER_H

#include <Arduino.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAXCOMMANDS 30
#define MAXPARAMS 4
#define MAXPARAMLEN 32
#define MAXLINELEN 100

// A pointer to this structure is passed to the command action function
// so that it can use the parameter strings
typedef struct cmdline {
  char *pstr[4];  // pointers to parameter strings
  uint16_t validParams;     // # of valid parameters for this command
  uint16_t cmdidx;
} cmdlinetype;

class CMDHandler
{
	protected:
	private:
	char cmdsbuffer[MAXPARAMS][MAXPARAMLEN];
	uint16_t numcommands;

	cmdlinetype cmdline;
	bool dbprint = false;

// default to using USB Serial IO for input
Stream *iostreamptr = &Serial;
	
	char* SetParamString( uint16_t pnum, char *token);
	uint16_t FindCommand(char ch1, char ch2);
	uint16_t ParseCommand(char *line);
	void ClearParams(void);
	void DoCommand(char *cmdstring);
	void AddToCommand(char ch) ;
	
	public:
		
		//  Default constructor
		CMDHandler(){
			numcommands=1;
		}
		
		~CMDHandler(){
			
		}

	void AddCommand( void (*cmfptr)(void*), const char *cmstring, uint16_t numparams);
	void SetCMDStream(Stream *sptr);
	bool CheckCommandInput(void);
	void SetDBPrint(bool dbpval);

}; // end of class header



#ifdef __cplusplus
}
#endif


#endif // CMDHANDLER_H
