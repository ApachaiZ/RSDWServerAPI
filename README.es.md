# 🐉 RSDWServerAPI

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)[![Version](https://img.shields.io/badge/version-0.2.1-blue.svg)](#-compatibilidad)
[![Juego](https://img.shields.io/badge/juego-RuneScape%3A%20Dragonwilds%201.0.1-orange.svg)](#-compatibilidad)
[![Motor](https://img.shields.io/badge/motor-UE%205.6.1-purple.svg)](#-compatibilidad)

## 🧭 Acerca de

**RSDWServerAPI** convierte un servidor dedicado estándar de
**RuneScape: Dragonwilds** en Linux en un servidor administrable a distancia.
Inyecta un plano de control completo en el proceso del juego en ejecución:

- 🌐 una **API REST** (stats de jugadores en vivo, teletransportes, curas, objetos, kicks, baneos…),
- 💬 un listener **Source RCON** para herramientas y paneles,
- 🎮 **comandos admin dentro del juego** (`/kick`, `/ban`, `/give`…) para owners/admins,
- 📣 un **relay de Discord** opcional (chat + anuncios de muertes cómicos).

Impulsa todo el ciclo de administración — desde un panel, un script, un bot de
Discord (el plugin compañero
[`dkbot-dragonwilds`](https://github.com/ApachaiZ/dkbot-dragonwilds)) o
cualquier cosa que hable HTTP.

> ⭐ **Esto es un fork** del proyecto original de **dkoz** —
> **[dkoz/RSDWServerAPI](https://github.com/dkoz/RSDWServerAPI)** — sin el
> cual nada de esto existiría.
>
> 🙏 **Enormes gracias al desarrollador original** por la base sólida, la
> arquitectura limpia (descubrimiento validado, marshalling de llamadas
> nativas) y la licencia MIT que permite a este fork construir sobre ella. El
> fork añade correcciones endurecidas para producción, offsets del build 1.0.1
> y funciones admin adicionales (teletransporte, tell, heal, feed, hydrate,
> modo dios). Las contribuciones upstream siguen siendo bienvenidas allí.
>
> 🌐 Idiomas: 🇬🇧 [English version](README.md) · 🇫🇷 [Version française](README.fr.md) · 🇩🇪 [Deutsche Version](README.de.md) · 🇪🇸 Español

## 🚀 TL;DR — configuración en 5 minutos

```bash
# 1. Compilar (Docker o make)
./build.sh                          # → dist/librsdwapi.so

# 2. Copiar junto al servidor, prefijar el comando de arranque
LD_PRELOAD=./librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port=7777

# 3. En el primer arranque el mod escribe rsdwapi/settings.ini junto al binario.
#    La API REST está ACTIVADA por defecto (puerto 8080), RCON desactivado.

# 4. Comprobar que funciona
curl http://127.0.0.1:8080/api/health
# → {"status":"healthy","engineReady":true}
```

> 🔐 **Antes de exponer nada**: pon `BindAddress=127.0.0.1` y un
> `BearerToken` en `settings.ini` (y envía
> `Authorization: Bearer <token>`). Ver [🔒 Seguridad](#-seguridad).

## 🧭 Cómo funciona, en un párrafo

Este mod es una **biblioteca compartida inyectada en el proceso del juego** al
arrancar (`LD_PRELOAD`). Espera a que el motor Unreal esté listo, localiza las
globales del motor (pool de nombres, lista de objetos, mundo — con escaneos de
respaldo validados) y sirve datos de juego en vivo y acciones admin por HTTP.
Nunca modifica archivos del juego, nunca toca los saves, y solo lee a través de
punteros validados: un parche del juego se degrada a un `503` + una línea de
log, no a un cuelgue. El proceso del juego queda intacto por lo demás — el
mismo binario ejecuta el servidor como antes.

---

## 📦 Compatibilidad

Verificado en vivo en un servidor dedicado de producción (Ubuntu, systemd + `LD_PRELOAD`).

| Versión del juego | Build (CL) | Build de Steam | Depot Linux | Estado |
|---|---|---|---|---|
| 1.0.0.x | 247725 (`++dominion+hotfix`) | — | — | ✅ verificado (offsets SDK) |
| **1.0.1** "Eye on Ashenfall" | **248854** (`++dominion+staging`) | **25811348** | **3501791** | ✅ verificado (offsets actualizados) |

- Motor: **UE 5.6.1** — sin cambios entre los dos builds de arriba.
- Plataforma: servidor dedicado Linux (`RSDragonwildsServer-Linux-Shipping`,
  imagen base `0x200000`, despojado: sin `.symtab`,
  `UObject::ProcessEvent` no exportado).

## ✨ Funciones

- 📡 **Datos de jugadores en vivo** — nombres, GUID de personaje, plataforma, ping, posición,
  rotación, velocidad, salud, sustento, hidratación, cada habilidad con XP, cada
  atributo flotante.
- 🚪 **Kick remoto** mediante la propia función de kick del juego, reaplicado mientras el
  servidor corre.
- 📢 **Broadcast** y 💬 **tell privado** mediante el propio chat del juego.
- 🎁 **Dar objetos** mediante la propia función de give del juego.
- 🔨 **Baneos** que sobreviven a los reinicios, jugadores offline baneables por SteamID64.
- 🌀 **Teletransporte** — jugador sobre jugador, o a coordenadas — mediante el
  `K2_TeleportTo` nativo del juego en el hilo de juego (reconocido por el motor, replicado).
- 💚 **Acciones admin** — heal (`IncreaseHealth` nativo), feed, hydrate, modo dios.
- 🎮 **Comandos admin en el juego** — `/kick`, `/ban`, `/unban`, `/give`, protegidos por la
  propia lista owner/admin del juego.
- 📣 **Relay de Discord** — chat del juego y muertes de jugadores a webhooks.
- 🔌 **Dos listeners independientes** — REST y Source RCON, cada uno activable por separado.
- 🔐 **Control de acceso en ambos** — bearer token o contraseña RCON, lista blanca
  de IPs, rate limiting por dirección y bloqueo automático tras fallos repetidos.
- 🛡️ **Offsets validados en tiempo de ejecución** — cada paso de descubrimiento se verifica y
  contraverifica antes de leer nada; un desajuste tras un parche del juego se degrada a
  `503` y una línea de log, no a servir disparates.

## 🚀 Inicio rápido

### Compilar

Docker es el único requisito. El contenedor de build es Debian bookworm,
idéntico a la imagen en la que corre el servidor dedicado, por lo que el
resultado nunca pide al host una glibc más nueva. `libstdc++` y `libgcc` se
enlazan estáticamente.

```bash
./build.sh
```

Salida: `dist/librsdwapi.so`.

Un simple `make` también funciona con una toolchain local; en hosts sin
`libstdc++` estática (Fedora…), quita los flags estáticos:

```bash
make LDFLAGS="-shared -pthread -ldl"
```

### Instalación

Copia `dist/librsdwapi.so` a la raíz del servidor y prefija el comando de
arranque:

```bash
LD_PRELOAD=./librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port=7777
```

### systemd (configuración de producción)

El drop-in de abajo es lo que corre en producción: `LD_PRELOAD` se aplica a
todo el servicio (el script lanzador *y* el binario del juego se inyectan —
inofensivo, ver [🏗️ Cómo funciona](#️-cómo-funciona)), y el directorio de
datos se fija con `RSDWAPI_DIR`:

```ini
# /etc/systemd/system/dragonwilds.service.d/rsdwapi.conf
[Service]
Environment=RSDWAPI_DIR=/home/steam/dragonwilds_linux_server/rsdwapi
Environment=LD_PRELOAD=/home/steam/dragonwilds_linux_server/librsdwapi.so
```

En Pterodactyl o Pelican, pon el `.so` en la raíz del servidor y ajusta el
arranque a:

```
LD_PRELOAD=/home/container/librsdwapi.so ./RSDragonwilds/Binaries/Linux/RSDragonwildsServer-Linux-Shipping -log -port={{SERVER_PORT}} -ini:Game:[/Script/Engine.GameSession]:MaxPlayers={{MAX_PLAYERS}}
```

Asigna el puerto de la API (y el de RCON, si lo activas) en el panel.

## ⚙️ Configuración

En el primer arranque el mod escribe `rsdwapi/settings.ini` junto al binario
del servidor (`RSDragonwilds/Binaries/Linux/rsdwapi/` en una instalación
estándar). Usa `RSDWAPI_DIR` para ponerlo en un lugar más accesible. El
archivo se relee al arrancar el servidor.

Un bearer token y una contraseña RCON se generan en el primer arranque y nunca
se regeneran — un valor vacío permanece vacío.

**La API REST está activada por defecto. RCON está desactivado por defecto.**

```ini
[General]
EnableLogging=true        # pasos de descubrimiento + cada petición
VerboseLogging=true       # detalle extra de escaneos; déjalo salvo que moleste

[API]
Enabled=true
BindAddress=0.0.0.0       # 🔒 en producción prefiere 127.0.0.1 + un token
Port=8080
BearerToken=              # vacío = sin auth alguna (no recomendado)
IPWhitelist=              # IPs / CIDRs separados por comas; vacío = todos

[RCON]
Enabled=false             # Source RCON, desactivado por defecto
BindAddress=0.0.0.0
Port=27020
Password=                 # vacío = RCON no arranca, por diseño
IPWhitelist=
CommandsPerMinute=60
CommandBurst=15
MaxFailedAuth=5
FailWindowSeconds=60
BanSeconds=300
MaxConnections=16

[Discord]
Enabled=false
WebhookUrl=               # relay de chat
DeathWebhookUrl=          # muertes; cae a WebhookUrl si está vacío
Username=Dragonwilds
```

## 🌐 API REST

Todos los endpoints viven bajo `/api/`. Cuando hay un bearer token
configurado, envía `Authorization: Bearer <token>` (los nombres de cabecera
son insensibles a mayúsculas — reqwest/hyper los ponen en minúsculas y eso
está gestionado).

- `200` con `{"status":"ok","message":"…"}` en caso de éxito.
- `401` token ausente/incorrecto · `404` ruta desconocida · `503` motor no
  listo (`{"error":"engine not initialised"}`).

### Lecturas

| Ruta | Descripción |
|---|---|
| `GET /api/hello` | Ping de vida. |
| `GET /api/health` | `{"status","version","uptimeSeconds","engineReady"}`. |
| `GET /api/players` | Todos los jugadores conectados, detalle completo. Filtros de consulta opcionales: `name`, `netId`, `characterName`, `characterGuid`. |
| `GET /api/items` | `{"items":[{"name","maxStackSize"}]}` — cada objeto `ItemData`. |
| `GET /api/bans` | `{"bans":[{"netId","name","reason","bannedAt"}]}` — baneos persistentes. |
| `GET /api/status` | Identidad del servidor: `serverName`, `worldName`, `ownerId`, `serverGuid`, `isDedicatedServer`, `crossplayEnabled`, estado del mundo. |

Entrada de `/api/players` (todos los campos son opcionales — no todas las
plataformas exponen todos):

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

### Acciones

Los identificadores de jugador se resuelven contra `playerId`, `uniqueNetId`,
`characterGuid`, `characterName` y `name` — la misma cadena funciona en todas
partes.

| Ruta | Cuerpo | Notas |
|---|---|---|
| `POST /api/kick` | `{"player","reason"?}` | Kick nativo del juego, reaplicado mientras el servidor corre. |
| `POST /api/broadcast` | `{"message","sender"?}` | Chat nativo del juego a todos. |
| `POST /api/tell` | `{"player","message"}` | Mensaje privado; requiere la derivación de ProcessEvent (ver abajo). |
| `POST /api/give` | `{"player","item","count"?}` | `count` por defecto 1. |
| `POST /api/ban` | `{"player","reason"?}` | Persistente; jugadores offline por SteamID64. |
| `POST /api/unban` | `{"player"}` | |
| `POST /api/teleport` | `{"from","to"}` | Teletransporta `from` a la posición de `to`. |
| `POST /api/tpto` | `{"player","x","y","z"}` | Coordenadas absolutas del mundo (doubles). |
| `POST /api/heal` | `{"player","amount"?}` | Cura completa por defecto; `amount` cura parcialmente, limitado al máximo. |
| `POST /api/feed` | `{"player"}` | Sustento → máx. |
| `POST /api/hydrate` | `{"player"}` | Hidratación → máx. |
| `POST /api/godmode` | `{"player","enabled"?}` | Conmuta `bCanDie`; por defecto `true`. |

```bash
curl -H "Authorization: Bearer $TOKEN" -X POST \
  -d '{"from":"ExamplePlayer","to":"ExampleFriend"}' http://127.0.0.1:8080/api/teleport
# → {"status":"ok","message":"teleported ExamplePlayer to ExampleFriend"}
```

## 🎮 Comandos de chat en el juego

Para jugadores que el juego considera owner o admin
(`PrivilegeFunctionLibrary`):

```
/kick <jugador> [motivo]
/ban <jugador> [motivo]
/unban <jugador o SteamID64>
/give <jugador> <objeto> [cantidad]
```

> ⚠️ La comprobación de admin se resuelve perezosamente en el primer uso: las
> clases de game features se registran después del arranque del mod, por lo
> que una resolución única puede fallarlas. Está gestionado en el código — la
> resolución reintenta en el primer comando en lugar de fallar todo el arranque.

## 💬 RCON

Source RCON en el puerto configurado (desactivado por defecto). Comandos:

```
help                      status
players [json]            player <nombre>
playercount
kick <jugador> [motivo]   ban <jugador> [motivo]    unban <jugador|id>
broadcast <mensaje>       tell <jugador> <mensaje>
give <jugador> <objeto> [cantidad]
tp <jugador> <objetivo>   tpto <jugador> <x> <y> <z>
heal <jugador>            feed <jugador>            hydrate <jugador>
godmode <jugador> <on|off>
```

## 🏗️ Cómo funciona

El mod es un objeto compartido inyectado con `LD_PRELOAD`. Su constructor
lanza un hilo de init que espera a que el motor del juego esté listo y luego
adjunta lectores y hooks. **Nada se lee ni se llama antes de validar el motor.**

### 🔍 Descubrimiento (por qué los parches no lo rompen)

Las globales de UE se localizan mediante offsets del SDK **y** escaneos de
respaldo, cada uno validado:

1. `GNames` (el pool FName) — offset SDK primero; al fallar, se escanea cada
   segmento escribible en busca de un pool cuya primera entrada decodifica a
   `None`. Las cadenas de nombres se resuelven desde aquí.
2. `GObjects` (el `TUObjectArray`) — offset SDK validado contra la estructura
   de chunks; al fallar, escaneo de segmentos escribibles. Los objetos se
   recorren por chunk.
3. `GWorld` — offset SDK validado (clase `World` + nivel persistente), si no,
   barrido de `GObjects` en busca del `World` vivo.

Los nombres de clase, las búsquedas de funciones y cada lectura de campo pasan
por punteros validados con comprobaciones de legibilidad — un campo movido se
degrada a dato faltante, no a un cuelgue.

Offsets actuales (build 1.0.1 / CL 248854, imagen base `0x200000`):

| Global | Relativo | Absoluto |
|---|---|---|
| `GNames` | `0x0DBDAFF8` | `0xDDDAFF8` |
| `GObjects` | `0x0DC96510` | `0xDE96510` |
| `GWorld` | `0x0DD525F8` | `0xDF525F8` |

### 🧵 Marshalling al hilo de juego

Las funciones nativas de UE deben ejecutarse en el **hilo de juego**. El mod
nunca las llama desde su hilo HTTP:

- Se instala una bomba en un RPC nativo del servidor
  (`PlayerController.ServerUpdateCamera`); cada vez que un jugador lo dispara,
  el trabajo encolado con `GameThread::RunSync` se drena en el hilo de juego.
  Así se ejecutan nativamente teletransporte, heal, give y kicks.
- `UObject::ProcessEvent` no está exportado en builds Linux Shipping, así que
  el mod deriva su dirección en tiempo de ejecución (recorrido de pila + slot
  de vtable compartido); hasta derivarla, las funciones de ProcessEvent
  (`tell`, `broadcast`) permanecen apagadas con una línea de log clara.

### 🧩 Llamadas nativas y el pool de objetos

`FindFunction(className, functionName)` barre el pool de objetos vivo y
empareja por nombre — por eso las actualizaciones del juego que **renombran**
UFunctions importan más que las que mueven offsets. En 1.0.1,
`AActor::TeleportTo` desapareció del pool; el mod cae a `K2_TeleportTo` (el
wrapper BlueprintCallable) con el offset de retorno correspondiente.

## 🔧 Adaptarse a un nuevo build del juego

Cuando el juego parchea, espera uno o varios de estos casos — por orden de
probabilidad:

1. **Globales movidas** (GNames/GObjects/GWorld). El escaneo de segmentos
   encuentra los nuevos valores; arranca el servidor una vez, lee el log
   (`GNames at … (segment scan)`) y actualiza `Offsets::G*` en
   `src/engine/dom_engine.h`. Los valores anteriores se conservan en el
   comentario.
2. **Campos de struct movidos** (offsets de miembros en `dom_engine.h`).
   Síntoma: valores plausibles pero erróneos, o campos vacíos en
   `/api/players`. Re-vuelca con el SDK (RSDWSDK/Dumpspace) o bisecciona con
   una sonda de solo lectura `/proc/<pid>/mem`.
3. **UFunctions renombradas/eliminadas** (p. ej. `TeleportTo` →
   `K2_TeleportTo`). Síntoma: líneas `… not found` en el log y una ruta de
   respaldo. Arreglo: resuelve el nuevo nombre en el pool.
4. **Clases de registro tardío** (game features). Síntoma: una resolución
   funciona en unos arranques y no en otros. Arreglo: resuelve perezosamente
   en el primer uso, nunca una vez al arrancar.
5. **Actualización del motor** (UE menor/mayor). Todo puede moverse; re-vuelca
   el SDK.

El ciclo más seguro: despliega → observa el log de arranque (cada paso de
descubrimiento se registra) → comprueba `/api/health` + `/api/players` + un
endpoint de acción → solo entonces confía.

## 📣 Integración con Discord (plugin dkbot)

En producción, la API la maneja el **plugin `dkbot-dragonwilds`** dentro del
bot modular **[dkbot-base](https://github.com/ApachaiZ/dkbot-base)**.
Instala el plugin y sus comandos de Discord `/dw` aparecen inmediatamente —
sin recompilar el núcleo del bot.

### Cómo está conectado

```text
Discord ──► dkbot-base ──► plugin dkbot-dragonwilds (.so, ABI v1)
                                │
                                ▼ HTTP + Bearer token
                          API REST en 127.0.0.1:8080
                                │
                                ▼ dentro del proceso (LD_PRELOAD)
                       librsdwapi.so en el servidor de juego
```

1. **Mismo host** — el bot y el servidor dedicado corren en la misma máquina
   Linux, así que la API se enlaza a `127.0.0.1` y nunca mira a internet.
2. **Lado del juego** — la unidad systemd `dragonwilds` recibe un drop-in con
   `Environment=LD_PRELOAD=…/librsdwapi.so` y
   `Environment=RSDWAPI_DIR=…/rsdwapi`; el mod arranca sus listeners cuando el
   motor está listo y lee `settings.ini` (bind + `BearerToken`).
3. **Lado del bot** — la unidad systemd `dakkabot` carga su `.env`
   (`EnvironmentFile=…/.config.d/.env`):
   `RSDW_API_BASE_URL=http://127.0.0.1:8080` y `RSDW_API_TOKEN=<mismo valor
   que BearerToken en settings.ini>`. Sin token coincidente, cada petición
   recibe `401`.
4. **Comandos** — el manifiesto rico del plugin se convierte en slash commands
   reales de Discord al arrancar el bot: `/dw status`, `/dw players`,
   `/dw player`, `/dw items`, `/dw bans`, `/dw top`, `/dw kick`, `/dw ban`,
   `/dw unban`, `/dw announce`, `/dw tell`, y el conjunto cheat conmutable
   por el propietario `/dw tp`, `/dw tpto`, `/dw give`, `/dw godmode`,
   `/dw heal`, `/dw feed`, `/dw hydrate`. El interruptor de cheats vive en
   `/dw-admin cheats` (solo administradores).
5. **Autocompletado** — `/api/players` y `/api/items` alimentan las listas de
   sugerencias (apodos de jugadores, nombres de objetos) directamente, sin caché.
6. **Respuestas** — cada comando devuelve una tarjeta Discord formateada
   (iconos, campos), nunca un volcado JSON crudo.
7. **Seguridad** — los comandos sensibles están protegidos por roles de
   Discord (moderador / administrador / propietario) y el propietario puede
   ocultar todo el conjunto cheat con un interruptor; cada acción se audita
   en los logs del bot.
8. **Mayúsculas de cabeceras** — reqwest/hyper envían `authorization` en
   minúsculas; la corrección de cabeceras insensibles a mayúsculas de este
   código es lo que hace funcionar el token.

## 🔒 Seguridad

- 🔐 Enlaza a `127.0.0.1` y pon un bearer token en producción; deja
  `IPWhitelist` para configuraciones multi-host. La API queda sin
  autenticación si no, por diseño.
- 📵 RCON permanece apagado salvo que lo necesites; su contraseña se genera una vez.
- 🚦 Los logins RCON fallidos se limitan por tasa y se bloquean por dirección.

## 🩺 Resolución de problemas

| Síntoma | Causa | Arreglo |
|---|---|---|
| `/api/health` responde `engineReady:false` para siempre | El wrapper del lanzador (también inyectado) tomó primero el puerto REST | Corregido: los listeners solo se enlazan en el proceso donde el motor se inicializó; actualiza el mod |
| Los clientes (reqwest/hyper, bots) reciben `401` con el token correcto; `curl` funciona | Los nombres de cabecera llegan en minúsculas (`authorization`); búsqueda sensible a mayúsculas | Corregido: nombres en minúsculas al parsear (RFC 9110); actualiza el mod |
| `chat commands unavailable` / búsquedas que fallan en algunos arranques | Las clases de game features se registran tras el arranque | Corregido: re-resolución perezosa en el primer uso |
| Heal/teletransporte «funciona» pero el jugador no ve nada | Las escrituras directas en memoria esquivan la replicación de UE (sin dirty mark) | Corregido: funciones nativas en el hilo de juego (`IncreaseHealth`, `K2_TeleportTo`) |
