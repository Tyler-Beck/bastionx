#include "bastionx/ui/ClipboardGuard.h"
#include <QApplication>
#include <QClipboard>
#include <sodium.h>

namespace bastionx {
namespace ui {

ClipboardGuard::ClipboardGuard(QObject* parent)
    : QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &ClipboardGuard::onTimerExpired);

    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &ClipboardGuard::onClipboardChanged);
}

void ClipboardGuard::setEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled) {
        timer_->stop();
        tracking_ = false;
    }
}

bool ClipboardGuard::isEnabled() const {
    return enabled_;
}

void ClipboardGuard::setClearSeconds(int seconds) {
    clear_seconds_ = seconds;
}

int ClipboardGuard::clearSeconds() const {
    return clear_seconds_;
}

void ClipboardGuard::clearNow() {
    if (tracking_) {
        QString current = QApplication::clipboard()->text();
        if (!current.isEmpty()) {
            auto current_hash = hashText(current);
            if (current_hash == last_hash_) {
                QApplication::clipboard()->clear();
            }
        }
        tracking_ = false;
        timer_->stop();
    }
}

void ClipboardGuard::onClipboardChanged() {
    if (!enabled_) return;

    QString text = QApplication::clipboard()->text();
    if (text.isEmpty()) {
        tracking_ = false;
        timer_->stop();
        return;
    }

    last_hash_ = hashText(text);
    tracking_ = true;
    timer_->start(clear_seconds_ * 1000);
}

void ClipboardGuard::onTimerExpired() {
    if (!tracking_) return;

    QString current = QApplication::clipboard()->text();
    if (!current.isEmpty()) {
        auto current_hash = hashText(current);
        if (current_hash == last_hash_) {
            QApplication::clipboard()->clear();
        }
    }
    tracking_ = false;
}

std::array<uint8_t, ClipboardGuard::kHashBytes> ClipboardGuard::hashText(const QString& text) {
    std::array<uint8_t, kHashBytes> hash{};
    QByteArray utf8 = text.toUtf8();
    crypto_generichash(hash.data(), kHashBytes,
                       reinterpret_cast<const unsigned char*>(utf8.constData()),
                       static_cast<unsigned long long>(utf8.size()),
                       nullptr, 0);
    return hash;
}

}  // namespace ui
}  // namespace bastionx
