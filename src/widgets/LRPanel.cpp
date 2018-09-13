#include "../LRComponents.hpp"

namespace lrt {

/**
 * @brief Extention for panel background
 * @param vg
 */
void LRPanel::draw(NVGcontext *vg) {
    FramebufferWidget::draw(vg);

    if (colorOnly) {
        nvgBeginPath(vg);
        nvgFillColor(vg, bgColor);
        nvgRect(vg, 0, 0, box.size.x - 0, box.size.y - 0);
        nvgFill(vg);
    }

    nvgBeginPath(vg);

    if (limit.isZero()) {
        nvgRect(vg, -MARGIN, -MARGIN, box.size.x + MARGIN * 2, box.size.y + MARGIN * 2);
    } else {
        nvgRect(vg, 0, 0, limit.x, limit.y);
    }

    NVGpaint paint = nvgLinearGradient(vg, offset.x, offset.y, box.size.x, box.size.y, inner, outer);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
}


void LRPanel::setInner(const NVGcolor &inner) {
    LRPanel::inner = inner;
}


void LRPanel::setOuter(const NVGcolor &outer) {
    LRPanel::outer = outer;
}


LRPanel::LRPanel() {}


const NVGcolor &LRPanel::getInner() const {
    return inner;
}


const NVGcolor &LRPanel::getOuter() const {
    return outer;
}

}