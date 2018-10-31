# JCR6 for C

WIP

This is just an experiment and I am totally not even sure if I can manage to do this, but I gotta try.
Here I'll be setting up the basis library to use JCR6 in C. Now I'm very well aware that JCR6 is a file format as C unfriendly as can be as it was set up for programming languages much higher level than C, and many things JCR6 relies on are simply not supported in C. This means I'll have to improvise. Simply trying to translate the BlitzMax code or the Go code even the C# code won't do (oddly enough I even think the C# code is least suitable of all).

I'm also quite aware that the mapping system I set up for this, will very likely not be the fastest solution possible. Yet I did not wanna rely on hthash for a very important reason. I wanted the JCR6 library to be as flexible as possible from the start, so I want to be as self-reliant as possible, and keep all this as much as possible to the C core features only. At least for now. Of course, people who are willing to help me to speed it all up are very welcome to do so, once this library is on the move.

Oh yeah, it goes without saying, but don't expect a detail documentation until this is all fully operational :P
