# KEYCAPS.md — ofxKeyCaps specification

Status: FROZEN except items marked PENDING (see section 13).
Target: openFrameworks 0.12.1, macOS Apple Silicon first; Linux and Windows must build.
Repo location: `~/Documents/of_v0.12.1_osx_release/addons/ofxKeyCaps/` (a real directory, not a symlink).

This document is the authority. Code follows it. Any change after freeze is logged in DECISIONS.md with its reason.

---

## 1. Purpose

Show the keys pressed in an openFrameworks window as rounded keycaps in a row at the bottom center of the window. The caps fade out after a short time. Mouse button actions can optionally be shown in the same row.

Every press is also printed to the terminal with a timestamp, so a recording can be edited against an exact list of when things happened. A sync mark sets the log's zero and shows on screen at the same moment.

It is meant for screen recordings, live demos and teaching.

## 2. Principles

- P1. **Nothing extra to install.** A user adds the addon in Project Generator, adds one member and two calls, and it works. There is no font file to supply, no Python and no Homebrew.
- P2. **Never interfere with the host app.** The addon sees every key and mouse event after the app does. It never consumes an event, never changes the app's GL state, camera, style or viewport, and never draws unless the host calls `draw()`.
- P3. **Pure core.** Everything that decides *what* is shown or logged (labels, merging, counting, timing, log text) lives in `src/core/`, which must not include `ofMain.h` or any oF header. The core can be tested without a window.
- P4. **Time is injected.** The core never reads a clock. Time comes in from outside, so behavior is the same at any frame rate and tests are deterministic.
- P5. **Bounded modules.** One job per .h/.cpp pair, so pieces can be lifted into other projects.

## 3. Scope

In scope (v1):
- Keys pressed while the oF window has focus.
- Merged caps for typed text, separate caps for combos and special keys, ×N repeat counting.
- Mouse button caps (click, press-and-hold, drag, scroll). Optional, off by default.
- A timestamped log of every press, printed through oF's logging, with a sync mark that resets the log's zero and appears on screen.
- Fade, slide-in and reflow animation.
- Hide/show, mask (privacy) and mark controls.
- Style options.

Out of scope (v1), slot reserved:
- System-wide capture (`MacGlobalInputSource`, Objective-C++). The `InputSource` interface (section 6.7) is the reserved slot. Adding it must not require edits to the core or the renderer.

Out of scope (v1), may come later:
- Writing the log to a file. v1 prints to the terminal only.
- Double-click detection. oF does not provide it natively, and v1 leaves it out.

Out of scope, not planned:
- Mouse position, cursor highlight or trails. Mouse enter and exit events.
- On-screen keyboard map.

## 4. Folder layout

```
ofxKeyCaps/
  addon_config.mk
  KEYCAPS.md             this spec
  DECISIONS.md           decisions and reversals log
  README.md
  src/
    ofxKeyCaps.h/.cpp    front end (section 6.10)
    core/                NO oF includes (enforced, section 11)
      KeyStroke.h
      MouseAction.h
      CoreSettings.h     the core's part of Style (D-023)
      CapLabeler.h/.cpp
      ScrollGesture.h/.cpp  shared gesture grouping (D-025)
      CapHistory.h/.cpp
      EventLog.h/.cpp
    of/
      InputSource.h
      OfInputSource.h/.cpp
      KeycapRenderer.h/.cpp
  tests/                 headless tests of src/core: `make test`, `make headers`, `make mutate`
    test_main.cpp
    mutate.sh            breaks one rule at a time in a copy of src/core (D-026)
    data/                recorded probe data replayed by the tests
    reference/           Python reference model that generated the expected scroll gestures
  example-keyprobe/      logs raw oF key and mouse events (build step 2)
  example-basic/         the addon in use
```

## 5. Data flow

```
oF key + mouse events → OfInputSource → KeyStroke / MouseAction
                                              │
                    ┌─────────────────────────┴───────────────┐
                    ↓                                         ↓
     CapHistory ← update(now)                   EventLog ← now (seconds)
                    ↓                                         ↓
          list of visible caps                     one text line per press
                    ↓                                         ↓
  KeycapRenderer.draw() (screen space)         ofLogNotice("ofxKeyCaps")
```

CapLabeler is called by CapHistory and EventLog, so caps and log lines use the same labels.

## 6. Module contracts

### 6.1 KeyStroke (core, data only)

One key press as the core sees it. It carries no oF types.

- `key`: either a printable character (a Unicode codepoint) or a named special key from the addon's own `SpecialKey` list: Return, Tab, Space, Backspace, Delete, Escape, the four arrows, Home, End, PageUp, PageDown, F1–F12.
- `modifiers`: the addon's own bitmask: Ctrl, Alt (Option), Shift, Cmd (Super on Linux/Windows). These are not oF constants. OfInputSource translates them.
- `isRepeat`: true if the operating system generated it because the key is being held down.
- `isModifierOnly`: true if the key pressed is itself only a modifier (for example, Shift pressed alone). Caps Lock counts as a modifier-only key (D-017).

### 6.2 MouseAction (core, data only)

One mouse button event as the core sees it. It carries no oF types and no position.

- `button`: Left, Right, Middle, or Other with a number (4–8).
- `phase`: Down, Up, Drag, or Scroll.
- `modifiers`: the same bitmask as KeyStroke.
- `scrollX`, `scrollY`: used only when `phase` is Scroll.
- `dragDistance`: used only when `phase` is Drag. How far the pointer has moved from where the button went down, in logical points. OfInputSource fills it in, so the core still never sees a position.

### 6.3 CapLabeler (core, pure)

Turns a KeyStroke or a MouseAction into label text. It has no state.

Keys:
- Modifier order is ⌃ ⌥ ⇧ ⌘ (the macOS convention).
- Symbol mode (default on macOS) uses the symbols ⌃ ⌥ ⇧ ⌘. Text mode (default on Linux and Windows) uses `Ctrl+Alt+Shift+Super+`.
- Letters in a combo are shown uppercase: ⌘S, ⌘⇧Z.
- Shift is not shown when it only produced the character itself: typing `A` or `!` shows `A` or `!`, not ⇧A. Shift is shown when it is combined with another modifier or a special key: ⌘⇧Z, ⇧Tab, ⇧↓.
- Option shows the key pressed, not the character it composes: Option+a shows `⌥A`, not `å` (D-016). The same holds for Ctrl: Ctrl+a shows `⌃A`.
- A modifier-only cap (when `showModifierOnly` is on) shows the modifier's symbol alone: ⇧, ⌃, ⌥, ⌘, and ⇪ for Caps Lock. Text mode uses Shift, Ctrl, Alt, Super, Caps.
- Special key labels:
  - Symbol mode: Return ↩, Tab ⇥, Backspace ⌫, Delete ⌦, Escape ⎋, arrows ← → ↑ ↓.
  - Text mode: Return, Tab, Bksp, Del, Esc, Left, Right, Up, Down.
  - Space, Home, End, PgUp, PgDn and F1–F12 are shown as words in both modes.

Mouse (modifier prefix as for keys, Shift always shown):
- Button names: `Click` (left), `Right-click`, `Middle-click`, `Button 4` to `Button 8`.
- While dragging, `click` becomes `drag`: `Drag`, `Right-drag`, `Middle-drag`, `Button 4 drag`.
- Scroll: `Scroll ↑`, `Scroll ↓`, `Scroll ←` or `Scroll →`, chosen from the gesture's summed `scrollX` and `scrollY` (M6). Text mode uses Up, Down, Left, Right. Negative `scrollY` is `Scroll ↓`, positive is `Scroll ↑` (measured, D-018). Negative `scrollX` is `Scroll →`, positive is `Scroll ←`, following the same convention: negative means moving toward the end of the content (D-022, confirmed on screen in step 4).
- A `Right-click` or `Right-drag` never shows ⌃, and a `Middle-click` or `Middle-drag` never shows ⌥ (D-021). On macOS, oF turns Ctrl+click into a right-click and Option+click into a middle-click, but leaves the modifier in the event.
- Examples: `⇧Click`, `⌘Right-click`, `⌘Drag`.

### 6.4 CapHistory (core, stateful, pure)

Holds the caps currently visible and applies the rules below.

- Input comes from `push(KeyStroke, now)` and `push(MouseAction, now)`. Time advances through `update(now)`, on the same clock as the pushes (D-024).
- The output is an ordered list of caps, oldest first. Each cap has its label text, its repeat count, its age, its opacity from 0 to 1, and two flags: `held` (a mouse button is still down) and `isMark`.

Cap kinds:
- **Text cap.** A run of printable characters typed with no modifier other than Shift.
- **Single cap.** Any combo with Ctrl, Alt or Cmd, any special key, and Space when it starts a new cap.
- **Mouse cap.** One mouse action (section 6.5).
- **Mark cap.** Made by a sync mark (section 7).

Key rules, applied in order:
- R1. **Modifier-only presses** are ignored by default. `Style.showModifierOnly` turns them on, and then they make a single cap.
- R2. **Mask mode.** While masked, characters in text caps are stored and shown as `•`. Single caps are still shown. Masking never removes caps that already exist.
- R3. **Merge.** A printable keystroke is added to the newest cap when all of these are true:
  - the newest cap is a text cap,
  - less than `mergeWindow` seconds have passed since that cap last changed,
  - the cap is shorter than `maxTextChars`.
  Otherwise it starts a new text cap.
- R4. **Space inside text.** Space added to an existing text cap is added as a normal space character. Space that would start a new cap is a single cap labelled `Space`.
- R5. **Repeat.** A single cap whose label equals the newest cap's label, arriving within `repeatWindow`, increases that cap's count instead of making a new cap. The count is shown as `×N` when N ≥ 2.
- R6. **Held keys.** An `isRepeat` keystroke increases the count on the newest cap, whatever its kind. It never adds characters to a text cap. Holding `a` shows `a ×12`, not `aaaaaaaaaaaa`.
- R7. **Backspace and editing keys** are always single caps. They never delete characters from a text cap. The display shows what was pressed, not the resulting text.
- R8. **Overflow.** A text cap that reaches `maxTextChars` stops growing. The next character starts a new text cap.

Shared rules:
- R9. **Capacity.** When there are more than `maxCaps` caps, the oldest cap that is not held is removed straight away, with no fade. If every cap is held, the oldest is removed.
- R10. **Lifetime.** A cap's age resets every time the cap changes (new character, count increase). It stays fully opaque for `holdTime`, then fades linearly to 0 over `fadeTime`, and is then removed. A held cap does not age (see M2).

Defaults (all in Style):
- `mergeWindow` 1.0 s
- `repeatWindow` 1.0 s
- `maxTextChars` 24
- `maxCaps` 6
- `holdTime` 1.5 s
- `fadeTime` 0.5 s

### 6.5 Mouse rules (part of CapHistory)

- M0. **Off by default.** When `Style.showMouse` is false, MouseActions are dropped. They are neither shown nor logged.
- M1. **Down** makes a mouse cap labelled for that button and those modifiers, with `held` set.
- M2. **Held.** A held cap stays fully opaque and does not age. The renderer draws it in `pressedColor`.
- M3. **Up** clears `held` on that button's cap, and its age starts from 0, so it holds for `holdTime` and then fades.
- M4. **Drag.** The first Drag event while a button is held whose `dragDistance` reaches `dragThreshold` changes that cap's label from click to drag (R5 does not apply). Drag events below the threshold, and all later Drag events, are ignored. There is no cap per mouse movement. The threshold exists because a plain trackpad click often sends one to four Drag events that move the pointer up to 2 points (D-020).
- M5. **Repeated clicks.** A Down whose label equals the newest cap's label, arriving within `repeatWindow` of that cap's Up, increases its count and sets `held` again (`Click ×3`). This is counting, not double-click detection.
- M6. **Scroll gestures.** A trackpad sends about 60 Scroll events a second, keeps sending through the momentum after the fingers lift, and mixes a small cross-axis amount into straight swipes (D-018). So events are grouped into **gestures**, and a gesture is one action:
  - Scroll events with no gap longer than `scrollGap` belong to the same gesture, momentum included.
  - The gesture's direction comes from its running sums of `scrollX` and `scrollY`: the axis with the larger absolute sum, and that sum's sign.
  - A gesture stays invisible until the absolute running sum on its direction axis reaches `scrollMinAmount`. A brush of the trackpad (measured: 3 events summing 0.3) makes no cap and no log line.
  - When a gesture becomes visible, it makes a scroll cap, or increases the newest cap's count if that is a scroll cap with the same direction whose gesture ended within `repeatWindow` (`Scroll ↓ ×3` is three swipes). The cap's label follows the running sums while the gesture lasts.
  - A sign reversal on the gesture's direction axis ends the gesture, and the reversing event starts a new one.
  - A gesture holds its cap like a held button (M2) until it ends, then the cap ages normally.
- M7. **Missed release.** A Down for a button whose cap is still held first releases that cap (as M3), then applies M1 or M5. This keeps a lost Up event from leaving a cap held forever.
- M8. **Mask mode** does not affect mouse caps.

### 6.6 EventLog (core, stateful, pure)

Builds one text line for each press. It holds only the time of the last mark and the state needed to collapse repeats.

- Input: `log(KeyStroke, now)`, `log(MouseAction, now)` and `mark(now)`, where `now` is seconds from any fixed origin. It returns zero or one line each time.
- Line format: `HH:MM:SS.mmm` then two spaces then the label. The time is `now` minus the time of the last mark. Before the first mark, the zero is the origin of `now` (app launch, as the front end supplies it).
- **Keys.** One line per press, with the press's own label (`h`, `⌘S`), not the merged cap text. `isRepeat` keystrokes make no line. Modifier-only presses follow `Style.showModifierOnly`.
- **Mask.** While masked, printable characters are logged as `•`.
- **Mouse** (only when `showMouse` is on):
  - `Click down` and `Click up` (with the button's label and modifiers).
  - `Drag start` once, at the Drag event that relabels the cap (M4).
  - `Scroll ↓` once per gesture, at the moment it becomes visible (M6), with the direction from the running sums at that moment. Every other event in the gesture makes no line.
- **Mark.** `mark(now)` makes the line `00:00:00.000  ● MARK` and resets the zero to `now`.
- Disabled (section 6.10): nothing is logged.

Example:
```
00:00:00.000  ● MARK
00:00:02.481  ⌘S
00:00:03.020  h
00:00:03.105  e
00:00:05.300  Click down
00:00:05.410  Click up
```

### 6.7 InputSource (of/, abstract interface)

The only job of an InputSource is to deliver KeyStrokes and MouseActions to a listener. It has `start()` and `stop()`, and a way to register the listener.

### 6.8 OfInputSource (of/)

- Listens to `ofEvents().keyPressed`, and to `mousePressed`, `mouseReleased`, `mouseDragged` and `mouseScrolled`, all at `OF_EVENT_ORDER_AFTER_APP`.
- Its listeners always report the event as not consumed.
- Translates `ofMouseEventArgs` into MouseActions by these rules, measured on macOS with oF 0.12.1 (D-020):
  - **Modifiers** come from the event's `modifiers` field, which is filled in on clicks (Cmd, Shift, Option and Ctrl were all seen).
  - **Buttons** come from `button`: OF_MOUSE_BUTTON_LEFT (0), OF_MOUSE_BUTTON_MIDDLE (1), OF_MOUSE_BUTTON_RIGHT (2), and 3–7 for buttons 4–8. A two-finger trackpad click arrives as right. Ctrl+click arrives as right, with Ctrl still in `modifiers`.
  - **Releases outside the window** are delivered, so a held cap is always released.
  - **Drag distance** is measured from where the button went down, in logical points (window coordinates divided by the pixel density, when high-resolution mode is on).
  - **Scroll** amounts pass through unchanged.
- Translates `ofKeyEventArgs` into KeyStrokes by these rules, all measured on macOS with oF 0.12.1 (D-015):
  - **Modifiers** come from the event's `modifiers` field only. `ofGetKeyPressed()` is never used: in the probe it reported Shift as held for over two minutes after Shift was let go.
  - **Printable keys with Ctrl, Option or Cmd held** are identified by `keycode`, not `key`. Ctrl+a arrives with `key` = 0x01 (a control code), Option+a arrives with `key` = `å`, and both have `keycode` = 65 (`A`).
  - **Printable keys with no modifier, or Shift only**, use `key`, which is already the shifted character (`A`, `!`, `{`).
  - **Special keys** use `key` (OF_KEY_RETURN, OF_KEY_BACKSPACE, OF_KEY_TAB, OF_KEY_ESC, the arrows, F1–F12, Space). `codepoint` is not used for them: Return arrives with codepoint 0x0A, Backspace with 0.
  - **Modifier presses arrive twice**, with the same `keycode` (340–347): once with `key` set to the modifier's flag (OF_KEY_SHIFT, OF_KEY_CONTROL, OF_KEY_ALT, OF_KEY_SUPER) and once with the left/right code (OF_KEY_LEFT_SHIFT … OF_KEY_RIGHT_SUPER). Only the left/right event becomes a KeyStroke.
  - **Caps Lock** (`keycode` 280) arrives with a meaningless printable `key` value. It becomes a modifier-only KeyStroke.
  - **Events with `keycode` = -1 are dropped.** The Fn (Globe) key sends a lone key-up with `keycode` -1 and a stale character in `key`.
  - `isRepeat` is used as delivered: the first repeat comes about 0.5 s after the press, then about 12 per second.
- Does not listen to `keyReleased` (D-015). Nothing in the core needs key releases, and a release can carry a different `key` from its press (Ctrl+a goes down as 0x01 and comes up as `a` once Ctrl is let go).
- Keeps the mouse listeners registered even when `showMouse` is off. CapHistory and EventLog drop the events (M0), so turning the option on at run time needs no re-registration.
- Ignores the hide/show toggle key and the mark key (section 7), so they never make a normal cap or log line.
- `stop()` and the destructor remove every listener. A destroyed addon must leave no dangling listeners.

### 6.9 KeycapRenderer (of/)

Draws the caps and owns nothing else.

Layout and look:
- Draws in window pixel space with the origin at top-left.
- The row is centered horizontally, with newest on the right and oldest on the left, and sits `marginBottom` above the bottom edge of the window.
- Each cap is a rounded rectangle (`cornerRadius`) filled with `capColor` and drawn with an optional `outlineColor`. The label is drawn in `textColor`. The `×N` count is drawn smaller, after the label, inside the same cap.
- A held cap is filled with `pressedColor`. A mark cap is filled with `markColor`.
- Cap width follows its text, with a minimum of `capHeight`, so a single key looks square.

Animation:
- Opacity multiplies all of a cap's colors.
- A new cap slides in from the right over `slideTime`.
- When a cap is removed, the remaining caps move to their new positions with frame-rate-independent smoothing (dt-based lerp).

Scale:
- Sizes are in logical points and are multiplied by the window's pixel density, so caps look the same size and stay sharp on Retina displays and in recordings.

State isolation:
- `draw()` saves and restores style and matrix (push/pop style, view and matrix), sets up a screen-space view for itself, and leaves depth test, blending, the camera and the viewport as it found them.

Font:
- Loaded once, from the system sans font (`OF_TTF_SANS`), with the glyph ranges needed for ⌃ ⌥ ⇧ ⌘ ⇪ ↩ ⇥ ⌫ ⌦ ⎋ ● and the arrows requested at load time.
- Glyphs are verified on screen (PENDING-2). If they don't draw, `Style.labelMode` falls back to text mode for that platform.

### 6.10 ofxKeyCaps (front end)

Public surface:
- `setup()`, which uses the default Style.
- `setup(const Style&)`.
- `draw()`.
- `mark()`.
- `setEnabled(bool)` / `isEnabled()`.
- `setMasked(bool)` / `isMasked()`.
- `setStyle(const Style&)` / `getStyle()`.
- `clear()`.

Behavior:
- The addon subscribes to `ofEvents().update` itself and passes `ofGetElapsedTimef()` to `CapHistory::update(now)`. The host never calls update.
- It passes the same `ofGetElapsedTimef()` as `now` to every push and to EventLog (D-024).
- Each line from EventLog is printed with `ofLogNotice("ofxKeyCaps")`, when `Style.log` is on. The line appears in Xcode's console or in Terminal, depending on how the app was launched. oF's own `ofSetLogLevel("ofxKeyCaps", OF_LOG_SILENT)` also silences it.
- When disabled, nothing is recorded, logged or drawn. Enabling again starts with no caps. The mark time is kept.

Minimum host usage, the whole integration:
- one `ofxKeyCaps keys;` member,
- `keys.setup();` in `setup()`,
- `keys.draw();` as the last line of `draw()`.

## 7. Controls

- **Sync mark.** `mark()`, or `Style.markKey` when it is set (unset by default). A mark does three things at the same moment:
  - EventLog prints `00:00:00.000  ● MARK` and resets its zero.
  - A mark cap labelled `● MARK` is added, drawn in `markColor`, with the normal lifetime.
  - The mark key itself is never shown as a normal cap or logged as a key.
  The mark cap appears in the recording at the moment the log's zero resets, so it works like a clapperboard: find that frame, and every logged time lines up from there.
- **Hide/show toggle key.** Off by default (`Style.toggleKey` unset). When set, pressing it switches `setEnabled`. The key is never shown as a cap.
- **Mask.** API only in v1: `setMasked(true)`. A mask hotkey can be added later through Style without a spec change to the core.
- Default keys for `markKey` and `toggleKey` are PENDING-3.

## 8. Style (one plain struct, all fields have defaults)

Position:
- `anchor`: BottomCenter (default), BottomLeft, BottomRight, Custom.
- `customPosition`: used when `anchor` is Custom.
- `marginBottom` 40 pt.

Size:
- `capHeight` 44 pt.
- `cornerRadius` 8 pt.
- `capSpacing` 8 pt.
- `fontSize` 20 pt.

Color:
- `capColor`: dark grey at 85% opacity.
- `pressedColor`: a lighter grey at 85% opacity.
- `markColor`: red at 85% opacity.
- `outlineColor`: none.
- `textColor`: white.

Timing:
- `mergeWindow`, `repeatWindow`, `holdTime`, `fadeTime` (defaults in section 6.4).
- `scrollGap` 0.3 s.
- `slideTime` 0.15 s.

Mouse thresholds:
- `dragThreshold` 5 pt.
- `scrollMinAmount` 1.0.

Limits:
- `maxCaps` 6.
- `maxTextChars` 24.

Behavior:
- `labelMode`: Symbols (default on macOS), Text (default on Linux and Windows). Used by caps and log lines.
- `showModifierOnly`: false.
- `showMouse`: false.
- `log`: true.
- `markKey`: unset.
- `toggleKey`: unset.

## 9. Example log session

What a user sees in the terminal while recording a short demo, with `showMouse` on:
```
[notice ] ofxKeyCaps: 00:00:00.000  ● MARK
[notice ] ofxKeyCaps: 00:00:01.250  Click down
[notice ] ofxKeyCaps: 00:00:01.330  Drag start
[notice ] ofxKeyCaps: 00:00:02.910  Click up
[notice ] ofxKeyCaps: 00:00:04.002  ⌘Z
[notice ] ofxKeyCaps: 00:00:04.410  ⌘Z
[notice ] ofxKeyCaps: 00:00:06.700  Scroll ↓
```

## 10. Testing

`tests/` builds `src/core/` alone, with no openFrameworks, using `make test`. Output follows the ofxManifold pattern: one GREEN or RED line per suite, then a suites summary.

Required suites:
- **Labeler.** Modifier order. Shift rules (`A`, `!`, ⌘⇧Z, ⇧Tab). Every special key in both modes. Uppercase letters in combos. Every mouse button, click and drag labels, scroll directions, modifiers on mouse labels.
- **Merge.** "hello" makes one cap. A gap longer than `mergeWindow` splits caps. Cmd+S breaks a text run. R4 Space cases. R8 overflow at 24 characters.
- **Repeat.** ⌘Z three times makes `⌘Z ×3`. A gap longer than `repeatWindow` makes a new cap. Held keys follow R6.
- **Mouse.** M0 drops everything when off. Down and Up hold and release (M1–M3). Drag relabels once, only at or past `dragThreshold`, and the recorded click jitter (1 to 4 Drag events, up to 2 pt) never relabels (M4). Click ×3 (M5). Missed release (M7). Held caps survive capacity (R9). Right-click drops ⌃ and Middle-click drops ⌥ (D-021).
- **Scroll.** Replayed from the recorded probe gestures: each recorded swipe, momentum included, makes exactly one gesture, with the measured direction. Gaps just under and just over `scrollGap`. A sign reversal splits a gesture. Three swipes the same way make `Scroll ↓ ×3`. Cross-axis jitter (±0.1 to ±0.2) never changes the direction. The recorded brush (3 events, sum 0.3) makes nothing, and a gesture logs once, when it crosses `scrollMinAmount`.
- **Lifetime.** Hold, fade and removal at exact dt boundaries. Age resets when a cap changes. A held cap does not age. The same result whether dt comes as 1×1.0 s or 100×0.01 s.
- **Capacity.** R9.
- **Mask.** R2, M8, and masked log lines.
- **Log.** Time format at 0, under a second, over an hour. The zero before the first mark. A mark resets the zero. One line per press. No lines for `isRepeat` or for scroll count increases. Mouse lines only when `showMouse` is on.

Mutation check: each suite must be shown going RED at least once, by deliberately breaking the rule it covers, before it counts. `make mutate` does this with `tests/mutate.sh`: it copies `src/core`, breaks one rule per mutant, rebuilds the tests against the copy and requires the covering suite to go RED. It also breaks principle P3 (an `ofMain.h` include) and requires `make headers` to go RED. `src/core` itself is never touched.

The Scroll suite replays the 645 scroll events recorded in probe pass 1 and compares the result with the gestures expected by `tests/reference/scroll_reference.py`, a separate Python model (D-026). Python is only needed to regenerate that expected data, never to build or run the tests.

The renderer, OfInputSource and the terminal output are verified on screen in `example-basic`.

## 11. Structural checks

- `make headers` (or an equivalent script in `tests/`) fails if any file in `src/core/` includes `ofMain.h` or any `of*.h` header.
- `addon_config.mk` makes no changes to Project Generator defaults, beyond what is needed to exclude `tests/` from host projects.

## 12. Build order (each step green before the next)

1. Spec frozen. DECISIONS.md started.
2. `example-keyprobe` prints the raw fields of every key and mouse event:
   - keys: `key`, `keycode`, `scancode`, `codepoint`, `modifiers`, `isRepeat`, for every key in a fixed probe list;
   - mouse: `button`, `type`, `modifiers`, `scrollX`, `scrollY`, for each button, with modifiers, while dragging, and while scrolling both ways.
   The results go into DECISIONS.md and resolve PENDING-1 and PENDING-5. Done: three passes, recorded as D-015 to D-022.
3. Core (KeyStroke, MouseAction, CoreSettings, CapLabeler, ScrollGesture, CapHistory, EventLog) with tests, all GREEN and each one shown RED.
4. OfInputSource and KeycapRenderer, plus `example-basic`, verified on screen and in the terminal. This resolves PENDING-2 and the rest of PENDING-5.
5. Style options, mark key, toggle key and mask. This resolves PENDING-3.
6. README and publish.
7. (Later, optional) MacGlobalInputSource. It gets its own spec section before any code.

## 13. Pending items (the only parts not frozen)

- **PENDING-1.** Resolved by D-015 (key translation rules in section 6.8).
- **PENDING-2.** Whether `OF_TTF_SANS` with the requested glyph ranges draws ⌃ ⌥ ⇧ ⌘ ⇪ ↩ ⇥ ⌫ ⌦ ⎋ ● on macOS, and what happens on Linux. Resolved by step 4.
- **PENDING-3.** Default keys for `markKey` and `toggleKey`, or keeping both unset. Mac laptop F-keys need Fn, so an F-key may be a poor default. Resolved by step 5.
- **PENDING-4.** License (to match Lee's other published addons). Resolved before step 6.
- **PENDING-5.** Mouse fields, mostly resolved by D-020. Two things could not be measured with a trackpad and are checked in step 4 instead:
  - The horizontal scroll labels (D-022): swipe left and right in `example-basic` and confirm that the arrow matches the way the content moved.
  - Option+click alone, and buttons 3–8: oF's source says Option+click arrives as middle. Extra buttons need a mouse that has them. Until a mouse confirms them, the labels follow the source code.
