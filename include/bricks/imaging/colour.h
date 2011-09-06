#pragma once

#include "bricks/object.h"
#include "bricks/collections/iterator.h"

namespace Bricks { namespace Imaging {
	class Colour
	{
	public:
		Colour(u8 r = 0, u8 g = 0, u8 b = 0, u8 a = 0xFF) : R(r), G(g), B(b), A(a) { }
		bool operator==(const Colour& rhs) const { return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A; }
		bool operator!=(const Colour& rhs) const { return !operator==(rhs); }

		u8 R;
		u8 G;
		u8 B;
		u8 A;
	};

	typedef Collections::List<Colour> Palette;

	namespace ColourType {
		enum Enum {
			Unknown = 0,
			Intensity,
			Palette,
			Red,
			Green,
			Blue,
			Alpha,
			Count
		};
	}

	namespace ColourTypeMask {
		enum Enum {
			Unknown			= 0x00,
			Intensity		= 0x01,
			Palette			= 0x02,
			Red				= 0x04,
			Green			= 0x08,
			Blue			= 0x10,
			Alpha			= 0x20,

			IntensityAlpha	= Intensity | Alpha,
			RGB				= Red | Green | Blue,
			RGBA			= RGB | Alpha,
		};
	}

	// TODO: Information on pixel component order (like BGRA)
	class PixelDescription
	{
	protected:
		u8 bitDepth[(int)ColourType::Count];
		static const ColourTypeMask::Enum bitDepthMap[];

	public:
		PixelDescription() { memset(bitDepth, 0, sizeof(bitDepth)); }
		PixelDescription(u8 bitDepth, ColourTypeMask::Enum mask) { for (int i = 0; i < ColourType::Count; i++) this->bitDepth[i] = (mask & bitDepthMap[i]) ? bitDepth : 0; }
		ColourTypeMask::Enum GetBitMask() const { int mask = 0; for (int i = 0; i < ColourType::Count; i++) { if (bitDepth[i]) mask |= bitDepthMap[i]; } return (ColourTypeMask::Enum)mask; }
		u8 GetBitDepth(ColourType::Enum type) const { return bitDepth[(int)type]; }
		u8 GetPixelDepth() const { u8 ret = 0; for (int i = 0; i < ColourType::Count; i++) ret += bitDepth[i]; return ret; }
		u8 GetPixelSize() const { return BRICKS_FEATURE_ROUND_UP(GetPixelDepth(), 8) / 8; }

		static const PixelDescription RGB8;
		static const PixelDescription RGBA8;
	};
} }