#include <chrono>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

int main() {
    // OpenGL settings we request for:
    sf::ContextSettings settings;
    settings.depthBits = 24;   // 24-bit depth buffer
    settings.stencilBits = 8;  // 8-bit stencil buffer
    // Request OpenGL 2.1 for compatibility with both SFML/Graphics AND macOS:
    //  - SFML's Graphics module implementation relies on legacy fixed-function pipeline features
    //     of OpenGL, which means only up to OpenGL 2.1. Any newer version requires Compatibility
    //     profile (i.e., maintaining legacy features).
    //  - macOS has deprecated OpenGL support, meaning for anything newer than 2.1 (i.e., 3.0 and
    //      newer), only Core profile is supported, i.e., REMOVING legacy fixed-function features.
    // As a result, if you want to both support macOS AND use SFML's Graphics features (e.g.,
    // sf::RenderWindow), you can use at latest OpenGL 2.1, which is a nightmare.
    settings.majorVersion = 2;
    settings.minorVersion = 1;
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Shaders", sf::State::Windowed, settings);
    // Query the settings we actually received (may be different):
    const auto& actualSettings = window.getSettings();
    std::cout << "OpenGL version " << actualSettings.majorVersion << "."
              << actualSettings.minorVersion << " flags: " << actualSettings.attributeFlags << '\n';
    std::cout << "Controls: [B] toggle blur, [A] toggle animation\n";
    // Fullscreen triangle, as described in the prep.
    sf::VertexArray fullscreenTriangle(sf::PrimitiveType::Triangles, 3);
    fullscreenTriangle[0].position = {-1, -1};  // bottom-left
    fullscreenTriangle[1].position = {3, -1};   // bottom-right
    fullscreenTriangle[2].position = {-1, 3};   // top-left
    fullscreenTriangle[0].texCoords = {0, 0};
    fullscreenTriangle[1].texCoords = {2, 0};
    fullscreenTriangle[2].texCoords = {0, 2};
    // load vertex and fragment shaders:
    sf::Shader shader(std::filesystem::path{"main.vert"}, std::filesystem::path{"main.frag"});
    sf::Shader blurShader(std::filesystem::path{"main.vert"}, std::filesystem::path{"blur.frag"});

    // Offscreen target the scene is rendered into before post-processing.
    sf::RenderTexture preBlurRenderTexture(window.getSize());

    const auto startTime = std::chrono::steady_clock::now();
    bool blurEnabled = true;
    bool animateEnabled = true;

    while (window.isOpen()) {
        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>()) {
                return 0;
            } else if (const auto* resized = e->getIf<sf::Event::Resized>()) {
                // Keep the offscreen target the same size as the window.
                if (!preBlurRenderTexture.resize(resized->size)) {
                    std::cerr << "Failed to resize render texture\n";
                    return 1;
                }
            } else if (const auto* key = e->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::B) blurEnabled = !blurEnabled;
                if (key->code == sf::Keyboard::Key::A) animateEnabled = !animateEnabled;
            }
        }

        const sf::Vector2u size = window.getSize();
        const sf::Vector2f sizeF(static_cast<float>(size.x), static_cast<float>(size.y));

        // Problem 2: SFML's mouse position has its origin at the top-left, but gl_FragCoord has
        // its origin at the bottom-left, so flip y.
        const sf::Vector2i mouse = sf::Mouse::getPosition(window);
        const sf::Vector2f mouseGl(static_cast<float>(mouse.x),
                                   sizeF.y - static_cast<float>(mouse.y));

        // Problem 3: elapsed time since program start (ms via chrono, passed as seconds).
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - startTime)
                                   .count();
        const float timeSeconds = static_cast<float>(elapsedMs) / 1000.f;

        shader.setUniform("windowSize", sizeF);
        shader.setUniform("mousePos", mouseGl);
        shader.setUniform("time", timeSeconds);
        shader.setUniform("animate", animateEnabled);

        if (blurEnabled) {
            // Problem 4: render the scene offscreen, then post-process it into the window.
            preBlurRenderTexture.clear(sf::Color::Black);
            preBlurRenderTexture.draw(fullscreenTriangle, &shader);
            preBlurRenderTexture.display();

            blurShader.setUniform("texture", preBlurRenderTexture.getTexture());
            blurShader.setUniform("textureSize", sizeF);

            window.clear(sf::Color::Black);
            window.draw(fullscreenTriangle, &blurShader);
        } else {
            window.clear(sf::Color::Black);
            window.draw(fullscreenTriangle, &shader);
        }
        window.display();
    }
    return 0;
}
