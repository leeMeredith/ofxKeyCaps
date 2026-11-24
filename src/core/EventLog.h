#pragma once

// EventLog — one timestamped text line per press (KEYCAPS.md 6.6, D-009,
// D-014). It returns lines; the front end prints them. Time comes in as
// `now` in seconds from any fixed origin (P4).

#include "CapLabeler.h"
#include "CoreSettings.h"
#include "KeyStroke.h"
#include "MouseAction.h"
#include "ScrollGesture.h"
#include <string>

namespace ofx {
namespace keycaps {
namespace core {

	class EventLog {
	public:
		explicit EventLog(const CoreSettings & settings = CoreSettings());

		void setSettings(const CoreSettings & settings) { settings_ = settings; }
		const CoreSettings & settings() const { return settings_; }

		void setMasked(bool masked) { masked_ = masked; }

		// Each returns true and fills `line` when the event makes a line.
		bool log(const KeyStroke & k, double now, std::string & line);
		bool log(const MouseAction & m, double now, std::string & line);

		// Resets the zero to `now` and returns "00:00:00.000  ● MARK".
		std::string mark(double now);

		double zero() const { return zero_; }

		// HH:MM:SS.mmm, negative times clamped to zero.
		static std::string formatTime(double seconds);

	private:
		std::string stamp(double now, const std::string & label) const;

		CoreSettings settings_;
		bool masked_ = false;
		double zero_ = 0.0;
		ScrollGesture scroll_;
		bool dragLogged_[9] = { false };
	};

} // namespace core
} // namespace keycaps
} // namespace ofx
