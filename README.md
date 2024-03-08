# MiniWin
a mini cross-platform window creation library in c99

---

### Backends
- [x] X11
- [x] Win32
- [ ] Cocoa

### Compiling
compiling this library is easy enough, just put [miniwin.c](./src/miniwin.c) & [miniwin.h](./src/miniwin.h)
somewhere in your project and when compiling you can choose to specify what backend you want to use.  The library
by default will check if `MWIN_BACKEND_*` is defined, if not it will use x11 if `__linux__` is defined or Win32
if `_WIN32`/`_WIN64` is defined.

| Backend |      Definition      | Linker Flags |
|---------|----------------------|--------------|
| Win32   | `MWIN_BACKEND_WIN32` | `-lgdi32`    |
| X11     | `MWIN_BACKEND_X11`   | `-lX11`      |

### API
The API is pretty self-explanatory, except the Pixel buffer you will be writing into will be
allocated, resized & deallocated by the library itself, you are just supposed to write into
the Pixel buffer and call `mwin_swap(...)` to show the updated buffer onto the screen.

---

## Thanks

