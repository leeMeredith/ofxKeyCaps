# DECISIONS.md — ofxKeyCaps

Decisions and reversals, in order. A reversal keeps the original entry and adds a new one that says what changed and why.

## D-001 — Capture: oF window only (v1)
Keys are captured only while the openFrameworks window has focus, through oF's own key events. This keeps the addon pure C++, permission-free and cross-platform. System-wide capture on macOS is reserved behind the `KeySource` interface (KEYCAPS.md 6.4), so it can be added later without editing the core or the renderer.

## D-002 — Typed letters merge into one cap
Letters typed in quick succession are shown as one cap ("hello") rather than one cap per key. Combos and special keys always get their own cap. The rules are R3–R8 in KEYCAPS.md 6.3.

## D-003 — Default position: bottom center
The row of caps is centered at the bottom of the window, with the newest cap on the right.

## D-004 — Name: ofxKeystrokes (reversed by D-008)
Check ofxaddons.com for an existing addon with this name before publishing.

## D-005 — The core uses its own key and modifier types
`src/core/` defines its own `SpecialKey` list and modifier bitmask instead of using oF constants. OfKeySource translates between them. This keeps the core free of oF headers (principle P3) and lets a future MacGlobalKeySource feed the same core.

## D-006 — The core never reads a clock
Time enters the core only through `push(stroke, now)` and `update(dt)`. Behavior is the same at any frame rate, and tests are deterministic.

## D-007 — Show what was pressed, not the resulting text
Backspace and editing keys get their own caps and never delete characters from a text cap (R7). Held keys add to the count instead of repeating characters (R6).

## D-008 — Renamed to ofxKeyCaps (reverses D-004)
"Keystrokes" names only the input, which every app receives. The addon's job is to show key presses to someone watching, and what appears on screen is a row of keycaps, so the name now describes the visible result. The spec file is renamed from KEYSTROKES.md to KEYCAPS.md to match. No GitHub repository named ofxKeyCaps existed on 2026-11-23. ofxaddons.com could not be reached that day and should be checked before publishing. The core type `KeyStroke` keeps its name, because it still describes one key press.

## D-009 — Timestamped press log, zero set by a sync mark
Every press is printed to the terminal with a timestamp, so a recording can be edited against an exact list of when things happened. The log goes through `ofLogNotice("ofxKeyCaps")`, so oF's own log levels can silence it. Before the first mark, the zero is app launch. `mark()`, or an optional mark key, resets the zero and shows a "● MARK" cap at the same moment, so the mark is visible in the recording and works like a clapperboard. Time-of-day stamps were considered and not chosen, because a screen recording's start time is hard to read off reliably.

## D-010 — Log goes to the terminal only (v1)
No log file in v1. A file writer can be added later beside the terminal output without changing EventLog.

## D-011 — Mouse buttons as caps in the same row, off by default
oF's native mouse events (pressed, released, dragged, scrolled) become caps in the same row as keys, so they share fading, ×N counting and the log. A cap stays highlighted while its button is held and starts fading on release. Dragging relabels the cap once. Scrolling counts events in one cap. Position, cursor highlight and trails are not planned. `Style.showMouse` is false by default.

## D-012 — No double-click detection in v1
oF does not deliver double-clicks natively. Repeated clicks are counted (`Click ×2`) by the existing repeat rule, which is counting, not detection.

## D-013 — Modules renamed to fit keys and mouse
Because mouse events now flow through the same path, KeySource becomes InputSource, OfKeySource becomes OfInputSource, KeyLabeler becomes CapLabeler, KeyHistory becomes CapHistory, and the reserved slot becomes MacGlobalInputSource. `KeyStroke` keeps its name, and `MouseAction` is added beside it. EventLog is a new core module.

## D-014 — One log line per press, repeats collapsed
The log records each press with its own label, not the merged cap text, so every key has its own time. Key auto-repeats and scroll events that only increase a count make no line, which keeps a held key or a trackpad scroll from flooding the terminal. Mouse buttons log down and up separately, so the length of a press can be read from the log.

## D-015 — Key translation rules, from probe passes 1 and 2 (resolves PENDING-1)
Measured on macOS with oF 0.12.1, through `ofEvents()` at `OF_EVENT_ORDER_AFTER_APP`. The rules are in KEYCAPS.md 6.8. What the probe showed:
- Ctrl+a arrives with `key` 0x01 (a control code) and `keycode` 65. Option+a arrives with `key` `å` and `keycode` 65. Cmd+a arrives with `key` `a`. So printable keys pressed with Ctrl, Option or Cmd are identified by `keycode`.
- With no modifier, or Shift only, `key` is already the typed character: `A`, `!`, `|`, `{`, `"`, `?`, `~`.
- Each modifier press and release arrives as two events with the same `keycode`: one carrying the modifier's flag value and one carrying the left or right key code. This held for both sides of Shift, Option and Cmd, and for left Ctrl.
- Caps Lock (`keycode` 280) arrives with an unrelated printable character in `key`.
- The Fn (Globe) key sends a lone key-up with `keycode` -1 and a stale character in `key`.
- A key's release can carry a different `key` from its press: Ctrl+a went down as 0x01 and came up as `a` after Ctrl was let go. The addon does not use key releases.
- `ofGetKeyPressed(OF_KEY_SHIFT)` reported Shift as held for about two and a half minutes after it was let go, while the event's `modifiers` field was correct throughout. The addon reads only the event field.
- `isRepeat` is reliable: first repeat about 0.5 s after the press, then about every 83 ms.
- Return arrives with codepoint 0x0A and Backspace with codepoint 0, so special keys are identified by `key`, not `codepoint`.
- Some shortcuts never reach the app: Cmd+Q (the menu quits first) and F11 (taken by macOS). No cap can show them.

## D-016 — Option and Ctrl combos show the key pressed, not the composed character
Option+a shows `⌥A`, not `å`, and Ctrl+a shows `⌃A`. This follows D-007 (show what was pressed). Someone typing an accented character on purpose sees `⌥E` then `E` rather than `é`, which is accepted for v1.

## D-017 — Caps Lock is a modifier-only key
Caps Lock is ignored by default, like Shift pressed alone (R1), and shows as ⇪ when `showModifierOnly` is on. VoiceOver users often set Caps Lock as the VoiceOver modifier, so it should not make a cap every time it is pressed.

## D-018 — Scroll is counted in gestures, not events (replaces the old M6)
A trackpad swipe arrived as 44 to 73 scroll events over 0.7 to 1.2 s, about 60 a second, including momentum after the fingers lifted. Values were fractional (0.1 to 2.8), and straight vertical swipes carried a cross-axis amount of ±0.1 to ±0.2. Counting per event would have shown `Scroll ↓ ×60` for one swipe. Events are now grouped into gestures by a gap of `scrollGap` (0.3 s). In pass 2, each swipe made exactly one gesture under that rule, with no sign changes inside it. The direction comes from the summed amounts: fingers moving up the trackpad (natural scrolling on) gave negative `scrollY`, labelled `Scroll ↓`, and fingers moving down gave positive, labelled `Scroll ↑`. The horizontal sign is still PENDING-5.

## D-019 — A click that brings the window to the front is shown
The click that focuses the probe window was delivered to the app (mouse down arrived just before mouse enter). The addon shows it like any other click rather than trying to filter it.

## D-020 — Mouse translation rules, from probe pass 3 (resolves most of PENDING-5)
Measured on a MacBook trackpad with oF 0.12.1. The rules are in KEYCAPS.md 6.8. What the probe showed:
- The event's `modifiers` field is filled in on clicks: Cmd+click arrived as left with cmd, and clicks with Ctrl, Option and Shift held arrived with all three.
- A two-finger trackpad click arrives as right (button 2). With Ctrl held, a click also arrived as right, with Ctrl still in `modifiers`, as oF's macOS source code says it should.
- A button let go outside the window is still delivered, for both left and right drags, so a held cap cannot be left stuck.
- A plain trackpad click often sends one to four Drag events, moving the pointer by up to 2 points. Real drags moved 30 to 300 points. Drag therefore only counts once the pointer has moved `dragThreshold` (5 pt) from where the button went down. To keep the core free of positions, OfInputSource measures the distance and passes it in `MouseAction.dragDistance`.
- A light brush of the trackpad made a 3-event scroll gesture summing 0.3, while the smallest real swipe summed 2.8. Gestures below `scrollMinAmount` (1.0) make no cap and no log line.
- The click that brings the window to the front was delivered again, confirming D-019.
- Not measured: Option+click on its own, and mouse buttons 3 to 8 (no mouse with extra buttons was connected).

## D-021 — Right-click hides ⌃, middle-click hides ⌥
oF turns Ctrl+click into a right-click and Option+click into a middle-click, and keeps the modifier in the event. Showing `⌃Right-click` for what the viewer sees as a right-click would be confusing, so a right-button cap never shows ⌃ and a middle-button cap never shows ⌥. The cost is that a real right-click made while Ctrl is held also shows as a plain `Right-click`, which is accepted.

## D-022 — Horizontal scroll uses the same convention as vertical
Two horizontal gestures were recorded (sums of -2.8 and +39.6), but the run did not record which way the fingers moved for each one. The labels follow the vertical convention: a negative amount means moving toward the end of the content, so negative `scrollX` is `Scroll →` and positive is `Scroll ←`. This is confirmed on screen in step 4. If it turns out wrong, the fix is a sign change in CapLabeler, recorded as a reversal here.

## D-023 — CoreSettings holds the core's part of Style
The core needs timing, limits, thresholds and label mode, but Style also holds colors, sizes and keys, which belong to the oF side. `src/core/CoreSettings.h` is a plain struct with the spec's defaults, and the oF-side Style will hold one. This keeps P3: the core never sees an oF type.

## D-024 — CapHistory::update(now) replaces update(dt) (partly reverses D-006)
D-006 had time enter the core as `push(stroke, now)` and `update(dt)`. That is two clocks: pushes carry absolute times, while updates add up frame times, and the two drift apart. CapHistory now takes `update(now)` on the same clock as the pushes, and works out ages as `now - changedAt`. The core still never reads a clock (P4). The front end passes `ofGetElapsedTimef()` to both. The Lifetime suite still checks that one 1.7 s step and 170 steps of 0.01 s give the same opacity.

## D-025 — ScrollGesture is a shared core module
The screen (CapHistory) and the log (EventLog) both need to know where a scroll gesture starts, ends and becomes visible. Writing that logic twice would let them disagree, so `src/core/ScrollGesture.h/.cpp` holds it, and CapHistory and EventLog each own one instance. Sums are kept in double precision.

## D-026 — Test tooling: recorded data, reference model, mutation script
- `tests/data/probe_pass1_scroll.txt` holds the 645 scroll events from probe pass 1. `tests/reference/scroll_reference.py` groups them into gestures with a separately written model (gaps first, then reversals, with a product test instead of a sign comparison) and generates `tests/data/probe_pass1_scroll.inc`. Both models find 15 visible gestures, one per recorded swipe.
- `tests/mutate.sh` (`make mutate`) proves each suite can fail. On 2026-11-24 all 11 mutants were caught: one or more per suite, plus an `ofMain.h` include caught by `make headers`.
- The tests build with `-Wall -Wextra -Wpedantic -Werror`. They were checked with g++ 13 and with clang (also with `-Wshadow -Wconversion`).
- No `addon_config.mk` is needed to keep tests out of host projects: Project Generator only adds an addon's `src/` and `libs/` folders, and `tests/` is outside both.
