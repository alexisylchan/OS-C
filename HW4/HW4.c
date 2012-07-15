/**
Author: Alexis YL Chan
Date: 10/17/2011
Starts the 4 threads to process characters from input stream or file (if given a file as an argument into the running process).
First thread reads character from input stream.
Second thread strips newlines and replace them with spaces.
Third thread strips consecutive asterixes and replace them with ^ .
Fourth thread sends 80 character lines (81 if including newline character) to output stream which is stdout.

**/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "buffer.h"

#define STRMAX 80
#define DEBUG 0
#define HW4SILENT 1

// Wrapper struct that is passed from main to the threads started.
// The same wrapper struct is used for all the threads for convenience.
struct readWriteStruct
{
  FILE *currStream;		// Input stream. This is only used by the thread that reads from input stream.
  BoundedBuffer *readBuffer;	// The buffer that the thread reads from. This is not used by the thread that reads from input stream.
  BoundedBuffer *writeBuffer;	// The buffer that the thread writes to. This is not used by the thread that writes to output stream.
  char *notification;		// Used for debugging purposes to identify threads.
  int sleep_time;		// Amount of time the thread needs to sleep.
};

typedef struct readWriteStruct ReadWriteStruct;

// Helper function that initializes the wrapper struct 
// Returns a pointer to the wrapper struct
ReadWriteStruct *
initializeReadWriteStruct (FILE * currStream, BoundedBuffer * readBuffer,
			   BoundedBuffer * writeBuffer,
			   const char *notification, int sleep_time)
{
  ReadWriteStruct *currStruct =
    (struct readWriteStruct *) malloc (sizeof (struct readWriteStruct));
  currStruct->currStream = currStream;
  currStruct->readBuffer = readBuffer;
  currStruct->writeBuffer = writeBuffer;
  currStruct->notification = (char *) malloc (sizeof (char) * 4);
  strcpy (currStruct->notification, notification);
  currStruct->sleep_time = sleep_time;

}

// Function that handles the first thread: reading from input stream
void *
readFromInput (void *s)
{
  ReadWriteStruct *readStruct = (ReadWriteStruct *) s;
  assert (readStruct->currStream != NULL);

  while (1)
    {

      // Get input character from input stream
      int ch = fgetc (readStruct->currStream);
      char charArray[1];
      charArray[0] = ch;
      // Terminate if error
      if (ferror (readStruct->currStream))
	{
	  perror ("Error reading input file");
	  st_thread_exit (NULL);
	}
      if (!HW4SILENT)
	{
	  printf ("%s \n", readStruct->notification);
	}

      //Allocate character
      char *ch_store = (char *) malloc (sizeof (char) * 2);
      strcpy (ch_store, charArray);

      //Deposit to buffer
      buffer_deposit (readStruct->writeBuffer, ch_store);
      //Allow consumer to run
      st_sleep (readStruct->sleep_time);

      //Terminate if EOF
      if (feof (readStruct->currStream))
	{

	  st_thread_exit (NULL);
	}

    }
  st_thread_exit (NULL);
}

// Function that handles the second thread: strip newlines and replace them with spaces.
void *
stripEndlines (void *s)
{
  ReadWriteStruct *stripEndlinesStruct = (ReadWriteStruct *) s;
  while (1)
    {
      char *ch = (char *) malloc (sizeof (char) * 2);
      buffer_remove (stripEndlinesStruct->readBuffer, ch);

      if (!HW4SILENT)
	{
	  printf ("%s %s\n", stripEndlinesStruct->notification, ch);
	}
      //Allocate character
      char *ch_store = (char *) malloc (sizeof (char) * 2);
      if (ch[0] == '\n')
	{
	  char charArray[1];
	  charArray[0] = ' ';
	  strcpy (ch_store, charArray);
	}
      else
	{
	  strcpy (ch_store, ch);
	}

      //Deposit to buffer
      buffer_deposit (stripEndlinesStruct->writeBuffer, ch_store);
      //Allow consumer to run
      st_sleep (stripEndlinesStruct->sleep_time);
      //Terminate if EOF
      if (ch[0] == EOF)
	{
	  free (ch);
	  free (ch_store);
	  st_thread_exit (NULL);
	}
      free (ch);
      free (ch_store);
    }
  st_thread_exit (NULL);
}

// Function that handles the third thread: strips consecutive asterixes and replace them with ^
void *
stripAsterix (void *s)
{
  ReadWriteStruct *stripAsterixStruct = (ReadWriteStruct *) s;


  while (1)
    {
      char *ch = (char *) malloc (sizeof (char) * 2);
      char *ch2 = (char *) malloc (sizeof (char) * 2);
      buffer_remove (stripAsterixStruct->readBuffer, ch);
      if (!HW4SILENT)
	{
	  printf ("%s %s\n", stripAsterixStruct->notification, ch);
	}
      //Allocate character
      char *ch_store = (char *) malloc (sizeof (char) * 2);
      char *ch_store2 = (char *) malloc (sizeof (char) * 2);
      if (ch[0] == '*')		// If found first asterix
	{

	  buffer_remove (stripAsterixStruct->readBuffer, ch2);
	  // Check if second asterix exists               
	  if (ch2[0] == '*')
	    {
	      char charArray[1];
	      charArray[0] = '^';	// Replace consecutive asterixes with ^
	      strcpy (ch_store, charArray);
	      buffer_deposit (stripAsterixStruct->writeBuffer, ch_store);


	    }
	  else
	    {			// If its a single asterix, put both original characters in the buffer
	      strcpy (ch_store, ch);
	      buffer_deposit (stripAsterixStruct->writeBuffer, ch_store);
	      st_sleep (stripAsterixStruct->sleep_time);

	      strcpy (ch_store2, ch2);
	      buffer_deposit (stripAsterixStruct->writeBuffer, ch_store2);

	    }
	}
      else
	{
	  strcpy (ch_store, ch);
	  buffer_deposit (stripAsterixStruct->writeBuffer, ch_store);

	}
      //Allow consumer to run
      st_sleep (stripAsterixStruct->sleep_time);

      //Terminate if EOF
      if (ch_store[0] == EOF)
	{
	  free (ch);
	  free (ch2);
	  free (ch_store);
	  free (ch_store2);
	  st_thread_exit (NULL);
	}
      free (ch);
      free (ch2);
      free (ch_store);
      free (ch_store2);
    }
  st_thread_exit (NULL);
}

// Function that handles last thread: send 80 character lines to output stream
void *
sendToOutput (void *s)
{
  ReadWriteStruct *writeStruct = (ReadWriteStruct *) s;
  char *chArray = (char *) malloc (sizeof (char) * STRMAX);
  int count = 0;
  while (1)
    {
      if (!HW4SILENT)
	{
	  printf ("Entered SendToOutput\n");
	}
      char *ch = (char *) malloc (sizeof (char) * 2);
      buffer_remove (writeStruct->readBuffer, ch);
      //If consumer reads an end of file from producer, free the character and terminate
      if (ch[0] == EOF)
	{
	  free (ch);
	  free (chArray);
	  st_thread_exit (NULL);
	}
      if (!HW4SILENT)
	{
	  printf ("%s %s\n", writeStruct->notification, ch);
	}
      else
	{
	  chArray[count] = ch[0];
	  count++;
	}
      if (count == STRMAX)
	{
	  printf ("%s\n", chArray);
	  free (chArray);
	  chArray = 0;
	  chArray = (char *) malloc (sizeof (char) * STRMAX);
	  count = 0;
	}
      st_sleep (writeStruct->sleep_time);
      free (ch);
    }
  free (chArray);
  st_thread_exit (NULL);
}


// Main function that starts all threads
int
main (int argc, char *argv[])
{
  FILE *inputFile;

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
  // Check that st is initialized
  if (st_init () < 0)
    {
      perror ("st_init");
      exit (1);
    }

  // Initialize all the bounded buffers required for storing characters. Each buffer is shared by 2 threads.
  BoundedBuffer *fromStreamBuffer = buffer_init (BUFFER_SIZE);	//Shared by thread reading from input stream and thread stripping newLines
  BoundedBuffer *throughStripEndline = buffer_init (BUFFER_SIZE);	//Shared by thread stripping newLines and thread stripping consecutive asterixes
  BoundedBuffer *throughStripAsterix = buffer_init (BUFFER_SIZE);	//Shared by thread stripping consecutive asterixes and thread writing to output stream

  // Initialize all the wrapper structs for sending arguments to the threads
  ReadWriteStruct *readStruct =
    initializeReadWriteStruct (inputFile, NULL, fromStreamBuffer, "RS1", 1);
  ReadWriteStruct *stripEndlinesStruct =
    initializeReadWriteStruct (NULL, fromStreamBuffer, throughStripEndline,
			       "SES", 1);
  ReadWriteStruct *stripAsterixStruct =
    initializeReadWriteStruct (NULL, throughStripEndline, throughStripAsterix,
			       "SAS", 1);
  ReadWriteStruct *writeStruct =
    initializeReadWriteStruct (stdout, throughStripAsterix, NULL, "WS1", 1);

  // Create threads
  st_thread_create (readFromInput, readStruct, 0, 0);
  st_thread_create (stripEndlines, stripEndlinesStruct, 0, 0);
  st_thread_create (stripAsterix, stripAsterixStruct, 0, 0);
  st_thread_create (sendToOutput, writeStruct, 0, 0);
  // Exit main thread
  st_thread_exit (NULL);

  // Free memory
  free (readStruct->notification);
  free (writeStruct->notification);
  free (stripEndlinesStruct->notification);
  free (stripAsterixStruct->notification);
  buffer_dispose (fromStreamBuffer);
  buffer_dispose (throughStripEndline);
  buffer_dispose (throughStripAsterix);

  free (readStruct);
  free (writeStruct);
  free (stripEndlinesStruct);
  free (stripAsterixStruct);



  return 0;
}
