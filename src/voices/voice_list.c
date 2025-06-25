#include <err.h>

#include <stdio.h>
#include <unistd.h>

#include "voice.h"
#include "voice_list.h"

void appendVoiceList(VoiceList *vl, Voice *v) {
  if (vl->head == NULL) {
    /* empty list */
    vl->head = v;
    vl->last = v;
    /* vl->head->prev = NULL; */
    /* vl->last->next = NULL; */
  } else {
    v->prev        = vl->last;
    vl->last->next = v;
    vl->last       = v;
  }
}

void removeVoiceList(VoiceList *vl, Voice *v) {
  if (v->prev == NULL && v->next == NULL) {
    /* Single-item list: blank it */
    voiceList(vl);
  } else if (v->prev == NULL) {
    /* Head of list */
    warnx("--HEAD--");
    vl->head       = v->next;
    vl->head->prev = NULL;
  } else if (v->next == NULL) {
    warnx("--LAST--");
    /* End of list */
    vl->last       = v->prev;
    vl->last->next = NULL;
  } else {
    warnx("--MIDDLE--");
    v->prev->next = v->next;
    v->next->prev = v->prev;
  }
  v->prev = NULL;
  v->next = NULL;
}

Voice * carVoiceList(VoiceList *vl) {
  Voice *v = vl->head;
  if (v != NULL) {
    vl->head       = v->next;
    vl->head->prev = NULL;
    v->prev  = NULL;
    v->next  = NULL;
  }
  return v;
}

void voiceList(VoiceList *vl) {
  vl->head = NULL;
  vl->last = NULL;
}

void printVoiceList(VoiceList *vl) {
  int i = 0;
  Voice *v = vl->head;
  while (v != NULL) {
    printf("%p:%d → ", (void*)v, v->note);
    v = v->next;
    i++;
    if (i > 8) {
      printf("∞\n"); /* Not good! */
      return;
    }
  }
  printf("∅\n");
}
