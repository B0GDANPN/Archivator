//
// Created by bogdan on 28.04.24.
//

#ifndef MYARCH_VIEW_H
#define MYARCH_VIEW_H
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../controller/Controller.h"

class View {
private:
    /*std::vector<std::wstring> selectFiles(const std::string& initialPath) {
        std::vector<std::wstring> selectedFiles;

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

        hr = pFileOpen->Show(NULL);
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

        return selectedFiles;
    }*/

    class Button : public sf::Drawable, public sf::Transformable {
    public:
        Button(const std::string &text, const sf::Vector2f &position, bool a, bool selected, sf::Font &font)
                : m_text(text, font) {
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
                m_text.setPosition(position + sf::Vector2f(250 - 35 * text.size(), 10));
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

        bool getSelected() { return m_isSelected; }

        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
            target.draw(m_sprite, states);
            target.draw(m_text, states);
        }

        sf::FloatRect getGlobalBounds() const {
            return m_sprite.getGlobalBounds();
        }

    private:
        sf::RectangleShape m_sprite;
        sf::Text m_text;
        bool m_isSelected;
    };

    sf::Text inputText;
    sf::Text outputText;
public:
    /*void setController(bool isTextOutput,const std::string& outputFile) {
        controller.isTextOutput=isTextOutput;
        controller.outputFile=outputFile;
    }*/

    //View(/*Controller controller*/)/*:controller(controller)*/ {}

    void start() {
        sf::RenderWindow window(sf::VideoMode(1200, 800), "SFML Window");

        sf::Font font;
        if (!font.loadFromFile("Consolas.ttf")) {
            std::cerr << "Font loading error!" << std::endl;
        }

        /*sf::RenderTexture renderTexture;
        if (!renderTexture.create(window.getSize().x, window.getSize().y)) {
            std::cerr << "Texture create error!" << std::endl;
        }
        renderTexture.clear(sf::Color::White);*/

        inputText.setFont(font);
        inputText.setCharacterSize(24);
        inputText.setFillColor(sf::Color::Black);
        inputText.setPosition(90, 90);

        outputText.setFont(font);
        outputText.setCharacterSize(24);
        outputText.setFillColor(sf::Color::Black);
        outputText.setPosition(90, 470);

        sf::Text inText("Input", font);
        inText.setCharacterSize(30);
        inText.setFillColor(sf::Color::Black);
        inText.setPosition(80, 30);

        sf::Text outText("Out", font);
        outText.setCharacterSize(30);
        outText.setFillColor(sf::Color::Black);
        outText.setPosition(80, 410);

        sf::RectangleShape inputArea(sf::Vector2f(700, 300));
        inputArea.setPosition(80, 80);
        inputArea.setFillColor(sf::Color::Transparent);
        inputArea.setOutlineThickness(3);
        inputArea.setOutlineColor(sf::Color::Black);

        sf::RectangleShape outputArea(sf::Vector2f(700, 300));
        outputArea.setPosition(80, 470);
        outputArea.setFillColor(sf::Color::Transparent);
        outputArea.setOutlineThickness(3);
        outputArea.setOutlineColor(sf::Color::Black);

        //Button textInButton("Text", sf::Vector2f(480, 12), true, true, font);
        //Button fileInButton("Files", sf::Vector2f(580, 12), true, false, font);

        Button textOutButton("Text", sf::Vector2f(480, 400), true, true, font);
        Button fileOutButton("Files", sf::Vector2f(580, 400), true, false, font);

        Button browseButton("Browse", sf::Vector2f(850, 200), false, false, font);
        Button startButton("Start", sf::Vector2f(850, 400), false, false, font);

        std::string inputStr;
        std::string showStr;
        int size[10] = {};
        int line = 0;

        while (window.isOpen()) {
            sf::Event event{};
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed) {  // Ctrl+V
                    if (event.key.code == sf::Keyboard::V && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                        std::string str = sf::Clipboard::getString().toAnsiString();
                        for (char c: str) {
                            if (c == '\n') {
                                if (line >= 9) {
                                    continue;
                                }
                                ++line;
                                inputStr += '\n';
                                showStr += '\n';
                            } else {
                                if (line > 9) {
                                    continue;
                                }
                                if (size[line] >= 53) {
                                    if (line >= 9) {
                                        continue;
                                    }
                                    ++line;
                                    showStr += '\n';
                                }
                                ++size[line];
                                inputStr += c;
                                showStr += c;
                            }
                        }
                        inputText.setString(showStr);
                        continue;
                    }
                } else if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode < 128) {
                        if (event.text.unicode == 22) {
                            continue;
                        }
                        if (event.text.unicode == '\b') { // Backspace
                            if (!inputStr.empty()) {
                                if (showStr.back() == '\n') {
                                    if (inputStr.back() != '\n') {
                                        showStr.pop_back();
                                    }
                                    --line;
                                } else {
                                    --size[line];
                                }
                                showStr.pop_back();
                                inputStr.pop_back();
                            }
                        } else if (event.text.unicode == '\r') { // Enter
                            if (line >= 9) {
                                continue;
                            }
                            ++line;
                            inputStr += '\n';
                            showStr += '\n';
                        } else {
                            if (line > 9) {
                                continue;
                            }
                            if (size[line] >= 53) {
                                if (line >= 9) {
                                    continue;
                                }
                                ++line;
                                showStr += '\n';
                            }
                            ++size[line];
                            inputStr += static_cast<char>(event.text.unicode);
                            showStr += static_cast<char>(event.text.unicode);
                        }
                        inputText.setString(showStr);
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) { // Mouse
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        /*if (textInButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            textInButton.setSelected(true);
                            fileInButton.setSelected(false);
                            //controller::setInput(true);
                        } else if (fileInButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            fileInButton.setSelected(true);
                            textInButton.setSelected(false);
                            //controller::setInput(false);
                        } else*/
                        if (textOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            textOutButton.setSelected(true);
                            fileOutButton.setSelected(false);
                            //controller.setOutput(true);
                        } else if (fileOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            fileOutButton.setSelected(true);
                            textOutButton.setSelected(false);
                            //controller.setOutput(false);
                        } else if (startButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            bool isTextOutput = textOutButton.getSelected();
                            std::string outputFile="";
                            if (!isTextOutput){
                                
                            }
                            Controller controller{isTextOutput,outputFile};
                            controller.start(inputStr);
                            //outputText.setString(outputStr);
                        }/* else if (browseButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            std::vector<std::wstring> selectedFiles = selectFiles("");
                            showStr.clear();
                            inputStr.clear();
                            line = 0;
                            for (int i = 0; i < 10; i++) {
                                size[i] = 0;
                            }
                            for (std::wstring filepath : selectedFiles) {
                                for (wchar_t wc : filepath) {
                                    if (size[line] >= 53) {
                                        showStr += '\n';
                                        if (line >= 9) {
                                            break;
                                        }
                                        line++;
                                    }
                                    showStr += static_cast<char>(wc);
                                    inputStr += static_cast<char>(wc);
                                    size[line]++;
                                }
                                if (line >= 9) {
                                    break;
                                } else {
                                    showStr += '\n';
                                    inputStr += '\n';
                                    line++;
                                }
                            }
                            inputText.setString(showStr);
                        }*/
                    }
                }
            }

            window.clear(sf::Color::White);
            window.draw(inputArea);
            window.draw(outputArea);
            window.draw(inputText);
            window.draw(outputText);
            window.draw(inText);
            window.draw(outText);
            //window.draw(textInButton);
            //window.draw(fileInButton);
            window.draw(textOutButton);
            window.draw(fileOutButton);
            window.draw(browseButton);
            window.draw(startButton);

            /*renderTexture.draw(inputText);
            renderTexture.display();

            sf::Sprite sprite(renderTexture.getTexture());

            // Определяем область вывода текстуры (обрезаем)
            sf::IntRect textureRect(0, 0, 690, 290);
            sprite.setTextureRect(textureRect);
            sprite.setPosition(90, 90);

            window.draw(sprite);*/

            window.display();
        }
    }

    void writeOutput(std::string str) {//TODO дописывать
        int line = 0;
        int size[10] = {};
        std::string outStr = "";
        for (char c: str) {
            if (c == '\n') {
                if (line >= 9) {
                    continue;
                }
                ++line;
                outStr += '\n';
            } else {
                if (line > 9) {
                    continue;
                }
                if (size[line] >= 53) {
                    if (line >= 9) {
                        continue;
                    }
                    ++line;
                    outStr += '\n';
                }
                ++size[line];
                outStr += c;
            }
        }
        outputText.setString(outStr);
    }
};

#endif MYARCH_VIEW_H
