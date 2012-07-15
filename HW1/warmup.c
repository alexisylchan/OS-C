#include <stdio.h>
#include <stdlib.h>

#define STRMAX 80
#define DEBUG 0

/* Allocate memory to 81 character array buffer */
char *
reallocateCharBuffer ()
{
  char *buffer = malloc (sizeof (char) * (STRMAX + 1));
  if (buffer == NULL)
    {
      if (DEBUG)
	{
	  perror ("Failed to allocate memory");
	}
      exit (1);
    }
  return buffer;
}

/* Insert character into buffer. Print buffer if number of characters in buffer reaches 80. Insert newline character before printing.*/
char *
insertCharIntoBuffer (char *buffer, int *buffer_count, char ch)
{
  if (DEBUG)
    printf ("count %d ", (*buffer_count));
  buffer[(*buffer_count)] = ch;
  (*buffer_count)++;
  if ((*buffer_count) > (STRMAX - 1))
    {
      buffer[STRMAX] = '\n';
      printf (buffer);
      free (buffer);
      buffer = 0;		/* Assign pointer to 0 to ensure that it is not pointing to garbage */
      (*buffer_count) = 0;
      buffer = reallocateCharBuffer ();	/* Reallocate memory for new buffer */
    }
  return buffer;
}

int
main (int argc, char *argv[])
{
  char *buffer;
  int ch, first_asterix;
  int *buffer_count;
  FILE *inputFile;

  buffer_count = malloc (sizeof (int));
  *buffer_count = 0;
  first_asterix = 0;		// boolean for determining whether we are reading the first or second asterix in a row

  /* Read inputFile name. If none specified, use stdin */
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

  /* First initialization of buffer */
  buffer = reallocateCharBuffer ();

  while (1)
    {
      /* Read character from stdin / input file */
      ch = fgetc (inputFile);
      if (ferror (inputFile))	/* Exit if error reading stdin or input file */
	{
	  if (DEBUG)
	    {
	      perror ("Error reading input file");
	      return 1;
	    }
	}
      else if (feof (inputFile))	/* Exit if end of file is found */
	{
	  return 0;
	}

      if (ch == '*')
	{
	  if (!first_asterix)	/* Store information that the first asterix is found */
	    {
	      first_asterix = 1;
	    }
	  else			/* For this to work, when program encounters any other character, first_asterix has to be set to 0 */
	    {
	      buffer = insertCharIntoBuffer (buffer, buffer_count, '^');	/* Second asterix is found, so insert ^ to replace **  */
	      first_asterix = 0;
	    }
	}
      else
	{
	  if (first_asterix)
	    {
	      /* If previous character was a first_asterix and current 
	         character is not, handle previous character as if it is
	         a normal character (do not replace with anything, just insert * ) */
	      buffer = insertCharIntoBuffer (buffer, buffer_count, '*');
	    }
	  first_asterix = 0;	/* Assume that all asterix-es have been taken care of. */
	  if (ch == '\n')
	    {
	      buffer = insertCharIntoBuffer (buffer, buffer_count, ' ');	/* Handle carriage return / newlines */
	    }
	  else
	    {
	      buffer = insertCharIntoBuffer (buffer, buffer_count, ch);	/* Handle all other cases */
	    }
	}
    }
  return 0;
}
