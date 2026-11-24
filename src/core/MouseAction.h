#pragma once

// MouseAction — one mouse button or scroll event as the core sees it
// (KEYCAPS.md 6.2). No oF types and no position.

#include "KeyStroke.h"
#include <cstdint>

namespace ofxKeyCaps {
namespace core {

	enum class MouseButton : uint8_t {
		Left,
		Right,
		Middle,
		Other, // buttons 4 to 8, numbered by otherNumber
	};

	enum class MousePhase : uint8_t {
		Down,
		Up,
		Drag,
		Scroll,
	};

	// Direction of a scroll gesture as a viewer reads it (D-018, D-022).
	enum class ScrollDir : uint8_t {
		Up,
		Down,
		Left,
		Right,
	};

	struct MouseAction {
		MouseButton button = MouseButton::Left;
		int otherNumber = 0; // 4..8 when button == Other
		MousePhase phase = MousePhase::Down;
		uint8_t modifiers = ModNone;
		float scrollX = 0.f; // Scroll only
		float scrollY = 0.f; // Scroll only
		float dragDistance = 0.f; // Drag only, logical points from the press point

		static MouseAction down(MouseButton b, uint8_t mods = ModNone) {
			MouseAction m;
			m.button = b;
			m.phase = MousePhase::Down;
			m.modifiers = mods;
			return m;
		}
		static MouseAction up(MouseButton b, uint8_t mods = ModNone) {
			MouseAction m = down(b, mods);
			m.phase = MousePhase::Up;
			return m;
		}
		static MouseAction drag(MouseButton b, float distance, uint8_t mods = ModNone) {
			MouseAction m = down(b, mods);
			m.phase = MousePhase::Drag;
			m.dragDistance = distance;
			return m;
		}
		static MouseAction scroll(float x, float y, uint8_t mods = ModNone) {
			MouseAction m;
			m.phase = MousePhase::Scroll;
			m.scrollX = x;
			m.scrollY = y;
			m.modifiers = mods;
			return m;
		}
		static MouseAction otherButton(int number, MousePhase phase, uint8_t mods = ModNone) {
			MouseAction m;
			m.button = MouseButton::Other;
			m.otherNumber = number;
			m.phase = phase;
			m.modifiers = mods;
			return m;
		}
	};

} // namespace core
} // namespace ofxKeyCaps
