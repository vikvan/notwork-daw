# Plan: Notwork — минимальный мультитрековый DAW (MVP)

## Context

Greenfield-проект в пустой директории `/Users/vanvik/notwork`. Задача — с нуля собрать
кроссплатформенный (в первую очередь macOS) многодорожечный DAW с записью/воспроизведением,
хранением проектов и таймлайном в духе Studio One/Cubase/Pro Tools. Стек фиксирован пользователем:
CMake, C++20, PortAudio, libsndfile, Qt6. Настройки должны переживать перезапуск.

MVP по решению пользователя включает: запись мультитрек, воспроизведение, drag аудиоивентов
по таймлайну, сохранение/загрузка проекта. Формат аудио — WAV 32-bit float.

Этот план — первый срез. После него — итеративное развитие (waveform-редактирование, plugins,
автоматизация, undo и т.д.), но в этом плане мы явно ограничиваемся MVP.

## Зависимости

Через `brew`:
```
brew install qt portaudio libsndfile pkg-config ninja
```
`cmake` уже установлен. Qt6 из brew кладёт CMake-конфиг в `$(brew --prefix qt)`, что подхватится
через `CMAKE_PREFIX_PATH`. PortAudio и libsndfile ставятся с `.pc`-файлами → используем
`PkgConfig` в CMake.

## Структура проекта

```
notwork/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── cmake/
│   └── FindPortAudioPkg.cmake        # (если нужно, чаще pkg-config достаточно)
└── src/
    ├── main.cpp
    ├── app/
    │   ├── MainWindow.{h,cpp}         # QMainWindow: транспорт + сплиттер
    │   ├── Settings.{h,cpp}           # QSettings-обёртка
    │   └── SettingsDialog.{h,cpp}     # диалог: SR / буфер / устройства
    ├── model/
    │   ├── Project.{h,cpp}            # sample rate, tracks[], путь, dirty-флаг
    │   ├── Track.{h,cpp}              # name, armed, mute, solo, inCh, outCh, events[]
    │   ├── AudioEvent.{h,cpp}         # clip_id, start_samples, offset_samples, length_samples
    │   └── ProjectIO.{h,cpp}          # JSON save/load, layout: <name>.notwork/{project.json, audio/}
    ├── engine/
    │   ├── AudioEngine.{h,cpp}        # PortAudio: open/close/start/stop, callback
    │   ├── DeviceList.{h,cpp}         # enum устройств, каналы, поддерживаемые SR
    │   ├── PlaybackScene.{h,cpp}      # immutable snapshot для RT-потока
    │   ├── AudioClip.{h,cpp}          # WAV, загруженный в RAM (float, деинтерливед)
    │   ├── Recorder.{h,cpp}           # writer-thread: ring→libsndfile
    │   └── RingBuffer.h               # lock-free SPSC (float)
    ├── gui/
    │   ├── TransportBar.{h,cpp}       # ⏮ ⏵ ⏹ ⏺ + счётчик времени
    │   ├── TrackHeader.{h,cpp}        # имя, arm/mute/solo, комбо inCh/outCh
    │   ├── TrackHeaderList.{h,cpp}    # QScrollArea с колонкой заголовков
    │   ├── TimelineScene.{h,cpp}      # QGraphicsScene: линейка + ряды
    │   ├── TimelineView.{h,cpp}       # QGraphicsView с зумом (Ctrl+wheel)
    │   ├── ClipItem.{h,cpp}           # QGraphicsItem: draggable, рисует waveform
    │   ├── Ruler.{h,cpp}              # QGraphicsItem: время сверху
    │   └── Style.{h,cpp}              # dark palette + QSS
    └── util/
        ├── Log.h                      # thin wrapper над qDebug/qWarning
        └── Time.h                     # samples ↔ seconds, snap-хелперы
```

## Ключевые архитектурные решения

### 1. Аудио-движок и реалтайм-контракт

PortAudio-callback работает в RT-потоке. Внутри callback: **никаких** malloc, mutex, log, file I/O.

- Открываем **дуплексный** stream через `Pa_OpenStream` с выбранными input/output устройствами
  и общим `sampleRate` / `framesPerBuffer` из настроек. Формат — `paFloat32`, интерливед.
- Callback:
  1. Для каждого armed-трека берёт свой канал из `input` (индекс `track.inCh`) и пушит фреймы
     в его SPSC `RingBuffer<float>` (writer-thread потом сольёт в WAV).
  2. Микширует playback: читает текущий `PlaybackScene`, для каждой активной ноды-события
     на позиции playhead достаёт сэмплы из `AudioClip` (RAM) и суммирует в соответствующий
     `output`-канал (`track.outCh`). MVP: только пре-загруженные в RAM клипы.
  3. Двигает атомарный `playhead_samples` (relaxed store).
- Транспорт-состояние: `std::atomic<TransportState>{Stopped|Playing|Recording}` +
  `std::atomic<int64_t> playhead_samples`.

### 2. Публикация состояния сцены

Мутации проекта случаются в GUI-потоке относительно редко. Используем **immutable snapshot**:
- `PlaybackScene` — POD с массивом активных клипов (`{clipPtr, trackIdx, inCh, outCh, startSample, offset, length}`).
- Публикация: `std::atomic<std::shared_ptr<const PlaybackScene>>` (C++20 гарантирует lock-free
  на большинстве платформ; если нет — принимаем компромисс: мутации редки, RT-жарит только
  `load()` без блокировок в горячем пути благодаря shared refcount).
- GUI-поток при любом изменении собирает новую сцену и делает `store()`.

Для MVP этого достаточно и просто.

### 3. Запись на диск

- Каждый armed трек имеет свой `RingBuffer<float>` (SPSC, capacity ≈ 4 сек аудио) и
  соответствующий `SndfileWriter` (WAV, 32-bit float, mono).
- Один writer-thread спит на condvar, просыпается каждые ~20 мс, забирает всё доступное из
  колец и пишет через `sf_write_float`. По остановке записи — flush + close, создаётся
  соответствующий `AudioClip` + `AudioEvent` на треке с `startSample = record_start_playhead`.

### 4. Настройки и восстановление

`QSettings` (native `.plist` на macOS) с ключами:
- `audio/sampleRate` (int, default 48000)
- `audio/bufferSize` (int, default 512)
- `audio/inputDeviceName` (string — по имени, не по индексу, чтобы переживало реконнект USB)
- `audio/outputDeviceName` (string)
- `ui/lastProjectPath` (string, опционально)

`Settings` — фасад над `QSettings`; `SettingsDialog` — модальный QDialog с комбобоксами,
заполняемыми из `DeviceList` (через `Pa_GetDeviceInfo`). Применение настроек = переоткрытие
PortAudio stream (Engine.reconfigure).

### 5. Проект на диске

Формат — папка `MySong.notwork/`:
```
MySong.notwork/
├── project.json      # схема ниже
└── audio/
    ├── track1_take1.wav
    └── ...
```

`project.json`:
```json
{
  "version": 1,
  "sampleRate": 48000,
  "tracks": [
    {
      "name": "Vox",
      "inCh": 0, "outCh": 0,
      "armed": false, "mute": false, "solo": false,
      "events": [
        {"file": "audio/track1_take1.wav", "start": 0, "offset": 0, "length": 132300}
      ]
    }
  ]
}
```
`start/offset/length` — в сэмплах при `sampleRate` проекта. Загрузка/сохранение через
`QJsonDocument`; аудио — рядом.

### 6. GUI

- **Тема**: dark palette + скромный QSS (тёмно-серый фон #1e1f22, треки чередующиеся #2a2b2f/#26272b,
  клип #3a7bd5 с рамкой). Без иконок сторонних библиотек — Unicode-символы для транспорта.
- **Верх**: `TransportBar` — кнопки rew/play/stop/rec, счётчик «bar.beat / mm:ss.mss».
- **Центр**: `QSplitter` горизонтально: слева `TrackHeaderList` (фиксированная ширина ~220 px),
  справа `TimelineView` (QGraphicsView + `TimelineScene`).
- **TrackHeader**: имя (редактируется двойным кликом), arm/mute/solo (маленькие цветные кнопки),
  два QComboBox — `In Ch` и `Out Ch` (заполняются из текущего Engine).
- **Timeline**: `QGraphicsScene`: сверху `Ruler` (полоса времени в секундах, deleg. paint),
  под ним ряды-треки (высота ~64 px). Клипы — `ClipItem` (QGraphicsItem), draggable
  по горизонтали (снап опционально), между треками — перетаскивание меняет `trackIdx`.
- **Zoom**: `Ctrl+wheel` — горизонтальное масштабирование. Инвариант: 1 pixel = N samples,
  N меняется от 1 (макс zoom) до 65536.
- **Waveform в ClipItem**: min/max по пикселю столбца из `AudioClip.data()`; кэш пиков
  считать лениво в фоне (в MVP — синхронно, клипы короткие; TODO кэшировать).
- Вертикальная синхронизация: `TrackHeaderList` и `TimelineView` шарят один
  `QScrollBar` (или подключаем `valueChanged` двух вертикальных scroll bars).

### 7. Транспорт-логика

- Play: engine.startPlayback(playhead) → callback начинает миксить.
- Rec: если хотя бы один трек armed → engine.startRecording(); одновременно можно быть в play,
  и armed-треки пишутся с текущей позиции playhead. Классическая DAW-семантика.
- Stop: engine.stop() → закрываем writer'ы, из завершённых WAV собираем `AudioEvent`,
  добавляем в модель, публикуем новый `PlaybackScene`, обновляем UI.

## CMake

Ключевые строчки (единый корневой `CMakeLists.txt`):
```cmake
cmake_minimum_required(VERSION 3.24)
project(notwork LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)
pkg_check_modules(SNDFILE   REQUIRED IMPORTED_TARGET sndfile)

qt_add_executable(notwork MACOSX_BUNDLE ${SOURCES})
target_link_libraries(notwork PRIVATE Qt6::Widgets PkgConfig::PORTAUDIO PkgConfig::SNDFILE)
```
Сборка: `cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=$(brew --prefix qt) && cmake --build build`.

## Что явно **не** входит в MVP

- Undo/redo, копипаст, split/trim клипов, кроссфейды.
- Стриминг воспроизведения с диска (всё в RAM).
- Плагины (VST/AU), MIDI, автоматизация, метроном, отсчёт-in.
- Sidecar peak-files (waveform считается на лету).
- Master-bus, эффекты, sends.

Всё это — следующие итерации после того, как MVP заработает.

## Порядок реализации (по коммитам, каждый — самостоятельно проверяем)

1. **Скелет + CMake + пустое окно**: `main.cpp`, `MainWindow` — просто показать окно с тёмной темой.
2. **DeviceList + SettingsDialog + persist**: список устройств из PortAudio, диалог, QSettings.
3. **AudioEngine (пустой callback) + транспорт-кнопки**: открытие stream, счётчик времени.
4. **Model (Project/Track/AudioEvent) + фиктивные 4 трека**.
5. **TrackHeaderList + TimelineView со статикой**: рёбра треков, линейка времени.
6. **ClipItem drag**: клипы-заглушки, перетаскивание по X и между рядами → обновляет модель.
7. **Запись**: RingBuffer, Recorder, писатель на диск, после stop появляются реальные клипы.
8. **AudioClip загрузка + waveform paint**: min/max пики в ClipItem.
9. **Playback mixing**: callback читает AudioClip'ы из PlaybackScene, суммирует в out-каналы.
10. **ProjectIO**: save/load .notwork.

## Верификация

После каждого шага:
- Build: `cmake --build build` — без ошибок и warnings-as-errors (позже подключим -Wall -Wextra).
- Ручные smoke:
  - Шаг 3: запуск, устройство слышно открывается (в meter'е позже увидим), кнопки работают.
  - Шаг 7: подключить USB-микрофон, arm трек, нажать rec, поговорить, stop → в
    `~/Music/notwork/<untitled>/audio/*.wav` появился файл, открывается в QuickTime.
  - Шаг 9: воспроизведение записанного клипа слышно на выбранном output-канале.
  - Шаг 10: save → перезапуск приложения → open — вся расстановка клипов восстановилась.

Итоговый sanity-check:
1. Открыть Notwork, выбрать audio interface с ≥2 входами.
2. Создать 2 трека, каждому назначить свой input-канал, оба armed.
3. Записать 10 секунд, stop.
4. Драг клипов по таймлайну.
5. Play — оба трека слышно на своих output-каналах.
6. Save, close, open — состояние восстановлено.

Если все шесть шагов проходят — MVP закрыт.
