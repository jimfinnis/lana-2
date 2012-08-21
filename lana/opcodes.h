#ifndef __OPCODES_H
#define __OPCODES_H

/**
 * @file
 * A list of all the opcodes handled by the virtual machine
 * and macros for assembling instructions and splitting them into opcode
 * and data.
 */

/// generate an instruction from 8 bits of operation and 24 bits of data
#define INST(op,data) ((op)+((data)<<8))
/// get the operation (the actual opcode) of an instruction
#define INSTOP(opcode) ((opcode) & 0xff)
/// get the data field of an instruction
#define INSTDATA(opcode) ((opcode) >> 8)

#define OP_LIT		1
#define OP_DUMMY	2
#define OP_COMMENT_SOL	3
#define OP_ADD		4
#define OP_SUB		5
#define OP_MUL		6
#define OP_DIV		7
#define OP_END		8
#define OP_EQUALS	9
#define OP_CALL		11
#define OP_LOCALS	12
#define OP_GET		13
#define OP_SET		14
#define OP_PROPREF	15
#define OP_IF		16
#define OP_ELSEIF	17
#define OP_ELSE		18
#define OP_ENDIF	19
#define OP_JMPELSEIF	20
#define OP_WHILE	21
#define OP_ENDWHILE	22
#define OP_RETURN	23
#define OP_REPEAT	24
#define OP_UNTIL	25
#define OP_NEQUALS	26
#define OP_SPARE1	27
#define OP_PAREN	28
#define OP_NEGATE	29
#define OP_NEAREQ	30
#define OP_NNEAREQ	31
#define OP_NOT		32
#define OP_SPECIAL	33
#define OP_LT		34
#define OP_LTE		35
#define OP_GT		36
#define OP_GTE		37
#define OP_SRCLINE	38
#define OP_SRCFILE	39
#define OP_COMMENT_EOL	40
#define OP_BLANKLINE	41
#define OP_COMMENT_EOFD	42
#define OP_GOTOFW	43
#define OP_GOTOBK	44
#define OP_LABEL	45
#define OP_GOTOMARKER	46
#define OP_BREAK	47
#define OP_CONTINUE	48
#define OP_TRUE		49
#define OP_FALSE	50
#define OP_QUICKIF	51
#define OP_THIS		52
#define OP_IMMED	53
#define OP_SQB		54
#define OP_LOGAND	55
#define OP_LOGOR	56
#define OP_BITAND	57
#define OP_BITOR	58
#define OP_BITNOT	59
#define OP_XOR		60
#define OP_FOR		61
#define OP_NEXT		62
#define OP_ENDFOR	63
#define OP_STARTESTMT	64
#define OP_ENDESTMT	65
#define OP_ENDESTMT2	66
#define OP_VARREFLOC	67
#define OP_VARREFPRM	68
#define OP_VARREFSES	69
#define OP_VARREFGLB	70
#define OP_LITIDENT	71
#define OP_MOD		72

#endif /* __OPCODES_H */
