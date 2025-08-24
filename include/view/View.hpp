#ifndef ARCHIVATOR_VIEW_HPP
#define ARCHIVATOR_VIEW_HPP
#ifdef _WIN32
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include <Windows.h>
#else

#endif

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <filesystem>


class View {
    static std::vector<std::wstring> select_files(const std::string &initial_path);
    class Button final : public sf::Drawable, public sf::Transformable {
        sf::RectangleShape m_sprite_;
        sf::Text m_text_;
        bool m_is_selected_{};
    public:
        Button(const std::string &text, const sf::Vector2f &position, bool a, bool selected, const sf::Font &font);

        bool is_pressed() const ;

        void set_selected(bool selected);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        sf::FloatRect get_global_bounds() const ;
    };

public:
    View() = default;

    static void start();

    static void print_text(const std::string &str, sf::Text &output_text, const sf::Vector2f &area, int shift = 0);
};

#endif