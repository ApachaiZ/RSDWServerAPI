# 🐉 RSDWServerAPI

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)[![Version](https://img.shields.io/badge/version-0.2.1-blue.svg)](#-kompatibilität)
[![Spiel](https://img.shields.io/badge/spiel-RuneScape%3A%20Dragonwilds%201.0.1-orange.svg)](#-kompatibilität)
[![Engine](https://img.shields.io/badge/engine-UE%205.6.1-purple.svg)](#-kompatibilität)

## 🧭 Über das Projekt

**RSDWServerAPI** macht aus einem Standard-Dedicated-Server für
**RuneScape: Dragonwilds** unter Linux einen fernsteuerbaren Server. Es
injiziert eine vollständige Steuerungsebene in den laufenden Spielprozess:

- 🌐 eine **REST-API** (Live-Spielerstatistiken, Teleports, Heilung, Items, Kicks, Bans…),
- 💬 einen **Source-RCON**-Listener für Tools und Panels,
- 🎮 **In-Game-Admin-Befehle** (`/kick`, `/ban`, `/give`…) für Owner/Admins,
- 📣 ein optionales **Discord-Relay** (Chat + komische Todesmeldungen).

Es treibt die gesamte Verwaltungsschleife an — von einem Panel, einem Skript,
einem Discord-Bot (dem Begleit-Plugin
[`dkbot-dragonwilds`](https://github.com/ApachaiZ/dkbot-dragonwilds)) oder
allem, was HTTP spricht.

> ⭐ **Dies ist ein Fork** des Originalprojekts von **dkoz** —
> **[dkoz/RSDWServerAPI](https://github.com/dkoz/RSDWServerAPI)** — ohne den
> nichts davon existieren würde.
>
> 🙏 **Großen Dank an den ursprünglichen Entwickler** für das solide Fundament,
> die saubere Architektur (validierte Discovery, Marshalling nativer Aufrufe)
> und die MIT-Lizenz, die es diesem Fork erlaubt, darauf aufzubauen. Der Fork
> ergänzt produktionshärtende Fixes, 1.0.1-Build-Offsets und zusätzliche
> Admin-Funktionen (Teleport, Tell, Heal, Feed, Hydrate, God-Mode). Upstream-
> Beiträge bleiben dort willkommen.
>
> 🌐 Sprachen: 🇬🇧 [English version](README.md) · 🇫🇷 [Version française](README.fr.md) · 🇩🇪 Deutsch · 🇪🇸 [Versión española](README.es.md)

## 🚀 TL;DR — Einrichtung in 5 Minuten

```bash
# 1. Build (Docker oder make)
./build.sh                          # → dist/librsdwapi.so

# 2. Neben den Server kopieren, Startbefehl prefixen
LD_PRELOAD=./librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port=7777

# 3. Beim ersten Start schreibt der Mod rsdwapi/settings.ini neben das Binary.
#    Die REST-API ist per Default AN (Port 8080), RCON AUS.

# 4. Funktion prüfen
curl http://127.0.0.1:8080/api/health
# → {"status":"healthy","engineReady":true}
```

> 🔐 **Bevor irgendetwas exponiert wird**: `BindAddress=127.0.0.1` und ein
> `BearerToken` in `settings.ini` setzen (dann
> `Authorization: Bearer <token>` senden). Siehe [🔒 Sicherheit](#-sicherheit).

## 🧭 So funktioniert es, in einem Absatz

Dieser Mod ist eine **Shared Library, die beim Start in den Spielprozess
injiziert wird** (`LD_PRELOAD`). Sie wartet, bis die Unreal-Engine bereit ist,
lokalisiert die Engine-Globals (Namenspool, Objektliste, Welt — mit
validierten Fallback-Scans) und liefert dann Live-Spieldaten und
Admin-Aktionen über HTTP. Sie ändert niemals Spieldateien, fasst niemals
Saves an und liest nur über validierte Pointer: Ein Spiel-Patch degradiert zu
einem `503` plus einer Logzeile, nicht zu einem Crash. Der Spielprozess
bleibt ansonsten unberührt — dasselbe Binary betreibt den Server wie zuvor.

---

## 📦 Kompatibilität

Live verifiziert auf einem produktiven Dedicated-Server (Ubuntu, systemd + `LD_PRELOAD`).

| Spielversion | Build (CL) | Steam-Build | Linux-Depot | Status |
|---|---|---|---|---|
| 1.0.0.x | 247725 (`++dominion+hotfix`) | — | — | ✅ verifiziert (SDK-Offsets) |
| **1.0.1** "Eye on Ashenfall" | **248854** (`++dominion+staging`) | **25811348** | **3501791** | ✅ verifiziert (Offsets aktualisiert) |

- Engine: **UE 5.6.1** — unverändert zwischen den beiden Builds oben.
- Plattform: Linux-Dedicated-Server (`RSDragonwildsServer-Linux-Shipping`,
  Basis-Image `0x200000`, gestrippt: kein `.symtab`,
  `UObject::ProcessEvent` nicht exportiert).

## ✨ Funktionen

- 📡 **Live-Spielerdaten** — Namen, Charakter-GUID, Plattform, Ping, Position, Rotation,
  Geschwindigkeit, Gesundheit, Sättigung, Hydratation, jeder Skill mit XP, jedes
  Float-Attribut.
- 🚪 **Remote-Kick** über die eigene Kick-Funktion des Spiels, solange der Server läuft.
- 📢 **Broadcast** und 💬 **privates Tell** über den eigenen Chat des Spiels.
- 🎁 **Items geben** über die eigene Give-Funktion des Spiels.
- 🔨 **Bans**, die Neustarts überleben; Offline-Spieler per SteamID64 bannbar.
- 🌀 **Teleport** — Spieler auf Spieler oder zu Koordinaten — über das native
  `K2_TeleportTo` des Spiels im Game-Thread (engine-anerkannt, repliziert).
- 💚 **Admin-Aktionen** — Heal (natives `IncreaseHealth`), Feed, Hydrate, God-Mode.
- 🎮 **In-Game-Admin-Befehle** — `/kick`, `/ban`, `/unban`, `/give`, geschützt
  durch die eigene Owner/Admin-Liste des Spiels.
- 📣 **Discord-Relay** — In-Game-Chat und Spielertode an Webhooks.
- 🔌 **Zwei unabhängige Listener** — REST und Source-RCON, jeweils separat schaltbar.
- 🔐 **Zugriffskontrolle auf beiden** — Bearer-Token oder RCON-Passwort,
  IP-Whitelisting, Rate-Limiting pro Adresse und Auto-Block nach wiederholten Fehlversuchen.
- 🛡️ **Offsets zur Laufzeit validiert** — jeder Discovery-Schritt wird geprüft und
  gegengeprüft, bevor etwas gelesen wird; eine Abweichung nach einem Spiel-Patch
  degradiert zu `503` und einer Logzeile statt Unsinn zu liefern.

## 🚀 Schnellstart

### Build

Docker ist die einzige Voraussetzung. Der Build-Container ist Debian bookworm —
identisch mit dem Image, in dem der Dedicated-Server läuft, sodass das Ergebnis
niemals eine neuere glibc vom Host verlangt. `libstdc++` und `libgcc` sind
statisch gelinkt.

```bash
./build.sh
```

Ausgabe: `dist/librsdwapi.so`.

Ein simples `make` funktioniert auch mit einer lokalen Toolchain; auf Hosts
ohne statisches `libstdc++` (Fedora…) die statischen Flags weglassen:

```bash
make LDFLAGS="-shared -pthread -ldl"
```

### Installation

`dist/librsdwapi.so` ins Server-Wurzelverzeichnis kopieren und den Startbefehl
prefixen:

```bash
LD_PRELOAD=./librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port=7777
```

### systemd (Produktions-Setup)

Das Drop-in unten ist, was in Produktion läuft: `LD_PRELOAD` gilt für den
ganzen Dienst (Launcher-Skript *und* Spiel-Binary werden injiziert —
harmlos, siehe [🏗️ So funktioniert es](#️-so-funktioniert-es)), und das
Datenverzeichnis wird mit `RSDWAPI_DIR` fixiert:

```ini
# /etc/systemd/system/dragonwilds.service.d/rsdwapi.conf
[Service]
Environment=RSDWAPI_DIR=/home/steam/dragonwilds_linux_server/rsdwapi
Environment=LD_PRELOAD=/home/steam/dragonwilds_linux_server/librsdwapi.so
```

Auf Pterodactyl oder Pelican das `.so` ins Server-Wurzelverzeichnis legen und
den Start wie folgt setzen:

```
LD_PRELOAD=/home/container/librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port={{SERVER_PORT}} -ini:Game:[/Script/Engine.GameSession]:MaxPlayers={{MAX_PLAYERS}}
```

API-Port (und RCON-Port, falls aktiviert) im Panel zuweisen.

## ⚙️ Konfiguration

Beim ersten Start schreibt der Mod `rsdwapi/settings.ini` neben das
Server-Binary (`RSDragonwilds/Binaries/Linux/rsdwapi/` bei Standard-
Installation). `RSDWAPI_DIR` setzt einen leichter erreichbaren Ort. Die Datei
wird beim Serverstart neu gelesen.

Ein Bearer-Token und ein RCON-Passwort werden beim ersten Start generiert und
danach nie wieder neu erzeugt — ein leerer Wert bleibt leer.

**Die REST-API ist per Default an. RCON ist per Default aus.**

```ini
[General]
EnableLogging=true        # Discovery-Schritte + jede Anfrage
VerboseLogging=true       # zusätzliche Scan-Details; anlassen, außer es stört

[API]
Enabled=true
BindAddress=0.0.0.0       # 🔒 in Produktion 127.0.0.1 + Token bevorzugen
Port=8080
BearerToken=              # leer = gar keine Auth (nicht empfohlen)
IPWhitelist=              # kommagetrennte IPs / CIDRs; leer = alle

[RCON]
Enabled=false             # Source-RCON, per Default aus
BindAddress=0.0.0.0
Port=27020
Password=                 # leer = RCON startet nicht, per Design
IPWhitelist=
CommandsPerMinute=60
CommandBurst=15
MaxFailedAuth=5
FailWindowSeconds=60
BanSeconds=300
MaxConnections=16

[Discord]
Enabled=false
WebhookUrl=               # Chat-Relay
DeathWebhookUrl=          # Tode; fällt auf WebhookUrl zurück, wenn leer
Username=Dragonwilds
```

## 🌐 REST-API

Alle Endpoints liegen unter `/api/`. Wenn ein Bearer-Token konfiguriert ist,
`Authorization: Bearer <token>` senden (Header-Namen sind
case-insensitiv — reqwest/hyper machen sie klein, das wird behandelt).

- `200` mit `{"status":"ok","message":"…"}` bei Erfolg.
- `401` fehlendes/falsches Token · `404` unbekannte Route · `503` Engine nicht
  bereit (`{"error":"engine not initialised"}`).

### Lese-Endpoints

| Route | Beschreibung |
|---|---|
| `GET /api/hello` | Liveness-Ping. |
| `GET /api/health` | `{"status","version","uptimeSeconds","engineReady"}`. |
| `GET /api/players` | Alle verbundenen Spieler, volles Detail. Optionale Query-Filter: `name`, `netId`, `characterName`, `characterGuid`. |
| `GET /api/items` | `{"items":[{"name","maxStackSize"}]}` — jedes `ItemData`-Objekt. |
| `GET /api/bans` | `{"bans":[{"netId","name","reason","bannedAt"}]}` — persistente Bans. |
| `GET /api/status` | Server-Identität: `serverName`, `worldName`, `ownerId`, `serverGuid`, `isDedicatedServer`, `crossplayEnabled`, Weltzustand. |

`/api/players`-Eintrag (alle Felder optional — nicht jede Plattform liefert alles):

```json
{
  "name": "ExamplePlayer", "characterName": "ExamplePlayer", "playerId": 1,
  "uniqueNetId": "76561198000000000", "characterGuid": "0123456789ABCDEF0123456789ABCDEF",
  "platform": "PC", "pingMs": 42, "spawned": true,
  "health": {"current": 150.0, "max": 180.0, "canDie": true, "isDead": false},
  "sustenance": {"current": 74.2, "max": 100.0},
  "hydration": {"current": 64.4, "max": 100.0},
  "location": {"x": 66887.4, "y": 185273.2, "z": 138.2},
  "rotation": {"pitch": 0.0, "yaw": 145.7, "roll": 0.0},
  "velocity": {"x": 0.0, "y": 0.0, "z": 0.0},
  "skills": [{"skill": "SKILL_Woodcutting", "currentXP": 69561}],
  "attributes": [{"name": "Health", "value": 150.0, "baseValue": 103.5, "shared": true}]
}
```

### Aktionen

Spieler-Identifikatoren werden gegen `playerId`, `uniqueNetId`,
`characterGuid`, `characterName` und `name` aufgelöst — derselbe String
funktioniert überall.

| Route | Body | Anmerkungen |
|---|---|---|
| `POST /api/kick` | `{"player","reason"?}` | Nativer Kick des Spiels, solange der Server läuft. |
| `POST /api/broadcast` | `{"message","sender"?}` | Nativer Spiel-Chat an alle. |
| `POST /api/tell` | `{"player","message"}` | Private Nachricht; benötigt die ProcessEvent-Ableitung (siehe unten). |
| `POST /api/give` | `{"player","item","count"?}` | `count` standardmäßig 1. |
| `POST /api/ban` | `{"player","reason"?}` | Persistent; Offline-Spieler per SteamID64. |
| `POST /api/unban` | `{"player"}` | |
| `POST /api/teleport` | `{"from","to"}` | Teleportiert `from` auf `to`s Position. |
| `POST /api/tpto` | `{"player","x","y","z"}` | Absolute Weltkoordinaten (doubles). |
| `POST /api/heal` | `{"player","amount"?}` | Vollheilung per Default; `amount` heilt teilweise, auf Max begrenzt. |
| `POST /api/feed` | `{"player"}` | Sättigung → max. |
| `POST /api/hydrate` | `{"player"}` | Hydratation → max. |
| `POST /api/godmode` | `{"player","enabled"?}` | Schaltet `bCanDie`; Default `true`. |

```bash
curl -H "Authorization: Bearer $TOKEN" -X POST \
  -d '{"from":"ExamplePlayer","to":"ExampleFriend"}' http://127.0.0.1:8080/api/teleport
# → {"status":"ok","message":"teleported ExamplePlayer to ExampleFriend"}
```

## 🎮 In-Game-Chat-Befehle

Für Spieler, die das Spiel als Owner oder Admin betrachtet
(`PrivilegeFunctionLibrary`):

```
/kick <spieler> [grund]
/ban <spieler> [grund]
/unban <spieler oder SteamID64>
/give <spieler> <item> [anzahl]
```

> ⚠️ Der Admin-Check wird beim ersten Gebrauch lazy aufgelöst: Game-Feature-
> Klassen registrieren sich nach dem Mod-Boot, ein einmaliger Lookup kann sie
> verpassen. Das ist im Code behandelt — der Lookup wiederholt sich beim
> ersten Befehl, statt den ganzen Lauf scheitern zu lassen.

## 💬 RCON

Source-RCON auf dem konfigurierten Port (per Default aus). Befehle:

```
help                      status
players [json]            player <name>
playercount
kick <spieler> [grund]    ban <spieler> [grund]     unban <spieler|id>
broadcast <nachricht>     tell <spieler> <nachricht>
give <spieler> <item> [anzahl]
tp <spieler> <ziel>       tpto <spieler> <x> <y> <z>
heal <spieler>            feed <spieler>             hydrate <spieler>
godmode <spieler> <on|off>
```

## 🏗️ So funktioniert es

Der Mod ist ein Shared Object, das mit `LD_PRELOAD` injiziert wird. Sein
Konstruktor startet einen Init-Thread, der wartet, bis die Spiel-Engine
bereit ist, und dann Reader und Hooks anhängt. **Nichts wird gelesen oder
aufgerufen, bevor die Engine validiert ist.**

### 🔍 Discovery (warum Patches es nicht crashen)

UE-Globals werden über SDK-Offsets **und** Fallback-Scans lokalisiert, jeweils validiert:

1. `GNames` (der FName-Pool) — SDK-Offset zuerst; bei Fehler wird jedes
   beschreibbare Segment nach einem Pool durchsucht, dessen erster Eintrag
   sich zu `None` dekodiert. Namensstrings werden von hier aufgelöst.
2. `GObjects` (das `TUObjectArray`) — SDK-Offset gegen die Chunk-Struktur
   validiert; bei Fehler ein Scan beschreibbarer Segmente. Objekte werden
   chunkweise durchlaufen.
3. `GWorld` — SDK-Offset validiert (`World`-Klasse + persistentes Level),
   sonst ein Sweep von `GObjects` nach der lebenden `World`.

Klassennamen, Funktions-Lookups und jeder Feldzugriff laufen über validierte
Pointer mit Lesbarkeitsprüfungen — ein verschobenes Feld degradiert zu
fehlenden Daten, nicht zu einem Crash.

Aktuelle Offsets (Build 1.0.1 / CL 248854, Basis-Image `0x200000`):

| Global | Relativ | Absolut |
|---|---|---|
| `GNames` | `0x0DBDAFF8` | `0xDDDAFF8` |
| `GObjects` | `0x0DC96510` | `0xDE96510` |
| `GWorld` | `0x0DD525F8` | `0xDF525F8` |

### 🧵 Game-Thread-Marshalling

Native UE-Funktionen müssen im **Game-Thread** laufen. Der Mod ruft sie nie
aus seinem HTTP-Thread auf:

- Eine Pumpe wird auf einem nativen Server-RPC
  (`PlayerController.ServerUpdateCamera`) installiert; bei jedem Auslösen
  durch einen Spieler wird mit `GameThread::RunSync` eingereihte Arbeit im
  Game-Thread abgearbeitet. So laufen Teleport, Heal, Give und Kicks nativ.
- `UObject::ProcessEvent` ist auf Linux-Shipping-Builds nicht exportiert,
  daher leitet der Mod seine Adresse zur Laufzeit ab (Stack-Walk + geteilter
  Vtable-Slot); bis dahin bleiben ProcessEvent-Funktionen (`tell`,
  `broadcast`) mit einer klaren Logzeile aus.

### 🧩 Native Aufrufe und der Objekt-Pool

`FindFunction(className, functionName)` durchsucht den lebenden Objekt-Pool
und matcht nach Name — deshalb zählen Spiel-Updates, die UFunctions
**umbenennen**, mehr als solche, die Offsets verschieben. In 1.0.1
verschwand `AActor::TeleportTo` aus dem Pool; der Mod fällt auf
`K2_TeleportTo` (den BlueprintCallable-Wrapper) mit passendem Return-Offset
zurück.

## 🔧 Anpassung an einen neuen Spiel-Build

Wenn das Spiel patcht, erwarte einen oder mehrere dieser Fälle — nach
Wahrscheinlichkeit geordnet:

1. **Globals verschoben** (GNames/GObjects/GWorld). Der Segment-Scan findet
   die neuen Werte; Server einmal starten, Log lesen (`GNames at … (segment
   scan)`) und `Offsets::G*` in `src/engine/dom_engine.h` aktualisieren. Die
   alten Werte bleiben im Kommentar.
2. **Struct-Felder verschoben** (Member-Offsets in `dom_engine.h`). Symptom:
   plausible, aber falsche Werte oder leere Felder in `/api/players`. Mit dem
   SDK (RSDWSDK/Dumpspace) neu dumpen oder mit einer Read-only-
   `/proc/<pid>/mem`-Sonde bissektieren.
3. **UFunctions umbenannt/entfernt** (z. B. `TeleportTo` → `K2_TeleportTo`).
   Symptom: `… not found`-Logzeilen und ein Fallback-Pfad. Fix: den neuen
   Namen im Pool auflösen.
4. **Spät registrierte Klassen** (Game-Features). Symptom: ein Lookup
   funktioniert in manchen Läufen und in anderen nicht. Fix: lazy beim
   ersten Gebrauch auflösen, nie einmalig beim Boot.
5. **Engine-Upgrade** (UE minor/major). Alles kann sich verschieben; SDK neu
   dumpen.

Die sicherste Schleife: deployen → Startlog beobachten (jeder
Discovery-Schritt wird geloggt) → `/api/health` + `/api/players` + einen
Action-Endpoint stichproben → erst dann vertrauen.

## 📣 Discord-Integration (dkbot-Plugin)

In Produktion wird die API vom **`dkbot-dragonwilds`-Plugin** im modularen
**[dkbot-base](https://github.com/ApachaiZ/dkbot-base)**-Bot gesteuert.
Plugin installieren und seine `/dw`-Discord-Befehle erscheinen sofort — ohne
Neukompilierung des Bot-Kerns.

### Verkabelung

```text
Discord ──► dkbot-base ──► dkbot-dragonwilds-Plugin (.so, ABI v1)
                                │
                                ▼ HTTP + Bearer-Token
                          REST-API auf 127.0.0.1:8080
                                │
                                ▼ im Prozess (LD_PRELOAD)
                       librsdwapi.so im Spiel-Server
```

1. **Gleicher Host** — Bot und Dedicated-Server laufen auf derselben
   Linux-Maschine, daher bindet die API auf `127.0.0.1` und ist nie dem
   Internet ausgesetzt.
2. **Spielseite** — die `dragonwilds`-systemd-Unit bekommt ein Drop-in mit
   `Environment=LD_PRELOAD=…/librsdwapi.so` und
   `Environment=RSDWAPI_DIR=…/rsdwapi`; der Mod startet seine Listener,
   sobald die Engine bereit ist, und liest `settings.ini` (Bind +
   `BearerToken`).
3. **Botseite** — die `dakkabot`-systemd-Unit lädt ihre `.env`
   (`EnvironmentFile=…/.config.d/.env`):
   `RSDW_API_BASE_URL=http://127.0.0.1:8080` und `RSDW_API_TOKEN=<gleicher
   Wert wie BearerToken in settings.ini>`. Ohne passendes Token erhält jede
   Anfrage `401`.
4. **Befehle** — das reiche Plugin-Manifest wird beim Bot-Start zu echten
   Discord-Slash-Commands: `/dw status`, `/dw players`, `/dw player`,
   `/dw items`, `/dw bans`, `/dw top`, `/dw kick`, `/dw ban`, `/dw unban`,
   `/dw announce`, `/dw tell` und das owner-schaltbare Cheat-Set `/dw tp`,
   `/dw tpto`, `/dw give`, `/dw godmode`, `/dw heal`, `/dw feed`,
   `/dw hydrate`. Der Cheat-Schalter selbst lebt in `/dw-admin cheats`
   (nur Administratoren).
5. **Autovervollständigung** — `/api/players` und `/api/items` speisen die
   Vorschlagslisten (Spieler-Nicknames, Item-Namen) direkt, ohne Cache.
6. **Antworten** — jeder Befehl liefert eine formatierte Discord-Embed-Karte
   (Icons, Felder), niemals einen rohen JSON-Dump.
7. **Sicherheit** — sensible Befehle sind durch Discord-Rollen (Moderator /
   Administrator / Owner) geschützt, und der Owner kann das ganze Cheat-Set
   mit einem Schalter ausblenden; jede Aktion wird in den Bot-Logs auditiert.
8. **Header-Groß-/Kleinschreibung** — reqwest/hyper senden `authorization`
   klein; der case-insensitive Header-Fix in dieser Codebasis macht das Token
   überhaupt erst funktionsfähig.

## 🔒 Sicherheit

- 🔐 In Produktion auf `127.0.0.1` binden und ein Bearer-Token setzen;
  `IPWhitelist` für Multi-Host-Setups. Die API ist sonst unauthentifiziert,
  per Design.
- 📵 RCON bleibt aus, außer benötigt; sein Passwort wird einmal generiert.
- 🚦 Fehlgeschlagene RCON-Logins sind ratenlimitiert und werden pro Adresse
  automatisch geblockt.

## 🩺 Fehlerbehebung

| Symptom | Ursache | Fix |
|---|---|---|
| `/api/health` antwortet dauerhaft `engineReady:false` | Der (ebenfalls injizierte) Launcher-Wrapper hat den REST-Port zuerst belegt | Behoben: Listener binden nur im Prozess, in dem die Engine initialisiert wurde; Mod aktualisieren |
| Clients (reqwest/hyper, Bots) erhalten `401` mit korrektem Token; `curl` funktioniert | Header-Namen kommen klein an (`authorization`); case-sensitiver Lookup | Behoben: Namen beim Parsen kleingemacht (RFC 9110); Mod aktualisieren |
| `chat commands unavailable` / Lookups scheitern in manchen Läufen | Game-Feature-Klassen registrieren sich nach dem Boot | Behoben: lazy Re-Auflösung beim ersten Gebrauch |
| Heal/Teleport „gelingt“, aber der Spieler sieht nichts | Direkte Speicher-Schreibvorgänge umgehen die UE-Replikation (kein Dirty-Mark) | Behoben: native Funktionen im Game-Thread (`IncreaseHealth`, `K2_TeleportTo`) |
