#include "ofxKeyCaps.h"
#include "ofMain.h"

#if !defined(TARGET_OPENGLES)
#include "ofAppGLFWWindow.h"
#endif

using namespace ofx::keycaps;

ofxKeyCaps::~ofxKeyCaps() {
	input_.stop();
	updateListener_.unsubscribe();
}

void ofxKeyCaps::setup() {
	setup(Style());
}

void ofxKeyCaps::setup(const Style & style) {
	style_ = style;
	setup_ = true;
	applyStyle();

	input_.setPixelScaleProvider([this] { return pixelScale(); });
	input_.start([this](const core::KeyStroke & k) { handleKey(k); },
		[this](const core::MouseAction & m) { handleMouse(m); });
	updateListener_ = ofEvents().update.newListener(this, &ofxKeyCaps::onUpdate, OF_EVENT_ORDER_AFTER_APP);
}

void ofxKeyCaps::applyStyle() {
	const float scale = pixelScale();
	if (!renderer_.isLoaded() || renderer_.loadedScale() != scale || renderer_.loadedFontSize() != style_.fontSize) {
		if (!renderer_.setup(style_, scale)) {
			ofLogError("ofxKeyCaps") << "no font could be loaded; caps will not be drawn";
		} else if (renderer_.symbolsAvailable()) {
			ofLogNotice("ofxKeyCaps") << "font \"" << renderer_.fontName() << "\" has every symbol";
		}
	}

	effective_ = style_.core;
	if (effective_.labelMode == core::LabelMode::Symbols && renderer_.isLoaded() && !renderer_.symbolsAvailable()) {
#if defined(TARGET_OSX)
		const core::LabelMode fallback = core::LabelMode::MacText; // Cmd+, Opt+ (D-031)
#else
		const core::LabelMode fallback = core::LabelMode::Text;
#endif
		ofLogWarning("ofxKeyCaps") << "no font with every symbol: " << renderer_.missingSymbols()
								   << "- using text labels with \"" << renderer_.fontName() << "\"";
		effective_.labelMode = fallback;
	}
	history_.setSettings(effective_);
	log_.setSettings(effective_);
}

void ofxKeyCaps::setStyle(const Style & style) {
	style_ = style;
	if (setup_) applyStyle();
}

void ofxKeyCaps::draw() {
	if (!setup_ || !enabled_) return;
	if (renderer_.loadedScale() != pixelScale()) applyStyle(); // moved to another display
	renderer_.draw(history_, style_, pixelScale(), now());
}

void ofxKeyCaps::onUpdate(ofEventArgs &) {
	if (enabled_) history_.update(now());
}

void ofxKeyCaps::handleKey(const core::KeyStroke & k) {
	if (!enabled_) return;
	const double t = now();
	history_.push(k, t);
	std::string line;
	if (log_.log(k, t, line)) print(line);
}

void ofxKeyCaps::handleMouse(const core::MouseAction & m) {
	if (!enabled_) return;
	const double t = now();
	history_.push(m, t);
	std::string line;
	if (log_.log(m, t, line)) print(line);
}

void ofxKeyCaps::mark() {
	if (!setup_ || !enabled_) return;
	const double t = now();
	print(log_.mark(t));
	history_.pushMark(t);
}

void ofxKeyCaps::setEnabled(bool enabled) {
	if (enabled && !enabled_) history_.clear(); // starts with no caps
	enabled_ = enabled;
}

void ofxKeyCaps::setMasked(bool masked) {
	masked_ = masked;
	history_.setMasked(masked);
	log_.setMasked(masked);
}

void ofxKeyCaps::clear() {
	history_.clear();
}

void ofxKeyCaps::print(const std::string & line) const {
	if (style_.log) ofLogNotice("ofxKeyCaps") << line;
}

double ofxKeyCaps::now() const {
	return ofGetElapsedTimef();
}

float ofxKeyCaps::pixelScale() const {
#if !defined(TARGET_OPENGLES)
	if (auto * w = dynamic_cast<ofAppGLFWWindow *>(ofGetWindowPtr())) {
		return static_cast<float>(std::max(1, w->getPixelScreenCoordScale()));
	}
#endif
	return 1.f;
}
