#pragma once

#include "Color.hpp"

namespace venom {
namespace theme {

using DGL_NAMESPACE::Color;

// Background colors
inline Color background()     { return Color::fromHTML("#08080F"); }
inline Color sectionBg()      { return Color::fromHTML("#0F0F1A"); }
inline Color headerBg()       { return Color::fromHTML("#0B0B14"); }
inline Color sectionHeaderBg(){ return Color::fromHTML("#0D0D18"); }

// Accent colors per section
inline Color oscAccent()      { return Color::fromHTML("#8B5CF6"); }  // Purple
inline Color filterAccent()   { return Color::fromHTML("#06B6D4"); }  // Cyan
inline Color envAccent()      { return Color::fromHTML("#22C55E"); }  // Green
inline Color lfoAccent()      { return Color::fromHTML("#F97316"); }  // Orange
inline Color fmAccent()       { return Color::fromHTML("#EF4444"); }  // Red-orange (FM matrix)
inline Color accentHighlight(){ return Color::fromHTML("#3B82F6"); }  // Blue

// Text colors
inline Color textPrimary()    { return Color::fromHTML("#E2E8F0"); }
inline Color textDim()        { return Color::fromHTML("#64748B"); }

// Knob colors
inline Color knobTrack()      { return Color::fromHTML("#1A1A28"); }
inline Color knobCenter()     { return Color::fromHTML("#08080F"); }

// Section divider
inline Color divider()        { return Color::fromHTML("#1A1A2E"); }
inline Color sectionBorder()  { return Color::fromHTML("#1A1A2E"); }

} // namespace theme
} // namespace venom
