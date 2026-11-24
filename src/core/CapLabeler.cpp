#include "CapLabeler.h"

namespace ofx {
namespace keycaps {
namespace core {
	namespace CapLabeler {

		namespace {
			bool symbols(LabelMode mode) { return mode == LabelMode::Symbols; }

			uint32_t upperAscii(uint32_t cp) {
				if (cp >= 'a' && cp <= 'z') return cp - ('a' - 'A');
				return cp;
			}

			std::string specialName(SpecialKey s, LabelMode mode) {
				const bool sym = symbols(mode);
				switch (s) {
				case SpecialKey::Return: return sym ? "\xE2\x86\xA9" : "Return"; // ↩
				case SpecialKey::Tab: return sym ? "\xE2\x87\xA5" : "Tab"; // ⇥
				case SpecialKey::Backspace: return sym ? "\xE2\x8C\xAB" : "Bksp"; // ⌫
				case SpecialKey::Delete: return sym ? "\xE2\x8C\xA6" : "Del"; // ⌦
				case SpecialKey::Escape: return sym ? "\xE2\x8E\x8B" : "Esc"; // ⎋
				case SpecialKey::Left: return sym ? "\xE2\x86\x90" : "Left"; // ←
				case SpecialKey::Right: return sym ? "\xE2\x86\x92" : "Right"; // →
				case SpecialKey::Up: return sym ? "\xE2\x86\x91" : "Up"; // ↑
				case SpecialKey::Down: return sym ? "\xE2\x86\x93" : "Down"; // ↓
				case SpecialKey::Space: return "Space";
				case SpecialKey::Home: return "Home";
				case SpecialKey::End: return "End";
				case SpecialKey::PageUp: return "PgUp";
				case SpecialKey::PageDown: return "PgDn";
				case SpecialKey::None: return "";
				default: break;
				}
				// F1..F12 are contiguous in the enum.
				int n = static_cast<int>(s) - static_cast<int>(SpecialKey::F1) + 1;
				return "F" + std::to_string(n);
			}

			std::string modifierKeyName(ModifierKey m, LabelMode mode) {
				const bool sym = symbols(mode);
				const bool mac = mode == LabelMode::MacText;
				switch (m) {
				case ModifierKey::Shift: return sym ? "\xE2\x87\xA7" : "Shift"; // ⇧
				case ModifierKey::Ctrl: return sym ? "\xE2\x8C\x83" : "Ctrl"; // ⌃
				case ModifierKey::Alt: return sym ? "\xE2\x8C\xA5" : (mac ? "Opt" : "Alt"); // ⌥
				case ModifierKey::Cmd: return sym ? "\xE2\x8C\x98" : (mac ? "Cmd" : "Super"); // ⌘
				case ModifierKey::CapsLock: return sym ? "\xE2\x87\xAA" : "Caps"; // ⇪
				case ModifierKey::None: break;
				}
				return "";
			}
		} // namespace

		void appendUtf8(std::string & out, uint32_t cp) {
			if (cp < 0x80) {
				out += static_cast<char>(cp);
			} else if (cp < 0x800) {
				out += static_cast<char>(0xC0 | (cp >> 6));
				out += static_cast<char>(0x80 | (cp & 0x3F));
			} else if (cp < 0x10000) {
				out += static_cast<char>(0xE0 | (cp >> 12));
				out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
				out += static_cast<char>(0x80 | (cp & 0x3F));
			} else {
				out += static_cast<char>(0xF0 | (cp >> 18));
				out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
				out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
				out += static_cast<char>(0x80 | (cp & 0x3F));
			}
		}

		int codepointCount(const std::string & utf8) {
			int n = 0;
			for (unsigned char c : utf8) {
				if ((c & 0xC0) != 0x80) ++n;
			}
			return n;
		}

		std::string modifierPrefix(uint8_t mods, LabelMode mode) {
			std::string s;
			if (symbols(mode)) {
				if (mods & ModCtrl) s += "\xE2\x8C\x83"; // ⌃
				if (mods & ModAlt) s += "\xE2\x8C\xA5"; // ⌥
				if (mods & ModShift) s += "\xE2\x87\xA7"; // ⇧
				if (mods & ModCmd) s += "\xE2\x8C\x98"; // ⌘
			} else if (mode == LabelMode::MacText) {
				if (mods & ModCtrl) s += "Ctrl+";
				if (mods & ModAlt) s += "Opt+";
				if (mods & ModShift) s += "Shift+";
				if (mods & ModCmd) s += "Cmd+";
			} else {
				if (mods & ModCtrl) s += "Ctrl+";
				if (mods & ModAlt) s += "Alt+";
				if (mods & ModShift) s += "Shift+";
				if (mods & ModCmd) s += "Super+";
			}
			return s;
		}

		std::string keyLabel(const KeyStroke & k, LabelMode mode) {
			if (k.isModifierOnly()) {
				return modifierKeyName(k.modifierKey, mode);
			}
			if (k.isSpecial()) {
				// Shift is shown with special keys: ⇧Tab, ⇧↓.
				return modifierPrefix(k.modifiers, mode) + specialName(k.special, mode);
			}
			std::string s;
			if (k.hasCommandModifier()) {
				// A combo: every modifier shown, letter uppercase (⌘⇧Z, ⌥A).
				s = modifierPrefix(k.modifiers, mode);
				appendUtf8(s, upperAscii(k.codepoint));
			} else {
				// Shift only produced the character: show it as typed (A, !).
				appendUtf8(s, k.codepoint);
			}
			return s;
		}

		std::string typedText(const KeyStroke & k, bool masked, LabelMode mode) {
			if (masked) return maskChar(mode);
			if (k.special == SpecialKey::Space) return " ";
			std::string s;
			appendUtf8(s, k.codepoint);
			return s;
		}

		std::string mouseLabel(MouseButton button, int otherNumber, uint8_t mods,
			bool dragging, LabelMode mode) {
			uint8_t shown = mods;
			std::string name;
			switch (button) {
			case MouseButton::Left:
				name = dragging ? "Drag" : "Click";
				break;
			case MouseButton::Right:
				shown &= static_cast<uint8_t>(~ModCtrl); // D-021
				name = dragging ? "Right-drag" : "Right-click";
				break;
			case MouseButton::Middle:
				shown &= static_cast<uint8_t>(~ModAlt); // D-021
				name = dragging ? "Middle-drag" : "Middle-click";
				break;
			case MouseButton::Other:
				name = "Button " + std::to_string(otherNumber) + (dragging ? " drag" : "");
				break;
			}
			return modifierPrefix(shown, mode) + name;
		}

		std::string scrollLabel(ScrollDir dir, LabelMode mode) {
			const bool sym = symbols(mode);
			switch (dir) {
			case ScrollDir::Up: return sym ? "Scroll \xE2\x86\x91" : "Scroll Up";
			case ScrollDir::Down: return sym ? "Scroll \xE2\x86\x93" : "Scroll Down";
			case ScrollDir::Left: return sym ? "Scroll \xE2\x86\x90" : "Scroll Left";
			case ScrollDir::Right: return sym ? "Scroll \xE2\x86\x92" : "Scroll Right";
			}
			return "Scroll";
		}

		std::string markLabel(LabelMode mode) {
			return symbols(mode) ? "\xE2\x97\x8F MARK" : "MARK"; // ● MARK
		}

		std::string countSuffix(int count, LabelMode mode) {
			if (count < 2) return "";
			return (symbols(mode) ? "\xC3\x97" : "x") + std::to_string(count); // ×N
		}

		std::string maskChar(LabelMode mode) {
			return symbols(mode) ? "\xE2\x80\xA2" : "*"; // •
		}

	} // namespace CapLabeler
} // namespace core
} // namespace keycaps
} // namespace ofx
