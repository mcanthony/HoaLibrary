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
 *  - Redistributions may not be sold, nor may they be used in a commercial product or activity.
 *  - Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  - Neither the name of the CICM nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MaxAmbisonic.h"

MaxAmbisonic::MaxAmbisonic(t_object* aParentObject, long argc, t_atom* argv)
{
	m_parent = aParentObject;
    if(atom_gettype(argv) == A_LONG || atom_gettype(argv) == A_FLOAT)
    {
        m_order = (long)atom_getfloat(argv);
    }
    m_number_of_box_text_items = argc;
    m_box_text_items = new t_atom[m_number_of_box_text_items];
    for(int i = 0; i < m_number_of_box_text_items; i++)
        m_box_text_items[i] = argv[i];
    
    OBJ_ATTR_LONG           ((t_object *)m_parent,"order", 0, m_order);
    OBJ_ATTR_ATTR_FORMAT    ((t_object *)m_parent,"order","label",    USESYM(symbol),0,"s",gensym_tr("Ambisonic Order"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"order","category", USESYM(symbol),0,str_tr("HoaLibrary"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"order","order",    USESYM(long),  0,"1");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"order","save",     USESYM(long),  1,"1");
    
    m_color_positive[0] = 0.;
    m_color_positive[1] = 0.;
    m_color_positive[2] = 1.;
    m_color_positive[3] = 1.;
    OBJ_ATTR_DOUBLE_ARRAY   ((t_object *)m_parent,"poscolor", 0, 4, m_color_positive);
    OBJ_ATTR_ATTR_FORMAT    ((t_object *)m_parent,"poscolor","label",    USESYM(symbol), 0,"s",gensym_tr("Positive Color"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"poscolor","style",    USESYM(symbol), 0,"rgba");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"poscolor","category", USESYM(symbol), 0,str_tr("HoaLibrary"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"poscolor","order",    USESYM(long),   0,"2");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"poscolor","save",     USESYM(long),   0,"1");
    
    m_color_negative[0] = 1.;
    m_color_negative[1] = 0.;
    m_color_negative[2] = 0.;
    m_color_negative[3] = 1.;
    OBJ_ATTR_DOUBLE_ARRAY   ((t_object *)m_parent,"negcolor", 0, 4, m_color_negative);
    OBJ_ATTR_ATTR_FORMAT    ((t_object *)m_parent,"negcolor","label",    USESYM(symbol),0,"s",gensym_tr("Negative Color"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"negcolor","style",    USESYM(symbol),0,"rgba");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"negcolor","category", USESYM(symbol),0,str_tr("HoaLibrary"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"negcolor","order",    USESYM(long),  0,"3");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"negcolor","save",     USESYM(long),  1,"1");
    
    m_auto_connect = 1;
    OBJ_ATTR_LONG           ((t_object *)m_parent,"autoconnect", 0, m_auto_connect);
    OBJ_ATTR_ATTR_FORMAT    ((t_object *)m_parent,"autoconnect","label",    USESYM(symbol),0,"s",gensym_tr("Auto Connection"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"autoconnect","style",    USESYM(symbol),0,"onoff");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"autoconnect","category", USESYM(symbol),0,str_tr("HoaLibrary"));
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"autoconnect","order",    USESYM(long),  0,"4");
    OBJ_ATTR_ATTR_PARSE     ((t_object *)m_parent,"autoconnect","save",     USESYM(long),  1,"1");
    
    m_line_selected = 0;
    m_number_of_object_to_keep = 0;
    m_object_to_keep = NULL;
}

long MaxAmbisonic::getOrder()
{
    return m_order;
}

void MaxAmbisonic::setOrder(long anOrder)
{
    m_order = anOrder;
    atom_setlong(m_box_text_items, m_order);
}

void MaxAmbisonic::attach_to_notification()
{
    object_attach_byptr_register(m_parent, m_parent, CLASS_BOX);
    object_obex_lookup(m_parent, gensym("#P"), (t_object **)&m_patcher);
    object_obex_lookup(m_parent, gensym("#B"), (t_object **)&m_box);
    m_patcherview = jpatcher_get_firstview(m_patcher);
    object_attach_byptr_register(m_parent, m_patcherview, CLASS_NOBOX);
}

void MaxAmbisonic::connect_outlets()
{
    for(t_object* line = jpatcher_get_firstline(m_patcher); line; line = jpatchline_get_nextline(line))
    {
        if (jpatchline_get_box1(line) == m_box && jpatchline_get_outletnum(line) == 0)
        {
            t_object* object_inlet = jpatchline_get_box2(line);
            if(object_attr_getlong(object_inlet, gensym("order")) != 0 && jpatchline_get_inletnum(line) == 0)
            {
                int number_of_harmonics = m_order * 2 + 1;
                if(object_attr_getlong(object_inlet, gensym("order")) * 2 + 1 < number_of_harmonics)
                    number_of_harmonics = object_attr_getlong(object_inlet, gensym("order")) * 2 + 1;
                
                CicmMax::connect_lines(m_patcher, m_box, object_inlet, number_of_harmonics);
            }
        }
    }
}

void MaxAmbisonic::connect_outlet_with_line(t_object* line)
{
    t_object *inlet_object;    
    t_max_err err = object_obex_lookup(jpatchline_get_box2(line), gensym("#B"), (t_object **)&inlet_object);
    if (err != MAX_ERR_NONE)
        return;
    
    int number_of_harmonics = m_order * 2 + 1;
    
    CicmMax::connect_lines(m_patcher, m_box, inlet_object, number_of_harmonics);
    m_line_selected = 0;
    m_object_to_connect = NULL;
}

void MaxAmbisonic::color_inlets()
{
    t_jrgba   color;
    for (t_object* line = jpatcher_get_firstline(m_patcher); line; line = jpatchline_get_nextline(line))
    {
        if(jpatchline_get_box2(line) == m_box && jpatchline_get_inletnum(line) < m_order * 2 + 1)
        {
            if(jpatchline_get_inletnum(line) % 2 == 0)
                object_attr_getjrgba(m_parent, gensym("poscolor"), &color);
            else
                object_attr_getjrgba(m_parent, gensym("negcolor"), &color);
            
            jpatchline_set_color(line, &color);
        }
    }
}

void MaxAmbisonic::color_outlets()
{
    t_jrgba   color;
    for (t_object* line = jpatcher_get_firstline(m_patcher); line; line = jpatchline_get_nextline(line))
    {
        if(jpatchline_get_box1(line) == m_box && jpatchline_get_outletnum(line) < m_order * 2 + 1)
        {
            if(jpatchline_get_inletnum(line) % 2 == 0)
                object_attr_getjrgba(m_parent, gensym("poscolor"), &color);
            else
                object_attr_getjrgba(m_parent, gensym("negcolor"), &color);
            
            jpatchline_set_color(line, &color);
        }
    }
}

void MaxAmbisonic::rename_box()
{
    char        name[256];
    char        tempory[256];

    strcpy(name, jbox_get_maxclass(m_box)->s_name);
    for(int i = 0; i < m_number_of_box_text_items; i++)
    {
        if(atom_gettype(m_box_text_items+i) == A_SYM)
            sprintf(tempory, " %s", atom_getsym(m_box_text_items+i)->s_name);
        else if(atom_gettype(m_box_text_items+i) == A_LONG)
            sprintf(tempory, " %ld", (long)atom_getlong(m_box_text_items+i));
        else if(atom_gettype(m_box_text_items+i) == A_FLOAT)
            sprintf(tempory, " %f", atom_getfloat(m_box_text_items+i));
        
        strcat(name, tempory);
    }
    object_method(jbox_get_textfield((t_object *)m_box), gensym("settext"), name);
}

t_max_err MaxAmbisonic::notify(t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    if(msg == gensym("connect"))
    {
        connect_outlets();
        color_outlets();
    }
    else if(msg == gensym("poscolor") || msg == gensym("negcolor"))
    {
        object_attr_setjrgba(m_parent, msg, (t_jrgba *)data);
    }
    else if(msg == gensym("autoconnect") || msg == gensym("order"))
    {
        object_attr_setlong(m_parent, msg, (long)data);
    }
    if(sender == m_patcherview)
    {
        if(msg == gensym("attr_modified"))
        {
            t_symbol* attr_name = (t_symbol *)object_method(data, gensym("getname"));
            
            if(attr_name == gensym("dirty") && jkeyboard_getcurrentmodifiers() == 8)
            {
                t_atom obj[1];
                atom_setobj(obj, m_object_to_connect);
                defer_low(m_parent, object_getmethod(m_parent, gensym("connect")), gensym("connect"), 1, obj);
            }
            else if(attr_name == gensym("selectedlines"))
            {
                long ac = 0;
                t_atom *av = NULL;
                
                object_attr_getvalueof(sender, attr_name, &ac, &av);
                
                if(ac && atom_gettype(av) == A_OBJ)
                {
                    t_object* this_object;
                    t_object* line = (t_object *)atom_getobj(av);
                    
                    object_obex_lookup(m_parent, gensym("#B"), (t_object **)&this_object);
                    
                    if(ac && jpatchline_get_box1(line) == this_object)
                    {
                        m_line_selected = 1;
                        m_object_to_connect = line;
                        if(jkeyboard_getcurrentmodifiers() == 24)
                        {
                            t_atom obj[1];
                            atom_setobj(obj, m_object_to_connect);
                            defer_low(m_parent, object_getmethod(m_parent, gensym("connect")), gensym("connect"), 1, obj);
                        }
                    }
                }
            }
            else
            {
                m_line_selected = 0;
                m_object_to_connect = NULL;
            }            
        }
    }
    return MAX_ERR_NONE;
}

MaxAmbisonic::~MaxAmbisonic()
{
	free(m_box_text_items);
    object_detach_byptr(m_parent, m_patcherview);
}
