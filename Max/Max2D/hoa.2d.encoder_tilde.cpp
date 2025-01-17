/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

/**
 @file      hoa.2d.encoder~.cpp
 @name      hoa.2d.encoder~
 @realname  hoa.2d.encoder~
 @type      object
 @module    hoa
 @author    Julien Colafrancesco, Pierre Guillot, Eliott Paris.
 
 @digest
 A 2d ambisonic encoder.
 
 @description
 <o>hoa.2d.encoder~</o> creates the circular harmonics of a signal depending of a given order (arg 1) and a position on a circle given in radians (right inlet).
 
 @discussion
 <o>hoa.2d.encoder~</o> creates the circular harmonics of a signal depending of a given order (arg 1) and a position on a circle given in radians (right inlet).
 
 @category ambisonics, hoa objects, audio, MSP
 
 @seealso hoa.2d.map~, hoa.2d.decoder~, hoa.3d.encoder~
 */

#include "Hoa2D.max.h"

typedef struct _hoa_encoder 
{
	t_pxobject      f_ob;
    double*         f_signals;
    Encoder*        f_encoder;
} t_hoa_encoder;

void *hoa_encoder_new(t_symbol *s, long argc, t_atom *argv);
void hoa_encoder_free(t_hoa_encoder *x);
void hoa_encoder_assist(t_hoa_encoder *x, void *b, long m, long a, char *s);

void hoa_encoder_float(t_hoa_encoder *x, double f);
void hoa_encoder_int(t_hoa_encoder *x, long n);

void hoa_encoder_dsp64(t_hoa_encoder *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void hoa_encoder_perform64(t_hoa_encoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void hoa_encoder_perform64_azimuth(t_hoa_encoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

t_hoa_err hoa_getinfos(t_hoa_encoder* x, t_hoa_boxinfos* boxinfos);

t_class *hoa_encoder_class;

#ifdef HOA_PACKED_LIB
int hoa_2d_encoder_main(void)
#else
int C74_EXPORT main(void)
#endif
{
	t_class *c;
	
	c = class_new("hoa.2d.encoder~", (method)hoa_encoder_new, (method)hoa_encoder_free, (long)sizeof(t_hoa_encoder), 0L, A_GIMME,0);
    class_setname((char *)"hoa.2d.encoder~", (char *)"hoa.2d.encoder~");
    
    hoa_initclass(c, (method)hoa_getinfos);

	// @method float @digest Set the azimuth of the encoding in radians
	// @description The <m>float</m> method sets the azimuth of the encoding in radians (between 0. and 2π).
	// @marg 0 @name azimuth @optional 0 @type float
	class_addmethod(c, (method)hoa_encoder_float,		"float",	A_FLOAT, 0);
	
	// @method int @digest Set the azimuth of the encoding in radians
	// @description The <m>int</m> method sets the azimuth of the encoding in radians (between 0 and 2π).
	// @marg 0 @name azimuth @optional 0 @type int
	class_addmethod(c, (method)hoa_encoder_int,         "int",		A_LONG, 0);
	
	// @method signal @digest Signal to encode / azimuth of the encoding in radians.
	// @description In the left inlet, the signal to encode. In The right inlet : set the azimuth of the encoding in radians
	class_addmethod(c, (method)hoa_encoder_dsp64,		"dsp64",	A_CANT, 0);
	class_addmethod(c, (method)hoa_encoder_assist,      "assist",	A_CANT, 0);
	
	class_dspinit(c);
	class_register(CLASS_BOX, c);
    class_alias(c, gensym("hoa.encoder~"));
	hoa_encoder_class = c;

	return 0;
}

void *hoa_encoder_new(t_symbol *s, long argc, t_atom *argv)
{
	// @arg 0 @name ambisonic-order @optional 0 @type int @digest The ambisonic order of decomposition
	// @description First argument is the ambisonic order of decomposition.
	t_hoa_encoder *x = NULL;
	int	order = 1;
    x = (t_hoa_encoder *)object_alloc(hoa_encoder_class);
	if (x)
	{		
		if(argc && argv && atom_gettype(argv) == A_LONG)
			order = clip_min(atom_getlong(argv), 1);
        
		x->f_encoder = new Hoa2D::Encoder(order);
		
		dsp_setup((t_pxobject *)x, 2);
		for (int i = 0; i < x->f_encoder->getNumberOfHarmonics(); i++)
			outlet_new(x, "signal");
		
        x->f_signals =  new double[x->f_encoder->getNumberOfHarmonics() * SYS_MAXBLKSIZE];
	}

	return (x);
}

t_hoa_err hoa_getinfos(t_hoa_encoder* x, t_hoa_boxinfos* boxinfos)
{
	boxinfos->object_type = HOA_OBJECT_2D;
	boxinfos->autoconnect_inputs = 0;
	boxinfos->autoconnect_outputs = x->f_encoder->getNumberOfHarmonics();
	boxinfos->autoconnect_inputs_type = HOA_CONNECT_TYPE_STANDARD;
	boxinfos->autoconnect_outputs_type = HOA_CONNECT_TYPE_AMBISONICS;
	return HOA_ERR_NONE;
}

void hoa_encoder_float(t_hoa_encoder *x, double f)
{
	x->f_encoder->setAzimuth(f);
}

void hoa_encoder_int(t_hoa_encoder *x, long n)
{
	x->f_encoder->setAzimuth(n);
}

void hoa_encoder_dsp64(t_hoa_encoder *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if(count[1])
        object_method(dsp64, gensym("dsp_add64"), x, hoa_encoder_perform64_azimuth, 0, NULL);
    else
        object_method(dsp64, gensym("dsp_add64"), x, hoa_encoder_perform64, 0, NULL);
}

void hoa_encoder_perform64_azimuth(t_hoa_encoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	for(int i = 0; i < sampleframes; i++)
    {
        x->f_encoder->setAzimuth(ins[1][i]);
        x->f_encoder->process(ins[0][i], x->f_signals + numouts * i);
    }
    for(int i = 0; i < numouts; i++)
    {
        cblas_dcopy(sampleframes, x->f_signals+i, numouts, outs[i], 1);
    }
}

void hoa_encoder_perform64(t_hoa_encoder *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	for(int i = 0; i < sampleframes; i++)
    {
        x->f_encoder->process(ins[0][i], x->f_signals + numouts * i);
    }
    for(int i = 0; i < numouts; i++)
    {
        cblas_dcopy(sampleframes, x->f_signals+i, numouts, outs[i], 1);
    }
}

void hoa_encoder_assist(t_hoa_encoder *x, void *b, long m, long a, char *s)
{
	if(m == ASSIST_INLET)
	{
        if(a == 0)
            sprintf(s,"(signal) Input");
        else
            sprintf(s,"(signal/float) Azimuth");
	} 
	else 
	{
		sprintf(s,"(signal) %s", x->f_encoder->getHarmonicName(a).c_str());
	}
}


void hoa_encoder_free(t_hoa_encoder *x) 
{
	dsp_free((t_pxobject *)x);
	delete x->f_encoder;
    delete [] x->f_signals;
}

