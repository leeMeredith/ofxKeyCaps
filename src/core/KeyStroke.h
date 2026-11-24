#pragma once

// KeyStroke — one key press as the core sees it (KEYCAPS.md 6.1).
// No oF types: OfInputSource translates ofKeyEventArgs into this.

#include <cstdint>

namespace ofxKeyCaps {
namespace core {

	// The addon's own modifier bitmask (D-005). Not oF constants.
	enum Modifier : uint8_t {
		ModNone = 0,
		ModCtrl = 1 << 0,
		ModAlt = 1 << 1, // Option on macOS
		ModShift = 1 << 2,
		ModCmd = 1 << 3, // Super on Linux and Windows
	};

	enum class SpecialKey : uint8_t {
		None,
		Return,
		Tab,
		Space,
		Backspace,
		Delete,
		Escape,
		Left,
		Right,
		Up,
		Down,
		Home,
		End,
		PageUp,
		PageDown,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	};

	// A key that is only a modifier, pressed on its own (R1, D-017).
	enum class ModifierKey : uint8_t {
		None,
		Shift,
		Ctrl,
		Alt,
		Cmd,
		CapsLock,
	};

	struct KeyStroke {
		// A printable character as a Unicode codepoint. Used when
		// special == None and modifierKey == None.
		uint32_t codepoint = 0;
		SpecialKey special = SpecialKey::None;
		ModifierKey modifierKey = ModifierKey::None;
		uint8_t modifiers = ModNone;
		bool isRepeat = false;

		bool isModifierOnly() const { return modifierKey != ModifierKey::None; }
		bool isSpecial() const { return special != SpecialKey::None; }
		bool isPrintable() const {
			return !isModifierOnly() && !isSpecial() && codepoint >= 32 && codepoint != 127;
		}
		// Ctrl, Alt or Cmd held: the press is a combo, never typed text.
		bool hasCommandModifier() const {
			return (modifiers & (ModCtrl | ModAlt | ModCmd)) != 0;
		}

		static KeyStroke character(uint32_t cp, uint8_t mods = ModNone) {
			KeyStroke k;
			k.codepoint = cp;
			k.modifiers = mods;
			return k;
		}
		static KeyStroke specialKey(SpecialKey s, uint8_t mods = ModNone) {
			KeyStroke k;
			k.special = s;
			k.modifiers = mods;
			return k;
		}
		static KeyStroke modifierOnly(ModifierKey m, uint8_t mods = ModNone) {
			KeyStroke k;
			k.modifierKey = m;
			k.modifiers = mods;
			return k;
		}
		KeyStroke repeated() const {
			KeyStroke k = *this;
			k.isRepeat = true;
			return k;
		}
	};

} // namespace core
} // namespace ofxKeyCaps
