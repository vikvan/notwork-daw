#include "model/ProjectIO.h"

#include "model/Project.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace notwork::model {

namespace {

constexpr int kFormatVersion = 1;

QJsonObject eventToJson(const AudioEvent& e) {
    QJsonObject o;
    o["name"]   = e.name;
    o["file"]   = e.filePath;
    o["start"]  = static_cast<qint64>(e.startSamples);
    o["offset"] = static_cast<qint64>(e.offsetSamples);
    o["length"] = static_cast<qint64>(e.lengthSamples);
    return o;
}

AudioEvent eventFromJson(const QJsonObject& o, const QDir& projectDir) {
    AudioEvent e;
    e.name          = o.value("name").toString();
    e.filePath      = o.value("file").toString();
    if (!e.filePath.isEmpty() && QDir::isRelativePath(e.filePath)) {
        e.filePath = QDir::cleanPath(projectDir.absoluteFilePath(e.filePath));
    }
    e.startSamples  = o.value("start").toVariant().toLongLong();
    e.offsetSamples = o.value("offset").toVariant().toLongLong();
    e.lengthSamples = o.value("length").toVariant().toLongLong();
    return e;
}

QJsonObject trackToJson(const Track& t) {
    QJsonObject o;
    o["name"]  = t.name;
    o["color"] = t.color.name();
    o["armed"] = t.armed;
    o["mute"]  = t.mute;
    o["solo"]  = t.solo;
    o["inCh"]  = t.inCh;
    o["outCh"] = t.outCh;
    QJsonArray events;
    for (const auto& e : t.events) events.append(eventToJson(e));
    o["events"] = events;
    return o;
}

Track trackFromJson(const QJsonObject& o, const QDir& projectDir) {
    Track t;
    t.name  = o.value("name").toString();
    t.color = QColor(o.value("color").toString("#3a7bd5"));
    t.armed = o.value("armed").toBool();
    t.mute  = o.value("mute").toBool();
    t.solo  = o.value("solo").toBool();
    t.inCh  = o.value("inCh").toInt();
    t.outCh = o.value("outCh").toInt();
    for (const auto& v : o.value("events").toArray()) {
        t.events.push_back(eventFromJson(v.toObject(), projectDir));
    }
    return t;
}

} // namespace

bool ProjectIO::save(const Project& project, const QString& path, QString* error) {
    QJsonObject root;
    root["version"]    = kFormatVersion;
    root["sampleRate"] = project.sampleRate();

    QJsonArray tracks;
    for (const auto& t : project.tracks()) tracks.append(trackToJson(t));
    root["tracks"] = tracks;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = "cannot open for write: " + f.errorString();
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool ProjectIO::load(Project& project, const QString& path, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = "cannot open: " + f.errorString();
        return false;
    }
    QJsonParseError perr{};
    const auto doc = QJsonDocument::fromJson(f.readAll(), &perr);
    if (doc.isNull() || !doc.isObject()) {
        if (error) *error = "invalid JSON: " + perr.errorString();
        return false;
    }
    const auto root = doc.object();
    const int sr = root.value("sampleRate").toInt(48000);

    const QDir projectDir = QFileInfo(path).absoluteDir();

    std::vector<Track> tracks;
    for (const auto& v : root.value("tracks").toArray()) {
        tracks.push_back(trackFromJson(v.toObject(), projectDir));
    }

    project.loadState(sr, std::move(tracks));
    return true;
}

} // namespace notwork::model
