#ifndef ARCHIVATOR_VIEW_H
#define ARCHIVATOR_VIEW_H
#ifdef _WIN32
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include <Windows.h>
#else

#include <gtk/gtk.h>

#endif
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <filesystem>


class View {
private:
    static std::vector<std::wstring> selectFiles(const std::string &initialPath) {
        std::vector<std::wstring> selectedFiles;
#ifdef _WIN32
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr)) {
            std::cerr << "Failed to initialize COM" << std::endl;
            return selectedFiles;
        }

        IFileOpenDialog* pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        if (FAILED(hr)) {
            std::cerr << "Failed to create FileOpenDialog instance" << std::endl;
            CoUninitialize();
            return selectedFiles;
        }

        DWORD dwOptions;
        hr = pFileOpen->GetOptions(&dwOptions);
        if (SUCCEEDED(hr)) {
            hr = pFileOpen->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST);
            if (FAILED(hr)) {
                std::cerr << "Failed to set dialog options" << std::endl;
                pFileOpen->Release();
                CoUninitialize();
                return selectedFiles;
            }
        }

        if (!initialPath.empty()) {
            IShellItem* pFolder;
            hr = SHCreateItemFromParsingName(std::wstring(initialPath.begin(), initialPath.end()).c_str(), NULL, IID_IShellItem, reinterpret_cast<void**>(&pFolder));
            if (SUCCEEDED(hr)) {
                pFileOpen->SetFolder(pFolder);
                pFolder->Release();
            }
        }

        hr = pFileOpen->Show(NULL);.
        if (SUCCEEDED(hr)) {
            IShellItemArray* pItems;
            hr = pFileOpen->GetResults(&pItems);
            if (SUCCEEDED(hr)) {
                DWORD dwNumItems;
                hr = pItems->GetCount(&dwNumItems);
                if (SUCCEEDED(hr)) {
                    // Обработка выбранных файлов
                    for (DWORD i = 0; i < dwNumItems; ++i) {
                        IShellItem* pItem;
                        hr = pItems->GetItemAt(i, &pItem);
                        if (SUCCEEDED(hr)) {
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            if (SUCCEEDED(hr)) {
                                selectedFiles.push_back(pszFilePath);
                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                }
                pItems->Release();
            }
        }

        pFileOpen->Release();
        CoUninitialize();
#else
        gtk_init(nullptr, nullptr);
        // Create the file chooser dialog directly
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Files",
                                                        nullptr,
                                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                                        "_Cancel",
                                                        GTK_RESPONSE_CANCEL,
                                                        "_Open",
                                                        GTK_RESPONSE_ACCEPT,
                                                        nullptr);
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        // Set the file chooser to allow selecting multiple files
        gtk_file_chooser_set_select_multiple(chooser, TRUE);

        // Run the dialog and wait for user interaction
        gint res = gtk_dialog_run(GTK_DIALOG(dialog));
        if (res == GTK_RESPONSE_ACCEPT) {
            GSList *files = gtk_file_chooser_get_filenames(chooser);
            for (GSList *iter = files; iter != nullptr; iter = iter->next) {
                const char *file_path = static_cast<const char *>(iter->data);
                fs::path chosen_file_path = fs::canonical(fs::path(file_path));
                fs::path current_dir = fs::current_path();
                fs::path relative_path = fs::relative(chosen_file_path, current_dir);
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter(new std::codecvt_utf8_utf16<wchar_t>);
                std::wstring wstr = converter.from_bytes(relative_path.string());
                selectedFiles.push_back(wstr);
            }
            // Free the list of file paths
            g_slist_free(files);
        }
        gtk_widget_destroy(dialog);
#endif

        return selectedFiles;
    }

    class Button : public sf::Drawable, public sf::Transformable {
    public:
        Button(const std::string &text, const sf::Vector2f &position, bool a, bool selected, sf::Font &font) : m_text(
                text, font) {
            m_sprite.setFillColor(sf::Color::White);
            m_sprite.setOutlineThickness(2);
            m_sprite.setOutlineColor(sf::Color::Black);
            if (a) {
                m_sprite.setSize(sf::Vector2f(100, 50));
                m_text.setCharacterSize(32);
                m_text.setPosition(position + sf::Vector2f(10, 5));
            } else {
                m_sprite.setSize(sf::Vector2f(300, 100));
                m_text.setCharacterSize(64);
                m_text.setPosition(position + sf::Vector2f(static_cast<float >(250 - 35 * text.size()), 10));
            }
            m_sprite.setPosition(position);

            m_text.setFillColor(sf::Color::Black);
            setSelected(selected);
        }

        bool isPressed() const {
            return m_isSelected;
        }

        void setSelected(bool selected) {
            m_isSelected = selected;
            if (selected) {
                m_sprite.setFillColor(sf::Color::Blue);
            } else {
                m_sprite.setFillColor(sf::Color::White);
            }
        }

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
            target.draw(m_sprite, states);
            target.draw(m_text, states);
        }

        sf::FloatRect getGlobalBounds() const {
            return m_sprite.getGlobalBounds();
        }

    private:
        sf::RectangleShape m_sprite;
        sf::Text m_text;
        bool m_isSelected{};
    };

public:
    View() = default;

    static void start() {
        sf::RenderWindow window(sf::VideoMode(1200, 800), "SFML Window");

        sf::Font font;
        if (!font.loadFromFile("Consolas.ttf")) {
            std::cerr << "Font loading error!" << std::endl;
        }

        sf::VertexArray arrowUp1(sf::Triangles, 3);
        arrowUp1[0].position = sf::Vector2f(732, 80);
        arrowUp1[1].position = sf::Vector2f(752, 110);
        arrowUp1[2].position = sf::Vector2f(712, 110);
        arrowUp1[0].color = sf::Color::Blue;
        arrowUp1[1].color = sf::Color::Blue;
        arrowUp1[2].color = sf::Color::Blue;

        sf::VertexArray arrowDown1(sf::Triangles, 3);
        arrowDown1[0].position = sf::Vector2f(732, 380);
        arrowDown1[1].position = sf::Vector2f(752, 350);
        arrowDown1[2].position = sf::Vector2f(712, 350);
        arrowDown1[0].color = sf::Color::Blue;
        arrowDown1[1].color = sf::Color::Blue;
        arrowDown1[2].color = sf::Color::Blue;

        sf::VertexArray arrowUp2(sf::Triangles, 3);
        arrowUp2[0].position = sf::Vector2f(732, 470);
        arrowUp2[1].position = sf::Vector2f(752, 500);
        arrowUp2[2].position = sf::Vector2f(712, 500);
        arrowUp2[0].color = sf::Color::Blue;
        arrowUp2[1].color = sf::Color::Blue;
        arrowUp2[2].color = sf::Color::Blue;

        sf::VertexArray arrowDown2(sf::Triangles, 3);
        arrowDown2[0].position = sf::Vector2f(732, 770);
        arrowDown2[1].position = sf::Vector2f(752, 740);
        arrowDown2[2].position = sf::Vector2f(712, 740);
        arrowDown2[0].color = sf::Color::Blue;
        arrowDown2[1].color = sf::Color::Blue;
        arrowDown2[2].color = sf::Color::Blue;


        sf::Text inputText;
        inputText.setFont(font);
        inputText.setCharacterSize(24);
        inputText.setFillColor(sf::Color::Black);
        inputText.setPosition(70, 90);

        sf::Text outputText;
        outputText.setFont(font);
        outputText.setCharacterSize(24);
        outputText.setFillColor(sf::Color::Black);
        outputText.setPosition(70, 470);

        sf::Text outputFilesText;
        outputFilesText.setFont(font);
        outputFilesText.setCharacterSize(24);
        outputFilesText.setFillColor(sf::Color::Black);
        outputFilesText.setPosition(840, 480);

        sf::Text inText("Input", font);
        inText.setCharacterSize(30);
        inText.setFillColor(sf::Color::Black);
        inText.setPosition(60, 40);

        sf::Text outText("Output", font);
        outText.setCharacterSize(30);
        outText.setFillColor(sf::Color::Black);
        outText.setPosition(60, 420);

        sf::Text outFileText("Output files", font);
        outFileText.setCharacterSize(30);
        outFileText.setFillColor(sf::Color::Black);
        outFileText.setPosition(865, 420);

        sf::RectangleShape inputArea(sf::Vector2f(700, 300));
        inputArea.setPosition(60, 80);
        inputArea.setFillColor(sf::Color::Transparent);
        inputArea.setOutlineThickness(5);
        inputArea.setOutlineColor(sf::Color::Black);

        sf::RectangleShape outputArea(sf::Vector2f(700, 300));
        outputArea.setPosition(60, 470);
        outputArea.setFillColor(sf::Color::Transparent);
        outputArea.setOutlineThickness(5);
        outputArea.setOutlineColor(sf::Color::Black);

        sf::RectangleShape rectangle1(sf::Vector2f(55, 300));
        rectangle1.setPosition(705, 80);
        rectangle1.setFillColor(sf::Color::Transparent);
        rectangle1.setOutlineThickness(3);
        rectangle1.setOutlineColor(sf::Color::Black);

        sf::RectangleShape rectangle2(sf::Vector2f(55, 300));
        rectangle2.setPosition(705, 470);
        rectangle2.setFillColor(sf::Color::Transparent);
        rectangle2.setOutlineThickness(3);
        rectangle2.setOutlineColor(sf::Color::Black);

        sf::RectangleShape outfileArea(sf::Vector2f(300, 300));
        outfileArea.setPosition(830, 470);
        outfileArea.setFillColor(sf::Color::Transparent);
        outfileArea.setOutlineThickness(3);
        outfileArea.setOutlineColor(sf::Color::Black);

        Button textOutButton("Text", sf::Vector2f(480, 400), true, true, font);
        Button fileOutButton("Files", sf::Vector2f(580, 400), true, false, font);

        Button browseButton("Browse", sf::Vector2f(830, 80), false, false, font);
        Button startButton("Start", sf::Vector2f(830, 280), false, false, font);

        std::string inputStr;
        std::string outputFilesStr;
        std::string outStr;

        int writeArea = 0;
        int shift1 = 0;
        int shift2 = 0;

        while (window.isOpen()) {
            sf::Event event{};
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed) {  // Ctrl+V
                    if (event.key.code == sf::Keyboard::V && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                        std::string str = sf::Clipboard::getString().toAnsiString();
                        if (writeArea == 0) {
                            inputStr += str;
                            printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                        } else {
                            outputFilesStr += str;
                            printText(outputFilesStr, outputFilesText, sf::Vector2f(300, 300));
                        }
                    }
                } else if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode < 128) {
                        if (event.text.unicode == 22) {
                            continue;
                        }
                        if (event.text.unicode == '\b') { // Backspace
                            if (writeArea == 0) {
                                if (!inputStr.empty()) {
                                    inputStr.pop_back();
                                    printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                                }
                            } else {
                                if (!outputFilesStr.empty()) {
                                    outputFilesStr.pop_back();
                                    printText(outputFilesStr, outputFilesText, sf::Vector2f(300, 300));
                                }
                            }
                        } else if (event.text.unicode == '\r') { // Enter
                            if (writeArea == 0) {
                                inputStr += '\n';
                                printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                            } else {
                                outputFilesStr += '\n';
                                printText(outputFilesStr, outputFilesText, sf::Vector2f(300, 300));
                            }
                        } else {
                            if (writeArea == 0) {
                                inputStr += static_cast<char>(event.text.unicode);
                                printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                            } else {
                                outputFilesStr += static_cast<char>(event.text.unicode);
                                printText(outputFilesStr, outputFilesText, sf::Vector2f(300, 300));
                            }
                        }
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) { // Mouse
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        if (textOutButton.getGlobalBounds().contains(static_cast<float >(event.mouseButton.x),
                                                                     static_cast<float >(event.mouseButton.y))) {
                            textOutButton.setSelected(true);
                            fileOutButton.setSelected(false);
                        } else if (fileOutButton.getGlobalBounds().contains(static_cast<float >(event.mouseButton.x),
                                                                            static_cast<float >(event.mouseButton.y))) {
                            fileOutButton.setSelected(true);
                            textOutButton.setSelected(false);
                        } else if (startButton.getGlobalBounds().contains(static_cast<float >(event.mouseButton.x),
                                                                          static_cast<float >(event.mouseButton.y))) {
                            bool isTextOutput = textOutButton.isPressed();
                            std::string outputFile;
                            if (!isTextOutput) {
                                outputFile = outputFilesStr;
                            }
                            std::ostringstream oss;
                            std::ostringstream &ref_oss = oss;
                            Controller controller{textOutButton.isPressed(), outputFile, ref_oss};
                            controller.start(inputStr);
                            outStr = oss.str();
                            printText(outStr, outputText, sf::Vector2f(700, 300));
                        } else if (sf::FloatRect(710, 80, 40, 40).contains(static_cast<float >(event.mouseButton.x),
                                                                           static_cast<float >(event.mouseButton.y))) {
                            if (shift1 != 0) {
                                --shift1;
                                printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                            }
                        } else if (sf::FloatRect(710, 340, 40, 40).contains(static_cast<float >(event.mouseButton.x),
                                                                            static_cast<float >( event.mouseButton.y))) {
                            ++shift1;
                            printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                        } else if (sf::FloatRect(710, 480, 40, 40).contains(static_cast<float >(event.mouseButton.x),
                                                                            static_cast<float >(event.mouseButton.y))) {
                            if (shift2 != 0) {
                                --shift2;
                                printText(outStr, outputText, sf::Vector2f(660, 300), shift2);
                            }
                        } else if (sf::FloatRect(710, 740, 40, 40).contains(static_cast<float >(event.mouseButton.x),
                                                                            static_cast<float >(event.mouseButton.y))) {
                            ++shift2;
                            printText(outStr, outputText, sf::Vector2f(700, 300), shift2);
                        } else if (inputArea.getGlobalBounds().contains(static_cast<float >(event.mouseButton.x),
                                                                        static_cast<float >(event.mouseButton.y))) {
                            writeArea = 0;
                        } else if (outfileArea.getGlobalBounds().contains(static_cast<float >(event.mouseButton.x),
                                                                          static_cast<float >( event.mouseButton.y))) {
                            writeArea = 1;
                        } else if (browseButton.getGlobalBounds().contains(static_cast<float >(event.mouseButton.x),

                                                                           static_cast<float >(event.mouseButton.y))) {
                            std::vector<std::wstring> selectedFiles = selectFiles("");
                            for (std::wstring filepath: selectedFiles) {
                                std::string str(filepath.begin(), filepath.end());
                                inputStr += ' ' + str;
                            }
                            printText(inputStr, inputText, sf::Vector2f(660, 300), shift1);
                        }
                    }
                }
            }

            window.clear(sf::Color::White);
            window.draw(inputArea);
            window.draw(outputArea);
            window.draw(outfileArea);

            window.draw(inputText);
            window.draw(outputText);
            window.draw(outputFilesText);

            window.draw(outFileText);
            window.draw(inText);
            window.draw(outText);
            window.draw(textOutButton);
            window.draw(fileOutButton);
            window.draw(browseButton);
            window.draw(startButton);

            window.draw(arrowUp1);
            window.draw(arrowDown1);
            window.draw(arrowUp2);
            window.draw(arrowDown2);

            window.draw(rectangle1);
            window.draw(rectangle2);

            window.display();
        }
    }

    static void printText(const std::string &str, sf::Text &outputText, const sf::Vector2f &area, int shift = 0) {
        int line = 0;
        int maxLine = static_cast<int>(area.y / 1.25 / 24) + shift;
        int maxLen = static_cast<int>(area.x / 24 * 1.75);
        std::vector<int> size(maxLine, 0);
        std::string outStr;
        for (char c: str) {
            if (c == '\n') {
                if (++line > shift) {
                    if (line < maxLine) {
                        outStr += '\n';
                    }
                }
            } else {
                if (line >= shift) {
                    if (line > maxLine - 1) {
                        continue;
                    }
                    if (size[line] >= maxLen) {
                        outStr += '\n';
                        ++line;
                    }
                    outStr += c;
                } else {
                    if (size[line] >= maxLen) {
                        if (++line >= shift) {
                            outStr += c;
                        }
                    }
                }
                ++size[line];
            }
        }
        outputText.setString(outStr);
    }
};

#endif ARCHIVATOR_VIEW_H