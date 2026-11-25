#pragma once

// example-basic — ofxKeyCaps in use (KEYCAPS.md build step 4).
// Mouse caps are on. A grid follows scrolling, so the scroll caps can be
// checked against the way the content moved (PENDING-5, D-022).

#include "ofMain.h"
#include "ofxKeyCaps.h"

class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void mouseScrolled(int x, int y, float scrollX, float scrollY) override;

private:
	ofxKeyCaps keyCaps;

	// The grid's offset, moved by scrolling.
	glm::vec2 offset { 0.f, 0.f };
	// How far the grid moved in the current scroll gesture.
	glm::vec2 gestureMove { 0.f, 0.f };
	float lastScroll = -1.f;
};
