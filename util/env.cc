

#include "mydb/env.h"

#include <cstdarg>

// This workaround can be removed when mydb::Env::DeleteFile is removed.
// See env.h for justification.
#if defined(_WIN32) && defined(MYDB_DELETEFILE_UNDEFINED)
#undef DeleteFile
#endif

namespace mydb {

Env::Env() = default;

Env::~Env() = default;

Status Env::NewAppendableFile(const std::string& fname, WritableFile** result) {
    return Status::NotSupported("NewAppendableFile", fname);
}

Status Env::RemoveDir(const std::string& dirname) { return DeleteDir(dirname); }
Status Env::DeleteDir(const std::string& dirname) { return RemoveDir(dirname); }

Status Env::RemoveFile(const std::string& fname) { return DeleteFile(fname); }
Status Env::DeleteFile(const std::string& fname) { return RemoveFile(fname); }

SequentialFile::~SequentialFile() = default;

RandomAccessFile::~RandomAccessFile() = default;

WritableFile::~WritableFile() = default;

Logger::~Logger() = default;

FileLock::~FileLock() = default;

void Log(Logger* info_log, const char* format, ...) {
    if (info_log != nullptr) {
        std::va_list ap;
        va_start(ap, format);
        info_log->Logv(format, ap);
        va_end(ap);
    }
}

static Status DoWriteStringToFile(Env* env, const Slice& data,
                                  const std::string& fname, bool should_sync) {
    WritableFile* file;
    Status s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
        return s;
    }
    s = file->Append(data);
    if (s.ok() && should_sync) {
        s = file->Sync();
    }
    if (s.ok()) {
        s = file->Close();
    }
    delete file; // Will auto-close if we did not close above
    if (!s.ok()) {
        env->RemoveFile(fname);
    }
    return s;
}

Status WriteStringToFile(Env* env, const Slice& data,
                         const std::string& fname) {
    return DoWriteStringToFile(env, data, fname, false);
}

Status WriteStringToFileSync(Env* env, const Slice& data,
                             const std::string& fname) {
    return DoWriteStringToFile(env, data, fname, true);
}

Status ReadFileToString(Env* env, const std::string& fname, std::string* data) {
    data->clear();
    SequentialFile* file;
    Status s = env->NewSequentialFile(fname, &file);
    if (!s.ok()) {
        return s;
    }
    static const int kBufferSize = 8192;
    char* space = new char[kBufferSize];
    while (true) {
        Slice fragment;
        s = file->Read(kBufferSize, &fragment, space);
        if (!s.ok()) {
            break;
        }
        data->append(fragment.data(), fragment.size());
        if (fragment.empty()) {
            break;
        }
    }
    delete[] space;
    delete file;
    return s;
}

EnvWrapper::~EnvWrapper() {}

} // namespace mydb
