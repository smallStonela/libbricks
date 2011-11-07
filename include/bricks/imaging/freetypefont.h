#pragma once

#include "bricks/config.h"

#ifdef BRICKS_CONFIG_IMAGING_FREETYPE

struct FT_LibraryRec_;
struct FT_FaceRec_;
struct FT_StreamRec_;

#include "bricks/imaging/font.h"
#include "bricks/io/stream.h"

namespace Bricks { namespace Imaging {
	class FreeTypeFont : public Font
	{
	protected:
		AutoPointer<IO::Stream> stream;
		FT_LibraryRec_* library;
		FT_FaceRec_* face;
		FT_StreamRec_* ftstream;

		ReturnPointer<FontGlyph> LoadGlyph(String::Character character);
		ReturnPointer<Image> RenderGlyph(const Pointer<FontGlyph>& glyph);

	public:
		FreeTypeFont(const Pointer<IO::Stream>& stream, int faceIndex = 0);
		~FreeTypeFont();

		void SetPixelSize(int pixelWidth, int pixelHeight = 0);
		void SetPointSize(int pointWidth, int pointHeight = 0, int dpiWidth = 0, int dpiHeight = 0);

		u32 GetHeight() const;
		s32 GetBaseline() const;
		s32 GetDescender() const;

		s32 GetKerning(const Pointer<FontGlyph>& glyph, const Pointer<FontGlyph>& previous);
	};
} }

#endif
