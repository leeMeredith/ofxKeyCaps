#pragma once

// KeycapRenderer — draws the caps (KEYCAPS.md 6.9). Owns the font and the
// slide animation, nothing else. Draws in window pixels and leaves the
// host's GL state, style, camera and viewport as it found them (P2).

#include "../core/CapHistory.h"
#include "Style.h"
#include "ofTrueTypeFont.h"
#include <cstdint>
#include <map>
#include <string>

namespace ofx {
namespace keycaps {

	class KeycapRenderer {
	public:
		// Loads the font at the given pixel scale. Tries fonts that have the
		// ⌃ ⌥ ⇧ ⌘ symbols first (D-027). Returns false if no font loaded.
		bool setup(const Style & style, float pixelScale);

		bool isLoaded() const { return font_.isLoaded(); }
		float loadedScale() const { return scale_; }
		float loadedFontSize() const { return fontSize_; }

		// True when the loaded font draws every symbol Symbols mode uses.
		bool symbolsAvailable() const { return symbolsOk_; }
		const std::string & fontName() const { return fontName_; }
		const std::string & missingSymbols() const { return missing_; }

		void draw(const core::CapHistory & history, const Style & style, float pixelScale, double now);

	private:
		struct Slot {
			float x = 0.f; // animated left edge, pixels
		};

		bool loadFont(const std::string & name, float pixelSize);
		bool fontHasSymbols(const std::string & name, std::string & missing) const;

		ofTrueTypeFont font_;
		ofTrueTypeFont countFont_;
		float scale_ = 0.f;
		float fontSize_ = 0.f;
		bool symbolsOk_ = false;
		std::string fontName_;
		std::string missing_;
		std::map<uint64_t, Slot> slots_;
		double lastDraw_ = -1.0;
	};

} // namespace keycaps
} // namespace ofx
