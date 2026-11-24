#include "ofApp.h"
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
void ofApp::exit() {
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
	ofDrawBitmapString("ofxKeyCaps example-keyprobe", 20, 30);
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
	  << "key=" << e.key << " " << describeKey(e.key)
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
	  << "key=" << e.key << " " << describeKey(e.key)
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
	std::ostringstream s;
	s << std::fixed << std::setprecision(3)
	  << prefix("MOUSE SCROLL")
	  << "scrollX=" << e.scrollX << "  scrollY=" << e.scrollY
	  << "  button=" << describeButton(e.button)
	  << "  mods=" << describeModifiers(e.modifiers)
	  << "  polled=" << polledModifiers();
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

std::string ofApp::describeKey(int key) {
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
	std::cout << "PROBE START  ofxKeyCaps example-keyprobe  openFrameworks "
			  << ofGetVersionMajor() << "." << ofGetVersionMinor() << "." << ofGetVersionPatch()
			  << std::endl;
	const char * lines[] = {
		"Do each step with the probe window in front. Pause a second between steps.",
		"KEYS",
		"K1  a, then Shift+a, then 1, then Shift+1",
		"K2  Space, Return, Tab, Backspace, Fn+Backspace (Delete), Esc",
		"K3  the four arrows, then Fn+Left (Home), Fn+Right (End), Fn+Up (PageUp), Fn+Down (PageDown)",
		"K4  F1 (with Fn if your F-keys are media keys), then F12",
		"K5  Ctrl+a, Option+a, Cmd+s, Cmd+Shift+z",
		"K6  Shift alone, Ctrl alone, Option alone, Cmd alone (tap each and let go)",
		"K7  hold a for about two seconds, then let go",
		"K8  hold Cmd, tap s, let go of s, then let go of Cmd",
		"MOUSE",
		"M1  left click, then right click (two-finger click on a trackpad)",
		"M2  Ctrl+click, Option+click, Shift+click, Cmd+click",
		"M3  click and drag inside the window, let go inside",
		"M4  click inside, drag outside the window, let go outside",
		"M5  scroll down, scroll up, scroll left, scroll right",
		"M6  middle click and side buttons, if your mouse has them",
		"Quit with Cmd+Q when done.",
	};
	for (const char * l : lines) {
		std::cout << l << std::endl;
	}
}
