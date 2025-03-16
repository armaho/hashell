# HaShell

HaShell is a simple but lowkey capable shell I built to learn about system calls and processes.

## Commands

- `exit`: exit shell using this command
- `path <path1> [path2 ... pathN]`: add new paths to the shell
- `ch <dir>`: change the current directory

## Usage

When you execute HaShell, a prompt like this pops up:

```
HaShell {current-directory}>
```

From this moment, you can command with to do run anything:

```
HaShell {current-directory}> ls -a
```

You can redirect the output of HaShell using `>`:

```
HaShell {current-directory}> ls -a > out.o
```

HaShell also supports piping, you can pipe multiple commands using `|`:

```
HaShell {current-directory}> ls -a | sort
```
