/**
 * HoaLibrary : A High Order Ambisonics Library
 * Copyright (c) 2012-2013 Julien Colafrancesco, Pierre Guillot, Eliott Paris, CICM, Universite Paris-8.
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/hoalibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *	- Redistributions may not be sold, nor may they be used in a commercial product or activity.
 *  - Redistributions of source code must retain the above copyright notice, 
 *		this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  - Neither the name of the CICM nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DEF_AMBISONICSSPECTRUM
#define DEF_AMBISONICSSPECTRUM

#include "../hoaAmbisonics/Planewaves.h"
#include "../hoaVector/AmbisonicsVector.h"
#include "../CicmLibrary/CicmFilters/CicmFilterBiquad.h"

class AmbisonicsSpectrum : public Planewaves
{
	
private:
    AmbisonicsVector*                   m_vector;
 
    vector <vector <FilterBiquad*> >    m_filter;
    Cicm_Vector_Float**                 m_filtered_signal_float;
    Cicm_Vector_Double**                m_filtered_signal_double;
    Cicm_Vector_Double                  m_frequency;
    
    Cicm_Vector_Double                  m_amplitude;
    Cicm_Vector_Double                  m_abscissa;
    Cicm_Vector_Double                  m_ordinate;
    
    double                              m_vector_values[4];
    double*                             m_loudspeakers_values;
    Cicm_Vector_Double                  m_temp_amplitude_value;
    void initializeFrequencyBands();
    
public:
	AmbisonicsSpectrum(long aNumberOfLoudspeakers = 1, long aNumberOfBands = 3, long aVectorSize = 0, long aSamplingRate = 44100);
    
    
    void setNumberOfLoudspeakers(long aNumberOfLoudspeakers, bool standardOnOff = 0);
    void setLoudspeakerAngle(long anIndex, double anAngle);
    void setFrequencyBand(long anIndex, double aFrequency);
    void setNumberOfBands(long aNumberOfBands);
    
    double getAmplitude(long aBandIndex);
    double getAbscissa(long aBandIndex);
    double getOrdinate(long aBandIndex);
    double getFrequencyBand(long anIndex);
    long   getNumberOfBands();
    
    void setVectorSize(long aVectorSize);
    void setSamplingRate(long aSamplingRate);
    
	~AmbisonicsSpectrum();
	
	/* Perform block sample */
	inline void process(float** inputs)
	{
        for(int h = 0; h < m_vector_size; h++)
        {
        }
	}
    
	inline void process(double** inputs)
	{
        for(int i = 0; i < m_filter.size(); i++)
        {
            for(int j = 0; j < m_number_of_loudspeakers; j++)
            {
                m_filter[i][j]->process(inputs[j], m_filtered_signal_double[i][j]);
            }
        }
        double amplitude;
        for(int i = 0; i < m_filter.size(); i++)
        {
            for(int k = 0; k < m_vector_size; k++)
            {
                amplitude = 0.;
                for(int j = 0; j < m_number_of_loudspeakers; j++)
                {
                    amplitude += fabs(m_filtered_signal_double[i][j][k]);
                }
                if(amplitude > m_temp_amplitude_value[i])
                    m_temp_amplitude_value[i] = amplitude;
            }
        }
    }
    
    inline void tick()
    {
        for(int i = 0; i < m_filter.size(); i++)
        {
            for(int j = 0; j < m_number_of_loudspeakers; j++)
            {
                //m_loudspeakers_values[j] = m_filtered_signal_double[i][j][m_vector_size-1];
                m_loudspeakers_values[j] = m_filtered_signal_double[i][j][0];
            }
            m_vector->process(m_loudspeakers_values, m_vector_values);
            m_abscissa[i] = m_vector_values[2];
            m_ordinate[i] = m_vector_values[3];
            m_amplitude[i] = m_temp_amplitude_value[i];
            m_temp_amplitude_value[i] = 0.;
        }
    }
    
};

#endif