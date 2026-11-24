#include "MacFontCheck.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_OSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <ApplicationServices/ApplicationServices.h>
#include <cctype>

namespace ofx {
namespace keycaps {

	namespace {
		std::string toStdString(CFStringRef s) {
			if (!s) return std::string();
			char buf[512];
			if (CFStringGetCString(s, buf, sizeof buf, kCFStringEncodingUTF8)) return std::string(buf);
			return std::string();
		}

		std::string lower(std::string s) {
			for (char & c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			return s;
		}
	} // namespace

	FontCheck checkFontCoverage(const std::string & fontName, const std::vector<uint32_t> & codepoints) {
		FontCheck result;
		result.checked = true;

		CFStringRef name = CFStringCreateWithCString(nullptr, fontName.c_str(), kCFStringEncodingUTF8);
		if (!name) return result;
		CTFontRef font = CTFontCreateWithName(name, 24.0, nullptr);
		CFRelease(name);
		if (!font) return result;

		// CoreText substitutes another font for an unknown name, so compare
		// the font it returned with the one asked for.
		CFStringRef full = CTFontCopyFullName(font);
		CFStringRef family = CTFontCopyFamilyName(font);
		result.resolvedName = toStdString(full);
		const std::string familyName = toStdString(family);
		if (full) CFRelease(full);
		if (family) CFRelease(family);
		const std::string want = lower(fontName);
		result.found = lower(result.resolvedName) == want || lower(familyName) == want;

		for (uint32_t cp : codepoints) {
			// Every symbol used is in the Basic Multilingual Plane: one UTF-16 unit.
			UniChar c = static_cast<UniChar>(cp);
			CGGlyph g = 0;
			const bool ok = cp <= 0xFFFF && CTFontGetGlyphsForCharacters(font, &c, &g, 1) && g != 0;
			if (!ok) result.missing.push_back(cp);
		}
		CFRelease(font);
		return result;
	}

} // namespace keycaps
} // namespace ofx

#else

namespace ofx {
namespace keycaps {

	FontCheck checkFontCoverage(const std::string &, const std::vector<uint32_t> &) {
		return FontCheck(); // checked == false: CoreText is macOS only
	}

} // namespace keycaps
} // namespace ofx

#endif
