# Fifth
This is a simple forth-like interpreter I wrote inspired by a similar project by Timothy Lottes. (https://timothylottes.github.io/20141231.html)

The language is stack based. Functions pop zero or more arguments off the stack and push zero or more results back onto the stack.


Here's a quick example of some code the interpreter can handle:
```
\This is a comment\
\This is a function named square, that squares its argument\
{ square dup * }
x2 square printx \This will print x4 (numbers are in hexadecimal)\


"file" fopen :file-handle \Open 'file' and store the handle in the variable 'file-handle'\
.file-handle x6 "foobar" fwrite \Write string "foobar" of length 6 into 'file'\
.file-handle fclose
```

Functions may be prefixed with `'` to call them conditionally based on whether a value popped from the stack is non-zero.
Here is a Fizz buzz example:
```
{ == - if0 }
{ != == if0 }
"fizz" :fizz-str
"buzz" :buzz-str
" " :space-str
"
" :newline
{ fizz .fizz-str prints }
{ buzz .buzz-str prints }
{ print-i .i printx }
{ fizzbuzz :i .i x3 % if0 'fizz
  .i x5 % if0 'buzz
  .i x3 % if0 if0 .i x5 % if0 if0 & 'print-i \If divisible by neither, print i\
  .space-str prints }
{ fizzbuzz-loop dup fizzbuzz  x1 + dup x10 != 'fizzbuzz-loop }
x1 fizzbuzz-loop
.newline prints
```

And finally a list of built-in functions:
* `dup` (a -- a a) Duplicate the element on the top of the stack.
* `drop` (a -- ) Pop and discard the element on the top of the stack.
* `swap` (a b -- b a) Swap the top two elements of the stack.
* `over` (a b -- a b a) Push a copy of the second element of the stack.
* `if0` (a -- b) Same as ! operator in C.
* `prints` (string -- )  Pop string off stack and print it.
* `hash` (string -- ) Pop string and push its hash.
* `strcmp` Pop two strings and compare. Push 1 if true 0 if false.
* `fopen` (path -- file-handle)
* `fread` (file-handle size ptr -- bytes-read)
* `fwrite` (file-handle size ptr -- )
* `fclose` (file-handle -- )
* `fsize` (file-handle -- file-size)
* `fseek` (pos file-handle -- )
* `ftell` (file-handle -- pos)
* `exit` Exits with return code on top of stack.
* `eval` Pops string and evaluates it
* `allot` (size -- addr) Allocates size bytes and pushes address of allocation.
* `!` (value addr -- ) Stores value at address `addr`. 
* `@` (addr -- value) Fetches cell (qword / 8 bytes) at `addr` and pushes it to the stack.
* `+` (a b -- c) Adds top two elements of stack
* `-` (a b -- c) Subtraction
* `*` (a b -- c) Multiplication
* `/` (a b -- c) Division
* `%` (a b -- c) Modulo
* `>>` (a b -- c) Bitwise shift right
* `<<` (a b -- c) Bitwise shift left
* `&` (a b -- c) Bitwise and
* `|` (a b -- c) Bitwise or
* `^` (a b -- c) Bitwise xor
* `~` (a -- b) Bitwise not
