#pragma once

// OfInputSource — key and mouse events from the openFrameworks window
// (KEYCAPS.md 6.8). Listens after the app, never consumes an event, and
// leaves the translation rules to core/RawInput.

#include "InputSource.h"
#include "ofEvents.h"
#include <functional>

namespace ofx {
namespace keycaps {

	class OfInputSource : public InputSource {
	public:
		OfInputSource() = default;
		~OfInputSource() override { stop(); }

		// Returns the window's pixel scale (1 or 2), used to turn drag
		// distances into logical points. Defaults to 1.
		void setPixelScaleProvider(std::function<float()> provider) { pixelScale_ = std::move(provider); }

		void start(KeyHandler onKey, MouseHandler onMouse) override;
		void stop() override;
		bool running() const { return running_; }

	private:
		void onKeyPressed(ofKeyEventArgs & e);
		void onMousePressed(ofMouseEventArgs & e);
		void onMouseReleased(ofMouseEventArgs & e);
		void onMouseDragged(ofMouseEventArgs & e);
		void onMouseScrolled(ofMouseEventArgs & e);

		KeyHandler onKey_;
		MouseHandler onMouse_;
		std::function<float()> pixelScale_;
		ofEventListeners listeners_;
		bool running_ = false;

		// Where each button went down, for drag distance (oF buttons 0..7).
		glm::vec2 pressAt_[8];
	};

} // namespace keycaps
} // namespace ofx
