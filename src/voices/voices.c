#include "voice.h"
#include "voices.h"

static Voice * voiceQueuePop(VoiceQueue *);
static void voiceQueuePush(VoiceQueue *, Voice *);
static Voice * voiceStackPop(VoiceStack *);
static void voiceStackPush(VoiceStack *, Voice *);

static Voice * voiceQueuePop(VoiceQueue *vq) {
  Voice *v = vq->voices[vq->readPos];
  vq->count--;
  vq->readPos = (vq->readPos + 1) & (VOICES_SIZE - 1);
  return v;
}

static void voiceQueuePush(VoiceQueue *vq, Voice *v) {
  vq->count++;
  vq->writePos = (vq->writePos + 1) & (VOICES_SIZE - 1);
  vq->voices[vq->writePos] = v;
}

static Voice * voiceStackPop(VoiceStack *vs) {
  return vs->voices[vs->count--];
}

static void voiceStackPush(VoiceStack *vs, Voice *v) {
  vs->voices[vs->count++] = v;
}

void voices(Voices *vs) {
  int i = 0;
  for (; i < VOICES_SIZE ; i++) {
    voice(&vs->voices[i]);
  }
}
