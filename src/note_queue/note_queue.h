#pragma once

#include <stdint.h>

#include "../structures/bit_set.h"
#include "../structures/int_array.h"

/* During the parse phase, it makes sense to grab all intended notes in one
 * go, then assign them to actual voices during the audio rendering stage
 * instead:
 * note-on → add to the on array, modulo VOICES_SIZE. This allows note barrages
 * exceeding VOICES_SIZE to be "stolen" according to age before they are even 
 * assigned. The len value can exceed VOICES_SIZE and is clamped during audio 
 * rendering. All the "surviving" notes are actually played.
 * note-off → added blindly to the off array, which is big enough to hold all
 * notes. Not all of these will remain here.
 * encountered → a bit set of all the activated voices from this parsing phase.
 * Generated after the "survival" stage of populating the on array, so holds a
 * max of VOICES_SIZE bits. When turning voices off, will check for membership
 * of the off note in this set. If the bit exists, it means the note was both
 * activated and deactivated during the same phase—this can happen with step
 * sequencers, etc. There may have to be special logic for this later */
typedef struct NoteQueue {
  BitSet128       encounteredOn;
  BitSet128       encounteredOff;
  IntArrayVoices  on;
  IntArrayNotes   off;
} NoteQueue;


void enqueueNoteOn(NoteQueue *, uint8_t);
void enqueueNoteOff(NoteQueue *, uint8_t);
void noteQueue(NoteQueue *);
