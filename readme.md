# Wish Shell

Wish shell is a simple but lowkey capable shell I built to learn about system calls and processes.

## Commands

- `exit`: exit shell using this command
- `path <path1> [path2 ... pathN]`: add new paths to the shell
- `ch <dir>`: change the current directory

## Usage

When you execute wish, a prompt like this pops up:

```
wish {current-directory}>
```

From this moment, you can command with to do run anything:

```
wish {current-directory}> ls -a
```

You can redirect the output of wish using `>`:

```
wish {current-directory}> ls -a > out.o
```
