#include "pineapple.h"
#include "gui.h"
#include "conf_file.h"

struct variable {
    char *name;
    int value;
};
/*
Just a test array, will be replaced by file line by line
*/
static const char *ini[] = {
    "var1=5",
    "var2=80"
};

static const char *lines[];

int conf_file ( void )
{
		//static const char filename[] = ".pineapplerc";
		//FILE *file = fopen(filename, "r");
		//if (file){
		//				char line[BUFSIZ];
		
		
		//Put all desired variable names in here, such as bigmv, or whatev
    struct variable vars[] = {
        {"var1"},
        {"var2"}
    };
    size_t i;

    for ( i = 0; i < length ( ini ); i++ ) {
        char temp[BUFSIZ];
        char *name;
        size_t j;

        /* Simulate reading from a stream */
        strcpy ( temp, ini[i] );

        if ( ( name = strtok ( temp, "=" ) ) == NULL )
            continue;

        for ( j = 0; j < length ( vars ); j++ ) {
            if ( strcmp ( vars[j].name, name ) == 0 )
                break;
        }

        if ( j != length ( vars ) ) {
            /*
                - Assumes there's another token
                - Assumes the next token is a valid int
								- Should have some precautionary measure
            */
            vars[j].value = atoi ( strtok ( NULL, "=" ) );
        }
    }
		//Just a for loop to show that something happened, not important
    for ( i = 0; i < length ( vars ); i++ )
        printf ( "%s = %d\n", vars[i].name, vars[i].value );

    return 0;
}

