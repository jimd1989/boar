/* Defines the enums and structs used to manage envelope state. */

#pragma once

#include <stdbool.h>

#include "wave.h"

typedef enum EnvStage {

  /* Marks which stage of A/D/S/R an envelope is currently in. */

  ENV_ATTACK = 0,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE,
  ENV_FINISHED
} EnvStage;

typedef struct EnvStep {

  /* Represents one stage of an Env (see below). Every stage contains its own
   * wavetable, which it iterpolates a value from at each applyEnv() call. */

  float       Level;
  Wave        Wave;
} EnvStep;

typedef struct Env {

  /* Envelopes track user input and then apply themselves against an arbitrary
   * synthesis parameter, hereafter referred to as "x":
   *
   * - Attack is the amount of time that elapses between initial key press and
   *   the moment x affects the synthesis at its full value. Attacks are
   *   generally used to "fade in" parameters. 
   *
   * - Decay is engaged after the attack phase reaches its zenith. It is the
   *   amount of time that elapses between a completed attack and the moment
   *   x reaches its sustain value.
   *
   * - Sustain occurs after a decay finishes. It is a constant value that x is
   *   multiplied against as long as the key is held down.
   *
   * - Release is engaged after the key is released. It is the amount of time
   *   that elapses after key release for x to dwindle back down to 0.
   *
   *  All stages of the envelope are optional, but at least one attack, decay,
   *  or sustain value must be provided for x to have any effect.
   *
   *  The amount of modulation an envelope performs on its target is governed
   *  by Env.Depth, which is between 0.0 (none) and 1.0 (full).
   *
   *  If an Env is set to loop, it will cycle through the AD stages until the
   *  key is released.
   *
   *  In terms of the program, every Voice has an individual Env struct for each
   *  parameter it expects to be modified by key events. Env.EnvStage and
   *  Env.Phase keep track of the local envelope state, while Env.Attack, 
   *  Env.Decay, Env.Sustain, and Env.Release point to externally-defined float
   *  values in a master Envs struct. This allows the program to change envelope
   *  settings for all voices in O(1) time. Env.Phase is also the value that is
   *  multiplied against external parameter x at any given time. */

  EnvStage      Stage;
  float         Phase;
  bool        * Loop;
  float       * Depth;
  EnvStep     * Attack;
  EnvStep     * Decay;
  float       * Sustain;
  EnvStep     * Release;
  Wave        * Wave;
} Env;

typedef struct Envs {

  /* Envs contains master values that Voice-local Env types point to. */

  bool                Loop;  
  float               Depth;
  unsigned int        Rate;
  EnvStep             Attack;
  EnvStep             Decay;
  float               Sustain;
  EnvStep             Release;
} Envs;

float applyEnv(Env *);
void resetEnv(Env *);
void retriggerEnv(Env *);
void setLoop(Envs *, const bool);
void setDepth(Envs *, const float);
void setAttackLevel(Envs *, const float);
void setDecayLevel(Envs *, const float);
void setSustainLevel(Envs *, const float);
void setReleaseLevel(Envs *, const float);
void setAttackWave(Envs *, const int);
void setDecayWave(Envs *, const int);
void setReleaseWave(Envs *, const int);
void makeEnv(Envs *, Env *);
void makeEnvs(Envs *, const unsigned int);
