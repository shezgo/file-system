# Custom File System

This project is a Unix-like, simple file system implementation written in C, built on top of a simulated block device, a low level partition system which was provided by Robert Bierman, professor at SFSU. I completed this individually as a main project for Operating systems class with the exception of the low level partition system.

The shell provides an interactive command-line interface to navigate, create, read, and manage files and directories stored within a volume file on disk.

---

## Free Space Bitmap and Contiguous Allocation

The file system tracks which disk blocks are free or in use using a **free space bitmap** — a compact array of bits where each bit corresponds to one disk block. A `0` bit means the block is free; a `1` bit means it is occupied.

### Layout on Disk

```
Block 0:        Volume Control Block (VCB) — filesystem metadata
Blocks 1–N:     Bitmap — one bit per block in the entire volume
Block N+1 onward: Root directory, then user files and directories
```

The bitmap itself occupies as many blocks as needed to represent every block in the volume. On initialization, blocks 0 through N (the VCB and bitmap blocks themselves) are pre-marked as used.

### Contiguous Allocation

When a file or directory needs space, `fsAlloc(n)` performs a **linear first-fit search** through the bitmap looking for `n` consecutive free bits. Once a run of `n` free blocks is found, all of them are marked used atomically and the starting block number is returned.

- **Directories** are always allocated **5 contiguous blocks**.
- **Files** are always allocated **10 contiguous blocks** at creation time, regardless of initial content.

This means every allocation is a single contiguous region on disk — there is no fragmentation within a single file or directory, and no indirection tables or extent lists. The tradeoff is that after repeated creates and deletes, free space can become fragmented into gaps smaller than 5 or 10 blocks, potentially causing allocations to fail even when total free space is sufficient. 

This design decision was made weighing the time and energy costs of implementing extents against creating other projects for potential employers, as I would need to refactor the free space allocation system, bitmap functions, and directory entry structure - additionally, each file would also need an additional black to store its extent list, and more. However, doing so would both improve space efficiency and size caps.

Every bitmap change is flushed to disk immediately, and the Volume Control Block's free block count is updated in tandem.

---

## Building and Running

### Prerequisites

- GCC
- GNU Make
- `readline` library (`libreadline-dev` on Debian/Ubuntu)
- *Note, this project was built in a Linux virtual machine.

### Commands

```bash
# Build the executable
make

# Build and run with the default volume (SampleVolume, 10 MB, 512-byte blocks)
make run

# Run under Valgrind for memory checking
make vrun

# Remove compiled objects and the executable
make clean
```

`make run` is equivalent to:

```bash
./fsshell SampleVolume 10000000 512
```

Where:
- `SampleVolume` — path to the volume file (created automatically on first run)
- `10000000` — total volume size in bytes (10 MB)
- `512` — block size in bytes

On first launch the volume file is created and initialized from scratch. On subsequent launches, the existing volume is reloaded from disk, preserving all files and directories as `persistent memory`.

<img width="500" height="400" alt="image" src="https://github.com/user-attachments/assets/a4da7e14-1381-44e0-90d5-643566255889" />


---

## Interactive Shell Commands

Once the program is running you are dropped into an interactive shell prompt. Type `help` to see all commands, or `exit` to quit. The shell supports **single and double quoted strings** and **backslash-escaped spaces** in arguments (e.g., `cd "my folder"` or `cd my\ folder`).

---

### `help`

```
help
```

Prints a summary of all available commands and their syntax.

<img width="400" height="200" alt="image" src="https://github.com/user-attachments/assets/0737d0ba-1fe1-4501-8f4c-fb430134f647" />

---

### `exit`

```
exit
```

Cleanly shuts down the file system (writes any pending state) and terminates the program. Equivalent to pressing `Ctrl+D` at the prompt.

<img width="400" height="40" alt="image" src="https://github.com/user-attachments/assets/05d09b62-3a9a-4c50-9c1e-110d6799ffe7" />


---

### `pwd`

```
pwd
```

Prints the absolute path of the current working directory.

**Example:**

<img width="309" height="56" alt="image" src="https://github.com/user-attachments/assets/8d28504a-4093-4894-86cd-d5d7fe605964" />

---

### `ls`

```
ls [--all | -a] [--long | -l] [pathname]
```

Lists the contents of a directory. With no arguments, lists the current working directory.

| Flag | Description |
|------|-------------|
| `--all` / `-a` | Include all entries (currently same as default; reserved for hidden file support) |
| `--long` / `-l` | Long format: shows permissions, size, timestamps, and name |

**Example:**

<img width="400" height="73" alt="image" src="https://github.com/user-attachments/assets/78f14050-2ae8-42a5-ad8a-0a6ea2a80b0b" />


---

### `cd`

```
cd <path>
```

Changes the current working directory. Supports:
- **Absolute paths** — starting with `/` (e.g., `cd /docs/images`)
- **Relative paths** — relative to the current directory (e.g., `cd images`)
- **`.`** — current directory (no-op)
- **`..`** — parent directory

**Example:**

<img width="303" height="69" alt="image" src="https://github.com/user-attachments/assets/91a071f5-896e-48b9-80d0-033133e6b3be" />


---

### `md`

```
md <pathname>
```

Creates a new directory at the specified path. The parent directory must already exist. Allocates 5 contiguous blocks on disk for the new directory. The new directory is initialized with `.` (self) and `..` (parent) entries.

**Example:**

<img width="350" height="150" alt="image" src="https://github.com/user-attachments/assets/56ca475d-2f39-41da-b3fc-60d3b9199c1f" />


---

### `touch`

```
touch <filename>
```

Creates a new, empty file in the current working directory. Allocates 10 contiguous blocks on disk immediately. The file size is recorded as 0 bytes until data is written via `cp2fs` or `cp`.

**Example:**

<img width="300" height="150" alt="image" src="https://github.com/user-attachments/assets/ed44c9b2-ae27-4001-ac62-db45b5b63802" />


---

### `cat`

```
cat <srcfile>
```

Reads and prints the contents of a file to the terminal. Reads sequentially in 200-byte chunks until the end of the file.

**Example:**

<img width="300" height="70" alt="image" src="https://github.com/user-attachments/assets/6e9a1a6c-c656-41e8-ada9-d7d901044412" />


---

### `cp`

```
cp <srcfile> [destfile]
```

Copies a file within the custom filesystem. If `destfile` is omitted, the destination defaults to the same name as the source (effectively a copy in place). Both paths must be within the custom filesystem.

**Example:**

<img width="300" height="75" alt="image" src="https://github.com/user-attachments/assets/f0ccf2a7-0f99-42cc-9508-83b7c1a81b86" />


---

### `mv`

```
mv <src> <dest>
```

Moves or renames a file or directory. Updates the directory entry in the parent to reflect the new name/location. No data blocks are moved — only the metadata is updated.

**Example:**

<img width="325" height="270" alt="image" src="https://github.com/user-attachments/assets/3c6ce143-ce13-4f20-9f34-5b0aab9fd179" />


---

### `rm`

```
rm <path>
```

Removes a file or an **empty** directory at the given path.

- If `path` is a **file**: its 10 allocated blocks are zeroed out on disk and freed in the bitmap.
- If `path` is a **directory**: the directory must be empty (no entries beyond `.` and `..`). Its 5 blocks are zeroed and freed.

**Example:**

<img width="312" height="164" alt="image" src="https://github.com/user-attachments/assets/a693023f-e053-4d16-ab36-00186c5e9ec4" />


---

### `cp2fs`

```
cp2fs <linux-src> [destfile]
```

Copies a file **from the Linux/host filesystem into the custom filesystem**. `linux-src` is a path on the host OS. `destfile` is the destination filename inside the custom filesystem (defaults to the source filename if omitted). This is the primary way to import data into the volume.

**Example:**

<img width="300" height="100" alt="image" src="https://github.com/user-attachments/assets/dcf7fee2-b322-405b-84c0-8cb00098feb6" />


---

### `cp2l`

```
cp2l <srcfile> [linux-dest]
```

Copies a file **from the custom filesystem out to the Linux/host filesystem**. `srcfile` is a path within the custom filesystem. `linux-dest` is the destination path on the host OS (defaults to the source filename in the current host directory if omitted). This is the primary way to export data from the volume.

**Example:**

<img width="350" height="72" alt="image" src="https://github.com/user-attachments/assets/b79a01ef-9f7f-4ae6-93c5-36f71f86e8c8" />
<img width="450" height="75" alt="image" src="https://github.com/user-attachments/assets/14b7c447-913d-4da0-a192-2c05806fee7b" />




---

### `history`

```
history
```

Prints a numbered list of all commands entered during the current session, in the order they were executed.

**Example:**

<img width="350" height="300" alt="image" src="https://github.com/user-attachments/assets/b40a0feb-8d33-41f9-9195-c8c3e9f4e820" />


---

## Architecture Overview

| Component | File(s) | Role |
|-----------|---------|------|
| Shell driver | `fsshell.c` | Command parsing, dispatch table, readline integration |
| Initialization | `fsInit.c/h` | Mount/unmount volume, bootstrap new filesystem |
| File system API | `mfs.c/h` | `mkdir`, `cd`, `ls`, `delete`, path resolution |
| Bitmap | `bitmap.c/h` | Free space tracking and contiguous block allocation |
| Buffered I/O | `b_io.c/h` | File open/read/write/close with per-block buffering |
| Directory entries | `directory_entry.c/h` | DE struct, directory init, slot management |
| Volume Control Block | `volume_control_block.c/h` | VCB struct, disk read/write |
| Low-level block I/O | `fsLow.h` | `LBAread` / `LBAwrite` for raw block access |
