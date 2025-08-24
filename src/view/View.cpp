#include <view/View.hpp>

#include "controller/Controller.hpp"
#ifdef _WIN32
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include <Windows.h>
#else
#include <gtk/gtk.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::wstring> View::select_files(const std::string &initial_path) {
    std::vector<std::wstring> selected_files;
#ifdef _WIN32
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return selected_files;
    }

    IFileOpenDialog *pFileOpen;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                          reinterpret_cast<void **>(&pFileOpen));
    if (FAILED(hr)) {
        std::cerr << "Failed to create FileOpenDialog instance" << std::endl;
        CoUninitialize();
        return selected_files;
    }

    DWORD dwOptions;
    hr = pFileOpen->GetOptions(&dwOptions);
    if (SUCCEEDED(hr)) {
        hr = pFileOpen->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST);
        if (FAILED(hr)) {
            std::cerr << "Failed to set dialog options" << std::endl;
            pFileOpen->Release();
            CoUninitialize();
            return selected_files;
        }
    }

    if (!initial_path.empty()) {
        IShellItem *pFolder;
        hr = SHCreateItemFromParsingName(std::wstring(initial_path.begin(), initial_path.end()).c_str(), NULL,
                                         IID_IShellItem, reinterpret_cast<void **>(&pFolder));
        if (SUCCEEDED(hr)) {
            pFileOpen->SetFolder(pFolder);
            pFolder->Release();
        }
    }

    hr = pFileOpen->Show(NULL);.
    if (SUCCEEDED(hr)) {
        IShellItemArray *pItems;
        hr = pFileOpen->GetResults(&pItems);
        if (SUCCEEDED(hr)) {
            DWORD dwNumItems;
            hr = pItems->GetCount(&dwNumItems);
            if (SUCCEEDED(hr)) {
                // Обработка выбранных файлов
                for (DWORD i = 0; i < dwNumItems; ++i) {
                    IShellItem *pItem;
                    hr = pItems->GetItemAt(i, &pItem);
                    if (SUCCEEDED(hr)) {
                        PWSTR pszFilePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr)) {
                            selected_files.push_back(pszFilePath);
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
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter(new std::codecvt_utf8_utf16<wchar_t>);
            std::wstring wstr = converter.from_bytes(relative_path.string());
            selected_files.push_back(wstr);
        }
        // Free the list of file paths
        g_slist_free(files);
    }
    gtk_widget_destroy(dialog);
#endif

    return selected_files;
}

View::Button::Button(const std::string &text, const sf::Vector2f &position, bool a, bool selected,
                     const sf::Font &font) : m_text_(
    text, font) {
    m_sprite_.setFillColor(sf::Color::White);
    m_sprite_.setOutlineThickness(2);
    m_sprite_.setOutlineColor(sf::Color::Black);
    if (a) {
        m_sprite_.setSize(sf::Vector2f(100, 50));
        m_text_.setCharacterSize(32);
        m_text_.setPosition(position + sf::Vector2f(10, 5));
    } else {
        m_sprite_.setSize(sf::Vector2f(300, 100));
        m_text_.setCharacterSize(64);
        m_text_.setPosition(position + sf::Vector2f(static_cast<float>(250 - 35 * text.size()), 10));
    }
    m_sprite_.setPosition(position);

    m_text_.setFillColor(sf::Color::Black);
    set_selected(selected);
}

bool View::Button::is_pressed() const {
    return m_is_selected_;
}

void View::Button::set_selected(bool selected) {
    m_is_selected_ = selected;
    if (selected) {
        m_sprite_.setFillColor(sf::Color::Blue);
    } else {
        m_sprite_.setFillColor(sf::Color::White);
    }
}

void View::Button::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    target.draw(m_sprite_, states);
    target.draw(m_text_, states);
}

sf::FloatRect View::Button::get_global_bounds() const {
    return m_sprite_.getGlobalBounds();
}

void View::start() {
    sf::RenderWindow window(sf::VideoMode(1200, 800), "SFML Window");

    sf::Font font;
    if (!font.loadFromFile("../Consolas.ttf")) {
        std::cerr << "Font loading error!" << std::endl;
    }

    sf::VertexArray arrow_up1(sf::Triangles, 3);
    arrow_up1[0].position = sf::Vector2f(732, 80);
    arrow_up1[1].position = sf::Vector2f(752, 110);
    arrow_up1[2].position = sf::Vector2f(712, 110);
    arrow_up1[0].color = sf::Color::Blue;
    arrow_up1[1].color = sf::Color::Blue;
    arrow_up1[2].color = sf::Color::Blue;

    sf::VertexArray arrow_down1(sf::Triangles, 3);
    arrow_down1[0].position = sf::Vector2f(732, 380);
    arrow_down1[1].position = sf::Vector2f(752, 350);
    arrow_down1[2].position = sf::Vector2f(712, 350);
    arrow_down1[0].color = sf::Color::Blue;
    arrow_down1[1].color = sf::Color::Blue;
    arrow_down1[2].color = sf::Color::Blue;

    sf::VertexArray arrow_up2(sf::Triangles, 3);
    arrow_up2[0].position = sf::Vector2f(732, 470);
    arrow_up2[1].position = sf::Vector2f(752, 500);
    arrow_up2[2].position = sf::Vector2f(712, 500);
    arrow_up2[0].color = sf::Color::Blue;
    arrow_up2[1].color = sf::Color::Blue;
    arrow_up2[2].color = sf::Color::Blue;

    sf::VertexArray arrow_down2(sf::Triangles, 3);
    arrow_down2[0].position = sf::Vector2f(732, 770);
    arrow_down2[1].position = sf::Vector2f(752, 740);
    arrow_down2[2].position = sf::Vector2f(712, 740);
    arrow_down2[0].color = sf::Color::Blue;
    arrow_down2[1].color = sf::Color::Blue;
    arrow_down2[2].color = sf::Color::Blue;


    sf::Text input_text;
    input_text.setFont(font);
    input_text.setCharacterSize(24);
    input_text.setFillColor(sf::Color::Black);
    input_text.setPosition(70, 90);

    sf::Text output_text;
    output_text.setFont(font);
    output_text.setCharacterSize(24);
    output_text.setFillColor(sf::Color::Black);
    output_text.setPosition(70, 470);

    sf::Text output_files_text;
    output_files_text.setFont(font);
    output_files_text.setCharacterSize(24);
    output_files_text.setFillColor(sf::Color::Black);
    output_files_text.setPosition(840, 480);

    sf::Text in_text("Input", font);
    in_text.setCharacterSize(30);
    in_text.setFillColor(sf::Color::Black);
    in_text.setPosition(60, 40);

    sf::Text out_text("Output", font);
    out_text.setCharacterSize(30);
    out_text.setFillColor(sf::Color::Black);
    out_text.setPosition(60, 420);

    sf::Text out_file_text("Output files", font);
    out_file_text.setCharacterSize(30);
    out_file_text.setFillColor(sf::Color::Black);
    out_file_text.setPosition(865, 420);

    sf::RectangleShape input_area(sf::Vector2f(700, 300));
    input_area.setPosition(60, 80);
    input_area.setFillColor(sf::Color::Transparent);
    input_area.setOutlineThickness(5);
    input_area.setOutlineColor(sf::Color::Black);

    sf::RectangleShape output_area(sf::Vector2f(700, 300));
    output_area.setPosition(60, 470);
    output_area.setFillColor(sf::Color::Transparent);
    output_area.setOutlineThickness(5);
    output_area.setOutlineColor(sf::Color::Black);

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

    sf::RectangleShape outfile_area(sf::Vector2f(300, 300));
    outfile_area.setPosition(830, 470);
    outfile_area.setFillColor(sf::Color::Transparent);
    outfile_area.setOutlineThickness(3);
    outfile_area.setOutlineColor(sf::Color::Black);

    Button text_out_button("Text", sf::Vector2f(480, 400), true, true, font);
    Button file_out_button("Files", sf::Vector2f(580, 400), true, false, font);

    Button browse_button("Browse", sf::Vector2f(830, 80), false, false, font);
    Button start_button("Start", sf::Vector2f(830, 280), false, false, font);

    std::string input_str;
    std::string output_files_str;
    std::string out_str;

    int write_area = 0;
    int shift1 = 0;
    int shift2 = 0;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                // Ctrl+V
                if (event.key.code == sf::Keyboard::V && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    std::string str = sf::Clipboard::getString().toAnsiString();
                    if (write_area == 0) {
                        input_str += str;
                        print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                    } else {
                        output_files_str += str;
                        print_text(output_files_str, output_files_text, sf::Vector2f(300, 300));
                    }
                }
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    if (event.text.unicode == 22) {
                        continue;
                    }
                    if (event.text.unicode == '\b') {
                        // Backspace
                        if (write_area == 0) {
                            if (!input_str.empty()) {
                                input_str.pop_back();
                                print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                            }
                        } else {
                            if (!output_files_str.empty()) {
                                output_files_str.pop_back();
                                print_text(output_files_str, output_files_text, sf::Vector2f(300, 300));
                            }
                        }
                    } else if (event.text.unicode == '\r') {
                        // Enter
                        if (write_area == 0) {
                            input_str += '\n';
                            print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                        } else {
                            output_files_str += '\n';
                            print_text(output_files_str, output_files_text, sf::Vector2f(300, 300));
                        }
                    } else {
                        if (write_area == 0) {
                            input_str += static_cast<char>(event.text.unicode);
                            print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                        } else {
                            output_files_str += static_cast<char>(event.text.unicode);
                            print_text(output_files_str, output_files_text, sf::Vector2f(300, 300));
                        }
                    }
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                // Mouse
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (text_out_button.get_global_bounds().contains(static_cast<float>(event.mouseButton.x),
                                                                 static_cast<float>(event.mouseButton.y))) {
                        text_out_button.set_selected(true);
                        file_out_button.set_selected(false);
                    } else if (file_out_button.get_global_bounds().contains(static_cast<float>(event.mouseButton.x),
                                                                        static_cast<float>(event.mouseButton.y))) {
                        file_out_button.set_selected(true);
                        text_out_button.set_selected(false);
                    } else if (start_button.get_global_bounds().contains(static_cast<float>(event.mouseButton.x),
                                                                      static_cast<float>(event.mouseButton.y))) {
                        bool is_text_output = text_out_button.is_pressed();
                        std::string output_file;
                        if (!is_text_output) {
                            output_file = output_files_str;
                        }
                        std::ostringstream oss;
                        std::ostringstream &ref_oss = oss;
                        Controller controller{text_out_button.is_pressed(), output_file, ref_oss};
                        controller.start(input_str);
                        out_str = oss.str();
                        print_text(out_str, output_text, sf::Vector2f(700, 300));
                    } else if (sf::FloatRect(710, 80, 40, 40).contains(static_cast<float>(event.mouseButton.x),
                                                                       static_cast<float>(event.mouseButton.y))) {
                        if (shift1 != 0) {
                            --shift1;
                            print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                        }
                    } else if (sf::FloatRect(710, 340, 40, 40).contains(static_cast<float>(event.mouseButton.x),
                                                                        static_cast<float>(event.mouseButton.y))) {
                        ++shift1;
                        print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                    } else if (sf::FloatRect(710, 480, 40, 40).contains(static_cast<float>(event.mouseButton.x),
                                                                        static_cast<float>(event.mouseButton.y))) {
                        if (shift2 != 0) {
                            --shift2;
                            print_text(out_str, output_text, sf::Vector2f(660, 300), shift2);
                        }
                    } else if (sf::FloatRect(710, 740, 40, 40).contains(static_cast<float>(event.mouseButton.x),
                                                                        static_cast<float>(event.mouseButton.y))) {
                        ++shift2;
                        print_text(out_str, output_text, sf::Vector2f(700, 300), shift2);
                    } else if (input_area.getGlobalBounds().contains(static_cast<float>(event.mouseButton.x),
                                                                    static_cast<float>(event.mouseButton.y))) {
                        write_area = 0;
                    } else if (outfile_area.getGlobalBounds().contains(static_cast<float>(event.mouseButton.x),
                                                                      static_cast<float>(event.mouseButton.y))) {
                        write_area = 1;
                    } else if (browse_button.get_global_bounds().contains(static_cast<float>(event.mouseButton.x),

                                                                       static_cast<float>(event.mouseButton.y))) {
                        std::vector<std::wstring> selected_files = select_files("");
                        for (std::wstring filepath: selected_files) {
                            std::string str(filepath.begin(), filepath.end());
                            input_str += ' ' + str;
                        }
                        print_text(input_str, input_text, sf::Vector2f(660, 300), shift1);
                    }
                }
            }
        }

        window.clear(sf::Color::White);
        window.draw(input_area);
        window.draw(output_area);
        window.draw(outfile_area);

        window.draw(input_text);
        window.draw(output_text);
        window.draw(output_files_text);

        window.draw(out_file_text);
        window.draw(in_text);
        window.draw(out_text);
        window.draw(text_out_button);
        window.draw(file_out_button);
        window.draw(browse_button);
        window.draw(start_button);

        window.draw(arrow_up1);
        window.draw(arrow_down1);
        window.draw(arrow_up2);
        window.draw(arrow_down2);

        window.draw(rectangle1);
        window.draw(rectangle2);

        window.display();
    }
}

void View::print_text(const std::string &str, sf::Text &output_text, const sf::Vector2f &area, int shift) {
    int line = 0;
    int max_line = static_cast<int>(area.y / 1.25 / 24) + shift;
    int max_len = static_cast<int>(area.x / 24 * 1.75);
    std::vector<int> size(max_line, 0);
    std::string out_str;
    for (char c: str) {
        if (c == '\n') {
            if (++line > shift) {
                if (line < max_line) {
                    out_str += '\n';
                }
            }
        } else {
            if (line >= shift) {
                if (line > max_line - 1) {
                    continue;
                }
                if (size[line] >= max_len) {
                    out_str += '\n';
                    ++line;
                }
                out_str += c;
            } else {
                if (size[line] >= max_len) {
                    if (++line >= shift) {
                        out_str += c;
                    }
                }
            }
            ++size[line];
        }
    }
    output_text.setString(out_str);
}
