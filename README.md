# ofxKeyCaps

An openFrameworks addon that shows the keys you press as fading keycaps at the bottom of the window, with optional caps for mouse clicks, drags and scrolls. It also prints a timestamped log of every press to the terminal, so a screen recording can be edited against an exact list of when things happened.

Made for screen recordings, live demos and teaching.

## What it shows

- Typed text merges into one cap: `hello`.
- Combos and special keys get their own caps: `⌘S`, `⌃⇧Z`, `⇧⇥`, `↩`, `F5`.
- Repeats count instead of piling up: `⌘Z ×3`, or `a ×12` for a held key.
- Optional mouse caps: `Click`, `⇧Click`, `Right-click`, `Drag`, `Scroll ↓ ×3`. A cap stays lit while its button is held. One trackpad swipe, momentum included, counts as one scroll.
- A sync mark: press ⌃⇧M and a red `● MARK` cap appears at the same moment the log's clock resets to zero. Find that frame in your recording, and every logged time lines up from there.

## Requirements

- openFrameworks 0.12.1.
- Developed and tested on macOS (Apple Silicon). It uses only oF's own events and system fonts, with nothing extra to install. Linux and Windows builds are expected to work, with text labels (`Ctrl+Alt+K`) instead of symbols, but have not been tested yet.

## Install

Clone into your openFrameworks addons folder:

```
cd of_v0.12.1_osx_release/addons
git clone https://github.com/leeMeredith/ofxKeyCaps.git
```

Then add `ofxKeyCaps` to your project with the Project Generator.

## Use

```cpp
// ofApp.h
#include "ofxKeyCaps.h"
ofxKeyCaps keys;

// ofApp::setup()
keys.setup();

// ofApp::draw(), as the last line
keys.draw();
```

That's all. The addon sees keys and mouse events after your app does and never takes them away from it. It draws on top of your scene and leaves your drawing state, camera and viewport as it found them.

### Options

Everything is in one Style struct, and every field has a default:

```cpp
ofxKeyCaps::Style style;
style.core.showMouse = true;                       // mouse caps (off by default)
style.core.showModifierOnly = true;                // caps for Shift, Cmd ... pressed alone
style.anchor = ofxKeyCaps::Style::Anchor::BottomLeft;
style.capHeight = 56;                              // logical points
style.core.holdTime = 2.0;                         // seconds fully visible
keys.setup(style);
```

Other fields include `marginBottom`, `fontSize`, `cornerRadius`, the colors (`capColor`, `pressedColor`, `markColor`, `textColor`, `outlineColor`), `fadeTime`, `maxCaps` and `log`. See `src/of/Style.h` and `src/core/CoreSettings.h`.

From code:

```cpp
keys.mark();              // same as the mark key
keys.setEnabled(false);   // hide, and stop logging
keys.setMasked(true);     // typed characters show as •, for passwords
keys.clear();
```

### Keys

- **⌃⇧M** marks: the log's zero resets and `● MARK` appears.
- **⌃⇧K** hides and shows the caps. It works while they are hidden, and writes `(hidden)` or `(shown)` to the log so gaps in a recording can be found.

Neither key ever shows as a cap or a log line. To change one, or turn it off:

```cpp
using namespace ofx::keycaps::core;
style.markKey = Hotkey::specialKey(SpecialKey::F5);
style.toggleKey = Hotkey::character('h', ModCtrl | ModShift);   // ⌃⇧H
style.markKey = Hotkey::none();                                 // off
```

The defaults avoid ⌃⌥, because that is VoiceOver's own modifier: with VoiceOver on, a ⌃⌥ key never reaches the app.

## The log

Every press prints one line through `ofLogNotice("ofxKeyCaps")`, in Xcode's console or in Terminal:

```
[notice ] ofxKeyCaps: mark ⌃⇧M, show/hide ⌃⇧K
[notice ] ofxKeyCaps: 00:00:00.000  ● MARK
[notice ] ofxKeyCaps: 00:00:01.464  Click down
[notice ] ofxKeyCaps: 00:00:01.714  Drag start
[notice ] ofxKeyCaps: 00:00:02.031  Click up
[notice ] ofxKeyCaps: 00:00:17.681  ⌘S
[notice ] ofxKeyCaps: 00:00:51.513  (hidden)
```

Held keys and the stream of scroll events from a trackpad do not flood it: one line per press, one per scroll gesture. To silence it, set `style.log = false`, or call `ofSetLogLevel("ofxKeyCaps", OF_LOG_SILENT)`.

## Examples

- **example-basic**: the addon with mouse caps on, and a grid that follows scrolling.
- **example-keyprobe**: prints the raw fields of every key and mouse event oF delivers. This is how the addon's rules were measured; it is useful if keys behave differently on your machine.

Build either with the Project Generator (import the example folder, then Update), or in Terminal with `make` inside the example folder.

## Tests

The logic that decides what is shown and logged is plain C++ with no openFrameworks, in `src/core`. Its tests replay the key presses and trackpad scrolls recorded on a real Mac:

```
make -C tests test      # every suite GREEN or RED
make -C tests mutate    # breaks each rule on purpose and checks its suite goes RED
```

## Known limits

- Only keys pressed while the openFrameworks window has focus are shown.
- Some keys never reach an app: ⌘Q (the menu quits first), F11 (taken by macOS), and anything VoiceOver or system shortcuts catch.
- With Ctrl or Option held, the key is read by its position on a US layout, so on other layouts a Ctrl or Option combo can show the US letter.
- Option+click and mouse buttons 4 to 8 follow openFrameworks' source but have not been tested with a real mouse.

## Design

`KEYCAPS.md` is the specification, and `DECISIONS.md` records every decision and every reversal, with the measurements behind them.

## License

MIT. See `LICENSE`.
