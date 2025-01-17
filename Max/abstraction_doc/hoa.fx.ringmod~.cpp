
// ------------------------------------------ hoa.fx.ringmod~ ------------------------------------------ //

/**
 @file      hoa.fx.ringmod~.cpp
 @name      hoa.fx.ringmod~
 @realname  hoa.fx.ringmod~
 @type      abstraction
 @module    hoa
 @author    Julien Colafrancesco, Pierre Guillot, Eliott Paris.
 
 @digest
 A ring modulation sound field diffuser.
 
 @description
 <o>hoa.fx.ringmod~</o> creates a diffuse sound field by modulating the amplitude of circular, spherical or plane waves signals depending on their number.
 This patcher requires to be loaded by a <o>hoa.process~</o> object to work.
 
 @discussion
 <o>hoa.fx.ringmod~</o> creates a diffuse sound field by modulating the amplitude of circular, spherical or plane waves signals depending on their number.
 This patcher requires to be loaded by a <o>hoa.process~</o> object to work.
 
 @keywords ambisonics, hoa objects, audio, MSP, abstraction
 
 @seealso hoa.process~, hoa.fx.decorrelation~, hoa.fx.delay~, hoa.fx.gain~, hoa.fx.grain~, hoa.fx.mirror~, hoa.fx.mixer~
 */

CLASS_ATTR_FLOAT            (c, "freq", 0, 0, 0);
CLASS_ATTR_LABEL            (c, "freq", 0, "Frequency (Hz)");
CLASS_ATTR_DEFAULT          (c, "freq", 0, "0");
// @description Set the ring modulation frequencies with a float or a list of float.

CLASS_ATTR_FLOAT            (c, "factor", 0, 0, 0);
CLASS_ATTR_LABEL            (c, "factor", 0, "Frequency scaling factor");
CLASS_ATTR_DEFAULT          (c, "factor", 0, "0");
// @description Set the frequency scaling factor (between 0 and 1), in "harmonics" mode, it will scale the frequencies of each instance depending on their index. This factor is useless in "planewaves" mode.