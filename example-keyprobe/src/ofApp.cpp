#include "ofApp.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetEscapeQuitsApp(false); // Esc must be probed, not quit the app.
	ofBackground(30);

	ofAddListener(ofEvents().keyPressed, this, &ofApp::onKeyPressed, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().keyReleased, this, &ofApp::onKeyReleased, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mousePressed, this, &ofApp::onMousePressed, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseReleased, this, &ofApp::onMouseReleased, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseDragged, this, &ofApp::onMouseDragged, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseScrolled, this, &ofApp::onMouseScrolled, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseEntered, this, &ofApp::onMouseEntered, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseExited, this, &ofApp::onMouseExited, OF_EVENT_ORDER_AFTER_APP);

	printChecklist();
}

//--------------------------------------------------------------
void ofApp::update() {
	if (scrollActive && ofGetElapsedTimef() - scrollLast > scrollGap) {
		endScrollGesture();
	}
}

//--------------------------------------------------------------
void ofApp::exit() {
	if (scrollActive) {
		endScrollGesture();
	}
	ofRemoveListener(ofEvents().keyPressed, this, &ofApp::onKeyPressed, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().keyReleased, this, &ofApp::onKeyReleased, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mousePressed, this, &ofApp::onMousePressed, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseReleased, this, &ofApp::onMouseReleased, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseDragged, this, &ofApp::onMouseDragged, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseScrolled, this, &ofApp::onMouseScrolled, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseEntered, this, &ofApp::onMouseEntered, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseExited, this, &ofApp::onMouseExited, OF_EVENT_ORDER_AFTER_APP);
	std::cout << "PROBE END  events=" << seq << std::endl;
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(255);
	ofDrawBitmapString("ofxKeyCaps example-keyprobe v2", 20, 30);
	ofDrawBitmapString("Every event prints in the Terminal. Esc does not quit; use Cmd+Q.", 20, 50);
	float y = 90;
	for (const auto & line : recent) {
		ofDrawBitmapString(line, 20, y);
		y += 18;
	}
}

//--------------------------------------------------------------
// Keys
//--------------------------------------------------------------
void ofApp::onKeyPressed(ofKeyEventArgs & e) {
	std::ostringstream s;
	s << prefix(e.isRepeat ? "KEY REPEAT " : "KEY DOWN   ")
	  << "key=" << e.key << " " << describeKey(e.key, e.keycode)
	  << "  keycode=" << e.keycode
	  << "  scancode=" << e.scancode
	  << "  codepoint=" << describeCodepoint(e.codepoint)
	  << "  mods=" << describeModifiers(e.modifiers)
	  << "  polled=" << polledModifiers()
	  << "  isRepeat=" << (e.isRepeat ? "yes" : "no");
	emit(s.str());
}

void ofApp::onKeyReleased(ofKeyEventArgs & e) {
	std::ostringstream s;
	s << prefix("KEY UP     ")
	  << "key=" << e.key << " " << describeKey(e.key, e.keycode)
	  << "  keycode=" << e.keycode
	  << "  scancode=" << e.scancode
	  << "  codepoint=" << describeCodepoint(e.codepoint)
	  << "  mods=" << describeModifiers(e.modifiers)
	  << "  polled=" << polledModifiers();
	emit(s.str());
}

//--------------------------------------------------------------
// Mouse
//--------------------------------------------------------------
void ofApp::onMousePressed(ofMouseEventArgs & e) {
	dragEventsThisPress = 0;
	std::ostringstream s;
	s << prefix("MOUSE DOWN ")
	  << "button=" << describeButton(e.button)
	  << "  mods=" << describeModifiers(e.modifiers)
	  << "  polled=" << polledModifiers()
	  << "  x=" << (int)e.x << " y=" << (int)e.y;
	emit(s.str());
}

void ofApp::onMouseReleased(ofMouseEventArgs & e) {
	std::ostringstream s;
	s << prefix("MOUSE UP   ")
	  << "button=" << describeButton(e.button)
	  << "  mods=" << describeModifiers(e.modifiers)
	  << "  polled=" << polledModifiers()
	  << "  x=" << (int)e.x << " y=" << (int)e.y
	  << "  dragEventsDuringPress=" << dragEventsThisPress;
	emit(s.str());
	dragEventsThisPress = 0;
}

void ofApp::onMouseDragged(ofMouseEventArgs & e) {
	// Only the first drag event of each press is printed; the rest are
	// counted and reported on release, so a drag does not flood the log.
	++dragEventsThisPress;
	if (dragEventsThisPress != 1) {
		return;
	}
	std::ostringstream s;
	s << prefix("MOUSE DRAG ")
	  << "(first of this press)  button=" << describeButton(e.button)
	  << "  mods=" << describeModifiers(e.modifiers)
	  << "  polled=" << polledModifiers()
	  << "  x=" << (int)e.x << " y=" << (int)e.y;
	emit(s.str());
}

void ofApp::onMouseScrolled(ofMouseEventArgs & e) {
	float now = ofGetElapsedTimef();
	if (scrollActive && now - scrollLast > scrollGap) {
		endScrollGesture();
	}
	if (!scrollActive) {
		scrollActive = true;
		scrollStart = now;
		scrollEvents = 0;
		scrollSumX = scrollSumY = 0.f;
		scrollPeakX = scrollPeakY = 0.f;
		scrollYSignChanges = 0;
		scrollLastY = 0.f;
		std::ostringstream s;
		s << std::fixed << std::setprecision(3)
		  << prefix("SCROLL START")
		  << "first scrollX=" << e.scrollX << "  scrollY=" << e.scrollY
		  << "  mods=" << describeModifiers(e.modifiers)
		  << "  polled=" << polledModifiers();
		emit(s.str());
	}
	++scrollEvents;
	scrollLast = now;
	scrollSumX += e.scrollX;
	scrollSumY += e.scrollY;
	if (std::fabs(e.scrollX) > std::fabs(scrollPeakX)) scrollPeakX = e.scrollX;
	if (std::fabs(e.scrollY) > std::fabs(scrollPeakY)) scrollPeakY = e.scrollY;
	if (e.scrollY != 0.f) {
		if (scrollLastY != 0.f && (e.scrollY > 0.f) != (scrollLastY > 0.f)) {
			++scrollYSignChanges;
		}
		scrollLastY = e.scrollY;
	}
}

void ofApp::endScrollGesture() {
	scrollActive = false;
	const char * dominant;
	if (std::fabs(scrollSumY) >= std::fabs(scrollSumX)) {
		dominant = scrollSumY >= 0.f ? "Y positive" : "Y negative";
	} else {
		dominant = scrollSumX >= 0.f ? "X positive" : "X negative";
	}
	std::ostringstream s;
	s << std::fixed << std::setprecision(3)
	  << prefix("SCROLL END  ")
	  << "events=" << scrollEvents
	  << "  duration=" << (scrollLast - scrollStart)
	  << "  sumX=" << scrollSumX << "  sumY=" << scrollSumY
	  << "  peakX=" << scrollPeakX << "  peakY=" << scrollPeakY
	  << "  ySignChanges=" << scrollYSignChanges
	  << "  dominant=" << dominant;
	emit(s.str());
}

void ofApp::onMouseEntered(ofMouseEventArgs & e) {
	std::ostringstream s;
	s << prefix("MOUSE ENTER") << "x=" << (int)e.x << " y=" << (int)e.y;
	emit(s.str());
}

void ofApp::onMouseExited(ofMouseEventArgs & e) {
	std::ostringstream s;
	s << prefix("MOUSE EXIT ") << "x=" << (int)e.x << " y=" << (int)e.y;
	emit(s.str());
}

//--------------------------------------------------------------
// Formatting
//--------------------------------------------------------------
std::string ofApp::prefix(const std::string & kind) {
	float now = ofGetElapsedTimef();
	float delta = (seq == 0) ? 0.f : now - lastTime;
	lastTime = now;
	++seq;
	std::ostringstream s;
	s << "#" << std::setw(4) << std::setfill('0') << seq << std::setfill(' ')
	  << std::fixed << std::setprecision(3)
	  << "  t=" << std::setw(8) << now
	  << "  dt=" << std::setw(7) << delta
	  << "  " << kind << "  ";
	return s.str();
}

std::string ofApp::hex(int value) {
	std::ostringstream s;
	s << "0x" << std::hex << std::uppercase << value;
	return s.str();
}

std::string ofApp::describeKey(int key, int keycode) {
	// Pressing a modifier sends two events with the same keycode: one whose
	// key is the modifier's bit flag (OF_KEY_SHIFT, OF_KEY_CONTROL, ...),
	// and one whose key is the left/right code (OF_KEY_LEFT_SHIFT, ...).
	// GLFW modifier keycodes are 340-347.
	if (keycode >= 340 && keycode <= 347) {
		if (key == OF_KEY_SHIFT) return "(modifier flag: shift)";
		if (key == OF_KEY_CONTROL) return "(modifier flag: ctrl)";
		if (key == OF_KEY_ALT) return "(modifier flag: option)";
		if (key == OF_KEY_SUPER) return "(modifier flag: cmd)";
	}
	// A list, not a switch: several OF_KEY_* names share a value
	// (OF_KEY_COMMAND == OF_KEY_SUPER), and the first match wins.
	static const std::vector<std::pair<int, const char *>> names = {
		{ OF_KEY_RETURN, "Return" }, { OF_KEY_ESC, "Esc" }, { OF_KEY_TAB, "Tab" },
		{ OF_KEY_BACKSPACE, "Backspace" }, { OF_KEY_DEL, "Delete" }, { OF_KEY_SPACE, "Space" },
		{ OF_KEY_LEFT_SHIFT, "LeftShift" }, { OF_KEY_RIGHT_SHIFT, "RightShift" },
		{ OF_KEY_LEFT_CONTROL, "LeftControl" }, { OF_KEY_RIGHT_CONTROL, "RightControl" },
		{ OF_KEY_LEFT_ALT, "LeftOption" }, { OF_KEY_RIGHT_ALT, "RightOption" },
		{ OF_KEY_LEFT_SUPER, "LeftCommand" }, { OF_KEY_RIGHT_SUPER, "RightCommand" },
		{ OF_KEY_F1, "F1" }, { OF_KEY_F2, "F2" }, { OF_KEY_F3, "F3" }, { OF_KEY_F4, "F4" },
		{ OF_KEY_F5, "F5" }, { OF_KEY_F6, "F6" }, { OF_KEY_F7, "F7" }, { OF_KEY_F8, "F8" },
		{ OF_KEY_F9, "F9" }, { OF_KEY_F10, "F10" }, { OF_KEY_F11, "F11" }, { OF_KEY_F12, "F12" },
		{ OF_KEY_LEFT, "LeftArrow" }, { OF_KEY_UP, "UpArrow" },
		{ OF_KEY_RIGHT, "RightArrow" }, { OF_KEY_DOWN, "DownArrow" },
		{ OF_KEY_PAGE_UP, "PageUp" }, { OF_KEY_PAGE_DOWN, "PageDown" },
		{ OF_KEY_HOME, "Home" }, { OF_KEY_END, "End" }, { OF_KEY_INSERT, "Insert" },
	};
	for (const auto & n : names) {
		if (n.first == key) {
			return std::string("(") + n.second + ")";
		}
	}
	if (key > 0 && key < 32) {
		return "(control code " + hex(key) + ")";
	}
	if (key >= 32) {
		std::string ch;
		ofUTF8Append(ch, (uint32_t)key);
		return "'" + ch + "'";
	}
	return "(none)";
}

std::string ofApp::describeCodepoint(uint32_t cp) {
	std::ostringstream s;
	s << "U+" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << cp;
	if (cp >= 32 && cp != 127) {
		std::string ch;
		ofUTF8Append(ch, cp);
		s << " '" << ch << "'";
	} else if (cp > 0) {
		s << " (control)";
	}
	return s.str();
}

std::string ofApp::describeModifiers(int mods) {
	std::string names;
	if (mods & OF_KEY_CONTROL) names += "ctrl ";
	if (mods & OF_KEY_ALT) names += "option ";
	if (mods & OF_KEY_SHIFT) names += "shift ";
	if (mods & OF_KEY_SUPER) names += "cmd ";
	if (!names.empty()) names.pop_back();
	return hex(mods) + " [" + names + "]";
}

std::string ofApp::polledModifiers() {
	std::string names;
	if (ofGetKeyPressed(OF_KEY_CONTROL)) names += "ctrl ";
	if (ofGetKeyPressed(OF_KEY_ALT)) names += "option ";
	if (ofGetKeyPressed(OF_KEY_SHIFT)) names += "shift ";
	if (ofGetKeyPressed(OF_KEY_SUPER)) names += "cmd ";
	if (!names.empty()) names.pop_back();
	return "[" + names + "]";
}

std::string ofApp::describeButton(int button) {
	std::string name;
	if (button == OF_MOUSE_BUTTON_LEFT) {
		name = "left";
	} else if (button == OF_MOUSE_BUTTON_RIGHT) {
		name = "right";
	} else if (button == OF_MOUSE_BUTTON_MIDDLE) {
		name = "middle";
	} else {
		name = "button" + ofToString(button + 1);
	}
	return ofToString(button) + " (" + name + ")";
}

//--------------------------------------------------------------
void ofApp::emit(const std::string & line) {
	std::cout << line << std::endl;
	recent.push_back(line);
	while (recent.size() > 26) {
		recent.pop_front();
	}
}

void ofApp::printChecklist() {
	std::cout << "PROBE START  ofxKeyCaps example-keyprobe v2  openFrameworks "
			  << ofGetVersionMajor() << "." << ofGetVersionMinor() << "." << ofGetVersionPatch()
			  << std::endl;
	const char * lines[] = {
		"Pass 2: only the steps pass 1 did not cover. Probe window in front.",
		"Pause about two seconds between steps, so each one stands apart in the log.",
		"KEYS",
		"K1  Shift+1",
		"K2  Space, Return, Backspace, Fn+Backspace (Delete), Esc",
		"K3  Left, Right, Up, Down arrows, then Fn+Left, Fn+Right, Fn+Up, Fn+Down",
		"K4  F1 and F12 (hold Fn if those keys control brightness or volume)",
		"K5  Ctrl+a, Option+a, Option+e, Cmd+Shift+z",
		"K6  tap Ctrl alone, Option alone, right Shift alone, right Cmd alone",
		"MOUSE",
		"M1  right click (two-finger click on a trackpad)",
		"M2  Ctrl+click, Option+click, Shift+click, Cmd+click",
		"M3  press inside, drag inside, let go inside",
		"M4  press inside, drag outside the window, let go outside",
		"M5  four separate scroll gestures, two seconds apart, in this order:",
		"    fingers move UP the trackpad, then DOWN, then LEFT, then RIGHT",
		"M6  middle click and side buttons, if a mouse with them is connected",
		"Quit with Cmd+Q when done.",
	};
	for (const char * l : lines) {
		std::cout << l << std::endl;
	}
}
