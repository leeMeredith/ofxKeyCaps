#pragma once

// Hotkey — a key plus exact modifiers, used for the mark and show/hide
// keys (KEYCAPS.md 7, D-034). Matching is pure, so it is tested headless.

#include "KeyStroke.h"
#include <cstdint>

namespace ofx {
namespace keycaps {
namespace core {

	struct Hotkey {
		uint32_t codepoint = 0; // a letter, digit or punctuation key, lowercase
		SpecialKey special = SpecialKey::None; // or a special key, such as F5
		uint8_t modifiers = ModNone; // must match exactly

		bool isSet() const { return codepoint != 0 || special != SpecialKey::None; }

		static Hotkey none() { return Hotkey(); }
		static Hotkey character(uint32_t cp, uint8_t mods) {
			Hotkey h;
			h.codepoint = lower(cp);
			h.modifiers = mods;
			return h;
		}
		static Hotkey specialKey(SpecialKey s, uint8_t mods = ModNone) {
			Hotkey h;
			h.special = s;
			h.modifiers = mods;
			return h;
		}

		// The key stroke as this hotkey's KeyStroke, for labelling it.
		KeyStroke asStroke() const {
			return special != SpecialKey::None ? KeyStroke::specialKey(special, modifiers)
											   : KeyStroke::character(codepoint, modifiers);
		}

		static uint32_t lower(uint32_t cp) {
			return (cp >= 'A' && cp <= 'Z') ? cp + ('a' - 'A') : cp;
		}
	};

	// True when the stroke is this hotkey: same key (letters compared without
	// case) and exactly the same modifiers. Repeats from holding the key down
	// never match, so a held hotkey fires once.
	inline bool matches(const Hotkey & h, const KeyStroke & k) {
		if (!h.isSet() || k.isRepeat || k.isModifierOnly()) return false;
		if (k.modifiers != h.modifiers) return false;
		if (h.special != SpecialKey::None) return k.special == h.special;
		return k.special == SpecialKey::None && Hotkey::lower(k.codepoint) == h.codepoint;
	}

} // namespace core
} // namespace keycaps
} // namespace ofx
