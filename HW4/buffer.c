
#include <assert.h>
#include "buffer.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/**
 *  Initialize bounded buffer to a size specified by the user
 */
BoundedBuffer *
buffer_init (int size)
{
 
  if (size < 0 || size > BUFFER_SIZE)
    {
      perror ("Size specified is out of bounds");
      return (NULL);
    }

  BoundedBuffer *BBuffer =
    (struct boundedBuffer *) malloc (sizeof (struct boundedBuffer));

  BBuffer->b_size = size;
  BBuffer->emptyBuffers = (semaphore *) malloc (sizeof (semaphore));
  BBuffer->charBuffers = (semaphore *) malloc (sizeof (semaphore));
  createSem (BBuffer->emptyBuffers, 1);
  createSem (BBuffer->charBuffers, 0);
  BBuffer->nextIn = 0;
  BBuffer->nextOut = 0;
  BBuffer->charCount = 0;
  return BBuffer;
}

/** 
 * Dispose of buffer and its memory
 */
void
buffer_dispose (BoundedBuffer * BBuffer)
{
  free (BBuffer->emptyBuffers);
  free (BBuffer->charBuffers);
  free (BBuffer);
}

/**
 * Deposit character into buffer.
 */
void
buffer_deposit (BoundedBuffer * BBuffer, char *ch)
{
  // Wait for an empty space in buffer
  down (BBuffer->emptyBuffers);

  // Check that there really is at least 1 empty space in buffer
  assert (BBuffer->charCount < BBuffer->b_size);

  BBuffer->buf[BBuffer->nextIn] = ch[0];
  BBuffer->nextIn = (BBuffer->nextIn + 1) % BBuffer->b_size;


  BBuffer->charCount = abs (BBuffer->nextOut - BBuffer->nextIn);


  if (DEBUG_BUFFER)
    {
      printf ("depositing %s nextIn %i ", BBuffer->buf, BBuffer->nextIn);
      printf ("charCount %i \n", BBuffer->charCount);
    }

  // After putting a character in the buffer,
  // if there is at least 1 empty space in buffer the producer itself 
  // can release the emptyBuffers semaphore so that it can continue
  // producing
  if (BBuffer->charCount < BBuffer->b_size)
    up (BBuffer->emptyBuffers);

  // Check that there is at least 1 character in the buffer
  if (BBuffer->charCount > 0)	// Signal that there is at least 1 character in the buffer
    up (BBuffer->charBuffers);


}

/**
 * Remove character from buffer
 */
void
buffer_remove (BoundedBuffer * BBuffer, char *ch)
{
  // Wait for at least 1 character in the buffer
  down (BBuffer->charBuffers);
  // Check that there really is at least 1 character in the buffer
  assert (BBuffer->charCount > 0);
  ch[0] = BBuffer->buf[BBuffer->nextOut];
  ch[1] = '\0';
  BBuffer->nextOut = (BBuffer->nextOut + 1) % BBuffer->b_size;
  BBuffer->charCount = abs (BBuffer->nextOut - BBuffer->nextIn);

  if (DEBUG_BUFFER)
    {
      printf ("removing %s nextOut %i character %s ", BBuffer->buf,
	      BBuffer->nextOut, ch);

      printf ("charCount %i \n", BBuffer->charCount);

    }

  // After removing a character from the buffer,
  // if there is at least 1 character in buffer the consumer itself 
  // can release the charBuffers semaphore so that it can continue
  // consuming
  if (BBuffer->charCount > 0)
    up (BBuffer->charBuffers);

  // Check that there is at least 1 empty space in the buffer
  if (BBuffer->charCount < BBuffer->b_size)	// Signal that there is at least 1 empty space in the buffer
    up (BBuffer->emptyBuffers);

}
