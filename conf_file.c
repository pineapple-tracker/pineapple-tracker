#include "pineapple.h"
#include "gui.h"
#include "conf_file.h"
static const int fvar_nums = 5000;
static const char cfilename[] = ".pineapplerc";

/* Struct for name,value pairs*/
struct variable {
    char *name;
    int value;
};
/*
* Is supposed to parse through the file, but is being a dumb cluster-f.
*/
char** parse_file(const char filename[], int fvar_nums){
	FILE *file = fopen ( filename, "r" );
	int i, j;
	char line[500]; /* or other suitable maximum line size */
	char **conf_vars;
	conf_vars=malloc( fvar_nums * sizeof(char *));
	for(i=0;i<fvar_nums;i++)
	{
		conf_vars[i]=malloc(fvar_nums * sizeof(char *));
	}
	
	for(i=0; i<fvar_nums; i++)
	{
		for(j=0; j<fvar_nums; j++)
		{
		conf_vars[i][j] = '\0';
		}
	}

	for(i=0; i<500; i++)
	{
		line[i] = '\0';
	}

	if ( file != NULL )
	{
		i=0;
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			strcpy(conf_vars[i], line);
			i++;
		}
		fclose ( file );
	}
	return conf_vars;
	//return 0;
}

/*
*Assigns variables
*/
void filevars ( const char **blah[], int el ){
	char *ini[el];
	size_t i;
	for(i=0;i<el;i++){
		ini[i] = &(*blah[i]);
		//printf("%s\n", blah[i]);
	}
    struct variable vars[] = {
        {"poop"},
        {"butt"}
    };
    

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

    //return 0;
}

int like_main( void){
	//int i;
	//char ** array = parse_file(filename, fvar_nums);
	/*char *ini[fvar_nums];
	for(i=0; i<=sizeof(array)/sizeof(int); i++){
		ini[i] = array[i];
		printf("%s", ini[i]);
	}*/
	char *ini[] = {
    "var1=5",
    "butt=80"
	};
	filevars(ini, length(ini));
	return 0;
}
