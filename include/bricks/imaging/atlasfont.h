#include "bricks/imaging/font.h"

namespace Bricks { namespace Imaging {
	class AtlasFont : public Font
	{
	protected:
		Bricks::Collections::Dictionary<String::Character, AutoPointer<FontGlyph> > glyphs;

		ReturnPointer<FontGlyph> LoadGlyph(String::Character character) { if (glyphs.ContainsKey(character)) return glyphs[character]; return NULL; }
		ReturnPointer<Image> RenderGlyph(FontGlyph* glyph) { return NULL; }

		void UpdateSize(FontGlyph* glyph) { if (!width && !height) { width = glyph->GetWidth(); height = glyph->GetHeight(); } }

	public:
		void AddGlyph(FontGlyph* glyph) { glyphs.Add(glyph->GetCharacter(), glyph); UpdateSize(glyph); }
		void AddGlyph(String::Character character, Image* image) { AddGlyph(character, image, image->GetWidth(), 0, image->GetHeight()); }
		void AddGlyph(String::Character character, Image* image, s32 advance, s32 bearingX, s32 bearingY) { AutoPointer<FontGlyph> glyph = autonew FontGlyph(this, character, image, character, advance, image->GetWidth(), image->GetHeight(), bearingX, bearingY); glyphs.Add(character, glyph); UpdateSize(glyph); }
		void RemoveGlyph(String::Character character) { RemoveGlyph(character); }
		void RemoveGlyph(FontGlyph* glyph) { RemoveGlyph(glyph->GetCharacter()); }
	};
} }
