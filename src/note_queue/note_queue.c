#include <stdint.h>

#include "../structures/bit_set.h"
#include "../structures/int_array.h"
#include "../voices/voice.h"
#include "note_queue.h"

void enqueueNoteOn(NoteQueue *nq, uint8_t note) {
  /* Note on queue wraps around but doesn't enqueue duplicate active notes. */
  int i = 0;
  if (isBitSet128(nq->encounteredOn, note)) { return; }
  i = nq->on.len & (VOICES_SIZE - 1);
  if (nq->on.len++ >= VOICES_SIZE) {
    /* Remove stolen notes */
    unsetBit128(nq->encounteredOn, nq->on.vals[i]);
  }
  nq->on.vals[i] = note;
  setBit128(nq->encounteredOn, note);
}

void enqueueNoteOff(NoteQueue *nq, uint8_t note) {
  /* Note off queue doesn't wrap around or enqueue duplicate notes. */
  if (isBitSet128(nq->encounteredOff, note)) { return; }
  setBit128(nq->encounteredOff, note);
  nq->off.vals[nq->off.len++] = note;
}

void noteQueue(NoteQueue *nq) {
  bitSet128(nq->encounteredOn);
  bitSet128(nq->encounteredOff);
  intArrayVoices(&nq->on);
  intArrayNotes(&nq->off);
}
