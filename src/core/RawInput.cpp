#include "RawInput.h"

namespace ofx {
namespace keycaps {
namespace core {

	uint8_t translateModifiers(int m) {
		uint8_t out = ModNone;
		if (m & raw::ModControl) out |= ModCtrl;
		if (m & raw::ModAlt) out |= ModAlt;
		if (m & raw::ModShift) out |= ModShift;
		if (m & raw::ModSuper) out |= ModCmd;
		return out;
	}

	namespace {
		bool modifierKeyFromCode(int code, ModifierKey & m) {
			switch (code) {
			case raw::CodeLeftShift:
			case raw::CodeRightShift: m = ModifierKey::Shift; return true;
			case raw::CodeLeftControl:
			case raw::CodeRightControl: m = ModifierKey::Ctrl; return true;
			case raw::CodeLeftAlt:
			case raw::CodeRightAlt: m = ModifierKey::Alt; return true;
			case raw::CodeLeftSuper:
			case raw::CodeRightSuper: m = ModifierKey::Cmd; return true;
			default: return false;
			}
		}

		// Special keys by keycode, never by key: with Ctrl held, Ctrl+H
		// arrives with key 8 (Backspace's value), Ctrl+I with 9 (Tab),
		// Ctrl+M with 13 (Return) and Ctrl+[ with 27 (Esc).
		bool specialFromCode(int code, SpecialKey & s) {
			switch (code) {
			case raw::CodeEnter:
			case raw::CodeKeypadEnter: s = SpecialKey::Return; return true;
			case raw::CodeTab: s = SpecialKey::Tab; return true;
			case raw::CodeSpace: s = SpecialKey::Space; return true;
			case raw::CodeBackspace: s = SpecialKey::Backspace; return true;
			case raw::CodeDelete: s = SpecialKey::Delete; return true;
			case raw::CodeEscape: s = SpecialKey::Escape; return true;
			case raw::CodeLeft: s = SpecialKey::Left; return true;
			case raw::CodeRight: s = SpecialKey::Right; return true;
			case raw::CodeUp: s = SpecialKey::Up; return true;
			case raw::CodeDown: s = SpecialKey::Down; return true;
			case raw::CodeHome: s = SpecialKey::Home; return true;
			case raw::CodeEnd: s = SpecialKey::End; return true;
			case raw::CodePageUp: s = SpecialKey::PageUp; return true;
			case raw::CodePageDown: s = SpecialKey::PageDown; return true;
			default: break;
			}
			if (code >= raw::CodeF1 && code <= raw::CodeF12) {
				s = static_cast<SpecialKey>(static_cast<int>(SpecialKey::F1) + (code - raw::CodeF1));
				return true;
			}
			return false;
		}

		bool printableCodepoint(int cp) {
			return cp >= 32 && cp != 127
				&& !(cp >= raw::KeyPrivateUseFirst && cp <= raw::KeyPrivateUseLast)
				&& cp <= 0x10FFFF;
		}

		bool printableKeycode(int code) {
			return (code > raw::CodeSpace && code <= raw::CodePrintableLast)
				|| code == raw::CodeWorld1 || code == raw::CodeWorld2;
		}
	} // namespace

	bool translateKey(const RawKey & in, KeyStroke & out) {
		out = KeyStroke();
		out.modifiers = translateModifiers(in.modifiers);
		out.isRepeat = in.isRepeat;

		// The Fn (Globe) key: a lone event with keycode -1 and a stale key.
		if (in.keycode < 0) return false;

		// Modifier keys arrive twice; keep only the left/right event.
		ModifierKey mk;
		if (modifierKeyFromCode(in.keycode, mk)) {
			if (in.key < raw::KeyLeftShift || in.key > raw::KeyRightSuper) return false;
			out.modifierKey = mk;
			return true;
		}
		if (in.keycode == raw::CodeCapsLock) {
			out.modifierKey = ModifierKey::CapsLock; // D-017
			return true;
		}

		SpecialKey sk;
		if (specialFromCode(in.keycode, sk)) {
			out.special = sk;
			return true;
		}
		if (in.keycode == raw::CodeInsert) return false; // no Insert cap

		const bool ctrlOrAlt = (out.modifiers & (ModCtrl | ModAlt)) != 0;
		if (ctrlOrAlt) {
			// Ctrl turns the key into a control code and Option composes a
			// character (å), so the physical key is read from keycode, which
			// for printable keys is the US-layout character (D-015, D-027).
			if (!printableKeycode(in.keycode)) return false;
			int cp = in.keycode;
			if (cp >= 'A' && cp <= 'Z') cp += 'a' - 'A';
			out.codepoint = static_cast<uint32_t>(cp);
			return true;
		}

		// No modifier, Shift, or Cmd: key is the character the layout typed.
		if (!printableCodepoint(in.key)) return false;
		out.codepoint = static_cast<uint32_t>(in.key);
		return true;
	}

	bool translateButton(int ofButton, MouseButton & button, int & otherNumber) {
		otherNumber = 0;
		switch (ofButton) {
		case raw::ButtonLeft: button = MouseButton::Left; return true;
		case raw::ButtonMiddle: button = MouseButton::Middle; return true;
		case raw::ButtonRight: button = MouseButton::Right; return true;
		default: break;
		}
		if (ofButton > raw::ButtonRight && ofButton <= raw::ButtonLast) {
			button = MouseButton::Other;
			otherNumber = ofButton + 1; // oF button 3 is the 4th button
			return true;
		}
		return false;
	}

} // namespace core
} // namespace keycaps
} // namespace ofx
