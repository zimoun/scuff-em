/* Copyright (C) 2005-2011 M. T. Homer Reid
 *
 * This file is part of SCUFF-EM.
 *
 * SCUFF-EM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SCUFF-EM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * ProcessOptions.cc -- libhrutil facility for processing options.
 *
 * this is similar to 'ProcessArguments,' but extended in three ways:                                      
 *
 * (1) in addition to command-line arguments, options and their 
 *     arguments can be specified in a little text file piped into 
 *     stdin
 *
 * (2) options can be specified more than once with different arguments
 *
 * (3) options may take more than one argument.
 *
 * homer reid        -- 1/2012                       
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "libhrutil.h"

#define MAXTOK 50
#define MAXSTR 1000

/***************************************************************/
/***************************************************************/
/***************************************************************/
void OSUsage(char *ProgName, OptStruct *OSArray, const char *format, ...)
{
  va_list ap;
  char buffer[MAXSTR];

  if (format)
   { va_start(ap,format);
     vsnprintfEC(buffer,MAXSTR,format,ap);
     fprintf(stderr,"\nerror: %s (aborting)\n\n",buffer);
     va_end(ap);
   };

  fprintf(stderr,"usage: %s [options]\n\n",ProgName);
  fprintf(stderr," options: \n\n");

  int na;
  OptStruct *OS;
  for(OS=OSArray; OS->Name!=0; OS++)
   { 
     sprintf(buffer,"--%s ",OS->Name);
     for(na=0; na<OS->NumArgs; na++) 
      strcat(buffer,"xx ");
     fprintf(stderr,"  %-20s",buffer);
     if ( OS->Description )
      fprintf(stderr,"(%s)",OS->Description);
     fprintf(stderr,"\n");
   };

  fprintf(stderr,"\n");
  
  exit(1);

}

/***************************************************************/
/* this routine is called by ProcessOptions() to process a     */
/* single option and its arguments.                            */
/* returns 0 on success, nonzero on failure. in the latter     */
/* case, ErrMsg is filled in with an error string.             */
/***************************************************************/
int ProcessOption(char *Option, char *Args[], int NumArgs,
                  OptStruct *OSArray, char *ErrMsg)
{
  int ni, na;

  OptStruct *OS;
  for(OS=OSArray; OS->Name!=0; OS++)
   if ( !StrCaseCmp(Option, OS->Name) )
    { 
      /*--------------------------------------------------------------*/
      /*- check how many times this option has been used so far.      */
      /*--------------------------------------------------------------*/
      if ( !(OS->NumInstances) )
       { 
         ni=0;
       }
      else if ( OS->MaxInstances==1 && *(OS->NumInstances)==1 )
       { 
         if (NumArgs==0)
          Warn("option --%s was unnecessarily specified multiple times",Option);
         else if (NumArgs==1)
          Warn("new value (%s) for option --%s overrides previously specified value",Args[0],Option);
         else if (NumArgs>1)
          Warn("new value (%s...) for option --%s overrides previously specified value",Args[0],Option);
         ni=0;
       }
      else if ( OS->MaxInstances>1 && *(OS->NumInstances)==(OS->MaxInstances) )
       { 
         return snprintf(ErrMsg,MAXSTR,"option --%s specified too many times",Option);
       }
      else 
       { 
         ni=(*(OS->NumInstances))++;
       };

      /*--------------------------------------------------------------*/
      /*- make sure the correct number of arguments were specified   -*/
      /*--------------------------------------------------------------*/
      if (NumArgs != OS->NumArgs && OS->Type!=PA_BOOL)
       return snprintf(ErrMsg,MAXSTR,"option --%s needs %i arguments (%i provided)", Option,OS->NumArgs,NumArgs);

      /*--------------------------------------------------------------*/
      /*- now switch off to process arguments according to the type   */
      /*--------------------------------------------------------------*/
      cdouble *cdArray = (cdouble *) OS->Storage;
      char **cArray = (char **) OS->Storage;
      double *dArray = (double *) OS->Storage;
      int *iArray = (int *) OS->Storage;
      switch(OS->Type)
       { 
         case PA_STRING:
           for(na=0; na<NumArgs; na++)
            cArray[ni*NumArgs + na] = strdupEC(Args[na]);
           return 0;
            
         case PA_BOOL:
           if (NumArgs>0)
            return snprintf(ErrMsg,MAXSTR,"boolean option --%s takes no arguments",Option);
           if (ni>1)
            fprintf(stderr,"** warning: boolean option --%s specified more than once",Option);
           *((bool *)(OS->Storage)) = true;
           return 0;

         case PA_DOUBLE:
           for(na=0; na<NumArgs; na++)
            if ( 1 != sscanf(Args[na],"%le",dArray+ni*NumArgs+na) )
             return snprintf(ErrMsg,MAXSTR,"invalid argument (%s) passed to option --%s",Args[na],Option);
           return 0;

         case PA_INT:
           for(na=0; na<NumArgs; na++)
            if ( 1 != sscanf(Args[na],"%i",iArray+ni*NumArgs+na) )
             return snprintf(ErrMsg,MAXSTR,"invalid argument (%s) passed to option --%s",Args[na],Option);
           return 0;
            
         case PA_CDOUBLE:
           for(na=0; na<NumArgs; na++)
            if ( S2CD(Args[na],cdArray+ni*NumArgs+na) )
             return snprintf(ErrMsg,MAXSTR,"invalid argument (%s) passed to option --%s",Args[na],Option);
           return 0;

         default: 
           // unknown value for 'Type' field in OptStruct structure
           ErrExit("%s:%i:internal error",__FILE__,__LINE__);
       };

    }; // if (!StrCaseCmp...)

  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  /*--------------------------------------------------------------*/
  return snprintf(ErrMsg,MAXSTR,"unknown option --%s",Option);

}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void ProcessOptions(int argc, char *argv[], OptStruct *OSArray)
{
  /***************************************************************/
  /***************************************************************/
  /***************************************************************/
  OptStruct *OS;
  for(OS=OSArray; OS->Name!=0; OS++)
   if (OS->NumInstances)
    *(OS->NumInstances)=0;

  /***************************************************************/
  /* first handle any arguments that might have been passed via  */
  /* stdin                                                       */
  /***************************************************************/
  char *Tokens[MAXTOK];
  int NumTokens;
  char ErrMsg[MAXSTR];
  if ( !isatty(fileno(stdin)) )
   { 
     char Line[MAXSTR];
     int LineNum=0;
     while(fgets(Line,MAXSTR,stdin))
      { 
        LineNum++;
        NumTokens=Tokenize(Line,Tokens,MAXTOK);

        if (NumTokens==0 || Tokens[0][0]=='#') 
         continue;

        if (ProcessOption(Tokens[0], Tokens+1, NumTokens-1, OSArray, ErrMsg))
         { snprintf(Line,MAXSTR,"stdin:%i: %s",LineNum,ErrMsg);
           OSUsage(argv[0], OSArray, Line);
         };
      };
   };

  /***************************************************************/
  /* and now handle command-line arguments ***********************/
  /***************************************************************/
  if (argc==1)
   return;

  /* make sure the first thing on the command line is an argument */
  int nargFirst=1;
  while ( argv[nargFirst]==0 )
   { nargFirst++;
     if (nargFirst==argc) return; // no non-NULL arguments
   };
  if ( strncmp(argv[nargFirst],"--",2) )
   OSUsage(argv[0], OSArray,"unknown option %s",argv[1]);
  
  int nt, narg;
  for(narg=nargFirst; narg<argc; narg++)
   { 
     /*--------------------------------------------------------------*/
     /*- this line allows ProcessOptions() to be preceded by other   */
     /*- command-line argument handlers, which indicate that they    */
     /*- have already processed an argument by setting the           */
     /*- corresponding slot(s) in the argv[] vector to NULL          */
     /*--------------------------------------------------------------*/
     if (argv[narg]==0) continue;

     /*--------------------------------------------------------------*/
     /*- collect all successive arguments that do not begin with     */
     /*- '--' as parameters for the current option                   */
     /*--------------------------------------------------------------*/
     for(nt=NumTokens=0; nt<MAXTOK && argv[narg+nt+1] && (narg+nt+1)<argc; nt++)
      { if ( !strncmp(argv[narg+nt+1],"--",2) )
         break;
        Tokens[NumTokens]=argv[narg+nt+1];
        NumTokens++;
      };
     if (ProcessOption(argv[narg]+2, Tokens, NumTokens, OSArray, ErrMsg))
      OSUsage(argv[0], OSArray, ErrMsg);

     narg+=NumTokens;

   };

}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void ProcessOptions(const char *ArgString, OptStruct *OSArray)
{
  char *ArgStringCopy=strdup(ArgString);
  char *Tokens[100];
  int NumTokens=Tokenize(ArgStringCopy,Tokens,100);
  ProcessOptions(NumTokens,Tokens,OSArray);
  free(ArgStringCopy);

}
