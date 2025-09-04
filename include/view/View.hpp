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


/**
 * @brief Graphical user interface controller for the application.
 *
 * Uses SFML to create a windowed interface where users can select files and initiate compression or decompression actions. The View class manages UI elements such as buttons and text display, and bridges user interaction with the Controller logic.
 */
class View {
    /**
     * @brief Open a file selection dialog (Windows-specific).
     * @param initial_path Initial directory path to open in the dialog.
     * @return A vector of wide strings representing selected file paths (could be multiple selections).
     *
     * This static function (available on Windows) invokes a system file picker allowing the user to choose files. The return is a list of file paths chosen. On other platforms, this may not be implemented or may use a different approach.
     */
    static std::vector<std::wstring> select_files(const std::string &initial_path);

    /**
     * @brief UI Button element for SFML interface.
     *
     * Represents an interactive button with text. Inherits from sf::Drawable and sf::Transformable to integrate with SFML's rendering system.
     */
    class Button final : public sf::Drawable, public sf::Transformable {
        sf::RectangleShape m_sprite_; ///< Graphical rectangle shape of the button.
        sf::Text m_text_;            ///< Text label displayed on the button.
        bool m_is_selected_{};       ///< Indicates if the button is currently selected (highlighted/focused).
    public:
        /**
         * @brief Construct a Button UI element.
         * @param text Display text on the button.
         * @param position Coordinate position of the button in the window.
         * @param a Unused parameter (could indicate button type or alignment in a future version).
         * @param selected Initial selected state (true if this button should start as selected/highlighted).
         * @param font SFML font used to render the button text.
         *
         * The constructor sets up the button's rectangle shape, position, text, and selection state. The appearance (size, colors) may be configured within the implementation.
         */
        Button(const std::string &text, const sf::Vector2f &position, bool a, bool selected, const sf::Font &font);

        /**
         * @brief Check if the button is pressed (selected state).
         * @return True if the button is currently in a pressed/selected state, false otherwise.
         *
         * This might reflect whether the button has been clicked or toggled. In this context, "pressed" likely means it has been activated by the user.
         */
        bool is_pressed() const;

        /**
         * @brief Set the selection state of the button.
         * @param selected Pass true to highlight/select the button, false to deselect it.
         *
         * This could change the button's visual appearance (e.g., color change) to indicate focus. It's typically called when navigating the UI or when the user clicks a button.
         */
        void set_selected(bool selected);

        /**
         * @brief Draw the button to the SFML render target (override).
         * @param target Render target (window or texture) to draw onto.
         * @param states Current render states (transform, blend mode, etc.).
         *
         * SFML calls this method to render the button. It draws the button's rectangle and text. This is invoked internally when `window.draw(button)` is called.
         */
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        /**
         * @brief Get the global bounding rectangle of the button.
         * @return A FloatRect defining the position and size of the button in the window coordinates.
         *
         * This is used for event handling (e.g., to detect if a mouse click lies within the button area).
         */
        sf::FloatRect get_global_bounds() const;
    };

public:
    View() = default;

    /**
     * @brief Launch the GUI application.
     *
     * Creates the main window, initializes the interface (buttons for different actions like "Compress" or "Decompress", text areas for output), and enters the event loop.
     * It handles user inputs such as button clicks or file selections and calls the appropriate Controller logic based on user actions.
     * This function runs until the user closes the application window.
     */
    static void start();

    /**
     * @brief Utility to display text within a defined area in the UI.
     * @param str The string to display.
     * @param output_text An SFML Text object that will be updated with the string content.
     * @param area A SFML 2D vector defining the area (width, height) within which to confine the text.
     * @param shift Vertical pixel offset to adjust text position (optional).
     *
     * This static helper formats and sets the given `output_text` to show `str` within the specified area, handling line breaks or clipping as needed. It can be used for multiline output display in the GUI.
     */
    static void print_text(const std::string &str, sf::Text &output_text, const sf::Vector2f &area, int shift = 0);
};

#endif