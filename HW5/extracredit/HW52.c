/**
Author: Alexis YL Chan
Date: 11/08/2011

Reads character from input stream, replaces 2 consecutive asterixes with a ^. Sends it to stdout. Third process in the pipeline.

**/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>

#define DEBUG 0
#define HW5SILENT 1
#define READ 0
#define WRITE 1

FILE *inputFile; 
// Main function that starts all threads
int
main (int argc, char *argv[])
{

  // If the an argument is provided, assume that it is the file to replace stdin as the input stream.
  // This is used for debugging and testing.
  if (argc > 1)
    {
      inputFile = fopen (argv[1], "r");
      if (inputFile == NULL)
	{
	  if (DEBUG)
	    {
	      perror
		("Error opening input file. Switching to standard input");
	    }
	  inputFile = stdin;
	}
    }
  else
    {
      inputFile = stdin;
    }
  while (1)
    {
      // Get input character from input stream
      int ch = fgetc (inputFile);


      if (ferror (inputFile))
	{
	  perror ("Error reading input file");
	  exit (1);
	}

      if (ch == '*')
	{
	  int ch2 = fgetc (inputFile);
	  if (ch2 == '*')
	    {
	      int ch3 = '^';
	      if (fputc (ch3, stdout) == -1)
		{
		  perror ("Error writing ");
		  break;
		}
	    }
	  else
	    {
	      if (fputc (ch, stdout) == -1)
		{
		  perror ("Error writing");
		  break;
		}
	      if (fputc (ch2, stdout) == -1)
		{
		  perror ("Error writing");
		  break;
		}
	      if (ch == EOF || ch2 == EOF)	// actually ch will not be EOF as it is '*'
		{
		  exit (0);
		}
	    }
	}
      else
	{
	  if (fputc (ch, stdout) == -1)
	    {
	      perror ("Error writing");
	      break;
	    }
	  if (ch == EOF)
	    {
	      exit (0);
	    }
	}
      if (feof (inputFile))
	{
	  exit (0);
	} 
    } 
  return 0;
}
