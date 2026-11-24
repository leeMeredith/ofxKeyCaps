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
