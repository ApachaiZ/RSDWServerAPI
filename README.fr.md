# 🐉 RSDWServerAPI

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)[![Version](https://img.shields.io/badge/version-0.2.1-blue.svg)](#-compatibilité)
[![Jeu](https://img.shields.io/badge/jeu-RuneScape%3A%20Dragonwilds%201.0.1-orange.svg)](#-compatibilité)
[![Moteur](https://img.shields.io/badge/moteur-UE%205.6.1-purple.svg)](#-compatibilité)

## 🧭 À propos

**RSDWServerAPI** transforme un serveur dédié **RuneScape: Dragonwilds** Linux
standard en serveur administrable à distance. Il injecte un plan de contrôle
complet dans le processus du jeu en cours d'exécution :

- 🌐 une **API REST** (stats joueurs en direct, téléportations, soins, dons, kicks, bans…),
- 💬 un écouteur **Source RCON** pour les outils et panneaux,
- 🎮 des **commandes admin en jeu** (`/kick`, `/ban`, `/give`…) pour owners/admins,
- 📣 un **relais Discord** optionnel (chat + annonces de morts comiques).

Il pilote toute la boucle d'administration — depuis un panneau, un script, un
bot Discord (le plugin compagnon
[`dkbot-dragonwilds`](https://github.com/ApachaiZ/dkbot-dragonwilds)), ou tout
programme qui parle HTTP.

> ⭐ **Ceci est un fork** du projet original de **dkoz** —
> **[dkoz/RSDWServerAPI](https://github.com/dkoz/RSDWServerAPI)** — sans qui
> rien de tout cela n'existerait.
>
> 🙏 **Un immense merci au développeur original** pour les fondations solides,
> l'architecture propre (découverte validée, marshalling des appels natifs) et
> la licence MIT qui permet à ce fork de construire dessus. Le fork ajoute des
> correctifs durcis pour la production, les offsets du build 1.0.1 et des
> fonctionnalités admin supplémentaires (téléportation, tell, heal, feed,
> hydrate, god mode). Les contributions amont y restent bienvenues.
>
> 🌐 Langues : 🇬🇧 [English version](README.md) · 🇫🇷 Français · 🇩🇪 [Deutsche Version](README.de.md) · 🇪🇸 [Versión española](README.es.md)

## 🚀 TL;DR — mise en place en 5 minutes

```bash
# 1. Build (Docker ou make)
./build.sh                          # → dist/librsdwapi.so

# 2. Copier à côté du serveur, préfixer la commande de lancement
LD_PRELOAD=./librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port=7777

# 3. Au premier lancement le mod écrit rsdwapi/settings.ini à côté du binaire.
#    L'API REST est ACTIVÉE par défaut (port 8080), RCON désactivé.

# 4. Vérifier que ça marche
curl http://127.0.0.1:8080/api/health
# → {"status":"healthy","engineReady":true}
```

> 🔐 **Avant toute exposition** : mettez `BindAddress=127.0.0.1` et un
> `BearerToken` dans `settings.ini` (puis envoyez
> `Authorization: Bearer <token>`). Voir [🔒 Sécurité](#-sécurité).

## 🧭 Comment ça marche, en un paragraphe

Ce mod est une **bibliothèque partagée injectée dans le processus du jeu** au démarrage
(`LD_PRELOAD`). Il attend que le moteur Unreal soit prêt, localise les globales du
moteur (pool de noms, liste d'objets, monde — avec des scans de repli validés), puis
sert les données de jeu en direct et les actions admin par HTTP. Il ne modifie jamais
les fichiers du jeu, ne touche jamais aux sauvegardes, et ne lit qu'à travers des
pointeurs validés : un patch du jeu se dégrade en un `503` + une ligne de log, pas en
un crash. Le processus du jeu reste inchangé par ailleurs — le même binaire fait
tourner le serveur comme avant.

---

## 📦 Compatibilité

Vérifié en direct sur un serveur dédié de production (Ubuntu, systemd + `LD_PRELOAD`).

| Version du jeu | Build (CL) | Build Steam | Depot Linux | Statut |
|---|---|---|---|---|
| 1.0.0.x | 247725 (`++dominion+hotfix`) | — | — | ✅ vérifié (offsets SDK) |
| **1.0.1** "Eye on Ashenfall" | **248854** (`++dominion+staging`) | **25811348** | **3501791** | ✅ vérifié (offsets mis à jour) |

- Moteur : **UE 5.6.1** — inchangé entre les deux builds ci-dessus.
- Plateforme : serveur dédié Linux (`RSDragonwildsServer-Linux-Shipping`, image de base
  `0x200000`, strippé : pas de `.symtab`, `UObject::ProcessEvent` non exporté).

## ✨ Fonctionnalités

- 📡 **Données joueurs en direct** — noms, GUID de personnage, plateforme, ping, position,
  rotation, vélocité, santé, sustenance, hydratation, chaque compétence avec XP, chaque
  attribut flottant.
- 🚪 **Kick à distance** via la propre fonction de kick du jeu, réappliqué tant que le
  serveur tourne.
- 📢 **Broadcast** et 💬 **tell privé** via le propre chat du jeu.
- 🎁 **Don d'objets** via la propre fonction de give du jeu.
- 🔨 **Bans** qui survivent aux redémarrages, joueurs hors ligne bannissables par SteamID64.
- 🌀 **Téléportation** — joueur sur joueur, ou vers des coordonnées — via le
  `K2_TeleportTo` natif du jeu sur le thread de jeu (reconnu par le moteur, répliqué).
- 💚 **Actions admin** — heal (`IncreaseHealth` natif), feed, hydrate, god mode.
- 🎮 **Commandes admin en jeu** — `/kick`, `/ban`, `/unban`, `/give`, conditionnées à la
  propre liste owner/admin du jeu.
- 📣 **Relais Discord** — chat du jeu et morts de joueurs postés vers des webhooks.
- 🔌 **Deux écouteurs indépendants** — REST et Source RCON, chacun activable séparément.
- 🔐 **Contrôle d'accès sur les deux** — bearer token ou mot de passe RCON, liste
  blanche d'IP, rate limiting par adresse et blocage automatique après échecs répétés.
- 🛡️ **Offsets validés à l'exécution** — chaque étape de découverte est vérifiée et
  contre-vérifiée avant toute lecture ; une discordance après un patch du jeu se
  dégrade en `503` et une ligne de log plutôt que de servir n'importe quoi.

## 🚀 Démarrage rapide

### Build

Docker est la seule exigence. Le conteneur de build est Debian bookworm, identique à
l'image dans laquelle tourne le serveur dédié, donc le résultat ne réclame jamais une
glibc plus récente que celle de l'hôte. `libstdc++` et `libgcc` sont liés statiquement.

```bash
./build.sh
```

Sortie : `dist/librsdwapi.so`.

Un simple `make` fonctionne aussi avec une toolchain locale ; sur les hôtes sans
`libstdc++` statique (Fedora…), retirez les flags statiques :

```bash
make LDFLAGS="-shared -pthread -ldl"
```

### Installation

Copiez `dist/librsdwapi.so` à la racine du serveur et préfixez la commande de
lancement :

```bash
LD_PRELOAD=./librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port=7777
```

### systemd (mise en place production)

Le drop-in ci-dessous est ce qui tourne en production : `LD_PRELOAD` s'applique à tout
le service (le script de lancement *et* le binaire du jeu sont injectés — sans danger,
voir [🏗️ Comment ça marche](#️-comment-ça-marche)), et le dossier de données est fixé
avec `RSDWAPI_DIR` :

```ini
# /etc/systemd/system/dragonwilds.service.d/rsdwapi.conf
[Service]
Environment=RSDWAPI_DIR=/home/steam/dragonwilds_linux_server/rsdwapi
Environment=LD_PRELOAD=/home/steam/dragonwilds_linux_server/librsdwapi.so
```

Sur Pterodactyl ou Pelican, mettez le `.so` à la racine du serveur et réglez le
démarrage sur :

```
LD_PRELOAD=/home/container/librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port={{SERVER_PORT}} -ini:Game:[/Script/Engine.GameSession]:MaxPlayers={{MAX_PLAYERS}}
```

Allouez le port de l'API (et le port RCON, si vous l'activez) dans le panneau.

## ⚙️ Configuration

Au premier lancement, le mod écrit `rsdwapi/settings.ini` à côté du binaire du serveur
(`RSDragonwilds/Binaries/Linux/rsdwapi/` sur une installation standard). Utilisez
`RSDWAPI_DIR` pour le mettre à un endroit plus accessible. Le fichier est relu au
démarrage du serveur.

Un bearer token et un mot de passe RCON sont générés au premier lancement et ne sont
plus jamais régénérés — une valeur vide reste vide.

**L'API REST est activée par défaut. RCON est désactivé par défaut.**

```ini
[General]
EnableLogging=true        # étapes de découverte + chaque requête
VerboseLogging=true       # détail des scans supplémentaires ; laissez actif sauf si bruyant

[API]
Enabled=true
BindAddress=0.0.0.0       # 🔒 préférez 127.0.0.1 + un token en production
Port=8080
BearerToken=              # vide = aucune auth du tout (déconseillé)
IPWhitelist=              # IPs / CIDR séparés par des virgules ; vide = tout le monde

[RCON]
Enabled=false             # Source RCON, désactivé par défaut
BindAddress=0.0.0.0
Port=27020
Password=                 # vide = RCON ne démarre pas, par conception
IPWhitelist=
CommandsPerMinute=60
CommandBurst=15
MaxFailedAuth=5
FailWindowSeconds=60
BanSeconds=300
MaxConnections=16

[Discord]
Enabled=false
WebhookUrl=               # relais chat
DeathWebhookUrl=          # morts ; retombe sur WebhookUrl si vide
Username=Dragonwilds
```

## 🌐 API REST

Tous les endpoints vivent sous `/api/`. Quand un bearer token est configuré, envoyez
`Authorization: Bearer <token>` (les noms d'en-tête sont insensibles à la casse —
reqwest/hyper les mettent en minuscules et c'est géré).

- `200` avec `{"status":"ok","message":"…"}` en cas de succès.
- `401` token manquant/erroné · `404` route inconnue · `503` moteur pas prêt
  (`{"error":"engine not initialised"}`).

### Lectures

| Route | Description |
|---|---|
| `GET /api/hello` | Ping de vie. |
| `GET /api/health` | `{"status","version","uptimeSeconds","engineReady"}`. |
| `GET /api/players` | Tous les joueurs connectés, détail complet. Filtres de requête optionnels : `name`, `netId`, `characterName`, `characterGuid`. |
| `GET /api/items` | `{"items":[{"name","maxStackSize"}]}` — chaque objet `ItemData`. |
| `GET /api/bans` | `{"bans":[{"netId","name","reason","bannedAt"}]}` — bans persistants. |
| `GET /api/status` | Identité du serveur : `serverName`, `worldName`, `ownerId`, `serverGuid`, `isDedicatedServer`, `crossplayEnabled`, état du monde. |

Entrée de `/api/players` (tous les champs sont optionnels — toutes les plateformes
n'exposent pas tout) :

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

### Actions

Les identifiants de joueur sont résolus contre `playerId`, `uniqueNetId`,
`characterGuid`, `characterName` et `name` — la même chaîne fonctionne partout.

| Route | Corps | Notes |
|---|---|---|
| `POST /api/kick` | `{"player","reason"?}` | Kick natif du jeu, réappliqué tant que le serveur tourne. |
| `POST /api/broadcast` | `{"message","sender"?}` | Chat natif du jeu vers tout le monde. |
| `POST /api/tell` | `{"player","message"}` | Message privé ; requiert la dérivation de ProcessEvent (voir plus bas). |
| `POST /api/give` | `{"player","item","count"?}` | `count` par défaut à 1. |
| `POST /api/ban` | `{"player","reason"?}` | Persistant ; joueurs hors ligne par SteamID64. |
| `POST /api/unban` | `{"player"}` | |
| `POST /api/teleport` | `{"from","to"}` | Téléporte `from` sur la position de `to`. |
| `POST /api/tpto` | `{"player","x","y","z"}` | Coordonnées monde absolues (doubles). |
| `POST /api/heal` | `{"player","amount"?}` | Soin complet par défaut ; `amount` soigne partiellement, plafonné au max. |
| `POST /api/feed` | `{"player"}` | Sustenance → max. |
| `POST /api/hydrate` | `{"player"}` | Hydratation → max. |
| `POST /api/godmode` | `{"player","enabled"?}` | Bascule `bCanDie` ; défaut `true`. |

```bash
curl -H "Authorization: Bearer $TOKEN" -X POST \
  -d '{"from":"ExamplePlayer","to":"ExampleFriend"}' http://127.0.0.1:8080/api/teleport
# → {"status":"ok","message":"teleported ExamplePlayer to ExampleFriend"}
```

## 🎮 Commandes chat en jeu

Pour les joueurs que le jeu considère owner ou admin (`PrivilegeFunctionLibrary`) :

```
/kick <joueur> [raison]
/ban <joueur> [raison]
/unban <joueur ou SteamID64>
/give <joueur> <objet> [nombre]
```

> ⚠️ Le contrôle admin se résout paresseusement au premier usage : les classes de
> game features s'enregistrent après le boot du mod, donc une résolution unique peut
> les manquer. C'est géré dans le code — la résolution retente à la première commande
> au lieu de faire échouer tout le lancement.

## 💬 RCON

Source RCON sur le port configuré (désactivé par défaut). Commandes :

```
help                      status
players [json]            player <nom>
playercount
kick <joueur> [raison]    ban <joueur> [raison]     unban <joueur|id>
broadcast <message>       tell <joueur> <message>
give <joueur> <objet> [nombre]
tp <joueur> <cible>       tpto <joueur> <x> <y> <z>
heal <joueur>             feed <joueur>             hydrate <joueur>
godmode <joueur> <on|off>
```

## 🏗️ Comment ça marche

Le mod est un objet partagé injecté avec `LD_PRELOAD`. Son constructeur lance un thread
d'init qui attend que le moteur du jeu soit prêt, puis attache lecteurs et hooks.
**Rien n'est lu ni appelé avant que le moteur ne soit validé.**

### 🔍 Découverte (pourquoi les patches ne le font pas planter)

Les globales UE sont localisées via les offsets SDK **et** des scans de repli, chacun
validé :

1. `GNames` (le pool FName) — offset SDK d'abord ; en échec, chaque segment
   inscriptible est scanné pour un pool dont la première entrée se décode en `None`.
   Les chaînes de noms se résolvent depuis ici.
2. `GObjects` (le `TUObjectArray`) — offset SDK validé contre la structure de chunks ;
   en échec, scan des segments inscriptibles. Les objets sont parcourus par chunk.
3. `GWorld` — offset SDK validé (classe `World` + niveau persistant), sinon balayage
   de `GObjects` pour le `World` vivant.

Les noms de classes, les recherches de fonctions et chaque lecture de champ passent par
des pointeurs validés avec vérifications de lisibilité — un champ déplacé se dégrade en
donnée manquante, pas en crash.

Offsets actuels (build 1.0.1 / CL 248854, image de base `0x200000`) :

| Globale | Relative | Absolue |
|---|---|---|
| `GNames` | `0x0DBDAFF8` | `0xDDDAFF8` |
| `GObjects` | `0x0DC96510` | `0xDE96510` |
| `GWorld` | `0x0DD525F8` | `0xDF525F8` |

### 🧵 Marshalling sur le thread de jeu

Les fonctions natives UE doivent s'exécuter sur le **thread de jeu**. Le mod ne les
appelle jamais depuis son thread HTTP :

- Une pompe est installée sur un RPC serveur natif (`PlayerController.ServerUpdateCamera`) ;
  à chaque déclenchement par un joueur, le travail mis en file avec `GameThread::RunSync`
  est drainé sur le thread de jeu. C'est ainsi que téléportation, heal, give et kicks
  s'exécutent nativement.
- `UObject::ProcessEvent` n'est pas exporté sur les builds Linux Shipping, donc le mod
  dérive son adresse à l'exécution (parcours de pile + slot de vtable partagé) ; tant
  qu'elle n'est pas dérivée, les fonctionnalités ProcessEvent (`tell`, `broadcast`)
  restent désactivées avec une ligne de log claire.

### 🧩 Appels natifs et pool d'objets

`FindFunction(className, functionName)` balaye le pool d'objets vivant et matche par
nom — c'est pourquoi les mises à jour du jeu qui **renomment** des UFunctions comptent
plus que celles qui déplacent des offsets. Sur 1.0.1, `AActor::TeleportTo` a disparu du
pool ; le mod retombe sur `K2_TeleportTo` (le wrapper BlueprintCallable) avec l'offset
de retour correspondant.

## 🔧 Adapter à un nouveau build du jeu

Quand le jeu patche, attendez-vous à un ou plusieurs de ces cas — par ordre de
probabilité :

1. **Globales déplacées** (GNames/GObjects/GWorld). Le scan de segments trouve les
   nouvelles valeurs ; lancez le serveur une fois, lisez le log (`GNames at … (segment
   scan)`) et mettez à jour `Offsets::G*` dans `src/engine/dom_engine.h`. Les anciennes
   valeurs sont conservées en commentaire.
2. **Champs de struct déplacés** (offsets de membres de classes dans `dom_engine.h`).
   Symptôme : valeurs plausibles mais fausses, ou champs vides dans `/api/players`.
   Re-dumpez avec le SDK (RSDWSDK/Dumpspace) ou bissectez avec une sonde en lecture
   seule `/proc/<pid>/mem`.
3. **UFunctions renommées/supprimées** (ex. `TeleportTo` → `K2_TeleportTo`). Symptôme :
   lignes `… not found` dans le log et un chemin de repli. Correctif : résolvez le
   nouveau nom dans le pool.
4. **Classes à enregistrement tardif** (game features). Symptôme : une résolution
   fonctionne sur certains lancements et pas d'autres. Correctif : résolvez paresseusement
   au premier usage, jamais une fois au boot.
5. **Mise à niveau du moteur** (UE mineur/majeur). Tout peut bouger ; re-dumpez le SDK.

La boucle la plus sûre : déployez → surveillez le log de démarrage (chaque étape de
découverte est loggée) → vérifiez `/api/health` + `/api/players` + un endpoint d'action
→ seulement ensuite, faites confiance.

## 📣 Intégration Discord (plugin dkbot)

En production, l'API est pilotée par le **plugin `dkbot-dragonwilds`** exécuté dans le
bot modulaire **[dkbot-base](https://github.com/ApachaiZ/dkbot-base)**. Installez le
plugin et ses commandes Discord `/dw` apparaissent immédiatement — sans recompilation
du cœur du bot.

### Comment c'est branché

```text
Discord ──► dkbot-base ──► plugin dkbot-dragonwilds (.so, ABI v1)
                                │
                                ▼ HTTP + Bearer token
                          API REST sur 127.0.0.1:8080
                                │
                                ▼ dans le processus (LD_PRELOAD)
                       librsdwapi.so dans le serveur de jeu
```

1. **Même hôte** — le bot et le serveur dédié tournent sur la même machine Linux,
   donc l'API se lie sur `127.0.0.1` et ne fait jamais face à internet.
2. **Côté jeu** — l'unité systemd `dragonwilds` reçoit un drop-in avec
   `Environment=LD_PRELOAD=…/librsdwapi.so` et
   `Environment=RSDWAPI_DIR=…/rsdwapi` ; le mod démarre ses écouteurs une fois le
   moteur prêt et lit `settings.ini` (bind + `BearerToken`).
3. **Côté bot** — l'unité systemd `dakkabot` charge son `.env`
   (`EnvironmentFile=…/.config.d/.env`) :
   `RSDW_API_BASE_URL=http://127.0.0.1:8080` et `RSDW_API_TOKEN=<même valeur
   que BearerToken dans settings.ini>`. Sans token correspondant, chaque requête
   reçoit `401`.
4. **Commandes** — le manifeste riche du plugin devient de vraies slash commands
   Discord au démarrage du bot : `/dw status`, `/dw players`, `/dw player`,
   `/dw items`, `/dw bans`, `/dw top`, `/dw kick`, `/dw ban`, `/dw unban`,
   `/dw announce`, `/dw tell`, et l'ensemble cheat commutable par le propriétaire
   `/dw tp`, `/dw tpto`, `/dw give`, `/dw godmode`, `/dw heal`, `/dw feed`,
   `/dw hydrate`. Le switch cheat lui-même vit dans `/dw-admin cheats`
   (administrateurs uniquement).
5. **Autocomplétion** — `/api/players` et `/api/items` alimentent les listes de
   suggestions (pseudos de joueurs, noms d'objets) directement, sans cache.
6. **Réponses** — chaque commande renvoie une carte Discord formatée
   (icônes, champs), jamais un dump JSON brut.
7. **Sécurité** — les commandes sensibles sont conditionnées aux rôles Discord
   (modérateur / administrateur / propriétaire) et le propriétaire peut masquer tout
   l'ensemble cheat d'un seul switch ; chaque action est auditée dans les logs du bot.
8. **Casse des en-têtes** — reqwest/hyper envoient `authorization` en minuscules ; le
   correctif d'en-têtes insensibles à la casse de ce code est ce qui fait marcher le
   token tout court.

## 🔒 Sécurité

- 🔐 Liez sur `127.0.0.1` et mettez un bearer token en production ; gardez
  `IPWhitelist` pour les configurations multi-hôtes. L'API est sinon non authentifiée,
  par conception.
- 📵 RCON reste désactivé sauf besoin ; son mot de passe est généré une fois.
- 🚦 Les connexions RCON en échec sont limitées en débit et auto-bloquées par adresse.

## 🩺 Dépannage

| Symptôme | Cause | Correctif |
|---|---|---|
| `/api/health` répond `engineReady:false` pour toujours | Le wrapper du lanceur (aussi injecté) a pris le port REST en premier | Corrigé : les écouteurs ne se lient que dans le processus où le moteur s'est initialisé ; mettez à jour le mod |
| Les clients (reqwest/hyper, bots) reçoivent `401` avec le bon token ; `curl` marche | Les noms d'en-tête arrivent en minuscules (`authorization`) ; recherche sensible à la casse | Corrigé : noms mis en minuscules au parse (RFC 9110) ; mettez à jour le mod |
| `chat commands unavailable` / résolutions qui échouent sur certains lancements | Les classes de game features s'enregistrent après le boot | Corrigé : re-résolution paresseuse au premier usage |
| Heal/téléportation « réussit » mais le joueur ne voit rien | Les écritures mémoire directes contournent la réplication UE (pas de dirty mark) | Corrigé : fonctions natives sur le thread de jeu (`IncreaseHealth`, `K2_TeleportTo`) |
