#pragma once

#include "rack.hpp"
#include "asset.hpp"
#include "widgets.hpp"

#define LCD_FONT_DIG7 "res/digital-7.ttf"
#define LCD_COLOR_FG nvgRGBA(0x00, 0xE1, 0xE4, 0xFF)
#define LCD_FONTSIZE 8
#define LCD_LETTER_SPACING 0

using namespace rack;


extern Plugin *plugin;


/**
 * @brief Standard LRT module
 */
struct LRModule : Module {
    long cnt = 0;


    /**
     * @brief Overtake default constructor for module to be compatible
     * @param numParams
     * @param numInputs
     * @param numOutputs
     * @param numLights
     */
    LRModule(int numParams, int numInputs, int numOutputs, int numLights = 0) :
            Module(numParams, numInputs, numOutputs, numLights) {}


    void step() override {
        Module::step();

        // increment counter
        cnt++;
    }
};


/**
 * @brief Standard LRT ModuleWidget definition
 */
struct LRModuleWidget : ModuleWidget {
    explicit LRModuleWidget(Module *module);
};


/**
 * @brief Emulation of a LCD monochrome display
 */
struct LCDWidget : Label {
    std::shared_ptr<Font> gLCDFont_DIG7;
    NVGcolor fg;
    NVGcolor bg;
    unsigned char length = 0;


    /**
     * @brief Constructor
     */
    LCDWidget(NVGcolor fg, unsigned char length);


    /**
     * @brief Draw LCD display
     * @param vg
     */
    void draw(NVGcontext *vg) override;
};


/**
 * @brief Indicator for control voltages on knobs
 */
struct Indicator {
    static constexpr float OVERFLOW_THRESOLD = 0.01f;

    /** flag to control drawing */
    bool active = false;

    /** color of indicator */
    NVGcolor normalColor = nvgRGBA(0x00, 0x00, 0x00, 0xBB);
    NVGcolor overflowColor = nvgRGBA(0xBB, 0x00, 0x00, 0xBB);

    /** radius from middle */
    float distance;

    /** normalized control voltage. must between [0..1] */
    float cv = 0.f;

    /** draw angle */
    float angle;
    float angle2;

    /** middle of parent */
    Vec middle;


    /**
     * @brief Init indicator
     * @param distance Radius viewed from the middle
     * @param angle Angle of active knob area
     */
    Indicator(float distance, float angle) {
        Indicator::distance = distance;
        Indicator::angle = angle;

        /** for optimization */
        angle2 = 2 * angle;
    }


    /**
     * @brief Draw routine for cv indicator
     * @param vg
     */
    void draw(NVGcontext *vg);

};


/**
 * @brief Standard LR Shadow
 */
struct LRShadow {
private:
    Rect box;
    float size = 0.65;
    float strength = 1.f;

    /** shadow shift */
    Vec shadowPos = Vec(3, 5);
public:
    /**
     * @brief Set the new offset of the shadow gradient
     * @param x
     * @param y
     */
    void setShadowPosition(float x, float y) {
        shadowPos = Vec(x, y);
    }


    void setBox(const Rect &box);
    void setSize(float size);
    void setStrength(float strength);

    /**
    * @brief Draw shadow for circular knobs
    * @param vg NVGcontext
    * @param strength Alpha value of outside gradient
    * @param size Outer size
    * @param shift XY Offset shift from middle
    */
    void drawShadow(NVGcontext *vg, float strength, float size);

    void draw(NVGcontext *vg);
};


/**
 * @brief The base of all knobs used in LR panels, includes a indicator
 */
struct LRKnob : SVGKnob {
private:
    static constexpr float ANGLE = 0.83f;

    /** setup indicator with default values */
    Indicator idc = Indicator(15.f, ANGLE);
    LRShadow shadow = LRShadow();
public:
    /**
     * @brief Default constructor
     */
    LRKnob() {
        minAngle = -ANGLE * (float) M_PI;
        maxAngle = ANGLE * (float) M_PI;
    }


    /**
     * @brief Set the value of the indicator
     * @param value
     */
    void setIndicatorValue(float value) {
        idc.cv = value;
    }


    /**
     * @brief Switch indicator on/off
     * @param active
     */
    void setIndicatorActive(bool active) {
        idc.active = active;
    }


    /**
     * @brief Get indicator state
     * @return
     */
    bool isIndicatorActive() {
        return idc.active;
    }


    /**
     * @brief Setup distance of indicator from middle
     * @param distance
     */
    void setIndicatorDistance(float distance) {
        idc.distance = distance;
    }


    /**
     * @brief Hook into setSVG() method to setup box dimensions correct for indicator
     * @param svg
     */
    void setSVG(std::shared_ptr<SVG> svg) {
        SVGKnob::setSVG(svg);

        /** inherit dimensions after loaded svg */
        idc.middle = Vec(box.size.x / 2, box.size.y / 2);
        shadow.setBox(box);
    }


    /**
     * @brief Route setter to shadow
     * @param x
     * @param y
     */
    void setShadowPosition(float x, float y) {
        shadow.setShadowPosition(x, y);
    }


    /**
     * @brief Creates a new instance of a LRKnob child
     * @tparam TParamWidget Subclass of LRKnob
     * @param pos Position
     * @param module Module pointer
     * @param paramId Parameter ID
     * @param minValue Min
     * @param maxValue Max
     * @param defaultValue Default
     * @return Pointer to new subclass of LRKnob
     */
    template<class TParamWidget>
    static TParamWidget *create(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
        auto *param = new TParamWidget();
        param->box.pos = pos;
        param->module = module;
        param->paramId = paramId;
        param->setLimits(minValue, maxValue);
        param->setDefaultValue(defaultValue);
        return param;
    }


    /**
     * @brief Draw knob
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        /** shadow */
        shadow.draw(vg);

        /** component */
        FramebufferWidget::draw(vg);

        /** indicator */
        idc.draw(vg);
    }
};


/**
 * @brief Quantize position to odd numbers to simulate a toogle switch
 */
struct LRToggleKnob : SVGKnob {
    LRToggleKnob(float length = 0.45) {
        minAngle = -length * (float) M_PI;
        maxAngle = length * (float) M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/MiddleKnob.svg")));
    }


    void onChange(EventChange &e) override {
        value = round(value);
        SVGKnob::onChange(e);
    }
};


/**
 * @brief Basic middle-sized knob
 */
struct LRBigKnob : LRKnob {
    LRBigKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/BigKnob.svg")));
        setIndicatorDistance(15);
        setShadowPosition(5, 6);
    }
};


/**
 * @brief Basic middle-sized knob
 */
struct LRMiddleKnob : LRKnob {
    LRMiddleKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/MiddleKnob.svg")));
        setIndicatorDistance(12);
        setShadowPosition(4, 4);
    }
};


/**
 * @brief Blue version of the Davies1900h
 */
struct LRSmallKnob : LRKnob {
    LRSmallKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/SmallKnob.svg")));
        setShadowPosition(3, 5);
    }
};


/**
 * @brief Alternative IO Port
 */
struct IOPort : SVGPort {
private:
    LRShadow shadow = LRShadow();

public:
    IOPort() {
        background->svg = SVG::load(assetPlugin(plugin, "res/IOPortB.svg"));
        background->wrap();
        box.size = background->box.size;

        /** inherit dimensions */
        shadow.setBox(box);
        shadow.setSize(0.55);
        shadow.setShadowPosition(2, 2);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shadow.draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Alternative screw head A
 */
struct ScrewDarkA : SVGScrew {
    ScrewDarkA() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/ScrewDark.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};


/**
 * @brief Custom switch based on original Rack files
 */
struct LRSwitch : SVGSwitch, ToggleSwitch {
    LRSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/Switch0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/Switch1.svg")));
    }
};


/**
 * @brief Standard LED Redlight
 */
struct LRRedLight : SmallLight<ModuleLightWidget> {
    LRRedLight();

    void draw(NVGcontext *vg) override;
};
