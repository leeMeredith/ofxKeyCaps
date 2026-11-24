// Headless tests for src/core (KEYCAPS.md section 10).
// Builds with no openFrameworks. Prints one GREEN or RED line per suite,
// then a suites summary, like the ofxManifold tests.

#include "CapHistory.h"
#include "CapLabeler.h"
#include "EventLog.h"
#include "Hotkey.h"
#include "RawInput.h"
#include "ScrollGesture.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "data/probe_keys.inc"
#include "data/probe_pass1_scroll.inc"

using namespace ofx::keycaps::core;

//--------------------------------------------------------------
// A small test harness
//--------------------------------------------------------------
struct Ctx {
	int checks = 0;
	int failures = 0;
	std::vector<std::string> messages;

	void check(bool ok, const std::string & what, const char * file, int line) {
		++checks;
		if (!ok) {
			++failures;
			if (messages.size() < 8) {
				messages.push_back(std::string(file) + ":" + std::to_string(line) + "  " + what);
			}
		}
	}
	void eq(const std::string & got, const std::string & want, const char * expr, const char * file, int line) {
		check(got == want, std::string(expr) + "  got \"" + got + "\"  want \"" + want + "\"", file, line);
	}
	void eq(long long got, long long want, const char * expr, const char * file, int line) {
		check(got == want, std::string(expr) + "  got " + std::to_string(got) + "  want " + std::to_string(want), file, line);
	}
};

#define CHECK(c) ctx.check((c), #c, __FILE__, __LINE__)
#define CHECK_STR(a, b) ctx.eq(std::string(a), std::string(b), #a, __FILE__, __LINE__)
#define CHECK_INT(a, b) ctx.eq((long long)(a), (long long)(b), #a, __FILE__, __LINE__)

// Short names for the symbols, as UTF-8.
static const std::string CTRL = "\xE2\x8C\x83";
static const std::string ALT = "\xE2\x8C\xA5";
static const std::string SHIFT = "\xE2\x87\xA7";
static const std::string CMD = "\xE2\x8C\x98";
static const std::string CAPS = "\xE2\x87\xAA";
static const std::string RET = "\xE2\x86\xA9";
static const std::string TAB = "\xE2\x87\xA5";
static const std::string BKSP = "\xE2\x8C\xAB";
static const std::string DEL = "\xE2\x8C\xA6";
static const std::string ESC = "\xE2\x8E\x8B";
static const std::string LEFT = "\xE2\x86\x90";
static const std::string RIGHT = "\xE2\x86\x92";
static const std::string UP = "\xE2\x86\x91";
static const std::string DOWN = "\xE2\x86\x93";
static const std::string TIMES = "\xC3\x97";
static const std::string BULLET = "\xE2\x80\xA2";
static const std::string DOT = "\xE2\x97\x8F";

static KeyStroke ch(char c, uint8_t mods = ModNone) { return KeyStroke::character((uint32_t)(unsigned char)c, mods); }
static KeyStroke sp(SpecialKey s, uint8_t mods = ModNone) { return KeyStroke::specialKey(s, mods); }

static CoreSettings mouseOn() {
	CoreSettings s;
	s.showMouse = true;
	return s;
}

static std::string shown(const CapHistory & h, size_t i) {
	return i < h.caps().size() ? h.displayText(h.caps()[i]) : std::string("<none>");
}

//--------------------------------------------------------------
// Labeler
//--------------------------------------------------------------
static void suiteLabeler(Ctx & ctx) {
	const LabelMode S = LabelMode::Symbols;
	const LabelMode T = LabelMode::Text;

	// Modifier order ⌃ ⌥ ⇧ ⌘.
	CHECK_STR(CapLabeler::modifierPrefix(ModCmd | ModShift | ModAlt | ModCtrl, S), CTRL + ALT + SHIFT + CMD);
	CHECK_STR(CapLabeler::modifierPrefix(ModCmd | ModCtrl, T), "Ctrl+Super+");

	// Mac text mode: the fallback on macOS when no font has the symbols (D-031).
	const LabelMode M = LabelMode::MacText;
	CHECK_STR(CapLabeler::modifierPrefix(ModCmd | ModShift | ModAlt | ModCtrl, M), "Ctrl+Opt+Shift+Cmd+");
	CHECK_STR(CapLabeler::keyLabel(ch('x', ModCmd), M), "Cmd+X");
	CHECK_STR(CapLabeler::keyLabel(ch('k', ModAlt), M), "Opt+K");
	CHECK_STR(CapLabeler::keyLabel(KeyStroke::modifierOnly(ModifierKey::Cmd), M), "Cmd");
	CHECK_STR(CapLabeler::keyLabel(KeyStroke::modifierOnly(ModifierKey::Alt), M), "Opt");
	CHECK_STR(CapLabeler::keyLabel(sp(SpecialKey::Return), M), "Return");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Left, 0, ModCmd, false, M), "Cmd+Click");
	CHECK_STR(CapLabeler::countSuffix(3, M), "x3");
	CHECK_STR(CapLabeler::scrollLabel(ScrollDir::Right, M), "Scroll Right");

	// Shift rules.
	CHECK_STR(CapLabeler::keyLabel(ch('A', ModShift), S), "A");
	CHECK_STR(CapLabeler::keyLabel(ch('!', ModShift), S), "!");
	CHECK_STR(CapLabeler::keyLabel(ch('z', ModCmd | ModShift), S), SHIFT + CMD + "Z");
	CHECK_STR(CapLabeler::keyLabel(sp(SpecialKey::Tab, ModShift), S), SHIFT + TAB);
	CHECK_STR(CapLabeler::keyLabel(sp(SpecialKey::Down, ModShift), S), SHIFT + DOWN);

	// Uppercase letters in combos; Option and Ctrl show the key (D-016).
	CHECK_STR(CapLabeler::keyLabel(ch('s', ModCmd), S), CMD + "S");
	CHECK_STR(CapLabeler::keyLabel(ch('a', ModAlt), S), ALT + "A");
	CHECK_STR(CapLabeler::keyLabel(ch('a', ModCtrl), S), CTRL + "A");
	CHECK_STR(CapLabeler::keyLabel(ch('a', ModCtrl), T), "Ctrl+A");
	CHECK_STR(CapLabeler::keyLabel(ch('1', ModCmd), S), CMD + "1");

	// Every special key, both modes.
	struct Sp { SpecialKey k; std::string sym; std::string txt; };
	const Sp specials[] = {
		{ SpecialKey::Return, RET, "Return" }, { SpecialKey::Tab, TAB, "Tab" },
		{ SpecialKey::Backspace, BKSP, "Bksp" }, { SpecialKey::Delete, DEL, "Del" },
		{ SpecialKey::Escape, ESC, "Esc" }, { SpecialKey::Left, LEFT, "Left" },
		{ SpecialKey::Right, RIGHT, "Right" }, { SpecialKey::Up, UP, "Up" },
		{ SpecialKey::Down, DOWN, "Down" }, { SpecialKey::Space, "Space", "Space" },
		{ SpecialKey::Home, "Home", "Home" }, { SpecialKey::End, "End", "End" },
		{ SpecialKey::PageUp, "PgUp", "PgUp" }, { SpecialKey::PageDown, "PgDn", "PgDn" },
		{ SpecialKey::F1, "F1", "F1" }, { SpecialKey::F12, "F12", "F12" },
	};
	for (const Sp & s : specials) {
		CHECK_STR(CapLabeler::keyLabel(sp(s.k), S), s.sym);
		CHECK_STR(CapLabeler::keyLabel(sp(s.k), T), s.txt);
	}

	// Modifier-only caps, including Caps Lock (D-017).
	CHECK_STR(CapLabeler::keyLabel(KeyStroke::modifierOnly(ModifierKey::Shift), S), SHIFT);
	CHECK_STR(CapLabeler::keyLabel(KeyStroke::modifierOnly(ModifierKey::CapsLock), S), CAPS);
	CHECK_STR(CapLabeler::keyLabel(KeyStroke::modifierOnly(ModifierKey::Cmd), T), "Super");

	// Mouse labels.
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Left, 0, ModNone, false, S), "Click");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Left, 0, ModShift, false, S), SHIFT + "Click");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Left, 0, ModCmd, true, S), CMD + "Drag");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Right, 0, ModCmd, false, S), CMD + "Right-click");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Middle, 0, ModNone, true, S), "Middle-drag");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Other, 4, ModNone, false, S), "Button 4");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Other, 5, ModNone, true, S), "Button 5 drag");

	// D-021: right hides ⌃, middle hides ⌥.
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Right, 0, ModCtrl, false, S), "Right-click");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Right, 0, ModCtrl | ModAlt | ModShift, false, S), ALT + SHIFT + "Right-click");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Middle, 0, ModAlt, false, S), "Middle-click");
	CHECK_STR(CapLabeler::mouseLabel(MouseButton::Left, 0, ModCtrl, false, S), CTRL + "Click");

	// Scroll directions (D-018, D-022).
	CHECK_STR(CapLabeler::scrollLabel(ScrollDir::Down, S), "Scroll " + DOWN);
	CHECK_STR(CapLabeler::scrollLabel(ScrollDir::Left, T), "Scroll Left");

	// Counts, mark, mask.
	CHECK_STR(CapLabeler::countSuffix(1, S), "");
	CHECK_STR(CapLabeler::countSuffix(3, S), TIMES + "3");
	CHECK_STR(CapLabeler::countSuffix(3, T), "x3");
	CHECK_STR(CapLabeler::markLabel(S), DOT + " MARK");
	CHECK_STR(CapLabeler::maskChar(S), BULLET);

	// UTF-8.
	std::string a;
	CapLabeler::appendUtf8(a, 0xE5); // å
	CHECK_STR(a, "\xC3\xA5");
	CHECK_INT(CapLabeler::codepointCount("h" + BULLET + "llo"), 5);
}

//--------------------------------------------------------------
// Merge (R3, R4, R7, R8)
//--------------------------------------------------------------
static void suiteMerge(Ctx & ctx) {
	{
		CapHistory h;
		double t = 0;
		for (char c : std::string("hello")) h.push(ch(c), t += 0.1);
		CHECK_INT(h.caps().size(), 1);
		CHECK_STR(shown(h, 0), "hello");
		CHECK(h.caps()[0].kind == CapKind::Text);
	}
	{
		// A gap longer than mergeWindow splits; exactly at the window splits too.
		CapHistory h;
		h.push(ch('a'), 0.0);
		h.push(ch('b'), 0.99);
		h.push(ch('c'), 0.99 + 1.0);
		CHECK_INT(h.caps().size(), 2);
		CHECK_STR(shown(h, 0), "ab");
		CHECK_STR(shown(h, 1), "c");
	}
	{
		// Cmd+S breaks a text run.
		CapHistory h;
		h.push(ch('h'), 0.0);
		h.push(ch('i'), 0.1);
		h.push(ch('s', ModCmd), 0.2);
		h.push(ch('x'), 0.3);
		CHECK_INT(h.caps().size(), 3);
		CHECK_STR(shown(h, 1), CMD + "S");
		CHECK_STR(shown(h, 2), "x");
	}
	{
		// Shift-typed characters stay in the run.
		CapHistory h;
		h.push(ch('H', ModShift), 0.0);
		h.push(ch('i'), 0.1);
		h.push(ch('!', ModShift), 0.2);
		CHECK_STR(shown(h, 0), "Hi!");
	}
	{
		// R4: Space inside text is a space; Space starting a cap is "Space".
		CapHistory h;
		h.push(ch('a'), 0.0);
		h.push(sp(SpecialKey::Space), 0.1);
		h.push(ch('b'), 0.2);
		CHECK_STR(shown(h, 0), "a b");
		h.push(sp(SpecialKey::Space), 5.0);
		CHECK_INT(h.caps().size(), 1);
		CHECK_STR(shown(h, 0), "Space");
		CHECK(h.caps()[0].kind == CapKind::Single);
	}
	{
		// R7: Backspace never edits a text cap.
		CapHistory h;
		h.push(ch('a'), 0.0);
		h.push(ch('b'), 0.1);
		h.push(sp(SpecialKey::Backspace), 0.2);
		CHECK_INT(h.caps().size(), 2);
		CHECK_STR(shown(h, 0), "ab");
		CHECK_STR(shown(h, 1), BKSP);
	}
	{
		// R8: overflow at 24 characters.
		CoreSettings s;
		s.maxCaps = 10;
		CapHistory h(s);
		for (int i = 0; i < 30; ++i) h.push(ch('x'), 0.01 * i);
		CHECK_INT(h.caps().size(), 2);
		CHECK_INT(CapLabeler::codepointCount(h.caps()[0].label), 24);
		CHECK_INT(CapLabeler::codepointCount(h.caps()[1].label), 6);
	}
	{
		// Option+a is a combo cap, not typed text (D-016).
		CapHistory h;
		h.push(ch('a', ModAlt), 0.0);
		CHECK_STR(shown(h, 0), ALT + "A");
		CHECK(h.caps()[0].kind == CapKind::Single);
	}
}

//--------------------------------------------------------------
// Repeat (R5, R6)
//--------------------------------------------------------------
static void suiteRepeat(Ctx & ctx) {
	{
		CapHistory h;
		h.push(ch('z', ModCmd), 0.0);
		h.push(ch('z', ModCmd), 0.4);
		h.push(ch('z', ModCmd), 0.8);
		CHECK_INT(h.caps().size(), 1);
		CHECK_STR(shown(h, 0), CMD + "Z " + TIMES + "3");
	}
	{
		// A gap longer than repeatWindow makes a new cap.
		CapHistory h;
		h.push(ch('z', ModCmd), 0.0);
		h.push(ch('z', ModCmd), 1.2);
		CHECK_INT(h.caps().size(), 2);
		CHECK_STR(shown(h, 1), CMD + "Z");
	}
	{
		// A different combo is a new cap.
		CapHistory h;
		h.push(ch('z', ModCmd), 0.0);
		h.push(ch('z', ModCmd | ModShift), 0.1);
		CHECK_INT(h.caps().size(), 2);
	}
	{
		// R6: held key counts, never repeats characters.
		CapHistory h;
		KeyStroke a = ch('a');
		h.push(a, 0.0);
		for (int i = 0; i < 11; ++i) h.push(a.repeated(), 0.5 + 0.083 * i);
		CHECK_INT(h.caps().size(), 1);
		CHECK_STR(shown(h, 0), "a " + TIMES + "12");
	}
	{
		// R6 on a combo: held ⌘Z counts on the ⌘Z cap.
		CapHistory h;
		KeyStroke z = ch('z', ModCmd);
		h.push(z, 0.0);
		h.push(z.repeated(), 0.5);
		CHECK_STR(shown(h, 0), CMD + "Z " + TIMES + "2");
	}
	{
		// Special keys repeat-count too: ↓ ↓ ↓.
		CapHistory h;
		h.push(sp(SpecialKey::Down), 0.0);
		h.push(sp(SpecialKey::Down), 0.2);
		h.push(sp(SpecialKey::Down), 0.4);
		CHECK_STR(shown(h, 0), DOWN + " " + TIMES + "3");
	}
}

//--------------------------------------------------------------
// Mouse (M0–M5, M7, R9 with held caps, D-021)
//--------------------------------------------------------------
static void suiteMouse(Ctx & ctx) {
	{
		// M0: dropped when off.
		CapHistory h;
		h.push(MouseAction::down(MouseButton::Left), 0.0);
		h.push(MouseAction::scroll(0, -5), 0.1);
		CHECK_INT(h.caps().size(), 0);
	}
	{
		// M1–M3: held while down, ages from the release.
		CapHistory h(mouseOn());
		h.push(MouseAction::down(MouseButton::Left), 0.0);
		h.update(5.0);
		CHECK_INT(h.caps().size(), 1);
		CHECK(h.caps()[0].held);
		CHECK(h.caps()[0].opacity == 1.f);
		h.push(MouseAction::up(MouseButton::Left), 5.0);
		CHECK(!h.caps()[0].held);
		h.update(5.0 + 1.5);
		CHECK(h.caps().size() == 1 && h.caps()[0].opacity == 1.f);
		h.update(5.0 + 2.0);
		CHECK_INT(h.caps().size(), 0);
	}
	{
		// M4: drag relabels once, at the threshold, never on click jitter.
		CapHistory h(mouseOn());
		h.push(MouseAction::down(MouseButton::Left), 0.0);
		h.push(MouseAction::drag(MouseButton::Left, 1.0f), 0.02); // recorded jitter
		h.push(MouseAction::drag(MouseButton::Left, 2.0f), 0.04);
		h.push(MouseAction::drag(MouseButton::Left, 2.0f), 0.06);
		h.push(MouseAction::drag(MouseButton::Left, 1.5f), 0.08);
		CHECK_STR(shown(h, 0), "Click");
		h.push(MouseAction::drag(MouseButton::Left, 4.99f), 0.10);
		CHECK_STR(shown(h, 0), "Click");
		h.push(MouseAction::drag(MouseButton::Left, 5.0f), 0.12);
		CHECK_STR(shown(h, 0), "Drag");
		h.push(MouseAction::drag(MouseButton::Left, 80.0f), 0.30);
		CHECK_STR(shown(h, 0), "Drag");
		CHECK_INT(h.caps().size(), 1);
	}
	{
		// Recorded pass-3 clicks: Cmd+click with one 1 pt drag event stays a click.
		CapHistory h(mouseOn());
		h.push(MouseAction::down(MouseButton::Left, ModCmd), 68.451);
		h.push(MouseAction::drag(MouseButton::Left, 1.0f, ModCmd), 68.451);
		h.push(MouseAction::up(MouseButton::Left, ModCmd), 68.700);
		CHECK_STR(shown(h, 0), CMD + "Click");
	}
	{
		// M5: repeated clicks count; a click after a drag does not.
		CapHistory h(mouseOn());
		double t = 0;
		for (int i = 0; i < 3; ++i) {
			h.push(MouseAction::down(MouseButton::Left), t);
			h.push(MouseAction::up(MouseButton::Left), t + 0.15);
			t += 0.35;
		}
		CHECK_INT(h.caps().size(), 1);
		CHECK_STR(shown(h, 0), "Click " + TIMES + "3");
		h.push(MouseAction::down(MouseButton::Left), 5.0);
		h.push(MouseAction::drag(MouseButton::Left, 50.f), 5.1);
		h.push(MouseAction::up(MouseButton::Left), 5.3);
		h.push(MouseAction::down(MouseButton::Left), 5.5);
		CHECK_STR(shown(h, h.caps().size() - 1), "Click");
		CHECK_STR(shown(h, h.caps().size() - 2), "Drag");
	}
	{
		// M7: a missed release is released by the next Down.
		CapHistory h(mouseOn());
		h.push(MouseAction::down(MouseButton::Right), 0.0);
		h.push(MouseAction::down(MouseButton::Right), 3.0);
		int held = 0;
		for (const Cap & c : h.caps()) held += c.held ? 1 : 0;
		CHECK_INT(held, 1);
	}
	{
		// D-021 through the history: Ctrl+click arrives as right.
		CapHistory h(mouseOn());
		h.push(MouseAction::down(MouseButton::Right, ModCtrl | ModAlt | ModShift), 0.0);
		CHECK_STR(shown(h, 0), ALT + SHIFT + "Right-click");
	}
	{
		// R9: held caps survive capacity.
		CoreSettings s = mouseOn();
		s.maxCaps = 3;
		CapHistory h(s);
		h.push(MouseAction::down(MouseButton::Left), 0.0);
		h.push(ch('a', ModCmd), 0.1);
		h.push(ch('b', ModCmd), 0.2);
		h.push(ch('c', ModCmd), 0.3);
		CHECK_INT(h.caps().size(), 3);
		CHECK(h.caps()[0].kind == CapKind::Mouse && h.caps()[0].held);
		CHECK_STR(shown(h, 1), CMD + "B");
	}
	{
		// R9 when every cap is held: the oldest goes, the new press stays held.
		CoreSettings s = mouseOn();
		s.maxCaps = 3;
		CapHistory h(s);
		h.push(MouseAction::down(MouseButton::Left), 0.0);
		h.push(MouseAction::down(MouseButton::Right), 0.1);
		h.push(MouseAction::down(MouseButton::Middle), 0.2);
		h.push(MouseAction::otherButton(4, MousePhase::Down), 0.3);
		CHECK_INT(h.caps().size(), 3);
		CHECK_STR(shown(h, 0), "Right-click");
		CHECK_STR(shown(h, 2), "Button 4");
		CHECK(h.caps()[2].held && h.caps()[2].button == MouseButton::Other);
	}
}

//--------------------------------------------------------------
// Scroll (M6), replayed from the recorded probe data
//--------------------------------------------------------------
static void suiteScroll(Ctx & ctx) {
	{
		// Every recorded swipe makes exactly the gesture the reference expects.
		ScrollGesture g;
		CoreSettings s;
		std::vector<std::pair<double, ScrollDir>> visible;
		std::vector<double> starts;
		double start = 0;
		int lastId = 0;
		for (const RecordedScroll & e : kPass1Scroll) {
			ScrollStep st = g.feed(e.x, e.y, e.t, s);
			if (st.gestureId != lastId) {
				start = e.t;
				lastId = st.gestureId;
			}
			if (st.becameVisible) {
				visible.push_back({ e.t, st.dir });
				starts.push_back(start);
			}
		}
		const size_t n = sizeof(kPass1Expected) / sizeof(kPass1Expected[0]);
		CHECK_INT(visible.size(), n);
		for (size_t i = 0; i < n && i < visible.size(); ++i) {
			const ExpectedGesture & x = kPass1Expected[i];
			CHECK(std::fabs(starts[i] - x.start) < 1e-6);
			CHECK(std::fabs(visible[i].first - x.visibleAt) < 0.02);
			const char * d = visible[i].second == ScrollDir::Up ? "Up"
				: visible[i].second == ScrollDir::Down ? "Down"
				: visible[i].second == ScrollDir::Left ? "Left" : "Right";
			CHECK_STR(d, x.dir);
		}
	}
	{
		// Through CapHistory: one cap per swipe direction run, counted.
		CapHistory h(mouseOn());
		for (const RecordedScroll & e : kPass1Scroll) h.push(MouseAction::scroll(e.x, e.y), e.t);
		h.update(200.0);
		CHECK_INT(h.caps().size(), 0); // all faded by then
	}
	{
		// Gaps just under and just over scrollGap.
		ScrollGesture g;
		CoreSettings s;
		g.feed(0, -0.6f, 0.0, s);
		ScrollStep a = g.feed(0, -0.6f, 0.29, s);
		CHECK_INT(a.gestureId, 1);
		ScrollStep b = g.feed(0, -0.6f, 0.29 + 0.31, s);
		CHECK_INT(b.gestureId, 2);
	}
	{
		// A sign reversal splits a gesture.
		ScrollGesture g;
		CoreSettings s;
		g.feed(0, -0.8f, 0.0, s);
		g.feed(0, -0.8f, 0.017, s);
		ScrollStep r = g.feed(0, 0.5f, 0.034, s);
		CHECK_INT(r.gestureId, 2);
		CHECK(r.endedPrevious);
	}
	{
		// Three swipes the same way make Scroll ↓ ×3; jitter never changes direction.
		CapHistory h(mouseOn());
		double t = 0;
		for (int swipe = 0; swipe < 3; ++swipe) {
			for (int i = 0; i < 40; ++i) {
				const float jitter = (i % 2) ? 0.2f : -0.2f;
				h.push(MouseAction::scroll(jitter, -0.5f), t);
				t += 0.017;
			}
			t += 0.5; // past scrollGap, within repeatWindow
			h.update(t);
		}
		CHECK_INT(h.caps().size(), 1);
		CHECK_STR(shown(h, 0), "Scroll " + DOWN + " " + TIMES + "3");
	}
	{
		// The recorded brush (3 events, sum 0.3) makes nothing.
		CapHistory h(mouseOn());
		h.push(MouseAction::scroll(0, 0.1f), 18.284);
		h.push(MouseAction::scroll(0, 0.1f), 18.300);
		h.push(MouseAction::scroll(0, 0.1f), 18.317);
		h.update(19.0);
		CHECK_INT(h.caps().size(), 0);
	}
	{
		// Horizontal: negative X is Scroll → (D-022). Recorded sums -2.8 and +39.6.
		CapHistory h(mouseOn());
		for (int i = 0; i < 19; ++i) h.push(MouseAction::scroll(-0.147f, 0), 7.334 + 0.015 * i);
		CHECK_STR(shown(h, 0), "Scroll " + RIGHT);
		for (int i = 0; i < 38; ++i) h.push(MouseAction::scroll(1.04f, 0.07f), 20.0 + 0.016 * i);
		CHECK_STR(shown(h, h.caps().size() - 1), "Scroll " + LEFT);
	}
	{
		// A gesture holds its cap until it ends, then ages from its last event.
		CapHistory h(mouseOn());
		for (int i = 0; i < 30; ++i) h.push(MouseAction::scroll(0, -0.5f), 0.017 * i);
		CHECK(h.caps().size() == 1 && h.caps()[0].held);
		h.update(0.017 * 29 + 0.31);
		CHECK(!h.caps()[0].held);
		CHECK(std::fabs(h.caps()[0].changedAt - 0.017 * 29) < 1e-9);
	}
}

//--------------------------------------------------------------
// Lifetime (R10)
//--------------------------------------------------------------
static void suiteLifetime(Ctx & ctx) {
	{
		CapHistory h;
		h.push(ch('s', ModCmd), 10.0);
		h.update(10.0 + 1.5);
		CHECK(h.caps().size() == 1 && h.caps()[0].opacity == 1.f);
		h.update(10.0 + 1.75);
		CHECK(h.caps().size() == 1 && std::fabs(h.caps()[0].opacity - 0.5f) < 1e-4);
		h.update(10.0 + 1.9999);
		CHECK_INT(h.caps().size(), 1);
		h.update(10.0 + 2.0);
		CHECK_INT(h.caps().size(), 0);
	}
	{
		// Age resets when a cap changes.
		CapHistory h;
		h.push(ch('z', ModCmd), 0.0);
		h.push(ch('z', ModCmd), 0.9);
		h.update(0.9 + 1.6);
		CHECK(h.caps().size() == 1 && h.caps()[0].opacity < 1.f && h.caps()[0].opacity > 0.f);
	}
	{
		// Same result whether time arrives as 1×1.0 s or 100×0.01 s.
		CapHistory a, b;
		a.push(ch('q', ModCmd), 0.0);
		b.push(ch('q', ModCmd), 0.0);
		a.update(1.7);
		for (int i = 1; i <= 170; ++i) b.update(0.01 * i);
		CHECK(a.caps().size() == 1 && b.caps().size() == 1);
		if (a.caps().size() == 1 && b.caps().size() == 1) {
			CHECK(std::fabs(a.caps()[0].opacity - b.caps()[0].opacity) < 1e-4);
		}
	}
	{
		// Mark caps live like any other cap.
		CapHistory h;
		h.pushMark(0.0);
		CHECK(h.caps().size() == 1 && h.caps()[0].isMark());
		CHECK_STR(shown(h, 0), DOT + " MARK");
		h.update(2.0);
		CHECK_INT(h.caps().size(), 0);
	}
}

//--------------------------------------------------------------
// Capacity (R9)
//--------------------------------------------------------------
static void suiteCapacity(Ctx & ctx) {
	CapHistory h;
	const char keys[] = "abcdefgh";
	for (int i = 0; i < 8; ++i) h.push(ch(keys[i], ModCmd), 0.1 * i);
	CHECK_INT(h.caps().size(), 6);
	CHECK_STR(shown(h, 0), CMD + "C");
	CHECK_STR(shown(h, 5), CMD + "H");
	// Ids are unique and increasing, for the renderer's animation.
	for (size_t i = 1; i < h.caps().size(); ++i) CHECK(h.caps()[i].id > h.caps()[i - 1].id);
}

//--------------------------------------------------------------
// Mask (R2, M8, masked log lines)
//--------------------------------------------------------------
static void suiteMask(Ctx & ctx) {
	CapHistory h(mouseOn());
	h.push(ch('o'), 0.0);
	h.setMasked(true);
	h.push(ch('k'), 0.1);
	h.push(ch('!', ModShift), 0.2);
	CHECK_STR(shown(h, 0), "o" + BULLET + BULLET); // existing text kept, new masked
	h.push(ch('v', ModCmd), 0.3);
	CHECK_STR(shown(h, 1), CMD + "V"); // single caps still shown
	h.push(MouseAction::down(MouseButton::Left), 0.4);
	CHECK_STR(shown(h, 2), "Click"); // M8

	EventLog log(mouseOn());
	log.setMasked(true);
	std::string line;
	CHECK(log.log(ch('p'), 1.0, line));
	CHECK_STR(line, "00:00:01.000  " + BULLET);
	CHECK(log.log(ch('c', ModCmd), 1.5, line));
	CHECK_STR(line, "00:00:01.500  " + CMD + "C");
}

//--------------------------------------------------------------
// Log (EventLog, D-009, D-014)
//--------------------------------------------------------------
static void suiteLog(Ctx & ctx) {
	CHECK_STR(EventLog::formatTime(0.0), "00:00:00.000");
	CHECK_STR(EventLog::formatTime(0.0005), "00:00:00.001");
	CHECK_STR(EventLog::formatTime(2.481), "00:00:02.481");
	CHECK_STR(EventLog::formatTime(3725.5), "01:02:05.500");
	CHECK_STR(EventLog::formatTime(-1.0), "00:00:00.000");

	EventLog log;
	std::string line;
	// Zero is the origin of `now` before the first mark.
	CHECK(log.log(ch('h'), 3.020, line));
	CHECK_STR(line, "00:00:03.020  h");
	// A mark resets the zero.
	CHECK_STR(log.mark(10.0), "00:00:00.000  " + DOT + " MARK");
	CHECK(log.log(ch('s', ModCmd), 12.481, line));
	CHECK_STR(line, "00:00:02.481  " + CMD + "S");
	// One line per press, the press's own label, not the merged text.
	CHECK(log.log(ch('e'), 13.105, line));
	CHECK_STR(line, "00:00:03.105  e");
	// No line for auto-repeats.
	CHECK(!log.log(ch('e').repeated(), 13.6, line));
	// Modifier-only presses follow showModifierOnly.
	CHECK(!log.log(KeyStroke::modifierOnly(ModifierKey::Shift), 14.0, line));
	// Mouse lines only when showMouse is on.
	CHECK(!log.log(MouseAction::down(MouseButton::Left), 14.0, line));

	EventLog m(mouseOn());
	m.mark(0.0);
	CHECK(m.log(MouseAction::down(MouseButton::Left), 5.300, line));
	CHECK_STR(line, "00:00:05.300  Click down");
	CHECK(!m.log(MouseAction::drag(MouseButton::Left, 2.f), 5.31, line)); // jitter
	CHECK(m.log(MouseAction::drag(MouseButton::Left, 6.f), 5.33, line));
	CHECK_STR(line, "00:00:05.330  Drag start");
	CHECK(!m.log(MouseAction::drag(MouseButton::Left, 60.f), 5.40, line));
	CHECK(m.log(MouseAction::up(MouseButton::Left), 5.410, line));
	CHECK_STR(line, "00:00:05.410  Click up");

	// One scroll line per gesture, when it crosses scrollMinAmount.
	int lines = 0;
	for (int i = 0; i < 40; ++i) {
		if (m.log(MouseAction::scroll(0, -0.4f), 6.0 + 0.017 * i, line)) {
			++lines;
			CHECK_STR(line, "00:00:06.034  Scroll " + DOWN);
		}
	}
	CHECK_INT(lines, 1);
	// A note line, used for (hidden) and (shown).
	CHECK_STR(m.note(7.5, "(hidden)"), "00:00:07.500  (hidden)");
	// The brush logs nothing.
	CHECK(!m.log(MouseAction::scroll(0, 0.1f), 9.0, line));
	CHECK(!m.log(MouseAction::scroll(0, 0.1f), 9.017, line));
	CHECK(!m.log(MouseAction::scroll(0, 0.1f), 9.034, line));
}

//--------------------------------------------------------------
// Keys (RawInput, KEYCAPS.md 6.8), replayed from the recorded probe data
//--------------------------------------------------------------
static std::string translated(int key, int keycode, int mods) {
	RawKey r;
	r.key = key;
	r.keycode = keycode;
	r.modifiers = mods;
	KeyStroke k;
	if (!translateKey(r, k)) return "DROP";
	return CapLabeler::keyLabel(k, LabelMode::Symbols);
}

static void suiteKeys(Ctx & ctx) {
	// Every recorded key press gets the label the reference model expects.
	int mismatches = 0;
	for (const RecordedKey & e : kProbeKeys) {
		RawKey r;
		r.key = e.key;
		r.keycode = e.keycode;
		r.modifiers = e.modifiers;
		r.isRepeat = e.isRepeat;
		KeyStroke k;
		const std::string got = translateKey(r, k) ? CapLabeler::keyLabel(k, LabelMode::Symbols) : "DROP";
		if (got != e.label) {
			++mismatches;
			CHECK_STR(got, e.label);
		}
		if (got != "DROP") CHECK(k.isRepeat == e.isRepeat);
	}
	CHECK_INT(mismatches, 0);

	// Ctrl turns letters into control codes that collide with specials.
	CHECK_STR(translated(8, 'H', raw::ModControl), CTRL + "H"); // not Backspace
	CHECK_STR(translated(9, 'I', raw::ModControl), CTRL + "I"); // not Tab
	CHECK_STR(translated(13, 'M', raw::ModControl), CTRL + "M"); // not Return
	CHECK_STR(translated(27, '[', raw::ModControl), CTRL + "["); // not Esc
	// Option composes; the key pressed is shown.
	CHECK_STR(translated(0xE5, 'A', raw::ModAlt), ALT + "A");
	CHECK_STR(translated(0xE9, 'E', raw::ModAlt | raw::ModShift), ALT + SHIFT + "E");
	// Modifier presses: only the left/right event counts.
	CHECK_STR(translated(raw::ModShift, raw::CodeLeftShift, raw::ModShift), "DROP");
	CHECK_STR(translated(0xe61, raw::CodeRightShift, raw::ModShift), SHIFT);
	CHECK_STR(translated(0xe67, raw::CodeRightSuper, raw::ModSuper), CMD);
	// Caps Lock, Fn, Insert, keypad Enter.
	CHECK_STR(translated('<', raw::CodeCapsLock, 0), CAPS);
	CHECK_STR(translated(0xE5, -1, 0), "DROP");
	CHECK_STR(translated(0xe014, raw::CodeInsert, 0), "DROP");
	CHECK_STR(translated(13, raw::CodeKeypadEnter, 0), RET);
	// Shift on specials, Cmd with a layout character.
	CHECK_STR(translated(9, raw::CodeTab, raw::ModShift), SHIFT + TAB);
	CHECK_STR(translated('z', 'Z', raw::ModSuper | raw::ModShift), SHIFT + CMD + "Z");
	CHECK_STR(translated(127, raw::CodeDelete, 0), DEL);
	CHECK_STR(translated(0xe00B, raw::CodeF12, 0), "F12");

	// Modifier bits.
	CHECK_INT(translateModifiers(0x17), ModCtrl | ModAlt | ModShift | ModCmd);
	CHECK_INT(translateModifiers(0), ModNone);

	// Mouse buttons: oF 1 is middle, 2 is right, 3..7 are buttons 4..8.
	MouseButton b;
	int n = 0;
	CHECK(translateButton(0, b, n) && b == MouseButton::Left);
	CHECK(translateButton(1, b, n) && b == MouseButton::Middle);
	CHECK(translateButton(2, b, n) && b == MouseButton::Right);
	CHECK(translateButton(3, b, n) && b == MouseButton::Other && n == 4);
	CHECK(translateButton(7, b, n) && b == MouseButton::Other && n == 8);
	CHECK(!translateButton(8, b, n));
	CHECK(!translateButton(-1, b, n));
}

//--------------------------------------------------------------
// Hotkeys (KEYCAPS.md 7, D-034), through the real key translation
//--------------------------------------------------------------
static KeyStroke rawStroke(int key, int keycode, int mods, bool repeat = false) {
	RawKey r;
	r.key = key;
	r.keycode = keycode;
	r.modifiers = mods;
	r.isRepeat = repeat;
	KeyStroke k;
	translateKey(r, k);
	return k;
}

static void suiteHotkeys(Ctx & ctx) {
	const Hotkey mark = Hotkey::character('m', ModCtrl | ModShift); // the default ⌃⇧M
	const Hotkey toggle = Hotkey::character('K', ModCtrl | ModShift); // given uppercase: still ⌃⇧K
	const int ctrlShift = raw::ModControl | raw::ModShift;

	// ⌃⇧M arrives as a control code (Ctrl+M is 13) with keycode 'M'.
	CHECK(matches(mark, rawStroke(13, 'M', ctrlShift)));
	CHECK(matches(toggle, rawStroke(11, 'K', ctrlShift)));
	// Exactly these modifiers: not fewer, not more.
	CHECK(!matches(mark, rawStroke(13, 'M', raw::ModControl)));
	CHECK(!matches(mark, rawStroke(0xB5, 'M', ctrlShift | raw::ModAlt)));
	CHECK(!matches(mark, rawStroke('m', 'M', 0)));
	CHECK(!matches(mark, rawStroke('M', 'M', raw::ModShift)));
	// A different key, and a held key's repeats, never match.
	CHECK(!matches(mark, rawStroke(11, 'K', ctrlShift)));
	CHECK(!matches(mark, rawStroke(13, 'M', ctrlShift, true)));
	// Cmd hotkeys compare the layout character without case.
	CHECK(matches(Hotkey::character('k', ModCmd | ModShift), rawStroke('K', 'K', raw::ModSuper | raw::ModShift)));
	// Special-key hotkeys.
	const Hotkey f5 = Hotkey::specialKey(SpecialKey::F5);
	CHECK(matches(f5, rawStroke(0xe004, raw::CodeF1 + 4, 0)));
	CHECK(!matches(f5, rawStroke(0xe004, raw::CodeF1 + 4, raw::ModShift)));
	// Turned off.
	CHECK(!Hotkey::none().isSet());
	CHECK(!matches(Hotkey::none(), rawStroke(13, 'M', ctrlShift)));
	CHECK(!matches(Hotkey::none(), KeyStroke()));
	// Labels for the setup notice.
	CHECK_STR(CapLabeler::keyLabel(mark.asStroke(), LabelMode::Symbols), CTRL + SHIFT + "M");
	CHECK_STR(CapLabeler::keyLabel(toggle.asStroke(), LabelMode::MacText), "Ctrl+Shift+K");
	CHECK_STR(CapLabeler::keyLabel(f5.asStroke(), LabelMode::Symbols), "F5");
}

//--------------------------------------------------------------
int main() {
	struct Suite {
		const char * name;
		void (*fn)(Ctx &);
	};
	const Suite suites[] = {
		{ "Labeler", suiteLabeler },
		{ "Keys", suiteKeys },
		{ "Merge", suiteMerge },
		{ "Repeat", suiteRepeat },
		{ "Mouse", suiteMouse },
		{ "Scroll", suiteScroll },
		{ "Lifetime", suiteLifetime },
		{ "Capacity", suiteCapacity },
		{ "Mask", suiteMask },
		{ "Log", suiteLog },
		{ "Hotkeys", suiteHotkeys },
	};
	int green = 0, red = 0;
	for (const Suite & s : suites) {
		Ctx ctx;
		s.fn(ctx);
		if (ctx.failures == 0) {
			++green;
			std::printf("GREEN  %-9s %3d checks\n", s.name, ctx.checks);
		} else {
			++red;
			std::printf("RED    %-9s %3d of %d checks failed\n", s.name, ctx.failures, ctx.checks);
			for (const std::string & m : ctx.messages) std::printf("         %s\n", m.c_str());
		}
	}
	std::printf("suites: %d green, %d red\n", green, red);
	return red == 0 ? 0 : 1;
}
