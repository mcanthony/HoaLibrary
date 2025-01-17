/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

/**
 @file      hoa.gain~.cpp
 @name      hoa.gain~
 @realname  hoa.gain~
 @type      object
 @module    hoa
 @author    Julien Colafrancesco, Pierre Guillot, Eliott Paris.
 
 @digest
 A multichannel gain~ object.
 
 @description
 <o>hoa.gain~</o> is a multichannel exponential scaling volume slider. hoa.gain~ is a slider that scales signals. it can also make a smooth transition as you move from one value of the slider to the next.
 
 @discussion
 <o>hoa.gain~</o> is a multichannel exponential scaling volume slider. hoa.gain~ is a slider that scales signals. it can also make a smooth transition as you move from one value of the slider to the next.
 
 @category ambisonics, hoa objects, GUI, msp
 
 @seealso gain~, hoa.process~, hoa.fx.gain~
 
 @illustration on @caption the hoa.gain~ GUI object
 @palette yes
 */

#include "HoaCommon.max.h"

class Line
{
	
private:
	long        m_vector_size;
	long        m_sampling_rate;
	
	double      m_value_old;
	double      m_value_new;
	double      m_value_step;
	long        m_counter;
	long        m_ramp;
	
	void init(long vector_size = 0, long sampling_rate = 44100);
	
public:
	Line(long samps = 4410, long vector_size = 0, long sampling_rate = 44100);
	Line(double ms = 100., long vector_size = 0, long sampling_rate = 44100);
	
	inline long		getVectorSize()		const {return m_vector_size;}
	inline long		getSamplingRate()	const {return m_sampling_rate;}
	inline long		getRampInSample()	const {return m_ramp;}
	inline double	getRampInMs()		const {return ((double)m_ramp / (double)m_sampling_rate) * 1000.;}
	inline double	getValue()			const {return m_value_new;}
	
	inline void setSamplingRate(const long sampling_rate) {m_sampling_rate = clip_min(sampling_rate, 1);}
	inline void setVectorSize(const long vector_size)		{m_vector_size = vector_size;}
	
	void setRampInSample(const long samps);
	void setRampInMs(const double ms);
	void setValueDirect(const double value);
	void setValue(const double value);
	void setValueAngleDirect(const double angle);
	void setValueAngle(const double angle);
	
	~Line() {};
	
	/* Perform sample by sample */
	double process();
};

#define MAX_IO 64
#define MIN_IO 1
#define DEF_IO 8

// mouse tracking stuff

static t_pt s_hoaGain_cum;
static double s_hoaGain_startval;

#define INRANGE(v,lo,hi) ((v)<=(hi)&&(v)>=(lo))

#define hoaGain_CORNERSIZE				(6.)

#define hoaGain_STRIPEWIDTH             (2.5)	// stripes width
#define hoaGain_DISPLAYINSET			(4.)	// amount subtracted from rect for value
#define knobMargin                      (2)		// Knob Margin

enum inputmode {
	DECIBELS	= 0,
	AMPLITUDE	= 1,
	MIDI		= 2
};

typedef struct _hoaGain
{
	t_pxjbox	j_box;
	double		j_min;			
	double		j_size;			// number of steps
	double		j_val;
	char		j_relative;		// relative mousing (like orig miller slider, but not like h/uslider)
	char		j_orientation;	// 0 = auto, 1 = horiz, 2 = vertical
	t_jrgba		j_brgba;
    t_jrgba     j_knobcolor;
    t_jrgba     j_stripecolor;
    
    // gain
    Line*		f_amp;
    char        f_inputMode;
    float       f_range[2];
    double      j_valdB;
    double      j_defaultValuedB;
	
    // inputs/output
    void*       f_inlet_val;
    void*       f_outlet_infos;
    
    // options
    double          f_interp;
    t_atom_long     f_numberOfChannels;
    
} t_hoaGain;

t_class	*s_hoaGain_class;

void *hoaGain_new(t_symbol *s, short argc, t_atom *argv);
void hoaGain_free(t_hoaGain *x);
t_max_err hoaGain_notify(t_hoaGain *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void hoaGain_assist(t_hoaGain *x, void *b, long m, long a, char *s);
void hoaGain_preset(t_hoaGain *x);
void hoaGain_tometer(t_hoaGain *x, t_symbol *s, long argc, t_atom *argv);
void HoaGain_reconnect_outlet(t_hoaGain *x);
/* DSP ------------------------------------- */

void hoaGain_dsp64(t_hoaGain *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void hoaGain_perform64(t_hoaGain *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void hoaGain_tick(t_hoaGain *x);
void hoaGain_output_dB(t_hoaGain *x);
/* Paint ------------------------------------- */
long hoaGain_oksize(t_hoaGain *x, t_rect *newrect);
void hoaGain_paint(t_hoaGain *x, t_object *view);
void draw_cursor(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz);
void draw_background(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz);
void draw_valuestripes(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz);
void draw_offstripes(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz);
void draw_dB_grid(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz);
/* Input ------------------------------------- */
void hoaGain_bang(t_hoaGain *x);
void hoaGain_int(t_hoaGain *x, long n);
void hoaGain_float(t_hoaGain *x, double f);
void hoaGain_float_dB(t_hoaGain *x, double dBValue); // for mouse value setting
void hoaGain_set_dB(t_hoaGain *x, double dBValue);
void hoaGain_set(t_hoaGain *x, t_symbol *s, long argc, t_atom *argv);
void hoaGain_setminmax(t_hoaGain *x, t_symbol *s, long argc, t_atom *argv);
void hoaGain_contextValue(t_hoaGain *x, long valueType, double value);
void hoaGain_setInputModeValue(t_hoaGain *x, double value, bool outputTheValue);
double hoaGain_getInputModeValue(t_hoaGain *x);
t_max_err hoaGain_setattr_interp(t_hoaGain *x, t_object *attr, long ac, t_atom *av);
t_max_err hoaGain_setattr_channels(t_hoaGain *x, t_object *attr, long ac, t_atom *av);
void hoaGain_resize_io(t_hoaGain *x, long newNumberOfChannel);
t_max_err hoaGain_setattr_range(t_hoaGain *x, t_object *attr, long ac, t_atom *av);
t_max_err hoaGain_setvalueof(t_hoaGain *x, long ac, t_atom *av);
t_max_err hoaGain_getvalueof(t_hoaGain *x, long *ac, t_atom **av);
/* Utility ------------------------------------- */
void hoaGain_assign(t_hoaGain *x, double f, long notify);
double hoaGain_constrain(t_hoaGain *x, double f);
double hoaGain_constrain_real_value(t_hoaGain *x, double f);
char hoaGain_ishorizontal(t_hoaGain *x, t_rect *r);
double hoaGain_postoval(t_hoaGain *x, t_pt pt, t_rect *r, char ishoriz);
double hoaGain_valtopos(t_hoaGain *x, double val, t_rect *r, char ishoriz);
double hoaGain_dBvaltopos(t_hoaGain *x, double dBval, t_rect *r, char ishoriz);
double hoaGain_dbtoval(t_hoaGain *x, double dBval);
double hoaGain_valtodb(t_hoaGain *x, double val);
/* Mouse ------------------------------------- */
void hoaGain_mousedoubleclick(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers);
void hoaGain_mousedown(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers);
void hoaGain_mousedragdelta(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers);
void hoaGain_mouseup(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers);

void hoaGain_set_gain(t_hoaGain *x);

t_hoa_err hoa_getinfos(t_hoaGain* x, t_hoa_boxinfos* boxinfos);

#ifdef HOA_PACKED_LIB
int hoa_gain_main(void)
#else
int C74_EXPORT main(void)
#endif
{
	t_class *c;

	c = class_new("hoa.gain~", (method)hoaGain_new, (method)hoaGain_free, sizeof(t_hoaGain), (method)NULL, A_GIMME, 0L);
    class_setname((char *)"hoa.gain~", (char *)"hoa.gain~");
    
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
    class_dspinitjbox(c);
	jbox_initclass(c, JBOX_FIXWIDTH | JBOX_COLOR );
	
	hoa_initclass(c, (method)hoa_getinfos);
    
    class_addmethod (c, (method) hoaGain_assist,              "assist",               A_CANT, 0);
    class_addmethod (c, (method) hoaGain_paint,               "paint",                A_CANT, 0);
    
    // @method signal @digest The inputs signals to be scaled by the slider.
	// @description The inputs signals to be scaled by the slider.
	class_addmethod (c, (method) hoaGain_dsp64,               "dsp64",                A_CANT, 0);
    
    // @method int @digest Set the value of the slider
	// @description The <m>int</m> method sets the value of the slider, ramps the output signal to the level corresponding to the new value over the specified ramp time, and outputs the slider's value out the right outlet.
    // @marg 0 @name value @optional 0 @type int
	class_addmethod (c, (method) hoaGain_int,                 "int",                  A_LONG, 0);
    
    // @method float @digest Set the value of the slider
	// @description The <m>float</m> method sets the value of the slider, ramps the output signal to the level corresponding to the new value over the specified ramp time, and outputs the slider's value out the right outlet.
    // @marg 0 @name value @optional 0 @type int
	class_addmethod (c, (method) hoaGain_float,               "float",                A_FLOAT, 0);
    
    // @method contextvalue @digest Set the value of the slider with DeciBels, Amplitude or MIDI value type.
    // @description Set the value of the slider with DeciBels, Amplitude or MIDI value type.
    // @marg 0 @name inputmode @optional 0 @type int @description 0 = DeciBels, 1 = 1mplitude, 2 = MIDI.
    // @marg 1 @name value @optional 0 @type float @description The slider value
    class_addmethod (c, (method) hoaGain_contextValue,        "contextvalue", A_LONG, A_FLOAT, 0);
    
    // @method bang @digest Send current value out right outlet.
    // @description The <m>bang</m> message send current value out right outlet.
	class_addmethod (c, (method) hoaGain_bang,                "bang",                         0);
    
    // @method set @digest Set the value of the slider without outputting slider value.
    // @description The word set, followed by a number, sets the value of the slider, ramps the output signal to the level corresponding to the new value over the specified ramp time, but does not output the slider's value out the right outlet.
    // @marg 1 @name value @optional 0 @type float @description The slider value
	class_addmethod (c, (method) hoaGain_set,                 "set",                  A_GIMME, 0);
    class_addmethod (c, (method) hoaGain_tometer,             "anything",             A_GIMME, 0);
    
    // @method (mouse) @digest click and drag to set the slider outlet.
    // @description Clicking and dragging with the mouse sets the value of the slider, ramps the output signal to the level corresponding to the new value over the specified ramp time, and outputs the slider’s value out the right outlet. double-click to set the slider value to <m>defvaldb</m>
    
    class_addmethod (c, (method) hoaGain_mousedoubleclick,    "mousedoubleclick",     A_CANT, 0);
	class_addmethod (c, (method) hoaGain_mousedown,           "mousedown",            A_CANT, 0);
	class_addmethod (c, (method) hoaGain_mousedragdelta,      "mousedragdelta",       A_CANT, 0);
	class_addmethod (c, (method) hoaGain_mouseup,             "mouseup",              A_CANT, 0);
	class_addmethod (c, (method) hoaGain_getvalueof,          "getvalueof",           A_CANT, 0);
	class_addmethod (c, (method) hoaGain_setvalueof,          "setvalueof",           A_CANT, 0);
	class_addmethod (c, (method) hoaGain_preset,              "preset",                       0);
	class_addmethod (c, (method) hoaGain_notify,              "notify",               A_CANT, 0); 
	class_addmethod (c, (method) hoaGain_oksize,              "oksize",               A_CANT, 0);
    
	CLASS_ATTR_DEFAULT(c,"patching_rect",0, "0. 0. 140. 22.");
	
	CLASS_STICKY_CATEGORY		(c,0,"Value");
	
    CLASS_ATTR_DOUBLE			(c, "interp", 0, t_hoaGain, f_interp);
	CLASS_ATTR_ACCESSORS		(c, "interp",(method)NULL,(method)hoaGain_setattr_interp);
    CLASS_ATTR_LABEL			(c, "interp", 0, "Ramp Time (ms)");
    CLASS_ATTR_FILTER_MIN		(c, "interp", 0);
    // @description Ramp time in milliseconds
	
	CLASS_ATTR_CHAR				(c,"relative", 0, t_hoaGain, j_relative);
	CLASS_ATTR_LABEL			(c,"relative", 0, "Mousing Mode");
	CLASS_ATTR_ENUMINDEX2		(c, "relative", 0, "Absolute", "Relative");
	CLASS_ATTR_BASIC			(c, "relative", 0);
    // @description Mousing can either be <b>absolute</b> or <b>relative</b>
    
    CLASS_ATTR_CHAR				(c,"inputmode", 0, t_hoaGain, f_inputMode);
	CLASS_ATTR_LABEL			(c,"inputmode", 0, "Input Mode");
	CLASS_ATTR_ENUMINDEX3		(c, "inputmode", 0, "DeciBels", "Amplitude", "Midi");
    // @description Input mode can either be in <b>DeciBels</b>, <b>Amplitude</b> or <b>Midi</b>
    
    CLASS_ATTR_DOUBLE			(c, "defvaldb", 0, t_hoaGain, j_defaultValuedB);
    CLASS_ATTR_LABEL			(c, "defvaldb", 0, "Default Value (dB)");
    // @description Default value in <b>DeciBels</b>, <b>Amplitude</b> or <b>Midi</b>
	
	CLASS_STICKY_CATEGORY_CLEAR(c);
	
	CLASS_ATTR_CHAR				(c,"orientation",0,t_hoaGain,j_orientation);
	CLASS_ATTR_LABEL			(c,"orientation",0,"Orientation");
	CLASS_ATTR_ENUMINDEX3		(c,"orientation", 0,"Automatic", "Horizontal", "Vertical");
	CLASS_ATTR_CATEGORY			(c, "orientation", 0, "Appearance");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "orientation", 0, "0");
    // @description Orientation can either be in <b>Automatic</b>, <b>Horizontal</b> or <b>Vertical</b>
	
	CLASS_STICKY_CATEGORY(c, 0, "Color");
    
	CLASS_ATTR_RGBA_LEGACY		(c, "bgcolor", "brgb", 0, t_hoaGain, j_brgba);
	CLASS_ATTR_ALIAS			(c,"bgcolor", "brgba");
	CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c,"bgcolor",0,"0.290196 0.309804 0.301961 1.");
	CLASS_ATTR_STYLE_LABEL		(c, "bgcolor", 0, "rgba", "Background Color");
	class_parameter_register_default_color(c, gensym("bgcolor"), ps_control_text_bg);
	CLASS_ATTR_BASIC			(c, "bgcolor", 0);
    // @description Sets the RGBA values for the background color of the <o>hoa.gain~</o> object
    
    CLASS_ATTR_RGBA				(c, "knobcolor", 0, t_hoaGain, j_knobcolor);
    CLASS_ATTR_STYLE_LABEL      (c, "knobcolor", 0, "rgba","Value Color");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "knobcolor", 0, "0.803922 0.898039 0.909804 1.0");
    // @description Sets the RGBA values for the knob color of the <o>hoa.gain~</o> object
    
    CLASS_ATTR_RGBA				(c, "stripecolor", 0, t_hoaGain, j_stripecolor);
    CLASS_ATTR_STYLE_LABEL      (c, "stripecolor", 0, "rgba","Off Color");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "stripecolor", 0, "0.376471 0.384314 0.4 1.0");
    // @description Sets the RGBA values for the "Off" value stripe color of the <o>hoa.gain~</o> object
	
	CLASS_STICKY_CATEGORY_CLEAR(c);
    
	CLASS_ATTR_INVISIBLE(c, "color", 0);
	CLASS_ATTR_ATTR_PARSE(c, "color","save", USESYM(long), 0, "0");
    // @exclude hoa.gain~

	CLASS_ATTR_DEFAULT_SAVE		(c,"relative",0,"0");
    CLASS_ATTR_DEFAULT_SAVE		(c,"defvaldb",0,"0");
    CLASS_ATTR_DEFAULT_SAVE		(c,"inputmode",0,"0");
    CLASS_ATTR_DEFAULT_SAVE		(c,"interp",0,"20");
	CLASS_ATTR_LABEL			(c,"min", 0, "Output Minimum");
	CLASS_ATTR_LABEL			(c,"mult", 0, "Output Multiplier");
	CLASS_ATTR_STYLE_LABEL		(c,"floatoutput",0,"onoff","Float Output");
	
	CLASS_ATTR_ORDER			(c, "relative",		0, "2");
	CLASS_ATTR_ORDER			(c, "size",			0, "3");
	CLASS_ATTR_ORDER			(c, "min",			0, "4");
	CLASS_ATTR_ORDER			(c, "mult",			0, "5");
    
    CLASS_ATTR_CATEGORY			(c, "channels", 0, "Custom");
    CLASS_ATTR_LONG				(c, "channels", 0, t_hoaGain, f_numberOfChannels);
    CLASS_ATTR_ACCESSORS        (c, "channels", (method)NULL,(method)hoaGain_setattr_channels);
	CLASS_ATTR_ORDER			(c, "channels", 0, "1");
	CLASS_ATTR_LABEL			(c, "channels", 0, "Number of Channels");
	CLASS_ATTR_FILTER_CLIP		(c, "channels", 1, MAX_IO);
	CLASS_ATTR_DEFAULT			(c, "channels", 0, "8");
	CLASS_ATTR_SAVE				(c, "channels", 1);
    
    CLASS_ATTR_CATEGORY			(c, "range", 0, "Value");
    CLASS_ATTR_FLOAT_ARRAY      (c, "range", 0, t_hoaGain, f_range, 2);
    CLASS_ATTR_ACCESSORS        (c, "range", (method)NULL,(method)hoaGain_setattr_range);
	CLASS_ATTR_ORDER			(c, "range", 0, "2");
	CLASS_ATTR_LABEL			(c, "range", 0, "Range (dB)");
	CLASS_ATTR_DEFAULT			(c, "range", 0, "-70. 18.");
	CLASS_ATTR_SAVE             (c, "range", 1);

	class_register(CLASS_BOX, c);
	s_hoaGain_class = c;
    
	return 0;
}

void *hoaGain_new(t_symbol *s, short argc, t_atom *argv)
{
	t_hoaGain* x;
	long flags; 
	t_dictionary *d=NULL;
	
	if (!(d=object_dictionaryarg(argc,argv)))
		return NULL;
	
	x = (t_hoaGain *) object_alloc(s_hoaGain_class);
	if (!x)
		return NULL;

	flags = 0 
			| JBOX_DRAWFIRSTIN 
			| JBOX_NODRAWBOX
			| JBOX_DRAWINLAST			// rbs -- I think we can turn this off
			| JBOX_GROWBOTH
			| JBOX_DRAWBACKGROUND
			| JBOX_MOUSEDRAGDELTA
			;

	x->j_val = 0;
    x->j_defaultValuedB = 0;
    x->j_valdB = x->j_defaultValuedB;
    x->f_interp = 20;
    x->f_numberOfChannels = 8;
    
	jbox_new((t_jbox *)x, flags, argc, argv);
	x->j_box.z_box.b_firstin = (t_object *)x;
    
    x->f_amp = new Line(x->f_interp, sys_getblksize(), sys_getsr());
    
    // inputs
	dsp_setupjbox((t_pxjbox *)x, x->f_numberOfChannels + 1);
    
    // outputs
    x->f_outlet_infos = outlet_new(x, NULL);
    for (int i=0; i < x->f_numberOfChannels; i++)
        outlet_new(x,"signal");
    
    attr_dictionary_process(x,d); // handle attribute args
    
    hoaGain_set_dB(x, x->j_defaultValuedB);
    
    jbox_ready((t_jbox *)x);
    x->j_box.z_misc = Z_NO_INPLACE;
    
	return x;
}

t_hoa_err hoa_getinfos(t_hoaGain* x, t_hoa_boxinfos* boxinfos)
{
	boxinfos->object_type = HOA_OBJECT_STANDARD;
	boxinfos->autoconnect_inputs = x->f_numberOfChannels;
	boxinfos->autoconnect_outputs = x->f_numberOfChannels;
	boxinfos->autoconnect_inputs_type = HOA_CONNECT_TYPE_STANDARD;
	boxinfos->autoconnect_outputs_type = HOA_CONNECT_TYPE_STANDARD;
	return HOA_ERR_NONE;
}

long hoaGain_oksize(t_hoaGain *x, t_rect *newrect)
{
    int isHoriz = hoaGain_ishorizontal(x, newrect);
    long minWidth, minHeight;
    minWidth = minHeight = 10;
    if (isHoriz) minWidth = 30; // horizontal
    else minHeight = 30; // vertical
    if (newrect->width < minWidth) newrect->width = minWidth;
    if (newrect->height < minHeight) newrect->height = minHeight;
    return 0;
}

void hoaGain_assist(t_hoaGain *x, void *b, long m, long a, char *s)
{
    // @in 0 @loop 1 @type signal @digest input signals to be scaled.
    // @in 1 @type float @digest slider value.
    
	if (m == ASSIST_INLET)
    {
        if (a != x->f_numberOfChannels)
            sprintf(s,"(signal) Audio Signal to be scaled (ch %ld)", a+1);
        else
        {
            switch (x->f_inputMode) {
                case DECIBELS:
                    sprintf(s,"(float/int) Set dB Value (%.2f-%.2f)", x->f_range[0], x->f_range[1]);
                    break;
                case AMPLITUDE:
                    sprintf(s,"(float/int) Set Amplitude Value (dB range : %.2f-%.2f)", x->f_range[0], x->f_range[1]);
                    break;
                case MIDI:
                    sprintf(s,"(int) Set Midi Value (dB range : %.2f-%.2f)", x->f_range[0], x->f_range[1]);
                    break;
                    
                default:
                    break;
            }
        }
	}
    
    // @out 0 @loop 1 @type signal @digest output corresponding input scaled signal.
    // @out 1 @type float @digest output slider value.
    
	else
    {
		if (a != x->f_numberOfChannels)
            sprintf(s,"(signal) Scaled Signal (ch %ld)", a+1);
        else
        {
            switch (x->f_inputMode) {
                case DECIBELS:
                    sprintf(s,"(float) dB Value (%.2f-%.2f)", x->f_range[0], x->f_range[1]);
                    break;
                case AMPLITUDE:
                    sprintf(s,"(float) Amplitude Value (dB range : %.2f-%.2f)", x->f_range[0], x->f_range[1]);
                    break;
                case MIDI:
                    sprintf(s,"(int) Midi Value (dB range : %.2f-%.2f)", x->f_range[0], x->f_range[1]);
                    break;
                    
                default:
                    break;
            }
        }
	}
}

void hoaGain_preset(t_hoaGain *x)
{
	void *z;
	if (!(z = gensym("_preset")->s_thing))
		return;
    
    binbuf_vinsert(z,(char*)"osslf",x,object_classname(x), gensym("contextvalue"), x->f_inputMode, hoaGain_getInputModeValue(x));
}

void hoaGain_contextValue(t_hoaGain *x, long valueType, double value)
{
    x->f_inputMode = clip_minmax(valueType, 0, 2);
    hoaGain_setInputModeValue(x, value, true);
}

void hoaGain_setInputModeValue(t_hoaGain *x, double value, bool outputTheValue)
{
    double dBValue = x->j_valdB;

    switch (x->f_inputMode) {
        case DECIBELS :
            dBValue = value;
            break;
        case AMPLITUDE :
            dBValue = atodb(value);
            break;
        case MIDI :
            dBValue = scale(value, 0, 128, -70, 0);
            break;
    }
    
    x->j_val = hoaGain_constrain_real_value(x, dBValue) - x->j_min;
    x->j_valdB = x->j_val + x->j_min;
        
    hoaGain_set_gain(x);
	if (outputTheValue)
		object_notify(x, gensym("modified"), NULL);
    
    jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
	jbox_redraw((t_jbox *)x);
}

void hoaGain_float_dB(t_hoaGain *x, double dBValue)
{
    
    x->j_val = hoaGain_constrain_real_value(x, dBValue) - x->j_min;
    x->j_valdB = x->j_val + x->j_min;
    hoaGain_set_gain(x);
    object_notify(x, gensym("modified"), NULL);
    jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
	jbox_redraw((t_jbox *)x);
    
    hoaGain_bang(x);
}

void hoaGain_set_dB(t_hoaGain *x, double dBValue)
{
    x->j_val = hoaGain_constrain_real_value(x, dBValue) - x->j_min;
    x->j_valdB = x->j_val + x->j_min;
    hoaGain_set_gain(x);
    object_notify(x, gensym("modified"), NULL);
    jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
	jbox_redraw((t_jbox *)x);    
}

double hoaGain_getInputModeValue(t_hoaGain *x)
{
    switch (x->f_inputMode) {
        case DECIBELS :
            return x->j_valdB;
            break;
        case AMPLITUDE :
            return x->j_valdB > -70 ? dbtoa(x->j_valdB) : 0;
            break;
        case MIDI :
            return scale(x->j_valdB, -70, 0, 0, 128);
            break;
        default:
            break;
    }
    return 0;
}

void hoaGain_free(t_hoaGain *x)
{
    dsp_freejbox((t_pxjbox *)x);
	jbox_free((t_jbox *)x);
    delete x->f_amp;
}

/* DSP ------------------------------------- */

void hoaGain_dsp64(t_hoaGain *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->f_amp->setSamplingRate(samplerate);
    x->f_amp->setVectorSize(maxvectorsize);
    object_method(dsp64, gensym("dsp_add64"), x, hoaGain_perform64, 0, NULL);
}
void hoaGain_perform64(t_hoaGain *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	int i, j;
	double gain;
    
    for(i = 0; i < sampleframes; i++)
	{
		gain = x->f_amp->process();
		
		for (j=0; j < x->f_numberOfChannels; j++)
		{
			outs[j][i] = gain * ins[j][i];
		}
	}
}

void hoaGain_set_gain(t_hoaGain *x)
{
    x->f_amp->setValue( x->j_valdB > -70.0f ? dbtoa(x->j_valdB) : 0.0f );
}

/* Paint ------------------------------------- */
void hoaGain_paint(t_hoaGain *x, t_object *view)
{
	t_rect rect;
    char isHoriz;    
	jbox_get_rect_for_view((t_object *)x, view, &rect);
    isHoriz = hoaGain_ishorizontal(x, &rect);
    draw_background(x, view, &rect, isHoriz);
    draw_offstripes(x, view, &rect, isHoriz);
    draw_valuestripes(x, view, &rect, isHoriz);
    draw_cursor(x, view, &rect, isHoriz);
}

void draw_background(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz)
{
    int zerodBpos;
    zerodBpos = hoaGain_dBvaltopos(x, 0, rect, isHoriz);
    
    t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("bg_layer"), rect->width, rect->height);
    
    if (g)
    {
        // background
        jgraphics_rectangle(g, 0, 0, rect->width, rect->height);
        jgraphics_set_source_jrgba(g, &x->j_brgba);
        jgraphics_fill(g);
        
        jgraphics_set_source_jrgba(g, &x->j_knobcolor);
        
        if (isHoriz)
        {
            jgraphics_line_draw_fast(g, zerodBpos - hoaGain_DISPLAYINSET*0.5-knobMargin, knobMargin*0.5, zerodBpos + hoaGain_DISPLAYINSET, knobMargin*0.5, 1);
            jgraphics_line_draw_fast(g, zerodBpos - hoaGain_DISPLAYINSET*0.5-knobMargin, rect->height - knobMargin*0.5, zerodBpos + hoaGain_DISPLAYINSET, rect->height - knobMargin*0.5, 1);
        }
        else
        {
            jgraphics_line_draw_fast(g, knobMargin*0.5, zerodBpos - hoaGain_DISPLAYINSET*0.5-knobMargin, knobMargin*0.5, zerodBpos + hoaGain_DISPLAYINSET, 1);
            jgraphics_line_draw_fast(g, rect->width - knobMargin*0.5, zerodBpos - hoaGain_DISPLAYINSET*0.5-knobMargin, rect->width - knobMargin*0.5, zerodBpos + hoaGain_DISPLAYINSET, 1);
        }
    }
    
    jbox_end_layer((t_object*)x, view, gensym("bg_layer"));
    jbox_paint_layer((t_object *)x, view, gensym("bg_layer"), 0, 0);
}

void draw_offstripes(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz)
{
    t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("offstripe_layer"), rect->width - (knobMargin*2), rect->height - (knobMargin*2));
    
    if (g)
    {
        jgraphics_translate(g, -hoaGain_STRIPEWIDTH, -hoaGain_STRIPEWIDTH);
        jgraphics_set_source_jrgba(g, &x->j_stripecolor);
        jgraphics_diagonal_line_fill(g, hoaGain_STRIPEWIDTH, 0, 0, rect->width, rect->height);
    }
    
    jbox_end_layer((t_object*)x, view, gensym("offstripe_layer"));
    jbox_paint_layer((t_object *)x, view, gensym("offstripe_layer"), knobMargin, knobMargin);
}

void draw_cursor(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz)
{    
	t_jgraphics *g;
    
	int pos = hoaGain_dBvaltopos(x, CLAMP(x->j_valdB, x->f_range[0], x->f_range[1]), rect, isHoriz);
    
    g = jbox_start_layer((t_object *)x, view, gensym("cursor_layer"), rect->width, rect->height);
    
	if (g)
	{
        // draw knob rect
        jgraphics_set_source_jrgba(g, &x->j_knobcolor);
        
        if (isHoriz)
            jgraphics_rectangle(g, pos - hoaGain_DISPLAYINSET*0.5, knobMargin, hoaGain_DISPLAYINSET, rect->height - (knobMargin*2));
        else
            jgraphics_rectangle(g, knobMargin, pos - hoaGain_DISPLAYINSET*0.5, rect->width - (knobMargin*2), hoaGain_DISPLAYINSET);
        
        jgraphics_fill(g);
	}

    jbox_end_layer((t_object*)x, view, gensym("cursor_layer"));
	jbox_paint_layer((t_object *)x, view, gensym("cursor_layer"), 0., 0.);
}

void draw_valuestripes(t_hoaGain *x, t_object *view, t_rect *rect, char isHoriz)
{
    t_jgraphics *g;
    t_rect layer;
    
    int pos = hoaGain_dBvaltopos(x, CLAMP(x->j_valdB, x->f_range[0], x->f_range[1]), rect, isHoriz);
    
    if (isHoriz)
    {
        layer.x = layer.y = knobMargin;
        layer.width = pos - hoaGain_DISPLAYINSET*0.5 - knobMargin*1.5;
        layer.height = rect->height - (knobMargin*2);
    }
    else
    {
        layer.x = knobMargin;
        layer.y = pos + hoaGain_DISPLAYINSET*0.5 + knobMargin*0.5;
        layer.width = rect->width - (knobMargin*2);
        layer.height = rect->height - layer.y - knobMargin;
    }
    
    if (MAX(layer.width, 0) == 0 || MAX(layer.height, 0) == 0)
        return;
    
    g = jbox_start_layer((t_object *)x, view, gensym("valuestripe_layer"), layer.width, layer.height);
    
    if (g)
    {
        if (isHoriz)
            jgraphics_translate(g, -hoaGain_STRIPEWIDTH, -hoaGain_STRIPEWIDTH);
        else
            jgraphics_translate(g, -hoaGain_STRIPEWIDTH, hoaGain_STRIPEWIDTH - pos);
        
        jgraphics_set_source_jrgba(g, &x->j_knobcolor);
        jgraphics_diagonal_line_fill(g, hoaGain_STRIPEWIDTH, 0, 0, rect->width, rect->height);
    }
    
    jbox_end_layer((t_object*)x, view, gensym("valuestripe_layer"));
    jbox_paint_layer((t_object *)x, view, gensym("valuestripe_layer"), layer.x, layer.y);
}

void hoaGain_bang(t_hoaGain *x)
{
    if (x->f_inputMode == MIDI)
        outlet_int(x->f_outlet_infos, (long)hoaGain_getInputModeValue(x));
    else
        outlet_float(x->f_outlet_infos, hoaGain_getInputModeValue(x));
}

void hoaGain_int(t_hoaGain *x, long n)
{
    hoaGain_assign(x, double(n), true);
	hoaGain_bang(x);
}

void hoaGain_float(t_hoaGain *x, double f)
{
    hoaGain_assign(x, f, true);
	hoaGain_bang(x);
}

void hoaGain_setminmax(t_hoaGain *x, t_symbol *s, long argc, t_atom *argv)
{
	double a, b;
	
	if (argc > 1) {
		double old_min = x->j_min;
		double old_size = x->j_size;
		a = b = 0;
		
		if (argv[0].a_type == A_LONG)
			a = (double)atom_getlong(&argv[0]);
		else if (argv[0].a_type == A_FLOAT)
			a = (double)atom_getfloat(&argv[0]);

		if (argv[1].a_type == A_LONG)
			b = (double)atom_getlong(&argv[1]);
		else if (argv[1].a_type == A_FLOAT)
			b = (double)atom_getfloat(&argv[1]);
		
		if (a == b) {
			x->j_min = 0.;
			x->j_size = 1.;
		} else  if (a < b) {
			x->j_min = a;
			x->j_size = b - a;
		} else {			
			x->j_min = b;
			x->j_size = a - b;
		}
        
        x->f_range[0] = x->j_min;
        x->f_range[1] = x->j_size + x->j_min;
		
		if (old_min != x->j_min || old_size != x->j_size)
        {
            jbox_invalidate_layer((t_object *)x, NULL, gensym("bg_layer"));
            jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
            jbox_redraw((t_jbox *)x);
        }
	}
}

void hoaGain_set(t_hoaGain *x, t_symbol *s, long argc, t_atom *argv)
{
	if (argc && (argv->a_type == A_FLOAT || argv->a_type == A_LONG))
		hoaGain_assign(x, atom_getfloat(argv), true);
}

t_max_err hoaGain_setattr_interp(t_hoaGain *x, t_object *attr, long ac, t_atom *av)
{
	double d;
	if (ac && av) {
		d = atom_getfloat(av);
        x->f_amp->setRampInMs(d);
        x->f_interp = x->f_amp->getRampInMs();
	}
	return MAX_ERR_NONE;
}

t_max_err hoaGain_setattr_channels(t_hoaGain *x, t_object *attr, long ac, t_atom *av)
{
	long d;
	if (ac && av)
    {
        if (atom_gettype(av) == A_LONG)
        {
            d = atom_getlong(av);
            hoaGain_resize_io(x, d);
            HoaGain_reconnect_outlet(x);
            hoaGain_tometer(x, gensym("channels"), ac, av);
        }
	}
	return MAX_ERR_NONE;
}

void hoaGain_resize_io(t_hoaGain *x, long newNumberOfChannel)
{
    int dspState = sys_getdspobjdspstate((t_object*)x);
    int lastNumberOfChannels = x->f_numberOfChannels;
    newNumberOfChannel = clip_minmax(newNumberOfChannel, long(MIN_IO), long(MAX_IO));
    if (lastNumberOfChannels != newNumberOfChannel) {
        if(dspState)
            object_method(gensym("dsp")->s_thing, gensym("stop"));
        
        t_object *b = NULL;
		object_obex_lookup(x, gensym("#B"), (t_object **)&b);
        object_method(b, gensym("dynlet_begin"));
        
        dsp_resize((t_pxobject*)x, newNumberOfChannel+1);
        
        outlet_delete(outlet_nth((t_object*)x, lastNumberOfChannels)); // delete value out outlet
        
        if(lastNumberOfChannels > newNumberOfChannel)
        {
            for(int i = lastNumberOfChannels; i > newNumberOfChannel; i--)
            {
                outlet_delete(outlet_nth((t_object*)x, i-1));
            }
        }
        else if(lastNumberOfChannels < newNumberOfChannel)
        {
            for(int i = lastNumberOfChannels; i < newNumberOfChannel; i++)
            {
                outlet_append((t_object*)x, NULL, gensym("signal"));
            }
        }
        
        x->f_outlet_infos = outlet_append((t_object*)x, NULL, NULL); // restore value out outlet
        
        object_method(b, gensym("dynlet_end"));
        
        if(dspState)
            object_method(gensym("dsp")->s_thing, gensym("start"));
        
        x->f_numberOfChannels = newNumberOfChannel;
    }
}

t_max_err hoaGain_setattr_range(t_hoaGain *x, t_object *attr, long ac, t_atom *av)
{
    hoaGain_setminmax(x, NULL, ac, av);
	return MAX_ERR_NONE;
}

t_max_err hoaGain_setvalueof(t_hoaGain *x, long ac, t_atom *av)
{
    if (ac && av) {
        if (ac >= 2) {
            x->f_inputMode = clip_minmax(atom_getlong(av), 0, 2);
            if (atom_gettype(av+1) == A_LONG || atom_gettype(av+1) == A_FLOAT)
                hoaGain_setInputModeValue(x, atom_getfloat(av+1), false);
            
            hoaGain_bang(x);
        }
	}
	return MAX_ERR_NONE;
}

t_max_err hoaGain_getvalueof(t_hoaGain *x, long *ac, t_atom **av)
{
    if (ac && av)
    {
		if (*ac && *av)
        {
            atom_setlong(*av+0, (long)x->f_inputMode);
            if (x->f_inputMode == MIDI)
                atom_setlong(*av+1, (long)hoaGain_getInputModeValue(x));
            else
                atom_setfloat(*av+1, hoaGain_getInputModeValue(x));
		}
        else
        {
            *ac = 2;
			*av = (t_atom *)getbytes(2 * sizeof(t_atom));
        }
        
        atom_setlong(*av+0, (long)x->f_inputMode);
        if (x->f_inputMode == MIDI)
            atom_setlong(*av+1, (long)hoaGain_getInputModeValue(x));
        else
        atom_setfloat(*av+1, hoaGain_getInputModeValue(x));
    }
	return MAX_ERR_NONE;
}

void hoaGain_assign(t_hoaGain *x, double f, long notify)
{
    hoaGain_setInputModeValue(x, f, notify);
}

double hoaGain_constrain_real_value(t_hoaGain *x, double f)
{
    return clip_minmax(f, double(x->j_min), x->j_size + x->j_min);
}

double hoaGain_constrain(t_hoaGain *x, double f)
{
    return clip_minmax(f, double(0), x->j_size);
}

char hoaGain_ishorizontal(t_hoaGain *x, t_rect *r)
{
	if (x->j_orientation == 0) { // automatic mode
		if (r->height > r->width)
			return (0);
		else
			return (1);
	} else
		return (x->j_orientation == 2 ? 0 : 1);	
}

double hoaGain_postoval(t_hoaGain *x, t_pt pt, t_rect *r, char ishoriz)
{
	double pos, range;
	
	if (ishoriz) {
		pos = pt.x;
		range = r->width;
	} else {
		pos = r->height - pt.y;
		range = r->height;
	}
	if (pos <= hoaGain_DISPLAYINSET)
		pos = 0.;
	else {
		pos -= hoaGain_DISPLAYINSET;
		pos = (pos / (range - (hoaGain_DISPLAYINSET*2))) * x->j_size;
	}
	return hoaGain_constrain(x,pos);
}

double hoaGain_valtopos(t_hoaGain *x, double val, t_rect *r, char ishoriz)
{
	double pos, range;
	double size;
	
	if (ishoriz)
		range = r->width;
	else
		range = r->height;
	
    size = x->j_size;
	
	if (size < 0)
		size = 0;
	if (size)
		pos = (val / size) * (range - (hoaGain_DISPLAYINSET*2));
	else
		pos = 0;
	
	pos += hoaGain_DISPLAYINSET;
	if (ishoriz)
		return pos;
	else
		return r->height - pos;
}

double hoaGain_dBvaltopos(t_hoaGain *x, double dBval, t_rect *r, char ishoriz)
{
    double val =  scale(dBval, x->f_range[0], x->f_range[1], 0, x->j_size);
    return hoaGain_valtopos(x, val, r, ishoriz);
}

double hoaGain_dbtoval(t_hoaGain *x, double dBval)
{
    return scale(dBval, x->f_range[0], x->f_range[1], 0, x->j_size);
}
double hoaGain_valtodb(t_hoaGain *x, double val)
{
    return scale(val, 0, x->j_size, x->f_range[0], x->f_range[1]);
}

void hoaGain_mousedoubleclick(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers)
{
    hoaGain_float_dB(x, x->j_defaultValuedB);
}

void hoaGain_mousedown(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers)
{
	t_rect rect;
	double val;
	char ishoriz;
	
	jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
	s_hoaGain_cum.x = s_hoaGain_cum.y = 0;
	ishoriz = hoaGain_ishorizontal(x, &rect);
	val = hoaGain_postoval(x, pt, &rect, ishoriz);
	if (!x->j_relative)
		hoaGain_float_dB(x, val + x->j_min);	// set value immediately
	s_hoaGain_startval = x->j_val;
}

void hoaGain_mousedragdelta(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers)
{
	t_rect rect;
	double inc;
	double val, cval;
	double factor;
	
	jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
	
	if (modifiers & eShiftKey)
    {
		pt.y *= 0.2;
		pt.x *= 0.2;
	}
    
	// need to cancel cum change if it is beyond zero or the max value
	s_hoaGain_cum.x += pt.x;
	s_hoaGain_cum.y -= pt.y;
	if (hoaGain_ishorizontal(x, &rect))
    {
		factor = x->j_size / (rect.width - (hoaGain_DISPLAYINSET*2));
		inc = s_hoaGain_cum.x * factor;
		val = s_hoaGain_startval + inc;
		cval = hoaGain_constrain(x,val);
		hoaGain_float_dB(x,cval + x->j_min);
		// in case we were constrained, adjust the cum
		if (cval != val)
			s_hoaGain_cum.x = (cval-s_hoaGain_startval) / factor;
	}
    else
    {
		factor = x->j_size / (rect.height - (hoaGain_DISPLAYINSET*2)); 
		inc = s_hoaGain_cum.y * factor;
		val = s_hoaGain_startval + inc;
		cval = hoaGain_constrain(x,val);
		hoaGain_float_dB(x, cval + x->j_min);
		if (cval != val)
			s_hoaGain_cum.y = (cval - s_hoaGain_startval) / factor; 
	}
    
    // force to 0dB :
    if (isInside(x->j_valdB, -0.05, 0.05) )
        hoaGain_float_dB(x, 0);
}

void hoaGain_mouseup(t_hoaGain *x, t_object *patcherview, t_pt pt, long modifiers)
{
	double pos, vp; 
	t_rect rect; 
	double mx, my;
	char ishoriz;

	jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
	ishoriz = hoaGain_ishorizontal(x, &rect);
	pos = hoaGain_valtopos(x, x->j_val, &rect, ishoriz);
	vp = pos + 0.5;
	if (ishoriz)
    {
		mx = vp; 
		my = pt.y;	// have y position be same as where mouse down clicked
	}
    else
    {
		mx = pt.x;	// have x position be same as where mouse down clicked
		my = vp; 
	}
	
	if (fabs(pt.x-mx) > 1. || fabs(pt.y-my) > 1.) // this make sure that if you click at the "same" location we don't move the mouse.
		jmouse_setposition_box(patcherview, (t_object*) x, mx, my); 
}

t_max_err hoaGain_notify(t_hoaGain *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	long argc = 0;
	t_atom *argv = NULL;
	t_symbol *name;
	
	if (msg == hoa_sym_attr_modified)
	{
		name = (t_symbol *)object_method((t_object *)data, hoa_sym_getname);
		if (name == gensym("color")) 
		{
			object_attr_getvalueof(x, gensym("color"), &argc, &argv);
			if (argc && argv) 
			{
				object_attr_setvalueof(x, gensym("bgcolor"), argc, argv);
				sysmem_freeptr(argv);
			}
		}
        else if (name == gensym("knobcolor"))
        {
            jbox_invalidate_layer((t_object *)x, NULL, gensym("bg_layer"));
            jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
            jbox_invalidate_layer((t_object *)x, NULL, gensym("valuestripe_layer"));
        }
        else if(name == gensym("bgcolor"))
		{
            jbox_invalidate_layer((t_object *)x, NULL, gensym("bg_layer"));
		}
        else if (name == gensym("stripecolor"))
        {
            jbox_invalidate_layer((t_object *)x, NULL, gensym("offstripe_layer"));
        }
        jbox_redraw((t_jbox *)x);
	}
    
	return jbox_notify((t_jbox *)x, s, msg, sender, data);
}

void hoaGain_tometer(t_hoaGain *x, t_symbol *s, long argc, t_atom *argv)
{
    t_object *patcher;
	t_object *gain;
    t_object *object;
    t_object *line;
	t_max_err err;
    
    if(argc && argv && ((s == gensym("angles") || s == gensym("offset") || s == gensym("channels"))))
    {
        err = object_obex_lookup(x, gensym("#P"), (t_object **)&patcher);
        if (err != MAX_ERR_NONE)
            return;
       
        err = object_obex_lookup(x, gensym("#B"), (t_object **)&gain);
        if (err != MAX_ERR_NONE)
            return;
        for (line = jpatcher_get_firstline(patcher); line; line = jpatchline_get_nextline(line))
        {
            if (jpatchline_get_box1(line) == gain)
            {
                object = jpatchline_get_box2(line);
                t_symbol *classname = object_classname(jbox_get_object(object));
                if(classname == gensym("hoa.2d.meter~") || classname == gensym("hoa.2d.gain~") || classname == gensym("hoa.2d.vector~"))
                {
                    object_method_typed(jbox_get_object(object), s, argc, argv, NULL);
                }
            }
        }
	}
}

void HoaGain_reconnect_outlet(t_hoaGain *x)
{
	t_object *patcher;
	t_object *gain_box;
    t_object *obj2_box;
    t_object *obj2;
    t_object *line;
	t_max_err err;
    
	err = object_obex_lookup(x, hoa_sym_pound_P, (t_object **)&patcher);
	if (err != MAX_ERR_NONE)
		return;
	
	err = object_obex_lookup(x, hoa_sym_pound_B, (t_object **)&gain_box);
	if (err != MAX_ERR_NONE)
		return;
	
    for (line = jpatcher_get_firstline(patcher); line; line = jpatchline_get_nextline(line))
    {
        if (jpatchline_get_box1(line) == gain_box)
        {
            obj2_box = jpatchline_get_box2(line);
            obj2 = jbox_get_object(obj2_box);
            t_symbol* classname = object_classname(obj2);
            
            if ( classname == hoa_sym_dac || (object_is_hoa(obj2) && classname != hoa_sym_hoa_pi && classname != hoa_sym_hoa_pi_tilde))
            {
                for(int i = 0; jbox_getinlet((t_jbox *)obj2_box, i) != NULL && i < x->f_numberOfChannels; i++)
                {
                    t_atom msg[4];
                    t_atom rv;
                    
                    atom_setobj(msg, gain_box);
                    atom_setlong(msg + 1, i);
                    atom_setobj(msg + 2, obj2_box);
                    atom_setlong(msg + 3, i);
                    
                    object_method_typed(patcher , hoa_sym_connect, 4, msg, &rv);
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------------------
// Line
// -------------------------------------------------------------------------------------------

Line::Line(long samps, long vector_size, long sampling_rate)
{
	init(vector_size, sampling_rate);
	setRampInSample(samps);
}

Line::Line(double ms, long vector_size, long sampling_rate)
{
	init(vector_size, sampling_rate);
	setRampInMs(ms);
}

void Line::init(const long vector_size, const long sampling_rate)
{
	m_value_old = 0.;
	m_value_new = 0.;
	m_value_step = 0.;
	m_counter = 0;
	
	setVectorSize(vector_size);
	setSamplingRate(sampling_rate);
}

double Line::process()
{
	m_value_old += m_value_step;
	if(m_counter++ >= m_ramp)
	{
		m_value_step = 0.;
		m_value_old  = m_value_new;
		m_counter    = 0;
	}
	return m_value_old;
}

void Line::setRampInSample(const long samps)
{
	m_ramp = clip_min(samps, (long)1);
}

void Line::setRampInMs(const double ms)
{
	setRampInSample(ms * (double)m_sampling_rate / 1000.);
}

void Line::setValueDirect(const double value)
{
	m_value_old = m_value_new = value;
	m_value_step = 0.;
	m_counter = 0;
}

void Line::setValue(const double value)
{
	m_value_new  = value;
	m_value_step = (m_value_new - m_value_old) / (double)m_ramp;
	m_counter = 0;
}

void Line::setValueAngleDirect(const double angle)
{
	m_value_old = m_value_new = wrap_twopi(angle);
	m_value_step = 0.;
	m_counter = 0;
}

void Line::setValueAngle(const double angle)
{
	m_value_new = wrap_twopi(angle);
	m_value_old = wrap_twopi(m_value_old);
    double distance;
    if(m_value_old > m_value_new)
        distance = (m_value_old - m_value_new);
    else
        distance = (m_value_new - m_value_old);
	if(distance <= HOA_PI)
	{
		m_value_step = (m_value_new - m_value_old) / (double)m_ramp;
	}
	else
	{
		if(m_value_new > m_value_old)
		{
			m_value_step = ((m_value_new - HOA_2PI) - m_value_old) / (double)m_ramp;
		}
		else
		{
			m_value_step = ((m_value_new + HOA_2PI) - m_value_old) / (double)m_ramp;
		}
	}
	m_counter = 0;
}
