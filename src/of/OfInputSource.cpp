#include "OfInputSource.h"
#include "../core/RawInput.h"
#include <cmath>

#if defined(__has_include)
#if __has_include(<GLFW/glfw3.h>)
#include <GLFW/glfw3.h>
#define OFXKEYCAPS_HAS_GLFW 1
#endif
#endif

namespace ofx {
namespace keycaps {

	// The core keeps oF's numbers as plain constants so it can be tested
	// without oF (D-027). These checks stop the build if oF ever changes them.
	namespace raw = core::raw;
	static_assert(raw::ModShift == OF_KEY_SHIFT, "OF_KEY_SHIFT changed");
	static_assert(raw::ModControl == OF_KEY_CONTROL, "OF_KEY_CONTROL changed");
	static_assert(raw::ModAlt == OF_KEY_ALT, "OF_KEY_ALT changed");
	static_assert(raw::ModSuper == OF_KEY_SUPER, "OF_KEY_SUPER changed");
	static_assert(raw::KeyLeftShift == OF_KEY_LEFT_SHIFT, "OF_KEY_LEFT_SHIFT changed");
	static_assert(raw::KeyRightSuper == OF_KEY_RIGHT_SUPER, "OF_KEY_RIGHT_SUPER changed");
	static_assert(raw::KeyPrivateUseFirst == OF_KEY_F1, "OF_KEY_F1 changed");
	static_assert(raw::ButtonLeft == OF_MOUSE_BUTTON_LEFT, "OF_MOUSE_BUTTON_LEFT changed");
	static_assert(raw::ButtonMiddle == OF_MOUSE_BUTTON_MIDDLE, "OF_MOUSE_BUTTON_MIDDLE changed");
	static_assert(raw::ButtonRight == OF_MOUSE_BUTTON_RIGHT, "OF_MOUSE_BUTTON_RIGHT changed");
	static_assert(raw::ButtonLast == OF_MOUSE_BUTTON_LAST, "OF_MOUSE_BUTTON_LAST changed");
#ifdef OFXKEYCAPS_HAS_GLFW
	static_assert(raw::CodeSpace == GLFW_KEY_SPACE, "GLFW_KEY_SPACE changed");
	static_assert(raw::CodePrintableLast == GLFW_KEY_GRAVE_ACCENT, "GLFW_KEY_GRAVE_ACCENT changed");
	static_assert(raw::CodeWorld1 == GLFW_KEY_WORLD_1, "GLFW_KEY_WORLD_1 changed");
	static_assert(raw::CodeWorld2 == GLFW_KEY_WORLD_2, "GLFW_KEY_WORLD_2 changed");
	static_assert(raw::CodeEscape == GLFW_KEY_ESCAPE, "GLFW_KEY_ESCAPE changed");
	static_assert(raw::CodeEnter == GLFW_KEY_ENTER, "GLFW_KEY_ENTER changed");
	static_assert(raw::CodeTab == GLFW_KEY_TAB, "GLFW_KEY_TAB changed");
	static_assert(raw::CodeBackspace == GLFW_KEY_BACKSPACE, "GLFW_KEY_BACKSPACE changed");
	static_assert(raw::CodeInsert == GLFW_KEY_INSERT, "GLFW_KEY_INSERT changed");
	static_assert(raw::CodeDelete == GLFW_KEY_DELETE, "GLFW_KEY_DELETE changed");
	static_assert(raw::CodeRight == GLFW_KEY_RIGHT, "GLFW_KEY_RIGHT changed");
	static_assert(raw::CodeLeft == GLFW_KEY_LEFT, "GLFW_KEY_LEFT changed");
	static_assert(raw::CodeDown == GLFW_KEY_DOWN, "GLFW_KEY_DOWN changed");
	static_assert(raw::CodeUp == GLFW_KEY_UP, "GLFW_KEY_UP changed");
	static_assert(raw::CodePageUp == GLFW_KEY_PAGE_UP, "GLFW_KEY_PAGE_UP changed");
	static_assert(raw::CodePageDown == GLFW_KEY_PAGE_DOWN, "GLFW_KEY_PAGE_DOWN changed");
	static_assert(raw::CodeHome == GLFW_KEY_HOME, "GLFW_KEY_HOME changed");
	static_assert(raw::CodeEnd == GLFW_KEY_END, "GLFW_KEY_END changed");
	static_assert(raw::CodeCapsLock == GLFW_KEY_CAPS_LOCK, "GLFW_KEY_CAPS_LOCK changed");
	static_assert(raw::CodeF1 == GLFW_KEY_F1, "GLFW_KEY_F1 changed");
	static_assert(raw::CodeF12 == GLFW_KEY_F12, "GLFW_KEY_F12 changed");
	static_assert(raw::CodeKeypadEnter == GLFW_KEY_KP_ENTER, "GLFW_KEY_KP_ENTER changed");
	static_assert(raw::CodeLeftShift == GLFW_KEY_LEFT_SHIFT, "GLFW_KEY_LEFT_SHIFT changed");
	static_assert(raw::CodeLeftControl == GLFW_KEY_LEFT_CONTROL, "GLFW_KEY_LEFT_CONTROL changed");
	static_assert(raw::CodeLeftAlt == GLFW_KEY_LEFT_ALT, "GLFW_KEY_LEFT_ALT changed");
	static_assert(raw::CodeLeftSuper == GLFW_KEY_LEFT_SUPER, "GLFW_KEY_LEFT_SUPER changed");
	static_assert(raw::CodeRightShift == GLFW_KEY_RIGHT_SHIFT, "GLFW_KEY_RIGHT_SHIFT changed");
	static_assert(raw::CodeRightControl == GLFW_KEY_RIGHT_CONTROL, "GLFW_KEY_RIGHT_CONTROL changed");
	static_assert(raw::CodeRightAlt == GLFW_KEY_RIGHT_ALT, "GLFW_KEY_RIGHT_ALT changed");
	static_assert(raw::CodeRightSuper == GLFW_KEY_RIGHT_SUPER, "GLFW_KEY_RIGHT_SUPER changed");
#endif

	void OfInputSource::start(KeyHandler onKey, MouseHandler onMouse) {
		stop();
		onKey_ = std::move(onKey);
		onMouse_ = std::move(onMouse);
		// After the app, so the app sees every event first. Void listeners
		// never consume an event (P2).
		listeners_.push(ofEvents().keyPressed.newListener(this, &OfInputSource::onKeyPressed, OF_EVENT_ORDER_AFTER_APP));
		listeners_.push(ofEvents().mousePressed.newListener(this, &OfInputSource::onMousePressed, OF_EVENT_ORDER_AFTER_APP));
		listeners_.push(ofEvents().mouseReleased.newListener(this, &OfInputSource::onMouseReleased, OF_EVENT_ORDER_AFTER_APP));
		listeners_.push(ofEvents().mouseDragged.newListener(this, &OfInputSource::onMouseDragged, OF_EVENT_ORDER_AFTER_APP));
		listeners_.push(ofEvents().mouseScrolled.newListener(this, &OfInputSource::onMouseScrolled, OF_EVENT_ORDER_AFTER_APP));
		running_ = true;
	}

	void OfInputSource::stop() {
		listeners_.unsubscribeAll();
		running_ = false;
	}

	void OfInputSource::onKeyPressed(ofKeyEventArgs & e) {
		if (!onKey_) return;
		core::RawKey r;
		r.key = e.key;
		r.keycode = e.keycode;
		r.modifiers = e.modifiers;
		r.isRepeat = e.isRepeat;
		core::KeyStroke k;
		if (core::translateKey(r, k)) onKey_(k);
	}

	void OfInputSource::onMousePressed(ofMouseEventArgs & e) {
		core::MouseAction m;
		if (!onMouse_ || !core::translateButton(e.button, m.button, m.otherNumber)) return;
		pressAt_[e.button] = glm::vec2(e.x, e.y);
		m.phase = core::MousePhase::Down;
		m.modifiers = core::translateModifiers(e.modifiers);
		onMouse_(m);
	}

	void OfInputSource::onMouseReleased(ofMouseEventArgs & e) {
		core::MouseAction m;
		if (!onMouse_ || !core::translateButton(e.button, m.button, m.otherNumber)) return;
		m.phase = core::MousePhase::Up;
		m.modifiers = core::translateModifiers(e.modifiers);
		onMouse_(m);
	}

	void OfInputSource::onMouseDragged(ofMouseEventArgs & e) {
		core::MouseAction m;
		if (!onMouse_ || !core::translateButton(e.button, m.button, m.otherNumber)) return;
		const float scale = pixelScale_ ? std::max(1.f, pixelScale_()) : 1.f;
		const glm::vec2 d = glm::vec2(e.x, e.y) - pressAt_[e.button];
		m.phase = core::MousePhase::Drag;
		m.modifiers = core::translateModifiers(e.modifiers);
		m.dragDistance = std::sqrt(d.x * d.x + d.y * d.y) / scale; // logical points
		onMouse_(m);
	}

	void OfInputSource::onMouseScrolled(ofMouseEventArgs & e) {
		if (!onMouse_) return;
		onMouse_(core::MouseAction::scroll(e.scrollX, e.scrollY, core::translateModifiers(e.modifiers)));
	}

} // namespace keycaps
} // namespace ofx
