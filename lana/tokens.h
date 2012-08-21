/**
 * @file
 * A list of all the tokens produced by the Tokeniser, along with
 * a few "pseudotokens" used in expression parsing, etc.
 */

#ifndef __TOKENS_H
#define __TOKENS_H

#define T_EQUALS        0
#define T_MUL           1
#define T_ADD           2
#define T_DIV           3
#define T_SUB           4
#define T_OPREN         5
#define T_CPREN         6
#define T_SEMI          7
#define T_IDENT         8
#define T_STRING        9
#define T_INT           10
#define T_END           11
#define T_OCURLY        12
#define T_CCURLY        13

#define T_IF		14
#define T_ENDIF		15
#define T_ELSE		16
#define T_WHILE		17
#define T_ENDWHILE	18

#define T_COMMA		19
#define T_FUNCTION	20
#define T_RETURNS	21
#define T_ENDFUNC	22
#define T_DOT		23
#define T_FLOAT		24
#define T_RETURN	25
#define T_REPEAT	26
#define T_UNTIL		27
#define T_NEAREQ	28
#define T_PLING		29
#define T_LT		30
#define T_GT		31
#define T_ELSEIF	32
#define T_COMMENT	33
#define T_COLON		34
#define T_GOTO		35
#define T_BREAK		36
#define T_CONTINUE	37
#define T_TRUE		38
#define T_FALSE		39
#define T_THIS		40
#define T_OSQB		41
#define T_CSQB		42
#define T_BITAND	43
#define T_BITOR		44
#define T_BITNOT	45
#define T_XOR		46
#define T_FOR		47
#define T_IN		48
#define T_ENDFOR	49
#define T_PROC		50
#define T_BACKTICK	51
//#define T_DOLLAR	52
#define T_LOAD		53
#define T_SAVE		54
#define T_SAVEVAR	55
#define T_PERC		56


// some pseudotokens, used as operators typically
#define T_EQUALITY	200
#define T_ASSIGN	201
#define T_NEGATE	202
#define T_NOTEQUALS	203
#define T_NOTNEAREQUALS	204
#define T_NOT		205
#define T_LTE		206
#define T_GTE		207
#define T_LOGAND	208
#define T_LOGOR		209
#endif /* __TOKENS_H */
