#include "VirtualKeyboard.hpp"

START_NAMESPACE_DGL

using namespace venom::theme;

VirtualKeyboard::VirtualKeyboard(NanoTopLevelWidget* parent)
    : NanoSubWidget(parent)
{
}

void VirtualKeyboard::setListener(Listener* listener)
{
    listener_ = listener;
}

void VirtualKeyboard::highlightNote(uint8_t note, bool on)
{
    if (on)
        activeNotes_.insert(note);
    else
        activeNotes_.erase(note);
    repaint();
}

bool VirtualKeyboard::isBlackKey(int note)
{
    int pc = note % 12;
    return (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10);
}

float VirtualKeyboard::whiteKeyWidth() const
{
    return static_cast<float>(getWidth()) / static_cast<float>(kNumWhiteKeys);
}

float VirtualKeyboard::blackKeyWidth() const
{
    return whiteKeyWidth() * 0.58f;
}

float VirtualKeyboard::blackKeyHeight() const
{
    return static_cast<float>(getHeight()) * 0.6f;
}

float VirtualKeyboard::whiteKeyX(int index) const
{
    return static_cast<float>(index) * whiteKeyWidth();
}

int VirtualKeyboard::whiteKeyIndex(int note) const
{
    int idx = 0;
    for (int n = kFirstNote; n < note; ++n) {
        if (!isBlackKey(n))
            ++idx;
    }
    return idx;
}

int VirtualKeyboard::noteAtPosition(float localX, float localY) const
{
    const float wkw = whiteKeyWidth();
    const float bkw = blackKeyWidth();
    const float bkh = blackKeyHeight();

    // First check black keys (they have priority)
    if (localY < bkh) {
        for (int note = kFirstNote; note <= kLastNote; ++note) {
            if (!isBlackKey(note))
                continue;

            int leftWhite = note - 1;
            while (leftWhite >= kFirstNote && isBlackKey(leftWhite))
                --leftWhite;

            if (leftWhite < kFirstNote)
                continue;

            int wIdx = whiteKeyIndex(leftWhite);
            float bx = whiteKeyX(wIdx) + wkw - bkw * 0.5f;

            if (localX >= bx && localX < bx + bkw)
                return note;
        }
    }

    // Then check white keys
    for (int note = kFirstNote; note <= kLastNote; ++note) {
        if (isBlackKey(note))
            continue;

        int wIdx = whiteKeyIndex(note);
        float wx = whiteKeyX(wIdx);

        if (localX >= wx && localX < wx + wkw)
            return note;
    }

    return -1;
}

int VirtualKeyboard::keyToNote(uint key) const
{
    // Standard DAW keyboard layout (FL Studio / Ableton style)
    // White keys (bottom row): A-L
    // Black keys (top row): W, E, T, Y, U, O, P
    // Z/X = octave down/up (handled in onKeyboard before this is called)

    int semitone = -1;

    switch (key) {
        // White keys — bottom letter row
        case 'a': return 48 + (octaveOffset_ * 12) + 0;   // C
        case 's': return 48 + (octaveOffset_ * 12) + 2;   // D
        case 'd': return 48 + (octaveOffset_ * 12) + 4;   // E
        case 'f': return 48 + (octaveOffset_ * 12) + 5;   // F
        case 'g': return 48 + (octaveOffset_ * 12) + 7;   // G
        case 'h': return 48 + (octaveOffset_ * 12) + 9;   // A
        case 'j': return 48 + (octaveOffset_ * 12) + 11;  // B
        case 'k': return 48 + (octaveOffset_ * 12) + 12;  // C+1
        case 'l': return 48 + (octaveOffset_ * 12) + 14;  // D+1

        // Black keys — top letter row (between corresponding white keys)
        case 'w': return 48 + (octaveOffset_ * 12) + 1;   // C#
        case 'e': return 48 + (octaveOffset_ * 12) + 3;   // D#
        case 't': return 48 + (octaveOffset_ * 12) + 6;   // F#
        case 'y': return 48 + (octaveOffset_ * 12) + 8;   // G#
        case 'u': return 48 + (octaveOffset_ * 12) + 10;  // A#
        case 'o': return 48 + (octaveOffset_ * 12) + 13;  // C#+1
        case 'p': return 48 + (octaveOffset_ * 12) + 15;  // D#+1

        default: return -1;
    }
}

void VirtualKeyboard::changeOctave(int delta)
{
    // Release all currently held keyboard notes before shifting
    for (auto it = activeKeyMap_.begin(); it != activeKeyMap_.end(); ++it) {
        int note = it->second;
        keyboardNotes_.erase(note);
        if (listener_ != nullptr)
            listener_->keyReleased(this, static_cast<uint8_t>(note));
    }
    activeKeyMap_.clear();

    octaveOffset_ = std::clamp(octaveOffset_ + delta, -2, 5);
    repaint();
}

void VirtualKeyboard::onNanoDisplay()
{
    const float w = getWidth();
    const float h = getHeight();
    const float wkw = whiteKeyWidth();
    const float bkw = blackKeyWidth();
    const float bkh = blackKeyHeight();
    const Color accent = oscAccent();

    // Background
    beginPath();
    roundedRect(0, 0, w, h, 4.0f);
    fillColor(sectionBg());
    fill();

    // Draw white keys
    for (int note = kFirstNote; note <= kLastNote; ++note) {
        if (isBlackKey(note))
            continue;

        int wIdx = whiteKeyIndex(note);
        float kx = whiteKeyX(wIdx);

        bool isActive = (note == activeNote_) ||
                        (activeNotes_.count(note) > 0) ||
                        (keyboardNotes_.count(note) > 0);

        beginPath();
        roundedRect(kx + 0.5f, 1.0f, wkw - 1.0f, h - 2.0f, 2.0f);

        if (isActive) {
            fillColor(Color(accent.red, accent.green, accent.blue, 0.6f));
        } else {
            fillColor(Color(228, 228, 236));
        }
        fill();

        // Border
        beginPath();
        roundedRect(kx + 0.5f, 1.0f, wkw - 1.0f, h - 2.0f, 2.0f);
        strokeColor(Color(50, 50, 65));
        strokeWidth(0.5f);
        stroke();

        // Note labels on C keys
        if (note % 12 == 0) {
            fontSize(8.0f);
            fillColor(Color(80, 80, 100));
            textAlign(ALIGN_CENTER | ALIGN_BOTTOM);
            const char* label = "C";
            int octave = (note / 12) - 1;
            char buf[8];
            std::snprintf(buf, sizeof(buf), "C%d", octave);
            text(kx + wkw * 0.5f, h - 4.0f, buf, nullptr);
        }
    }

    // Draw black keys (on top)
    for (int note = kFirstNote; note <= kLastNote; ++note) {
        if (!isBlackKey(note))
            continue;

        int leftWhite = note - 1;
        while (leftWhite >= kFirstNote && isBlackKey(leftWhite))
            --leftWhite;

        if (leftWhite < kFirstNote)
            continue;

        int wIdx = whiteKeyIndex(leftWhite);
        float bx = whiteKeyX(wIdx) + wkw - bkw * 0.5f;

        bool isActive = (note == activeNote_) ||
                        (activeNotes_.count(note) > 0) ||
                        (keyboardNotes_.count(note) > 0);

        beginPath();
        roundedRect(bx, 1.0f, bkw, bkh, 2.0f);

        if (isActive) {
            fillColor(Color(accent.red, accent.green, accent.blue, 0.85f));
        } else {
            fillColor(Color(22, 22, 32));
        }
        fill();

        beginPath();
        roundedRect(bx, 1.0f, bkw, bkh, 2.0f);
        strokeColor(Color(40, 40, 55));
        strokeWidth(0.5f);
        stroke();
    }

    // Octave indicator (top-left corner)
    {
        int baseOctave = 3 + octaveOffset_;  // offset 0 = C3
        char buf[16];
        std::snprintf(buf, sizeof(buf), "Z/X Oct: C%d", baseOctave);
        fontSize(9.0f);
        fillColor(Color(120, 120, 150));
        textAlign(ALIGN_LEFT | ALIGN_TOP);
        text(6.0f, 4.0f, buf, nullptr);
    }
}

bool VirtualKeyboard::onMouse(const MouseEvent& ev)
{
    if (ev.button != 1)
        return false;

    // ev.pos is already local to this widget (DPF transforms it)
    const float localX = static_cast<float>(ev.pos.getX());
    const float localY = static_cast<float>(ev.pos.getY());

    if (!contains(ev.pos)) {
        if (isMouseDown_ && !ev.press) {
            if (activeNote_ >= 0 && listener_ != nullptr)
                listener_->keyReleased(this, static_cast<uint8_t>(activeNote_));
            activeNote_ = -1;
            isMouseDown_ = false;
            repaint();
            return true;
        }
        return false;
    }

    if (ev.press) {
        isMouseDown_ = true;
        int note = noteAtPosition(localX, localY);
        if (note >= 0 && note != activeNote_) {
            if (activeNote_ >= 0 && listener_ != nullptr)
                listener_->keyReleased(this, static_cast<uint8_t>(activeNote_));

            activeNote_ = note;
            if (listener_ != nullptr)
                listener_->keyPressed(this, static_cast<uint8_t>(note), 100);
            repaint();
        }
        return true;
    } else {
        if (activeNote_ >= 0 && listener_ != nullptr)
            listener_->keyReleased(this, static_cast<uint8_t>(activeNote_));
        activeNote_ = -1;
        isMouseDown_ = false;
        repaint();
        return true;
    }
}

bool VirtualKeyboard::onMotion(const MotionEvent& ev)
{
    if (!isMouseDown_)
        return false;

    // ev.pos is already local to this widget
    const float localX = static_cast<float>(ev.pos.getX());
    const float localY = static_cast<float>(ev.pos.getY());

    int note = noteAtPosition(localX, localY);

    if (note != activeNote_) {
        if (activeNote_ >= 0 && listener_ != nullptr)
            listener_->keyReleased(this, static_cast<uint8_t>(activeNote_));

        activeNote_ = note;

        if (note >= 0 && listener_ != nullptr)
            listener_->keyPressed(this, static_cast<uint8_t>(note), 100);

        repaint();
    }

    return true;
}

bool VirtualKeyboard::onKeyboard(const KeyboardEvent& ev)
{
    // Octave controls: Z = octave down, X = octave up
    if (ev.key == 'z' || ev.key == 'x') {
        if (ev.press) {
            changeOctave(ev.key == 'z' ? -1 : +1);
        }
        return true;  // consume both press and release
    }

    if (ev.press) {
        int note = keyToNote(ev.key);
        if (note < 0 || note > 127)
            return false;

        // Avoid retriggering if already held
        if (keyboardNotes_.count(note) > 0)
            return true;

        // Store keycode→note mapping for robust release handling
        // (ev.key may differ between press and release on some platforms)
        activeKeyMap_[ev.keycode] = note;
        keyboardNotes_.insert(note);
        if (listener_ != nullptr)
            listener_->keyPressed(this, static_cast<uint8_t>(note), 100);
    } else {
        // On release, look up by keycode instead of ev.key
        auto it = activeKeyMap_.find(ev.keycode);
        if (it == activeKeyMap_.end())
            return false;

        int note = it->second;
        activeKeyMap_.erase(it);
        keyboardNotes_.erase(note);
        if (listener_ != nullptr)
            listener_->keyReleased(this, static_cast<uint8_t>(note));
    }

    repaint();
    return true;
}

END_NAMESPACE_DGL
