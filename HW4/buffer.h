
// A bounded-buffer abstract type to encapsulate 
// all of the implementation details of the buffer
// and export functions "deposit" and "remove"

#include "st.h"
#include "semaphore.h"
#define BUFFER_SIZE 20
#define DEBUG_BUFFER 0



struct boundedBuffer
{
  semaphore *emptyBuffers;      // Ensures that there is at least 1 empty space in the buffer
  semaphore *charBuffers;       // Ensures that there is at least 1 character in the buffer
  int buf[BUFFER_SIZE];         // Actual buffer
  int b_size;
  int nextIn;			// Only accessed by producer
  int nextOut;			// Only accessed by consumer
  int charCount;		// Edited by both producer and consumer. Potential critical section

};

typedef struct boundedBuffer BoundedBuffer;

BoundedBuffer *buffer_init (int size);
void buffer_deposit (BoundedBuffer * BBuffer, char *c);

void buffer_remove (BoundedBuffer * BBuffer, char *c);
