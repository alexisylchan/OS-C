/**
Author: Alexis YL Chan
Date: 11/08/2011
Starts the 4 processes to process characters from input stream or file (if given a file as an argument into the parent process).
First process reads character from input stream.
Second process strips newlines and replace them with spaces.
Third process strips consecutive asterixes and replace them with ^ .
Fourth process sends 80 character lines (81 if including newline character) to output stream which is stdout.

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

#define STRMAX 80
#define DEBUG 0
#define HW5SILENT 1
#define READ 0
#define WRITE 1

FILE *inputFile;
int fromStreamBufferPipe[2];
int throughStripEndlinesPipe[2];
int throughStripAsterixPipe[2];

pid_t readFromInput_id,
  stripEndlines_id, stripAsterix_id, sendToOutput_id, term_pid;
int child_status;

int c;
char parent_data;

void
do_ReadFromInput ()
{
  close (fromStreamBufferPipe[READ]);	// Close reading of fromStreamBufferPipe
  while (1)
    {
      // Get input character from input stream
      int ch = fgetc (inputFile);

      // Terminate if error
      if (ferror (inputFile))
	{
	  perror ("Error reading input file");
	  exit (1);
	}
      if (write (fromStreamBufferPipe[WRITE], &ch, 1) == -1)
	{
	  perror ("Error writing to fromStreamBufferPipe");
	  break;
	}
      if (feof (inputFile))
	{
	  close (fromStreamBufferPipe[WRITE]);
	  exit (0);
	}
    }
  close (fromStreamBufferPipe[WRITE]);
  exit (0);

}

void
do_StripEndlines ()
{

  close (fromStreamBufferPipe[WRITE]);
  close (throughStripEndlinesPipe[READ]);

  char child1_data;
  int rc;

  while (1)
    {
      rc = read (fromStreamBufferPipe[READ], &child1_data, 1);
      if (child1_data == '\n')
	child1_data = ' ';

      if (write (throughStripEndlinesPipe[WRITE], &child1_data, 1) == -1)
	{
	  perror ("Error writing to throughStripEndlinesPipe");
	  break;
	}
      if (child1_data == EOF)
	{
	  close (fromStreamBufferPipe[READ]);
	  close (throughStripEndlinesPipe[WRITE]);
	  exit (0);
	}
    }
  close (fromStreamBufferPipe[READ]);
  close (throughStripEndlinesPipe[WRITE]);
  exit (0);
}



void
do_StripAsterix ()
{

  close (throughStripEndlinesPipe[WRITE]);
  close (throughStripAsterixPipe[READ]);

  char child1_data, child2_data;
  char write_char;
  int rc;


  while (1)
    {
      rc = read (throughStripEndlinesPipe[READ], &child1_data, 1);
      if (child1_data == '*')
	{
	  rc = read (throughStripEndlinesPipe[READ], &child2_data, 1);
	  if (child2_data == '*')
	    {
	      write_char = '^';
	      if (write (throughStripAsterixPipe[WRITE], &write_char, 1) ==
		  -1)
		{
		  perror ("Error writing to throughStripAsterixPipe");
		  break;
		}
	    }
	  else
	    {
	      if (write (throughStripAsterixPipe[WRITE], &child1_data, 1) ==
		  -1)
		{
		  perror ("Error writing to throughStripAsterixPipe");
		  break;
		}
	      if (write (throughStripAsterixPipe[WRITE], &child2_data, 1) ==
		  -1)
		{
		  perror ("Error writing to throughStripAsterixPipe");
		  break;
		}
	      if (child1_data == EOF || child2_data == EOF)	// actually child1_data will not be EOF as it is '*'
		{
		  close (throughStripEndlinesPipe[READ]);
		  close (throughStripAsterixPipe[WRITE]);
		  exit (0);
		}
	    }
	}
      else
	{
	  if (write (throughStripAsterixPipe[WRITE], &child1_data, 1) == -1)
	    {
	      perror ("Error writing to throughStripAsterixPipe");
	      break;
	    }
	  if (child1_data == EOF)
	    {
	      close (throughStripEndlinesPipe[READ]);
	      close (throughStripAsterixPipe[WRITE]);
	      exit (0);
	    }
	}


    }
  close (throughStripEndlinesPipe[READ]);
  close (throughStripAsterixPipe[WRITE]);
  exit (0);
}

void
do_SendToOutput ()
{
  char child1_data;
  char *charArray;
  charArray = (char *) malloc (sizeof (char) * (STRMAX + 1));
  int rc;
  int count = 0;

  close (throughStripAsterixPipe[WRITE]);
  while (1)
    {

      rc = read (throughStripAsterixPipe[READ], &child1_data, 1);
      if (child1_data == EOF)
	{
	  close (throughStripAsterixPipe[READ]);
	  free (charArray);
	  exit (0);
	}
      charArray[count] = child1_data;
      count++;
      if (count >= STRMAX)
	{
	  charArray[count] = '\n';
	  fputs (charArray, stdout);
	  free (charArray);
	  charArray = 0;
	  charArray = (char *) malloc (sizeof (char) * STRMAX + 1);
	  count = 0;
	}
    }
  close (throughStripAsterixPipe[READ]);
  exit (0);
}


void
terminateChild (pid_t * status_id, pid_t * child_id, int *status)
{

  *status_id = waitpid (*child_id, status, 0);
  if (*status_id == -1)
    perror ("waitpid");
  else
    {
      if (!HW5SILENT)
	{
	  if (WIFEXITED (*status))
	    printf ("PID %d exited, status =  %d \n", *child_id,
		    WEXITSTATUS (*status));
	  else
	    printf ("PID %d did not exit normally\n", *child_id);
	}
    }

}

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



  /**  
  * create pipes to connect processes
  */
  if (pipe (fromStreamBufferPipe) == -1)
    {
      perror ("Error creating fromStreamBufferPipe\n");
      exit (0);
    }
  if (pipe (throughStripEndlinesPipe) == -1)
    {
      perror ("Error creating throughStripEndlinesPipe\n");
      exit (0);
    }

  if (pipe (throughStripAsterixPipe) == -1)
    {
      perror ("Error creating throughAsterixPipe\n");
      exit (0);
    }

	/**
	* fork off child processes
	*/

  readFromInput_id = fork ();
  if (readFromInput_id == -1)
    {
      perror ("Error forking readFromInput");
      exit (-1);
    }
  if (readFromInput_id == 0)
    {
      do_ReadFromInput ();
    }
  else
    {
      stripEndlines_id = fork ();
      if (stripEndlines_id == -1)
	{
	  perror ("Error forking stripEndlines");
	  exit (-1);
	}
      if (stripEndlines_id == 0)
	{
	  do_StripEndlines ();
	}
      else
	{

	  stripAsterix_id = fork ();
	  if (stripAsterix_id == -1)
	    {
	      perror ("Error forking stripAsterix");
	      exit (-1);
	    }
	  if (stripAsterix_id == 0)
	    {
	      do_StripAsterix ();
	    }
	  else
	    {
	      sendToOutput_id = fork ();
	      if (sendToOutput_id == -1)
		{
		  perror ("Error forking sendToOutput");
		  exit (-1);
		}
	      if (sendToOutput_id == 0)
		{
		  do_SendToOutput ();
		}
	      else
		{
		  terminateChild (&term_pid, &readFromInput_id,
				  &child_status);
		  terminateChild (&term_pid, &stripEndlines_id,
				  &child_status);
		  terminateChild (&term_pid, &stripAsterix_id, &child_status);
		  terminateChild (&term_pid, &sendToOutput_id, &child_status);


		  if (!HW5SILENT)
		    printf ("Parent process terminating.\n");
		  exit (0);
		}
	    }
	}
    }
  return 0;
}
