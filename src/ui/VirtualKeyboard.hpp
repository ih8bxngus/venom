#pragma once

#include "NanoVG.hpp"
#include "VenomTheme.hpp"

#include <cstdint>
#include <set>
#include <unordered_map>

START_NAMESPACE_DGL

class VirtualKeyboard : public NanoSubWidget
{
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void keyPressed(VirtualKeyboard* kb, uint8_t note, uint8_t velocity) = 0;
        virtual void keyReleased(VirtualKeyboard* kb, uint8_t note) = 0;
    };

    explicit VirtualKeyboard(NanoTopLevelWidget* parent);

    void setListener(Listener* listener);

    // External highlight (e.g., from MIDI input)
    void highlightNote(uint8_t note, bool on);

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent& ev) override;
    bool onMotion(const MotionEvent& ev) override;
    bool onKeyboard(const KeyboardEvent& ev) override;

private:
    Listener* listener_ = nullptr;
    int activeNote_ = -1;       // Mouse-held note
    bool isMouseDown_ = false;

    // All currently active notes (mouse + keyboard + external)
    std::set<int> activeNotes_;
    std::set<int> keyboardNotes_;  // Notes from computer keyboard
    std::unordered_map<uint, int> activeKeyMap_;  // keycode → note for robust release

    // 44 keys: C2 (36) to B5 (83)
    static constexpr int kFirstNote = 36;   // C2
    static constexpr int kLastNote  = 83;   // B5
    static constexpr int kNumWhiteKeys = 26; // 7 per octave × 3 + 5 (C2-B5 has 3.67 octaves → 26 white keys)

    int noteAtPosition(float localX, float localY) const;
    static bool isBlackKey(int note);
    int whiteKeyIndex(int note) const;
    float whiteKeyX(int index) const;
    float whiteKeyWidth() const;
    float blackKeyWidth() const;
    float blackKeyHeight() const;

    // Computer keyboard → MIDI note mapping
    int keyToNote(uint key) const;
    void changeOctave(int delta);
    int octaveOffset_ = 0;  // 0 = base C3 (MIDI 48), range -2..+5

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualKeyboard)
};

END_NAMESPACE_DGL
