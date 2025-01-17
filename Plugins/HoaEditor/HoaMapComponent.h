/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef __HOA_MAP_PLUGIN__
#define __HOA_MAP_PLUGIN__

#include <JuceHeader.h>
#include "../../../Sources/Hoa2D/Hoa2D.h"

#ifdef __APPLE__

#include "PluginProcessor.h"

#elif _WINDOWS

#ifdef HOA_QUADRA
#include "..\HoaQuadra\Source\PluginProcessor.h"
#endif
#ifdef HOA_OCTO
#include "..\HoaOcto\Source\PluginProcessor.h"
#endif
#ifdef HOA_BINAURAL
#include "..\HoaBinaural\Source\PluginProcessor.h"
#endif
#ifdef HOA_FIVEDOTONE
#include "..\HoaFiveDotOne\Source\PluginProcessor.h"
#endif
#ifdef HOA_HEXA
#include "..\HoaHexa\Source\PluginProcessor.h"
#endif
#ifdef HOA_HEXADECA
#include "..\HoaHexaDeca\Source\PluginProcessor.h"
#endif

#endif

class HoaMapComponent : public Component
{
private:
    SourcesManager*         m_sources;
	AudioProcessorEditor*	m_master;
    HoaToolsAudioProcessor* m_processor;
    double                  m_sources_size;
    long                    m_source_selected;
    long                    m_drag_mode;
    
    void draw_background(Graphics& g);
    void draw_sources(Graphics& g);
    
public:
    HoaMapComponent(AudioProcessorEditor* master, HoaToolsAudioProcessor* processor);
    ~HoaMapComponent();
    
    void paint(Graphics& g);
    
    
    void mouseMove(const MouseEvent &event);
    void mouseDown(const MouseEvent &event);
    void mouseDrag(const MouseEvent &event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);
};


#endif