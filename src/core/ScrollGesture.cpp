#include "ScrollGesture.h"
#include <cmath>

namespace ofx {
namespace keycaps {
namespace core {

	bool ScrollGesture::yAxis() const {
		// Ties go to the vertical axis, the common case.
		return std::fabs(sumY_) >= std::fabs(sumX_);
	}

	ScrollDir ScrollGesture::dirFromSums() const {
		if (yAxis()) {
			return sumY_ < 0.0 ? ScrollDir::Down : ScrollDir::Up; // D-018
		}
		return sumX_ < 0.0 ? ScrollDir::Right : ScrollDir::Left; // D-022
	}

	void ScrollGesture::start(float scrollX, float scrollY, double now) {
		active_ = true;
		visible_ = false;
		sumX_ = scrollX;
		sumY_ = scrollY;
		last_ = now;
		++id_;
	}

	ScrollStep ScrollGesture::feed(float scrollX, float scrollY, double now, const CoreSettings & s) {
		ScrollStep step;

		if (active_) {
			bool ended = false;
			if (now - last_ > s.scrollGap) {
				ended = true; // gap
			} else {
				// Reversal: this event points against the gesture's sum on
				// the gesture's own direction axis.
				const double sum = yAxis() ? sumY_ : sumX_;
				const double v = yAxis() ? scrollY : scrollX;
				if (sum != 0.0 && v != 0.0 && ((sum > 0.0) != (v > 0.0))) {
					ended = true;
				}
			}
			if (ended) {
				step.endedPrevious = visible_;
				step.previousEndTime = last_;
				active_ = false;
			}
		}

		if (!active_) {
			start(scrollX, scrollY, now);
		} else {
			sumX_ += scrollX;
			sumY_ += scrollY;
			last_ = now;
		}

		const double axisSum = yAxis() ? sumY_ : sumX_;
		if (!visible_ && std::fabs(axisSum) >= s.scrollMinAmount) {
			visible_ = true;
			step.becameVisible = true;
		}
		step.visible = visible_;
		step.dir = dirFromSums();
		step.gestureId = id_;
		return step;
	}

	bool ScrollGesture::expire(double now, const CoreSettings & s) {
		if (active_ && now - last_ > s.scrollGap) {
			const bool wasVisible = visible_;
			active_ = false;
			visible_ = false;
			return wasVisible;
		}
		return false;
	}

	void ScrollGesture::reset() {
		active_ = false;
		visible_ = false;
		sumX_ = sumY_ = 0.0;
		last_ = 0.0;
	}

} // namespace core
} // namespace keycaps
} // namespace ofx
