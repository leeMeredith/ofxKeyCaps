#include "KeycapRenderer.h"
#include "../core/CapLabeler.h"
#include "ofMain.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace ofx {
namespace keycaps {

	namespace {
		// Every symbol Symbols mode can draw (KEYCAPS.md 6.3, 6.9).
		const std::vector<uint32_t> kSymbols = {
			0x2303, 0x2325, 0x21E7, 0x2318, 0x21EA, // ⌃ ⌥ ⇧ ⌘ ⇪
			0x21A9, 0x21E5, 0x232B, 0x2326, 0x238B, // ↩ ⇥ ⌫ ⌦ ⎋
			0x2190, 0x2191, 0x2192, 0x2193, // ← ↑ → ↓
			0x25CF, 0x00D7, 0x2022, // ● × •
		};

		// Fonts to try, symbol-capable first (D-028, D-032).
		std::vector<std::string> candidateFonts() {
#if defined(TARGET_OSX)
			return { "Lucida Grande", "Menlo", OF_TTF_SANS };
#elif defined(TARGET_LINUX)
			return { "DejaVu Sans", OF_TTF_SANS };
#elif defined(TARGET_WIN32)
			return { "Segoe UI Symbol", OF_TTF_SANS };
#else
			return { OF_TTF_SANS };
#endif
		}

		std::string utf8(uint32_t cp) {
			std::string s;
			core::CapLabeler::appendUtf8(s, cp);
			return s;
		}
	} // namespace

	bool KeycapRenderer::loadFont(const std::string & name, float pixelSize) {
		ofTrueTypeFontSettings s(name, std::max(1, static_cast<int>(std::lround(pixelSize))));
		s.antialiased = true;
		s.addRanges({ ofUnicode::Latin, ofUnicode::Latin1Supplement, ofUnicode::LatinA,
			ofUnicode::GeneralPunctuation, ofUnicode::Arrows, ofUnicode::MiscTechnical,
			ofUnicode::GeometricShapes });
		if (!font_.load(s)) return false;

		ofTrueTypeFontSettings c(name, std::max(1, static_cast<int>(std::lround(pixelSize * 0.72f))));
		c.antialiased = true;
		c.addRanges({ ofUnicode::Latin, ofUnicode::Latin1Supplement });
		countFont_.load(c);
		fontName_ = name;
		return true;
	}

	bool KeycapRenderer::setup(const Style & style, float pixelScale) {
		scale_ = std::max(1.f, pixelScale);
		fontSize_ = style.fontSize;
		symbolsOk_ = false;
		missing_.clear();
		slots_.clear();

		const float px = style.fontSize * scale_;
		for (const std::string & name : candidateFonts()) {
			// CoreText says whether the font really has every symbol (D-032).
			const FontCheck check = checkFontCoverage(name, kSymbols);
			if (!check.checked) {
				missing_ = "(the symbol check needs macOS) ";
				break;
			}
			if (check.found && check.missing.empty()) {
				if (loadFont(name, px)) {
					symbolsOk_ = true;
					return true;
				}
				continue;
			}
			std::string report = "\"" + name + "\" ";
			if (!check.found) {
				report += "not installed (macOS offered \"" + check.resolvedName + "\") ";
			} else {
				report += "lacks ";
				for (uint32_t cp : check.missing) report += utf8(cp) + " ";
			}
			missing_ += report;
		}
		// No candidate has every symbol: use the first that loads at all.
		for (const std::string & name : candidateFonts()) {
			if (loadFont(name, px)) return true;
		}
		return false;
	}

	void KeycapRenderer::draw(const core::CapHistory & history, const Style & style, float pixelScale, double now) {
		if (!font_.isLoaded()) return;
		const auto & caps = history.caps();
		const core::LabelMode mode = history.settings().labelMode;

		const float s = std::max(1.f, pixelScale);
		const float h = style.capHeight * s;
		const float pad = h * 0.32f;
		const float gap = h * 0.16f;
		const float spacing = style.capSpacing * s;
		const float radius = style.cornerRadius * s;
		const float W = static_cast<float>(ofGetWidth());
		const float H = static_cast<float>(ofGetHeight());

		// Layout: widths, total, row origin.
		std::vector<float> widths(caps.size());
		std::vector<std::string> suffixes(caps.size());
		float total = 0.f;
		for (size_t i = 0; i < caps.size(); ++i) {
			suffixes[i] = core::CapLabeler::countSuffix(caps[i].count, mode);
			float content = font_.stringWidth(caps[i].label);
			if (!suffixes[i].empty()) content += gap + countFont_.stringWidth(suffixes[i]);
			widths[i] = std::max(h, content + pad * 2.f);
			total += widths[i];
		}
		if (!caps.empty()) total += spacing * static_cast<float>(caps.size() - 1);

		float x0 = 0.f;
		float top = H - style.marginBottom * s - h;
		const float centerX = (W - total) * 0.5f;
		const float leftX = style.marginSide * s;
		const float rightX = W - style.marginSide * s - total;
		const float topY = style.marginTop * s;
		switch (style.anchor) {
		case Style::Anchor::BottomCenter: x0 = centerX; break;
		case Style::Anchor::BottomLeft: x0 = leftX; break;
		case Style::Anchor::BottomRight: x0 = rightX; break;
		case Style::Anchor::TopCenter: x0 = centerX; top = topY; break;
		case Style::Anchor::TopLeft: x0 = leftX; top = topY; break;
		case Style::Anchor::TopRight: x0 = rightX; top = topY; break;
		case Style::Anchor::Center: x0 = centerX; top = (H - h) * 0.5f; break;
		case Style::Anchor::Custom:
			x0 = style.customPosition.x * s - total * 0.5f;
			top = style.customPosition.y * s - h;
			break;
		}

		// Slide animation: frame-rate-independent smoothing toward the
		// target x; new caps start to the right of their place.
		const double dt = lastDraw_ < 0.0 ? 0.0 : std::min(0.1, std::max(0.0, now - lastDraw_));
		lastDraw_ = now;
		const float k = style.slideTime > 0.0
			? static_cast<float>(1.0 - std::exp(-dt * 4.0 / style.slideTime))
			: 1.f;

		std::set<uint64_t> live;
		std::vector<float> xs(caps.size());
		float target = x0;
		for (size_t i = 0; i < caps.size(); ++i) {
			live.insert(caps[i].id);
			auto it = slots_.find(caps[i].id);
			if (it == slots_.end()) {
				it = slots_.emplace(caps[i].id, Slot { target + h * 0.6f }).first;
			}
			it->second.x += (target - it->second.x) * k;
			xs[i] = it->second.x;
			target += widths[i] + spacing;
		}
		for (auto it = slots_.begin(); it != slots_.end();) {
			it = live.count(it->first) ? std::next(it) : slots_.erase(it);
		}
		if (caps.empty()) return;

		// Draw, saving and restoring everything touched (P2).
		const bool depthWasOn = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
		ofPushStyle();
		ofPushView();
		ofViewport(0, 0, W, H);
		ofSetupScreen();
		ofDisableDepthTest();
		ofEnableAlphaBlending();

		const float capTextHeight = font_.getStringBoundingBox("H", 0, 0).height;
		for (size_t i = 0; i < caps.size(); ++i) {
			const core::Cap & c = caps[i];
			const float a = std::max(0.f, std::min(1.f, c.opacity));
			if (a <= 0.f) continue;
			ofColor fill = c.isMark() ? style.markColor : (c.held ? style.pressedColor : style.capColor);
			fill.a = static_cast<unsigned char>(fill.a * a);
			const ofRectangle r(xs[i], top, widths[i], h);

			ofFill();
			ofSetColor(fill);
			ofDrawRectRounded(r, radius);
			if (style.outlineColor.a > 0) {
				ofColor o = style.outlineColor;
				o.a = static_cast<unsigned char>(o.a * a);
				ofNoFill();
				ofSetLineWidth(std::max(1.f, s));
				ofSetColor(o);
				ofDrawRectRounded(r, radius);
				ofFill();
			}

			const float labelW = font_.stringWidth(c.label);
			float content = labelW;
			if (!suffixes[i].empty()) content += gap + countFont_.stringWidth(suffixes[i]);
			const float tx = r.x + (r.width - content) * 0.5f;
			const float baseline = r.y + (h + capTextHeight) * 0.5f;
			ofColor t = style.textColor;
			t.a = static_cast<unsigned char>(t.a * a);
			ofSetColor(t);
			font_.drawString(c.label, tx, baseline);
			if (!suffixes[i].empty()) {
				countFont_.drawString(suffixes[i], tx + labelW + gap, baseline);
			}
		}

		ofPopView();
		ofPopStyle();
		if (depthWasOn) ofEnableDepthTest();
	}

} // namespace keycaps
} // namespace ofx
