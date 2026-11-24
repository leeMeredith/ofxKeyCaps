#pragma once

// ScrollGesture — groups scroll events into gestures (KEYCAPS.md M6, D-018).
// CapHistory and EventLog each own one, so the screen and the log always
// agree on where a gesture starts, ends and becomes visible (D-025).

#include "CoreSettings.h"
#include "MouseAction.h"

namespace ofxKeyCaps {
namespace core {

	// What one scroll event did to the gesture state.
	struct ScrollStep {
		bool endedPrevious = false; // a gesture ended before this event (gap or reversal)
		double previousEndTime = 0.0; // time of the ended gesture's last event
		bool becameVisible = false; // this event made the current gesture visible
		bool visible = false; // the current gesture is visible after this event
		ScrollDir dir = ScrollDir::Down; // current gesture's direction
		int gestureId = 0; // current gesture's id, counting from 1
	};

	class ScrollGesture {
	public:
		// Feed one Scroll event at time `now` (seconds).
		ScrollStep feed(float scrollX, float scrollY, double now, const CoreSettings & s);

		// Ends the active gesture if no event has arrived for longer than
		// scrollGap. Returns true if it ended; lastEventTime() is its end.
		bool expire(double now, const CoreSettings & s);

		bool active() const { return active_; }
		bool visible() const { return active_ && visible_; }
		ScrollDir direction() const { return dirFromSums(); }
		int gestureId() const { return id_; }
		double lastEventTime() const { return last_; }

		void reset();

	private:
		ScrollDir dirFromSums() const;
		bool yAxis() const;
		void start(float scrollX, float scrollY, double now);

		bool active_ = false;
		bool visible_ = false;
		double sumX_ = 0.0;
		double sumY_ = 0.0;
		double last_ = 0.0;
		int id_ = 0;
	};

} // namespace core
} // namespace ofxKeyCaps
