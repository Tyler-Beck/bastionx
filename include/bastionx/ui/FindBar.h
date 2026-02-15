#ifndef BASTIONX_UI_FINDBAR_H
#define BASTIONX_UI_FINDBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QPropertyAnimation>

namespace bastionx {
namespace ui {

class FindBar : public QWidget {
    Q_OBJECT

public:
    explicit FindBar(QTextEdit* editor, QWidget* parent = nullptr);

    void showFind();
    void showReplace();
    void hideBar();

private slots:
    void onFindTextChanged(const QString& text);
    void onFindNext();
    void onFindPrev();
    void onReplace();
    void onReplaceAll();

private:
    void highlightAllMatches();
    void updateMatchLabel();
    void clearHighlights();
    int countMatches() const;

    QTextEdit* editor_;

    QLineEdit* find_input_ = nullptr;
    QLabel* match_label_ = nullptr;
    QPushButton* prev_btn_ = nullptr;
    QPushButton* next_btn_ = nullptr;
    QLineEdit* replace_input_ = nullptr;
    QPushButton* replace_btn_ = nullptr;
    QPushButton* replace_all_btn_ = nullptr;
    QPushButton* close_btn_ = nullptr;

    // Replace row widgets (toggled visibility)
    QWidget* replace_row_ = nullptr;

    // Height animations for smooth transitions
    QPropertyAnimation* height_animation_ = nullptr;
    QPropertyAnimation* height_animation_min_ = nullptr;

    int current_match_ = 0;
    int total_matches_ = 0;
};

}  // namespace ui
}  // namespace bastionx

#endif  // BASTIONX_UI_FINDBAR_H
