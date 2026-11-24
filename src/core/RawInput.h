#pragma once

// RawInput — translates the raw numbers in oF key and mouse events into
// KeyStrokes and MouseActions (KEYCAPS.md 6.8, D-015, D-020, D-027).
//
// It lives in the core, with oF's values copied in as plain numbers, so the
// rules can be tested headless against the recorded probe events.
// OfInputSource.cpp static_asserts that every copied value still matches
// openFrameworks.

#include "KeyStroke.h"
#include "MouseAction.h"

namespace ofx {
namespace keycaps {
namespace core {

	namespace raw {
		// ofKeyEventArgs::modifiers / ofMouseEventArgs::modifiers bits.
		constexpr int ModShift = 0x1; // OF_KEY_SHIFT
		constexpr int ModControl = 0x2; // OF_KEY_CONTROL
		constexpr int ModAlt = 0x4; // OF_KEY_ALT
		constexpr int ModSuper = 0x10; // OF_KEY_SUPER

		// ofKeyEventArgs::key values.
		constexpr int KeyLeftShift = 0xe60; // OF_KEY_LEFT_SHIFT .. OF_KEY_RIGHT_SUPER = 0xe67
		constexpr int KeyRightSuper = 0xe67;
		constexpr int KeyPrivateUseFirst = 0xe000; // OF_KEY_F1 and up use the private use area
		constexpr int KeyPrivateUseLast = 0xf8ff;

		// ofKeyEventArgs::keycode values (GLFW key tokens).
		constexpr int CodeSpace = 32;
		constexpr int CodePrintableLast = 96; // GLFW_KEY_GRAVE_ACCENT
		constexpr int CodeWorld1 = 161;
		constexpr int CodeWorld2 = 162;
		constexpr int CodeEscape = 256;
		constexpr int CodeEnter = 257;
		constexpr int CodeTab = 258;
		constexpr int CodeBackspace = 259;
		constexpr int CodeInsert = 260;
		constexpr int CodeDelete = 261;
		constexpr int CodeRight = 262;
		constexpr int CodeLeft = 263;
		constexpr int CodeDown = 264;
		constexpr int CodeUp = 265;
		constexpr int CodePageUp = 266;
		constexpr int CodePageDown = 267;
		constexpr int CodeHome = 268;
		constexpr int CodeEnd = 269;
		constexpr int CodeCapsLock = 280;
		constexpr int CodeF1 = 290;
		constexpr int CodeF12 = 301;
		constexpr int CodeKeypadEnter = 335;
		constexpr int CodeLeftShift = 340;
		constexpr int CodeLeftControl = 341;
		constexpr int CodeLeftAlt = 342;
		constexpr int CodeLeftSuper = 343;
		constexpr int CodeRightShift = 344;
		constexpr int CodeRightControl = 345;
		constexpr int CodeRightAlt = 346;
		constexpr int CodeRightSuper = 347;

		// ofMouseEventArgs::button values.
		constexpr int ButtonLeft = 0; // OF_MOUSE_BUTTON_LEFT
		constexpr int ButtonMiddle = 1; // OF_MOUSE_BUTTON_MIDDLE
		constexpr int ButtonRight = 2; // OF_MOUSE_BUTTON_RIGHT
		constexpr int ButtonLast = 7; // OF_MOUSE_BUTTON_8
	} // namespace raw

	// The fields of one ofKeyEventArgs that the translation reads.
	struct RawKey {
		int key = 0;
		int keycode = 0;
		int modifiers = 0;
		bool isRepeat = false;
	};

	// The oF modifier bits as the core's Modifier bits.
	uint8_t translateModifiers(int ofModifiers);

	// A key press as a KeyStroke. Returns false when the event is dropped:
	// the duplicate flag event of a modifier press, keycode -1 (the Fn key),
	// Insert, and anything with no printable character.
	bool translateKey(const RawKey & in, KeyStroke & out);

	// An oF mouse button number as the core's button. Returns false for
	// numbers outside 0..7.
	bool translateButton(int ofButton, MouseButton & button, int & otherNumber);

} // namespace core
} // namespace keycaps
} // namespace ofx
