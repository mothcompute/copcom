# i am using the testing name for this, copcom

copper compiler i guess. i use this because i am not a respectable amiga programmer. in fact at time of writing i am NOT an amiga programmer AT ALL!

the syntax is as follows:

```
m a8 a16       MOVE / you can put comments here!
w a8 a7 a7 a7  WAIT / VP HP VE (BFD in bit 6) HE
s a8 a7 a7 a7  SKIP / VP HP VE (BFD in bit 6) HE
```

`aX` is an operand with X bits. it can be any integer, and if prefixed with `x` it will be treated as hex.

leaving out arguments is undefined behavior, by which i mean i put in exactly zero safeguards for it because i am too lazy to make a proper wordizer!

example:
```
m x10 xaa55
w x30 x30 xff xff
s x30 x30 xff xff ASDKASL
s x20 x20 x10 x10 salkalk
```

the command line is as follows:

```
copcom in0 out0  in1 out1  in2 out2  # for as many input files as you have
```

public domain because this code sucks!
