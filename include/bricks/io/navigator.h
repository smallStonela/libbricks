#pragma once

#include "bricks/object.h"
#include "bricks/io/stream.h"
#include "bricks/io/endian.h"

namespace Bricks { namespace IO {
	class EndOfStreamException : public Exception
	{
	public:
		EndOfStreamException(const String& message = String::Empty) : Exception(message) { }
	};
	
	class StreamException : public Exception
	{
	public:
		StreamException(const String& message = String::Empty) : Exception(message) { }
	};
	
	class StreamNavigator : public Object
	{
	protected:
		AutoPointer<Stream> stream;
		Endian::Enum endianness;

	public:
		StreamNavigator(const Pointer<Stream>& stream, Endian::Enum endianness) : stream(stream), endianness(endianness) { }

		virtual void Pad(u64 size) = 0;
		void PadTo(u64 position) { if (position < stream->GetPosition()) throw InvalidArgumentException("position"); Pad(position - stream->GetPosition()); }
		void PadToMultiple(int round) { PadTo(BRICKS_FEATURE_ROUND_UP(stream->GetPosition(), round)); }

		u64 GetPosition() { return stream->GetPosition(); }
		void SetPosition(u64 position) { stream->SetPosition(position); }
		u64 GetLength() { return stream->GetLength(); }
		bool IsEndOfFile() { return GetPosition() == GetLength(); }
		ReturnPointer<Stream> GetStream() { return stream; }
	};

	class StreamReader : public StreamNavigator
	{
	public:
		StreamReader(const Pointer<Stream>& stream, Endian::Enum endianness = Endian::Native) : StreamNavigator(stream, endianness) { }

#define BRICKS_STREAM_READ(size) \
		u##size ReadInt##size(Endian::Enum endian = Endian::Unknown) { \
			char data[sizeof(u##size)]; \
			if (stream->Read(data, sizeof(u##size)) != sizeof(u##size)) \
				throw EndOfStreamException(); \
			return EndianConvert##size<u##size>(endian ?: endianness, data); \
		}
		BRICKS_STREAM_READ(16);
		BRICKS_STREAM_READ(32);
		BRICKS_STREAM_READ(64);
#undef BRICKS_STREAM_READ
		u8 ReadByte() { u8 data; if (stream->Read(&data, sizeof(data)) != sizeof(data)) throw EndOfStreamException(); return data; }
		void ReadBytes(void* data, size_t size) { if (stream->Read(data, size) != size) throw EndOfStreamException(); }

		String ReadCString(int division) {
			// TODO: StringBuilder, this is fail.
			String ret;
			while (true) {
				int read = stream->ReadByte();
				if (read < 0 || read == division)
					break;
				ret += (char)read;
			}
			return ret;
		}

		String ReadString(int length) {
			char buffer[length];
			ReadBytes(buffer, length);
			return String(buffer, length);
		}

		String ReadString() { return ReadCString('\0'); }
		
		void Pad(u64 size) {
			u8 padding[0x100];
			while (size > 0) {
				size_t sz = BRICKS_FEATURE_MIN(sizeof(padding), size);
				if (stream->Read(padding, sz) != sz)
					throw EndOfStreamException();
				size -= sz;
			}
		}
	};

	class StreamWriter : public StreamNavigator
	{
	public:
		StreamWriter(const Pointer<Stream>& stream, Endian::Enum endianness = Endian::Native) : StreamNavigator(stream, endianness) { }

#define BRICKS_STREAM_WRITE(size) \
		void WriteInt##size(u##size value, Endian::Enum endian = Endian::Unknown) { \
			u##size data; \
			EndianConvert##size(endian ?: endianness, &data, value); \
			if (stream->Write(&data, sizeof(u##size)) != sizeof(u##size)) \
				throw StreamException(); \
		}
		BRICKS_STREAM_WRITE(16);
		BRICKS_STREAM_WRITE(32);
		BRICKS_STREAM_WRITE(64);
#undef BRICKS_STREAM_WRITE
		void WriteByte(u8 data) { if (stream->Write(&data, sizeof(data)) != sizeof(data)) throw StreamException(); }
		void WriteBytes(const void* data, size_t size) { if (stream->Write(data, size) != size) throw StreamException(); }
		void WriteString(const String& str, size_t size = String::npos) {
			if (size == String::npos)
				size = str.GetSize();
			WriteBytes(str.CString(), BRICKS_FEATURE_MIN(str.GetSize(), size));
			if (size > str.GetSize())
				Pad(size - str.GetSize());
		}

		void Pad(u64 size) {
			u8 padding[0x100];
			while (size > 0) {
				size_t sz = BRICKS_FEATURE_MIN(sizeof(padding), size);
				if (stream->Write(padding, sz) != sz)
					throw StreamException();
				size -= sz;
			}
		}

		// TODO: Implement a proper TextWriter with encoding support.
		void WriteLine(const String& string = String::Empty) { WriteString(string); WriteByte('\n'); }
	};
} }
