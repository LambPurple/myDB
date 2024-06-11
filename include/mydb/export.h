

#ifndef STORAGE_MYDB_INCLUDE_EXPORT_H_
#define STORAGE_MYDB_INCLUDE_EXPORT_H_

#if !defined(MYDB_EXPORT)

#if defined(MYDB_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(MYDB_COMPILE_LIBRARY)
#define MYDB_EXPORT __declspec(dllexport)
#else
#define MYDB_EXPORT __declspec(dllimport)
#endif // defined(MYDB_COMPILE_LIBRARY)

#else // defined(_WIN32)
#if defined(MYDB_COMPILE_LIBRARY)
#define MYDB_EXPORT __attribute__((visibility("default")))
#else
#define MYDB_EXPORT
#endif
#endif // defined(_WIN32)

#else // defined(MYDB_SHARED_LIBRARY)
#define MYDB_EXPORT
#endif

#endif // !defined(MYDB_EXPORT)

#endif // STORAGE_MYDB_INCLUDE_EXPORT_H_
