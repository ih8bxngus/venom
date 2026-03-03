#include "PresetBrowser.hpp"
#include <algorithm>
#include <cstring>
#include <set>

START_NAMESPACE_DGL

using namespace venom::theme;

PresetBrowser::PresetBrowser(NanoTopLevelWidget* parent)
    : NanoSubWidget(parent)
{
    // Allow drawing the dropdown overlay outside widget bounds
    setNeedsFullViewportDrawing(true);
}

void PresetBrowser::setPresetCount(int count)
{
    presetCount_ = count;
}

void PresetBrowser::setCurrentIndex(int index)
{
    currentIndex_ = index;
    repaint();
}

int PresetBrowser::getCurrentIndex() const
{
    return currentIndex_;
}

void PresetBrowser::setPresetName(const char* name)
{
    presetName_ = name;
    repaint();
}

void PresetBrowser::addPreset(int index, const char* name, const char* category)
{
    presets_.push_back({name, category, index});
}

void PresetBrowser::setListener(Listener* listener)
{
    listener_ = listener;
}

// ---------------------------------------------------------------------------
// Dropdown open/close helpers

float PresetBrowser::calcDropdownHeight() const
{
    if (presets_.empty())
        return 0.0f;

    std::set<std::string> seen;
    int catCount = 0;
    for (const auto& p : presets_) {
        if (seen.insert(p.category).second)
            ++catCount;
    }

    float totalH = 8.0f; // top padding
    totalH += static_cast<float>(catCount) * kCategoryItemH;
    totalH += static_cast<float>(presets_.size()) * kDropdownItemH;
    totalH += static_cast<float>(catCount) * 4.0f; // gap after each category
    totalH += 4.0f; // bottom padding
    return totalH;
}

void PresetBrowser::openDropdown()
{
    if (dropdownOpen_)
        return;

    baseHeight_ = static_cast<float>(getHeight());
    dropdownOpen_ = true;

    // Expand widget height to encompass the dropdown so contains() passes
    float ddH = calcDropdownHeight();
    setSize(getWidth(), static_cast<uint>(baseHeight_ + 2.0f + ddH + 4.0f));

    // Bring to front so we get mouse events before other widgets
    toFront();
    repaint();
}

void PresetBrowser::closeDropdown()
{
    if (!dropdownOpen_)
        return;

    dropdownOpen_ = false;

    // Restore original height
    if (baseHeight_ > 0.0f)
        setSize(getWidth(), static_cast<uint>(baseHeight_));

    repaint();
}

// ---------------------------------------------------------------------------
// Drawing

void PresetBrowser::onNanoDisplay()
{
    const float w = static_cast<float>(getWidth());
    // Use baseHeight_ for the header area when dropdown is open
    const float h = dropdownOpen_ ? baseHeight_ : static_cast<float>(getHeight());

    // Background
    beginPath();
    roundedRect(0, 0, w, h, 4.0f);
    fillColor(Color(10, 10, 18, 200));
    fill();

    // Subtle border
    beginPath();
    roundedRect(0, 0, w, h, 4.0f);
    strokeColor(Color(30, 30, 48));
    strokeWidth(1.0f);
    stroke();

    const Color accent = oscAccent();

    // Left arrow (◀)
    {
        float cx = kArrowWidth * 0.5f;
        float cy = h * 0.5f;
        float sz = 4.0f;

        beginPath();
        moveTo(cx + sz, cy - sz);
        lineTo(cx - sz, cy);
        lineTo(cx + sz, cy + sz);
        closePath();
        fillColor(accent);
        fill();
    }

    // Right arrow (▶)
    {
        float cx = w - kArrowWidth * 0.5f;
        float cy = h * 0.5f;
        float sz = 4.0f;

        beginPath();
        moveTo(cx - sz, cy - sz);
        lineTo(cx + sz, cy);
        lineTo(cx - sz, cy + sz);
        closePath();
        fillColor(accent);
        fill();
    }

    // Preset name (centered) with down arrow indicator
    fontSize(11.0f);
    fillColor(textPrimary());
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);

    // Show "Category: Name" format
    std::string displayText = presetName_;
    if (!presets_.empty() && currentIndex_ >= 0 && currentIndex_ < static_cast<int>(presets_.size())) {
        displayText = std::string(presets_[currentIndex_].category) + " : " + presets_[currentIndex_].name;
    }
    text(w * 0.5f, h * 0.5f, displayText.c_str(), nullptr);

    // Small dropdown triangle
    {
        float tx = w * 0.5f + 80.0f;
        float ty = h * 0.5f;
        beginPath();
        moveTo(tx, ty - 2.0f);
        lineTo(tx + 4.0f, ty + 2.0f);
        lineTo(tx - 4.0f, ty + 2.0f);
        closePath();
        fillColor(textDim());
        fill();
    }

    // Draw dropdown overlay if open
    if (dropdownOpen_)
        drawDropdown();
}

void PresetBrowser::drawDropdown()
{
    if (presets_.empty())
        return;

    // Gather unique categories in order
    std::vector<const char*> categories;
    std::set<std::string> seen;
    for (const auto& p : presets_) {
        if (seen.insert(p.category).second)
            categories.push_back(p.category);
    }

    float totalH = calcDropdownHeight();

    const float dropW = static_cast<float>(getWidth()) - 8.0f;
    const float dropX = 4.0f;
    const float dropY = baseHeight_ + 2.0f;

    // Dropdown background
    beginPath();
    roundedRect(dropX, dropY, dropW, totalH, 6.0f);
    fillColor(Color(12, 12, 20, 240));
    fill();

    beginPath();
    roundedRect(dropX, dropY, dropW, totalH, 6.0f);
    strokeColor(Color(40, 40, 60));
    strokeWidth(1.0f);
    stroke();

    float cy = dropY + 6.0f;

    for (const auto& cat : categories) {
        // Category header
        fontSize(9.0f);
        fillColor(textDim());
        textAlign(ALIGN_LEFT | ALIGN_TOP);
        text(dropX + 12.0f, cy + 2.0f, cat, nullptr);
        cy += kCategoryItemH;

        // Presets in this category
        for (const auto& p : presets_) {
            if (std::strcmp(p.category, cat) != 0)
                continue;

            // Highlight current preset
            if (p.index == currentIndex_) {
                beginPath();
                roundedRect(dropX + 4.0f, cy, dropW - 8.0f, kDropdownItemH, 3.0f);
                fillColor(Color(oscAccent().red, oscAccent().green, oscAccent().blue, 0.2f));
                fill();
            }

            fontSize(11.0f);
            fillColor(p.index == currentIndex_ ? oscAccent() : textPrimary());
            textAlign(ALIGN_LEFT | ALIGN_MIDDLE);
            text(dropX + 20.0f, cy + kDropdownItemH * 0.5f, p.name, nullptr);
            cy += kDropdownItemH;
        }
        cy += 4.0f;
    }
}

// ---------------------------------------------------------------------------
// Hit testing

int PresetBrowser::hitTestDropdown(float localX, float localY) const
{
    if (presets_.empty())
        return -1;

    std::vector<const char*> categories;
    std::set<std::string> seen;
    for (const auto& p : presets_) {
        if (seen.insert(p.category).second)
            categories.push_back(p.category);
    }

    const float dropX = 4.0f;
    const float dropW = static_cast<float>(getWidth()) - 8.0f;
    const float dropY = baseHeight_ + 2.0f;

    if (localX < dropX || localX > dropX + dropW)
        return -1;

    float cy = dropY + 6.0f;

    for (const auto& cat : categories) {
        cy += kCategoryItemH;

        for (const auto& p : presets_) {
            if (std::strcmp(p.category, cat) != 0)
                continue;

            if (localY >= cy && localY < cy + kDropdownItemH)
                return p.index;

            cy += kDropdownItemH;
        }
        cy += 4.0f;
    }

    return -1;
}

// ---------------------------------------------------------------------------
// Mouse handling

bool PresetBrowser::onMouse(const MouseEvent& ev)
{
    if (ev.button != 1)
        return false;

    // ev.pos is already local to this widget (DPF transforms it)
    const float localX = static_cast<float>(ev.pos.getX());
    const float localY = static_cast<float>(ev.pos.getY());

    // The "header" area is the original widget height
    const float headerH = dropdownOpen_ ? baseHeight_ : static_cast<float>(getHeight());

    if (ev.press) {
        // --- Dropdown is open: handle clicks in dropdown area ---
        if (dropdownOpen_) {
            // Check if clicking on a dropdown item
            int hit = hitTestDropdown(localX, localY);
            if (hit >= 0) {
                currentIndex_ = hit;
                closeDropdown();
                if (listener_ != nullptr)
                    listener_->presetSelected(this, currentIndex_);
                return true;
            }

            // Check if clicking in the header area (arrows or center)
            if (localY >= 0 && localY < headerH) {
                if (localX >= 0 && localX < kArrowWidth && presetCount_ > 0) {
                    // Previous
                    closeDropdown();
                    currentIndex_ = (currentIndex_ - 1 + presetCount_) % presetCount_;
                    if (listener_ != nullptr)
                        listener_->presetSelected(this, currentIndex_);
                    return true;
                }
                if (localX > static_cast<float>(getWidth()) - kArrowWidth &&
                    localX < static_cast<float>(getWidth()) && presetCount_ > 0) {
                    // Next
                    closeDropdown();
                    currentIndex_ = (currentIndex_ + 1) % presetCount_;
                    if (listener_ != nullptr)
                        listener_->presetSelected(this, currentIndex_);
                    return true;
                }
                // Click on center area while open → close
                closeDropdown();
                return true;
            }

            // Click outside everything → close dropdown
            closeDropdown();
            return true;  // consume event so widgets underneath don't get it
        }

        // --- Dropdown is closed: normal header interaction ---
        if (localX < 0 || localX > static_cast<float>(getWidth()) ||
            localY < 0 || localY > headerH)
            return false;  // outside widget bounds

        if (presetCount_ <= 0)
            return false;

        if (localX < kArrowWidth) {
            // Previous
            currentIndex_ = (currentIndex_ - 1 + presetCount_) % presetCount_;
            if (listener_ != nullptr)
                listener_->presetSelected(this, currentIndex_);
            repaint();
            return true;
        } else if (localX > static_cast<float>(getWidth()) - kArrowWidth) {
            // Next
            currentIndex_ = (currentIndex_ + 1) % presetCount_;
            if (listener_ != nullptr)
                listener_->presetSelected(this, currentIndex_);
            repaint();
            return true;
        } else {
            // Open dropdown
            openDropdown();
            return true;
        }
    }

    // On release: consume if dropdown is open to prevent click-through
    if (dropdownOpen_)
        return true;

    return false;
}

END_NAMESPACE_DGL
