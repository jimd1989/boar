/* Functions related to management of Env types. For more detailed information,
 * consult "envelope.h". */

#include <stdbool.h>

#include "envelope.h"

#include "constants/defaults.h"
#include "constants/maximums.h"
#include "numerical.h"
#include "synthesis.h"
#include "wave.h"

static float envSpeed(const unsigned int, const float);
static void setEnvLevel(const unsigned int, EnvStep *, const float);
static void incrementAttack(Env *);
static void incrementDecay(Env *);
static void incrementRelease(Env *);

static float
envSpeed(const unsigned int rate, float f) {

/* Derives the increment value of an envelope stage, and hence its speed, from
 * the overall sample rate of the program "rate", and a user-specified float
 * value between 0.0 and 1.0 "f", where "f" is the number of seconds the
 * envelope stage should last. Due to float (not double) resolution, the
 * timing becomes inaccurate above 30 seconds. Unsure if longer envelopes are
 * worth converting all phase to a higher resolution. */

  if (!f) {
    f = 0.000001f;
  }
  return 1.0f / ((float)rate * f);
}

static void
setEnvLevel(const unsigned int rate, EnvStep *est, const float f) {

/* Sets the speed of a specific envelope stage in terms of argument f. f must
 * be less than 1.0 or else it will reference an out of bounds index of
 * WAVE_EXPONENTIAL in expCurve(). f is capped at 1.0 at the REPL level with
 * truncateFloat(). Make sure that this is the case for all user-facing 
 * procedures that call setEnvParameter(). */

  est->Level = envSpeed(rate, f);
}

static void
incrementAttack(Env *e) {

/* Increments Env.Phase according to envelope's attack speed. */

  e->Phase = truncateFloat(e->Phase + e->Attack->Level, 0.99f);
  if (e->Phase >= 0.99f) {
    e->Stage = ENV_DECAY;
    e->Wave = &e->Decay->Wave;
  }
}

static void
incrementDecay(Env *e) {

/* Decrements Env.Phase according to envelope's decay speed. */ 

  e->Phase = liftFloat(e->Phase - e->Decay->Level, *e->Sustain);
  if (e->Phase == *e->Sustain) {
    if (*e->Loop) {
      e->Stage = ENV_ATTACK;
      e->Phase = 0.0f;
    } else {
      e->Stage = ENV_SUSTAIN;
    }
  }
}

static void
incrementRelease(Env *e) {

/* Decrements Env.Phase according to envelope's release speed. */ 

  e->Phase = liftFloat(e->Phase - e->Release->Level, 0.0f);
  if (e->Phase == 0.0f) {
    e->Stage = ENV_FINISHED;
  }
}

static void
incrementEnv(Env *e) {

/* Polls the envelope stage, runs the appropriate Env increment function. */

  switch((unsigned int)e->Stage){
    case ENV_ATTACK:
      incrementAttack(e);
      break;
    case ENV_DECAY:
      incrementDecay(e);
      break;
    case ENV_SUSTAIN:
      break;
    case ENV_RELEASE:
      incrementRelease(e);
      break;
  }
}

float
applyEnv(Env *e) {

/* Returns a sample from the envelope's stage's wavetable based upon the
 * envelope's phase. Weights it according to Env.Depth. */

  float level = 0.0f;

  incrementEnv(e);
  switch((unsigned int)e->Stage){
    case ENV_ATTACK:
      level = interpolateCycle(&e->Attack->Wave, e->Phase);
      break;
    case ENV_DECAY:
      level = interpolateCycle(&e->Decay->Wave, e->Phase);
      break;
    case ENV_SUSTAIN:
      return *e->Sustain;
    case ENV_RELEASE:
      level = interpolateCycle(&e->Release->Wave, e->Phase);
      break;
  }
  
  return 1.0f - ((1.0f - level) * *e->Depth);
}

void
resetEnv(Env *e) {

/* Sets an Env back to attack mode with 0.0 phase. */    

  e->Phase = 0.0f;
  e->Stage = ENV_ATTACK;
}

void
retriggerEnv(Env *e) {

/* Sets an Env back to attack, but leaves its phase. */

  e->Stage = ENV_ATTACK;
}
void
setLoop(Envs *es, const bool enabled) {

/* Sets the loop state of an envelope. */

  es->Loop = enabled;
}

void
setDepth(Envs *es, const float level) {

/* Sets the modulation depth of an envelope. */  

  es->Depth = truncateFloat(level, 1.0f);
}

void
setAttackLevel(Envs *es, const float f) {

/* Sets the attack speed for an envelope. */

  setEnvLevel(es->Rate, &es->Attack, liftFloat(f, 0.0f));
}

void
setDecayLevel(Envs *es, const float f) {

/* Sets the decay speed for an envelope. */

  setEnvLevel(es->Rate, &es->Decay, liftFloat(f, 0.0f));
}

void
setSustainLevel(Envs *es, const float f) {

/* Sets the sustain level for an envelope. */

  es->Sustain = truncateFloat(f, 1.0f);
}

void
setReleaseLevel(Envs *es, const float f) {

/* Sets the release speed for an envelope. */

  setEnvLevel(es->Rate, &es->Release, liftFloat(f, 0.0f));
}

void
setAttackWave(Envs *es, const int wt) {

/* Sets the wavetable of the attack stage. */

  selectWave(&es->Attack.Wave, wt);
}

void
setDecayWave(Envs *es, const int wt) {

/* Sets the wavetable of the attack stage. */

  selectWave(&es->Decay.Wave, wt);
}

void
setReleaseWave(Envs *es, const int wt) {

/* Sets the wavetable of the attack stage. */

  selectWave(&es->Release.Wave, wt);
}

void
makeEnv(Envs *es, Env *e) {

/* Assigns an Env pointers to all the fields in an Envs. */

  e->Loop = &es->Loop;
  e->Depth = &es->Depth;
  e->Attack = &es->Attack;
  e->Decay = &es->Decay;
  e->Sustain = &es->Sustain;
  e->Release = &es->Release;
  e->Stage = ENV_FINISHED;
}

void
makeEnvs(Envs *es, const unsigned int rate) {

/* Assigns Envs its sample rate and sets it to a sustain-only configuration. */

  es->Loop = false;
  es->Depth = 1.0f;
  es->Rate = rate;
  setAttackLevel(es, 0.0f);
  setDecayLevel(es, 0.0f);
  setSustainLevel(es, 0.99f);
  setReleaseLevel(es, 0.0f);
  setAttackWave(es, WAVE_TYPE_SAW);
  setDecayWave(es, WAVE_TYPE_SAW);
  setReleaseWave(es, WAVE_TYPE_SAW);
}
