#include <QApplication>
#include <sodium.h>
#include <filesystem>
#include <cstdlib>
#include "bastionx/ui/MainWindow.h"
#include "ui/StyleSheet.h"

int main(int argc, char* argv[]) {
    if (sodium_init() < 0) {
        return 1;
    }

    // Ensure %APPDATA%/Bastionx/ directory exists
    const char* appdata_env = std::getenv("APPDATA");
    if (!appdata_env) {
        return 1;
    }
    std::filesystem::path vault_dir = std::filesystem::path(appdata_env) / "Bastionx";
    std::filesystem::create_directories(vault_dir);
    std::string vault_path = (vault_dir / "vault.db").string();

    QApplication app(argc, argv);
    app.setApplicationName("Bastionx");
    app.setApplicationVersion("0.1.0");
    app.setStyleSheet(bastionx::ui::kStyleSheet);

    bastionx::ui::MainWindow window(vault_path);
    window.setMinimumSize(900, 600);
    window.setWindowTitle("Bastionx");
    window.show();

    return app.exec();
}
