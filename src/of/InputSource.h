#pragma once

// InputSource — the reserved slot for where key and mouse events come from
// (KEYCAPS.md 6.7, D-001). OfInputSource is the v1 implementation; a later
// MacGlobalInputSource can fill the same slot without touching the core
// or the renderer.

#include "../core/KeyStroke.h"
#include "../core/MouseAction.h"
#include <functional>

namespace ofx {
namespace keycaps {

	class InputSource {
	public:
		using KeyHandler = std::function<void(const core::KeyStroke &)>;
		using MouseHandler = std::function<void(const core::MouseAction &)>;

		virtual ~InputSource() = default;

		// Begin delivering events to the handlers.
		virtual void start(KeyHandler onKey, MouseHandler onMouse) = 0;

		// Stop delivering events and remove every listener.
		virtual void stop() = 0;
	};

} // namespace keycaps
} // namespace ofx
