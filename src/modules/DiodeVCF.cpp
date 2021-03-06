#include <dsp/common.hpp>
#include "../LindenbergResearch.hpp"
#include "../dsp/DiodeLadder.hpp"
#include "../dsp/Hardclip.hpp"
#include "../LRModel.hpp"


using namespace rack;
using namespace lrt;

using lrt::DiodeLadderFilter;

struct DiodeVCFWidget;


struct DiodeVCF : LRModule {
    enum ParamIds {
        FREQUENCY_PARAM,
        RES_PARAM,
        SATURATE_PARAM,
        FREQUENCY_CV_PARAM,
        RESONANCE_CV_PARAM,
        SATURATE_CV_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FILTER_INPUT,
        FREQUCENCY_CV_INPUT,
        RESONANCE_CV_INPUT,
        SATURATE_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LP_OUTPUT,
        HP_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    DiodeVCF() : LRModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        configParam(FREQUENCY_PARAM, 0.f, 1.0f, 0.f);
        configParam(RES_PARAM, 0.0f, 1.0, 0.0f);
        configParam(SATURATE_PARAM, 0.f, 1.0, 0.0f);

        configParam(FREQUENCY_CV_PARAM, -1.f, 1.0f, 0.f);
        configParam(RESONANCE_CV_PARAM, -1.f, 1.0f, 0.f);
        configParam(SATURATE_CV_PARAM, -1.f, 1.0f, 0.f);
    }


    DiodeLadderFilter *lpf = new DiodeLadderFilter(APP->engine->getSampleRate());
    LRPanel *panel;

    DiodeVCFWidget *reflect;

    bool aged = false;
    bool hidef = false;

    void process(const ProcessArgs &args) override;
    void onSampleRateChange() override;
};


void DiodeVCF::onSampleRateChange() {
    Module::onSampleRateChange();
    lpf->setSamplerate(APP->engine->getSampleRate());
}


/**
 * @brief Blank Panel with Logo
 */
struct DiodeVCFWidget : LRModuleWidget {
    LRBigKnob *frqKnob, *resKnob;
    LRMiddleKnob *saturateKnob;


    DiodeVCFWidget(DiodeVCF *module) : LRModuleWidget(module) {
        panel->addSVGVariant(LRGestaltType::DARK, APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/DiodeLadderVCFClassic.svg")));
        panel->addSVGVariant(LRGestaltType::LIGHT, APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/DiodeLadderVCF.svg")));
        panel->addSVGVariant(LRGestaltType::AGED, APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/DiodeLadderVCFAged.svg")));

        panel->init();
        addChild(panel);

        box.size = panel->box.size;

        // reflect module widget
        if (!isPreview) module->reflect = this;

        // ***** SCREWS **********
//        addChild(createWidget<ScrewLight>(Vec(15, 1)));
//        addChild(createWidget<ScrewLight>(Vec(box.size.x - 30, 1)));
//        addChild(createWidget<ScrewLight>(Vec(15, 366)));
//        addChild(createWidget<ScrewLight>(Vec(box.size.x - 30, 366)));
        // ***** SCREWS **********

        // ***** MAIN KNOBS ******
        frqKnob = createParam<LRBigKnob>(Vec(32.6, 75.5), module, DiodeVCF::FREQUENCY_PARAM);
        resKnob = createParam<LRBigKnob>(Vec(151.6, 75.5), module, DiodeVCF::RES_PARAM);
        saturateKnob = createParam<LRMiddleKnob>(Vec(99.5, 164.8), module, DiodeVCF::SATURATE_PARAM);

        addParam(frqKnob);
        addParam(resKnob);
        addParam(saturateKnob);

        addParam(createParam<LRSmallKnob>(Vec(39.9, 251.4), module, DiodeVCF::FREQUENCY_CV_PARAM));
        addParam(createParam<LRSmallKnob>(Vec(177, 251.4), module, DiodeVCF::RESONANCE_CV_PARAM));
        addParam(createParam<LRSmallKnob>(Vec(108.5, 251.4), module, DiodeVCF::SATURATE_CV_PARAM));
        // ***** MAIN KNOBS ******

        // ***** CV INPUTS *******
        addInput(createInput<LRIOPortCV>(Vec(37.4, 284.4), module, DiodeVCF::FREQUCENCY_CV_INPUT));
        addInput(createInput<LRIOPortCV>(Vec(175.3, 284.4), module, DiodeVCF::RESONANCE_CV_INPUT));
        addInput(createInput<LRIOPortCV>(Vec(106.4, 284.4), module, DiodeVCF::SATURATE_CV_INPUT));
        // ***** CV INPUTS *******


        // ***** INPUTS **********
        addInput(createInput<LRIOPortAudio>(Vec(37.4, 318.5), module, DiodeVCF::FILTER_INPUT));
        // ***** INPUTS **********

        // ***** OUTPUTS *********
        addOutput(createOutput<LRIOPortAudio>(Vec(175.3, 318.5), module, DiodeVCF::LP_OUTPUT));
        addOutput(createOutput<LRIOPortAudio>(Vec(106.4, 318.5), module, DiodeVCF::HP_OUTPUT));
        // ***** OUTPUTS *********
    }
};


void DiodeVCF::process(const ProcessArgs &args) {
    float freqcv = 0, rescv = 0, satcv = 0;

    if (inputs[FREQUCENCY_CV_INPUT].isConnected()) {
        freqcv = inputs[FREQUCENCY_CV_INPUT].getVoltage() / 10 * dsp::quadraticBipolar(params[FREQUENCY_CV_PARAM].getValue());
    }


    if (inputs[RESONANCE_CV_INPUT].isConnected()) {
        rescv = inputs[RESONANCE_CV_INPUT].getVoltage() / 10 * dsp::quadraticBipolar(params[RESONANCE_CV_PARAM].getValue());
    }

    if (inputs[SATURATE_CV_INPUT].isConnected()) {
        satcv = inputs[SATURATE_CV_INPUT].getVoltage() / 10 * dsp::quadraticBipolar(params[SATURATE_CV_PARAM].getValue());
    }

    float frq = clamp(params[FREQUENCY_PARAM].getValue() + freqcv, 0.f, 1.f);
    float res = clamp((params[RES_PARAM].getValue() + rescv) * DiodeLadderFilter::MAX_RESONANCE, 0.f, DiodeLadderFilter::MAX_RESONANCE);
    float sat = clamp(dsp::quarticBipolar((params[SATURATE_PARAM].getValue()) + satcv) * 14 + 1, 0.f, 15.f);

    reflect->frqKnob->setIndicatorActive(inputs[FREQUCENCY_CV_INPUT].isConnected());
    reflect->resKnob->setIndicatorActive(inputs[RESONANCE_CV_INPUT].isConnected());
    reflect->saturateKnob->setIndicatorActive(inputs[SATURATE_CV_INPUT].isConnected());

    reflect->frqKnob->setIndicatorValue(params[FREQUENCY_PARAM].getValue() + freqcv);
    reflect->resKnob->setIndicatorValue(params[RES_PARAM].getValue() + rescv);
    reflect->saturateKnob->setIndicatorValue(params[SATURATE_PARAM].getValue() + satcv);

    lpf->setFrequency(frq);
    lpf->setResonance(res);
    lpf->setSaturation(sat);

    lpf->low = !hidef;

    lpf->setIn(inputs[FILTER_INPUT].getVoltage() / 10.f);
    lpf->invalidate();
    lpf->process();

    /* compensate gain drop on resonance inc.
    float q = params[RES_PARAM].getValue() * 1.8f + 1;*/

    outputs[HP_OUTPUT].setVoltage(lpf->getOut2() * 6.5f);  // hipass
    outputs[LP_OUTPUT].setVoltage(lpf->getOut() * 10.f);   // lowpass
}

//TODO: [2019-05-23 10:32] => recover oversampling menu

Model *modelDiodeVCF = createModel<DiodeVCF, DiodeVCFWidget>("DIODE_VCF");
