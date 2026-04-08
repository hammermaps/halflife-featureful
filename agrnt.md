# Agent Notes – Half-Life Featureful SDK

## Projektübersicht

Dieses Repository enthält das **Half-Life Featureful SDK** – eine erweiterte Half-Life SDK für GoldSource & Xash3D mit Bugfixes und Features für Mod-Entwickler.

## Wichtige Verzeichnisse

| Verzeichnis | Beschreibung |
|-------------|--------------|
| `cl_dll/`   | Client-seitige DLL (Clientcode) |
| `dlls/`     | Server-seitige DLL (Spiellogik, Entitäten) |
| `common/`   | Gemeinsamer Code zwischen Client und Server |
| `engine/`   | Engine-Interfaces und Header |
| `game_shared/` | Zwischen Client und Server geteilter Spielcode |
| `pm_shared/` | Player-Movement-Code |
| `public/`   | Öffentliche Header |
| `features/` | Featurespezifische Implementierungen |
| `tests/`    | Unit-Tests |
| `utils/`    | Hilfstools |

## Build

```bash
# CMake (empfohlen)
cmake -B build
cmake --build build

# WAF
python waf configure
python waf build
```

Weitere Details: [README.md](README.md)

## Optimierungen

- [OPTIMIZATION_RECOMMENDATIONS.md](OPTIMIZATION_RECOMMENDATIONS.md) – Deutsch
- [OPTIMIZATION_RECOMMENDATIONS_EN.md](OPTIMIZATION_RECOMMENDATIONS_EN.md) – Englisch
- [goldsource.md](goldsource.md) – Vollständige TODO-Inventur & Modernisierungsplan

## Dokumentation

Online-Dokumentation: https://freeslave.github.io/halflife-featureful/
