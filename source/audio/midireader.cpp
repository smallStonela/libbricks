#include "bricks/audio/midireader.h"
#include "bricks/core/returnpointer.h"
#include "bricks/io/streamreader.h"
#include "bricks/io/substream.h"
#include "bricks/io/stream.h"

using namespace Bricks;
using namespace Bricks::IO;
using namespace Bricks::Audio;

namespace Bricks { namespace Audio {
	static int ReadInteger(Stream* stream)
	{
		int ret = 0;
		u8 num;
		do {
			num = stream->ReadByte();
			ret <<= 7;
			ret |= num & 0x7F;
		} while (num & 0x80);
		return ret;
	}

	static AutoPointer<MidiTimeDivision> ReadDivision(StreamReader* reader)
	{
		u16 division = reader->ReadInt16();
		if (division & 0x8000)
			return autonew MidiFramesPerSecondDivision((division & 0x7F00) >> 8, division & 0xFF);
		return autonew MidiTicksPerBeatDivision(division & 0x7FFF);
	}

	MidiReader::MidiReader(Stream* stream)
	{
		reader = autonew StreamReader(stream, Endian::BigEndian);

		if (reader->ReadInt32() != MagicHeader1)
			BRICKS_FEATURE_THROW(FormatException());
		if (reader->ReadInt32() != MagicHeader2)
			BRICKS_FEATURE_THROW(FormatException());

		midiType = (MidiType::Enum)reader->ReadInt16();
		trackCount = reader->ReadInt16();
		division = ReadDivision(reader);

		trackIndex = 0;
		ReadTrack();

		this->reader = reader;
	}

	MidiReader::~MidiReader()
	{

	}

	bool MidiReader::EndOfFile() const
	{
		return trackIndex >= trackCount;
	}

	bool MidiReader::EndOfTrack() const
	{
		return reader->GetPosition() >= trackPosition + trackSize;
	}

	void MidiReader::ReadTrack()
	{
		if (reader->ReadInt32() != MagicTrackHeader)
			BRICKS_FEATURE_THROW(FormatException());

		trackSize = reader->ReadInt32();

		trackPosition = reader->GetPosition();

		previousIdentifier = 0;
	}

	void MidiReader::NextTrack()
	{
		trackIndex++;

		if (EndOfFile()) {
			trackSize = 0;
			trackPosition = reader->GetPosition();
		} else {
			reader->SetPosition(trackPosition + trackSize);

			ReadTrack();
		}
	}

	void MidiReader::SeekTrack(u32 index)
	{
		if (index >= trackCount)
			BRICKS_FEATURE_THROW(InvalidArgumentException());

		if (index <= trackIndex) {
			reader->SetPosition(sizeof(MagicHeader1) + sizeof(MagicHeader2) + sizeof(u16) * 3);
			trackIndex = 0;
			ReadTrack();
		}
		while (trackIndex < index)
			NextTrack();
	}

	ReturnPointer<Stream> MidiReader::GetTrackStream()
	{
		if (EndOfFile())
			BRICKS_FEATURE_THROW(InvalidOperationException());

		return autonew Substream(reader->GetStream(), trackPosition, trackSize);
	}

	static AutoPointer<MidiMetaEvent> CreateMetaEvent(u32 delta, MidiEventType::Enum type, u32 length, void* data)
	{
		MidiMetaEvent event(delta, type, length, data);
		switch (type) {
			case MidiEventType::Comment:
			case MidiEventType::Copyright:
			case MidiEventType::TrackName:
			case MidiEventType::InstrumentName:
			case MidiEventType::Lyric:
			case MidiEventType::Marker:
			case MidiEventType::CuePoint:
				return autonew MidiTextEvent(event);
			case MidiEventType::Tempo:
				return autonew MidiTempoEvent(event);
			case MidiEventType::TimeSignature:
				return autonew MidiTimeSignatureEvent(event);
			default:
				return autonew MidiMetaEvent(event);
		}
	}

	static AutoPointer<MidiChannelEvent> CreateChannelEvent(u32 delta, MidiEventType::Enum type, u8 channel, u8 parameter1, u8 parameter2)
	{
		MidiChannelEvent event(delta, type, channel, parameter1, parameter2);
		switch (type) {
			case MidiEventType::NoteOn:
			case MidiEventType::NoteOff:
			case MidiEventType::NoteAftertouch:
				return autonew MidiNoteEvent(event);
			default:
				return autonew MidiChannelEvent(event);
		}
	}

	ReturnPointer<MidiEvent> MidiReader::ReadEvent()
	{
		if (EndOfTrack())
			BRICKS_FEATURE_THROW(InvalidOperationException());

		u32 delta = ReadInteger(reader->GetStream());
		u8 identifier = reader->ReadByte();

		switch (identifier) {
			case 0xFF: {
						   MidiEventType::Enum type = (MidiEventType::Enum)reader->ReadByte();
						   u32 length = ReadInteger(reader->GetStream());
						   u8 data[length];
						   reader->ReadBytes(data, length);
						   return CreateMetaEvent(delta, type, length, data); }
			case 0xF0:
					   BRICKS_FEATURE_THROW(NotImplementedException());
			case 0xF7:
					   BRICKS_FEATURE_THROW(NotImplementedException());
			default: {
						 u8 parameter1;
						 if (!(identifier & 0x80)) {
							 parameter1 = identifier;
							 identifier = previousIdentifier;
						 } else {
							 previousIdentifier = identifier;
							 parameter1 = reader->ReadByte();
						 }

						 MidiEventType::Enum type = (MidiEventType::Enum)(identifier >> 4);
						 u8 parameter2 = 0;
						 if (type != 0x0C && type != 0x0D)
							 parameter2 = reader->ReadByte();

						 return CreateChannelEvent(delta, type, identifier & 0x0F, parameter1, parameter2); }
		}
	}
} }
