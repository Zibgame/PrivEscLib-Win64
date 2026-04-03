# 🔐 PrivEsc-Win64

## 🧠 Overview

PrivEsc-Win64 is a low-level Windows privilege escalation library written in C++. It provides a unified interface to handle privilege elevation flows, process relaunching, and security context transitions.

This project is designed for:

* Offensive security research
* Red team tooling
* Low-level Windows internals learning

---

## ⚙️ Features

* 🚀 Automatic privilege elevation (RUNAS)
* 🔁 Self-relaunch with elevated token
* 🧠 Admin detection via token inspection
* 🧩 Modular architecture (strategy-based design)
* 🛠️ Clean static library integration

---

## 🧱 Project Structure

```
src/
├── core/
│   └── privesc.cpp
├── runas/
│   └── elevate_runas.cpp
├── other/
│   └── isadmin.cpp
├── fodhelper/
│   └── fodhelper.cpp
└── examples/
    └── basic.cpp

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

## 🧩 API Usage

### Include

```cpp
#include "privesc.hpp"
```

### Elevate Privileges

```cpp
privesc(RUNAS);
```

### Full Example

```cpp
int main()
{
    privesc(RUNAS);

    // Code here runs as admin
    return 0;
}
```

---

## 🧠 How It Works

1. Check if the current process has administrative privileges
2. If not, relaunch itself using Windows ShellExecute with "runas"
3. Spawn elevated process
4. Kill the original process
5. Continue execution in elevated context

---

## ⚔️ Use Cases

* Post-exploitation tooling
* Privilege-aware applications
* Red team frameworks
* Windows internals experimentation

---

## ⚠️ Disclaimer

This project is intended for educational and authorized security research purposes only.

---

## 🚀 Future Improvements

* Additional elevation strategies
* Better error handling
* Logging system
* Detection bypass research modules

---

## 🧠 Philosophy

Minimal. Direct. Effective.

Designed to give full control over privilege context with zero overhead.
