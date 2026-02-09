#ifndef BASTIONX_UI_CLIPBOARDGUARD_H
#define BASTIONX_UI_CLIPBOARDGUARD_H

#include <QObject>
#include <QTimer>
#include <array>
#include <cstdint>

namespace bastionx {
namespace ui {

/**
 * @brief Monitors clipboard and auto-clears sensitive data after a timeout
 *
 * Uses BLAKE2b hash to track ownership: only clears clipboard if the content
 * still matches what we put there (avoids clearing other apps' clipboard content).
 */
class ClipboardGuard : public QObject {
    Q_OBJECT

public:
    static constexpr int kDefaultClearSeconds = 30;
    static constexpr size_t kHashBytes = 32;

    explicit ClipboardGuard(QObject* parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setClearSeconds(int seconds);
    int clearSeconds() const;

    /// Immediately clear clipboard if we own it
    void clearNow();

private slots:
    void onClipboardChanged();
    void onTimerExpired();

private:
    static std::array<uint8_t, kHashBytes> hashText(const QString& text);

    bool enabled_ = true;
    int clear_seconds_ = kDefaultClearSeconds;
    QTimer* timer_ = nullptr;
    std::array<uint8_t, kHashBytes> last_hash_{};
    bool tracking_ = false;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_CLIPBOARDGUARD_H
