# Notwork

Кросс-платформенный (macOS-first) многодорожечный DAW на C++20 / Qt6 / PortAudio / libsndfile.

MVP уровня «записать несколько дорожек с микрофона, разложить клипы по таймлайну,
свести в реальном времени и сохранить проект в файл». Все архитектурные решения и
подробный список того, что сделано и что впереди — в [PROGRESS.md](PROGRESS.md).

## Возможности

- Транспорт: play / stop / record, счётчик `mm:ss.mss`, вертикальный playhead.
- Мультитрековая запись через PortAudio, WAV 32-bit float mono через libsndfile.
- Таймлайн на `QGraphicsScene`: заголовки треков, шкала, чередующиеся дорожки,
  клипы с pre-computed waveform (min/max peaks).
- Drag клипов по X и между дорожками со snap к рядам.
- Playback-микшер: immutable `PlaybackScene`, RT-callback без аллокаций и блокировок,
  per-track mute / solo.
- Save / Load проектов в `.notwork` (JSON): File → Open (⌘O), Save (⌘S), Save As (⇧⌘S).
- Preferences: sample rate, размер буфера, устройства ввода/вывода, всё в `QSettings`.

## Скриншоты

_TBD._

## Сборка (macOS)

Зависимости через Homebrew:

```bash
brew install cmake qt@6 portaudio libsndfile pkg-config
```

Сборка:

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build build -j
open build/notwork.app
```

При первом запуске macOS спросит доступ к микрофону — разрешение прописано в
`Info.plist.in` (`NSMicrophoneUsageDescription`).

## Проект

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

## Архитектурные принципы

- **RT-контракт**: PortAudio-callback никогда не аллоцирует, не блокируется и не пишет в файл.
- **Публикация playback-состояния**: immutable `shared_ptr<const PlaybackScene>`,
  GUI собирает новый snapshot, меняет указатель под коротким мьютексом. Callback
  берёт локальную копию shared_ptr и миксует без блокировок.
- **Публикация записи**: per-track SPSC `RingBuffer<float>`. Callback пушит фреймы,
  фоновый writer-поток дренит их в libsndfile.
- **Модель** — `Project` как `QObject` с одним сигналом `changed`. UI и engine слушают.

## Роадмап

Следующие итерации (undo/redo, trim/split клипов, метроном, мастер-шина,
peak-file cache, стриминг с диска, плагины, MIDI) — расписаны в
[PROGRESS.md](PROGRESS.md).

## Лицензия

MIT — см. [LICENSE](LICENSE).
