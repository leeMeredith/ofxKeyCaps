#pragma once

// CoreSettings — the part of Style that the core needs (KEYCAPS.md 8).
// Plain values with the spec's defaults. The oF-side Style holds one of
// these, so the core never sees an oF type (D-023).

namespace ofx {
namespace keycaps {
namespace core {

	enum class LabelMode {
		Symbols, // ⌃ ⌥ ⇧ ⌘ ↩ ...
		Text, // Ctrl+Alt+Shift+Super+, Return ... (Linux, Windows)
		MacText, // Ctrl+Opt+Shift+Cmd+, Return ... (macOS without the symbols, D-031)
	};

	struct CoreSettings {
		// Timing, seconds.
		double mergeWindow = 1.0;
		double repeatWindow = 1.0;
		double holdTime = 1.5;
		double fadeTime = 0.5;
		double scrollGap = 0.3;

		// Limits.
		int maxCaps = 6;
		int maxTextChars = 24;

		// Mouse thresholds.
		float dragThreshold = 5.f; // logical points
		float scrollMinAmount = 1.f;

		// Behavior.
		LabelMode labelMode = LabelMode::Symbols;
		bool showModifierOnly = false;
		bool showMouse = false;
	};

} // namespace core
} // namespace keycaps
} // namespace ofx
