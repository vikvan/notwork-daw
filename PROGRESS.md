# Notwork — прогресс и планы

## Что сделано (MVP)

Кросс-платформенный (macOS-first) многодорожечный DAW, собранный за один заход.
Все пункты плана из `PLAN.md` закрыты, каждый — отдельным коммитом.

| Коммит | Что                                                                    |
|--------|-------------------------------------------------------------------------|
| `b729d85` | Скелет: CMake, C++20, Qt6, PortAudio, libsndfile. Пустое окно с тёмной темой. Preferences (SR / буфер / устройства) с сохранением в QSettings. AudioEngine, транспорт-бар (⏮ ⏵ ⏹ ⏺ + mm:ss.mss). |
| `8afd260` | Модель `Project / Track / AudioEvent`. UI: заголовки треков (имя, arm/mute/solo, In/Out combos), таймлайн со шкалой и чередующимися дорожками, статические клипы. Splitter, синхронный вертикальный скролл. |
| `6be9d1a` | Drag клипов по X и между дорожками. Snap к рядам, clamp ≥ 0. `Project::moveEvent` пишет обратно в модель с queued-уведомлением. |
| `ec56422` | Мультитрековая запись. SPSC ring buffer, writer-поток, WAV 32-bit float mono через libsndfile. `NSMicrophoneUsageDescription` в Info.plist. Вертикальный playhead-линейка. |
| `6ef5a62` | Загрузка WAV в память + waveform (min/max по колонкам) прямо в клипах. Кэш `path → shared_ptr` через `weak_ptr`. |
| `50900e3` | Playback-микшер. Immutable `PlaybackScene`, публикация под коротким мьютексом, RT-callback читает snapshot и суммирует моно-сэмплы в per-track `outCh`. Mute + solo. |
| `f1069b5` | Save/Load проектов. `.notwork` JSON с абсолютными путями к WAV. Меню File: Open (⌘O), Save (⌘S), Save As (⇧⌘S). |

### Ключевые архитектурные решения

- **RT-контракт**: PortAudio-callback никогда не аллоцирует, не блокируется, не пишет в файл.
- **Публикация playback-состояния**: immutable `shared_ptr<const PlaybackScene>`, GUI-поток
  собирает новый snapshot и меняет указатель под коротким мьютексом. Callback берёт
  локальную копию shared_ptr и работает без блокировок.
- **Публикация записи**: per-track SPSC `RingBuffer<float>`. Callback пушит фреймы,
  фоновый writer-поток дренит их в libsndfile.
- **Модель**: `Project` — `QObject` с одним сигналом `changed`. UI и engine слушают.
- **Timeline**: `QGraphicsScene` — линейка и лейны рисуются в `drawBackground`, клипы —
  `QGraphicsItem`ы с pre-computed peaks. Playhead — отдельный `QGraphicsLineItem`.

### Куда что лежит

```
notwork/
├── PLAN.md, PROGRESS.md, Info.plist.in, CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── app/    — MainWindow, Settings, SettingsDialog
│   ├── engine/ — PortAudioSession, DeviceList, AudioEngine, Recorder,
│   │            RingBuffer, AudioClip, PlaybackScene
│   ├── model/  — Project, Track, AudioEvent, ProjectIO
│   └── gui/    — TransportBar, TrackHeader, TrackHeaderList,
│                  TimelineScene, TimelineView, ClipItem, Style
```

## Роадмап следующих итераций

Отсортировал по субъективной пользе на нашу связку. Каждый пункт — самостоятельный слайс,
можно брать в любом порядке.

### 1. Мелкие эргономические фиксы (полдня)
- Undo/Redo — хотя бы на drag клипов, add/remove event, rename track. `QUndoStack`.
- Snap на секунды / доли такта (пока drag свободный).
- Delete/Backspace удаляют выделенный клип.
- Двойной клик по клипу — переименовать.
- Ctrl+scroll — горизонтальный zoom (пока spp фиксировано).

### 2. Проектная папка + консолидация (полдня)
Сейчас `.notwork` — просто JSON с абсолютными путями. Плановая структура —
папка с `audio/` и относительными путями внутри JSON. Save As должен предложить
скопировать/переместить упоминаемые WAV внутрь проекта.

### 3. Работа с клипами (день-два)
- Trim: тянуть левый/правый край клипа (меняет `offsetSamples` / `lengthSamples`).
- Split по playhead.
- Fade in / fade out (простая линейная envelope в callback-миксе).
- Copy / paste.

### 4. Peak-file cache (полдня)
Сейчас peaks считаются синхронно на конструкции ClipItem. Для длинных клипов
это залипает. Считать в фоне и кешировать бинарником рядом с WAV (`.peak`).

### 5. Стриминг воспроизведения с диска (день)
Сейчас все клипы держим в RAM. Для больших сессий сделать read-ahead ring
buffer per-clip, чтение из `libsndfile` в отдельном треде.

### 6. Метроном + count-in (полдня)
BPM/размер в проекте, ruler в bars/beats, генерация щелчков в callback,
count-in при записи.

### 7. Мастер-шина и уровни (день)
- Мастер-фейдер, per-track фейдер.
- Peak/RMS-метры per-track и на мастере (30 Hz UI-таймер, атомарные max/rms
  из callback).
- Solo/mute уже есть в модели — покажем визуально.

### 8. Плагины / эффекты (много)
- CLAP или Audio Unit host — сначала один эффект на трек, потом цепочки/sends.
- Отдельная тема, требует свою итерацию планирования.

### 9. MIDI (много)
- Отдельный тип трека, PortMidi/CoreMIDI. Софт-синтезатор — SFZ/SoundFont.
- Тоже отдельная итерация.

### Технический долг, который стоит подобрать по пути
- Тесты. Сейчас нулевые. `PlaybackScene` mix, `SpscRingBuffer`, `ProjectIO` —
  идеальные кандидаты на unit-тесты (GoogleTest).
- CI — GitHub Actions на macos-latest, build + tests + clang-tidy.
- Warnings-as-errors + clang-tidy профиль.
- Segregate GUI-поток от engine-потока сигналами (сейчас всё Qt::DirectConnection
  на main thread, но пора начать думать про Qt::QueuedConnection для будущего
  offload'а engine на выделенный тред).
- Consistency: некоторые константы (row height, ruler height) продублированы —
  вынести в `gui/TimelineMetrics.h`.
