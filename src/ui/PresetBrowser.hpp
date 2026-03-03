#pragma once

#include "NanoVG.hpp"
#include "VenomTheme.hpp"

#include <string>
#include <vector>

START_NAMESPACE_DGL

class PresetBrowser : public NanoSubWidget
{
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void presetSelected(PresetBrowser* browser, int index) = 0;
    };

    struct PresetEntry {
        const char* name;
        const char* category;
        int index;
    };

    explicit PresetBrowser(NanoTopLevelWidget* parent);

    void setPresetCount(int count);
    void setCurrentIndex(int index);
    int getCurrentIndex() const;
    void setPresetName(const char* name);
    void addPreset(int index, const char* name, const char* category);

    void setListener(Listener* listener);

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent& ev) override;

private:
    int currentIndex_ = 0;
    int presetCount_ = 0;
    std::string presetName_ = "Init";
    Listener* listener_ = nullptr;
    bool dropdownOpen_ = false;
    float baseHeight_ = 0.0f;  // original widget height before dropdown expand

    std::vector<PresetEntry> presets_;

    static constexpr float kArrowWidth = 28.0f;
    static constexpr float kDropdownItemH = 22.0f;
    static constexpr float kCategoryItemH = 18.0f;

    void openDropdown();
    void closeDropdown();
    float calcDropdownHeight() const;
    void drawDropdown();
    int hitTestDropdown(float localX, float localY) const;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};

END_NAMESPACE_DGL
