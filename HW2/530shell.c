/**
Implemented forking of child process. Child process expects the first input argument to be a full file path. If the first input argument is not a valid file path (using the stat command), child process assumes that the first input argument could be a file name and appends each entry in the environment PATH (using the getenv command) to the first input argument. If the first input argument is not in any of the path entries in the environment's PATH, then the child process prints an error message to the stdout.
**/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#define DEBUG 0

#define STRING_INIT 200
#define INPUT_LIMIT 200
int
main (int argc, char *argv[])
{
  /* Allow alternative input stream for testing. Uses stdin by default */
  FILE *inputFile;

  /* Forked child process's PID */
  int childPID;

  /* Command prompt sent to stdout for every time the process is done processing the user input and is waiting for another command */
  char *commandprompt = (char *) malloc (sizeof (char) * STRING_INIT);

  /* Data structure for book-keeping information about line read from input stream */
  char *inputStr;
  size_t *inputStrLen;
  int getlineResult;

  /* Data structure for parsing line from input stream and separating them into tokens to be sent to the executing process */
  char **arguments;		/* Array of arguments sent to execvp */
  char *filename = NULL;	/* Name of process to be executed */
  char *tmpToken = NULL;	/* Temporary token for testing for white space */
  char *whitespace = (char *) malloc (sizeof (char) * 7);	/* c-string of white space delimiters */
  int tokenIndex;
  struct stat *buf;

  char *envpath;		/* c-string to store environment */
  char *pathToken = NULL;	/* temporary token to store path entry */
  int statResult;
  /* Store result of executing process */
  int execvResult;

  /* Store result of waiting */
  int *waitStatus;
  int waitResult;
  /* Write to c-strings */
  sprintf (commandprompt, "Enter a command:\n");
  sprintf (whitespace, " \f\n\r\t\v");

  /* Read inputFile name. If none specified, use stdin. This allows me to use input files for debugging. */
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

  /* Inintial allocation of memory for input string */
  inputStr = (char *) malloc (sizeof (char) * STRING_INIT);
  inputStrLen = (size_t *) malloc (sizeof (size_t));

  /* Output a prompt to stdout */
  printf ("%s", commandprompt);

  /* Set initial value of input command length. This is only a suggestion to getline. Getline reallocates memory if the required size is larger. */
  if (inputStrLen)
    *inputStrLen = STRING_INIT;

  while (inputFile != NULL)
    {
      getlineResult = getline (&inputStr, inputStrLen, inputFile);
      if (getlineResult == EOF)	/* Terminate if EOF is reached */
	{
	  break;
	}
      else if (getlineResult == (-1))	/* will probably never happen as getline treats EOF and -1 as the same */
	{
	  perror ("Error reading input:");
	  continue;
	}

      childPID = fork ();
      if (childPID == (-1))
	{
	  perror ("Error creating child thread");
	}
      else if (childPID == 0)	/* Execution block for child process */
	{
	  arguments = (char **) malloc (sizeof (char *) * INPUT_LIMIT);


	  tokenIndex = 0;

	  arguments[tokenIndex] = (char *) strtok (inputStr, whitespace);

	  if (arguments[tokenIndex] == NULL)	/* End child process if the first command is null i.e. no process name is given. */
	    {
	      free (arguments);
	      exit (1);
	    }

	  tokenIndex++;

	  while ((tmpToken = (char *) strtok (NULL, whitespace)) != NULL)	/* Tokenize by whitespace */
	    {
	      arguments[tokenIndex] = tmpToken;
	      tokenIndex++;

	      if (tokenIndex > INPUT_LIMIT)	/* If the number of tokens is larger than what the program supports (INPUT_LIMIT - 1 because this includes the first argument which is the file name), print an error message */
		{
		  printf
		    ("Sorry, this shell can only handle commands with a maximum of %d arguments.\n",
		     (INPUT_LIMIT - 1));
		  free (arguments);
		  exit (1);
		}
	    }
	  /* Allocate memory for buffer of file */
	  filename = (char *) malloc (sizeof (char) * STRING_INIT);
	  sprintf (filename, "%s", arguments[0]);

	  /* Allocate memory for buffer of file status */
	  buf = (struct stat *) malloc (sizeof (struct stat));

	  /* If file name not valid it could mean that the user did not include path in his query. Search for file in the env $PATH */
	  if ((statResult = stat (arguments[0], buf)) == (-1))
	    {
	      free (buf);
	      free (filename);
	      filename = (char *) malloc (sizeof (char) * STRING_INIT);
	      buf = (struct stat *) malloc (sizeof (struct stat));

	      /* No need to allocate memory for envpath as getenv returns the pointer to the env value in the shell */
	      envpath = getenv ("PATH");
	      if (envpath != NULL)
		{
		  pathToken = (char *) strtok (envpath, ":");
		  if (pathToken != NULL)
		    {
		      if (DEBUG)
			printf ("%s/%s", pathToken, arguments[0]);
		      sprintf (filename, "%s/%s", pathToken, arguments[0]);
		      if ((statResult = stat (filename, buf)) == (-1))
			{
			  free (buf);
			  free (filename);
			  filename =
			    (char *) malloc (sizeof (char) * STRING_INIT);
			  buf = (struct stat *) malloc (sizeof (struct stat));


			  while ((pathToken =
				  (char *) strtok (NULL, ":")) != NULL)
			    {
			      sprintf (filename, "%s/%s", pathToken,
				       arguments[0]);
			      if ((statResult = stat (filename, buf)) != (-1))
				{
				  break;
				}
			      if (DEBUG)
				printf ("Did not find file in %s", pathToken);
			      free (buf);
			      free (filename);
			      filename =
				(char *) malloc (sizeof (char) * STRING_INIT);
			      buf =
				(struct stat *) malloc (sizeof (struct stat));

			    }
			  if (statResult == (-1))
			    {
			      free (buf);
			      free (filename);
			      free (arguments);
			      perror ("Could not find the specified file");
			      exit (1);
			    }




			}
		    }
		}
	      else
		{
		  free (buf);
		  free (filename);
		  free (arguments);
		  perror ("Could not find the specified file");
		  exit (1);
		}


	    }

	  /* execvp input arguments takes the filename as the first argument and also the first element of the arguments array */
	  execvResult = execv (filename, arguments);


	  /* Print error message if the execution failed. */
	  if (execvResult == -1)
	    {
	      perror ("Error has occurred");
	      free (buf);
	      free (filename);
	      free (arguments);
	      exit (1);
	    }
	  free (buf);
	  free (filename);
	  free (arguments);
	  exit (0);

	}
      else
	{
	  waitStatus = (int *) malloc (sizeof (int));
	  if ((waitResult = wait (waitStatus)) == (-1))
	    perror ("Error:");

	  free (waitStatus);
	  free (inputStr);
	  free (inputStrLen);

	  /* Subsequent allocation of memory for input string. */
	  inputStr = (char *) malloc (sizeof (char) * STRING_INIT);
	  inputStrLen = (size_t *) malloc (sizeof (size_t));

	  printf ("%s", commandprompt);
	  if (inputStrLen)
	    *inputStrLen = STRING_INIT;
	}

    }
  free (inputStr);
  free (inputStrLen);
  free (whitespace);
  free (commandprompt);
  return 0;
}
