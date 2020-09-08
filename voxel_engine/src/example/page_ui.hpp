#ifndef _PAGE_UI_HPP_
#define _PAGE_UI_HPP_

#include "../utils.hpp"
#include "../UI.hpp"

/**
 *\addtogroup example_game
 * @{
 */

/// The type of a function for on-click events
using fn_on_click = function<void()>;

/// The Button class represents a UI button

class Button {
public:
    /// The UI Element of the Button
    UIElement elem;
    /// The text that the Button will display
    string text;
    /// Creates a new button, with a callback when the button was clicked
    Button(UIElement elem, string text, fn_on_click on_click_function);
    /// Trigger a click event
    void on_click(fn_on_click on_click_function);
private:
    fn_on_click on_click_function;
    friend class PageUI;
};

/// The PageUI class represents a UI Page in the game.
class PageUI {
public:
    /// Create a new PageUI
    PageUI();

    /// The list of buttons for this page
    vector<Button> buttons;
    /// The UIElement background for this page
    optional<UIElement> background;
    /// The font for rendering button text
    Font* font;

    /// Click at a given position
    void click(ivec2 position);

    /// Render the Page UI
    void render();
};

/**@}*/

#endif
