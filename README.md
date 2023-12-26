The Barely Operational Audio REPL (BOAR) is a modest polyphonic synthesizer written in C. It opens a handle to a [sndio](http://www.sndio.org) device, drops the user in a shell, and allows him or her to issue quick and easy audio-generating commands. boar operates through stdin so it can accept piped output in the context of larger scripts. While it lacks many features that fully-fledged synths boast, it is a hassle-free easel to explore timbres with. The source code was also written to be terse and readable, which will hopefully demystify certain audio programming concepts to anybody curious enough to peer into it. 

## Installation

boar was written on OpenBSD with the sndio library as its sole target. If sndio is not already installed on your machine, you will have to get it through your distro's package manager. The C code itself should be largely C89, but some BSD extensions were used. A different C standard may have to be specified with `-lbsd` in the Makefile if building on other OSes.

Installation is the typical process:

+ `make`
+ `make install` (may have to be root)
+ `make uninstall` (to remove)

## Basic usage

boar starts with sane defaults, but it has some command-line flags. Almost all of them take the form `-parameter n`, where `n` is an integer:

+ `polyphony`: The number of notes possible to play at once. Users are advised to set a value high enough to avoid voice stealing, which could have unpleasant clicks.
+ `rate`: The sample rate of the audio output. Setting a very low sample rate can actually have a rather pleasant effect, but you will hear buzzing and pitch aberrations with non-blocking IO. Recompile the program to block IO by editing the final argument of `sio_open` in `audio_init.c` from `true` to `false`.
+ `blocks`: boar will attempt to be as responsive as possible, defaulting to the minimum buffer size allowed by your soundcard settings. This might be too difficult to keep up with though. If you encounter glitctching audio, run bloar with `- blocks n`, where `n` will be an integer multiple of the minimum buffer size. The larger this value, the less responsive boar will be to live input.

sndio sends these flags to the hardware, which may disagree with some of your parameters. My soundcard won't accept a `bufsize` less than 960, for example. The program will adjust these settings accordingly.

Typing in `boar` starts the shell. There is no baked-in readline support, so you may want to run it with `rlwrap` to take advantage of command history:

    $ rlwrap boar
    boar: Welcome. You can exit at any time by pressing q + enter.

You can adjust the master volume (loudness) with `l`. Its arguments can be valued between 0.0 and 1.0:

    l 0.5

Whitespace doesn't matter too much. These are valid functions as well.

    l0.5
       l   0.5

You shouldn't hear anything yet. You can change that with the `n` command, which takes a MIDI note as its argument:

    n 36
    n 48
    n 60
    n 72
    n 84
    n 96
    n 108
    n 120

A chord of sines should be playing now. These are the fundamental carrier waves. Each one of them has an associated modulator wave that can be adjusted through calls to the `P` and `L` functions. The argument to `P` sets the pitch ratio between the modulator and the note played. If the note's frequency is 440hz, then `P 2.0` will make the modulator 880hz:

    P 2.0

You shouldn't hear any change in the sound yet, because the modulation depth is set to 0.0. The intensity of modulation is addressed through the `L` command. Setting it to 1.0 means that the carrier frequency (f<sub>c</sub>) will be offset by the modulating frequency (f<sub>m</sub>) to the full extent of f<sub>m</sub>, since f<sub>m</sub> × 1.0 = f<sub>m</sub>. In other words, the f<sub>c</sub> valued at 440hz is having its frequency modulated ±880hz in a cycle of 880hz:

    L 1.0

The modulation should result in a squareish tone: almost like a church organ.

You can also get an actual square wave at any time by changing the wavetable number with `w` or `W`:

    w 2
    W 2

A full list of waves is available in `man boar`.

The lowercase command changes the carrier wave, while the uppercase one changes the modulator. You can set them back to sines with:

    w 1
    W 1

Modulators can have lower pitches too:

    P 0.5

This should also sound like an organ, though with a distinct timbre.

    P 0.001
    L 400.0

After entering the above parameters, f<sub>m</sub> is so slow and the modulation is so pronounced that phase of the offset is perceptible to human hearing. This is low frequency oscillation, or an LFO. Playing around with these settings can yield truly alien tones:

    L 3.567
    P 1.51

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

sets the ADSR of the modulator. All commands are in seconds.

Listen to how the following settings affect boar:

    a 0.05
    A 0.3
    D 0.3
    S 0.1
    R 0.35
    r 0.35
    P 2.0
    L 17.5

Giving different envelope settings to the modulator results in an effect similar to a filter sweep on an analog synthesizer.

Each A, D, and R stage increments/decrements in a linear manner by default, but their patterns actually just reference the same wavetables available in the `w` and `W` commands. It doesn't make too much musical sense, but you can have a sine curve as an attack, a backwards sine as decay, and random noise as a release:

    A. 1
    D. -1
    R. 7

Use `a. d. r.` for carriers and `A. D. R.` for modulators.

## Typical routing

When it comes to actual performance, I rarely find myself using a direct boar shell. More often than not, I have it listening to a FIFO, which allows boar to be a "server" with multiple "clients." Consider the following example, with a pipe of two `boar` commands listening to a FIFO `synth`:

    mkfifo synth
    boar <> synth
    boar: Welcome. You can exit at any time by pressing q + enter.

Then from another shell, I create a dumb repl with `cat`, which allows me to issue multiple operations on a single line—a far saner way of editing patches:

    rlwrap cat > synth
    a0.5;d0.5;s0.5;r0.5
    n60;n65;n69
    o60;o65;n69

I usually don't play notes in this repl though; I just use it for configuring settings. Something like [pop](https://github.com/jimd1989/pop) or [boar-midi](https://github.com/jimd1989/boar-midi) can simultaneously send score data into the `synth` FIFO instead.

And since everything is just FIFOs and pipes, there's nothing stopping `synth` from listening to `nc` and having a friend duet with you from his/her own machine. You aren't limited to a single carrier:modulator pair either of course. Use `tee` to send the same notes to multiple instances of `boar`. The entire Unix ecosystem is now your DAW.

## And that's not all

Make sure to read `man boar` too. There are even more features, such as touch sensitivity, fixed-rate operators, and reverse waves that are documented there.

## Acknowledgements

+ [John Chowning](https://en.wikipedia.org/wiki/John_Chowning) is the man. His book _FM Theory and Applications_ was my bible as I learned the basics of synthesis.
+ [Curtis Roads](https://en.wikipedia.org/wiki/Curtis_Roads) rules too. His _Computer Music Tutorial_ is about as thick as 5 bibles.
+ [Vintage Synth Explorer](http://forum.vintagesynth.com) is full of knowledgeable and helpful people, even if you're not after a "vintage" sound.
+ Ditto for [KVR Audio](https://kvraudio.com).
