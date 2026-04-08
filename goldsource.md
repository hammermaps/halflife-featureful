# GoldSource Modernisierungsplan – Half-Life Featureful SDK

> **Stand**: April 2026 | **Codebase**: ~270.000 LOC in 594 Dateien | **C++ Standard**: C++11/14

Dieses Dokument enthält eine vollständige Inventur aller TODO/FIXME/HACK/BUGBUG-Marker im Code sowie einen umfassenden Modernisierungsplan.

---

## Inhaltsverzeichnis

1. [Zusammenfassung der Code-Marker](#1-zusammenfassung-der-code-marker)
2. [Alle TODOs im Code](#2-alle-todos-im-code)
3. [Alle FIXMEs im Code](#3-alle-fixmes-im-code)
4. [Alle HACKs im Code](#4-alle-hacks-im-code)
5. [Alle BUGBUGs im Code](#5-alle-bugbugs-im-code)
6. [Unsichere String-Operationen](#6-unsichere-string-operationen)
7. [Modernisierungsempfehlungen](#7-modernisierungsempfehlungen)
8. [Priorisierter Aktionsplan](#8-priorisierter-aktionsplan)

---

## 1. Zusammenfassung der Code-Marker

| Typ | Anzahl | Server (`dlls/`) | Client (`cl_dll/`) | Shared (`pm_shared/`) | Utils (`utils/`) |
|-----|--------|-------------------|--------------------|-----------------------|------------------|
| TODO | 94 | 48 | 17 | 0 | 29 |
| FIXME | 27 | 9 | 10 | 5 | 3 |
| HACK/HACKHACK | 62 | 42 | 12 | 2 | 0 |
| BUGBUG | 31 | 26 | 1 | 0 | 0 |
| **Gesamt** | **214** | **125** | **40** | **7** | **32** |

---

## 2. Alle TODOs im Code

### 2.1 Server-seitig (`dlls/`)

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `animating.cpp` | 151 | Untested alternative approach | 🟡 Mittel |
| `apache.cpp` | 1146 | Smoke can't be expressed via trace attack effects | 🟢 Niedrig |
| `cbase.cpp` | 952 | Should loop over all waves and stop each? | 🟡 Mittel |
| `cbase.cpp` | 1473 | Children might have different sizes – report largest | 🟢 Niedrig |
| `cbase.h` | 92 | Replace by actual definitions from physint.h | 🟡 Mittel |
| `crossbow.cpp` | 190 | Make crossbow damage configurable | 🟡 Mittel |
| `effects.cpp` | 1838 | Maybe add spawnflag to allow changing HL behavior | 🟢 Niedrig |
| `fgrunt.cpp` | 3211 | Account for custom trace attack rules | 🟡 Mittel |
| `flybee.cpp` | 1091 | Weird code – needs investigation | 🟡 Mittel |
| `func_tank.cpp` | 1684 | Aliases not supported in Featureful yet | 🔴 Hoch |
| `game.cpp` | 405 | Optimize game initialization code | 🟡 Mittel |
| `gargantua.cpp` | 309 | Use radius damage instead? | 🟡 Mittel |
| `gargantua.cpp` | 356 | Make into client side effects? | 🟢 Niedrig |
| `geneworm.cpp` | 970 | Determine direction of velocity to apply | 🟡 Mittel |
| `geneworm.cpp` | 1181 | Possibly unused code path | 🟢 Niedrig |
| `grapple.cpp` | 92 | Clamp at sv_maxvelocity for grapple tip | 🔴 Hoch |
| `grapple.cpp` | 653 | CTF support missing | 🟡 Mittel |
| `handgrenade.cpp` | 89 | Unimplemented feature | 🔴 Hoch |
| `hgrunt.cpp` | 398 | Remove unnecessary override? | 🟢 Niedrig |
| `hwgrunt.cpp` | 239 | Feature not supported yet | 🔴 Hoch |
| `islave.cpp` | 1029 | Should restore vort's actual values after death | 🟡 Mittel |
| `islave.cpp` | 1136 | Check if target is living creature, not machine | 🟡 Mittel |
| `monstermaker.cpp` | 40 | SF_MONSTERMAKER_WAIT_FOR_SCRIPT unimplemented | 🔴 Hoch |
| `monsters.cpp` | 306 | Check against player who produced sound | 🟡 Mittel |
| `monsters.cpp` | 4754 | Won't work for classify that dislikes own kind | 🟡 Mittel |
| `multiplay_gamerules.cpp` | 1023 | Make output go direct to console | 🟢 Niedrig |
| `osprey.cpp` | 607 | What if grunt has different render mode? | 🟡 Mittel |
| `plats.cpp` | 1004 | Could cause problems with nullptr target? | 🟡 Mittel |
| `player.cpp` | 1103 | Make armor behavior depend on game rules | 🟡 Mittel |
| `player.cpp` | 2098 | Send HUD Update | 🔴 Hoch |
| `player.cpp` | 5849 | Make player colors configurable | 🟡 Mittel |
| `ropes.cpp` | 701 | Move to common header | 🟢 Niedrig |
| `schedule.cpp` | 732 | Not sure if framerate reset needed | 🟢 Niedrig |
| `scientist.cpp` | 1089 | Better check for dangerous enemies | 🟡 Mittel |
| `squadmonster.cpp` | 569 | Better check for player relationship | 🟡 Mittel |
| `tor.cpp` | 16 | Empty TODO – needs clarification | 🟢 Niedrig |
| `triggers.cpp` | 4204 | C promotion rules question | 🟢 Niedrig |
| `triggers.cpp` | 6271 | Change Activate to PostSpawn | 🟡 Mittel |
| `tripmine.cpp` | 272 | Original code passed 1 as flags – meaning unclear | 🟡 Mittel |
| `tripmine.cpp` | 463 | Different size in singleplayer – why? | 🟢 Niedrig |
| `util.h` | 590 | Move declaration to movewith.h | 🟢 Niedrig |
| `vehicle.cpp` | 314 | Fix float for test demo | 🔴 Hoch |
| `voltigore.cpp` | 1327 | Empty TODO | 🟢 Niedrig |
| `weapons.h` | 222-223 | Check return type usage of ExtractAmmo/ExtractClipAmmo | 🟡 Mittel |
| `weapons_shared.cpp` | 23 | Protect against out of bounds access | 🔴 Hoch |
| `weapons_shared.cpp` | 1023 | Client updates time slower than server | 🟡 Mittel |
| `weapons_shared.cpp` | 1488 | Properly send spreads for multiple bullet shots | 🟡 Mittel |

### 2.2 Client-seitig (`cl_dll/`)

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `cdll_int.cpp` | 999 | Load SDL2 and call ShowSimpleMessageBox | 🟡 Mittel |
| `entity.cpp` | 710 | Sven Co-op muzzle flash stub | 🟢 Niedrig |
| `health.cpp` | 106 | Update local health data | 🟡 Mittel |
| `health.cpp` | 395 | Get shift value of health | 🟡 Mittel |
| `hl/hl_weapons.cpp` | 199 | Disabled sound – now via server-side soundscript | 🟢 Niedrig |
| `hud_journal.cpp` | 244 | Deduplicate inventory item rendering | 🟡 Mittel |
| `input_xash3d.cpp` | 203 | Scroll on phones not implemented | 🟡 Mittel |
| `particleman/CBaseParticle.cpp` | 100 | Particle direction calculation uncertain | 🟡 Mittel |
| `particleman/CBaseParticle.cpp` | 208 | Should account for stretch Y | 🟡 Mittel |
| `particleman/CFrustum.cpp` | 36 | Unnecessary parameter passing | 🟢 Niedrig |
| `particleman/CFrustum.cpp` | 59 | 4x4 matrix multiplication needs cleanup | 🟡 Mittel |
| `particleman/IParticleMan_Active.cpp` | 137 | Engine doesn't support printing size_t | 🟢 Niedrig |
| `StudioModelRenderer.cpp` | 475-477 | Entity data should be stored/cached, use look-up table | 🟡 Mittel |
| `vgui_ScorePanel.cpp` | 241 | Remove testing code | 🟢 Niedrig |
| `view.cpp` | 346 | Xash3D-FWGS rollangle/rollspeed support | 🟡 Mittel |

### 2.3 Utils (`utils/`)

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `fake_vgui/include/VGUI.h` | 42-78 | 18 TODOs für VGUI-Architekturverbesserungen | 🟢 Niedrig |
| `fake_vgui/include/VGUI_Panel.h` | 14, 21 | VGUI Panel TODOs | 🟢 Niedrig |
| `fake_vgui/include/VGUI_App.h` | 33-34 | Access modifiers reorganization | 🟢 Niedrig |
| `fake_vgui/include/VGUI_Color.h` | 14-17 | Rename color methods | 🟢 Niedrig |

---

## 3. Alle FIXMEs im Code

### 3.1 Server-seitig (`dlls/`)

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `animating.cpp` | 168 | Events get missed without this workaround | 🔴 Hoch |
| `client.cpp` | 1731 | Move to script | 🟡 Mittel |
| `client.cpp` | 1888 | Move to script | 🟡 Mittel |
| `multiplay_gamerules.cpp` | 608 | Unnecessary cast to read m_iDeaths | 🟢 Niedrig |
| `player.cpp` | 5069 | Remove for deathmatch testing | 🟢 Niedrig |
| `player.cpp` | 5093 | Remove for deathmatch testing | 🟢 Niedrig |
| `scientist.cpp` | 366 | ACT_CROUCHIDLE looks lame | 🟢 Niedrig |

### 3.2 Client-seitig (`cl_dll/`)

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `ev_hldm.cpp` | 104 | Check if playtexture sounds movevar is set | 🟡 Mittel |
| `hl/hl_weapons.cpp` | 556 | Should be a method in each weapon | 🟡 Mittel |
| `input_goldsource.cpp` | 593 | Call through to engine? | 🟡 Mittel |
| `StudioModelRenderer.cpp` | 574 | Make work for clipped case | 🟡 Mittel |
| `studio_util.cpp` | 288 | Rescale inputs to 1/2 angle | 🟡 Mittel |
| `view.cpp` | 144 | Use Quaternions to avoid discontinuities | 🔴 Hoch |
| `view.cpp` | 453 | Needs to be predictable | 🟡 Mittel |
| `view.cpp` | 479 | Old settings caused exposed polys on models | 🟡 Mittel |
| `view.cpp` | 557 | Origin sent at 1/128 – change? | 🟡 Mittel |
| `view.cpp` | 744 | steptime < 0 error handling | 🔴 Hoch |
| `demo.cpp` | 26 | Need buffer helper functions | 🟡 Mittel |

### 3.3 Player Movement (`pm_shared/`)

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `pm_math.cpp` | 191 | Use Quaternions to avoid discontinuities | 🔴 Hoch |
| `pm_math.cpp` | 285 | sqrt FIXME | 🟡 Mittel |
| `pm_math.cpp` | 302 | sqrt FIXME | 🟡 Mittel |
| `pm_shared.cpp` | 286 | mp_footsteps needs to be a movevar | 🟡 Mittel |
| `pm_shared.cpp` | 302 | Move to player state | 🟡 Mittel |
| `pm_shared.cpp` | 1218 | Check steep slope | 🟡 Mittel |

---

## 4. Alle HACKs im Code

### 4.1 Server-seitig (`dlls/`) – 42 HACKs

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `basemonster.h` | 140 | `m_HackedGunPos` – HACK until gun end query | 🔴 Hoch |
| `bmodels.cpp` | 204 | HACKHACK for special effects | 🟡 Mittel |
| `bmodels.cpp` | 228 | Speed encoded in rendercolor – ugly hack | 🔴 Hoch |
| `bullsquid.cpp` | 713 | Hardcoded spit origin from 3ds | 🟡 Mittel |
| `buttons.cpp` | 1784 | Button behaves like door | 🟡 Mittel |
| `buttons.cpp` | 1967 | Multiple player packets per frame | 🔴 Hoch |
| `cbase.cpp` | 297 | Node graph entity touching prevention | 🟡 Mittel |
| `cbase.cpp` | 414 | Reset save pointers for real restore | 🟡 Mittel |
| `client.cpp` | 1604 | Somewhat... (vague hack) | 🟡 Mittel |
| `combat.cpp` | 49 | Gib velocity equations don't work | 🔴 Hoch |
| `combat.cpp` | 1216 | Don't kill monsters in script | 🟡 Mittel |
| `effects.cpp` | 106 | Speed in rendercolor | 🟡 Mittel |
| `explode.cpp` | 308 | Fake keyvalue for explosion setup | 🟡 Mittel |
| `func_break.cpp` | 238 | matGlass decal client notification | 🟢 Niedrig |
| `func_break.cpp` | 852 | Should work but doesn't | 🔴 Hoch |
| `func_tank.cpp` | 1059 | Make noise for AI to hear | 🟡 Mittel |
| `gargantua.cpp` | 1392 | Turn off flames on script/death | 🟡 Mittel |
| `gargantua.cpp` | 2004 | Cut and pasted from explode.cpp | 🔴 Hoch |
| `gonome.cpp` | 842 | Stop double attack sound | 🟡 Mittel |
| `kingpin.cpp` | 1692 | HACK to change blood color | 🟢 Niedrig |
| `monsters.cpp` | 1570 | Commented out origin hack | 🟢 Niedrig |
| `monsters.h` | 92 | bits_MEMORY_KILLED remember Killed() called | 🟡 Mittel |
| `nodes.cpp` | 194, 1258, 1272, 1769, 1777 | 5× bodyqueue HACKHACK | 🔴 Hoch |
| `nodes.cpp` | 1624 | TOUCH HACK for node graph | 🟡 Mittel |
| `pathcorner.cpp` | 334 | Dead end detection | 🟢 Niedrig |
| `pitdrone.cpp` | 532 | Hardcoded spit origin from 3ds | 🟡 Mittel |
| `plats.cpp` | 1580 | Train stop position depends on speed | 🟡 Mittel |
| `player.cpp` | 2905 | Look for func_tracktrain classname | 🟡 Mittel |
| `player.cpp` | 2997 | Traceline hit detection while not animating | 🟡 Mittel |
| `player.cpp` | 4301 | Use crouch HACK | 🟡 Mittel |
| `player.cpp` | 4327 | Time variable repurposed | 🟡 Mittel |
| `player.cpp` | 5149 | Exhaustible weapon as real weapon | 🔴 Hoch |
| `player.cpp` | 5453 | Game title display | 🟢 Niedrig |
| `ropes.cpp` | 759 | Integer underflow dependency | 🔴 Hoch |
| `satchel.cpp` | 164 | Ground detection workaround | 🟡 Mittel |
| `schedule.cpp` | 627 | Thin wide bounding box workaround | 🔴 Hoch |
| `scripted.cpp` | 1156 | Float origin and drop to floor | 🟡 Mittel |
| `sound.cpp` | 178, 736 | Save/restore design hacks | 🟡 Mittel |
| `squeakgrenade.cpp` | 710 | Origin change due to physics code | 🔴 Hoch |
| `triggers.cpp` | 1152 | HEALTH field overloaded | 🟡 Mittel |
| `triggers.cpp` | 1609 | Multiplayer packet receipt timing | 🔴 Hoch |
| `tripmine.cpp` | 316 | Simple box using global | 🟡 Mittel |
| `turret.cpp` | 1140 | Trigger on death condition | 🟡 Mittel |

### 4.2 Client-seitig (`cl_dll/`) – 12 HACKs

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `hud.cpp` | 1599 | m_iPlayerNum should be initialized elsewhere | 🟡 Mittel |
| `in_camera.cpp` | 414-415 | External KeyDown/KeyUp HACK | 🟡 Mittel |
| `input_goldsource.cpp` | 839, 901 | HACKHACK: dead viewangles, viewcode change | 🟡 Mittel |
| `input_xash3d.cpp` | 187, 214 | HACKHACK: same as goldsource input | 🟡 Mittel |
| `vgui_ScorePanel.cpp` | 239 | HACK_GetPlayerUniqueID function | 🟡 Mittel |
| `view.cpp` | 1105 | Dead body clipping hack | 🟡 Mittel |
| `voice_status.cpp` | 584-585 | extern HACK_GetPlayerUniqueID | 🟡 Mittel |

### 4.3 Player Movement (`pm_shared/`) – 2 HACKs

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `pm_shared.cpp` | 1843 | HACKHACK – Collision bug fudge | 🔴 Hoch |
| `pm_shared.cpp` | 2330 | HACK HACK HACK – no description | 🔴 Hoch |

---

## 5. Alle BUGBUGs im Code

| Datei | Zeile | Beschreibung | Priorität |
|-------|-------|--------------|-----------|
| `dlls/bloater.cpp` | 730 | CheckDist should be derived from ground speed | 🟡 Mittel |
| `dlls/buttons.cpp` | 684 | BUGBUG FIX (no description) | 🔴 Hoch |
| `dlls/buttons.cpp` | 1897 | Latency design issue | 🟡 Mittel |
| `dlls/controller.cpp` | 978 | CheckDist from ground speed | 🟡 Mittel |
| `dlls/defaultai.cpp` | 253 | ActiveIdle schedule issues | 🟡 Mittel |
| `dlls/doors.cpp` | 950 | Triggered doors don't work with rotating doors | 🔴 Hoch |
| `dlls/fgrunt.cpp` | 745 | Need decent fail schedule | 🔴 Hoch |
| `dlls/fgrunt.cpp` | 1404, 1500 | 3-round burst & movetype check | 🟡 Mittel |
| `dlls/func_break.cpp` | 564 | Zero delay doesn't work | 🔴 Hoch |
| `dlls/func_break.cpp` | 859 | 256 entity limit on breakable | 🟡 Mittel |
| `dlls/hgrunt.cpp` | 373 | 3-round burst called before fire | 🟡 Mittel |
| `dlls/hgrunt.cpp` | 466 | Movetype check for jumping players | 🟡 Mittel |
| `dlls/hgrunt.cpp` | 1577 | Need decent fail schedule | 🔴 Hoch |
| `dlls/monsters.cpp` | 331 | Virtual function for food scent? | 🟡 Mittel |
| `dlls/monsters.cpp` | 935 | Doesn't work 100% | 🔴 Hoch |
| `dlls/monsters.cpp` | 1563 | SetOrigin and triggers | 🟡 Mittel |
| `dlls/monsters.cpp` | 2407, 2501 | CheckDist & magic numbers | 🟡 Mittel |
| `dlls/monsters.cpp` | 2639 | Minor hack | 🟢 Niedrig |
| `dlls/monsters.cpp` | 4411 | No magic numbers! | 🟡 Mittel |
| `dlls/nodes.cpp` | 1174 | Returns 0 on connection problem | 🔴 Hoch |
| `dlls/nodes.cpp` | 2717 | File times 20 hours ahead | 🟡 Mittel |
| `dlls/player.cpp` | 2188 | Off-use design issue | 🟡 Mittel |
| `dlls/scripted.cpp` | 873 | Doesn't call Killed() | 🔴 Hoch |
| `dlls/squadmonster.cpp` | 536 | Planes not aligned to gun firing direction | 🔴 Hoch |
| `dlls/triggers.cpp` | 1628, 1646 | 32 player limit | 🟡 Mittel |
| `dlls/tripmine.cpp` | 436 | Fix the model | 🟡 Mittel |
| `dlls/turret.cpp` | 512 | Should spin down first | 🟡 Mittel |
| `cl_dll/entity.cpp` | 764 | Needs to be time based | 🟡 Mittel |

---

## 6. Unsichere String-Operationen

### 6.1 Übersicht

| Funktion | `dlls/` | `cl_dll/` | `pm_shared/` | `game_shared/` | `utils/` | Gesamt |
|----------|---------|-----------|--------------|-----------------|----------|--------|
| `strcpy` | 58 | 43 | 1 | 1 | 3 | **106** |
| `strcat` | 39 | 15 | 0 | 3 | 0 | **59** (inkl. strlcat) |
| `sprintf` | 45 | 78 | 0 | 0 | 3 | **126** |
| **Gesamt** | **142** | **136** | **1** | **4** | **6** | **291** |

### 6.2 Empfohlene Ersetzungen

| Unsicher | Sicher (Empfohlen) | Bereits verfügbar? |
|----------|--------------------|--------------------|
| `strcpy` | `strlcpy` / `strncpy` | ✅ `strlcpy` verfügbar |
| `strcat` | `strlcat` / `strncat` | ✅ `strlcat` verfügbar |
| `sprintf` | `snprintf` | ✅ Standard C |

---

## 7. Modernisierungsempfehlungen

### 7.1 C++ Standard Upgrade (C++11 → C++17)

**Aktuell**: `set(CMAKE_CXX_STANDARD 11)` (mit C++14 Fallback für einige Targets)

| Feature | Nutzen | Anwendung im Projekt |
|---------|--------|----------------------|
| `std::string_view` | Vermeidet unnötige String-Kopien | String-Vergleiche in Entities, Sound-Lookups |
| `std::optional` | Explizite Null-Werte statt Magic Numbers | Rückgabewerte von Suchfunktionen |
| `std::variant` | Typsichere Unions | Entity-Daten, Konfigurationswerte |
| `constexpr if` | Compile-time Branches | Plattform-spezifischer Code |
| Structured Bindings | Sauberere Pair/Tuple-Dekonstruktion | Map-Iterationen |
| `[[nodiscard]]` | Verhindert ignorierte Rückgabewerte | Fehlerbehandlung |
| `[[fallthrough]]` | Explizite switch-Fallthrough | Zahlreiche switch-Statements |
| Fold Expressions | Vereinfachte variadic Templates | Logging, Debug-Output |
| Inline Variables | Header-only Globals | Shared Constants |
| `std::filesystem` | Portables Dateisystem | Node-Graph-Dateien, Konfigurationen |

### 7.2 Memory Safety Modernisierung

| Problem | Lösung | Dateien betroffen |
|---------|--------|-------------------|
| Raw `malloc`/`free` | `std::vector`, `std::unique_ptr` | `dlls/nodes.cpp` (26×), 16 weitere |
| Raw Pointer-Ownership | Smart Pointers | Entity-System, Monster-AI |
| Buffer Overflows (291×) | `snprintf`, `strlcpy` | Gesamte Codebase |
| Missing Null-Checks | `std::optional`, Assertions | Entity-Lookups |
| Magic Numbers | `constexpr` Constants, Enums | Überall |

### 7.3 Architektur-Verbesserungen

#### 7.3.1 Entity-System Modernisierung
- **Aktuell**: C-style Vererbungshierarchie mit massiven Header-Dateien (`cbase.h`: 105 virtuelle Funktionen)
- **Empfehlung**: Interface-Segregation, `final` Keyword für Leaf-Klassen
- **Vorteil**: Bessere Devirtualisierung, kleinere vtables

#### 7.3.2 AI-System Refactoring
- **Aktuell**: Monolithische Monster-Klassen mit kopierten Schedules
- **Empfehlung**: Komposition statt Vererbung, State-Machine-Pattern
- **Betroffene Dateien**: `basemonster.h` (84 virtuelle Funktionen), `hgrunt.h` (20), `talkmonster.h` (12)

#### 7.3.3 Sound-System
- **Aktuell**: Mix aus hardcodierten Strings und Soundscript-System
- **Empfehlung**: Vollständige Migration auf Soundscript-System
- **Betroffene Dateien**: `dlls/sound.cpp`, `game_shared/soundscripts.cpp`

#### 7.3.4 Save/Restore System
- **Aktuell**: C-style Macros mit `DEFINE_FIELD`, manuelle Serialisierung
- **Empfehlung**: Reflections-basiertes System oder Code-Generierung
- **Betroffene Dateien**: Nahezu alle Entity-Klassen

### 7.4 Build-System Modernisierung

| Verbesserung | Aufwand | Nutzen |
|--------------|---------|--------|
| LTO standardmäßig für Release | 🟢 Gering | 5-15% Performance |
| Precompiled Headers | 🟢 Gering | 30-50% schnellere Builds |
| Unity Builds | 🟡 Mittel | 50% schnellere Builds |
| ccache Integration | 🟢 Gering | 10× schneller bei Rebuilds |
| `#pragma once` für alle Header | 🟢 Gering | Konsistenz (40 Header fehlen) |
| C++17 Standard | 🟡 Mittel | Modernere Features |
| GitHub Actions Cache | 🟢 Gering | Schnellere CI |
| Ninja Generator | 🟢 Gering | Schnellere Builds |

### 7.5 Code-Qualität & Tooling

| Tool | Zweck | Priorität |
|------|-------|-----------|
| `clang-tidy` | Statische Analyse, Modernisierung | 🔴 Hoch |
| `cppcheck` | Zusätzliche statische Analyse | 🟡 Mittel |
| AddressSanitizer | Memory-Fehler finden | 🔴 Hoch |
| UBSan | Undefined Behavior erkennen | 🔴 Hoch |
| `-Wall -Wextra` | Mehr Compiler-Warnungen | 🔴 Hoch |
| Code Coverage | Test-Abdeckung messen | 🟡 Mittel |
| Fuzzing (libFuzzer) | Robustheit testen | 🟢 Niedrig |

### 7.6 Node-Graph-System (`dlls/nodes.cpp`)

Das Node-Graph-System ist der am stärksten modernisierungsbedürftige Code:
- **26 raw malloc/calloc/realloc** Aufrufe
- **5 HACKHACK-Marker** (bodyqueue-Missbrauch)
- **2 BUGBUG-Marker** (Verbindungsfehler, Zeitzonen)
- **21 sprintf** Aufrufe (alle unsicher)

**Empfehlung**: Vollständiges Refactoring mit:
- `std::vector` statt malloc-Arrays
- Proper Entity-Referenzen statt bodyqueue-Hack
- `snprintf` für alle String-Operationen
- Modernes File-I/O mit Fehlerbehandlung

### 7.7 VGUI-System Modernisierung

Das `utils/fake_vgui/` System enthält 29 TODOs, die hauptsächlich Architektur-Schulden beschreiben:
- Access-Modifier-Organisation
- Naming-Konventionen
- String-Handling (`strdup` → `std::string`)
- Signal-System-Redesign

**Empfehlung**: Da dies ein Fake/Stub-System ist, niedrige Priorität – nur modernisieren wenn aktiv genutzt.

---

## 8. Priorisierter Aktionsplan

### Phase 1: Quick Wins (1–2 Wochen)

- [ ] **LTO für Release-Builds aktivieren** – `CMakeLists.txt` ändern
- [ ] **Precompiled Headers einführen** – `extdll.h`, `util.h`, `cbase.h`
- [ ] **ccache/sccache aktivieren** – CMake Launcher Variablen
- [ ] **GitHub Actions Cache** – `.github/workflows/build.yml`
- [ ] **40 Header auf `#pragma once`** migrieren
- [ ] **Compiler-Warnungen aktivieren** – `-Wall -Wextra`

### Phase 2: Sicherheit (2–3 Wochen)

- [ ] **126 `sprintf` → `snprintf`** ersetzen (höchste Priorität: `dlls/nodes.cpp`)
- [ ] **106 `strcpy` → `strlcpy`** ersetzen
- [ ] **59 `strcat` → `strlcat`** ersetzen
- [ ] **AddressSanitizer in CI** aktivieren
- [ ] **UBSan in CI** aktivieren

### Phase 3: Code-Qualität (3–4 Wochen)

- [ ] **Hohe-Priorität BUGBUGs beheben** (10 Stück)
  - `buttons.cpp:684`, `doors.cpp:950`, `func_break.cpp:564`
  - `fgrunt.cpp:745`, `hgrunt.cpp:1577`, `monsters.cpp:935`
  - `nodes.cpp:1174`, `scripted.cpp:873`, `squadmonster.cpp:536`
- [ ] **Hohe-Priorität HACKs refactorn** (12 Stück)
  - `combat.cpp:49` (Gib velocity), `bmodels.cpp:228` (rendercolor)
  - `nodes.cpp` (5× bodyqueue), `ropes.cpp:759` (integer underflow)
  - `schedule.cpp:627` (bounding box), `triggers.cpp:1609` (packet timing)
- [ ] **Hohe-Priorität TODOs implementieren** (8 Stück)
  - `weapons_shared.cpp:23` (bounds check), `handgrenade.cpp:89`
  - `monstermaker.cpp:40`, `vehicle.cpp:314`, `player.cpp:2098`
  - `func_tank.cpp:1684`, `hwgrunt.cpp:239`, `grapple.cpp:92`
- [ ] **clang-tidy Integration** in CI Pipeline
- [ ] **`dlls/nodes.cpp` komplett refactorn**

### Phase 4: Modernisierung (4–6 Wochen)

- [ ] **C++ Standard auf 17 anheben**
- [ ] **`std::string_view`** für String-Vergleiche einführen
- [ ] **`std::optional`** für nullable Rückgabewerte
- [ ] **`constexpr`** für Compile-time-Konstanten
- [ ] **`final` Keyword** für Leaf-Klassen (Monster, Waffen)
- [ ] **42 raw malloc** durch `std::vector`/Smart Pointers ersetzen

### Phase 5: Architektur (2–3 Monate)

- [ ] **Entity-System Interface-Segregation**
- [ ] **AI State-Machine-Pattern** implementieren
- [ ] **Sound-System vollständig auf Soundscripts** migrieren
- [ ] **Collision/Physics HACKs** durch korrekte Implementierungen ersetzen
- [ ] **Unit-Tests erweitern** (aktuell 16 Testdateien → Ziel: 50+)
- [ ] **Benchmark-Suite** aufbauen

---

## Zusammenfassung

| Kategorie | Aktuell | Ziel |
|-----------|---------|------|
| Unsichere String-Ops | 291 | 0 |
| Code-Marker (TODO/FIXME/HACK/BUGBUG) | 214 | < 50 |
| Raw malloc/calloc/realloc | 42 | 0 |
| C++ Standard | C++11 | C++17 |
| Header ohne `#pragma once` | 40 | 0 |
| Unit-Tests | 16 | 50+ |
| Build-Zeit (Full) | ~2-5 min | < 1 min |
| Compiler-Warnungen | Teilweise | `-Wall -Wextra -Wpedantic` |
| Static Analysis | Keine | clang-tidy + cppcheck |
| Sanitizers in CI | Keine | ASan + UBSan |

> **Geschätzter Gesamtaufwand**: 4–6 Monate bei einem Entwickler, priorisiert nach Impact.
>
> **Wichtig**: Alle Änderungen sollten inkrementell erfolgen und durch Tests abgesichert werden. Das GoldSource-Engine-Interface darf nicht verändert werden – nur die SDK-interne Implementierung.
