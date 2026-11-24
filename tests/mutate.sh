#!/usr/bin/env bash
# Mutation check (KEYCAPS.md section 10): break one rule at a time in a
# copy of src/core, rebuild the tests against the copy, and require the
# suite that covers the rule to go RED. src/core itself is never touched.
#
# Prints CAUGHT or SURVIVED per mutant, then a summary. Exit 1 if any
# mutant survives or any mutation fails to apply.

set -u
cd "$(dirname "$0")"

CORE=../src/core
WORK=$(mktemp -d "${TMPDIR:-/tmp}/keycaps-mutate.XXXXXX")
trap 'rm -rf "$WORK"' EXIT

caught=0
survived=0
broken=0

# mutant NAME SUITE FILE FROM TO
mutant() {
	local name=$1 suite=$2 file=$3 from=$4 to=$5
	rm -rf "$WORK/core" "$WORK/build"
	cp -R "$CORE" "$WORK/core"
	FROM="$from" TO="$to" perl -0pi -e 's/\Q$ENV{FROM}\E/$ENV{TO}/' "$WORK/core/$file"
	if cmp -s "$CORE/$file" "$WORK/core/$file"; then
		echo "BROKEN   $name (pattern not found in $file)"
		broken=$((broken + 1))
		return
	fi
	local out
	out=$(make -s test CORE="$WORK/core" BUILD="$WORK/build" 2>&1)
	if ! echo "$out" | grep -q "^suites:" && [ "$suite" != "Headers" ]; then
		echo "BROKEN   $name (the mutated core did not build)"
		broken=$((broken + 1))
	elif echo "$out" | grep -q "^RED    $suite "; then
		echo "CAUGHT   $name -> $suite RED"
		caught=$((caught + 1))
	elif echo "$out" | grep -q "HEADERS RED" && [ "$suite" = "Headers" ]; then
		echo "CAUGHT   $name -> make headers RED"
		caught=$((caught + 1))
	else
		echo "SURVIVED $name (expected $suite RED)"
		survived=$((survived + 1))
	fi
}

mutant "D-021 right-click keeps Ctrl" Labeler CapLabeler.cpp \
	'shown &= static_cast<uint8_t>(~ModCtrl); // D-021' \
	'// mutated'

mutant "D-015 modifier flag events not dropped" Keys RawInput.cpp \
	'if (in.key < raw::KeyLeftShift || in.key > raw::KeyRightSuper) return false;' \
	'// mutated'

mutant "D-015 Ctrl combos read key instead of keycode" Keys RawInput.cpp \
	'const bool ctrlOrAlt = (out.modifiers & (ModCtrl | ModAlt)) != 0;' \
	'const bool ctrlOrAlt = false;'

mutant "R8 overflow at 25 instead of 24" Merge CapHistory.cpp \
	'CapLabeler::codepointCount(n->label) < settings_.maxTextChars' \
	'CapLabeler::codepointCount(n->label) <= settings_.maxTextChars'

mutant "R6 held key does not count" Repeat CapHistory.cpp \
	$'if (k.isRepeat) {\n\t\t\tif (Cap * n = newest()) {\n\t\t\t\tn->count += 1;' \
	$'if (k.isRepeat) {\n\t\t\tif (Cap * n = newest()) {\n\t\t\t\tn->count += 0;'

mutant "M4 drag ignores the threshold" Mouse CapHistory.cpp \
	'm.dragDistance >= settings_.dragThreshold' \
	'm.dragDistance >= 0.f'

mutant "M6 reversal never splits" Scroll ScrollGesture.cpp \
	'if (sum != 0.0 && v != 0.0 && ((sum > 0.0) != (v > 0.0))) {' \
	'if (sum != 0.0 && v != 0.0 && false) {'

mutant "M6 brush becomes visible" Scroll ScrollGesture.cpp \
	'std::fabs(axisSum) >= s.scrollMinAmount' \
	'std::fabs(axisSum) >= 0.0'

mutant "R10 no fade" Lifetime CapHistory.cpp \
	'c.opacity = static_cast<float>(1.0 - (age - hold) / fade);' \
	'c.opacity = 1.f;'

mutant "R9 drops the newest cap" Capacity CapHistory.cpp \
	'caps_.erase(victim);' \
	'caps_.erase(caps_.end() - 1);'

mutant "R2 mask ignored" Mask CapLabeler.cpp \
	'if (masked) return maskChar(mode);' \
	'if (false && masked) return maskChar(mode);'

mutant "Log minutes wrap at 100" Log EventLog.cpp \
	'(ms / 60000) % 60' \
	'(ms / 60000) % 100'

mutant "P3 core includes ofMain.h" Headers KeyStroke.h \
	'#include <cstdint>' \
	$'#include <cstdint>\n#include "ofMain.h"'

echo "mutants: $caught caught, $survived survived, $broken broken"
[ "$survived" -eq 0 ] && [ "$broken" -eq 0 ]
