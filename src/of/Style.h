#pragma once

// Style — one plain struct, all fields with defaults (KEYCAPS.md 8).
// Sizes are logical points; the renderer multiplies them by the window's
// pixel scale. `core` holds the settings the core uses (D-023).

#include "../core/CoreSettings.h"
#include "../core/Hotkey.h"
#include "ofColor.h"
#include "ofConstants.h"
#include <glm/vec2.hpp>

namespace ofx {
namespace keycaps {

	struct Style {
		enum class Anchor {
			BottomCenter,
			BottomLeft,
			BottomRight,
			Custom, // customPosition is the bottom center of the row
		};

		// Position.
		Anchor anchor = Anchor::BottomCenter;
		glm::vec2 customPosition { 0.f, 0.f };
		float marginBottom = 40.f;
		float marginSide = 40.f; // used by BottomLeft and BottomRight

		// Size.
		float capHeight = 44.f;
		float cornerRadius = 8.f;
		float capSpacing = 8.f;
		float fontSize = 20.f;

		// Color.
		ofColor capColor { 40, 40, 40, 217 };
		ofColor pressedColor { 110, 110, 110, 217 };
		ofColor markColor { 200, 35, 35, 217 };
		ofColor outlineColor { 0, 0, 0, 0 }; // alpha 0: no outline
		ofColor textColor { 255, 255, 255, 255 };

		// Timing.
		double slideTime = 0.15;

		// Log.
		bool log = true;

		// Keys (KEYCAPS.md 7, D-034). Never shown as caps or logged as keys.
		// Change them, or set them to core::Hotkey::none() to turn them off.
		// The defaults avoid ⌃⌥, which is VoiceOver's own modifier.
		core::Hotkey markKey = core::Hotkey::character('m', core::ModCtrl | core::ModShift); // ⌃⇧M
		core::Hotkey toggleKey = core::Hotkey::character('k', core::ModCtrl | core::ModShift); // ⌃⇧K

		// Timing, limits, thresholds and label mode for the core.
		core::CoreSettings core;

		Style() {
#ifdef TARGET_OSX
			core.labelMode = core::LabelMode::Symbols;
#else
			core.labelMode = core::LabelMode::Text;
#endif
		}
	};

} // namespace keycaps
} // namespace ofx
