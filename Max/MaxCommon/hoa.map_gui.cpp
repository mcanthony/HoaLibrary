/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

/**
 @file      hoa.map.cpp
 @name      hoa.map
 @realname  hoa.map
 @type      object
 @module    hoa
 @author    Julien Colafrancesco, Pierre Guillot, Eliott Paris.
 
 @digest
 A graphic user interface to spatialize sources on a plane.
 
 @description
 <o>hoa.map</o> allows you to spatialize several sources in a 2d or 3d context. You can add and remove sources, change coordinates, add description and create groups. <o>hoa.map</o> is dedicated to control a <o>hoa.2d.map~</o> or a <o>hoa.3d.map~</o> object.
 
 @discussion
 <o>hoa.map</o> allows you to spatialize several sources in a 2d or 3d context. You can add and remove sources, change coordinates, add description and create groups. <o>hoa.map</o> is dedicated to control a <o>hoa.2d.map~</o> or a <o>hoa.3d.map~</o> object.
 
 @category ambisonics, hoa objects, audio, GUI, MSP
 
 @seealso hoa.2d.map, hoa.3d.map~, hoa.2d.space, hoa.2d.meter~, hoa.2d.scope~, hoa.2d.encoder~, hoa.2d.wider~, hoa.2d.decoder~, hoa.3d.encoder~, hoa.3d.scope~, hoa.3d.wider~, hoa.3d.decoder~
 
 @illustration on @caption hoa.map GUI object
 @palette yes
 */

#include "HoaCommon.max.h"

#define MAX_ZOOM 1.
#define MIN_ZOOM 0.01
#define MAX_NUMBER_OF_SOURCES 64

#define ODD_BINDING_SUFFIX "map1572"

typedef enum _BindingMapMsgFlag {
	BMAP_REDRAW		= 0x01,
	BMAP_NOTIFY		= 0x02,
	BMAP_OUTPUT		= 0x04
} BindingMapMsgFlag;

typedef struct _textfield {
	t_jbox			j_box;
    t_object*       j_patcher;
    t_object*       j_patcherview;
    t_jfont*        jfont;
} t_textfield;

void hoa_textfield_init(void);
t_textfield* textfield_new(t_symbol *s, short argc, t_atom *argv);
void textfield_free(t_textfield *x);
void textfield_select(t_textfield *x);
void textfield_doselect(t_textfield *x);
void textfield_paint(t_textfield *x, t_object *view);
long textfield_key(t_textfield *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);
long textfield_keyfilter(t_textfield *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter);
void textfield_enter(t_textfield *x);
t_max_err textfield_notify(t_textfield *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_textfield_class = NULL;

typedef struct _linkmap t_linkmap;

typedef struct  _hoamap
{
	t_jbox          j_box;
	t_rect          rect;
	t_jfont*        jfont;
    
	void*		f_out_sources;
    void*		f_out_groups;
    void*		f_out_infos;
    
    t_object*   f_patcher;
    t_object*   f_colorpicker;
    t_object*   f_textfield;
    
	SourcesManager*     f_source_manager;
	SourcesManager*     f_self_source_manager;
    
    t_pt        f_cursor_position;
	int			f_mouse_was_dragging;
    long        f_index_of_selected_source;
    long        f_index_of_selected_group;
    long        f_index_of_source_to_remove;
    long        f_index_of_group_to_remove;
    long        f_index_of_source_to_color;
    long        f_index_of_group_to_color;
    
    t_jrgba		f_color_bg;
    t_jrgba     f_color_selection;
    
    int         f_cartesian_drag;
    
    double      f_size_source;
	double		f_zoom_factor;
    
    t_rect		f_rect_selection;
	int			f_rect_selection_exist;
	
	long		f_showgroups;
	long		f_save_with_patcher;
    
    // options :
    t_atom_long f_output_mode;			// Polar Cartesian
	t_atom_long f_coord_view;			// xy xz yz
	
	t_symbol*	f_binding_name;
	t_linkmap*	f_listmap;
	int			f_output_enabled;
} t_hoa_map;

typedef struct _linkmap
{
	t_linkmap *next;
	t_hoa_map *map;
	void update_headptr(t_linkmap *linkmap_headptr, SourcesManager* sourcesManager)
	{
		map->f_listmap = linkmap_headptr;
		map->f_source_manager = sourcesManager;
		if(next != NULL)
			next->update_headptr(linkmap_headptr, sourcesManager);
	}
} t_linkmap;


t_class *hoamap_class;

// general methods
void *hoamap_new(t_symbol *s, int argc, t_atom *argv);
void hoamap_free(t_hoa_map *x);
void hoamap_tick(t_hoa_map *x);
void hoamap_assist(t_hoa_map *x, void *b, long m, long a, char *s);
void hoamap_preset(t_hoa_map *x);
t_max_err hoamap_setvalueof(t_hoa_map *x, long ac, t_atom *av);
t_max_err hoamap_getvalueof(t_hoa_map *x, long *ac, t_atom **av);
void hoamap_jsave(t_hoa_map *x, t_dictionary *d);
t_max_err hoamap_notify(t_hoa_map *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
t_max_err hoamap_zoom(t_hoa_map *x, t_object *attr, long argc, t_atom *argv);

void hoamap_source(t_hoa_map *x, t_symbol *s, short ac, t_atom *av);
void hoamap_group(t_hoa_map *x, t_symbol *s, short ac, t_atom *av);
void hoamap_set(t_hoa_map *x, t_symbol *s, short ac, t_atom *av);
void hoamap_bang(t_hoa_map *x);
void hoamap_infos(t_hoa_map *x);
void hoamap_clear_all(t_hoa_map *x);
void hoamap_output(t_hoa_map *x);

void hoamap_color_picker(t_hoa_map *x);
void hoamap_text_field(t_hoa_map *x);

/* Paint Methods */
void hoamap_paint(t_hoa_map *x, t_object *view);
void draw_background(t_hoa_map *x, t_object *view, t_rect *rect);
void draw_sources(t_hoa_map *x,  t_object *view, t_rect *rect);
void draw_groups(t_hoa_map *x,  t_object *view, t_rect *rect);
void draw_rect_selection(t_hoa_map *x,  t_object *view, t_rect *rect);

void hoamap_mousedown(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers);
void hoamap_mousedrag(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers);
void hoamap_mouseup(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers);
void hoamap_mouseenter(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers);
void hoamap_mousemove(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers);
void hoamap_mouseleave(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers);
void hoamap_mousewheel(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers, double x_inc, double y_inc);
long hoamap_key(t_hoa_map *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);

t_hoa_err hoa_getinfos(t_hoa_map* x, t_hoa_boxinfos* boxinfos);
void hoamap_deprecated(t_hoa_map *x, t_symbol* s, long ac, t_atom* av);

t_max_err bindname_set(t_hoa_map *x, t_object *attr, long argc, t_atom *argv);
void hoamap_send_binded_map_update(t_hoa_map *x, long flags); // BindingMapMsgFlag

#ifdef HOA_PACKED_LIB
int hoa_map_gui_main(void)
#else
int C74_EXPORT main(void)
#endif
{
    hoa_textfield_init();
	t_class *c;
    
	c = class_new("hoa.map", (method)hoamap_new, (method)hoamap_free, (short)sizeof(t_hoa_map), 0L, A_GIMME, 0);
	class_setname((char *)"hoa.map", (char *)"hoa.map");
    class_setname((char *)"hoa.2d.map", (char *)"hoa.map");
    class_setname((char *)"hoa.3d.map", (char *)"hoa.map");
    
	hoa_initclass(c, NULL);
	
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
	jbox_initclass(c, JBOX_COLOR | JBOX_FIXWIDTH | JBOX_FONTATTR);
	
	class_addmethod(c, (method) hoamap_assist,           "assist",			A_CANT,	0);
	class_addmethod(c, (method) hoamap_paint,            "paint",			A_CANT,	0);
	class_addmethod(c, (method) hoamap_notify,           "notify",			A_CANT, 0);
	class_addmethod(c, (method) hoamap_preset,			 "preset",			0);
    class_addmethod(c, (method) hoamap_getvalueof,		 "getvalueof",		A_CANT, 0);
	class_addmethod(c, (method) hoamap_setvalueof,		 "setvalueof",		A_CANT, 0);
	class_addmethod(c, (method) hoamap_jsave,            "jsave",			A_CANT, 0);
    
	// @method bang @digest Output current sources values.
	// @description The <m>bang</m> Output current sources values.
    class_addmethod(c, (method) hoamap_bang,             "bang",			A_CANT, 0);
	
	// @method getinfo @digest Report informations in the rightmost outlet.
	// @description The <m>getinfo</m> message report informations in the rightmost outlet.
    class_addmethod(c, (method) hoamap_infos,            "getinfo",					0);
    
	// @method set @digest Send source or group relative instructions without causing output.
	// @description The <m>set</m> message send <m>source</m> or <m>group</m> relative instructions like their position, color, a description, a mute state...
	// @marg 0 @name source-or-group @optional 0 @type symbol
	// @marg 1 @name source-index @optional 0 @type int
	// @marg 2 @name modifier-message @optional 0 @type symbol
	// @marg 3 @name message-arguments @optional 0 @type float/int/symbol
    class_addmethod(c, (method) hoamap_set,				 "set",				A_GIMME,0);
	
	// @method source @digest Send source relative instructions.
	// @description The <m>source</m> message send source relative instructions like their position, color, add a description, mute them...
	// @marg 0 @name source-index @optional 0 @type int
	// @marg 1 @name modifier-message @optional 0 @type symbol
	// @marg 2 @name message-arguments @optional 0 @type float/int/symbol
    class_addmethod(c, (method) hoamap_source,           "source",			A_GIMME,0);
	
	// @method group @digest Send group relative instructions.
	// @description The <m>group</m> message send group relative instructions like their position, color, add a description, mute them...
	// @marg 0 @name group-index @optional 0 @type int
	// @marg 1 @name modifier-message @optional 0 @type symbol
	// @marg 2 @name message-arguments @optional 0 @type float/int/symbol
    class_addmethod(c, (method) hoamap_group,            "group",			A_GIMME,0);
	
	class_addmethod(c, (method) hoamap_deprecated,       "anything",		A_GIMME,0);
	
	// @method clear @digest Remove all sources and groups.
	// @description The <m>clear</m> message remove all sources and groups.
    class_addmethod(c, (method) hoamap_clear_all,        "clear",			A_NOTHING ,0);
    
	// @method (mouse) @digest Change sources or group position, set zoom factor.
	// @description Click and drag the sources to set their position. Right-click to make appear contextual setting menu, use the mouse-weel to zoom. Movement can be restricted to the azimuth if you press <b>alt</b> key while you are dragging a source or a group, or restricted to the radius with <b>shift</b> key. Press <b>cmd+a</b> to create a group containing all existing sources.
    class_addmethod(c, (method) hoamap_mousedown,        "mousedown",		A_CANT, 0);
    class_addmethod(c, (method) hoamap_mousedrag,        "mousedrag",		A_CANT, 0);
    class_addmethod(c, (method) hoamap_mouseup,          "mouseup",			A_CANT, 0);
    class_addmethod(c, (method) hoamap_mouseenter,       "mouseenter",		A_CANT, 0);
    class_addmethod(c, (method) hoamap_mousemove,        "mousemove",		A_CANT, 0);
    class_addmethod(c, (method) hoamap_mouseleave,       "mouseleave",		A_CANT, 0);
    class_addmethod(c, (method) hoamap_mousewheel,		 "mousewheel",		A_CANT, 0);
    class_addmethod(c, (method) hoamap_key,              "key",				A_CANT, 0);
	
	CLASS_ATTR_DEFAULT			(c, "patching_rect", 0, "0 0 220 220");
	// @exclude hoa.3d.map
	CLASS_ATTR_INVISIBLE		(c, "color", 0);
	// @exclude hoa.3d.map
	CLASS_ATTR_INVISIBLE		(c, "textcolor", 0);
	// @exclude hoa.3d.map
    
    /* Colors */
	CLASS_ATTR_RGBA				(c, "bgcolor", 0, t_hoa_map, f_color_bg);
	CLASS_ATTR_CATEGORY			(c, "bgcolor", 0, "Color");
	CLASS_ATTR_STYLE			(c, "bgcolor", 0, "rgba");
	CLASS_ATTR_LABEL			(c, "bgcolor", 0, "Background Color");
	CLASS_ATTR_ORDER			(c, "bgcolor", 0, "1");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bgcolor", 0, "0.75 0.75 0.75 1.");
	// @description Sets the RGBA values for the background color of the <o>hoa.3d.map</o> object
    
    CLASS_ATTR_RGBA				(c, "selcolor", 0, t_hoa_map, f_color_selection);
	CLASS_ATTR_CATEGORY			(c, "selcolor", 0, "Color");
	CLASS_ATTR_STYLE			(c, "selcolor", 0, "rgba");
	CLASS_ATTR_LABEL			(c, "selcolor", 0, "Selection Color");
	CLASS_ATTR_ORDER			(c, "selcolor", 0, "4");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "selcolor", 0, "0.36 0.37 0.7 0.5");
	// @description Sets the RGBA values for the selection in a <o>hoa.3d.map</o> object
	
    /* Behavior */
	CLASS_STICKY_CATEGORY(c, 0, "Behavior");
	CLASS_ATTR_LONG				(c, "view", 0, t_hoa_map, f_coord_view);
	CLASS_ATTR_LABEL			(c, "view", 0, "Coordinate View");
	CLASS_ATTR_ENUMINDEX		(c, "view", 0, "xy xz yz");
	CLASS_ATTR_DEFAULT          (c, "view", 0,  "0");
    CLASS_ATTR_SAVE             (c, "view", 1);
    CLASS_ATTR_ORDER			(c, "view", 0, "1");
	// @description Sets the coordinates display mode. coordinates display mode can be <b>xy</b>, <b>xz</b> or <b>yz</b>
	
	CLASS_ATTR_LONG				(c, "outputmode", 0, t_hoa_map, f_output_mode);
	CLASS_ATTR_LABEL			(c, "outputmode", 0, "Output Mode");
	CLASS_ATTR_ENUMINDEX		(c, "outputmode", 0, "Polar Cartesian");
	CLASS_ATTR_DEFAULT          (c, "outputmode", 0,  "0");
    CLASS_ATTR_SAVE             (c, "outputmode", 1);
    CLASS_ATTR_ORDER			(c, "outputmode", 0, "2");
	// @description Sets the output mode. Output mode can be <b>polar</b> or <b>cartesian</b>
    
	CLASS_ATTR_DOUBLE			(c, "zoom", 0, t_hoa_map, f_zoom_factor);
    CLASS_ATTR_ACCESSORS		(c, "zoom", NULL, hoamap_zoom);
	CLASS_ATTR_LABEL			(c, "zoom", 0, "Zoom");
	CLASS_ATTR_DEFAULT          (c, "zoom", 0, "0.35");
    CLASS_ATTR_ORDER			(c, "zoom", 0, "4");
    CLASS_ATTR_SAVE             (c, "zoom", 1);
	// @description Sets the zoom factor
	
	CLASS_ATTR_LONG             (c, "showgroups", 0, t_hoa_map, f_showgroups);
	CLASS_ATTR_STYLE_LABEL      (c, "showgroups", 0, "onoff", "Use and show group");
	CLASS_ATTR_DEFAULT          (c, "showgroups", 0, "1");
	CLASS_ATTR_SAVE             (c, "showgroups", 1);
	CLASS_ATTR_ORDER            (c, "showgroups", 0, "5");
	// @description Use and show group ?
	
	CLASS_ATTR_LONG             (c, "save", 0, t_hoa_map, f_save_with_patcher);
	CLASS_ATTR_STYLE_LABEL      (c, "save", 0, "onoff", "Save Object State with Patcher");
	CLASS_ATTR_DEFAULT          (c, "save", 0, "1");
	CLASS_ATTR_SAVE             (c, "save", 1);
	CLASS_ATTR_ORDER            (c, "save", 0, "6");
	// @description Check this to save object' state with patcher. Warning : if Parameter Mode and Initial are enable, this <m>save</m> method is no longer effective.
	CLASS_STICKY_CATEGORY_CLEAR(c);
	
	CLASS_ATTR_SYM				(c, "mapname", 0, t_hoa_map, f_binding_name);
	CLASS_ATTR_LABEL			(c, "mapname", 0, "Map Name");
	CLASS_ATTR_CATEGORY			(c, "mapname", 0, "Name");
	CLASS_ATTR_ACCESSORS		(c, "mapname", NULL, bindname_set);
	CLASS_ATTR_DEFAULT          (c, "mapname", 0,  "");
    CLASS_ATTR_SAVE             (c, "mapname", 1);
    CLASS_ATTR_ORDER			(c, "mapname", 0, "1");
	// @description Use the <m>mapmode</m> attribute to bind multiple <o>hoa.map</o> objects together.

	class_register(CLASS_BOX, c);
    
    class_alias(c, gensym("hoa.2d.map"));
    class_alias(c, gensym("hoa.3d.map"));
    
	hoamap_class = c;
	return 0;
}

void *hoamap_new(t_symbol *s, int argc, t_atom *argv)
{
	t_hoa_map *x =  NULL; 
	t_dictionary *d;
	long flags;
	
	if (!(d = object_dictionaryarg(argc,argv)))
		return NULL;
	
	x = (t_hoa_map *)object_alloc(hoamap_class);
	flags = 0 
	| JBOX_DRAWFIRSTIN 
	| JBOX_DRAWINLAST
    | JBOX_NODRAWBOX
	| JBOX_DRAWBACKGROUND
	| JBOX_GROWY
	| JBOX_HILITE
	;
    
	jbox_new(&x->j_box, flags, argc, argv);
	x->f_self_source_manager = new SourcesManager(1./MIN_ZOOM - 5.);
    x->f_source_manager = x->f_self_source_manager;
	
    x->j_box.b_firstin = (t_object*) x;
        
    x->f_out_infos      = listout(x);
    x->f_out_groups     = listout(x);
	x->f_out_sources    = listout(x);
		
	x->jfont = jfont_create(jbox_get_fontname((t_object *)x)->s_name, (t_jgraphics_font_slant)jbox_get_font_slant((t_object *)x), (t_jgraphics_font_weight)jbox_get_font_weight((t_object *)x), jbox_get_fontsize((t_object *)x));

    x->f_patcher = NULL;
    x->f_colorpicker = NULL;
	
	x->f_rect_selection_exist = 0;
    x->f_index_of_selected_source = -1;
    x->f_index_of_selected_group = -1;
	
	x->f_binding_name = hoa_sym_nothing;
	x->f_listmap = NULL;
	x->f_output_enabled = 1;
	x->f_save_with_patcher = 1;
	
	attr_dictionary_process(x, d);
	
	// restore object state
    
	long ac = 0;
	t_atom *av = NULL;
    dictionary_copyatoms(d, gensym("map_saved_state"), &ac, &av);
	if (ac && av)
		hoamap_setvalueof(x, ac, av);
	
	jbox_ready(&x->j_box);
	return (x);
}

void linkmap_add_with_binding_name(t_hoa_map *x, t_symbol* binding_name)
{
	char strname[2048];
	t_symbol* name = NULL;
	t_object *jp = NULL;
	object_obex_lookup(x, hoa_sym_pound_P, &jp);
	if (jp && (jp = jpatcher_get_toppatcher(jp)))
	{
		sprintf(strname, "p%ld_%s_%s", (long)jp, binding_name->s_name, ODD_BINDING_SUFFIX);
		name = gensym(strname);
		//object_post((t_object*)x, "name : %s", name->s_name);
		
		// t_listmap null => new t_listmap
		if(name->s_thing == NULL)
		{
			x->f_listmap = (t_linkmap *)sysmem_newptr(sizeof(t_linkmap));
			if (x->f_listmap)
			{
				x->f_listmap->map = x;
				x->f_listmap->next = NULL;
				name->s_thing = (t_object *)x->f_listmap;
				x->f_source_manager = x->f_self_source_manager;
			}
		}
		else // t_listmap exist => add our object in it
		{
			t_linkmap *temp, *temp2;
			
			if(x->f_listmap != NULL)
			{
				temp = x->f_listmap;
				while(temp)
				{
					if(temp->next != NULL && temp->next->map == x)
					{
						temp2 = temp->next->next;
						sysmem_freeptr(temp->next);
						temp->next = temp2;
					}
					temp = temp->next;
				}
			}
			
			x->f_listmap = (t_linkmap *)name->s_thing;
			temp = x->f_listmap;
			t_hoa_map* head_map = temp->map;
			
			while(temp)
			{
				if(temp->next == NULL)
				{
					temp2 = (t_linkmap *)sysmem_newptr(sizeof(t_linkmap));
					if (temp2)
					{
						temp2->map = x;
						temp2->next = NULL;
						temp->next = temp2;
						temp->next->map->f_source_manager = head_map->f_self_source_manager;
					}
					break;
				}
				temp = temp->next;
			}
		}
	}
}

void linkmap_remove_with_binding_name(t_hoa_map *x, t_symbol* binding_name)
{
	char strname[2048];
	t_symbol* name = NULL;
	t_object *jp = NULL;
	object_obex_lookup(x, hoa_sym_pound_P, &jp);
    
	if (jp && (jp = jpatcher_get_toppatcher(jp)))
	{
		sprintf(strname, "p%ld_%s_%s", (long)jp, binding_name->s_name, ODD_BINDING_SUFFIX);
		name = gensym(strname);
		
		if(name->s_thing != NULL)
		{
			t_linkmap *temp, *temp2;
			temp = (t_linkmap *)name->s_thing;
			t_hoa_map* head_map = temp->map;
			int counter = 0;
			
			while(temp)
			{
				if (counter == 0 && temp->map == x) // head of the linkmap
				{
					head_map = temp->map;
					
					if(temp->next == NULL) // is also the last item of the linkmap
					{
						name->s_thing = NULL;
					}
					else
					{
						name->s_thing = (t_object *)temp->next;
						
						// bind all object to the next SourcesManager (next becoming the new head of the t_linkmap)
						head_map->f_source_manager->copyTo(temp->next->map->f_self_source_manager);
						temp->next->update_headptr((t_linkmap *)name->s_thing, temp->next->map->f_self_source_manager);
					}
					
                    //if(x->f_listmap)
                    //    sysmem_freeptr(x->f_listmap);
					x->f_listmap = NULL;
					
					x->f_source_manager = x->f_self_source_manager; // not sure if this is necessary (normally it is the same pointer)
				}
                
				else if(temp->next != NULL && temp->next->map == x)
				{
					// we restore the original pointer
					temp->next->map->f_source_manager = temp->next->map->f_self_source_manager;
					// then we copy the shared SourcesManager into the original one
					head_map->f_self_source_manager->copyTo(temp->next->map->f_source_manager);
					
					temp2 = temp->next->next;
					sysmem_freeptr(temp->next);
					x->f_listmap = NULL;
					temp->next = temp2;
				}
				
				temp = temp->next;
			}
		}
	}
	
}

t_max_err bindname_set(t_hoa_map *x, t_object *attr, long argc, t_atom *argv)
{
	if (argc && argv && atom_gettype(argv) == A_SYM)
	{
		t_symbol* new_binding_name = atom_getsym(argv);
		
		if(new_binding_name != x->f_binding_name)
		{
			// remove previous binding
			if (x->f_binding_name != hoa_sym_nothing)
            {
                defer_low(x, (method)linkmap_remove_with_binding_name, x->f_binding_name, NULL, NULL);
                //linkmap_remove_with_binding_name(x, x->f_binding_name);
            }
			
			// add new one
			if (new_binding_name != hoa_sym_nothing)
			{
				// use deferlow to have the good toppatcher pointer when the patch is being loaded
				defer_low(x, (method)linkmap_add_with_binding_name, new_binding_name, NULL, NULL);
				//linkmap_add_with_binding_name(x, new_binding_name);
				x->f_binding_name = new_binding_name;
			}
			else
				x->f_binding_name = hoa_sym_nothing;
		}
	}
	else
	{
		if (x->f_binding_name != hoa_sym_nothing)
			linkmap_remove_with_binding_name(x, x->f_binding_name);
		
		x->f_binding_name = hoa_sym_nothing;
	}
	
	object_notify(x, hoa_sym_modified, NULL);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
    jbox_redraw((t_jbox *)x);
    hoamap_output(x);
	
	return MAX_ERR_NONE;
}

void hoamap_send_binded_map_update(t_hoa_map *x, long flags)
{
	if(x->f_listmap)
	{
		t_linkmap *temp = x->f_listmap;
		t_object* mapobj;
		while (temp)
		{
			mapobj = (t_object*)temp->map;
			
			if (mapobj != (t_object*)x)
			{
				if (flags & BMAP_REDRAW)
				{
					jbox_invalidate_layer((t_object *)mapobj, NULL, hoa_sym_sources_layer);
					jbox_invalidate_layer((t_object *)mapobj, NULL, hoa_sym_groups_layer);
					jbox_redraw((t_jbox *)mapobj);
				}
				if (flags & BMAP_NOTIFY)
					object_notify(mapobj, hoa_sym_modified, NULL);
				if (flags & BMAP_OUTPUT && x->f_output_enabled)
					object_method(mapobj, hoa_sym_bang);
			}
			
			temp = temp->next;
		}
	}
}

void hoamap_deprecated(t_hoa_map *x, t_symbol* s, long ac, t_atom* av)
{
	if (s == hoa_sym_slot)
	{
		object_error((t_object*)x, "slot method is deprecated, please use preset or pattr system instead");
	}
	else if (s == hoa_sym_trajectory)
	{
		object_error((t_object*)x, "trajectory is deprecated, please use preset or pattr system instead");
	}
}

t_hoa_err hoa_getinfos(t_hoa_map* x, t_hoa_boxinfos* boxinfos)
{
	boxinfos->object_type = HOA_OBJECT_2D;
	boxinfos->autoconnect_inputs = 0;
	boxinfos->autoconnect_outputs = 0;
	boxinfos->autoconnect_inputs_type = HOA_CONNECT_TYPE_STANDARD;
	boxinfos->autoconnect_outputs_type = HOA_CONNECT_TYPE_STANDARD;
	return HOA_ERR_NONE;
}

void hoamap_free(t_hoa_map *x)
{
    //defer_low(x, (method)linkmap_remove_with_binding_name, x->f_binding_name, NULL, NULL);
	linkmap_remove_with_binding_name(x, x->f_binding_name);
	
	jbox_free(&x->j_box);
    jfont_destroy(x->jfont);
	
    //delete x->f_source_manager;
	delete x->f_self_source_manager;
	
    if(x->f_patcher)
        object_free(x->f_patcher);
    if(x->f_colorpicker )
        object_free(x->f_colorpicker);
    if(x->f_textfield)
        object_free(x->f_textfield);
}

void hoamap_assist(t_hoa_map *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
		sprintf(s,"(messages) Behavior and appearance");
	else
    {
        if(a == 0)
            sprintf(s,"(list) Sources coordinates");
        if(a == 1)
            sprintf(s,"(list) Groups coordinates");
        if(a == 2)
            sprintf(s,"(list) Infos");
    }
}

/**********************************************************/
/*          Intialisation par l'utilisateur               */
/**********************************************************/

void hoamap_clear_all(t_hoa_map *x)
{
	// mute all source and output before clearing them to notify hoa.#.map~
	for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
		if(x->f_source_manager->sourceGetExistence(i))
			x->f_source_manager->sourceSetMute(i, 1);
	
	hoamap_output(x);
	hoamap_send_binded_map_update(x, BMAP_OUTPUT);
	
	// now we can clear, then notify, output and redraw all maps
    x->f_source_manager->clearAll();
    
    object_notify(x, hoa_sym_modified, NULL);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
    jbox_redraw((t_jbox *)x);
    hoamap_output(x);
	hoamap_send_binded_map_update(x, BMAP_REDRAW | BMAP_OUTPUT | BMAP_NOTIFY);
}

void hoamap_set(t_hoa_map *x, t_symbol *s, short ac, t_atom *av)
{
	x->f_output_enabled = 0;
	if (ac && av && atom_gettype(av) == A_SYM)
	{
		t_symbol* msgtype = atom_getsym(av);
		av++; ac--;
		if (msgtype == hoa_sym_source)
			object_method_typed(x, hoa_sym_source, ac, av, NULL);
		else if (msgtype == hoa_sym_group)
			object_method_typed(x, hoa_sym_group, ac, av, NULL);
	}
	x->f_output_enabled = 1;
}

void hoamap_source(t_hoa_map *x, t_symbol *s, short ac, t_atom *av)
{
	int index;
	int exist;
	
	if (ac && av && atom_gettype(av) == A_SYM && atom_getsym(av) == hoa_sym_source_preset_data)
	{
		av++; ac--;
		
		// source / index / exist / abscissa / ordinate / height / mutestate / r / g / b / a / description
		
		for(int i = 0; i < MAX_NUMBER_OF_SOURCES*12; i += 12)
		{
			if( (i <= ac-12) && atom_gettype(av+i) == A_SYM && atom_getsym(av+i) == hoa_sym_source)
			{
				if ( atom_isNumber(av+i+1) && atom_isNumber(av+i+2))
				{
					index = atom_getlong(av+i+1) - 1;
					exist = atom_getlong(av+i+2);
					
					if(exist &&
					   atom_gettype(av+i+3) == A_FLOAT && atom_gettype(av+i+4) == A_FLOAT && atom_gettype(av+i+5) == A_FLOAT &&
					   (atom_gettype(av+i+6) == A_FLOAT || atom_gettype(av+i+6) == A_LONG) &&
					   atom_gettype(av+i+7) == A_FLOAT && atom_gettype(av+i+8) == A_FLOAT &&
					   atom_gettype(av+i+9) == A_FLOAT && atom_gettype(av+i+10) == A_FLOAT &&
					   atom_gettype(av+i+11) == A_SYM)
					{
						x->f_source_manager->sourceSetCartesian(index, atom_getfloat(av+i+3), atom_getfloat(av+i+4), atom_getfloat(av+i+5));
						x->f_source_manager->sourceSetMute(index, atom_getlong(av+i+6));
						x->f_source_manager->sourceSetColor(index, atom_getfloat(av+i+7), atom_getfloat(av+i+8), atom_getfloat(av+i+9), atom_getfloat(av+i+10));
						x->f_source_manager->sourceSetDescription(index, atom_getsym(av+i+11)->s_name);
					}
					else if (!exist && x->f_source_manager->sourceGetExistence(index))
					{
						x->f_source_manager->sourceRemove(index);
						t_atom out_av[3];
						atom_setlong(out_av, index + 1);
						atom_setsym(out_av+1, hoa_sym_mute);
						atom_setlong(out_av+2, 1);
						outlet_list(x->f_out_sources, 0L, 3, out_av);
					}
				}
			}
		}
		
		// no need to (repaint | notify | output) here => group will do this just after
	}
	
    else if(ac && av && atom_gettype(av) == A_LONG && atom_getlong(av) >= 0 && atom_gettype(av+1) == A_SYM)
    {
		t_symbol* param = atom_getsym(av+1);
		long index = atom_getlong(av) -1;
		
		int causeOutput = 1;
		
        if(param == hoa_sym_polar || param == hoa_sym_pol)
		{
			if (ac >= 5 && atom_isNumber(av+2) && atom_isNumber(av+3) && atom_isNumber(av+4))
				x->f_source_manager->sourceSetPolar(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4));
			else if (ac >= 4 && atom_isNumber(av+2) && atom_isNumber(av+3))
				x->f_source_manager->sourceSetPolar(index, atom_getfloat(av+2), atom_getfloat(av+3));
		}
        else if(param == hoa_sym_radius)
			x->f_source_manager->sourceSetRadius(index, atom_getfloat(av+2));
        else if(param == hoa_sym_azimuth)
			x->f_source_manager->sourceSetAzimuth(index, atom_getfloat(av+2));
		else if(param == hoa_sym_elevation)
			x->f_source_manager->sourceSetElevation(index, atom_getfloat(av+2));
        else if(param == hoa_sym_cartesian || param == hoa_sym_car)
		{
			if (ac >= 5 && atom_isNumber(av+2) && atom_isNumber(av+3) && atom_isNumber(av+4))
				x->f_source_manager->sourceSetCartesian(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4));
			else if (ac >= 4 && atom_isNumber(av+2) && atom_isNumber(av+3))
				x->f_source_manager->sourceSetCartesian(index, atom_getfloat(av+2), atom_getfloat(av+3));
		}
        else if(param == hoa_sym_abscissa)
			x->f_source_manager->sourceSetAbscissa(index, atom_getfloat(av+2));
        else if(param == hoa_sym_ordinate)
            x->f_source_manager->sourceSetOrdinate(index, atom_getfloat(av+2));
		else if(param == hoa_sym_height)
            x->f_source_manager->sourceSetHeight(index, atom_getfloat(av+2));
        else if(param == hoa_sym_remove)
        {
            x->f_source_manager->sourceRemove(index);
            t_atom av[3];
            atom_setlong(av, index);
            atom_setsym(av+1, hoa_sym_mute);
            atom_setlong(av+2, 1);
            outlet_list(x->f_out_sources, 0L, 3, av);
        }
        else if(param == hoa_sym_mute)
			x->f_source_manager->sourceSetMute(index, atom_getlong(av+2));
        else if(param == hoa_sym_description)
        {
			causeOutput = 0;
            char description[250];
            char number[250];
            if(atom_gettype(av+1) == A_SYM)
            {
                strcpy(description, atom_getsym(av+2)->s_name);
                strcat(description, " ");
                if(atom_getsym(av+2) == hoa_sym_remove)
                {
                    x->f_source_manager->sourceSetDescription(index, "");
                    object_notify(x, hoa_sym_modified, NULL);
                    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
                    jbox_redraw((t_jbox *)x);
                    return;
                }
            }
            for(int i = 3; i < ac; i++)
            {
                if(atom_gettype(av+i) == A_SYM)
                {
                    strcat(description, atom_getsym(av+i)->s_name);
                    strcat(description, " ");
                }
                else if(atom_gettype(av+i) == A_LONG)
                {
                    sprintf(number, "%ld ", (long)atom_getlong(av+i));
                    strcat(description, number);
                }
                else if(atom_gettype(av+i) == A_FLOAT)
                {
                    sprintf(number, "%f ", atom_getfloat(av+i));
                    strcat(description, number);
                }
            }
            x->f_source_manager->sourceSetDescription(index, description);
        }
        else if(param == hoa_sym_color && ac >= 5)
        {
            x->f_source_manager->sourceSetColor(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4), atom_getfloat(av+5));
			causeOutput = 0;
        }
		else
		{
			causeOutput = 0;
		}
		
		object_notify(x, hoa_sym_modified, NULL);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
		jbox_redraw((t_jbox *)x);
		
		if (causeOutput)
		{
			hoamap_output(x);
			hoamap_send_binded_map_update(x, BMAP_OUTPUT);
		}
		
		hoamap_send_binded_map_update(x, BMAP_REDRAW | BMAP_NOTIFY);
    }
}

void hoamap_group(t_hoa_map *x, t_symbol *s, short ac, t_atom *av)
{
	if ( ac && av && atom_gettype(av) == A_SYM && atom_getsym(av) == hoa_sym_group_preset_data)
	{
		av++; ac--;
		
		// group / index / exist / symbol(srcIndex1, srcIndex2...) / r / g / b / a / description, .. groupe / index ...
		
		int index;
		int exist;
		long sources_ac;
		t_atom* sources_av;
		
		for ( int i = 0; i < MAX_NUMBER_OF_SOURCES; i++)
			x->f_source_manager->groupRemove(i);
		
		for(int i = 0; i < MAX_NUMBER_OF_SOURCES*9; i += 9)
		{
			if( (i <= ac-9) && atom_gettype(av+i) == A_SYM && atom_getsym(av+i) == hoa_sym_group)
			{
				if ( (atom_gettype(av+i+1) == A_LONG || atom_gettype(av+i+1) == A_FLOAT) &&
					 (atom_gettype(av+i+2) == A_LONG || atom_gettype(av+i+2) == A_FLOAT))
				{
					index = atom_getlong(av+i+1) -1;
					exist = atom_getlong(av+i+2);
					
					if(exist &&
					   atom_gettype(av+i+3) == A_SYM &&
					   atom_gettype(av+i+4) == A_FLOAT && atom_gettype(av+i+5) == A_FLOAT &&
					   atom_gettype(av+i+6) == A_FLOAT && atom_gettype(av+i+7) == A_FLOAT &&
					   atom_gettype(av+i+8) == A_SYM)
					{
						sources_ac = 0;
						sources_av = NULL;
						atom_setparse(&sources_ac, &sources_av, atom_getsym(av+i+3)->s_name);
						
						if (sources_ac && sources_av)
						{
							for(long j = 0; j < sources_ac; j++)
								if (atom_gettype(sources_av+j) == A_LONG)
									x->f_source_manager->groupSetSource(index, atom_getlong(sources_av+j) - 1);
						}
						
						x->f_source_manager->groupSetColor(index,
														   atom_getfloat(av+i+4),
														   atom_getfloat(av+i+5),
														   atom_getfloat(av+i+6),
														   atom_getfloat(av+i+7));
						
						x->f_source_manager->groupSetDescription(index, atom_getsym(av+i+8)->s_name);
					}
				}
			}
		}
		
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
		jbox_redraw((t_jbox *)x);
		hoamap_output(x);
	}
    else if(ac > 1 && av && atom_gettype(av) == A_LONG && atom_getlong(av) >= 0 && atom_gettype(av+1) == A_SYM)
    {
		long index = atom_getlong(av) - 1;
		t_symbol* param = atom_getsym(av+1);
		int causeOutput = 1;
		
        if(param == hoa_sym_set)
        {
            x->f_source_manager->groupRemove(index);
            for(int i = 2; i < ac; i++)
			{
                x->f_source_manager->groupSetSource(index, atom_getlong(av+i) -1);
			}
        }
        else if(param == hoa_sym_polar || param == hoa_sym_pol)
		{
			if (ac >= 5 && atom_isNumber(av+2) && atom_isNumber(av+3) && atom_isNumber(av+4))
				x->f_source_manager->groupSetPolar(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4));
			else if (ac >= 4 && atom_isNumber(av+2) && atom_isNumber(av+3))
				x->f_source_manager->groupSetPolar(index, atom_getfloat(av+2), atom_getfloat(av+3));
		}
        else if(param == hoa_sym_azimuth)
			x->f_source_manager->groupSetAzimuth(index, atom_getfloat(av+2));
		else if(param == hoa_sym_elevation)
			x->f_source_manager->groupSetElevation(index, atom_getfloat(av+2));
        else if(param == hoa_sym_cartesian || param == hoa_sym_car)
		{
			if (ac >= 5 && atom_isNumber(av+2) && atom_isNumber(av+3) && atom_isNumber(av+4))
				x->f_source_manager->groupSetCartesian(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4));
			else if (ac >= 4 && atom_isNumber(av+2) && atom_isNumber(av+3))
				x->f_source_manager->groupSetCartesian(index, atom_getfloat(av+2), atom_getfloat(av+3));
		}
        else if(param == hoa_sym_abscissa)
			x->f_source_manager->groupSetAbscissa(index, atom_getfloat(av+2));
        else if(param == hoa_sym_ordinate)
            x->f_source_manager->groupSetOrdinate(index, atom_getfloat(av+2));
		else if(param == hoa_sym_height)
            x->f_source_manager->sourceSetHeight(index, atom_getfloat(av+2));
        else if(param == hoa_sym_relpolar)
		{
			if (ac >= 5 && atom_isNumber(av+2) && atom_isNumber(av+3) && atom_isNumber(av+4))
				x->f_source_manager->groupSetRelativePolar(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4));
			else if (ac >= 4 && atom_isNumber(av+2) && atom_isNumber(av+3))
				x->f_source_manager->groupSetRelativePolar(index, atom_getfloat(av+2), atom_getfloat(av+3));
		}
        else if(param == hoa_sym_relradius)
		{
            x->f_source_manager->groupSetRelativeRadius(index, atom_getfloat(av+2));
		}
        else if(param == hoa_sym_relazimuth)
		{
            x->f_source_manager->groupSetRelativeAzimuth(index, atom_getfloat(av+2));
		}
		else if(param == hoa_sym_relelevation)
		{
            x->f_source_manager->groupSetRelativeElevation(index, atom_getfloat(av+2));
		}
        else if(param == hoa_sym_mute)
		{
            x->f_source_manager->groupSetMute(index, atom_getlong(av+2));
		}
        else if(param == hoa_sym_remove)
        {
            x->f_source_manager->groupRemove(index);
            t_atom av[3];
            atom_setlong(av, index+1);
            atom_setsym(av+1, hoa_sym_mute);
            atom_setlong(av+2, 1);
            outlet_list(x->f_out_groups, 0L, 3, av);
        }
        else if(param == hoa_sym_description)
        {
			causeOutput = 0;
            char description[250];
            char number[250];
            if(atom_gettype(av+1) == A_SYM)
            {
                strcpy(description, atom_getsym(av+2)->s_name);
                strcat(description, " ");
                if(atom_getsym(av+2) == hoa_sym_remove)
                {
                    x->f_source_manager->groupSetDescription(index, "");
                    object_notify(x, hoa_sym_modified, NULL);
                    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
                    jbox_redraw((t_jbox *)x);
                    return;
                }
            }
            for(int i = 3; i < ac; i++)
            {
                if(atom_gettype(av+i) == A_SYM)
                {
                    strcat(description, atom_getsym(av+i)->s_name);
                    strcat(description, " ");
                }
                else if(atom_gettype(av+i) == A_LONG)
                {
                    sprintf(number, "%ld ", (long)atom_getlong(av+i));
                    strcat(description, number);
                }
                else if(atom_gettype(av+i) == A_FLOAT)
                {
                    sprintf(number, "%f ", atom_getfloat(av+i));
                    strcat(description, number);
                }
            }
            x->f_source_manager->groupSetDescription(index, description);
        }
        else if(param == hoa_sym_color && ac >= 6)
        {
			causeOutput = 0;
            x->f_source_manager->groupSetColor(index, atom_getfloat(av+2), atom_getfloat(av+3), atom_getfloat(av+4), atom_getfloat(av+5));
        }
		else
		{
			causeOutput = 0;
		}
		
		object_notify(x, hoa_sym_modified, NULL);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
		jbox_redraw((t_jbox *)x);
		if (causeOutput)
		{
			hoamap_output(x);
			hoamap_send_binded_map_update(x, BMAP_OUTPUT);
		}
		hoamap_send_binded_map_update(x, BMAP_REDRAW | BMAP_NOTIFY);
    }
}

/**********************************************************/
/*                  Preset et Pattr                       */
/**********************************************************/

void hoamap_preset(t_hoa_map *x)
{
	void* z;
	double* color;
	bool exist;
	long ac;
	t_atom* av;
	t_atom* avptr;
	
    if(!(z = gensym("_preset")->s_thing))
        return;
	
	if(!x->f_source_manager->getExistence())
		return;
	
	
	// source / index / exist / abscissa / ordinate / height / mutestate / r / g / b / a / description, .. source / index ...
	
	ac = (MAX_NUMBER_OF_SOURCES * 12 + 4);
	av = (t_atom*)getbytes(ac * sizeof(t_atom));
	avptr = av;
	
	atom_setobj(avptr++, x);
    atom_setsym(avptr++, object_classname(x));
	atom_setsym(avptr++, hoa_sym_source);
	atom_setsym(avptr++, hoa_sym_source_preset_data);
	
	for(int i = 0; i < MAX_NUMBER_OF_SOURCES; i++)
	{
		atom_setsym(avptr++, hoa_sym_source);
		atom_setlong(avptr++, i+1);
		exist = x->f_source_manager->sourceGetExistence(i);
		atom_setlong(avptr++, exist);
		
		if(exist)
		{
			color = x->f_source_manager->sourceGetColor(i);
			
			atom_setfloat(avptr++, x->f_source_manager->sourceGetAbscissa(i));
			atom_setfloat(avptr++, x->f_source_manager->sourceGetOrdinate(i));
			atom_setfloat(avptr++, x->f_source_manager->sourceGetHeight(i));
			atom_setlong(avptr++, x->f_source_manager->sourceGetMute(i));
			atom_setfloat(avptr++, color[0]);
			atom_setfloat(avptr++, color[1]);
			atom_setfloat(avptr++, color[2]);
			atom_setfloat(avptr++, color[3]);
			atom_setsym(avptr++, gensym(x->f_source_manager->sourceGetDescription(i).c_str()));
		}
		else
		{
			atom_setfloat(avptr++, 0.);
			atom_setfloat(avptr++, 0.);
			atom_setfloat(avptr++, 0.);
			atom_setlong (avptr++, 0 );
			atom_setfloat(avptr++, 0.);
			atom_setfloat(avptr++, 0.);
			atom_setfloat(avptr++, 0.);
			atom_setfloat(avptr++, 1.);
			atom_setsym(avptr++, hoa_sym_nothing);
		}
	}
	
	binbuf_insert(z, NULL, (MAX_NUMBER_OF_SOURCES * 12 + 4), av);
	
	// group / index / exist / symbol(srcIndex1_srcIndex2_...) / r / g / b / a / description, .. groupe / index ...

	int number_of_sources = 0;
	std::string temp_str = "";
	char temp_char[4];
	avptr = av;
	
	atom_setobj(avptr++, x);
    atom_setsym(avptr++, object_classname(x));
	atom_setsym(avptr++, hoa_sym_group);
	
	atom_setsym(avptr++, hoa_sym_group_preset_data);
	
	for(int i = 0; i < MAX_NUMBER_OF_SOURCES; i++)
	{
		atom_setsym(avptr++, hoa_sym_group);
		atom_setlong(avptr++, i+1);
		exist = x->f_source_manager->groupGetExistence(i);
		atom_setlong(avptr++, exist);
		
		if(exist)
		{
			number_of_sources = x->f_source_manager->groupGetNumberOfSources(i);
			temp_str.clear();
			
			for(long j = 0; j < number_of_sources; j++)
			{
				if (j) temp_str.push_back(' ');
				sprintf(temp_char, "%ld", x->f_source_manager->groupGetSourceIndex(i, j) + 1);
				temp_str += temp_char;
			}
			atom_setsym(avptr++, gensym(temp_str.c_str()));
			
			color = x->f_source_manager->groupGetColor(i);
			atom_setfloat(avptr++, color[0]);
			atom_setfloat(avptr++, color[1]);
			atom_setfloat(avptr++, color[2]);
			atom_setfloat(avptr++, color[3]);
			
			atom_setsym(avptr++, gensym(x->f_source_manager->groupGetDescription(i).c_str()));
		}
		else
		{
			atom_setsym(avptr++, hoa_sym_nothing);
			atom_setfloat(avptr++, 0);
			atom_setfloat(avptr++, 0);
			atom_setfloat(avptr++, 0);
			atom_setfloat(avptr++, 0);
			atom_setsym(avptr++, hoa_sym_nothing);
		}
	}
	
	binbuf_insert(z, NULL, (MAX_NUMBER_OF_SOURCES * 9 + 4), av);
	freebytes(av, ac * sizeof(t_atom));
}

t_max_err hoamap_setvalueof(t_hoa_map *x, long ac, t_atom *av)
{
	int source_ac = MAX_NUMBER_OF_SOURCES * 12;
	int group_ac = MAX_NUMBER_OF_SOURCES * 9;
	
	if ( (ac >= source_ac + 1) && av && atom_gettype(av) == A_SYM && atom_getsym(av) == hoa_sym_source_preset_data)
	{
		object_method(x, hoa_sym_source, NULL, source_ac + 1, av);
	}
	if ((ac >= (source_ac + group_ac + 2)) && (av + source_ac + 1) && atom_gettype(av + source_ac + 1) == A_SYM && atom_getsym(av + source_ac + 1) == hoa_sym_group_preset_data)
	{
		object_method(x, hoa_sym_group, NULL, ac - (group_ac + 1), av + source_ac + 1);
	}
	return MAX_ERR_NONE;
}

t_max_err hoamap_getvalueof(t_hoa_map *x, long *ac, t_atom **av)
{
	if(ac && av)
    {
		int exist;
		double *color;
		t_atom *avptr;
		int number_of_sources = 0;
		std::string temp_str = "";
		char temp_char[4];
		long max_ac = (MAX_NUMBER_OF_SOURCES * 12) + (MAX_NUMBER_OF_SOURCES * 9) + 2;
		
		if(*ac > 0)
			freebytes(*av, *ac * sizeof(t_atom));
		
		*ac = max_ac;
		avptr = *av = (t_atom *)getbytes(*ac * sizeof(t_atom));
		
		atom_setsym(avptr++, hoa_sym_source_preset_data);
		
		for(int i = 0; i < MAX_NUMBER_OF_SOURCES; i++)
		{
			atom_setsym(avptr++, hoa_sym_source);
			atom_setlong(avptr++, i+1);
			exist = x->f_source_manager->sourceGetExistence(i);
			atom_setlong(avptr++, exist);
			
			if(exist)
			{
				color = x->f_source_manager->sourceGetColor(i);
				
				atom_setfloat(avptr++, x->f_source_manager->sourceGetAbscissa(i));
				atom_setfloat(avptr++, x->f_source_manager->sourceGetOrdinate(i));
				atom_setfloat(avptr++, x->f_source_manager->sourceGetHeight(i));
				atom_setlong(avptr++, x->f_source_manager->sourceGetMute(i));
				atom_setfloat(avptr++, color[0]);
				atom_setfloat(avptr++, color[1]);
				atom_setfloat(avptr++, color[2]);
				atom_setfloat(avptr++, color[3]);
				
				if (x->f_source_manager->sourceGetDescription(i).empty())
					atom_setsym(avptr++, hoa_sym_nothing);
				else
					atom_setsym(avptr++, gensym(x->f_source_manager->sourceGetDescription(i).c_str()));
			}
			else
			{
				atom_setfloat(avptr++, 0.);
				atom_setfloat(avptr++, 0.);
				atom_setfloat(avptr++, 0.);
				atom_setlong (avptr++, 0 );
				atom_setfloat(avptr++, 0.);
				atom_setfloat(avptr++, 0.);
				atom_setfloat(avptr++, 0.);
				atom_setfloat(avptr++, 1.);
				atom_setsym(avptr++, hoa_sym_nothing);
			}
		}
		
		// GROUPS :
		
		atom_setsym(avptr++, hoa_sym_group_preset_data);
		
		for(int i = 0; i < MAX_NUMBER_OF_SOURCES; i++)
		{
			atom_setsym(avptr++, hoa_sym_group);
			atom_setlong(avptr++, i+1);
			exist = x->f_source_manager->groupGetExistence(i);
			atom_setlong(avptr++, exist);
			
			if(exist)
			{
				number_of_sources = x->f_source_manager->groupGetNumberOfSources(i);
				temp_str.clear();
				
				for(long j = 0; j < number_of_sources; j++)
				{
					if (j) temp_str.push_back(' ');
					sprintf(temp_char, "%ld", x->f_source_manager->groupGetSourceIndex(i, j) + 1);
					temp_str += temp_char;
				}
				atom_setsym(avptr++, gensym(temp_str.c_str()));
				
				color = x->f_source_manager->groupGetColor(i);
				atom_setfloat(avptr++, color[0]);
				atom_setfloat(avptr++, color[1]);
				atom_setfloat(avptr++, color[2]);
				atom_setfloat(avptr++, color[3]);
				
				if (x->f_source_manager->groupGetDescription(i).empty())
					atom_setsym(avptr++, hoa_sym_nothing);
				else
					atom_setsym(avptr++, gensym(x->f_source_manager->groupGetDescription(i).c_str()));
			}
			else
			{
				atom_setsym(avptr++, hoa_sym_nothing);
				atom_setfloat(avptr++, 0);
				atom_setfloat(avptr++, 0);
				atom_setfloat(avptr++, 0);
				atom_setfloat(avptr++, 0);
				atom_setsym(avptr++, hoa_sym_nothing);
			}
		}
    }
	
	return MAX_ERR_NONE;
}

void hoamap_jsave(t_hoa_map *x, t_dictionary *d)
{
	if (x->f_save_with_patcher)
	{
		long ac = 0;
		t_atom* av = NULL;
		hoamap_getvalueof(x, &ac, &av);
		dictionary_appendatoms(d, gensym("map_saved_state"), ac, av);
		freebytes(av, ac * sizeof(t_atom));
	}
	else if(dictionary_hasentry(d, gensym("map_saved_state")))
	{
		dictionary_chuckentry(d, gensym("map_saved_state"));
	}
}

/**********************************************************/
/*                      Zoom and Notify                   */
/**********************************************************/

t_max_err hoamap_zoom(t_hoa_map *x, t_object *attr, long argc, t_atom *argv)
{
    if(argc >= 1 && argv && atom_gettype(argv) == A_FLOAT)
            x->f_zoom_factor = clip_minmax(atom_getfloat(argv), MIN_ZOOM, MAX_ZOOM);
    
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_background_layer);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
    return MAX_ERR_NONE;
}

t_max_err hoamap_notify(t_hoa_map *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	t_symbol *name;
	
    if (msg == hoa_sym_free)
    {
		if (sender == x->f_patcher)
        {
			x->f_patcher = NULL;
            x->f_textfield = NULL;
            x->f_colorpicker = NULL;
        }
	}
    else if (msg == hoa_sym_endeditbox)
    {
        if(x->f_textfield)
            object_free(x->f_textfield);
        if(x->f_patcher)
            object_free(x->f_patcher);
    }
    else if(msg == hoa_sym_text)
    {
        if (sender == x->f_textfield)
        {
            if(x->f_index_of_source_to_color > -1)
            {
                x->f_source_manager->sourceSetDescription(x->f_index_of_source_to_color, (char *)data);
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
                object_notify(x, hoa_sym_modified, NULL);
				hoamap_send_binded_map_update(x, BMAP_NOTIFY);
            }
            else if(x->f_index_of_group_to_color > -1)
            {
                x->f_source_manager->groupSetDescription(x->f_index_of_group_to_color, (char *)data);
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
                object_notify(x, hoa_sym_modified, NULL);
				hoamap_send_binded_map_update(x, BMAP_NOTIFY);
            }
        }
        jbox_redraw((t_jbox *)x);
    }
	if (msg == hoa_sym_attr_modified)
    {
        if (sender == x->f_colorpicker)
        {
            t_atom *av = NULL;
            long    ac = 0;

            name = (t_symbol *)object_method(data, hoa_sym_getname);
            if(name == gensym("currentcolor"))
            {
                object_attr_getvalueof(sender, name, &ac, &av);
                if (ac && av)
                {
                    if(x->f_index_of_source_to_color > -1)
                    {
                        x->f_source_manager->sourceSetColor(x->f_index_of_source_to_color, atom_getfloat(av), atom_getfloat(av+1), atom_getfloat(av+2), atom_getfloat(av+3));
                        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
                        object_notify(x, hoa_sym_modified, NULL);
						hoamap_send_binded_map_update(x, BMAP_NOTIFY | BMAP_REDRAW);
                    }
                    else if(x->f_index_of_group_to_color > -1)
                    {
                        x->f_source_manager->groupSetColor(x->f_index_of_group_to_color, atom_getfloat(av), atom_getfloat(av+1), atom_getfloat(av+2), atom_getfloat(av+3));
                        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
                        object_notify(x, hoa_sym_modified, NULL);
						hoamap_send_binded_map_update(x, BMAP_NOTIFY | BMAP_REDRAW);
                    }
                }
            }
        }
        else
        {
            name = (t_symbol *)object_method((t_object *)data, hoa_sym_getname);
            if(name == hoa_sym_bgcolor)
            {
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_background_layer);
            }
			else if(name == gensym("showgroups"))
            {
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
            }
			else if(name == gensym("view"))
            {
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
            }
            else if(name == gensym("fontname") || name == gensym("fontface") || name == gensym("fontsize"))
            {
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
            }
            else if(name == gensym("zoom"))
            {
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_background_layer);
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
                jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
            }
        }
        jbox_redraw((t_jbox *)x);
    }
        
            
	return jbox_notify((t_jbox *)x, s, msg, sender, data);
}

/**********************************************************/
/*                          Sortie                        */
/**********************************************************/

void hoamap_bang(t_hoa_map *x)
{
    hoamap_output(x);
}

void hoamap_output(t_hoa_map *x)
{
	if (!x->f_output_enabled)
		return;
	
	t_atom av[5];
    atom_setsym(av+1, hoa_sym_mute);
	
	// output group mute state
	for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
    {
        if(x->f_source_manager->groupGetExistence(i))
        {
            atom_setlong(av, i+1);
            atom_setfloat(av+2, x->f_source_manager->groupGetMute(i));
            outlet_list(x->f_out_groups, 0L, 4, av);
        }
    }
	// output source mute state
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
    {
        if(x->f_source_manager->sourceGetExistence(i))
        {
            atom_setlong(av, i+1);
            atom_setlong(av+2, x->f_source_manager->sourceGetMute(i));
            outlet_list(x->f_out_sources, 0L, 3, av);
        }
    }
    if(x->f_output_mode == 0)
    {
        atom_setsym(av+1, hoa_sym_polar);
		for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
        {
            if(x->f_source_manager->groupGetExistence(i))
            {
				atom_setlong(av, i+1);
				atom_setfloat(av+2, x->f_source_manager->sourceGetRadius(i));
				atom_setfloat(av+3, x->f_source_manager->groupGetAzimuth(i));
				atom_setfloat(av+4, x->f_source_manager->groupGetElevation(i));
				outlet_list(x->f_out_groups, 0L, 5, av);
            }
        }
        for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
        {
            if(x->f_source_manager->sourceGetExistence(i))
            {
                atom_setlong(av, i+1);
				atom_setfloat(av+2, x->f_source_manager->sourceGetRadius(i));
				atom_setfloat(av+3, x->f_source_manager->sourceGetAzimuth(i));
				atom_setfloat(av+4, x->f_source_manager->sourceGetElevation(i));
				outlet_list(x->f_out_sources, 0L, 5, av);
            }
        }
    }
    else
    {
        atom_setsym(av+1, hoa_sym_cartesian);
		for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
        {
            if(x->f_source_manager->groupGetExistence(i))
            {
				atom_setlong(av, i+1);
                atom_setfloat(av+2, x->f_source_manager->groupGetAbscissa(i));
				atom_setfloat(av+3, x->f_source_manager->groupGetOrdinate(i));
				atom_setfloat(av+4, x->f_source_manager->groupGetHeight(i));
				outlet_list(x->f_out_groups, 0L, 5, av);
            }
        }
        for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
        {
            if(x->f_source_manager->sourceGetExistence(i))
            {
                atom_setlong(av, i+1);
				atom_setfloat(av+2, x->f_source_manager->sourceGetAbscissa(i));
				atom_setfloat(av+3, x->f_source_manager->sourceGetOrdinate(i));
				atom_setfloat(av+4, x->f_source_manager->sourceGetHeight(i));
				outlet_list(x->f_out_sources, 0L, 5, av);
            }
        }
    }
}

void hoamap_infos(t_hoa_map *x)
{
    t_atom avNumber[3];
    t_atom* avIndex;
    t_atom* avSource;
    t_atom avMute[4];
    
    /* Sources */
    long numberOfSource = 0;
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
    {
        if(x->f_source_manager->sourceGetExistence(i))
            numberOfSource++;
    }
	
    atom_setsym(avNumber, hoa_sym_source);
    atom_setsym(avNumber+1, hoa_sym_number);
    atom_setlong(avNumber+2, numberOfSource);
    outlet_list(x->f_out_infos, 0L, 3, avNumber);
    
    avIndex = new t_atom[numberOfSource+2];
    atom_setsym(avIndex, hoa_sym_source);
    atom_setsym(avIndex+1, hoa_sym_index);
    for(int i = 0, j = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
    {
        if(x->f_source_manager->sourceGetExistence(i))
        {
            atom_setlong(avIndex+j+2, i+1);
            j++;
        }
    }
    outlet_list(x->f_out_infos, 0L, numberOfSource+2, avIndex);
    delete [] avIndex;
    
    atom_setsym(avMute, hoa_sym_source);
    atom_setsym(avMute+1, hoa_sym_mute);
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
    {
        if(x->f_source_manager->sourceGetExistence(i))
        {
            atom_setlong(avMute+2, i+1);
            atom_setlong(avMute+3, x->f_source_manager->sourceGetMute(i));
            outlet_list(x->f_out_infos, 0L, 4, avMute);
        }
    }
    
    /* Groups */
    long numberOfGroups = 0;
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
    {
        if(x->f_source_manager->groupGetExistence(i))
        {
            numberOfGroups++;
        }
    }
    atom_setsym(avNumber, hoa_sym_group);
    atom_setsym(avNumber+1, hoa_sym_number);
    atom_setlong(avNumber+2, numberOfGroups);
    outlet_list(x->f_out_infos, 0L, 3, avNumber);
    
    avIndex = new t_atom[numberOfGroups+2];
    atom_setsym(avIndex, hoa_sym_group);
    atom_setsym(avIndex+1, hoa_sym_index);
    for(int i = 0, j = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
    {
        if(x->f_source_manager->groupGetExistence(i))
        {
            atom_setlong(avIndex+j+2, i+1);
            j++;
            avSource = new t_atom[x->f_source_manager->groupGetNumberOfSources(i)+3];
            atom_setsym(avSource, hoa_sym_group);
            atom_setsym(avSource+1, hoa_sym_source);
            atom_setlong(avSource+2, i+1);
            for(int k = 0; k < x->f_source_manager->groupGetNumberOfSources(i); k++)
            {
                atom_setlong(avSource+3+k, x->f_source_manager->groupGetSourceIndex(i, k) + 1);
            }
            outlet_list(x->f_out_infos, 0L, x->f_source_manager->groupGetNumberOfSources(i)+3, avSource);
            delete [] avSource;
        }
    }
    outlet_list(x->f_out_infos, 0L, numberOfGroups+2, avIndex);
    delete [] avIndex;
    
    atom_setsym(avMute, hoa_sym_group);
    atom_setsym(avMute+1, hoa_sym_mute);
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
    {
        if(x->f_source_manager->groupGetExistence(i))
        {
            atom_setlong(avMute+2, i+1);
            atom_setlong(avMute+3, x->f_source_manager->groupGetMute(i));
            outlet_list(x->f_out_infos, 0L, 4, avMute);
        }
    }
}

/**********************************************************/
/*                          Paint                         */
/**********************************************************/

void hoamap_paint(t_hoa_map *x, t_object *view)
{
	t_rect rect;
	jbox_get_rect_for_view((t_object *)x, view, &rect);
	x->rect = rect;
    
    /* Pas de groupes avec un nombre de source inférieur à 2 et pas de doublons de groupes */
    
    x->f_source_manager->groupClean();
    
	draw_background(x, view, &rect);
    draw_sources(x, view, &rect);
	if (x->f_showgroups)
	{
		draw_groups(x, view, &rect);
		draw_rect_selection(x, view, &rect);
	}
}

void draw_background(t_hoa_map *x,  t_object *view, t_rect *rect)
{
    t_jgraphics *g;
    t_jrgba black;
    double w = rect->width;
    double h = rect->height;
    t_pt ctr = {w*0.5, h*0.5};
    double maxctr = max(w, h)*0.5;
    
    t_jrgba bgcolor = x->f_color_bg;
    vector_add(3, (double*)&bgcolor, -0.1);
    vector_clip_minmax(3, (double*)&bgcolor, 0., 1.);
    
    double contrastBlack = 0.12;
    black = x->f_color_bg;
	
	vector_add(3, (double*)&black, -contrastBlack);
	vector_clip_minmax(3, (double*)&black, 0., 1.);
	
	g = jbox_start_layer((t_object *)x, view, hoa_sym_background_layer, w, h);
	
	if (g)
    {
        jgraphics_rectangle(g, 0., 0., rect->width, rect->height);
        jgraphics_set_source_jrgba(g, &bgcolor);
        jgraphics_fill(g);
        
        jgraphics_set_source_jrgba(g, &x->f_color_bg);
        jgraphics_set_line_width(g, 1);
        jgraphics_arc(g, ctr.x, ctr.y, maxctr * (1./MIN_ZOOM * x->f_zoom_factor),  0., HOA_2PI);
        jgraphics_fill(g);
        
        double ecart = x->f_zoom_factor * maxctr;
        if(ecart < 10 && ecart >= 5) ecart *= 4;
        else if(ecart < 5 && ecart > 2.5) ecart *= 8;
        else if(ecart < 2.5) ecart *= 16;
        ecart = int(ecart);
        
        jgraphics_set_source_jrgba(g, &black);
        jgraphics_set_line_width(g, 1);
        
		for(double i = 0; i < maxctr; i += ecart)
        {
            jgraphics_move_to(g, 0., ctr.y - i);
            jgraphics_line_to(g, w, ctr.y - i);
            jgraphics_move_to(g, 0., ctr.y + i);
            jgraphics_line_to(g, w, ctr.y + i);
            jgraphics_move_to(g, ctr.x - i, 0.);
            jgraphics_line_to(g, ctr.x - i, w);
            jgraphics_move_to(g, ctr.x + i, 0.);
            jgraphics_line_to(g, ctr.x + i, w);
            jgraphics_stroke(g);
        }
        
        /* Circles */
        double radius = x->f_zoom_factor * (maxctr*2) / 10.;
        for(int i = 5; i > 0; i--)
        {
            jgraphics_arc(g, ctr.x, ctr.y, (double)i * radius - 1,  0., HOA_2PI);
            jgraphics_stroke(g);
        }
        
		jbox_end_layer((t_object*)x, view, hoa_sym_background_layer);
	}
	jbox_paint_layer((t_object *)x, view, hoa_sym_background_layer, 0., 0.);
}

void draw_sources(t_hoa_map *x,  t_object *view, t_rect *rect)
{
	int i;
	double fontSize;
	t_jtextlayout *jtl;
	t_jrgba sourceColor;
	char description[250];
	
	t_pt sourceDisplayPos, groupDisplayPos, textDisplayPos;
	
	double* color;
    
    double w = rect->width;
    double h = rect->height;
    t_pt ctr = {w*0.5, h*0.5};
	
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, hoa_sym_sources_layer, rect->width, rect->height);
	
    x->jfont = jfont_create(jbox_get_fontname((t_object *)x)->s_name, (t_jgraphics_font_slant)jbox_get_font_slant((t_object *)x), (t_jgraphics_font_weight)jbox_get_font_weight((t_object *)x), jbox_get_fontsize((t_object *)x));
    x->f_size_source = jbox_get_fontsize((t_object *)x) / 2.;
    fontSize = jbox_get_fontsize((t_object *)x);
	
	if (g)
    {
        jtl = jtextlayout_create();
        jgraphics_set_line_width(g, x->f_size_source * 0.2);
		for(i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
        {
            if(x->f_source_manager->sourceGetExistence(i))
            {
				switch (x->f_coord_view)
				{
					case 0 : // XY
					{
						sourceDisplayPos.x = (x->f_source_manager->sourceGetAbscissa(i) * x->f_zoom_factor + 1.) * ctr.x;
						sourceDisplayPos.y = (-x->f_source_manager->sourceGetOrdinate(i) * x->f_zoom_factor + 1.) * ctr.y;
						break;
					}
					case 1 : // XZ
					{
						sourceDisplayPos.x = (x->f_source_manager->sourceGetAbscissa(i) * x->f_zoom_factor + 1.) * ctr.x;
						sourceDisplayPos.y = (-x->f_source_manager->sourceGetHeight(i) * x->f_zoom_factor + 1.) * ctr.y;
						break;
					}
					case 2 : // YZ
					{
						sourceDisplayPos.x = (x->f_source_manager->sourceGetOrdinate(i) * x->f_zoom_factor + 1.) * ctr.x;
						sourceDisplayPos.y = (-x->f_source_manager->sourceGetHeight(i) * x->f_zoom_factor + 1.) * ctr.y;
						break;
					}
					default: break;
				}
				
				color = x->f_source_manager->sourceGetColor(i);
				jrgba_set(&sourceColor, color[0], color[1], color[2], color[3]);
                
                if(!x->f_source_manager->sourceGetDescription(i).empty())
                    sprintf(description,"%i : %s", i+1, x->f_source_manager->sourceGetDescription(i).c_str());
                else
                    sprintf(description,"%i", i+1);
				            
                textDisplayPos.x = sourceDisplayPos.x - 2. * x->f_size_source;
                textDisplayPos.y = sourceDisplayPos.y - x->f_size_source - fontSize - 1.;

                jtextlayout_settextcolor(jtl, &sourceColor);
                jtextlayout_set(jtl, description, x->jfont, textDisplayPos.x, textDisplayPos.y, fontSize * 10., fontSize * 2., JGRAPHICS_TEXT_JUSTIFICATION_LEFT, JGRAPHICS_TEXTLAYOUT_USEELLIPSIS);
                jtextlayout_draw(jtl, g);
			
                if (x->f_index_of_selected_source == i)
                {
                    jgraphics_set_source_jrgba(g, &x->f_color_selection);
                    jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source * 1.5,  0., HOA_2PI);
                    jgraphics_fill(g);
                    
					if (x->f_showgroups)
					{
						int groupIndex;
						for(int index = 0; index < x->f_source_manager->sourceGetNumberOfGroups(i); index++)
						{
							jgraphics_move_to(g, sourceDisplayPos.x, sourceDisplayPos.y);
							groupIndex = x->f_source_manager->sourceGetGroupIndex(i, index);
							
							switch (x->f_coord_view)
							{
								case 0 : // XY
								{
									groupDisplayPos.x = (x->f_source_manager->groupGetAbscissa(groupIndex) * x->f_zoom_factor + 1.) * ctr.x;
									groupDisplayPos.y = (-x->f_source_manager->groupGetOrdinate(groupIndex) * x->f_zoom_factor + 1.) * ctr.y;
									break;
								}
								case 1 : // XZ
								{
									groupDisplayPos.x = (x->f_source_manager->groupGetAbscissa(groupIndex) * x->f_zoom_factor + 1.) * ctr.x;
									groupDisplayPos.y = (-x->f_source_manager->groupGetHeight(groupIndex) * x->f_zoom_factor + 1.) * ctr.y;
									break;
								}
								case 2 : // YZ
								{
									groupDisplayPos.x = (x->f_source_manager->groupGetOrdinate(groupIndex) * x->f_zoom_factor + 1.) * ctr.x;
									groupDisplayPos.y = (-x->f_source_manager->groupGetHeight(groupIndex) * x->f_zoom_factor + 1.) * ctr.y;
									break;
								}
								default: break;
							}
							
							jgraphics_line_to(g, groupDisplayPos.x, groupDisplayPos.y);
							jgraphics_stroke(g);
						}
					}
                }
                
                if(!x->f_source_manager->sourceGetMute(i))
                {
                    jgraphics_set_source_jrgba(g, &sourceColor);
					jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source * 0.7,  0., HOA_2PI);
                    jgraphics_fill(g);
                    //jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source,  0., HOA_2PI);
                    //jgraphics_stroke(g);
                }
                if(x->f_source_manager->sourceGetMute(i))
                {
                    jgraphics_set_source_jrgba(g, &sourceColor);
                    jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source * 0.6,  0., HOA_2PI);
                    jgraphics_fill(g);
                    t_jrgba red = {1., 0., 0., 1.};
                    jgraphics_set_source_jrgba(g, &red); 
                    jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source,  0., HOA_2PI);
                    jgraphics_stroke(g);
                    jgraphics_move_to(g, sourceDisplayPos.x + abscissa(x->f_size_source * 1., HOA_PI2 / 2.), sourceDisplayPos.y + ordinate(x->f_size_source * 1., HOA_PI2 / 2.));
                    jgraphics_line_to(g, sourceDisplayPos.x + abscissa(x->f_size_source * 1., HOA_PI2 * 5. / 2.), sourceDisplayPos.y + ordinate(x->f_size_source * 1., HOA_PI * 5. / 4.));
                    jgraphics_stroke(g);
                }
                
            }
        }
		jtextlayout_destroy(jtl);

		jbox_end_layer((t_object*)x, view, hoa_sym_sources_layer);
    }
	jbox_paint_layer((t_object *)x, view, hoa_sym_sources_layer, 0., 0.);
}


void draw_groups(t_hoa_map *x,  t_object *view, t_rect *rect)
{
	int i;
	double fontSize;
	t_jtextlayout *jtl;
	t_jrgba sourceColor;    
	char description[250] = {0};
	
	t_pt sourceDisplayPos, groupDisplayPos, textDisplayPos;
	
	double* color;
    
    double w = rect->width;
    double h = rect->height;
    t_pt ctr = {w*0.5, h*0.5};
	
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, hoa_sym_groups_layer, w, h);
	x->jfont = jfont_create(jbox_get_fontname((t_object *)x)->s_name, (t_jgraphics_font_slant)jbox_get_font_slant((t_object *)x), (t_jgraphics_font_weight)jbox_get_font_weight((t_object *)x), jbox_get_fontsize((t_object *)x));
    x->f_size_source = jbox_get_fontsize((t_object *)x) / 2.;
    fontSize = jbox_get_fontsize((t_object *)x);
	
	if (g)
    {
        jtl = jtextlayout_create();
        jgraphics_set_line_width(g, x->f_size_source * 0.2);
		for(i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
        {
            if(x->f_source_manager->groupGetExistence(i))
            {
				switch (x->f_coord_view)
				{
					case 0 : // XY
					{
						sourceDisplayPos.x = (x->f_source_manager->groupGetAbscissa(i) * x->f_zoom_factor + 1.) * ctr.x;
						sourceDisplayPos.y = (-x->f_source_manager->groupGetOrdinate(i) * x->f_zoom_factor + 1.) * ctr.y;
						break;
					}
					case 1 : // XZ
					{
						sourceDisplayPos.x = (x->f_source_manager->groupGetAbscissa(i) * x->f_zoom_factor + 1.) * ctr.x;
						sourceDisplayPos.y = (-x->f_source_manager->groupGetHeight(i) * x->f_zoom_factor + 1.) * ctr.y;
						break;
					}
					case 2 : // YZ
					{
						sourceDisplayPos.x = (x->f_source_manager->groupGetOrdinate(i) * x->f_zoom_factor + 1.) * ctr.x;
						sourceDisplayPos.y = (-x->f_source_manager->groupGetHeight(i) * x->f_zoom_factor + 1.) * ctr.y;
						break;
					}
					default: break;
				}
                
				color = x->f_source_manager->groupGetColor(i);
				jrgba_set(&sourceColor, color[0], color[1], color[2], color[3]);
                
                if(!x->f_source_manager->groupGetDescription(i).empty())
                    sprintf(description,"%i : %s", i+1, x->f_source_manager->groupGetDescription(i).c_str());
                else
                    sprintf(description,"%i", i+1);
    
                textDisplayPos.x = sourceDisplayPos.x - 2. * x->f_size_source;
                textDisplayPos.y = sourceDisplayPos.y - x->f_size_source - fontSize - 1.;
                
                jtextlayout_settextcolor(jtl, &sourceColor);
                jtextlayout_set(jtl, description, x->jfont, textDisplayPos.x, textDisplayPos.y, fontSize * 10., fontSize * 2., JGRAPHICS_TEXT_JUSTIFICATION_LEFT, JGRAPHICS_TEXTLAYOUT_USEELLIPSIS);
                jtextlayout_draw(jtl, g);
                
                if (x->f_index_of_selected_group == i)
                {
                    jgraphics_set_source_jrgba(g, &x->f_color_selection);
                    jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source * 1.5,  0., HOA_2PI);
                    jgraphics_fill(g);
                    
                    for(int index = 0; index < x->f_source_manager->groupGetNumberOfSources(i); index++)
                    {
                        jgraphics_move_to(g, sourceDisplayPos.x, sourceDisplayPos.y);
                        int groupIndex = x->f_source_manager->groupGetSourceIndex(i, index);
						
						switch (x->f_coord_view)
						{
							case 0 : // XY
							{
								groupDisplayPos.x = (x->f_source_manager->sourceGetAbscissa(groupIndex) * x->f_zoom_factor + 1.) * ctr.x;
								groupDisplayPos.y = (-x->f_source_manager->sourceGetOrdinate(groupIndex) * x->f_zoom_factor + 1.) * ctr.y;
								break;
							}
							case 1 : // XZ
							{
								groupDisplayPos.x = (x->f_source_manager->sourceGetAbscissa(groupIndex) * x->f_zoom_factor + 1.) * ctr.x;
								groupDisplayPos.y = (-x->f_source_manager->sourceGetHeight(groupIndex) * x->f_zoom_factor + 1.) * ctr.y;
								break;
							}
							case 2 : // YZ
							{
								groupDisplayPos.x = (x->f_source_manager->sourceGetOrdinate(groupIndex) * x->f_zoom_factor + 1.) * ctr.x;
								groupDisplayPos.y = (-x->f_source_manager->sourceGetHeight(groupIndex) * x->f_zoom_factor + 1.) * ctr.y;
								break;
							}
							default: break;
						}
						
                        jgraphics_line_to(g, groupDisplayPos.x, groupDisplayPos.y);
                        jgraphics_stroke(g);
                    }
                }
                jgraphics_set_source_jrgba(g, &sourceColor);
                for(int j = 1; j < 4; j += 2)
                {
                    jgraphics_move_to(g, sourceDisplayPos.x, sourceDisplayPos.y);
                    jgraphics_line_to(g, sourceDisplayPos.x + abscissa(x->f_size_source * 1., HOA_2PI * j / 4. + HOA_PI2 / 2.), sourceDisplayPos.y + ordinate(x->f_size_source * 1., HOA_2PI * j / 4. + HOA_PI2 / 2.));
                }
                jgraphics_stroke(g);
                if(!x->f_source_manager->groupGetMute(i))
                {
                    jgraphics_set_source_jrgba(g, &sourceColor);
                    jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source * 1.,  0., HOA_2PI);
                    jgraphics_stroke(g);
                
                    for(int j = 0; j < 2; j++)
                    {
                        jgraphics_move_to(g, sourceDisplayPos.x, sourceDisplayPos.y);
                        jgraphics_line_to(g, sourceDisplayPos.x + abscissa(x->f_size_source * 1., HOA_2PI * j / 2. + HOA_PI2 / 2.), sourceDisplayPos.y + ordinate(x->f_size_source * 1., HOA_2PI * j / 2. + HOA_PI2 / 2.));
                    }
                }
                jgraphics_stroke(g);
                if(x->f_source_manager->groupGetMute(i))
                {
                    t_jrgba red = {1., 0., 0., 1.};
                    jgraphics_set_source_jrgba(g, &red);
                    jgraphics_arc(g, sourceDisplayPos.x, sourceDisplayPos.y, x->f_size_source,  0., HOA_2PI);
                    jgraphics_stroke(g);
                    for(int j = 0; j < 2; j++)
                    {
                        jgraphics_move_to(g, sourceDisplayPos.x, sourceDisplayPos.y);
                        jgraphics_line_to(g, sourceDisplayPos.x + abscissa(x->f_size_source * 1., HOA_2PI * j / 2. + HOA_PI2 / 2.), sourceDisplayPos.y + ordinate(x->f_size_source * 1., HOA_2PI * j / 2. + HOA_PI2 / 2.));
                    }
                    jgraphics_stroke(g);
                }
                 
            }
        }
		jtextlayout_destroy(jtl);
        
		jbox_end_layer((t_object*)x, view, hoa_sym_groups_layer);
    }
	jbox_paint_layer((t_object *)x, view, hoa_sym_groups_layer, 0., 0.);
}


void draw_rect_selection(t_hoa_map *x,  t_object *view, t_rect *rect)
{
	t_jgraphics *g;
    t_jrgba strokecolor = x->f_color_selection;
    strokecolor.alpha = 0.8;
	t_rect sel;
    
    g = jbox_start_layer((t_object *)x, view, hoa_sym_rect_selection_layer, rect->width, rect->height);
    
	if (g)
    {
		if (x->f_rect_selection_exist)
        {
			sel.x = floor(x->f_rect_selection.x) + 0.5;
			sel.y = floor(x->f_rect_selection.y) + 0.5;
			sel.width = x->f_rect_selection.width;
			sel.height = x->f_rect_selection.height;
			
			jgraphics_set_source_jrgba(g, &x->f_color_selection);
			jgraphics_rectangle(g, sel.x, sel.y, sel.width, sel.height);
			jgraphics_fill(g);
			
			jgraphics_set_source_jrgba(g, &strokecolor);
			jgraphics_set_line_width(g, 1);
			jgraphics_rectangle(g, sel.x, sel.y, sel.width, sel.height);
			jgraphics_stroke(g);
		}
		jbox_end_layer((t_object*)x, view, hoa_sym_rect_selection_layer);
	}
	jbox_paint_layer((t_object *)x, view, hoa_sym_rect_selection_layer, 0., 0.);
}

/**********************************************************/
/*                   Souris et clavier                    */
/**********************************************************/

void hoamap_mousedown(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    t_pt cursor;
    cursor.x = ((pt.x / x->rect.width * 2.) - 1.) / x->f_zoom_factor;
    cursor.y = ((-pt.y / x->rect.height * 2.) + 1.) / x->f_zoom_factor;
    double maxwh = max(x->rect.width, x->rect.height);
    double distanceSelected = (x->f_size_source / maxwh * 2.) / x->f_zoom_factor;
	double distanceSelected_test;
    x->f_cursor_position.x = cursor.x;
    x->f_cursor_position.y = cursor.y;
    
    x->f_index_of_selected_source = -1;
    x->f_index_of_selected_group = -1;
    
    x->f_rect_selection_exist = -1;
    x->f_rect_selection.width = x->f_rect_selection.height = 0.;
	x->f_cartesian_drag = 0;
	
	t_pt displayed_coords;
        
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
    {
		switch (x->f_coord_view)
		{
			case 0 : // XY
			{
				displayed_coords.x = x->f_source_manager->sourceGetAbscissa(i);
				displayed_coords.y = x->f_source_manager->sourceGetOrdinate(i);
				break;
			}
			case 1 : // XZ
			{
				displayed_coords.x = x->f_source_manager->sourceGetAbscissa(i);
				displayed_coords.y = x->f_source_manager->sourceGetHeight(i);
				break;
			}
			case 2 : // YZ
			{
				displayed_coords.x = x->f_source_manager->sourceGetOrdinate(i);
				displayed_coords.y = x->f_source_manager->sourceGetHeight(i);
				break;
			}
			default: break;
		}
		
		distanceSelected_test = distance(displayed_coords.x, displayed_coords.y, cursor.x, cursor.y);
		
        if(x->f_source_manager->sourceGetExistence(i) && distanceSelected_test <= distanceSelected)
        {
            distanceSelected = distanceSelected_test;
            x->f_index_of_selected_source = i;
        }
    }
    
    if(x->f_index_of_selected_source == -1 && x->f_showgroups)
    {
        for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
        {
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					displayed_coords.x = x->f_source_manager->groupGetAbscissa(i);
					displayed_coords.y = x->f_source_manager->groupGetOrdinate(i);
					break;
				}
				case 1 : // XZ
				{
					displayed_coords.x = x->f_source_manager->groupGetAbscissa(i);
					displayed_coords.y = x->f_source_manager->groupGetHeight(i);
					break;
				}
				case 2 : // YZ
				{
					displayed_coords.x = x->f_source_manager->groupGetOrdinate(i);
					displayed_coords.y = x->f_source_manager->groupGetHeight(i);
					break;
				}
				default: break;
			}
			
			distanceSelected_test = distance(displayed_coords.x, displayed_coords.y, cursor.x, cursor.y);
			
			if(x->f_source_manager->groupGetExistence(i) && distanceSelected_test <= distanceSelected)
			{
				distanceSelected = distanceSelected_test;
				x->f_index_of_selected_group = i;
			}
        }
    }

    if(modifiers == 160 || modifiers == 416) // right click
    {
		int causeOutput = 0;
		int causeRedraw = 0;
		int causeNotify = 0;
		
        int posX, posY;
        t_pt pos;
        jmouse_getposition_global(&posX, &posY);
        pos.x = posX;
        pos.y = posY;
        x->f_index_of_source_to_remove = x->f_index_of_selected_source;
        x->f_index_of_group_to_remove = x->f_index_of_selected_group;
        t_jpopupmenu* popup = jpopupmenu_create();
        jpopupmenu_setfont(popup, x->jfont);
       
        if(x->f_index_of_selected_group != -1) 
        {
            x->f_index_of_group_to_color = x->f_index_of_selected_group;
            x->f_index_of_selected_group = -1;
            x->f_index_of_source_to_color = -1;
            jpopupmenu_additem(popup, 0, "Group Menu", NULL, 0, 1, NULL);
            jpopupmenu_addseperator(popup);
            jpopupmenu_additem(popup, 1, "Remove group", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 2, "Remove group and sources", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 3, "Mute group", NULL, 0, x->f_source_manager->groupGetMute(x->f_index_of_group_to_remove), NULL);
            jpopupmenu_additem(popup, 4, "Unmute group", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 5, "Set group color", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 6, "Set group description", NULL, 0, 0, NULL);
            
            int choice = jpopupmenu_popup(popup, pos, 0);
            switch (choice)
            {
                case 1:
                {
                    t_atom av[3];
                    atom_setlong(av, x->f_index_of_group_to_remove+1);
                    atom_setsym(av+1, hoa_sym_mute);
                    atom_setlong(av+2, 1);
                    outlet_list(x->f_out_groups, 0L, 3, av);
                    x->f_source_manager->groupRemove(x->f_index_of_group_to_remove);
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 2:
                {
					x->f_source_manager->groupSetMute(x->f_index_of_group_to_remove, 1);
					hoamap_output(x);
					hoamap_send_binded_map_update(x, BMAP_OUTPUT);
					x->f_source_manager->groupRemoveWithSources(x->f_index_of_group_to_remove);
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 3: // Mute group
                {
                    x->f_source_manager->groupSetMute(x->f_index_of_group_to_remove, 1);
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 4: // Unmute group
                {
                    x->f_source_manager->groupSetMute(x->f_index_of_group_to_remove, 0);
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 5: // Set group color
                {
                    hoamap_color_picker(x);
					causeOutput = causeRedraw = causeNotify = 0;
                    break;
                }
                case 6: // Set group description
                {
                    hoamap_text_field(x);
					causeOutput = causeRedraw = causeNotify = 0;
                    break;
                }
                default:
				{
					causeOutput = causeRedraw = causeNotify = 0;
					break;
				}
            }
        }
        else if(x->f_index_of_selected_source != -1)
        {
            x->f_index_of_source_to_color = x->f_index_of_selected_source;
            x->f_index_of_selected_source = -1;
            x->f_index_of_group_to_color = -1;
            int muted = x->f_source_manager->sourceGetMute(x->f_index_of_source_to_remove);
            jpopupmenu_additem(popup, 0, "Source Menu", NULL, 0, 1, NULL);
            jpopupmenu_addseperator(popup);
            jpopupmenu_additem(popup, 1, "Remove source", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 2, muted ? "Unmute source" : "Mute source", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 3, "Set source color", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 4, "Set source description", NULL, 0, 0, NULL);
            int choice = jpopupmenu_popup(popup, pos, 0);
            switch (choice)
            {
                case 1:
                {
                    t_atom av[3];
                    atom_setlong(av, x->f_index_of_source_to_remove+1);
                    atom_setsym(av+1, hoa_sym_mute);
                    atom_setlong(av+2, 1);
                    outlet_list(x->f_out_sources, 0L, 3, av);
                    x->f_source_manager->sourceRemove(x->f_index_of_source_to_remove);
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 2:
                {
                    if(x->f_source_manager->sourceGetMute(x->f_index_of_source_to_remove))
                        x->f_source_manager->sourceSetMute(x->f_index_of_source_to_remove, 0);
                    else
                        x->f_source_manager->sourceSetMute(x->f_index_of_source_to_remove, 1);
					
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 3:
                {
                    hoamap_color_picker(x);
					causeOutput = causeRedraw = causeNotify = 0;
                    break;
                }
                case 4:
                {
                    hoamap_text_field(x);
					causeOutput = causeRedraw = causeNotify = 0;
                    break;
                }
                default:
				{
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
				}
            }
            jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
            jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
            jbox_redraw((t_jbox *)x);
			hoamap_send_binded_map_update(x, BMAP_REDRAW | BMAP_OUTPUT | BMAP_NOTIFY);
            hoamap_output(x);
        }
        else
        {
            t_jpopupmenu* subpopup = jpopupmenu_create();
            jpopupmenu_setfont(subpopup, x->jfont);
            x->f_index_of_group_to_color = -1;
            x->f_index_of_source_to_color = -2;
            jpopupmenu_additem(popup, 0, "Menu", NULL, 0, 1, NULL);
            jpopupmenu_addseperator(popup);
            jpopupmenu_additem(popup, 1, "Add source", NULL, 0, 0, NULL);
            jpopupmenu_additem(popup, 2, "Clear all", NULL, 0, 0, NULL);
            int choice = jpopupmenu_popup(popup, pos, 0);
            int check = 0;
            switch (choice)
            {
                case 1:
                {
                    for(int i = 0; check == 0; i++)
                    {
                        if (x->f_source_manager->sourceGetExistence(i) < 1)
                        {
                            check = 1;
                            x->f_index_of_selected_source = i;
                            hoamap_mousedrag(x, patcherview, pt, modifiers);
                        }
                    }
					causeOutput = causeRedraw = causeNotify = 1;
                    break;
                }
                case 2: // Clear All
                {
                    hoamap_clear_all(x);
					causeOutput = causeRedraw = causeNotify = 0; // (did in the hoamap_clear_all() function)
                    break;
                }
                default:
				{
					causeOutput = causeRedraw = causeNotify = 0;
                    break;
				}
            }
        }
        jpopupmenu_destroy(popup);
		
		if (causeRedraw)
		{
			jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
			jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
			jbox_redraw((t_jbox *)x);
			hoamap_send_binded_map_update(x, BMAP_REDRAW);
		}
		
		if (causeNotify)
		{
			object_notify(x, hoa_sym_modified, NULL);
			hoamap_send_binded_map_update(x, BMAP_NOTIFY);
		}
		
		if (causeOutput)
		{
			hoamap_output(x);
			hoamap_send_binded_map_update(x, BMAP_OUTPUT);
		}
    }
	
	if (x->f_index_of_selected_group != -1)
	{
		t_pt mouse_pos;
		switch (x->f_coord_view)
		{
			case 0 : // XY
			{
				mouse_pos.x = (x->f_source_manager->groupGetAbscissa(x->f_index_of_selected_group) * x->f_zoom_factor + 1.) * (x->rect.width*0.5);
				mouse_pos.y = (-x->f_source_manager->groupGetOrdinate(x->f_index_of_selected_group) * x->f_zoom_factor + 1.) * (x->rect.height*0.5);
				break;
			}
			case 1 : // XZ
			{
				mouse_pos.x = (x->f_source_manager->groupGetAbscissa(x->f_index_of_selected_group) * x->f_zoom_factor + 1.) * (x->rect.width*0.5);
				mouse_pos.y = (-x->f_source_manager->groupGetHeight(x->f_index_of_selected_group) * x->f_zoom_factor + 1.) * (x->rect.height*0.5);
				break;
			}
			case 2 : // YZ
			{
				mouse_pos.x = (x->f_source_manager->groupGetOrdinate(x->f_index_of_selected_group) * x->f_zoom_factor + 1.) * (x->rect.width*0.5);
				mouse_pos.y = (-x->f_source_manager->groupGetHeight(x->f_index_of_selected_group) * x->f_zoom_factor + 1.) * (x->rect.height*0.5);
				break;
			}
			default: break;
		}
		x->f_cursor_position.x = mouse_pos.x;
		x->f_cursor_position.y = mouse_pos.y;
		jmouse_setposition_box(patcherview, (t_object*)x, mouse_pos.x, mouse_pos.y);
	}
	else if (x->f_index_of_selected_source != -1)
	{
		t_pt mouse_pos;
		switch (x->f_coord_view)
		{
			case 0 : // XY
			{
				mouse_pos.x = (x->f_source_manager->sourceGetAbscissa(x->f_index_of_selected_source) * x->f_zoom_factor + 1.) * (x->rect.width*0.5);
				mouse_pos.y = (-x->f_source_manager->sourceGetOrdinate(x->f_index_of_selected_source) * x->f_zoom_factor + 1.) * (x->rect.height*0.5);
				break;
			}
			case 1 : // XZ
			{
				mouse_pos.x = (x->f_source_manager->sourceGetAbscissa(x->f_index_of_selected_source) * x->f_zoom_factor + 1.) * (x->rect.width*0.5);
				mouse_pos.y = (-x->f_source_manager->sourceGetHeight(x->f_index_of_selected_source) * x->f_zoom_factor + 1.) * (x->rect.height*0.5);
				break;
			}
			case 2 : // YZ
			{
				mouse_pos.x = (x->f_source_manager->sourceGetOrdinate(x->f_index_of_selected_source) * x->f_zoom_factor + 1.) * (x->rect.width*0.5);
				mouse_pos.y = (-x->f_source_manager->sourceGetHeight(x->f_index_of_selected_source) * x->f_zoom_factor + 1.) * (x->rect.height*0.5);
				break;
			}
			default: break;
		}
		x->f_cursor_position.x = mouse_pos.x;
		x->f_cursor_position.y = mouse_pos.y;
		jmouse_setposition_box(patcherview, (t_object*)x, mouse_pos.x, mouse_pos.y);
	}
	
    
    if(x->f_index_of_selected_source == -1 && x->f_index_of_selected_group == -1 && x->f_showgroups)
    {
        x->f_rect_selection.x = pt.x;
        x->f_rect_selection.y = pt.y;
        x->f_rect_selection_exist = 1;
    }
}


void hoamap_mousedrag(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    t_pt cursor;
    cursor.x = ((pt.x / x->rect.width * 2.) - 1.) / x->f_zoom_factor;
    cursor.y = ((-pt.y / x->rect.height * 2.) + 1.) / x->f_zoom_factor;
    
    t_pt mousedelta = {x->f_cursor_position.x - cursor.x, x->f_cursor_position.y - cursor.y};
	
	int causeOutput = 0;
	int causeRedraw = 0;
	int causeNotify = 0;
	
	// check if we wanna constrain drag to cartesian
#ifdef _WINDOWS
	if(modifiers == 24 && x->f_mouse_was_dragging) // alt
#else
	if (modifiers == 17 && x->f_mouse_was_dragging) // cmd
#endif
	{
		if(x->f_cartesian_drag == 0)
			x->f_cartesian_drag = (fabs(mousedelta.x) >= fabs(mousedelta.y)) ? 1 : 2;
	}
	else
		x->f_cartesian_drag = 0;
	
	
	// source is dragged
	if (x->f_index_of_selected_source != -1)
    {
		// Angle
#ifdef _WINDOWS
		if(modifiers == 21) // Ctrl
#else
		if( (modifiers == 148 || modifiers == 404)) // Ctrl
#endif
		{
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					x->f_source_manager->sourceSetAzimuth(x->f_index_of_selected_source, azimuth(cursor.x, cursor.y));
					break;
				}
				case 1 : // XZ
				{
					double source_radius = radius(x->f_source_manager->sourceGetAbscissa(x->f_index_of_selected_source), x->f_source_manager->sourceGetHeight(x->f_index_of_selected_source));
					double mouse_azimuth = wrap_twopi(azimuth(cursor.x, cursor.y));
					
					x->f_source_manager->sourceSetAbscissa(x->f_index_of_selected_source, abscissa(source_radius, mouse_azimuth));
					x->f_source_manager->sourceSetHeight(x->f_index_of_selected_source, ordinate(source_radius, mouse_azimuth));
					break;
				}
				case 2 : // YZ
				{
					double source_radius = radius(x->f_source_manager->sourceGetOrdinate(x->f_index_of_selected_source), x->f_source_manager->sourceGetHeight(x->f_index_of_selected_source));
					double mouse_azimuth = wrap_twopi(azimuth(cursor.x, cursor.y));
					
					x->f_source_manager->sourceSetOrdinate(x->f_index_of_selected_source, abscissa(source_radius, mouse_azimuth));
					x->f_source_manager->sourceSetHeight(x->f_index_of_selected_source, ordinate(source_radius, mouse_azimuth));
					break;
				}
				default: break;
			}
			
			causeOutput = causeRedraw = causeNotify = 1;
		}
		
		 // Radius
#ifdef _WINDOWS
		else if(modifiers == 18) // Shift
#else
		else if(modifiers == 18 || modifiers == 274) // Shift
#endif
		{
            x->f_source_manager->sourceSetRadius(x->f_index_of_selected_source, radius(cursor.x, cursor.y));
			causeOutput = causeRedraw = causeNotify = 1;
		}
		
		 // Cartesian constrain
#ifdef _WINDOWS
		else if(x->f_cartesian_drag) // Shift
#else
		else if (modifiers == 17) // cmd
#endif
        {
            if (x->f_cartesian_drag == 1)
			{
				if (x->f_coord_view == 0 || x->f_coord_view == 1)
					x->f_source_manager->sourceSetAbscissa(x->f_index_of_selected_source, cursor.x);
				else if (x->f_coord_view == 2)
					x->f_source_manager->sourceSetOrdinate(x->f_index_of_selected_source, cursor.x);
			}
            else if(x->f_cartesian_drag == 2)
			{
				if (x->f_coord_view == 0)
					x->f_source_manager->sourceSetOrdinate(x->f_index_of_selected_source, cursor.y);
				else
					x->f_source_manager->sourceSetHeight(x->f_index_of_selected_source, cursor.y);
			}
			
          causeOutput = causeRedraw = causeNotify = 1;
        }
        else
		{
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					x->f_source_manager->sourceSetCartesian(x->f_index_of_selected_source, cursor.x, cursor.y);
					break;
				}
				case 1 : // XZ
				{
					x->f_source_manager->sourceSetCartesian(x->f_index_of_selected_source, cursor.x, x->f_source_manager->sourceGetOrdinate(x->f_index_of_selected_source), cursor.y);
					break;
				}
				case 2 : // YZ
				{
					x->f_source_manager->sourceSetCartesian(x->f_index_of_selected_source, x->f_source_manager->sourceGetAbscissa(x->f_index_of_selected_source), cursor.x, cursor.y);
					break;
				}
				default: break;
			}
			causeOutput = causeRedraw = causeNotify = 1;
		}
    }
	
	// group is dragged
    else if (x->f_index_of_selected_group != -1)
    {

		// Angle
#ifdef _WINDOWS
		if(modifiers == 21) // Ctrl
#else
		if( (modifiers == 148 || modifiers == 404)) // Ctrl
#endif
		{
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					x->f_source_manager->groupSetRelativeAzimuth(x->f_index_of_selected_group, azimuth(cursor.x, cursor.y));
					break;
				}
				case 1 : // XZ
				{
					if (x->f_mouse_was_dragging)
					{
						t_pt source_display;
						int srcIndex, grpIndex;
						double source_radius, source_azimuth, mouse_azimuth, mouse_azimuth_prev;
						grpIndex = x->f_index_of_selected_group;
						mouse_azimuth = wrap_twopi(azimuth(cursor.x, cursor.y));
						mouse_azimuth_prev = wrap_twopi(azimuth(x->f_cursor_position.x, x->f_cursor_position.y));
						
						for(int i = 0; i < x->f_source_manager->groupGetNumberOfSources(grpIndex); i++)
						{
							srcIndex = x->f_source_manager->groupGetSourceIndex(grpIndex, i);
							
							source_display.x = x->f_source_manager->sourceGetAbscissa(srcIndex);
							source_display.y = x->f_source_manager->sourceGetHeight(srcIndex);
							source_radius = radius(source_display.x, source_display.y);
							source_azimuth = azimuth(source_display.x, source_display.y);
							source_azimuth += mouse_azimuth - mouse_azimuth_prev;
							
							x->f_source_manager->sourceSetAbscissa(srcIndex, abscissa(source_radius, source_azimuth));
							x->f_source_manager->sourceSetHeight(srcIndex, ordinate(source_radius, source_azimuth));
						}
					}
					break;
				}
				case 2 : // YZ
				{
					if (x->f_mouse_was_dragging)
					{
						t_pt source_display;
						int srcIndex, grpIndex;
						double source_radius, source_azimuth, mouse_azimuth, mouse_azimuth_prev;
						grpIndex = x->f_index_of_selected_group;
						mouse_azimuth = wrap_twopi(azimuth(cursor.x, cursor.y));
						mouse_azimuth_prev = wrap_twopi(azimuth(x->f_cursor_position.x, x->f_cursor_position.y));
						
						for(int i = 0; i < x->f_source_manager->groupGetNumberOfSources(grpIndex); i++)
						{
							srcIndex = x->f_source_manager->groupGetSourceIndex(grpIndex, i);
							
							source_display.x = x->f_source_manager->sourceGetOrdinate(srcIndex);
							source_display.y = x->f_source_manager->sourceGetHeight(srcIndex);
							source_radius = radius(source_display.x, source_display.y);
							source_azimuth = azimuth(source_display.x, source_display.y);
							source_azimuth += mouse_azimuth - mouse_azimuth_prev;
							
							x->f_source_manager->sourceSetOrdinate(srcIndex, abscissa(source_radius, source_azimuth));
							x->f_source_manager->sourceSetHeight(srcIndex, ordinate(source_radius, source_azimuth));
						}
					}
					break;
				}
				default: break;
			}
			causeOutput = causeRedraw = causeNotify = 1;
		}
		
		 // Radius
#ifdef _WINDOWS
		else if(modifiers == 18) // Shift
#else
		else if(modifiers == 18 || modifiers == 274) // Shift
#endif
		{
            x->f_source_manager->groupSetRelativeRadius(x->f_index_of_selected_group, radius(cursor.x, cursor.y));
			causeOutput = causeRedraw = causeNotify = 1;
		}
		
		 // Angle + radius
#ifdef _WINDOWS
		else if(modifiers == 23) // Shift + Ctrl
#else
		else if (modifiers == 150 || modifiers == 406) // Shift + alt (Maj on/off)
#endif
		{
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					x->f_source_manager->groupSetRelativePolar(x->f_index_of_selected_group, radius(cursor.x, cursor.y), azimuth(cursor.x, cursor.y));
					break;
				}
				case 1 : // XZ
				{
					if (x->f_mouse_was_dragging)
					{
						t_pt source_display;
						int srcIndex, grpIndex;
						double source_radius, source_azimuth, mouse_azimuth, mouse_azimuth_prev, mouse_radius, mouse_radius_prev;
						grpIndex = x->f_index_of_selected_group;
						mouse_radius = clip_min(radius(cursor.x, cursor.y), 0);
						mouse_radius_prev = clip_min(radius(x->f_cursor_position.x, x->f_cursor_position.y), 0);
						mouse_azimuth = wrap_twopi(azimuth(cursor.x, cursor.y));
						mouse_azimuth_prev = wrap_twopi(azimuth(x->f_cursor_position.x, x->f_cursor_position.y));
						
						for(int i = 0; i < x->f_source_manager->groupGetNumberOfSources(grpIndex); i++)
						{
							srcIndex = x->f_source_manager->groupGetSourceIndex(grpIndex, i);
							
							source_display.x = x->f_source_manager->sourceGetAbscissa(srcIndex);
							source_display.y = x->f_source_manager->sourceGetHeight(srcIndex);
							source_radius = radius(source_display.x, source_display.y);
							source_radius += mouse_radius - mouse_radius_prev;
							source_azimuth = azimuth(source_display.x, source_display.y);
							source_azimuth += mouse_azimuth - mouse_azimuth_prev;
							
							x->f_source_manager->sourceSetAbscissa(srcIndex, abscissa(source_radius, source_azimuth));
							x->f_source_manager->sourceSetHeight(srcIndex, ordinate(source_radius, source_azimuth));
						}
					}
					break;
				}
				case 2 : // YZ
				{
					if (x->f_mouse_was_dragging)
					{
						t_pt source_display;
						int srcIndex, grpIndex;
						double source_radius, source_azimuth, mouse_azimuth, mouse_azimuth_prev, mouse_radius, mouse_radius_prev;
						grpIndex = x->f_index_of_selected_group;
						mouse_radius = clip_min(radius(cursor.x, cursor.y), 0);
						mouse_radius_prev = clip_min(radius(x->f_cursor_position.x, x->f_cursor_position.y), 0);
						mouse_azimuth = wrap_twopi(azimuth(cursor.x, cursor.y));
						mouse_azimuth_prev = wrap_twopi(azimuth(x->f_cursor_position.x, x->f_cursor_position.y));
						
						for(int i = 0; i < x->f_source_manager->groupGetNumberOfSources(grpIndex); i++)
						{
							srcIndex = x->f_source_manager->groupGetSourceIndex(grpIndex, i);
							
							source_display.x = x->f_source_manager->sourceGetOrdinate(srcIndex);
							source_display.y = x->f_source_manager->sourceGetHeight(srcIndex);
							source_radius = radius(source_display.x, source_display.y);
							source_radius += mouse_radius - mouse_radius_prev;
							source_azimuth = azimuth(source_display.x, source_display.y);
							source_azimuth += mouse_azimuth - mouse_azimuth_prev;
							
							x->f_source_manager->sourceSetOrdinate(srcIndex, abscissa(source_radius, source_azimuth));
							x->f_source_manager->sourceSetHeight(srcIndex, ordinate(source_radius, source_azimuth));
						}
					}
					break;
				}
				default: break;
			}
			causeOutput = causeRedraw = causeNotify = 1;
		}
		
// constrain drag to cartesian
#ifdef _WINDOWS
		else if(x->f_cartesian_drag) // Alt
#else
		else if (modifiers == 17) // cmd
#endif
		{
			if (x->f_cartesian_drag == 1)
			{
				if (x->f_coord_view == 0 || x->f_coord_view == 1)
					x->f_source_manager->groupSetAbscissa(x->f_index_of_selected_group, cursor.x);
				else if (x->f_coord_view == 2)
					x->f_source_manager->groupSetOrdinate(x->f_index_of_selected_group, cursor.x);
			}
            else if(x->f_cartesian_drag == 2)
			{
				if (x->f_coord_view == 0)
					x->f_source_manager->groupSetOrdinate(x->f_index_of_selected_group, cursor.y);
				else
					x->f_source_manager->groupSetHeight(x->f_index_of_selected_group, cursor.y);
			}
			
			causeOutput = causeRedraw = causeNotify = 1;
		}
        else
		{
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					x->f_source_manager->groupSetCartesian(x->f_index_of_selected_group, cursor.x, cursor.y);
					break;
				}
				case 1 : // XZ
				{
					x->f_source_manager->groupSetAbscissa(x->f_index_of_selected_group, cursor.x);
					x->f_source_manager->groupSetHeight(x->f_index_of_selected_group, cursor.y);
					break;
				}
				case 2 : // YZ
				{
					x->f_source_manager->groupSetOrdinate(x->f_index_of_selected_group, cursor.x);
					x->f_source_manager->groupSetHeight(x->f_index_of_selected_group, cursor.y);
					break;
				}
				default: break;
			}
			causeOutput = causeRedraw = causeNotify = 1;
		}
    }
    else
    {
		x->f_rect_selection.width = pt.x - x->f_rect_selection.x;
		x->f_rect_selection.height = pt.y - x->f_rect_selection.y;
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_rect_selection_layer);
		jbox_redraw((t_jbox *)x);
		causeOutput = causeRedraw = causeNotify = 0;
    }
    
    x->f_cursor_position.x = cursor.x;
    x->f_cursor_position.y = cursor.y;
	x->f_mouse_was_dragging = 1;
	
	if (causeNotify)
	{
		object_notify(x, hoa_sym_modified, NULL);
		hoamap_send_binded_map_update(x, BMAP_NOTIFY);
	}
	
	if (causeRedraw)
	{
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
		jbox_redraw((t_jbox *)x);
		hoamap_send_binded_map_update(x, BMAP_REDRAW);
	}
	
	if (causeOutput)
	{
		hoamap_output(x);
		hoamap_send_binded_map_update(x, BMAP_OUTPUT);
	}
}

void hoamap_mouseup(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    x->f_index_of_selected_source = -1;
    x->f_index_of_selected_group = -1;
	x->f_mouse_was_dragging = 0;
	
	t_pt screen_source_coord;
	
	int causeOutput = 0;
	int causeRedraw = 0;
	int causeNotify = 0;
    
    if(x->f_rect_selection_exist)
    {
        int indexOfNewGroup = -1;
        for(int i = 0; indexOfNewGroup == -1; i++)
        {
            if (x->f_source_manager->groupGetExistence(i) == 0)
            {
                indexOfNewGroup = i;
            }
        }
    
        double x1 = ((x->f_rect_selection.x / x->rect.width * 2.) - 1.) / x->f_zoom_factor;
        double x2 = (((x->f_rect_selection.x + x->f_rect_selection.width) / x->rect.width * 2.) - 1.) / x->f_zoom_factor;
        double y1 = ((-x->f_rect_selection.y / x->rect.height * 2.) + 1.) / x->f_zoom_factor;
        double y2 = (((-x->f_rect_selection.y - x->f_rect_selection.height) / x->rect.height * 2.) + 1.) / x->f_zoom_factor;
        
        for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
        {
            if(x->f_source_manager->sourceGetExistence(i) && indexOfNewGroup >= 0)
            {
				switch (x->f_coord_view)
				{
					case 0 : // XY
					{
						screen_source_coord.x = x->f_source_manager->sourceGetAbscissa(i);
						screen_source_coord.y = x->f_source_manager->sourceGetOrdinate(i);
						break;
					}
					case 1 : // XZ
					{
						screen_source_coord.x = x->f_source_manager->sourceGetAbscissa(i);
						screen_source_coord.y = x->f_source_manager->sourceGetHeight(i);
						break;
					}
					case 2 : // YZ
					{
						screen_source_coord.x = x->f_source_manager->sourceGetOrdinate(i);
						screen_source_coord.y = x->f_source_manager->sourceGetHeight(i);
						break;
					}
					default: break;
				}
                                
                if(((screen_source_coord.x > x1 && screen_source_coord.x < x2) || (screen_source_coord.x < x1 && screen_source_coord.x > x2)) && ((screen_source_coord.y > y1 && screen_source_coord.y < y2) || (screen_source_coord.y < y1 && screen_source_coord.y > y2)))
                {
                    x->f_source_manager->groupSetSource(indexOfNewGroup, i);
                    x->f_index_of_selected_group = indexOfNewGroup;
					causeOutput = causeRedraw = causeNotify = 1;
                }
            }
        }
    }
    
    x->f_rect_selection_exist = x->f_rect_selection.width = x->f_rect_selection.height = 0;
	
	jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_rect_selection_layer);
	jbox_redraw((t_jbox *)x);
    
	if (causeNotify)
	{
		object_notify(x, hoa_sym_modified, NULL);
		hoamap_send_binded_map_update(x, BMAP_NOTIFY);
	}
	
	if (causeRedraw)
	{
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
		jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
		jbox_redraw((t_jbox *)x);
		hoamap_send_binded_map_update(x, BMAP_REDRAW);
	}
	
	if (causeOutput)
	{
		hoamap_output(x);
		hoamap_send_binded_map_update(x, BMAP_OUTPUT);
	}
}

void hoamap_mousewheel(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers, double x_inc, double y_inc)
{
	if (modifiers == eAltKey)
    {
		double newZoom = x->f_zoom_factor + y_inc / 100.;
        x->f_zoom_factor = clip_minmax(newZoom, MIN_ZOOM, MAX_ZOOM);
        object_notify(x, hoa_sym_modified, NULL);
        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_background_layer);
        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
        jbox_redraw((t_jbox *)x);
	}
}

void hoamap_mouseenter(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    ;
}

void hoamap_mousemove(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    t_pt cursor, displayed_coords;
    cursor.x = ((pt.x / x->rect.width * 2.) - 1.) / x->f_zoom_factor;
    cursor.y = ((-pt.y / x->rect.height * 2.) + 1.) / x->f_zoom_factor;
    double maxwh = max(x->rect.width, x->rect.height);
    double distanceSelected = (x->f_size_source / maxwh * 2.) / x->f_zoom_factor;
	double distanceSelected_test;
    x->f_cursor_position.x = cursor.x;
    x->f_cursor_position.y = cursor.y;
    
    x->f_index_of_selected_source = -1;
    x->f_index_of_selected_group = -1;
	
	// test if mouse is over a source
    for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
    {
		switch (x->f_coord_view)
		{
			case 0 : // XY
			{
				displayed_coords.x = x->f_source_manager->sourceGetAbscissa(i);
				displayed_coords.y = x->f_source_manager->sourceGetOrdinate(i);
				break;
			}
			case 1 : // XZ
			{
				displayed_coords.x = x->f_source_manager->sourceGetAbscissa(i);
				displayed_coords.y = x->f_source_manager->sourceGetHeight(i);
				break;
			}
			case 2 : // YZ
			{
				displayed_coords.x = x->f_source_manager->sourceGetOrdinate(i);
				displayed_coords.y = x->f_source_manager->sourceGetHeight(i);
				break;
			}
			default: break;
		}
		
		distanceSelected_test = distance(displayed_coords.x, displayed_coords.y, cursor.x, cursor.y);
		
        if(x->f_source_manager->sourceGetExistence(i) && distanceSelected_test <= distanceSelected)
        {
            distanceSelected = distanceSelected_test;
            x->f_index_of_selected_source = i;
        }
    }
	
	// test if mouse is over a group
    if(x->f_index_of_selected_source == -1 && x->f_showgroups)
    {
		for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfGroup(); i++)
        {
			switch (x->f_coord_view)
			{
				case 0 : // XY
				{
					displayed_coords.x = x->f_source_manager->groupGetAbscissa(i);
					displayed_coords.y = x->f_source_manager->groupGetOrdinate(i);
					break;
				}
				case 1 : // XZ
				{
					displayed_coords.x = x->f_source_manager->groupGetAbscissa(i);
					displayed_coords.y = x->f_source_manager->groupGetHeight(i);
					break;
				}
				case 2 : // YZ
				{
					displayed_coords.x = x->f_source_manager->groupGetOrdinate(i);
					displayed_coords.y = x->f_source_manager->groupGetHeight(i);
					break;
				}
				default: break;
			}
			
			distanceSelected_test = distance(displayed_coords.x, displayed_coords.y, cursor.x, cursor.y);
			
			if(x->f_source_manager->groupGetExistence(i) && distanceSelected_test <= distanceSelected)
			{
				distanceSelected = distanceSelected_test;
				x->f_index_of_selected_group = i;
			}
        }
    }
    
    if(x->f_index_of_selected_source >= 0 || x->f_index_of_selected_group >= 0)
        jmouse_setcursor(patcherview, (t_object *)x, JMOUSE_CURSOR_POINTINGHAND);
    else jmouse_setcursor(patcherview, (t_object *)x, JMOUSE_CURSOR_ARROW);
    
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
    jbox_redraw((t_jbox *)x);
}

void hoamap_mouseleave(t_hoa_map *x, t_object *patcherview, t_pt pt, long modifiers)
{
    x->f_index_of_selected_source = -1;
    x->f_index_of_selected_group = -1;
	jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
    jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
    jbox_redraw((t_jbox *)x);
}

long hoamap_key(t_hoa_map *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
    int filter = 0;

#ifdef _WINDOWS
	if (keycode == 97 && modifiers == 5 && textcharacter == 1) // Control + a
#else
	if (keycode == 97 && modifiers == 1 && textcharacter == 0) //cmd+a
#endif
    {
		if (!x->f_showgroups)
			return filter;
		
		int indexOfNewGroup = -1;
        for(int i = 0; indexOfNewGroup == -1; i++)
        {
            if (x->f_source_manager->groupGetExistence(i) == 0)
            {
                indexOfNewGroup = i;
            }
        }
        
        for(int i = 0; i <= x->f_source_manager->getMaximumIndexOfSource(); i++)
        {
            if(x->f_source_manager->sourceGetExistence(i) && indexOfNewGroup >= 0)
            {
                x->f_source_manager->groupSetSource(indexOfNewGroup, i);
                x->f_index_of_selected_group = indexOfNewGroup;
            }
        }
        
		object_notify(x, hoa_sym_modified, NULL);
        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_sources_layer);
        jbox_invalidate_layer((t_object *)x, NULL, hoa_sym_groups_layer);
        jbox_redraw((t_jbox *)x);
		hoamap_send_binded_map_update(x, BMAP_REDRAW | BMAP_OUTPUT | BMAP_NOTIFY);
        
        filter = 1;
	}
	return filter;
}
    
void hoamap_color_picker(t_hoa_map *x)
{
    if(x->f_patcher)
        object_free(x->f_patcher);
    if(x->f_colorpicker )
        object_free(x->f_colorpicker);
    
    x->f_patcher = NULL;
    x->f_colorpicker = NULL;
    
    t_dictionary *dico = dictionary_new();
    char parsebuf[256];
    t_atom a;
    long ac = 0;
    t_atom *av = NULL;
    
    sprintf(parsebuf,"@defrect 0 0 128 32 @openrect 0 0 128 32 @title color @enablehscroll 0 @enablevscroll 0 @presentation 0 @toolbarid \"\"");
    atom_setparse(&ac,&av,parsebuf);
    attr_args_dictionary(dico,ac,av);
    atom_setobj(&a,dico);
    sysmem_freeptr(av);
    x->f_patcher = (t_object *)object_new_typed(CLASS_NOBOX,hoa_sym_jpatcher,1, &a);
    freeobject((t_object *)dico);
    
    x->f_colorpicker = newobject_sprintf(x->f_patcher, "@maxclass colorpicker @patching_rect 0 0 128 32");
    object_attach_byptr_register(x, x->f_patcher, CLASS_NOBOX);
    object_attach_byptr_register(x, x->f_colorpicker, CLASS_BOX);
    
    object_method(x->f_colorpicker, hoa_sym_bang);
}

void hoamap_text_field(t_hoa_map *x)
{
    if(x->f_textfield)
        object_free(x->f_textfield);
    if(x->f_patcher)
        object_free(x->f_patcher);
    
    x->f_patcher = NULL;
    x->f_textfield = NULL;
    
    t_dictionary *dico = dictionary_new();
    char parsebuf[256];
    t_atom a;
    long ac = 0;
    t_atom *av = NULL;
    
    int posX, posY;
    jmouse_getposition_global(&posX, &posY);
    
    sprintf(parsebuf,"@defrect %i %i 256 32 @openrect 0 0 256 32 @title Description @enablehscroll 0 @enablevscroll 0 @presentation 0 @toolbarvisible 0 @retain 1\"\"", posX, posY);
    atom_setparse(&ac,&av,parsebuf);
    attr_args_dictionary(dico,ac,av);
    atom_setobj(&a,dico);
    sysmem_freeptr(av);
    x->f_patcher = (t_object *)object_new_typed(CLASS_NOBOX,hoa_sym_jpatcher,1, &a);
    freeobject((t_object *)dico);
    
    x->f_textfield = newobject_sprintf(x->f_patcher, "@maxclass hoa.textfield @patching_rect 0 0 256 32");
    
    object_attach_byptr_register(x, x->f_patcher, CLASS_NOBOX);
    object_attach_byptr_register(x, x->f_textfield, CLASS_NOBOX);
    
    object_method(x->f_patcher,hoa_sym_noedit, 1);
    object_method(x->f_patcher,hoa_sym_vis);
}

/**************************************************************************************************************************/
/**************************************************************************************************************************/
/**************************************************************************************************************************/
/**************************************************************************************************************************/
/**************************************************************************************************************************/

void hoa_textfield_init(void)
{
	t_class *c;	
	c = class_new("hoa.textfield",
				  (method)textfield_new,
				  (method)textfield_free,
				  sizeof(t_textfield),
				  (method)NULL,
				  A_GIMME,
				  0L);
	
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
	jbox_initclass(c, JBOX_TEXTFIELD | JBOX_FONTATTR | JBOX_FIXWIDTH);
	
	class_addmethod(c, (method)textfield_paint,         "paint",		A_CANT, 0);
	class_addmethod(c, (method)textfield_key,			"key",			A_CANT, 0);
	class_addmethod(c, (method)textfield_keyfilter,     "keyfilter",	A_CANT, 0);
	class_addmethod(c, (method)textfield_enter,         "enter",		A_CANT, 0);
	class_addmethod(c, (method)textfield_select,		"select",		0);
	class_addmethod(c, (method)textfield_notify,		"notify",		A_CANT, 0);
    
	CLASS_ATTR_DEFAULT(c, "rect", 0, "0. 0. 100. 20.");
	
	class_register(CLASS_BOX, c);
	s_textfield_class = c;
}


t_textfield* textfield_new(t_symbol *name, short argc, t_atom *argv)
{
	t_textfield* x;
	t_dictionary *d = NULL;
	
	if (!(d = object_dictionaryarg(argc,argv)))
		return NULL;
 	
	x = (t_textfield*)object_alloc(s_textfield_class);
	if (x) {
		long		flags;
		t_object	*textfield;
		
		flags = 0
        | JBOX_DRAWFIRSTIN
        | JBOX_NODRAWBOX
        | JBOX_DRAWINLAST
        | JBOX_TRANSPARENT
        | JBOX_GROWBOTH
        | JBOX_TEXTFIELD
		;
		
		jbox_new(&x->j_box, flags, argc, argv);
		x->j_box.b_firstin = (t_object*) x;
        x->j_patcher = NULL;
        x->j_patcherview = NULL;
		t_jrgba textcolor = {0., 0., 0., 1.};
		textfield = jbox_get_textfield((t_object*) x);
		if (textfield)
        {
			textfield_set_editonclick(textfield, 1);
			textfield_set_textmargins(textfield, 3, 3, 3, 3);
			textfield_set_textcolor(textfield, &textcolor);
		}
        
		attr_dictionary_process(x, d);
		jbox_ready(&x->j_box);
	}
	return x;
}

void textfield_free(t_textfield *x)
{
	jbox_free(&x->j_box);
}

void textfield_select(t_textfield *x)
{
	defer(x, (method)textfield_doselect, 0, 0, 0);
}

void textfield_doselect(t_textfield *x)
{
	t_object *p = NULL;
	object_obex_lookup(x, hoa_sym_pound_P, &p);
	if (p) {
		t_atom rv;
		long ac = 1;
		t_atom av[1];
		atom_setobj(av, x);
		object_method_typed(p, gensym("selectbox"), ac, av, &rv);
	}
}


long textfield_key(t_textfield *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
	char buff[256];
	buff[0] = textcharacter;
	buff[1] = 0;
	object_method(patcherview, hoa_sym_insertboxtext, x, buff);
    long size	= 0;
	char *text	= NULL;
	t_object *textfield = jbox_get_textfield((t_object *)x);
	object_method(textfield, hoa_sym_gettextptr, &text, &size);
    object_notify(x, hoa_sym_text, text);
	jbox_redraw((t_jbox *)x);
	return 1;
}

long textfield_keyfilter(t_textfield *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter)
{
	t_atom arv;
	long rv = 1;
	long k = *keycode;
	
	if (k == JKEY_TAB || k == JKEY_ENTER || k == JKEY_RETURN || k == JKEY_ESC) {
		object_method_typed(patcherview, hoa_sym_endeditbox, 0, NULL, &arv);
		rv = 0;
	}
    long size	= 0;
    char *text	= NULL;
    t_object *textfield = jbox_get_textfield((t_object *)x);
    object_method(textfield, hoa_sym_gettextptr, &text, &size);
    object_notify(x, hoa_sym_text, text);
    if (!rv) object_notify(x, hoa_sym_endeditbox, NULL);
	return rv;
}

void textfield_enter(t_textfield *x)
{
	long size	= 0;
	char *text	= NULL;
	t_object *textfield = jbox_get_textfield((t_object *)x);
	object_method(textfield, hoa_sym_gettextptr, &text, &size);
    object_notify(x, hoa_sym_text, text);
}

void textfield_paint(t_textfield *x, t_object *view)
{
	t_rect rect;
	t_jgraphics *g = (t_jgraphics*) patcherview_get_jgraphics(view);
    jbox_get_rect_for_view((t_object*) x, view, &rect);
    if(x->j_patcher == NULL)
    {
        object_obex_lookup(x, hoa_sym_pound_P, &x->j_patcher);
        object_attach_byptr_register(x, x->j_patcher, CLASS_NOBOX);
    }
    if(x->j_patcherview == NULL)
    {
        x->j_patcherview = object_attr_getobj(x->j_patcher, hoa_sym_firstview);
        object_attach_byptr_register(x, x->j_patcherview, CLASS_NOBOX);
    }
	
    t_jrgba white = {1., 1., 1., 1.};
	jgraphics_rectangle_rounded(g, 0., 0., rect.width, rect.height, 2., 2.);
	jgraphics_set_source_jrgba(g, &white);
	jgraphics_fill(g);
    
    
    t_jrgba grey = {0., 0., 0., 1.};
    jgraphics_rectangle(g, 0., 0., rect.width, rect.height);
	jgraphics_set_source_jrgba(g, &grey);
    jgraphics_set_line_width(g, 1.);
	jgraphics_stroke(g);
	
}

t_max_err textfield_notify(t_textfield *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    if (sender == x->j_patcher)
    {
		if (msg == hoa_sym_free)
        {
			x->j_patcher = NULL;
            x->j_patcherview = NULL;
        }
    }
    if(sender == x->j_patcherview)
    {
        if (msg == hoa_sym_attr_modified)
        {
            t_symbol* attrname = (t_symbol *)object_method(data, hoa_sym_getname);
            if (attrname == hoa_sym_rect)
            {
                t_atom *av = NULL;
                long    ac = 0;
                object_attr_getvalueof(x->j_patcherview, hoa_sym_rect, &ac, &av);
                if (ac && av)
                {
                    atom_setlong(av, 0);
                    atom_setlong(av+1, 0);
                    object_method(x, hoa_sym_rect, ac, av);
                    freebytes(av, sizeof(t_atom) * ac);
                }
                jbox_redraw((t_jbox *)x);
            }
        }
    }
    return MAX_ERR_NONE;
}
