; $Id: bit_close_ei.asm,v 1.5 2016-06-16 20:23:52 dom Exp $
;
; VZ 200 - 1 bit sound functions
;
; Close sound and restore interrupts
;
; Stefano Bodrato - 31/03/2008
;

    SECTION code_clib
    PUBLIC  bit_close_ei
    PUBLIC  _bit_close_ei
    PUBLIC  bit_close
    PUBLIC  _bit_close
    EXTERN  __bit_irqstatus
    EXTERN  saved_bank1

    INCLUDE "target/laser500/def/laser500.def"

bit_close_ei:
_bit_close_ei:
    call    bit_close

    push    hl
    ld      hl, (__bit_irqstatus)
    ex      (sp), hl
    pop     af
    ret     po
    ei
    ret

bit_close:
_bit_close:
    ld      a, (saved_bank1)
    ld      (SYSVAR_bank1), a
    out     ($41), a
    ret
