#pragma once

#include "bricks/core/object.h"
#include "bricks/core/exception.h"
#include "bricks/core/copypointer.h"
#include "bricks/io/filesystem.h"

struct zip;
struct zip_file;

namespace Bricks { namespace IO { class Stream; } }

namespace Bricks { namespace Compression {

#if BRICKS_CONFIG_COMPRESSION_LIBZIP
	class LibZipException : public Exception
	{
	protected:
		int error;
		int systemError;

	public:
		LibZipException(int error, const String& message = String::Empty) : Exception(message), error(error), systemError(0) { }
		LibZipException(struct zip* zipfile);
		LibZipException(struct zip_file* file);

		int GetErrorCode() const { return error; }
		int GetSystemErrorCode() const { return systemError; }
	};

	class ZipFilesystem : public IO::Filesystem, public NoCopy
	{
	protected:
		struct zip* zipfile;
		String currentDirectory;

		String TransformPath(const String& path) const;
		String TransformPathReverse(const String& path) const;

	public:
		ZipFilesystem(IO::Stream* stream);
		ZipFilesystem(const String& filepath);
		~ZipFilesystem();

		IO::FileHandle Open(
			const String& path,
			IO::FileOpenMode::Enum createmode = IO::FileOpenMode::Create,
			IO::FileMode::Enum mode = IO::FileMode::ReadWrite,
			IO::FilePermissions::Enum permissions = IO::FilePermissions::OwnerReadWrite
		);
		size_t Read(IO::FileHandle fd, void* buffer, size_t size);
		size_t Write(IO::FileHandle fd, const void* buffer, size_t size);
		u64 Tell(IO::FileHandle fd) const;
		void Seek(IO::FileHandle fd, s64 offset, IO::SeekType::Enum whence);
		void Flush(IO::FileHandle fd);
		void Truncate(IO::FileHandle fd, u64 length);
		void Close(IO::FileHandle fd);
		IO::FileHandle Duplicate(IO::FileHandle fd);

		IO::FileHandle OpenDirectory(const String& path);
		ReturnPointer<IO::FileNode> ReadDirectory(IO::FileHandle fd);
		size_t TellDirectory(IO::FileHandle fd);
		void SeekDirectory(IO::FileHandle fd, size_t offset);
		void CloseDirectory(IO::FileHandle fd);
		
		IO::FileInfo Stat(const String& path);
		IO::FileInfo FileStat(IO::FileHandle fd);

		bool IsFile(const String& path) const;
		bool IsDirectory(const String& path) const;
		bool Exists(const String& path) const;

		void DeleteFile(const String& path);
		void DeleteDirectory(const String& path, bool recursive);

		String GetCurrentDirectory() const;
		void ChangeCurrentDirectory(const String& path);

		void CreateFile(const String& path, IO::FilePermissions::Enum permissions);
		void CreateDirectory(const String& path, IO::FilePermissions::Enum permissions);
	};
#endif

} }
