#include "widgets/counter_widget.hpp"
#include "imgui.h"
#include <iostream>

void CounterWidget::render(entt::registry &registry, gui::Hooks &h) {
    // example of state management
    auto [count, set_count] = h.use_state<int>(0);

    // example of callback
    auto click = h.use_callback<std::function<void()>, int>(
        [&]() { set_count(count + 1); }, {count});

    // this will be called whenever the "count" changed
    h.use_effect<int>(
        [&]() {
            std::cout << "clicked: " << count << std::endl;
            return std::nullopt;
        },
        {count});
    ImGui::Begin("Counter");
    ImGui::Text("Count: %d", count);

    if (ImGui::Button("Click")) {
        click();
    }
    ImGui::End();
}