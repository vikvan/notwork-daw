#pragma once

#include <QString>

namespace notwork::model {

class Project;

class ProjectIO {
public:
    // Reads a .notwork JSON file and applies it to `project`.
    // WAV paths in the file are resolved relative to the project file's dir.
    static bool load(Project& project, const QString& path, QString* error = nullptr);

    // Writes `project` to a .notwork JSON file. WAV filePaths are stored as
    // absolute paths (MVP — no consolidation into a project folder yet).
    static bool save(const Project& project, const QString& path, QString* error = nullptr);
};

} // namespace notwork::model
