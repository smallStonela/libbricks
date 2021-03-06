#include "bricks/compression/zipfilesystem.h"

#if BRICKS_CONFIG_COMPRESSION_LIBZIP
#include "bricks/io/filestream.h"
#include "bricks/core/math.h"

#include <zip.h>
#include <stdio.h>

using namespace Bricks::IO;

namespace Bricks { namespace Compression {
	struct ZipFilesystemFile {
		struct zip_file* file;
		int index;
		s64 position;

		ZipFilesystemFile(zip_file* file, int index) : file(file), index(index), position(0) { }
	};

	LibZipException::LibZipException(struct zip* zipfile) : Exception(zip_strerror(zipfile))
	{
		zip_error_get(zipfile, &error, &systemError);
	}

	LibZipException::LibZipException(struct zip_file* file) : Exception(zip_file_strerror(file))
	{
		zip_file_error_get(file, &error, &systemError);
	}

	ZipFilesystem::ZipFilesystem(Stream* stream)
	{
		// XXX: Hack around our filesystem classes to get the FD libzip requires.
		int fd;
		FileHandle fsfd;
		Filesystem* filesystem;
#if BRICKS_CONFIG_RTTI
		FileStream* filestream = CastToDynamic<FileStream>(stream);
		if (filestream) {
			filesystem = filestream->GetFilesystem();
			if (IsTypeOf<PosixFilesystem>(filesystem))
				fsfd = (int)(fd = filesystem->Duplicate(filestream->GetHandle()));
			else if (IsTypeOf<C89Filesystem>(filesystem))
				fd = fileno((FILE*)(fsfd = filesystem->Duplicate(filestream->GetHandle())));
			else
				BRICKS_FEATURE_THROW(NotSupportedException());
		} else
#endif
			BRICKS_FEATURE_THROW(NotSupportedException());

		int error;
		zipfile = zip_fdopen(fd, 0, &error);
		if (!zipfile) {
			filesystem->Close(fsfd);
			BRICKS_FEATURE_THROW(LibZipException(error));
		}
	}

	ZipFilesystem::ZipFilesystem(const String& filepath)
	{
		int error;
		zipfile = zip_open(filepath.CString(), 0, &error);
		if (!zipfile)
			BRICKS_FEATURE_THROW(LibZipException(error));
	}

	ZipFilesystem::~ZipFilesystem()
	{
		zip_close(zipfile);
	}

	String ZipFilesystem::TransformPath(const String& path) const
	{
		if (currentDirectory.GetLength())
			return FilePath(currentDirectory) / path;
		return path;
	}

	String ZipFilesystem::TransformPathReverse(const String& path) const
	{
		if (currentDirectory.GetLength() && !path.Compare(currentDirectory + "/", currentDirectory.GetLength()))
			return path.Substring(currentDirectory.GetLength() + 1);
		return path;
	}

	FileHandle ZipFilesystem::Open(
			const String& path,
			FileOpenMode::Enum createmode,
			FileMode::Enum mode,
			FilePermissions::Enum permissions)
	{
		if (createmode & ~FileOpenMode::Open)
			BRICKS_FEATURE_THROW(NotImplementedException());

		int index = zip_name_locate(zipfile, TransformPath(path).CString(), 0);
		if (index < 0)
			BRICKS_FEATURE_THROW(FileNotFoundException(path));
		struct zip_file* file = zip_fopen_index(zipfile, index, 0);
		if (!file)
			BRICKS_FEATURE_THROW(LibZipException(zipfile));
		ZipFilesystemFile* ret = new ZipFilesystemFile(file, index);
		return (FileHandle)ret;
	}
	
	size_t ZipFilesystem::Read(FileHandle fd, void* buffer, size_t size)
	{
		ZipFilesystemFile* file = (ZipFilesystemFile*)fd;
		s64 ret = zip_fread(file->file, buffer, size);
		if (ret < 0)
			BRICKS_FEATURE_THROW(LibZipException(file->file));
		file->position += ret;
		return ret;
	}

	size_t ZipFilesystem::Write(FileHandle fd, const void* buffer, size_t size)
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	u64 ZipFilesystem::Tell(FileHandle fd) const
	{
		return ((ZipFilesystemFile*)fd)->position;
	}

	FileHandle ZipFilesystem::Duplicate(FileHandle fd)
	{
		ZipFilesystemFile* file = (ZipFilesystemFile*)fd;
		if (!file)
			BRICKS_FEATURE_THROW(InvalidArgumentException());

		ZipFilesystemFile* newfile = new ZipFilesystemFile(file->file, file->index);
		return (FileHandle)newfile; // TODO: The file's position needs to be updated with the parent's; it currently is not and this will make things go boom
	}

	void ZipFilesystem::Seek(FileHandle fd, s64 offset, SeekType::Enum whence)
	{
		ZipFilesystemFile* file = (ZipFilesystemFile*)fd;
		switch (whence) {
			case SeekType::Current:
				offset += file->position;
				break;
			case SeekType::End: {
				struct zip_stat st;
				int ret = zip_stat_index(zipfile, file->index, 0, &st);
				if (ret < 0)
					BRICKS_FEATURE_THROW(LibZipException(zipfile));
				offset += st.size;
				break; }
			default:
				break;
		}

		if (offset < file->position) {
			zip_fclose(file->file);
			file->position = 0;
			file->file = zip_fopen_index(zipfile, file->index, 0);
			if (!file->file)
				BRICKS_FEATURE_THROW(LibZipException(zipfile));
		} else
			offset -= file->position;

		while (offset > 0) {
			char buffer[0x100];
			int toread = Math::Min(sizeof(buffer), (u64)offset);
			s64 ret = zip_fread(file->file, buffer, toread);
			if (ret < 0)
				BRICKS_FEATURE_THROW(LibZipException(file->file));
			file->position += ret;
			if (ret < toread)
				break;
			offset -= ret;
		}
	}

	void ZipFilesystem::Flush(FileHandle fd)
	{

	}

	void ZipFilesystem::Truncate(FileHandle fd, u64 length)
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	void ZipFilesystem::Close(FileHandle fd)
	{
		ZipFilesystemFile* file = (ZipFilesystemFile*)fd;
		if (!file)
			BRICKS_FEATURE_THROW(InvalidArgumentException());
		if (file->file)
			zip_fclose(file->file);
		delete file;
	}

	static struct stat StatFromZipStat(const struct zip* zipfile, const struct zip_stat& zst)
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		char lastchar = zst.name[strlen(zst.name) - 1];
		st.st_mode = (lastchar == '/' || lastchar == '\\') ? S_IFDIR : S_IFREG;
		st.st_dev = (dev_t)(size_t)zipfile;
//		if (zst.valid & ZIP_STAT_NAME)
//			st.st_blksize = z;
		if (zst.valid & ZIP_STAT_INDEX)
			st.st_ino = zst.index;
		if (zst.valid & ZIP_STAT_SIZE)
			st.st_size = zst.size;
#if BRICKS_ENV_LINUXBSD
		if (zst.valid & ZIP_STAT_COMP_SIZE) {
			st.st_blocks = 1;
			st.st_blksize = zst.comp_size;
		}
#endif
		if (zst.valid & ZIP_STAT_MTIME)
			st.st_mtime = zst.mtime;
		return st;
	}

	FileInfo ZipFilesystem::Stat(const String& path)
	{
		struct zip_stat zst;
		int ret = zip_stat(zipfile, TransformPath(path).CString(), 0, &zst);
		if (ret < 0) {
			if (path[path.GetLength() - 1] != '/')
				return Stat(path + "/");
			BRICKS_FEATURE_THROW(FileNotFoundException(path));
		}
		return FileInfo(StatFromZipStat(zipfile, zst), TransformPathReverse(zst.name), this);
	}

	FileInfo ZipFilesystem::FileStat(FileHandle fd)
	{
		ZipFilesystemFile* file = (ZipFilesystemFile*)fd;
		struct zip_stat zst;
		int ret = zip_stat_index(zipfile, file->index, 0, &zst);
		if (ret < 0)
			BRICKS_FEATURE_THROW(FileNotFoundException(String::Format("Zip file index %d", file->index)));
		return FileInfo(StatFromZipStat(zipfile, zst), TransformPathReverse(zst.name), this);
	}

	bool ZipFilesystem::IsFile(const String& path) const
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	bool ZipFilesystem::IsDirectory(const String& path) const
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	bool ZipFilesystem::Exists(const String& path) const
	{
		struct zip_stat zst;
		bool exists = zip_stat(zipfile, TransformPath(path).CString(), 0, &zst) >= 0;
		if (!exists && path.GetLength() && path[path.GetLength() - 1] != '/')
			return Exists(path + "/");
		return exists;
	}

	struct ZipFilesystemDir {
		int index;
		int position;

		ZipFilesystemDir(int index) : index(index), position(index + 1) { }
	};

	FileHandle ZipFilesystem::OpenDirectory(const String& path)
	{
		String zippath = TransformPath(path);
		if (zippath[zippath.GetLength() - 1] != '/')
			zippath += "/";
		int index = zip_name_locate(zipfile, zippath.CString(), 0);
		if (index < 0)
			BRICKS_FEATURE_THROW(FileNotFoundException(path));
		const char* dirname = zip_get_name(zipfile, index, 0);
		char lastchar = dirname[strlen(dirname) - 1];
		if (lastchar != '/' && lastchar != '\\')
			BRICKS_FEATURE_THROW(FileNotFoundException(path));
		ZipFilesystemDir* dir = new ZipFilesystemDir(index);
		return (FileHandle)dir;
	}

	ReturnPointer<FileNode> ZipFilesystem::ReadDirectory(FileHandle fd)
	{
		ZipFilesystemDir* dir = (ZipFilesystemDir*)fd;
		struct zip_stat zst;
		const char* dirname = zip_get_name(zipfile, dir->index, 0);
		int len = strlen(dirname);
		while (zip_stat_index(zipfile, dir->position, 0, &zst) >= 0) {
			const char* filename = zip_get_name(zipfile, dir->position, 0);
			dir->position++;
			if (strncmp(dirname, filename, len))
				continue;
			return autonew FilesystemNode(FileInfo(StatFromZipStat(zipfile, zst), zst.name, this));
		}
		return NULL;
	}

	size_t ZipFilesystem::TellDirectory(FileHandle fd)
	{
		ZipFilesystemDir* dir = (ZipFilesystemDir*)fd;
		return dir->position;
	}
	
	void ZipFilesystem::SeekDirectory(FileHandle fd, size_t offset)
	{
		ZipFilesystemDir* dir = (ZipFilesystemDir*)fd;
		dir->position = offset;
	}

	void ZipFilesystem::CloseDirectory(FileHandle fd)
	{
		if (!fd)
			BRICKS_FEATURE_THROW(InvalidArgumentException());
		ZipFilesystemDir* dir = (ZipFilesystemDir*)fd;
		delete dir;
	}

	void ZipFilesystem::DeleteFile(const String& path)
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	void ZipFilesystem::DeleteDirectory(const String& path, bool recursive)
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	String ZipFilesystem::GetCurrentDirectory() const
	{
		return currentDirectory;
	}

	void ZipFilesystem::ChangeCurrentDirectory(const String& path)
	{
		currentDirectory = path;
	}

	void ZipFilesystem::CreateFile(const String& path, FilePermissions::Enum permissions)
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}

	void ZipFilesystem::CreateDirectory(const String& path, FilePermissions::Enum permissions)
	{
		BRICKS_FEATURE_THROW(NotImplementedException());
	}
} }
#endif
