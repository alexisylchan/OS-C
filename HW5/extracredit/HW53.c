/**
Author: Alexis YL Chan
Date: 11/08/2011
Reads character from input stream, accumulates up to 80 characters. Sends 80 character streams to stdout. Fourth process in the pipeline.

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
#define STRMAX 80

FILE *inputFile;
 
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
  char *charArray;
  charArray = (char *) malloc (sizeof (char) * (STRMAX + 1));
  int count = 0; 
  while (1)
    {
      // Get input character from input stream
      int ch = fgetc (inputFile);


      if (ferror (inputFile))
	{
	  perror ("Error reading input file");
	  exit (1);
	}
      if (ch == EOF)
	{
	  free (charArray);
	  exit (0);
	}
      charArray[count] = ch;
      count++;
      if (count >= STRMAX)
	{
	  charArray[count] = '\n';
	  fputs (charArray, stdout);
	  free (charArray);
	  charArray = 0;
	  charArray = (char *) malloc (sizeof (char) * (STRMAX + 1));
	  count = 0;
	} 
    } 
  return 0;
}
