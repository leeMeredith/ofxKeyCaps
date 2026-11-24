#pragma once

// CapHistory — the caps currently visible, and the rules that make,
// merge, count, hold and expire them (KEYCAPS.md 6.4 and 6.5).
// Time comes in from outside as `now` in seconds (P4, D-024).

#include "CapLabeler.h"
#include "CoreSettings.h"
#include "KeyStroke.h"
#include "MouseAction.h"
#include "ScrollGesture.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ofx {
namespace keycaps {
namespace core {

	enum class CapKind {
		Text, // a run of typed characters
		Single, // a combo or a special key
		Mouse, // a mouse button
		Scroll, // a scroll gesture
		Mark, // a sync mark
	};

	struct Cap {
		uint64_t id = 0; // stable for the cap's life, for the renderer's animation
		CapKind kind = CapKind::Single;
		std::string label; // without the count; for Text caps, the typed text
		int count = 1;
		double changedAt = 0.0; // when the cap last changed (R10)
		bool held = false; // a button is down or a gesture is running (M2)
		float age = 0.f; // seconds since changedAt; 0 while held
		float opacity = 1.f; // 0..1

		// Mouse caps.
		MouseButton button = MouseButton::Left;
		int otherNumber = 0;
		uint8_t modifiers = ModNone;
		bool dragged = false;
		double releasedAt = 0.0;

		// Scroll caps.
		ScrollDir dir = ScrollDir::Down;
		int gestureId = 0;

		bool isMark() const { return kind == CapKind::Mark; }
	};

	class CapHistory {
	public:
		explicit CapHistory(const CoreSettings & settings = CoreSettings());

		void setSettings(const CoreSettings & settings);
		const CoreSettings & settings() const { return settings_; }

		void setMasked(bool masked) { masked_ = masked; }
		bool masked() const { return masked_; }

		void push(const KeyStroke & k, double now);
		void push(const MouseAction & m, double now);
		void pushMark(double now);

		// Ages caps, ends scroll gestures after scrollGap, removes caps whose
		// fade has finished.
		void update(double now);

		void clear();

		// Oldest first.
		const std::vector<Cap> & caps() const { return caps_; }

		// The full text drawn on a cap: label plus count ("⌘Z ×3").
		std::string displayText(const Cap & c) const;

	private:
		Cap & addCap(CapKind kind, const std::string & label, double now, bool held = false);
		Cap * newest();
		Cap * heldMouseCap(MouseButton b, int otherNumber);
		Cap * scrollCap(int gestureId);
		void releaseCap(Cap & c, double at);
		void enforceCapacity();
		void pushScroll(const MouseAction & m, double now);

		CoreSettings settings_;
		bool masked_ = false;
		std::vector<Cap> caps_;
		uint64_t nextId_ = 1;
		ScrollGesture scroll_;
	};

} // namespace core
} // namespace keycaps
} // namespace ofx
