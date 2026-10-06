# COS 217 Local Environment — Setup and Build Guide

This directory approximates Princeton's **armlab** Linux cluster, where COS
217 students build, test, debug, and submit. You cannot SSH to armlab, so
this setup substitutes your Mac's tools while keeping the *workflow* — the
commands, the files, and the habits — as close to the course as possible.

---

## 1. How this maps to armlab

| armlab (course)                | Your Mac (this setup)                          |
|--------------------------------|-------------------------------------------------|
| Linux (Ubuntu, ARM aarch64)    | macOS (Darwin, ARM apple silicon)              |
| `gcc217` (course gcc wrapper)  | `bin/gcc217` (this setup's wrapper → clang)    |
| `gcc` (GNU)                    | `cc` / `clang` (Apple; `gcc` is an alias for it)|
| `make`                         | `make` (already installed)                     |
| `gdb`                          | `lldb` (macOS's debugger; no `gdb` on macOS)    |
| `git`                          | `git` (already installed)                      |
| `emacs` (course editor)        | VS Code                                         |
| `submit` / `FeedbackCOS217.py` | n/a — self-study; no armlab account             |

The `gcc217` wrapper (in `bin/`, already on your `PATH` via `~/.zshrc`) runs:

```
cc -std=c90 -pedantic -Wall -Wextra ...
```

- `-std=c90` — **ISO C90**, the dialect the course and King's book use
- `-pedantic` — warns about GNU extensions that ISO C90 forbids (e.g. `//` comments)
- `-Wall -Wextra` — the warning hygiene the course grades on ("we will
  deduct points if gcc217 generates warning messages")

Open a **new terminal window** (or run `source ~/.zshrc`) so `gcc217` is on
your `PATH`. Verify:

```
$ gcc217 --version
Apple clang version ...          # this confirms the wrapper works
```

---

## 2. The four build stages (the heart of the course's build workflow)

A C program goes from source to executable in four stages. Assignment 1
requires you to run them **separately ("the long way")** at least once, so
learn this now. Each stage consumes the previous stage's output:

```
  decomment.c      decomment.i      decomment.s      decomment.o    decomment
  (source)   -->  (preprocessed)--> (assembly)  -->  (object)  -->  (executable)
                PREPROCESS       COMPILE          ASSEMBLE        LINK
                (gcc217 -E)      (gcc217 -S)      (gcc217 -c)     (gcc217)
```

### Stage 1: Preprocess — `gcc217 -E` → `.i`

The **C preprocessor** merges `#include` files, expands `#define` macros,
and (the subject of Assignment 1) removes comments. Output is still C, just
bigger and comment-free:

```
gcc217 -E decomment.c -o decomment.i
```

View it: `more decomment.i` (or open it in VS Code). Notice how your
comments are gone and header contents have been pasted in.

### Stage 2: Compile — `gcc217 -S` → `.s`

Translates (preprocessed) C into **assembly language** for your machine:

```
gcc217 -S decomment.i -o decomment.s
```

View it: `more decomment.s`. Readable-ish: look for your function names
(`_main`, etc.). This is where syntax errors and warnings are reported.

### Stage 3: Assemble — `gcc217 -c` → `.o`

Translates assembly into **object code** (machine instructions, but not
yet a runnable program):

```
gcc217 -c decomment.s -o decomment.o
```

`more decomment.o` shows gibberish — that is expected (the assignment says
the same about the last two files).

### Stage 4: Link — `gcc217` (no flag) → executable

Combines your object code with the C runtime library (e.g. where `getchar`
and `printf` live) and produces the executable:

```
gcc217 decomment.o -o decomment
```

### The shortcut

For everyday builds, one command does all four stages:

```
gcc217 decomment.c -o decomment        # or:  make
```

Use the shortcut while iterating; do the four-step version when you want to
see (and the assignment wants you to see) what each stage produces.

> **Tip for Assignment 1:** run your `decomment` on its own intermediates,
> e.g. `./decomment < decomment.c > out` — the assignment specifically asks
> you to test your program against its own source code.

---

## 3. Running and testing (filters, redirection, exit status)

`decomment` is a **filter**: it reads **stdin**, writes to **stdout**, and
sends errors/warnings to **stderr**. From the shell you control all three
streams with redirection:

```
./decomment < somefile.c > output 2> errors
```

- `< file` — feed `file` to the program's stdin
- `> file` — capture stdout to `file` (errors still show in the terminal)
- `2> file` — capture stderr to `file`

Check the **exit status** of the last command (`EXIT_SUCCESS` = 0,
`EXIT_FAILURE` = 1):

```
echo $?
```

Compare outputs with `diff` (as the assignment does):

```
diff -y expectedOutput myOutput      # side-by-side, differences marked
diff -q expectedOutput myOutput      # quiet: only says if they differ
```

Test against a big file (~2 million characters), as the assignment suggests:

```
for i in `seq 1 100000`; do echo "This is line $i."; done >> overflowbait
./decomment < overflowbait > output 2> errors
rm overflowbait output errors
```

### Course test files

`assignment1/` contains the course's official test suite (cloned from
https://github.com/COS217/Decomment): 60+ `.txt` inputs covering every
behavior in the spec (single-line comments, multi-line comments, comments
inside string/char literals, escapes, unterminated comments, ...), plus the
`readme` file you must eventually fill in.

To test your program against one of them, read the `.txt` file first to
know what behavior to expect, then:

```
./decomment < 03multilinecomment1.txt ; echo "exit: $?"
```

---

## 4. Building with make

`assignment1/Makefile` encodes the dependency graph:

```
decomment: decomment.o           ← link
decomment.o: decomment.c         ← compile+assemble (-c)
```

From `assignment1/`:

```
make              # build decomment (only rebuilds what changed)
make clean        # remove decomment, *.i, *.s, *.o
```

`make` is worth internalizing — the course covers it around week 4, and
every subsequent assignment uses it.

---

## 5. Debugging with lldb (in place of gdb)

The course teaches `gdb`. macOS doesn't support `gdb`, but `lldb` is
equivalent for everything you'll need. First, build with `-g` (the Makefile
already does) so debug info is embedded:

```
gcc217 -g decomment.c -o decomment
lldb ./decomment
```

Inside lldb:

| gdb (course) | lldb (yours)        | what it does                    |
|--------------|---------------------|---------------------------------|
| `break main` | `breakpoint set -n main` (or `b main`) | set a breakpoint on `main` |
| `run < f.txt`| `process launch -stdin f.txt` (or `run < f.txt` — works too) | run with stdin from a file |
| `next`       | `next` (or `n`)     | step over                       |
| `step`       | `step` (or `s`)     | step into                       |
| `print x`    | `expression x` (or `p x`) | print a variable          |
| `continue`   | `continue` (or `c`) | resume until next breakpoint    |
| `quit`       | `quit`              | leave lldb                      |

A typical session for decomment: `b main`, `run < 03multilinecomment1.txt`,
then `n` through the DFA loop, `p cPrev` / `p iLine` to watch your state
variables.

---

## 6. ISO C90 rules to live by

The compiler will catch violations, but know the big ones in advance:

1. **Comments are `/* ... */` only.** `//` is a C99 feature; `gcc217` warns
   on it. (Note the irony: your `decomment` program must *ignore* `//`
   while your own source must never *use* it.)
2. **Declare variables at the top of a block**, before any statements.
   `int i = 0; for (...)` then `int j;` later in the same block is a C99
   liberty; C90 wants all declarations first.
3. **No C99/C11 library functions.** `snprintf` is C99 — use `sprintf`
   (carefully), or `fprintf(stderr, ...)` directly. Stick to what King
   Ch. 1–7 and the lectures cover.
4. **Main returns `int`**, and `main(int argc, char *argv[])` if you use
   arguments. Use `EXIT_SUCCESS`/`EXIT_FAILURE` from `<stdlib.h>`.
5. **Every function gets a comment** describing what it does *from the
   caller's perspective* — parameters by name, return value, and any
   reading/writing of stdin/stdout/stderr. Not *how* it works.

### Course style rules the editor enforces

- **72-character line limit** — VS Code shows a ruler at column 72 in this
  workspace; keep text before it (green marker in course emacs = column 72
  OK; gray = too long → here, stay left of the ruler).
- **Spaces, not tabs** — this workspace indents 3 spaces per level
  (matching course sample code); it will never insert a tab character.
- Style is graded (K&P rules 1–24, summarized in the course's
  `asgts/style.pdf`).

---

## 7. VS Code in this workspace

Open the **`COS217` folder** (not `assignment1` alone) so the workspace
settings apply: `code /Users/Chris/Documents/Programming/COS217`

Install the recommended **C/C++** extension when prompted (provides
IntelliSense, error squiggles, debugging). It is configured for C90.

Useful tasks (Terminal → Run Task…, ⌘⇧B for the default build):

- **build (gcc217 shortcut)** — builds the open `.c` file to an executable
- **stage 1–4** — run the open file through each build stage individually,
  producing `.i` / `.s` / `.o` / executable
- **run as filter** — runs the open file's executable with
  `input.txt`, capturing `output.txt` and `errors.txt`

---

## 8. Differences from the real armlab (know these)

1. **`sampledecomment` won't run on your Mac.** It's an ARM *Linux* ELF
   binary; macOS can't execute Linux binaries. The course's
   `testdecomment` script diffs your program against it, so that script
   won't work here without a Linux runtime. Options if you want it later:
   install **Docker** or **OrbStack** (both run ARM Linux containers natively
   on your Apple-silicon Mac), then run the test suite inside a container.
   Until then, test against the spec examples + `.txt` files + the
   behaviors listed in the assignment — that covers everything.
2. **`clang` vs `gcc` diagnostics.** Warnings you see may be phrased
   differently than on armlab, and Apple clang treats a couple of things
   (like calling an undeclared library function) as errors where armlab's
   gcc warns. Your code should be warning-free under `gcc217` either way.
3. **`submit`, `FeedbackCOS217.py`** don't exist here — self-study, so
   there's nothing to submit. Still do the `readme` questions: they're
   good review.

---

## 9. Directory layout

```
COS217/
├── BUILD_GUIDE.md        this guide
├── bin/gcc217            the course's build command (on PATH via ~/.zshrc)
├── .vscode/              workspace settings (C90, rulers, tasks)
├── welcome.c             toolchain sanity check (lecture 1 homage)
├── welcome.i/.s/.o/...  products of the four-stage demo build
└── assignment1/          your working directory for Assignment 1
    ├── Makefile          build with: make
    ├── readme           course readme to fill in when done
    ├── *.txt            course test inputs (60+)
    ├── testdecomment    course test script (needs Linux; see §8)
    ├── testdecommentdiff
    ├── sampledecomment  reference binary (ARM Linux; see §8)
    └── decomment.c      ← yours to write (the DFA + program)
```

Workflow per assignment: work in `assignmentN/`, edit in VS Code, build with
`gcc217` or `make`, test with `.txt` files + `diff`, debug with `lldb`,
commit with `git`.

---

## 10. First commands to run (cheat sheet)

```
cd ~/Documents/Programming/COS217/assignment1

gcc217 decomment.c -o decomment          # shortcut build (once written)
make                                     # same thing via the Makefile

gcc217 -E decomment.c -o decomment.i     # the long way: preprocess
gcc217 -S decomment.i -o decomment.s     #               compile
gcc217 -c decomment.s -o decomment.o     #               assemble
gcc217 decomment.o -o decomment           #               link

./decomment < 01normal1.txt ; echo $?     # run a test, check exit status
./decomment < 12untermcomment1.txt        # unterminated-comment test
                                          # (exit status should be 1)
```

Happy de-commenting.
