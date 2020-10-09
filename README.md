# downgen
Downgen is a toy 2D level generator for vertically scrolling games. It is inspired by
rxi's [blog article](https://rxi.github.io/level_generation_using_markov_chains.html)
in which they present a much nicer version written in Lua for the LOVE framework.


This repo's version is written in C (<400 lines of my code, ~400 lines of dependencies,
according to [cloc](https://github.com/AlDanial/cloc))
and implements a simple markov transition system using an adjaceny matrix of transition
probabilities (encoded as a count of transitions occuring in the given seed level).


There is no particular reason to do something like this in C, but for some reason I very
much like putting together a few tiny C libraries to create something, especially
a visual effect.

In this case the libraries are a very easy to use GIF encoder called
[gifenc](https://github.com/lecram/gifenc), and my favorite command line argument
parser [optfetch](https://github.com/moon-chilled/OptFetch).

## Building
There is a Makefile:
```bash
make
```
which creates the executable 'downgen'. If you don't like make, feel free to enter:
```bash
cc -O0 -g -o downgen deps/optfetch/optfetch.c deps/gifenc/gifenc.c main.c table.c -Ideps/optfetch -Ideps/gifenc
```
which is all the Makefile does. The -O0 is only used to make the code more debuggable, while -O3 seems
to bring about a 2x speedup on my machine.

## Usage
The help printed by downgen documents its usage. Note that if you give no arguments it will still
generate a gif. The gif is always called 'level.gif'.
```bash
$ ./downgen --help
Usage: downgen [OPTION]...
  Create a gif of a vertically scrolling level from a given input level

  --file, -f FILE    Use the given file as the input.
                     The file should contain 0's and 1's, one column per
                     line, with the same number of characters in each line
  --dim,-d  N        Set the GIF dimensions (width and height in pixels of each block.
                     For example, 5 makes each cell in the output a 5x5 pixel block
                     Defaults to 20.
  --height,-h N      Set the number of rows in the output image
                     Defaults to 50
  --speed,-s N       Set the speed of the gif- 1 means 10ms per frame
                     Defaults to 10
  --print,-p         Print out transition table information
  --help             Print this help message

```

The input files look like level1.txt, level2.txt, and level3.txt in the repo- they
are a series of 0 and 1 characters, in same-width columns, separated by newlines.

A 1 shows up as a green square, and a 0 as a black square.

## Code Notes
Just some notes on the code and the process of creating this tool:


One is that I have been getting into the habit of checking against constants by
using the constant as the first argument to conditionals, such as
```c
if (NULL == file)
```
instead of
```c
if (file == NULL)
```
just because some safety-critical coding standards suggest this, and if it helps
in even a small number of cases, I might as well make it a habit.

## The Name
I've been playing a very fun game called [DownWell](https://downwellgame.com/),
so when I was thinking of a name for this tool, the word 'down' came to mind.
It generates levels in which one would go down.



