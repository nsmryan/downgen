# downgen
Downgen is a toy 2D level generator for vertically scrolling games. It is inspired by
rxi's [blog article](https://rxi.github.io/level_generation_using_markov_chains.html)
in which they present a much nicer version written in Lua for the LOVE framework.

This repo's version is written in C (<300 lines in main.c, ~400 lines of dependencies,
according to [cloc](https://github.com/AlDanial/cloc))
and implements a simple markov transition system using an adjaceny matrix of transition
probabilities (encoded as a count of transitions occuring in the given seed level).


There is no particular reason to do something like this in C, but for some reason I very
much like putting together a few tiny C libraries to create something, especially
a visual effect.

In this case the libraries are a very easy to use GIF encoder called
[gifenc](https://github.com/lecram/gifenc), and my favorite command line argument
parser [optfetch](https://github.com/moon-chilled/OptFetch).


## Code Notes
There are some things to note about the code- some of which are probably a bit weird.

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



