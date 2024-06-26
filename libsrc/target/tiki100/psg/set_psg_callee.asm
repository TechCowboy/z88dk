;
;	TIKI-100 specific routines
;	by Stefano Bodrato, Fall 2015
;
;	int set_psg(int reg, int val);
;
;	Play a sound by PSG
;
;
;	$Id: set_psg_callee.asm,v 1.2 2016-06-10 21:13:58 dom Exp $
;

    SECTION code_clib
    PUBLIC  set_psg_callee
    PUBLIC  _set_psg_callee

    PUBLIC  asm_set_psg


set_psg_callee:
_set_psg_callee:
    pop     hl
    pop     de
    ex      (sp), hl
asm_set_psg:
    ld      bc, $16
    out     (c), l
    ld      c, $17
    out     (c), e
    ret
