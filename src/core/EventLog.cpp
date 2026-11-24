#include "EventLog.h"
#include <cmath>
#include <cstdio>

namespace ofxKeyCaps {
namespace core {

	namespace {
		int buttonSlot(const MouseAction & m) {
			switch (m.button) {
			case MouseButton::Left: return 0;
			case MouseButton::Right: return 1;
			case MouseButton::Middle: return 2;
			case MouseButton::Other: break;
			}
			const int n = m.otherNumber;
			return (n >= 4 && n <= 8) ? n - 1 : 8;
		}
	} // namespace

	EventLog::EventLog(const CoreSettings & settings)
		: settings_(settings) { }

	std::string EventLog::formatTime(double seconds) {
		if (!(seconds > 0.0)) seconds = 0.0;
		const long long ms = std::llround(seconds * 1000.0);
		const long long h = ms / 3600000;
		const long long m = (ms / 60000) % 60;
		const long long s = (ms / 1000) % 60;
		const long long f = ms % 1000;
		char buf[32];
		std::snprintf(buf, sizeof buf, "%02lld:%02lld:%02lld.%03lld", h, m, s, f);
		return buf;
	}

	std::string EventLog::stamp(double now, const std::string & label) const {
		return formatTime(now - zero_) + "  " + label;
	}

	std::string EventLog::mark(double now) {
		zero_ = now;
		return stamp(now, CapLabeler::markLabel(settings_.labelMode));
	}

	bool EventLog::log(const KeyStroke & k, double now, std::string & line) {
		const LabelMode mode = settings_.labelMode;
		if (k.isRepeat) return false; // D-014
		if (k.isModifierOnly() && !settings_.showModifierOnly) return false;

		std::string label;
		if (k.isPrintable() && !k.hasCommandModifier()) {
			// The press's own character, masked when masked.
			label = CapLabeler::typedText(k, masked_, mode);
		} else {
			label = CapLabeler::keyLabel(k, mode);
		}
		line = stamp(now, label);
		return true;
	}

	bool EventLog::log(const MouseAction & m, double now, std::string & line) {
		if (!settings_.showMouse) return false;
		const LabelMode mode = settings_.labelMode;
		const int slot = buttonSlot(m);

		switch (m.phase) {
		case MousePhase::Down:
			dragLogged_[slot] = false;
			line = stamp(now, CapLabeler::mouseLabel(m.button, m.otherNumber, m.modifiers, false, mode) + " down");
			return true;

		case MousePhase::Up:
			line = stamp(now, CapLabeler::mouseLabel(m.button, m.otherNumber, m.modifiers, false, mode) + " up");
			return true;

		case MousePhase::Drag:
			if (dragLogged_[slot] || m.dragDistance < settings_.dragThreshold) return false;
			dragLogged_[slot] = true;
			line = stamp(now, CapLabeler::mouseLabel(m.button, m.otherNumber, m.modifiers, true, mode) + " start");
			return true;

		case MousePhase::Scroll: {
			const ScrollStep step = scroll_.feed(m.scrollX, m.scrollY, now, settings_);
			if (!step.becameVisible) return false;
			line = stamp(now, CapLabeler::scrollLabel(step.dir, mode));
			return true;
		}
		}
		return false;
	}

} // namespace core
} // namespace ofxKeyCaps
