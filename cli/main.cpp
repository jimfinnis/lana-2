#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


#include <readline/readline.h>
#include <readline/history.h>


#include "lana/api.h"
#include "lana/debug.h"
#include "../tests/asserter.h"


/*
 * Main test program
 */

int main(int argc,char *argv[]) {
    
    lana::API *api = new lana::API;
    lana::Session *ses = new lana::Session(api);
    
    api->setArgcArgv(argc,argv);
//    TestObject::reg(api);
    
    printf("%s command line interpreter\n",LANA_API_VERSION);
    
    AsserterHost *ah = new AsserterHost(api);
    
    char c;
    
    int debflags = 0;
    bool interactive=false;
    opterr=0;
    while((c=getopt(argc,argv,"dsegtr"))!=-1){
        switch(c){
        case 'r':
            interactive = true;
            break;
        case 'd':
            debflags |= LDEBUG_DUMP;
            break;
        case 's':
            debflags |= LDEBUG_SHOW;
            break;
        case 'e':
            debflags |= LDEBUG_EMIT;
            break;
        case 't':
            debflags |= LDEBUG_TRACE;
            break;
        case 'g':
            debflags |= LDEBUG_SRCDATA;
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            fprintf(stderr,"usage: lana [-d][-s][-e][-t][-g][-r] [file.l]\n");
            fprintf(stderr,"   -r : read the file then go into interactive mode\n");
            fprintf(stderr,"   -d : dump bytecode after each compilation\n");
            fprintf(stderr,"   -s : show lines being fed into compiler\n");
            fprintf(stderr,"   -e : show bytecode emission\n");
            fprintf(stderr,"   -g : add debugging opcodes\n");
            fprintf(stderr,"   -t : trace bytecode execution\n");
            return 1;
        default:
            abort();
        }
    }
    
    
    api->setDebug(debflags);
    
    if(argc>optind){
        try {
            ses->feedFile(argv[optind]);
        } catch(lana::Exception &e) {
            printf("error: %s\n",e.what());
        }
    }
    
    if(argc<=optind || interactive){
        for(;;){
            char *line;
            const char *prompt = ses->awaitingInput() ? " > " : ". ";
            
#if defined(HAVE_READLINE_READLINE_H)
            line = readline(prompt);
            if(!line)break; // end-of-file
#else
            char buf[1024];
            fputs(prompt,stdout);
            fgets(buf,1024,stdin);
            if(feof(stdin))break; // end-of-file
            if(buf[0]=='*')break;
            line=buf;
#endif
            
            try{
#if defined(HAVE_READLINE_HISTORY_H)
                // line only added to history if non-zero length
                if(*line)
                    add_history(line);
#endif
                ses->feed(line);
            } catch(lana::Exception &e) {
                printf("error: %s\n",e.what());
            }
        }
        printf("Bye!\n");
    }
    
    delete ah;
    delete ses;
    delete api;
    
    return 0;
}
