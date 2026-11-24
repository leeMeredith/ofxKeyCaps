#pragma once

// example-keyprobe — KEYCAPS.md build step 2.
// Prints the raw fields of every key and mouse event oF delivers, one line
// per event, so PENDING-1 (keys) and PENDING-5 (mouse) can be answered from
// measurement. It listens the same way OfInputSource will: through
// ofEvents() at OF_EVENT_ORDER_AFTER_APP, never consuming an event.
//
// v2: names the modifier-flag key events, and prints each scroll gesture
// as a START line and an END summary instead of one line per event.

#include "ofMain.h"
#include <deque>

class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;

private:
	// Listeners, registered at OF_EVENT_ORDER_AFTER_APP.
	void onKeyPressed(ofKeyEventArgs & e);
	void onKeyReleased(ofKeyEventArgs & e);
	void onMousePressed(ofMouseEventArgs & e);
	void onMouseReleased(ofMouseEventArgs & e);
	void onMouseDragged(ofMouseEventArgs & e);
	void onMouseScrolled(ofMouseEventArgs & e);
	void onMouseEntered(ofMouseEventArgs & e);
	void onMouseExited(ofMouseEventArgs & e);

	// Scroll gestures: a run of scroll events with no gap longer than
	// scrollGap seconds. Closed from update() once the gap has passed.
	void endScrollGesture();
	static constexpr float scrollGap = 0.3f;
	bool scrollActive = false;
	float scrollStart = 0.f;
	float scrollLast = 0.f;
	int scrollEvents = 0;
	float scrollSumX = 0.f;
	float scrollSumY = 0.f;
	float scrollPeakX = 0.f;
	float scrollPeakY = 0.f;
	int scrollYSignChanges = 0;
	float scrollLastY = 0.f;

	// Formatting.
	std::string prefix(const std::string & kind);
	static std::string describeKey(int key, int keycode);
	static std::string describeCodepoint(uint32_t cp);
	static std::string describeModifiers(int mods);
	static std::string polledModifiers();
	static std::string describeButton(int button);
	static std::string hex(int value);

	void emit(const std::string & line);
	void printChecklist();

	int seq = 0;
	float lastTime = 0.f;
	int dragEventsThisPress = 0;
	std::deque<std::string> recent;
};
