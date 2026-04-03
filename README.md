# 🔓 Windows Privilege Escalation Library

## 🧠 Overview

PrivEsc-Win64 is a low-level Windows privilege escalation library written in C++. It provides a simple and consistent API to handle privilege elevation, process relaunching, and security context management.

This project is designed for:

* Offensive security research
* Red team tooling
* Advanced Windows internals learning

---

## ⚙️ Features

* 🚀 Multiple elevation methods (`RUNAS`, `FODHELPER`)
* 🔁 Self-relaunch with elevated privileges
* 🧠 Admin detection via token inspection
* 🧩 Modular architecture
* 🛠️ Lightweight static library
* 🔧 Direct WinAPI usage (no abstraction bloat)

---

## 🧱 Project Structure

```
src/
├── core/
│   └── privesc.cpp
├── runas/
│   └── elevate_runas.cpp
├── fodhelper/
│   ├── fodhelper.cpp
│   └── regedit.cpp
├── other/
│   └── isadmin.cpp
├── examples/
│   └── basic.cpp

include/
└── privesc.hpp

build/
```

---

## 🔧 Build

### Requirements

* Windows
* MinGW (g++)
* mingw32-make

### Compile

```
mingw32-make re
```

### Output

```
build/
├── libprivesc.a
└── bin/
    └── basic.exe
```

---

## 🧩 Public API

```cpp

enum method
{
    RUNAS,
    FODHELPER
};

bool elevate_privileges(method meto);
bool is_admin();
char *get_myh_path(void);

```

---

## 🧪 Usage

### Basic Example

```cpp
#include "privesc.hpp"

int main(void)
{
    elevate_privileges(RUNAS);

    if (is_admin())
        printf("Admin\n");
    else
        printf("Not admin\n");

    return (0);
}
```

---

## 🧠 How It Works

### RUNAS

* Uses `ShellExecute` with the `runas` verb
* Triggers UAC prompt
* Relaunches the binary with elevated privileges

### FODHELPER

* Creates a registry override in `HKCU`
* Sets execution behavior via registry keys
* Launches an auto-elevated Windows binary
* Executes payload in elevated context

---

## ⚔️ Use Cases

* Privilege-aware applications
* Security research
* Red team tooling
* Windows internals experimentation

---

## ⚠️ Disclaimer

This project is intended strictly for educational purposes and authorized security research.

Do NOT use this code on systems you do not own or have explicit permission to test.

---

## 🚀 Future Improvements

* Additional elevation methods
* Automatic cleanup mechanisms
* Logging system
* Detection modules
* Improved error handling

---

## 🧠 Philosophy

Minimal. Direct. Low-level.

No magic, no abstraction layers — only control.
