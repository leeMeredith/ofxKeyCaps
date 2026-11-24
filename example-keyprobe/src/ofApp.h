#pragma once

// example-keyprobe — KEYCAPS.md build step 2.
// Prints the raw fields of every key and mouse event oF delivers, one line
// per event, so PENDING-1 (keys) and PENDING-5 (mouse) can be answered from
// measurement. It listens the same way OfInputSource will: through
// ofEvents() at OF_EVENT_ORDER_AFTER_APP, never consuming an event.

#include "ofMain.h"
#include <deque>

class ofApp : public ofBaseApp {
public:
	void setup() override;
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

	// Formatting.
	std::string prefix(const std::string & kind);
	static std::string describeKey(int key);
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
