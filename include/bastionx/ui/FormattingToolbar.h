#ifndef BASTIONX_UI_FORMATTINGTOOLBAR_H
#define BASTIONX_UI_FORMATTINGTOOLBAR_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>

namespace bastionx {
namespace ui {

class FormattingToolbar : public QWidget {
    Q_OBJECT

public:
    explicit FormattingToolbar(QTextEdit* editor, QWidget* parent = nullptr);

private slots:
    void onBold();
    void onItalic();
    void onUnderline();
    void onStrikethrough();
    void onHeading(int level);
    void onBulletList();
    void onNumberedList();
    void onBlockquote();
    void onCodeBlock();
    void onHorizontalRule();
    void updateButtonStates();

private:
    void setupUi();
    QPushButton* makeButton(const QString& text, const QString& tooltip);
    QWidget* makeSeparator();
    QWidget* makeGroupSeparator();
    void setButtonActive(QPushButton* btn, bool active);

    // State cache to prevent redundant polish/unpolish
    struct ButtonState {
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool strike = false;
        int heading_level = 0;
        bool bullet_list = false;
        bool numbered_list = false;
        bool blockquote = false;
        bool code_block = false;
    };
    ButtonState last_state_;

    QTextEdit* editor_;
    QPushButton* bold_btn_ = nullptr;
    QPushButton* italic_btn_ = nullptr;
    QPushButton* underline_btn_ = nullptr;
    QPushButton* strike_btn_ = nullptr;
    QPushButton* h1_btn_ = nullptr;
    QPushButton* h2_btn_ = nullptr;
    QPushButton* h3_btn_ = nullptr;
    QPushButton* bullet_btn_ = nullptr;
    QPushButton* numbered_btn_ = nullptr;
    QPushButton* quote_btn_ = nullptr;
    QPushButton* code_btn_ = nullptr;
    QPushButton* hr_btn_ = nullptr;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_FORMATTINGTOOLBAR_H
