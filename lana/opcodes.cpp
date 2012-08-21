#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "language.h"
#include "cg.h"
#include "consts.h"
#include "tokeniser.h"
#include "tokens.h"
#include "opcodes.h"
#include "growable.h"
#include "session.h"

const char *opcodes[] = {
    "",
    "lit","dummy","comment_sol",
    "add","sub","mul","div",
    "end","equals","SPARE0",
    "call",
    "locals","get","set","propref",
    "if","elseif","else","endif","jmpelseif",
    "while","endwhile","return","repeat","until","notequals",
    "SPARE1","paren","negate","neareq","notneareq","not",
    "special","lt","lte","gt","gte","srcline","srcfile","comment_eol",
    "blankline","comment_eofd","gotofw","gotobk","label","gotomarker","break","continue",
    "true","false","quickif","this","immed","sqb",
    "logand","logor","bitand","bitor","bitnot","xor","for","next","endfor",
    "startestmt","endestmt","endestmt2","varrefloc","varrefprm","varrefses",
    "varrefglb","litident","mod",
};

char *Language::dumpInst(instruction *p,Session *ses){
    static char buf[256];
    char *name;
    
    int op = INSTOP(*p);
    int d = INSTDATA(*p);
    switch(op){
    case OP_VARREFLOC:
        name = getLocalName(d);
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],name,d);
        break;
    case OP_VARREFPRM:
        name = getParamName(d);
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],name,d);
        break;
    case OP_VARREFGLB:
        name = getGlobalName(d);
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],name,d);
        break;
    case OP_VARREFSES:
        name = ses->getSesVarName(d);
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],name,d);
        break;
    // things with forward jumps
    case OP_WHILE:
    case OP_FOR:
    case OP_ELSE:
    case OP_ELSEIF:
    case OP_JMPELSEIF:
    case OP_GOTOFW:
    case OP_BREAK:
    case OP_QUICKIF:
    case OP_IF: {
            instruction *dest = p+INSTDATA(*p);
            sprintf(buf,"%8x   %2d: %10s (-> %8x) (0x%x)",p,op,opcodes[op],dest,d);
    }
        break;
   /// things with backward jumps
    case OP_GOTOBK:
    case OP_UNTIL:
    case OP_NEXT:
    case OP_CONTINUE:
    case OP_ENDWHILE:{
            instruction *dest = p-INSTDATA(*p);
            sprintf(buf,"%8x   %2d: %10s (-> %8x) (0x%x)",p,op,opcodes[op],dest,d);
    }
        break;
    case OP_PROPREF:{
        const char *name = consts->getStr(d);
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],name,d);
    }
        break;
    case OP_COMMENT_SOL:
    case OP_COMMENT_EOFD:
    case OP_COMMENT_EOL:{
        ConstDesc *e = consts->get(d);
        char *mem = (char *)e->get();
        int pos = *(short*)mem;
        mem+=sizeof(short);
        sprintf(buf,"%8x   %2d: %10s (%s/%d) (%d)",p,op,opcodes[op],mem,pos,d);
    }
    case OP_LABEL:
    case OP_GOTOMARKER:
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],consts->getStr(d),d);
        break;
    case OP_IMMED:
        sprintf(buf,"%8x   %2d: %10s (%d)",p,op,opcodes[op],d);
        break;
    case OP_LIT: {
        char out[256];
        ConstDesc *e = consts->get(d);
        if(e){
            char *ptr = (char *)e->get();
            
            switch(e->getType()){
            case CT_STRING:
                out[0]='\'';
                strncpy(out+1,ptr,64);
                strcat(out,"'");
                break;
            case CT_INT:
                sprintf(out,"%d",*(int *)ptr);
                break;
            case CT_FLOAT:
                sprintf(out,"%f",*(float *)ptr);
                break;
            case CT_FUNC:
                strcpy(out,"<<FUNCTION>>");
                break;
            case CT_LDT:
                strcpy(out,"<<LDT>>");
                break;
            case CT_COMMENT:
                strcpy(out,"<<COMMENT>>");
            default:
                strcpy(out,"<<BADTYPE>>");
                break;
            }
        }else{
            strcpy(out,"<<NOENT>>");
        }
        sprintf(buf,"%8x   %2d: %10s (%s) (%d)",p,op,opcodes[op],out,d);
        break;
    }
    default:
        sprintf(buf,"%8x   %2d: %10s (%d)",p,op,opcodes[op],d);
    }
    return buf;
}
