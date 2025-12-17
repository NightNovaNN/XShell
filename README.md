# XShell

**XShell** is a reality-aware command shell focused on **performance awareness, safety, and execution insight**.

Unlike traditional shells that merely run commands, XShell is designed to **observe, measure, and enforce discipline** around command execution.

> The shell shouldn’t just obey.  
> It should teach.

---

## Core Philosophy

XShell is built around three ideas:

1. **Time is a first-class constraint**
2. **Commands should be safe by default**
3. **Developers should understand where time actually goes**

This results in tooling that trains better intuition, not just faster typing.

---

## Features

### Ergonomic Primitives

- **`mkcd`** — create a directory and enter it
- **`extract`** — universal archive extraction
- **`count`** — line / file counter
- **`backup`** — timestamped backups

---

### Safety & State

- **`shadow`** — dry-run wrapper  
  Prints what *would* run without executing it.

- **Project Variables (`var`, `-v`)**  
  Project-scoped environment variables instead of global pollution.

---

### Performance Awareness

#### `budget` — timeboxed execution

Run a command with a strict time budget:

```sh
budget 5 make
````

If the budget is exceeded:

* the command is terminated
* the event is logged
* the developer is warned

This trains awareness of expensive operations.

---

#### `measure` — correct benchmarking

XShell benchmarking is **not naive**.

* runs commands multiple times
* discards warmup runs
* reports steady-state performance

```sh
measure make
```

Example output:

```sh
run 1: 0.23s
run 2: 0.21s
run 3: 0.22s
avg (steady): 0.22s
```

---

#### `profile` — execution anatomy *(planned)*

Not:

* how long
* what changed

But:

* **where the time went**

Example target output:

```sh
Command profile: make

Process startup:   12%
CPU execution:     63%
Disk I/O:          18%
Child processes:    7%

Hotspots:
- gcc (48%)
- ld  (11%)
- ar  (4%)

Total (steady-state): 1.92s
```

This provides X-ray vision into command execution.

---

## Platform Support

### Windows

* Native implementation using WinAPI
* `CreateProcess`, Job Objects, high-resolution timers
* No POSIX emulation required

### Linux / macOS

* POSIX implementation using `fork`, `exec`, `wait`
* Accurate timing via `CLOCK_MONOTONIC`

XShell uses **separate OS-specific cores** with a shared philosophy.

---

## Building (Windows / MinGW)

```powershell
gcc xshell_win.c -O3 -Wall -Wextra -o xshell.exe
```

Run:

```powershell
.\xshell.exe
```

---

## Project Structure (planned)

```sh
XShell/
├── xshell_win.c
├── xshell_posix.c
├── Makefile
└── README.md
```

---

## Status

XShell is in **early development**.
The core execution, measurement, and budgeting logic is functional.

Planned additions:

* `profile` command
* project variable persistence
* archive extraction
* cross-platform wrappers
* logging and history

---

## License

MIT


