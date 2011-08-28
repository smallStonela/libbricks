#pragma once

#include "bricks/io/stream.h"
#include "bricks/io/filesystem.h"

namespace Bricks { namespace IO {
	class FileStream : public Object, public Stream
	{
	protected:
		AutoPointer<Filesystem> System;
		FileHandle Handle;

	public:
		FileStream(
			const String& path,
			FileOpenMode::Enum createmode = FileOpenMode::Open,
			FileMode::Enum mode = FileMode::ReadWrite,
			FilePermissions::Enum permissions = FilePermissions::OwnerReadWrite,
			const Pointer<Filesystem>& filesystem = NULL
		) : System(filesystem ?: Filesystem::GetDefault()), Handle(System->Open(path, createmode, mode)) { }
		FileStream(FileHandle handle, const Pointer<Filesystem>& filesystem = NULL) : System(filesystem ?: Pointer<Filesystem>(Filesystem::GetDefault())), Handle(handle) { }
		size_t Read(void* buffer, size_t size) { return System->Read(Handle, buffer, size); }
		size_t Write(const void* buffer, size_t size) { return System->Write(Handle, buffer, size); }
		u64 GetLength() const { return System->FileStat(Handle).GetSize(); }
		void SetLength(u64 length) { System->Truncate(Handle, length); }
		u64 GetPosition() const { return System->Tell(Handle); }
		void SetPosition(u64 position) { Seek(position, SeekType::Beginning); }
		void Seek(s64 offset, SeekType::Enum whence) { System->Seek(Handle, offset, whence); }
		void Flush() { System->Flush(Handle); }
		void Close() { System->Close(Handle); }

		ReturnPointer<Filesystem> GetFilesystem() const { return System; }
		FileHandle GetHandle() const { return Handle; }
	};
} }
