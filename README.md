The Barely Operational Audio REPL (BOAR) is a modest polyphonic synthesizer written in ANSI C. It opens a handle to a [sndio](http://www.sndio.org) device, drops the user in a shell, and allows him or her to issue quick and easy audio-generating commands. boar operates through stdin so it can accept piped output in the context of larger scripts. While it lacks many features that fully-fledged synths boast, it is a hassle-free easel to explore timbres with. The source code was also written to be terse and readable, which will hopefully demystify certain audio programming concepts to anybody curious enough to peer into it. 

## Installation

boar was written on OpenBSD with the sndio library as its sole target. If sndio is not already installed on your machine, you will have to get it through your distro's package manager. The C code itself should be largely standard; let me know if there's a BSDism that I didn't account for.

Installation is the typical process:

+ `tar xzvf boar.tar.gz`
+ `cd boar`
+ `make`
+ `make install` (may have to be root)
+ `make uninstall` (to remove)

## Basic usage

boar starts with sane defaults, but it has some command-line flags. Almost all of them take the form `-parameter n`, where `n` is an integer:

+ `bits`: The bit-depth of the audio output. Currently locked to 16bit signed ints.
+ `bufsize`: The size of the audio output buffer in frames. Shorter buffers result in more responsiveness, but are taxing upon system resources.
+ `chan`: The number of channels of audio output.
+ `echo-notes`: This flag requires no additional arguments. When enabled, note on/off commands are echoed to stdout, which allows multiple instances of boar to work in a pipeline and function as one instrument.
+ `polyphony`: The number of notes possible to play at once.
+ `rate`: The sample rate of the audio output.

sndio sends these flags to the hardware, which may disagree with some of your parameters. My soundcard won't accept a `bufsize` less than 960, for example. The program will adjust these settings accordingly.

Typing in `boar` starts the shell. There is no baked-in readline support, so you may want to run it with `rlwrap` to take advantage of command history:

    $ rlwrap boar
    boar: Welcome. You can exit at any time by pressing q + enter.

You can adjust the master volume (loudness) with `l`. Its arguments can be valued between 0.0 and 1.0:

    l 0.5

You shouldn't hear anything yet. You can change that with the `n` command, which takes a MIDI note as its argument:

    n 36
    n 48
    n 60
    n 72
    n 84
    n 96
    n 108
    n 120

A chord of sines should be playing now. These are the fundamental carrier waves. Each one of them has an associated modulator wave that can be adjusted through calls to the `p` and `m` functions. The argument to `p` sets the pitch ratio between a carrier and its modulator. If the carrier is oscillating at 440hz, then `p 2.0` will make its modulator 880hz:

    p 2.0

You shouldn't hear any change in the sound yet, because the modulation depth is set to 0.0. The intensity of modulation is addressed through the `m` command. Setting it to 1.0 means that the carrier frequency (f<sub>c</sub>) will be offset by the modulating frequency (f<sub>m</sub>) to the full extent of f<sub>m</sub>, since f<sub>m</sub> × 1.0 = f<sub>m</sub>. In other words, the f<sub>c</sub> valued at 440hz is having its frequency modulated ±880hz in a cycle of 880hz:

    m 1.0

The modulation should result in a squareish tone: almost like a church organ.

You can also get an actual square wave at any time by changing the wavetable number with `w` or `W`:

    w 1
    W 1

A full list of waves is available in `man boar`.

The lowercase command changes the carrier wave, while the uppercase one changes the modulator. You can set them back to sines with:

    w 0
    W 0

Modulators can have lower pitches than their carriers too:

    p 0.5

This should also sound like an organ, though with a distinct timbre.

    p 0.001
    m 400.0

After entering the above parameters, f<sub>m</sub> is so slow and the modulation is so pronounced that phase of the offset is perceptible to human hearing. This is low frequency oscillation, or an LFO. Playing around with these settings can yield truly alien tones:

    m 3.567
    p 1.51

Generally speaking, "non clean" ratios are more harmonically complex. If your ears are hurting at this point, you can turn notes off with `o`:

    o 120
    o 108
    o 96
    o 84
    o 72
    o 60
    o 48
    o 36

Entering:

    q

will quit you out. ctrl+c is always an option as well.

That's essentially it. Nothing you're going to play _Oxygene_ on, but a low-stakes way to drone away and get inspired.

## Advanced usage

[Envelopes](https://en.wikipedia.org/wiki/ADSR_envelope) give more dynamism to your performance. boar has attack/decay/sustain/release (ADSR) envelopes for both its carriers and its modulators. The commands:

    a
    d
    s
    r

set the ADSR of the carrier, and:

    A
    D
    S
    R

sets the ADSR of the modulator. All commands accept values between 0.0 and 1.0, where 0 indicates an instantaneous effect, and 1 is a 10 second long envelope stage.

Listen to how the following settings affect boar:

    a 0.05
    A 0.3
    D 0.3
    S 0.1
    R 0.35
    r 0.35
    p 2.0
    m 17.5

Giving different envelope settings to the modulator results in an effect similar to a filter sweep on an analog synthesizer.

Each A, D, and R stage increments/decrements in a linear manner by default, but their patterns actually just reference the same wavetables available in the `w` and `W` commands. It doesn't make too much musical sense, but you can have a sine curve as an attack, or random noise as a release. These tables can be changed by referencing a specific stage with the parameter, or `P` command. The following will tell boar to modify the modulator attack wave:

    P A

An integer value can be pushed to the parameter pointed to by `P` with the value command `v`. The following sets the modulator attack to a sine:

    v 0

This interface is a slightly unwieldy, but allows the performer to sculpt wave shapes unthinkable on traditional synthesizers.

You can also squeeze more sonic mileage out of boar by running multiple instances of it in a pipe. Running:

    rlwrap boar -echo-notes | boar

will chain together two boars, giving a 4 operator instrument with parallel carrier:modulator paths—similar to a certain algorithm on the Yamaha TX81Z. The `echo-notes` option ensures that both boars receive note commands at the same time.

Of course these extra oscillators are of no use unless they can be given distinct settings. The echo command does this:

    e p 0.001
    e m 400.0

Commands prefaced with "e" will not be evaluated in the local instance of boar, but echoed to stdout, where the next boar in the pipe can run them. Echoes can stack too:

    e e v 0.0

This will mute the third boar in the pipe.

Why not throw other things in the pipe? Consider the script `delay.sh`, which echoes its input back after $1 seconds:

    #!/bin/sh
    
    while read n
    do
        sleep $1
        echo $n
    done

Now you have a primitive delay effect:

    boar -echo-notes | ./delay 0.5 | boar

This pipe-friendly simplicity lends itself to all kinds of extensibility. You can write a script that translates MIDI keyboard input to boar on/off notes, then fire them off to a FIFO that boar is listening to. You can run `sed` somewhere in the pipeline to mess with echoed notes. Go hog wild with your imagination; the shell is your DAW.

## Caveats

Basic is beautiful, but I'd like to incorporate dithering, free-running LFOs, velocity-sensitive responses, etc. into boar eventually. Keep your nose to the ground for updates.

## Acknowledgements

+ [John Chowning](https://en.wikipedia.org/wiki/John_Chowning) is the man. His book _FM Theory and Applications_ was my bible as I learned the basics of synthesis.
+ [Curtis Roads](https://en.wikipedia.org/wiki/Curtis_Roads) rules too. His _Computer Music Tutorial_ is about as thick as 5 bibles.
+ [Vintage Synth Explorer](http://forum.vintagesynth.com) is full of knowledgeable and helpful people, even if you're not after a "vintage" sound.
+ Ditto for [KVR Audio](https://kvraudio.com).
