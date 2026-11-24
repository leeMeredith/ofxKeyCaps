#pragma once

// ofxKeyCaps — shows the keys pressed in an openFrameworks window as
// fading keycaps, with an optional row entry for mouse buttons, and prints
// a timestamped log of every press. See KEYCAPS.md.
//
// The whole integration:
//     ofxKeyCaps keys;          // member of ofApp
//     keys.setup();             // in setup()
//     keys.draw();              // last line of draw()

#include "core/CapHistory.h"
#include "core/EventLog.h"
#include "of/KeycapRenderer.h"
#include "of/OfInputSource.h"
#include "of/Style.h"
#include "ofEvents.h"

class ofxKeyCaps {
public:
	using Style = ofx::keycaps::Style;

	ofxKeyCaps() = default;
	~ofxKeyCaps();
	ofxKeyCaps(const ofxKeyCaps &) = delete;
	ofxKeyCaps & operator=(const ofxKeyCaps &) = delete;

	void setup();
	void setup(const Style & style);

	// Draws in window pixels; call it last in draw().
	void draw();

	// Sync mark: the log's zero resets and a "● MARK" cap appears (KEYCAPS.md 7).
	void mark();

	void setEnabled(bool enabled);
	bool isEnabled() const { return enabled_; }

	void setMasked(bool masked);
	bool isMasked() const { return masked_; }

	void setStyle(const Style & style);
	const Style & getStyle() const { return style_; }

	void clear();

	// True when the loaded font draws ⌃ ⌥ ⇧ ⌘ and the other symbols. When it
	// does not, Symbols mode falls back to text labels (PENDING-2, D-031, D-032).
	bool symbolsAvailable() const { return renderer_.symbolsAvailable(); }

private:
	void onUpdate(ofEventArgs &);
	void handleKey(const ofx::keycaps::core::KeyStroke & k);
	void handleMouse(const ofx::keycaps::core::MouseAction & m);
	void applyStyle();
	void print(const std::string & line) const;
	float pixelScale() const;
	double now() const;

	Style style_;
	ofx::keycaps::core::CoreSettings effective_; // style_.core after the font fallback
	ofx::keycaps::core::CapHistory history_;
	ofx::keycaps::core::EventLog log_;
	ofx::keycaps::OfInputSource input_;
	ofx::keycaps::KeycapRenderer renderer_;
	ofEventListener updateListener_;
	bool setup_ = false;
	bool enabled_ = true;
	bool masked_ = false;
};
