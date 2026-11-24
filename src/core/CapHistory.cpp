#include "CapHistory.h"
#include <algorithm>

namespace ofx {
namespace keycaps {
namespace core {

	CapHistory::CapHistory(const CoreSettings & settings)
		: settings_(settings) { }

	void CapHistory::setSettings(const CoreSettings & settings) {
		settings_ = settings;
		enforceCapacity();
	}

	void CapHistory::clear() {
		caps_.clear();
		scroll_.reset();
	}

	std::string CapHistory::displayText(const Cap & c) const {
		const std::string suffix = CapLabeler::countSuffix(c.count, settings_.labelMode);
		return suffix.empty() ? c.label : c.label + " " + suffix;
	}

	// The new cap is marked held before R9 runs, so capacity never removes
	// a cap that is about to be held. Returns the new cap.
	Cap & CapHistory::addCap(CapKind kind, const std::string & label, double now, bool held) {
		Cap c;
		c.id = nextId_++;
		c.kind = kind;
		c.label = label;
		c.changedAt = now;
		c.held = held;
		caps_.push_back(c);
		const uint64_t id = c.id;
		enforceCapacity();
		for (Cap & x : caps_) {
			if (x.id == id) return x;
		}
		return caps_.back(); // removed by R9 (every other cap held): harmless
	}

	Cap * CapHistory::newest() {
		return caps_.empty() ? nullptr : &caps_.back();
	}

	Cap * CapHistory::heldMouseCap(MouseButton b, int otherNumber) {
		for (auto it = caps_.rbegin(); it != caps_.rend(); ++it) {
			if (it->kind == CapKind::Mouse && it->held && it->button == b
				&& (b != MouseButton::Other || it->otherNumber == otherNumber)) {
				return &*it;
			}
		}
		return nullptr;
	}

	Cap * CapHistory::scrollCap(int gestureId) {
		for (auto it = caps_.rbegin(); it != caps_.rend(); ++it) {
			if (it->kind == CapKind::Scroll && it->gestureId == gestureId) return &*it;
		}
		return nullptr;
	}

	void CapHistory::releaseCap(Cap & c, double at) {
		c.held = false;
		c.releasedAt = at;
		c.changedAt = at; // M3: age starts from the release
	}

	// R9: drop the oldest cap that is not held; if all are held, the oldest.
	void CapHistory::enforceCapacity() {
		const size_t maxCaps = static_cast<size_t>(std::max(1, settings_.maxCaps));
		while (caps_.size() > maxCaps) {
			auto victim = std::find_if(caps_.begin(), caps_.end(),
				[](const Cap & c) { return !c.held; });
			if (victim == caps_.end()) victim = caps_.begin();
			caps_.erase(victim);
		}
	}

	//--------------------------------------------------------------
	// Keys (R1–R8)
	//--------------------------------------------------------------
	void CapHistory::push(const KeyStroke & k, double now) {
		update(now);
		const LabelMode mode = settings_.labelMode;

		// R1: modifier-only presses.
		if (k.isModifierOnly()) {
			if (!settings_.showModifierOnly) return;
			if (k.isRepeat) return;
			addCap(CapKind::Single, CapLabeler::keyLabel(k, mode), now);
			return;
		}

		// R6: held keys count on the newest cap, whatever its kind.
		if (k.isRepeat) {
			if (Cap * n = newest()) {
				n->count += 1;
				n->changedAt = now;
				return;
			}
			// Nothing to count on: treat it as a fresh press.
		}

		const bool typed = !k.hasCommandModifier()
			&& (k.isPrintable() || k.special == SpecialKey::Space);

		if (typed) {
			Cap * n = newest();
			const bool canMerge = n && n->kind == CapKind::Text
				&& (now - n->changedAt) < settings_.mergeWindow
				&& CapLabeler::codepointCount(n->label) < settings_.maxTextChars;

			if (canMerge) {
				// R3, R4: append to the text run (Space as a space).
				n->label += CapLabeler::typedText(k, masked_, mode);
				n->changedAt = now;
				return;
			}
			if (k.special != SpecialKey::Space) {
				// R3, R8: start a new text run.
				addCap(CapKind::Text, CapLabeler::typedText(k, masked_, mode), now);
				return;
			}
			// R4: Space that would start a cap is a single cap "Space".
		}

		// Combos and special keys (R5, R7).
		const std::string label = CapLabeler::keyLabel(k, mode);
		Cap * n = newest();
		if (n && n->kind == CapKind::Single && n->label == label
			&& (now - n->changedAt) < settings_.repeatWindow) {
			n->count += 1;
			n->changedAt = now;
			return;
		}
		addCap(CapKind::Single, label, now);
	}

	//--------------------------------------------------------------
	// Mouse (M0–M8)
	//--------------------------------------------------------------
	void CapHistory::push(const MouseAction & m, double now) {
		if (!settings_.showMouse) return; // M0
		update(now);
		const LabelMode mode = settings_.labelMode;

		switch (m.phase) {
		case MousePhase::Scroll:
			pushScroll(m, now);
			return;

		case MousePhase::Down: {
			// M7: a missed release is released first.
			if (Cap * stale = heldMouseCap(m.button, m.otherNumber)) {
				releaseCap(*stale, now);
			}
			const std::string label = CapLabeler::mouseLabel(m.button, m.otherNumber,
				m.modifiers, false, mode);
			Cap * n = newest();
			if (n && n->kind == CapKind::Mouse && !n->held && !n->dragged
				&& n->label == label && (now - n->releasedAt) < settings_.repeatWindow) {
				// M5: repeated clicks count.
				n->count += 1;
				n->held = true;
				n->changedAt = now;
				return;
			}
			Cap & c = addCap(CapKind::Mouse, label, now, true); // M1
			c.button = m.button;
			c.otherNumber = m.otherNumber;
			c.modifiers = m.modifiers;
			return;
		}

		case MousePhase::Drag: {
			// M4: relabel once, at or past the threshold.
			Cap * c = heldMouseCap(m.button, m.otherNumber);
			if (c && !c->dragged && m.dragDistance >= settings_.dragThreshold) {
				c->dragged = true;
				c->label = CapLabeler::mouseLabel(c->button, c->otherNumber,
					c->modifiers, true, mode);
				c->changedAt = now;
			}
			return;
		}

		case MousePhase::Up: {
			if (Cap * c = heldMouseCap(m.button, m.otherNumber)) {
				releaseCap(*c, now); // M3
			}
			return;
		}
		}
	}

	void CapHistory::pushScroll(const MouseAction & m, double now) {
		const LabelMode mode = settings_.labelMode;
		const int previousId = scroll_.gestureId();
		const ScrollStep step = scroll_.feed(m.scrollX, m.scrollY, now, settings_);

		if (step.endedPrevious) {
			if (Cap * prev = scrollCap(previousId)) {
				if (prev->held) releaseCap(*prev, step.previousEndTime);
			}
		}

		if (step.becameVisible) {
			Cap * n = newest();
			if (n && n->kind == CapKind::Scroll && !n->held && n->dir == step.dir
				&& (now - n->releasedAt) < settings_.repeatWindow) {
				// Another swipe the same way: count it (Scroll ↓ ×3).
				n->count += 1;
				n->held = true;
				n->gestureId = step.gestureId;
				n->changedAt = now;
				return;
			}
			Cap & c = addCap(CapKind::Scroll, CapLabeler::scrollLabel(step.dir, mode), now, true);
			c.dir = step.dir;
			c.gestureId = step.gestureId;
			return;
		}

		if (step.visible) {
			// The label follows the running sums while the gesture lasts.
			if (Cap * c = scrollCap(step.gestureId)) {
				if (c->dir != step.dir) {
					c->dir = step.dir;
					c->label = CapLabeler::scrollLabel(step.dir, mode);
				}
			}
		}
	}

	void CapHistory::pushMark(double now) {
		update(now);
		addCap(CapKind::Mark, CapLabeler::markLabel(settings_.labelMode), now);
	}

	//--------------------------------------------------------------
	// Lifetime (R10)
	//--------------------------------------------------------------
	void CapHistory::update(double now) {
		const int gestureId = scroll_.gestureId();
		const double gestureLast = scroll_.lastEventTime();
		if (scroll_.expire(now, settings_)) {
			if (Cap * c = scrollCap(gestureId)) {
				if (c->held) releaseCap(*c, gestureLast);
			}
		}

		const double hold = settings_.holdTime;
		const double fade = settings_.fadeTime;
		for (Cap & c : caps_) {
			if (c.held) {
				c.age = 0.f;
				c.opacity = 1.f;
				continue;
			}
			const double age = std::max(0.0, now - c.changedAt);
			c.age = static_cast<float>(age);
			if (age <= hold) {
				c.opacity = 1.f;
			} else if (fade > 0.0 && age < hold + fade) {
				c.opacity = static_cast<float>(1.0 - (age - hold) / fade);
			} else {
				c.opacity = 0.f;
			}
		}
		caps_.erase(std::remove_if(caps_.begin(), caps_.end(),
						[&](const Cap & c) {
							return !c.held && (now - c.changedAt) >= hold + fade;
						}),
			caps_.end());
	}

} // namespace core
} // namespace keycaps
} // namespace ofx
