#include <unistd.h>

#include "voice.h"
#include "voice_list.h"

void appendVoiceList(VoiceList *vl, Voice *v) {
  if (vl->head == NULL) {
    vl->head = v;
    vl->last = v;
  } else {
    v->prev  = vl->last;
    vl->last = v;
  }
}

void removeVoiceList(VoiceList *vl, Voice *v) {
  if (v->prev == NULL && v->next == NULL) {
    /* One item list; clear it */
    voiceList(vl);
  } else if (v->prev == NULL) {
    /* Remove head of list */
    vl->head = v->next;
    v->next = NULL;
  } else if (v->next == NULL) {
    /* Remove last item of list */
    vl->last = v->prev;
    v->prev = NULL;
  } else {
    v->prev->next = v->next;
    v->next->prev = v->prev;
    v->prev       = NULL;
    v->next       = NULL;
  }
}

Voice * carVoiceList(VoiceList *vl) {
  Voice *v = vl->head;
  if (v != NULL) {
    vl->head = v->next;
    v->next = NULL;
  }
  return v;
}

void voiceList(VoiceList *vl) {
  vl->head = NULL;
  vl->last = NULL;
}
