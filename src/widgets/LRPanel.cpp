#include <window.hpp>
#include "../LRComponents.hpp"

namespace lrt {

/**
 * @brief Extention for panel background
 * @param vg
 */
void LRPanel::draw(NVGcontext *vg) {
    FramebufferWidget::draw(vg);
}


/**
 * @brief Setup UI interface
 * @param gestalt Pointer to gestalt id
 * @param gradient Pointer to grandient flag
 * @param patina Pointer to patina flag
 */
void LRPanel::setupGestalt(LRGestalt *gestalt, bool *gradient, bool *patina) {
    this->gestalt = gestalt;
    this->gradient = gradient;
    this->patina = patina;
}


/**
 * @brief Constructor
 */
LRPanel::LRPanel() {
    panelWidget = new SVGWidget();
    addChild(panelWidget);
}


/**
 * @brief Initialize a Panel and setup gestalt and UI
 */
void LRPanel::init() {
    auto svg = getSVGVariant(NIL); // INIT

    if (svg != nullptr) {
        panelWidget->setSVG(svg);
    }


    box.size = panelWidget->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);

    /* setup patina widget */
    patinaWidget = new LRPatinaWidget("res/panels/LaikaPatina.svg", box.size);
    patinaWidget->randomize();
    patinaWidget->visible = *patina;
    addChild(patinaWidget);

    /* setup gradient variants */
    auto gradientDark = new LRGradientWidget(box.size, nvgRGBAf(0.5, 0.5f, 0.6f, 0.33f), nvgRGBAf(0.0f, 0.0f, 0.0f, 0.2f), Vec(-100, -30));
    gradientDark->visible = false;
    addChild(gradientDark);
    gradients.push_back(gradientDark);

    auto gradientLight = new LRGradientWidget(box.size, nvgRGBAf(0.3, 0.3, 0.f, 0.09f), nvgRGBAf(0.f, 0.f, 0.f, 0.7f), Vec(-10, -10));
    gradientLight->visible = false;
    addChild(gradientLight);
    gradients.push_back(gradientLight);

    auto gradientAged = new LRGradientWidget(box.size, nvgRGBAf(0.5, 0.5, 0.f, 0.1f), nvgRGBAf(0.f, 0.f, 0.f, 0.73f), Vec(-10, -10));
    gradientAged->visible = false;
    addChild(gradientAged);
    gradients.push_back(gradientAged);

    auto *pb = new PanelBorder();
    pb->box.size = box.size;
    addChild(pb);

    dirty = true;
}


/**
 * @brief Set the gradient for the current variant on or off
 * @param invert Automaticaly invert state
 */
void LRPanel::setGradientVariant(bool invert) {
    *gradient = invert == !*gradient;

    for (auto i = 0u; i < gradients.size(); i++) {
        if (*gestalt - 1 == i) gradients[i]->visible = *gradient;
        else gradients[i]->visible = false;
        //debug("gradient:%i gest:%i index:%i isVisable:%i", *gradient, *gestalt, i, gradients[i]->visible);
    }

    dirty = true;
}


/**
 * @brief Setup patina on / off
 * @param enabled
 */
void LRPanel::setPatina(bool enabled) {
    *patina = enabled;
    patinaWidget->visible = *patina;
    dirty = true;
}


/**
 * @brief One frame increment from render engine triggered
 */
void LRPanel::step() {
    if (isNear(gPixelRatio, 1.0)) {
        // Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
        oversample = 2.0;
    }

    if (invalidGestalt()) {
        //debug("gestalt about to change: %i ==> %i", this->prevID, *this->gestalt);
        auto svg = getSVGVariant(*gestalt - 1);

        if (svg != nullptr) {
            panelWidget->setSVG(svg);
            box.size = panelWidget->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
        }

        setGradientVariant(false);
        setPatina(*patina);

        dirty = true;
        syncGestalt();
    }

}


}