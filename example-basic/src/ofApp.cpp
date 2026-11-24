#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofBackground(30);

	ofxKeyCaps::Style style;
	style.core.showMouse = true;
	keys.setup(style);
	keys.mark(); // the log's zero is now

	std::cout << "example-basic: font symbols "
			  << (keys.symbolsAvailable() ? "available" : "NOT available, using text labels")
			  << std::endl;
	std::cout << "example-basic: type, click, drag and scroll. Each scroll gesture also prints"
			  << " which way the grid moved. Quit with Cmd+Q." << std::endl;
}

//--------------------------------------------------------------
void ofApp::update() {
	// Report a finished scroll gesture: the grid moves with the content, so
	// "grid moved up" should match a Scroll down cap, and "grid moved left"
	// should match Scroll right (D-018, D-022).
	if (lastScroll >= 0.f && ofGetElapsedTimef() - lastScroll > 0.3f) {
		const bool vertical = std::fabs(gestureMove.y) >= std::fabs(gestureMove.x);
		std::string way;
		if (vertical) way = gestureMove.y < 0 ? "UP" : "DOWN";
		else way = gestureMove.x < 0 ? "LEFT" : "RIGHT";
		std::cout << "example-basic: grid moved " << way << " ("
				  << (int)gestureMove.x << ", " << (int)gestureMove.y << " px)" << std::endl;
		gestureMove = { 0.f, 0.f };
		lastScroll = -1.f;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	// A grid that follows scrolling.
	const float step = 60.f;
	const float ox = std::fmod(offset.x, step);
	const float oy = std::fmod(offset.y, step);
	ofSetColor(60);
	for (float x = ox - step; x < ofGetWidth() + step; x += step) ofDrawLine(x, 0, x, ofGetHeight());
	for (float y = oy - step; y < ofGetHeight() + step; y += step) ofDrawLine(0, y, ofGetWidth(), y);

	ofSetColor(200);
	ofDrawBitmapString("ofxKeyCaps example-basic", 20, 30);
	ofDrawBitmapString("Type, click, drag and scroll. The log prints in the Terminal. Cmd+Q quits.", 20, 50);

	keys.draw(); // last, on top of everything
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int, int, float scrollX, float scrollY) {
	const glm::vec2 move = glm::vec2(scrollX, scrollY) * 20.f;
	offset += move;
	gestureMove += move;
	lastScroll = ofGetElapsedTimef();
}
