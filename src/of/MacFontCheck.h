#pragma once

// MacFontCheck — asks macOS (CoreText) whether a font really has a set of
// characters (D-032). Replaces the outline comparison of D-028, which gave
// false answers on macOS. No openFrameworks types, so it can be compiled
// and checked on its own.

#include <cstdint>
#include <string>
#include <vector>

namespace ofx {
namespace keycaps {

	struct FontCheck {
		bool checked = false; // false where CoreText is not available
		bool found = false; // macOS resolved the name to that font, not a substitute
		std::string resolvedName; // the font macOS actually returned
		std::vector<uint32_t> missing; // codepoints the font has no glyph for
	};

	// On macOS: looks the font up by name and checks every codepoint.
	// Elsewhere: returns checked == false.
	FontCheck checkFontCoverage(const std::string & fontName, const std::vector<uint32_t> & codepoints);

} // namespace keycaps
} // namespace ofx
