/*
 *      Small C+ Compiler -
 *
 *      Main() part
 *
 */

#include "../config.h"
#include "ccdefs.h"

#if defined(__MSDOS__) && defined(__TURBOC__)
extern unsigned _stklen = 8192U; /* Default stack size 4096 bytes is too small. */
#endif

static char   *c_output_extension = "asm";
static char   *c_output_file = NULL;
static char    c_debug_adb_file = 0;
static char    c_debug_adb_defc = 0;
       char    c_debug_entry_points = 0;
int     c_banked_style = BANKED_STYLE_REGULAR;


static int      gargc; /* global copies of command line args */
static char   **gargv;
static SYMBOL  *savecurr;    /* copy of currfn for #include */
static int      saveline;    /* copy of lineno  "    " */
static int      saveinfn;    /* copy of infunc  "    " */
static int      filenum; /* next argument to be used */

UT_string       *debug_utstr;
UT_string       *debug2_utstr;

static Type *type_double4 = &(Type){ KIND_DOUBLE, 4, 0, .len=1 }; 
static Type *type_double8 = &(Type){ KIND_DOUBLE, 8, 0, .len=1 }; 

char Filenorig[FILENAME_LEN + 1];


int c_notaltreg; /* No alternate registers */
int c_standard_escapecodes = 0; /* \n = 10, \r = 13 */
int c_disable_builtins = 0;
int c_cline_directive = 0;
int c_cpu = CPU_Z80;
int c_params_offset = 2;
int c_old_diagnostic_fmt = 0;
char *c_zcc_opt = "zcc_opt.def";

/* Settings for genmath + math48 */
int c_fp_mantissa_bytes = 5;
int c_fp_exponent_bias = 128;
int c_fp_fudge_offset = 0;    // Fudge offset for z88math - it starts pickup from FA+1
int c_fp_size = 6;

enum maths_mode c_maths_mode = MATHS_Z80;

uint32_t c_speed_optimisation = OPT_RSHIFT32|OPT_LSHIFT32;

char *c_rodata_section = "rodata_compiler";
char *c_data_section = "data_compiler";
char *c_bss_section = "bss_compiler";
char *c_code_section = "code_compiler";
char *c_init_section = "code_crt_init";
char *c_home_section = "code_home";

#include "option.h"



static void dumpsymdebug(void);
static void dumpdebug(void);
static void dumpfns(void);
static void dumpvars(void);
static void parse(void);
static void errsummary(void);
static char *nextarg(int n, char *s, int size);
static void setup_sym(void);
static void info(void);
static void openout(void);

static void SetWarning(option *arg, char* val);
static void SetDefine(option *arg, char *val);
static void SetUndefine(option *arg, char *val);
static void DispInfo(option *arg, char *val);
static void opt_code_speed(option *arg, char* val);
static void atexit_deallocate(void);


static option  sccz80_opts[] = {
    { 'v', "verbose", OPT_BOOL, "Be verbose", &c_verbose, NULL, 0 },
    { 'h', "help", OPT_FUNCTION|OPT_BOOL, "Show this help page", NULL, DispInfo, 0 },
    { 'o', "output", OPT_STRING, "Set the output filename", &c_output_file, NULL, 0 },
    { 0, "", OPT_HEADER, "CPU Targetting:", NULL, NULL, 0 },
    { 0, "m8080", OPT_ASSIGN|OPT_INT, "Generate output for the i8080", &c_cpu, NULL, CPU_8080 },
    { 0, "m8085", OPT_ASSIGN|OPT_INT, "Generate output for the i8085", &c_cpu, NULL, CPU_8085 },
    { 0, "mez80_z80", OPT_ASSIGN|OPT_INT, "Generate output for the ez80 in z80 mode", &c_cpu, NULL, CPU_EZ80_Z80 },
    { 0, "mz80", OPT_ASSIGN|OPT_INT, "Generate output for the z80", &c_cpu, NULL, CPU_Z80 },
    { 0, "mz80n", OPT_ASSIGN|OPT_INT, "Generate output for the z80n", &c_cpu, NULL, CPU_Z80N },
    { 0, "mz180", OPT_ASSIGN|OPT_INT, "Generate output for the z180", &c_cpu, NULL, CPU_Z180 },
    { 0, "mr2ka", OPT_ASSIGN|OPT_INT, "Generate output for the Rabbit 2000A", &c_cpu, NULL, CPU_R2KA },
    { 0, "mr3k", OPT_ASSIGN|OPT_INT, "Generate output for the Rabbit 3000", &c_cpu, NULL, CPU_R3K },
    { 0, "mr4k", OPT_ASSIGN|OPT_INT, "Generate output for the Rabbit 4000", &c_cpu, NULL, CPU_R4K },
    { 0, "mr6k", OPT_ASSIGN|OPT_INT, "Generate output for the Rabbit 4000", &c_cpu, NULL, CPU_R4K|CPU_R6K },
    { 0, "mgbz80", OPT_ASSIGN|OPT_INT, "Generate output for the Gameboy CPU", &c_cpu, NULL, CPU_GBZ80 },
    { 0, "mkc160", OPT_ASSIGN|OPT_INT, "Generate output for the KC160", &c_cpu, NULL, CPU_KC160 },
    { 0, "", OPT_HEADER, "Code generation options", NULL, NULL, 0 },
    { 0, "unsigned", OPT_BOOL, "Make all types unsigned", &c_default_unsigned, NULL, 0 },
    { 0, "disable-builtins", OPT_BOOL|OPT_DOUBLE_DASH, "Disable builtin functions",&c_disable_builtins, NULL, 0},
    { 0, "params-offset", OPT_INT, "=<num> Base offset for the function parameters (default: 2)",&c_params_offset,NULL, 0},
    { 0, "doublestr", OPT_BOOL, "Store FP constants as strings", &c_double_strings, NULL, 0 },
    { 0, "math-z88", OPT_ASSIGN|OPT_INT, "(deprecated) Make FP constants match z88", &c_maths_mode, NULL, MATHS_Z88 },
    { 0, "banked-style=regular", OPT_ASSIGN|OPT_INT, "Use regular banked calling style", &c_banked_style, NULL, BANKED_STYLE_REGULAR },
    { 0, "banked-style=ti", OPT_ASSIGN|OPT_INT, "Use ticalc banked calling style", &c_banked_style, NULL, BANKED_STYLE_TICALC },
    
    { 0, "fp-exponent-bias", OPT_INT, "=<num> FP exponent bias (default: 128)", &c_fp_exponent_bias, NULL, 0 },
    { 0, "fp-mantissa-size", OPT_INT, "=<num> FP mantissa size (default: 5 bytes)", &c_fp_mantissa_bytes, NULL, 0 },
    { 0, "fp-mode=z80", OPT_ASSIGN|OPT_INT, "Use 48 bit doubles", &c_maths_mode, NULL, MATHS_Z80 },
    { 0, "fp-mode=ieee", OPT_ASSIGN|OPT_INT, "Use 32 bit IEEE doubles", &c_maths_mode, NULL, MATHS_IEEE },
    { 0, "fp-mode=mbf32", OPT_ASSIGN|OPT_INT, "Use 32 bit Microsoft Binary format", &c_maths_mode, NULL, MATHS_MBFS },
    { 0, "fp-mode=mbf40", OPT_ASSIGN|OPT_INT, "Use 40 bit Microsoft binary format", &c_maths_mode, NULL, MATHS_MBF40 },
    { 0, "fp-mode=mbf64", OPT_ASSIGN|OPT_INT, "Use 64 bit Microsoft binary format", &c_maths_mode, NULL, MATHS_MBF64 },
    { 0, "fp-mode=z88", OPT_ASSIGN|OPT_INT, "Use 40 bit z88 doubles", &c_maths_mode, NULL, MATHS_Z88 },
    { 0, "fp-mode=am9511", OPT_ASSIGN|OPT_INT, "Use 32 bit AM9511 doubles", &c_maths_mode, NULL, MATHS_AM9511 },
    
    { 0, "noaltreg", OPT_BOOL, "Try not to use the alternative register set", &c_notaltreg, NULL, 0 },
    { 0, "standard-escape-chars", OPT_BOOL, "Use standard mappings for \\r and \\n", &c_standard_escapecodes, NULL, 0},
    { 0, "set-r2l-by-default", OPT_BOOL, "Use r->l calling convention by default", &c_use_r2l_calling_convention, NULL, 0 },
    { 0, "constseg", OPT_STRING|OPT_DOUBLE_DASH, "=<name> Set the const section name", &c_rodata_section, NULL, 0 },
    { 0, "codeseg", OPT_STRING|OPT_DOUBLE_DASH, "=<name> Set the code section name", &c_code_section, NULL, 0 },
    { 0, "bssseg", OPT_STRING|OPT_DOUBLE_DASH, "=<name> Set the bss section name", &c_bss_section, NULL, 0 },
    { 0, "dataseg", OPT_STRING|OPT_DOUBLE_DASH, "=<name> Set the data section name", &c_data_section, NULL, 0 },
    { 0, "initseg", OPT_STRING|OPT_DOUBLE_DASH, "=<name> Set the initialisation section name", &c_init_section, NULL, 0 },
    { 0, "gcline", OPT_BOOL, "Generate C_LINE directives", &c_cline_directive, NULL, 0 },
    { 0, "opt-code-speed", OPT_FUNCTION|OPT_STRING|OPT_DOUBLE_DASH, "Optimise for speed not size", NULL, opt_code_speed, 0},
    { 0, "", OPT_HEADER, "Framepointer configuration (for debugging):", NULL, NULL, 0 },
    { 0, "frameix", OPT_ASSIGN|OPT_INT, "Use ix as the frame pointer", &c_framepointer_is_ix, NULL, 1},
    { 0, "frameiy", OPT_ASSIGN|OPT_INT, "Use iy as the frame pointer", &c_framepointer_is_ix, NULL, 0},
    { 0, "zcc-opt", OPT_STRING, "Location for zcc_opt.def", &c_zcc_opt, NULL, (intptr_t)(void *)"zcc_opt.def"},

    { 0, "", OPT_HEADER, "Error/warning handling:", NULL, NULL, 0 },
    { 0, "stop-on-error", OPT_BOOL, "Stop on any error", &c_errstop, NULL, 0 },
    { 0, "old-diagnostic-format", OPT_BOOL|OPT_DOUBLE_DASH, "Use old style diagnostic messages", &c_old_diagnostic_fmt, NULL, 0 },
#if 0
    { 0, "Wnone", OPT_FUNCTION|OPT_BOOL, "Disable all warnings", NULL, SetNoWarn, 0 },
    { 0, "Wall", OPT_FUNCTION|OPT_BOOL, "Enable all warnings", NULL, SetAllWarn, 0 },
    { 0, "Wn", OPT_FUNCTION, "<num> Disable a warning", NULL, UnSetWarning, 0},
#endif
    { 0, "W", OPT_FUNCTION, "<type> Enable a class of warnings", NULL,  SetWarning, 0 },
    { 0, "", OPT_HEADER, "Debugging options", NULL, NULL, 0 },
    { 0, "debug-sect", OPT_BOOL, "Create adb/cdb debug section", &c_debug_adb_file, NULL, 0 },
    { 0, "debug-defc", OPT_BOOL, "Create adb/cdb debug defc", &c_debug_adb_defc, NULL, 0 },
    { 0, "cc", OPT_BOOL, "Intersperse the assembler output with the source C code", &c_intermix_ccode, NULL, 0 },
    { 0, "intlog", OPT_INT, "=<val> Enable some extra logging", &debuglevel, NULL, 0 },
    { 0, "ext", OPT_STRING, "=<ext> Set the file extension for the generated output", &c_output_extension, NULL, 0 },
    { 0, "D", OPT_FUNCTION, "Define a preprocessor directive", NULL, SetDefine, 0 },
    { 0, "U", OPT_FUNCTION, "Undefine a preprocessor directive", NULL, SetUndefine, 0 },
    { 0, "", OPT_HEADER, "Assignments can either be = or as next argument", NULL, NULL, 0},
    { 0 }
};




/*
 *
 *      Code...
 *
 */

/*
 *      Compiler begins execution here
 */
int main(int argc, char** argv)
{
    gargc = argc;
    gargv = argv;

    /* allocate space for arrays */
    litq = MALLOC(FNLITQ); /* literals, these 2 dumped end */
    dubq = MALLOC(FNLITQ); /* Doubles */
    tempq = MALLOC(LITABSZ); /* Temp strings... */
    glbq = MALLOC(LITABSZ); /* Used for glb lits, dumped now */
    symtab = NULL;
    loctab = MALLOC(NUMLOC * sizeof(SYMBOL));
    wqueue = MALLOC(NUMWHILE * sizeof(WHILE_TAB));
    gotoq = MALLOC(NUMGOTO * sizeof(GOTO_TAB));

    swnext = MALLOC(NUMCASE * sizeof(SW_TAB));
    swend = swnext + (NUMCASE - 1);

    stage = MALLOC(STAGESIZE);
    stagelast = stage + STAGELIMIT;


    glbcnt = 0; /* clear global symbols */
    locptr = STARTLOC; /* clear local symbols */
    wqptr = wqueue; /* clear while queue */
    gltptr = 0; /* clear literal pools */
    *litq = 0; /* First entry in literal queue is zero */
    litptr = 1; /* So miniprintf search works */

    Zsp = /* stack ptr (relative) */
        errcnt = /* no errors */
        c_errstop = /* keep going after an error */
        eof = /* not eof yet */
        ncmp = /* no open compound states */
        lastst = /* not first file to asm */
        lineno = /* no lines read from file */
        infunc = /* not in function now */
        0; /*  ...all set to zero.... */

    stagenext = NULL; /* direct output mode */

    input = /* no input file */
        inpt2 = /* or include file */
        saveout = /* no diverted output */
        output = NULL; /* no open units */

    currfn = NULL; /* no function yet */
    macptr = cmode = 1; /* clear macro pool and enable preprocessing */
    ncomp = need_floatpack = 0;
    c_default_unsigned = NO;
    nxtlab = 0;/* start numbers at lowest possible */
    c_intermix_ccode = 0; /* don't include the C text as comments */
    c_errstop =0;  /* don't stop after errors */
    c_verbose = 0;
    c_double_strings = 0;
    c_notaltreg = NO;
    debuglevel = NO;
    c_framepointer_is_ix = -1;
    c_use_r2l_calling_convention = NO;

    setup_sym(); /* define some symbols */
    /* Parse the command line options */
    atexit(atexit_deallocate); /* To free everything */
    clear();
    filenum = 0;
    gargc = option_parse(sccz80_opts, argc, argv);
    clear();

    if (gargc == 0) {
        info();
        exit(1);
    }

    if ( c_maths_mode == MATHS_IEEE ) {
        c_fp_size = 4;
        type_double = type_double4;
        c_fp_exponent_bias = 126;
        c_fp_mantissa_bytes = 3;
        WriteDefined("CLIB_32BIT_FLOATS", 1);
    } else if ( c_maths_mode == MATHS_MBFS ) {
        c_fp_size = 4;
        type_double = type_double4;
        c_fp_exponent_bias = 128;
        c_fp_mantissa_bytes = 3;
        WriteDefined("CLIB_32BIT_FLOATS", 1);
    } else if ( c_maths_mode == MATHS_MBF40 ) {
        c_fp_exponent_bias = 128;
        c_fp_mantissa_bytes = 4;
    } else if ( c_maths_mode == MATHS_MBF64 ) {
        c_fp_size = 8;
        type_double = type_double8;
        c_fp_exponent_bias = 128;
        c_fp_mantissa_bytes = 7;
        WriteDefined("CLIB_64BIT_FLOATS", 1);
    } else if ( c_maths_mode == MATHS_Z88 ) {
        c_fp_exponent_bias = 127;
        c_fp_mantissa_bytes = 4;
        c_fp_fudge_offset = 1;
    } else if ( c_maths_mode == MATHS_AM9511 ) {
        type_double = type_double4;
        c_fp_exponent_bias = 0;
        c_fp_mantissa_bytes = 3;
        c_fp_size = 4;
    }

    if ( c_debug_adb_file || c_debug_adb_defc ) {
        c_cline_directive = 1;
        if ( c_framepointer_is_ix == -1 ) {
            c_debug_entry_points = 1;
        }
    }


    if ( c_cpu == CPU_8080 ) {
        c_notaltreg = 1;
        WriteDefined("CPU_8080", 1);
    }

    if ( c_cpu == CPU_8085 ) {
        c_notaltreg = 1;
        WriteDefined("CPU_8085", 1);
    }

    if ( c_cpu == CPU_GBZ80 ) {
        c_notaltreg = 1;
        WriteDefined("CPU_GBZ80", 1);
    }

    utstring_new(debug_utstr);
    utstring_new(debug2_utstr);

    litlab = getlabel(); /* Get labels for function lits*/
    openout(); /* get the output file */
    openin(); /* and initial input file */
    gen_file_header(); /* intro code */
    parse(); /* process ALL input */




    gen_switch_section(c_bss_section);
    gen_switch_section(c_code_section);
    outstr("; --- Start of Optimiser additions ---\n\n");
    /* dump literal queues, with label */
    /* litq starts from 1, so literp has to be -1 */
    dumplits(0, YES, litptr - 1, litlab, litq + 1);
    write_constant_queue();
    dumpvars();
    dumpfns();

    if ( c_debug_adb_defc ) {
        dumpsymdebug();
    }
    if ( c_debug_adb_file ) {
        dumpdebug();
    }

    gen_file_footer(); /* follow-up code */
    closeout();
    errsummary(); /* summarize errors */
    if (errcnt)
        exit(1);
    exit(0);
}

/*
 *      Abort compilation
 */
void ccabort(void)
{
    if (inpt2 != NULL)
        endinclude();
    if (input != NULL)
        fclose(input);
    closeout();
    fprintf(stderr, "Compilation aborted\n");
    exit(1);
}

/*
 * Process all input text
 *
 * At this level, only static declarations,
 * defines, includes, and function
 * definitions are legal...
 */
void parse(void)
{
    while (eof == 0) { /* do until no more input */
        if (amatch("extern")) {
            dodeclare(EXTERNAL);
        } else if (amatch("static")) {
            dodeclare(LSTATIC);
        } else if (amatch("typedef")) {
            dodeclare(TYPDEF);
        } else if (dodeclare(STATIK)) {
            ;
        } else if ( amatch("__addressmod")) {
            parse_addressmod();
        } else if (ch() == '#') {
            if (match("#asm")) {
                doasm();
            } else if (match("#include")) {
                doinclude();
            } else if (match("#define")) {
                addmac();
            } else {
                clear();
                blanks();
            }
        } else {
            declare_func_kr();
        }
        blanks(); /* force eof if pending */
    }
}

/*
 *      Report errors for user
 */
void errsummary(void)
{
    /* see if anything left hanging... */
    if (ncmp) {
        errorfmt("Missing closing bracket", 0);
        nl();
    }
    if (c_verbose) {
        printf("Symbol table usage: %d\n", glbcnt);
        // printf("Structures defined: %ld\n", (long)(tagptr - tagtab));
        // printf("Members defined:    %ld\n", (long)(membptr - membtab));
        // printf("There %s %d %s in compilation.\n", (errcnt == 1 ? "was" : "were"), errcnt, (errcnt == 1 ? "error" : "errors"));
    }
}

/*
 * places in s the n-th argument (up to "size"
 * bytes). If successful, returns s. Returns 0
 * if the n-th argument doesn't exist.
 */

char *nextarg(int n, char* s, int size)
{
    char* str;
    char* str2;
    int i;

    if (n < 0 || n >= gargc)
        return NULL;
    i = 0;
    str = str2 = gargv[n];
    while (++i < size && (*s++ = *str++))
        ;
    if (*str2 == '\0')
        return NULL;
    return s;
}

/*
 * make a few preliminary entries in the symbol table
 */

void setup_sym(void)
{
    defmac("Z80");
    defmac("SMALL_C");

    addglb("asm", asm_function("asm"), 0, KIND_LONG, 0, LSTATIC);
    addglb("__asm__", asm_function("__asm__"), 0, KIND_LONG, 0, LSTATIC);
}

void info(void)
{
    fputs(titlec, stderr);
    fputs(Version, stderr);
    fputs("\n(C) 1980-2023 Cain, Van Zandt, Hendrix, Yorston, z88dk\n", stderr);
    fprintf(stderr, "Usage: %s [flags] [file]\n", gargv[0]);
}


static void dumpdebug(void)
{
    const char *debug = utstring_body(debug_utstr);
    const char *end = NULL;

    gen_switch_section("__ADBDEBUG");
    while ( ( end = strchr(debug,'\n')) != NULL ) {
        defmesg();
        outfmt("%.*s\\n\"\n", end - debug, debug);
        debug = end+1;        
    }
}

static void dumpsymdebug(void)
{
    const char *debug = utstring_body(debug2_utstr);
    const char *end = NULL;

    while ( ( end = strchr(debug,'\n')) != NULL ) {
        outfmt("%.*s\n", end - debug, debug);
        debug = end+1;        
    }
}

/*
 ***********************************************************************
 *
 *
 *      Routines To Write Out Scope of labels and to Dump
 *      the static variables, also for dumping the literal pool
 *
 *
 ***********************************************************************
 */

static void dumpfns(void)
{
    int type, storage;
    SYMBOL* ptr;
    FILE* fp;

    outstr("\n\n; --- Start of Scope Defns ---\n\n");

    if (!glbcnt)
        return;

    for ( ptr = symtab; ptr != NULL; ptr = ptr->hh.next ) {
        if (ptr->name[0] != '0' && ptr->ctype ) {
            type = ptr->type;
            storage = ptr->storage;
            if ( type == KIND_ENUM )
                continue;
            if ( ptr->ident == ID_MACRO )
                continue;
            if (ptr->ctype->kind == KIND_PORT8 || ptr->ctype->kind == KIND_PORT16 ) {
                outfmt("\tdefc\t_%s =\t%d\n", ptr->name, ptr->ctype->value);
            } else {
                if ( storage != LSTATIC && storage != TYPDEF ) {
                    GlobalPrefix();                    
                    outname(ptr->name, dopref(ptr)); nl();
                    debug_write_symbol(ptr);
                    if ( ptr->ctype->flags & BANKED && c_banked_style == BANKED_STYLE_TICALC) {
                         GlobalPrefix();
                         outstr(BANKED_SYMBOL_PREFIX);
                         outname(ptr->name, dopref(ptr)); nl();
                    }
                }
                if ( ptr->flags & ASSIGNED_ADDR ) {
                    outfmt("\tdefc\t"); outname(ptr->name,1); outfmt("\t= %d\n", ptr->ctype->value);
                } 
            }
        }
    }

    if ((fp = fopen(c_zcc_opt, "a")) == NULL) {
        errorfmt("Can't open zcc_opt.def file", 1);
    }

    if (need_floatpack) {
        fprintf(fp, "\nIF !NEED_floatpack\n");
        fprintf(fp, "\tDEFINE\tNEED_floatpack\n");
        fprintf(fp, "ENDIF\n\n");
    }
    /*
     * Now, we're gonna use #pragma define _FAR_PTR to indicate whether we need
     * far stuff - this has to go with a -D_FAR_PTR from the compile line
     * as well for everything to work just right, so if we find this then
     * we can indicate to the startup code via zcc_opt.def what the scam
     * is - this could be used for eg. to allocate space for file structures
     * etc
     */
    if ((ptr = findglb("_FAR_PTR")) && ptr->ident == ID_MACRO) {
        fprintf(fp, "\nIF !NEED_farstartup\n");
        fprintf(fp, "\tDEFINE NEED_farstartup\n");
        fprintf(fp, "ENDIF\n\n");
    }

    if (printf_format_option) {
        fprintf(fp, "\nIF !DEFINED_CRT_printf_format\n");
        fprintf(fp, "\tdefc\tDEFINED_CRT_printf_format = 1\n");
        fprintf(fp, "\tdefc CRT_printf_format = 0x%08x\n", printf_format_option);
        fprintf(fp, "ELSE\n");
        fprintf(fp, "\tUNDEFINE temp_printf_format\n");
        fprintf(fp, "\tdefc temp_printf_format = CRT_printf_format\n");
        fprintf(fp, "\tUNDEFINE CRT_printf_format\n");
        fprintf(fp, "\tdefc CRT_printf_format = temp_printf_format | 0x%08x\n", printf_format_option);
        fprintf(fp, "ENDIF\n\n");
        fprintf(fp, "\nIF !NEED_printf\n");
        fprintf(fp, "\tDEFINE\tNEED_printf\n");
        fprintf(fp, "ENDIF\n\n");
    }

    if (scanf_format_option) {
        fprintf(fp, "\nIF !DEFINED_CRT_scanf_format\n");
        fprintf(fp, "\tdefc\tDEFINED_CRT_scanf_format = 1\n");
        fprintf(fp, "\tdefc CRT_scanf_format = 0x%08x\n", scanf_format_option);
        fprintf(fp, "ELSE\n");
        fprintf(fp, "\tUNDEFINE temp_scanf_format\n");
        fprintf(fp, "\tdefc temp_scanf_format = CRT_scanf_format\n");
        fprintf(fp, "\tUNDEFINE CRT_scanf_format\n");
        fprintf(fp, "\tdefc CRT_scanf_format = temp_scanf_format | 0x%08x\n", scanf_format_option);
        fprintf(fp, "ENDIF\n\n");
        fprintf(fp, "\nIF !NEED_scanf\n");
        fprintf(fp, "\tDEFINE\tNEED_scanf\n");
        fprintf(fp, "ENDIF\n\n");
    }
    fclose(fp);


    outstr("\n\n; --- End of Scope Defns ---\n\n");
}

/*
 * Dump a function into the zcc_opt.def file - this allows us to pass
 * important functions to the appstartup code so it can call them
 * when appropriate, we also XREF it so we don't have to do that in
 * the startup code
 */

void WriteDefined(char* sname, int value)
{
    FILE* fp;

    if ((fp = fopen(c_zcc_opt, "a")) == NULL) {
        errorfmt("Can't open zcc_opt.def file", 1);
    }
    fprintf(fp, "\nIF !DEFINED_%s\n", sname);
    fprintf(fp, "\tdefc\tDEFINED_%s = 1\n", sname);
    fprintf(fp, "\tdefc %s = %d\n", sname, value);
    fprintf(fp, "ENDIF\n\n");
    fclose(fp);
}

/*
 */
void dumpvars(void)
{
    int ident, type, storage;
    SYMBOL* ptr;

    if (!glbcnt)
        return;

    /* Start at the start! */
    outstr("; --- Start of Static Variables ---\n\n");

    gen_switch_section(c_bss_section); // gen_switch_section("bss");

    for ( ptr = symtab; ptr != NULL; ptr = ptr->hh.next ) {
        if (ptr->name[0] != '0' ) {
            ident = ptr->ident;
            type = ptr->ctype ? ptr->ctype->kind : KIND_NONE;
            storage = ptr->storage;
            if ( ptr->initialised )
                continue;
            if ( ident == ID_ENUM || ident == ID_MACRO || ident == ID_GOTOLABEL )
                continue;
            if ( type == KIND_FUNC || type == KIND_PORT8 || type == KIND_PORT16 )
                continue;
            if ( storage == TYPDEF ||  storage == EXTERNAL ) 
                continue;
            if ( type == KIND_ENUM )
                continue;
            if ( ptr->ctype->size == -1 )
                continue;
            if ( ptr->bss_section ) gen_switch_section(ptr->bss_section);
            prefix();
            outname(ptr->name, 1);
            col();
            defstorage();
            outdec(ptr->ctype->size);
            nl();
        }
    }

    /* Switch back to standard section */
    gen_switch_section(c_code_section); // gen_switch_section("code");
}

/*
 *      Dump the literal pool if it's not empty
 *      Modified by djm to be able to input which queue should be
 *      dumped..
 */
void dumplits(
    int size, int pr_label,
    int queueptr, int queuelab,
    unsigned char* queue)
{
    int j, k, lit;

    if (queueptr) {
        if (pr_label) {
            gen_switch_section(c_rodata_section); // gen_switch_section("text");
            prefix();
            queuelabel(queuelab);
            col();
            nl();
        }
        k = 0;
        while (k < queueptr) {
            /* pseudo-op to define byte */
            if (infunc)
                j = 1;
            else
                j = 10;
            if (size == 1)
                defbyte();
            else if (size == 4)
                deflong();
            else if (size == 0) {
                defmesg();
                j = 30;
            } else if ( size == 6 ) {
                defbyte();
                j = 6;
                size = 1;
            } else if ( size == 8 ) {
                defbyte();
                j = 8;
                size = 1;
            } else
                defword();
            while (j--) {
                if (size == 0) {
                    lit = getint(queue + k, 1);
                    if (lit >= 32 && lit <= 126 && lit != '"' && lit != '\\')
                        outbyte(lit);
                    else {
                        outstr("\"\n");
                        defbyte();
                        outdec(lit);
                        nl();
                        lit = 0;
                    }
                    k++;
                    if (j == 0 || k >= queueptr || lit == 0) {
                        if (lit)
                            outbyte('"');
                        nl();
                        break;
                    }
                } else {
                    outdec(getint(queue + k, size));
                    k += size;
                    if (j == 0 || k >= queueptr) {
                        nl(); /* need <cr> */
                        break;
                    }
                    outbyte(','); /* separate bytes */
                }
            }
        }
        //gen_switch_section(c_code_section); // gen_switch_section("code");
    }
}

/*
 * dump zeroes for default initial value
 * (or rather, get loader to do it for us)
 */
int dumpzero(int size, int count)
{
    if (count <= 0)
        return (0);
    defstorage();
    outdec(size * count);
    nl();
    return (size * count);
}

/********************************************************************
 *
 *      Routines to open the assembly and C source files
 *
 ********************************************************************
 */

/*
 *      Get output filename
 */
void openout(void)
{
    char filen2[FILENAME_LEN + 1];
    char extension[FILENAME_LEN+1];

    snprintf(extension,sizeof(extension),".%s",c_output_extension);
    FILE* fp;
    clear(); /* erase line */
    output = 0; /* start with none */
    if (nextarg(filenum, filen2, FILENAME_LEN) == NULL)
        return;
    if ((fp = fopen(filen2, "r")) == NULL) {
        fprintf(stderr, "Cannot open source file: %s\n", filen2);
        exit(1);
    }
    fclose(fp); /* Close it again... */

    /* copy file name to string */
    strcpy(Filename, filen2);

    if ( c_output_file != NULL ) {
        if ((output = fopen(c_output_file, "w")) == NULL && (!eof)) {
            fprintf(stderr, "Cannot open output file: %s\n", line);
            exit(1);
        }
    } else {
        strcpy(Filenorig, filen2);
        changesuffix(filen2, extension); /* Change appendix to .asm */
        if ((output = fopen(filen2, "w")) == NULL && (!eof)) {
            fprintf(stderr, "Cannot open output file: %s\n", line);
            exit(1);
        }
    }
    clear(); /* erase line */
}

/*
 *      Get (next) input file
 */
void openin(void)
{
    input = NULL; /* none to start with */
    while (input == NULL) { /* any above 1 allowed */
        clear(); /* clear line */
        if (eof)
            break; /* if user said none */
        /* Deleted hopefully irrelevant code */
        if (Filename[0] == '-') {
            if (ncomp == 0)
                info();
            exit(1);
        }
        if ((input = fopen(Filename, "r")) == NULL) {
            fprintf(stderr, "Can't open: %s\n", Filename);
            exit(1);
        } else {
            unsigned char bombuf[4];
            unsigned char utf8bom[] = { 0xef, 0xbb, 0xbf };
            if (c_verbose)
                fprintf(stderr, "Compiling: %s\n", Filename);
            ncomp++;
            // (void) ! to suppress warn unused result
            (void) !fread(bombuf, sizeof(char), 3, input);
            if ( memcmp(bombuf, utf8bom, 3) ) {
                rewind(input);
            }
            
            newfile();
        }
    }
    clear(); /* erase line */
}

/*
 *      Reset line count, etc.
 */
void newfile(void)
{
    lineno = /* no lines read */
        infunc = 0; /* therefore not in fn. */
    currfn = NULL; /* no fn. yet */
}

/*
 *      Open an include file
 */
void doinclude(void)
{
    char name[FILENAME_LEN + 1], *cp;

    blanks(); /* skip over to name */
    if (c_verbose) {
        toconsole();
        outstr(line);
        nl();
        tofile();
    }

    if (inpt2) {
        errorfmt("Can't nest include files", 1);
    } else {
        /* ignore quotes or angle brackets round file name */
        strcpy(name, line + lptr);
        cp = name;
        if (*cp == '"' || *cp == '<') {
            name[strlen(name) - 1] = '\0';
            ++cp;
        }
        if ((inpt2 = fopen(cp, "r")) == NULL) {
            errorfmt( "Can't open include file", 1 );
            closeout();
            exit(1);
        } else {
            saveline = lineno;
            savecurr = currfn;
            saveinfn = infunc;
            newfile();
        }
    }
    clear(); /* clear rest of line */
    /* so next read will come from */
    /* new file (if open) */
}

/*
 *      Close an include file
 */
void endinclude(void)
{
    if (c_verbose) {
        toconsole();
        outstr("#end include\n");
        tofile();
    }

    inpt2 = 0;
    lineno = saveline;
    currfn = savecurr;
    infunc = saveinfn;
}

/*
 *      Close the output file
 */
void closeout(void)
{
    tofile(); /* if diverted, return to file */
    if (output) {
        /* if open, close it */
        fclose(output);
    }
    output = 0; /* mark as closed */
}




static void opt_code_speed(option *arg, char* val)
{
    char   *ptr = val - 1;

    c_speed_optimisation = 0;

    do {
        ptr++;
        if ( strncmp(ptr,"all",3) == 0 ) {
            c_speed_optimisation = ~0;
            break;
        } else if ( strncmp(ptr, "lshift32", 8) == 0 ) {
            c_speed_optimisation |= OPT_LSHIFT32;
        } else if ( strncmp(ptr, "rshift32", 8) == 0 ) {
            c_speed_optimisation |= OPT_RSHIFT32;
        } else if ( strncmp(ptr, "add32", 5) == 0 ) {
            c_speed_optimisation |= OPT_ADD32;
        } else if ( strncmp(ptr, "sub32", 5) == 0 ) {
            c_speed_optimisation |= OPT_SUB32;
        } else if ( strncmp(ptr, "sub16", 5) == 0 ) {
            c_speed_optimisation |= OPT_SUB16;
        } else if ( strncmp(ptr, "intcompare", 10) == 0 ) {
            c_speed_optimisation |= OPT_INT_COMPARE;
        } else if ( strncmp(ptr, "charcompare", 10) == 0 ) {
            c_speed_optimisation |= OPT_CHAR_COMPARE;
        } else if ( strncmp(ptr, "longcompare", 11) == 0 ) {
            c_speed_optimisation |= OPT_LONG_COMPARE;
        } else if ( strncmp(ptr, "ucharmult", 9) == 0 ) {
            c_speed_optimisation |= OPT_UCHAR_MULT;
        } else if ( strncmp(ptr, "floatconst", 10) == 0 ) {
            c_speed_optimisation |= OPT_DOUBLE_CONST;
        }
    } while ( (ptr = strchr(ptr, ',')) != NULL );
}



void SetDefine(option *arg, char* val)
{
    defmac(val);
}

void SetUndefine(option *arg, char* val)
{
    strcpy(line, val);
    delmac();
}


void SetWarning(option *arg, char *value) 
{
    parse_warning_option(value);
}


void DispInfo(option *arg, char *val)
{
    option *cur = &sccz80_opts[0];
    info();

    option_list(cur);

    exit(1);
}

void DispVersion(char* arg)
{
    info();
    exit(1);
}




/*
 *      This routine called via atexit to clean up memory
 */

void atexit_deallocate(void)
{
    FREENULL(litq);
    FREENULL(dubq);
    FREENULL(tempq);
    FREENULL(glbq);
    FREENULL(loctab);
    FREENULL(wqueue);
//    FREENULL(swnext);
    FREENULL(stage);
    FREENULL(gotoq);
}

void* mymalloc(size_t size)
{
    void* ptr;

    if ((ptr = calloc(size, 1)) != NULL)
        return ptr;
    else {
        fprintf(stderr, "Out of memory...\n");
        exit(1);
    }
    return 0; /* Sigh */
}

