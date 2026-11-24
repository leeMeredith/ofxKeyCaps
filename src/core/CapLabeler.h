#pragma once

// CapLabeler — turns KeyStrokes and MouseActions into label text
// (KEYCAPS.md 6.3). Pure functions, no state.

#include "CoreSettings.h"
#include "KeyStroke.h"
#include "MouseAction.h"
#include <cstdint>
#include <string>

namespace ofxKeyCaps {
namespace core {

	namespace CapLabeler {

		// Modifier prefix in ⌃ ⌥ ⇧ ⌘ order, or Ctrl+Alt+Shift+Super+.
		std::string modifierPrefix(uint8_t mods, LabelMode mode);

		// The cap label for one key press, as a single cap shows it:
		// "⌘S", "⌥A", "⇧Tab", "↩", "Space", "F5", "A", "!".
		std::string keyLabel(const KeyStroke & k, LabelMode mode);

		// The text a printable, uncombined key adds to a text cap:
		// the character itself, or the mask bullet when masked.
		std::string typedText(const KeyStroke & k, bool masked, LabelMode mode);

		// "Click", "⇧Right-click", "⌘Drag", "Button 4 drag". A right
		// button never shows ⌃ and a middle button never shows ⌥ (D-021).
		std::string mouseLabel(MouseButton button, int otherNumber, uint8_t mods,
			bool dragging, LabelMode mode);

		// "Scroll ↓" or "Scroll Down".
		std::string scrollLabel(ScrollDir dir, LabelMode mode);

		// "● MARK" or "MARK".
		std::string markLabel(LabelMode mode);

		// "×3" or "x3". Empty for counts below 2.
		std::string countSuffix(int count, LabelMode mode);

		// The bullet used for masked characters.
		std::string maskChar(LabelMode mode);

		// Number of Unicode codepoints in a UTF-8 string.
		int codepointCount(const std::string & utf8);

		// Append one codepoint to a UTF-8 string.
		void appendUtf8(std::string & out, uint32_t cp);

	} // namespace CapLabeler

} // namespace core
} // namespace ofxKeyCaps
