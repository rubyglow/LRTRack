#include "LindenbergResearch.hpp"

Plugin *pluginInstance;


void init(Plugin *p) {
    pluginInstance = p;

    p->addModel(modelType35);
    p->addModel(modelDiodeVCF);
    p->addModel(modelBlankPanel);
    p->addModel(modelBlankPanelWood);
    p->addModel(modelVCO);
    p->addModel(modelSimpleFilter);
    p->addModel(modelReShaper);
    p->addModel(modelBlankPanelEmpty);
    p->addModel(modelBlankPanelSmall);
    p->addModel(modelAlmaFilter);
    p->addModel(modelMS20Filter);
    p->addModel(modelWestcoast);
    p->addModel(modelQuickMix);
    p->addModel(modelVULevelMeter);
    p->addModel(modelTestDriver);



#ifdef LRT_DEBUG
    DEBUG("Operating in DEBUG mode...");
    DEBUG("Found %d research modules.", p->models.size());
    #endif


/*






    p->addModel(modelEchoBox);




    //  p->addModel(modelSpeck);




    */
}
